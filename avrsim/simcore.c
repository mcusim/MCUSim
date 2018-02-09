/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mcusim/hex/ihex.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/peripheral_lua.h"
#include "mcusim/avr/sim/decoder.h"
#include "mcusim/avr/sim/gdb_rsp.h"
#include "mcusim/avr/sim/vcd_dump.h"
#include "mcusim/avr/sim/interrupt.h"

#define REG_ZH			0x1F
#define REG_ZL			0x1E

typedef int (*init_func)(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Function to process interrupt request according to the order */
static int handle_irq(struct MSIM_AVR *mcu);

/* Cell contains AVR MCU part and its init function. */
struct init_cell {
	char partno[20];
	char name[20];
	init_func f;
};

/* Init functions for supported AVR MCUs. */
static struct init_cell init_funcs[] = {
	{ "m8",		"ATmega8",	MSIM_M8AInit },
	{ "m8a",	"ATmega8A",	MSIM_M8AInit },
	{ "m328",	"ATmega328",	MSIM_M328Init },
	{ "m328p",	"ATmega328P",	MSIM_M328PInit },
	{ "m2560",	"ATmega2560",	MSIM_M2560Init }
};

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp);

int MSIM_SimulateAVR(struct MSIM_AVR *mcu, unsigned long steps,
		     unsigned long addr)
{
	unsigned long tick;	/* Number of pulses sinse simulation start */
	unsigned char fall;	/* Main clock rise or a fall flag */
	FILE *vcd_f;

	tick = fall = 0;

	vcd_f = NULL;
	/* Do we have registers to dump? */
	if (mcu->vcd_regsn[0] >= 0) {
		vcd_f = MSIM_VCDOpenDump(mcu, "dump.vcd");
		if (!vcd_f) {
			fprintf(stderr, "ERRO: Failed to open dump "
					"file: dump.vcd\n");
			return -1;
		}
	}

	/*
	 * Main simulation loop. Each iteration represents rise (R) or
	 * fall (F) of the microcontroller's clock pulse. It's necessary
	 * to dump CLK_IO to the timing diagram in a pulse-accurate way.
	 *
	 *                          MAIN LOOP ITERATIONS
	 *            R     F     R     F     R     F     R     F     R
	 *           /     /     /     /     /     /     /     /     /
	 *
	 *          |           |           |           |           |
	 *          |_____      |_____      |_____      |_____      |_____
	 *          |     |     |     |     |     |     |     |     |     |
	 * CLK_IO   |     |     |     |     |     |     |     |     |     |
	 *          |     |_____|     |_____|     |_____|     |_____|     |__
	 *          |           |           |           |           |
	 *          |           |___________|           |___________|
	 *          |           |           |           |           |
	 * CLK_IO/2 |           |           |           |           |
	 *          |___________|           |___________|           |________
	 *          |           |           |           |           |
	 *          |           |           |           |           |
	 *          |           |           |           |           |
	 */
	while (1) {
		/* Terminate simulation? */
		if (!fall && mcu->state == AVR_MSIM_STOP)
			break;
		/* Wait for request from GDB in MCU stopped mode */
		if (!fall && mcu->state == AVR_STOPPED && MSIM_RSPHandle()) {
			if (vcd_f != NULL)
				fclose(vcd_f);
			return 1;
		}

		/* Tick peripherals written in Lua */
		if (!fall)
			MSIM_TickLuaPeripherals(mcu);
		/* Tick timers. NOTE: MCU-specific! */
		if (!fall && (mcu->tick_timers != NULL))
			mcu->tick_timers(mcu);
		/* Dump registers to VCD */
		if (vcd_f)
			MSIM_VCDDumpFrame(vcd_f, mcu, tick, fall);

		/* Test scope of program counter */
		if ((mcu->pc+1) >= mcu->pm_size) {
			if (vcd_f != NULL)
				fclose(vcd_f);
			fprintf(stderr, "ERRO: Program counter is out of "
					"scope: pc=0x%4lX, pc+1=0x%4lX, "
					"flash_addr=0x%4lX\n",
					mcu->pc, mcu->pc+1, mcu->pm_size-1);
			return 1;
		}

		/* Decode next instruction */
		if (!fall && (mcu->state == AVR_RUNNING ||
		    mcu->state == AVR_MSIM_STEP)) {
			if (MSIM_StepAVR(mcu)) {
				if (vcd_f != NULL)
					fclose(vcd_f);
				return 1;
			}
		}

		/* Provide IRQs based on MCU flags. NOTE: MCU-specific! */
		if (!fall && (mcu->provide_irqs != NULL))
			mcu->provide_irqs(mcu);

		/* Handle IRQ if interrupts are enabled globally */
		if (!fall &&
		    MSIM_ReadSREGFlag(mcu, AVR_SREG_GLOB_INT) &&
		    mcu->intr->exec_main == 0 &&
		    (mcu->state == AVR_RUNNING || mcu->state == AVR_MSIM_STEP))
			handle_irq(mcu);

		/* Halt MCU after a single step performed */
		if (!fall && mcu->state == AVR_MSIM_STEP)
			mcu->state = AVR_STOPPED;

		mcu->intr->exec_main = 0;
		tick++;
		fall = !fall ? 1 : 0;
	}
	if (vcd_f != NULL)
		fclose(vcd_f);
	return 0;
}

