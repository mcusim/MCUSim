/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

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
#define CLK_RISE		0
#define CLK_FALL		1

#ifndef ULLONG_MAX
/* For compilers without "unsigned long long" support
 * NOTE: Amount of simulation time for VCD dump will be limited to
 * 134218 ms (MCU clock is 16 MHz) because of a limit of 32-bits
 * unsigned integer.
 */
#define TICKS_MAX	ULONG_MAX
#define ticks_t		unsigned long
#else
#define TICKS_MAX	ULLONG_MAX
#define ticks_t		unsigned long long
#endif

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
                     unsigned long addr, unsigned char firmware_test)
{
	ticks_t tick;		/* Number of rises and falls sinse start */
	unsigned char tick_ovf;	/* Have we reached maximum number of ticks? */
	FILE *vcd_f;		/* File to store VCD dump to */
	int ret_code;		/* Code to return */
	char dump_name[256];

	tick = tick_ovf = 0;
	vcd_f = NULL;
	ret_code = 0;

	/* Do we have registers to dump? */
	if (mcu->vcdd->bit[0].regi >= 0) {
		snprintf(&dump_name[0], sizeof dump_name, "%s-trace.vcd",
		         mcu->name);
		vcd_f = MSIM_VCDOpenDump(mcu, dump_name);
		if (!vcd_f) {
			fprintf(stderr, "[e]: Failed to open dump file: "
			        "dump.vcd\n");
			return -1;
		}
	}

	/* Force MCU to run in firmware test mode. */
	if (firmware_test) {
		mcu->state = AVR_RUNNING;
	}

	/* Main simulation loop. Each iteration represents both rise (R) and
	 * fall (F) of the microcontroller's clock pulse. It's necessary
	 * to dump CLK_IO to the timing diagram in a pulse-accurate way.
	 *
	 *                          MAIN LOOP ITERATIONS
	 *            R     F     R     F     R     F     R     F     R
	 *           /     /     /     /     /     /     /     /     /
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
	 */
	while (1) {
		/* The main simulation loop can be terminated by setting
		 * MCU state to AVR_MSIM_STOP. The primary (and maybe only)
		 * source of this state setting is a command from debugger.
		 */
		if ((mcu->ic_left == 0) && (mcu->state == AVR_MSIM_STOP)) {
			break;
		}
		if ((mcu->ic_left == 0) && (mcu->state == AVR_MSIM_TESTFAIL)) {
			ret_code = 1;
			break;
		}

		/* Wait for request from GDB in MCU stopped mode */
		if (!firmware_test && !mcu->ic_left &&
		                (mcu->state == AVR_STOPPED) &&
		                MSIM_RSPHandle()) {
			if (vcd_f) {
				fclose(vcd_f);
			}
			return 1;
		}

		/* Tick peripherals written in Lua */
		MSIM_TickLuaPeripherals(mcu);
		/* Tick timers (MCU-defined!) */
		if (mcu->tick_timers) {
			mcu->tick_timers(mcu);
		}
		/* Dump registers to VCD */
		if (vcd_f && !tick_ovf) {
			MSIM_VCDDumpFrame(vcd_f, mcu, tick, CLK_RISE);
		}

		/* Test scope of a program counter */
		if ((mcu->pc+1) >= mcu->pm_size) {
			if (vcd_f) {
				fclose(vcd_f);
			}
			fprintf(stderr, "[e]: Program counter is out of "
			        "scope: pc=0x%4lX, pc+1=0x%4lX, "
			        "flash_addr=0x%4lX\n",
			        mcu->pc, mcu->pc+1, mcu->pm_size-1);
			return 1;
		}

		/* Decode next instruction. It's usually hard to say in
		 * which state the MCU registers will be between neighbor
		 * cycles of a multi-cycle instruction. This talk may be
		 * taken into account:
		 * https://electronics.stackexchange.com/questions/132171/
		 * 	what-happens-to-avr-registers-during-multi-
		 * 	cycle-instructions,
		 * but this change of LSB and MSB can be MCU-specific and
		 * not a general way of how it really works. Detailed
		 * information can be obtained directly from Atmel, but
		 * there is no intention to do this in order not to
		 * unveil their secrets. However, any details they're ready
		 * to share are highly welcome.
		 *
		 * Simulator doesn't guarantee anything special
		 * here either. The only thing you may rely on is instruction
		 * which will be completed _after all_ of these cycles
		 * required to finish instruction itself.
		 */
		if ((mcu->ic_left || mcu->state == AVR_RUNNING ||
		                mcu->state == AVR_MSIM_STEP) &&
		                MSIM_StepAVR(mcu)) {
			if (vcd_f) {
				fclose(vcd_f);
			}
			return 1;
		}

		/* Provide IRQs (MCU-defined!) based on MCU flags and handle
		 * them if this is possible.
		 *
		 * It's important to understand an interrupt may occur during
		 * execution of a multi-cycle instruction. This instruction
		 * is completed before the interrupt is served (according to
		 * the multiple AVR datasheets). It means that we may provide
		 * IRQs, but will have to wait required number of cycles
		 * to serve them.
		 */
		if (mcu->provide_irqs) {
			mcu->provide_irqs(mcu);
		}
		if (!mcu->ic_left && !mcu->intr->exec_main &&
		                MSIM_ReadSREGFlag(mcu, AVR_SREG_GLOB_INT) &&
		                (mcu->state == AVR_RUNNING ||
		                 mcu->state == AVR_MSIM_STEP)) {
			handle_irq(mcu);
		}

		/* Halt MCU after a single step performed */
		if (!mcu->ic_left && mcu->state == AVR_MSIM_STEP) {
			mcu->state = AVR_STOPPED;
		}

		/* All cycles of a single instruction from a main program
		 * have to be performed.
		 */
		if (!mcu->ic_left) {
			mcu->intr->exec_main = 0;
		}

		/* Increment ticks or print a warning message in case of
		 * maximum amount of ticks reached (extremely unlikely
		 * if compiler supports "unsigned long long" type).
		 */
		if (tick == TICKS_MAX) {
			tick_ovf = 1;
			printf("[!]: Maximum amount of simulation ticks "
			       "reached!\n");
			if (vcd_f) {
				printf("[!]: VCD dump won't be recorded "
				       "further\n");
			}
		} else {
			tick++;
		}
		/* Dump fall to VCD */
		if (vcd_f && !tick_ovf) {
			MSIM_VCDDumpFrame(vcd_f, mcu, tick, CLK_FALL);
		}
		tick++;
	}

	if (vcd_f) {
		fclose(vcd_f);
	}
	return ret_code;
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

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++) {
		if (!strcmp(init_funcs[i].partno, mcu_name)) {
			if (init_funcs[i].f(mcu, &args)) {
				return -1;
			} else {
				mcu_found = 1;
			}
		}
	}

	if (!mcu_found) {
		fprintf(stderr, "[e]: Model is not supported: %s\n",
		        mcu_name);
		return -1;
	}

	if (load_progmem(mcu, fp)) {
		fprintf(stderr, "[e]: Program memory cannot be loaded from a "
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
		fprintf(stderr, "[e]: Cannot read from the filestream\n");
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
		if (rec.type != IHEX_TYPE_00) {
			continue;
		}

		memcpy(mem_rec.data, mcu->pm + rec.address,
		       (uint16_t) rec.dataLen);
		mem_rec.address = rec.address;
		mem_rec.dataLen = rec.dataLen;
		mem_rec.type = rec.type;
		mem_rec.checksum = 0;

		mem_rec.checksum = Checksum_IHexRecord(&mem_rec);
		if (mem_rec.checksum != rec.checksum) {
			printf("[e]: Checksum is not correct: 0x%X (memory) "
			       "!= 0x%X (file)\nFile record:\n",
			       mem_rec.checksum, rec.checksum);
			Print_IHexRecord(&rec);
			printf("[e]: Memory record: \n");
			Print_IHexRecord(&mem_rec);
			return -1;
		}
	}
	return 0;
}