int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size,
		 unsigned char *mpm, FILE *fp)
{
	unsigned int i;
	char mcu_found = 0;
	struct MSIM_InitArgs args;

	args.pm = pm;
	args.dm = dm;
	args.pmsz = pm_size;
	args.dmsz = dm_size;

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++)
		if (!strcmp(init_funcs[i].partno, mcu_name)) {
			if (init_funcs[i].f(mcu, &args))
				return -1;
			else
				mcu_found = 1;
		}

	if (!mcu_found) {
		fprintf(stderr, "ERRO: Model is not supported: %s\n",
				mcu_name);
		return -1;
	}

	if (load_progmem(mcu, fp)) {
		fprintf(stderr, "ERRO: Program memory cannot be loaded from a "
				"file\n");
		return -1;
	}
	mcu->state = AVR_STOPPED;
	mcu->mpm = mpm;
	mcu->read_from_mpm = 0;
	return 0;
}

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp)
{
	IHexRecord rec, mem_rec;

	if (!fp) {
		fprintf(stderr, "ERRO: Cannot read from the filestream\n");
		return -1;
	}

	/* Copy HEX data to program memory of the MCU */
	while (Read_IHexRecord(&rec, fp) == IHEX_OK) {
		switch (rec.type) {
		case IHEX_TYPE_00:	/* Data */
			memcpy(mcu->pm + rec.address,
			       rec.data, (uint16_t) rec.dataLen);
			break;
		case IHEX_TYPE_01:	/* End of File */
		default:		/* Other types, unlikely occured */
			continue;
		}
	}

	/* Verify checksum of the loaded data */
	rewind(fp);
	while (Read_IHexRecord(&rec, fp) == IHEX_OK) {
		if (rec.type != IHEX_TYPE_00)
			continue;

		memcpy(mem_rec.data, mcu->pm + rec.address,
		       (uint16_t) rec.dataLen);
		mem_rec.address = rec.address;
		mem_rec.dataLen = rec.dataLen;
		mem_rec.type = rec.type;
		mem_rec.checksum = 0;

		mem_rec.checksum = Checksum_IHexRecord(&mem_rec);
		if (mem_rec.checksum != rec.checksum) {
			printf("ERRO: Checksum is not correct: 0x%X (memory) "
			       "!= 0x%X (file)\nFile record:\n",
			       mem_rec.checksum, rec.checksum);
			Print_IHexRecord(&rec);
			printf("ERRO: Memory record: \n");
			Print_IHexRecord(&mem_rec);
			return -1;
		}
	}
	return 0;
}

static int handle_irq(struct MSIM_AVR *mcu)
{
	unsigned int i;

	/*
	 * Look for the priority IRQ to process:
	 * i == 0		highest priority
	 * i == AVR_IRQ_NUM-1	lowest priority
	 */
	for (i = 0; i < AVR_IRQ_NUM; i++)
		if (mcu->intr->irq[i] > 0)
			break;

	if (i == AVR_IRQ_NUM) {		/* No IRQ, do nothing */
		return 2;
	} else {			/* Execute ISR */
		/* Clear selected IRQ */
		mcu->intr->irq[i] = 0;

		/* Disable interrupts globally */
		MSIM_UpdateSREGFlag(mcu, AVR_SREG_GLOB_INT, 0);
		/* Push PC onto the stack */
		MSIM_StackPush(mcu, (unsigned char)(mcu->pc & 0xFF));
		MSIM_StackPush(mcu, (unsigned char)((mcu->pc >> 8) & 0xFF));
		/* Load interrupt vector to PC */
		mcu->pc = mcu->intr->ivt+(i*2);

		/* Switch MCU to step mode to notify user about interrupt */
		if (mcu->intr->trap_at_isr && mcu->state == AVR_RUNNING)
			mcu->state = AVR_MSIM_STEP;

		return 0;
	}
}

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu, enum MSIM_AVRSREGFlag flag,
			 unsigned char set_f)
{
	unsigned char v;

	if (!mcu) {
		fprintf(stderr, "ERRO: MCU is null");
		return;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		v = 0x01;
		break;
	case AVR_SREG_ZERO:
		v = 0x02;
		break;
	case AVR_SREG_NEGATIVE:
		v = 0x04;
		break;
	case AVR_SREG_TWOSCOM_OF:
		v = 0x08;
		break;
	case AVR_SREG_SIGN:
		v = 0x10;
		break;
	case AVR_SREG_HALF_CARRY:
		v = 0x20;
		break;
	case AVR_SREG_T_BIT:
		v = 0x40;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		break;
	}

	if (set_f)
		*mcu->sreg |= v;
	else
		*mcu->sreg &= (unsigned char)~v;
}

unsigned char MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
				enum MSIM_AVRSREGFlag flag)
{
	unsigned char pos;

	if (!mcu) {
		fprintf(stderr, "ERRO: MCU is null");
		return UINT8_MAX;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		pos = 0;
		break;
	case AVR_SREG_ZERO:
		pos = 1;
		break;
	case AVR_SREG_NEGATIVE:
		pos = 2;
		break;
	case AVR_SREG_TWOSCOM_OF:
		pos = 3;
		break;
	case AVR_SREG_SIGN:
		pos = 4;
		break;
	case AVR_SREG_HALF_CARRY:
		pos = 5;
		break;
	case AVR_SREG_T_BIT:
		pos = 6;
		break;
	case AVR_SREG_GLOB_INT:
		pos = 7;
		break;
	}

	return (unsigned char)((*mcu->sreg >> pos) & 0x01);
}

void MSIM_StackPush(struct MSIM_AVR *mcu, uint8_t val)
{
	uint16_t sp;

	sp = (uint16_t) ((*mcu->spl) | (*mcu->sph << 8));
	mcu->dm[sp--] = val;
	*mcu->spl = (uint8_t) (sp & 0xFF);
	*mcu->sph = (uint8_t) (sp >> 8);
}

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu)
{
	uint16_t sp;
	uint8_t v;

	sp = (uint16_t) ((*mcu->spl) | (*mcu->sph << 8));
	v = mcu->dm[++sp];
	*mcu->spl = (uint8_t) (sp & 0xFF);
	*mcu->sph = (uint8_t) (sp >> 8);

	return v;
}

void MSIM_PrintParts(void)
{
	unsigned int i;

	printf("INFO: Valid parts are:\n");
	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++)
		printf("INFO: %-10s= %s\n", init_funcs[i].partno,
					    init_funcs[i].name);
}