static int handle_irq(struct MSIM_AVR *mcu)
{
	unsigned int i;
	int ret;

	/* Let's try to find IRQ with the highest priority:
	 * i == 0		highest priority
	 * i == AVR_IRQ_NUM-1	lowest priority
	 */
	for (i = 0; i < AVR_IRQ_NUM; i++) {
		if (mcu->intr->irq[i] > 0) {
			break;
		}
	}

	ret = 0;
	if (i != AVR_IRQ_NUM) {
		/* Execute ISR */
		/* Clear selected IRQ */
		mcu->intr->irq[i] = 0;

		/* Disable interrupts globally.
		 * NOTE: It isn't applicable for AVR XMEGA cores. */
		if (!mcu->xmega) {
			MSIM_UpdateSREGFlag(mcu, AVR_SREG_GLOB_INT, 0);
		}

		/* Push PC onto the stack */
		MSIM_StackPush(mcu, (unsigned char)(mcu->pc & 0xFF));
		MSIM_StackPush(mcu, (unsigned char)((mcu->pc >> 8) & 0xFF));
		if (mcu->pc_bits > 16) {
			MSIM_StackPush(mcu, (unsigned char)
			               ((mcu->pc >> 16) & 0xFF));
		}

		/* Load interrupt vector to PC */
		mcu->pc = mcu->intr->ivt+(i*2);

		/* Switch MCU to step mode if it's necessary */
		if (mcu->intr->trap_at_isr && mcu->state == AVR_RUNNING) {
			mcu->state = AVR_MSIM_STEP;
		}
	} else {
		/* No IRQ, do nothing */
		ret = 2;
	}
	return ret;
}

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu, enum MSIM_AVRSREGFlag flag,
                         unsigned char set_f)
{
	unsigned char v;

	if (!mcu) {
		fprintf(stderr, "[e]: MCU is null");
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

	if (set_f) {
		*mcu->sreg |= v;
	} else {
		*mcu->sreg &= (unsigned char)~v;
	}
}

unsigned char MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
                                enum MSIM_AVRSREGFlag flag)
{
	unsigned char pos;

	if (!mcu) {
		fprintf(stderr, "[e]: MCU is null");
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

void MSIM_StackPush(struct MSIM_AVR *mcu, unsigned char val)
{
	unsigned int sp;

	sp = (unsigned int)((*mcu->spl) | (*mcu->sph<<8));
	mcu->dm[sp--] = val;
	*mcu->spl = (unsigned char) (sp & 0xFF);
	*mcu->sph = (unsigned char) (sp >> 8);
}

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu)
{
	unsigned int sp;
	unsigned char v;

	sp = (unsigned int)((*mcu->spl) | (*mcu->sph<<8));
	v = mcu->dm[++sp];
	*mcu->spl = (unsigned char)(sp & 0xFF);
	*mcu->sph = (unsigned char)(sp >> 8);
	return v;
}

void MSIM_PrintParts(void)
{
	unsigned int i;

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++) {
		printf("%-10s= %s\n", init_funcs[i].partno,
		       init_funcs[i].name);
	}
}
