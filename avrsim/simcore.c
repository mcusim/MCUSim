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

#define REG_ZH			0x1F
#define REG_ZL			0x1E

typedef int (*init_func)(struct MSIM_AVR *mcu,
			 unsigned char *pm, unsigned long pm_size,
			 unsigned char *dm, unsigned long dm_size);

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
	char inf_loop, stop;

	stop = 0;
	inf_loop = !steps ? 1 : 0;
	steps = !steps ? 1 : steps;
	while (steps > 0) {
		if (addr >= mcu->flashstart && addr <= mcu->flashend &&
		    addr == mcu->pc)
			stop = 1;

		if (MSIM_StepAVR(mcu))
			return 1;

		MSIM_TickLuaPeripherals(mcu);
		if (mcu->tick_8timers != NULL)
			mcu->tick_8timers(mcu);

		if (stop)
			break;
		if (!inf_loop)
			steps--;
	}
	return 0;
}

int MSIM_PrintInstructions(struct MSIM_AVR *mcu, unsigned long start_addr,
			   unsigned long end_addr, unsigned long steps)
{
	unsigned short inst, msb, lsb;
	unsigned long loc_pc;

	loc_pc = mcu->pc;
	if (start_addr > mcu->flashend || start_addr < mcu->flashstart)
		return 0;
	if (end_addr > mcu->flashend || end_addr < mcu->flashstart)
		end_addr = start_addr + steps;
	if (end_addr < start_addr)
		return 0;

	while (loc_pc <= end_addr) {
		lsb = (unsigned short) mcu->pm[loc_pc];
		msb = (unsigned short) mcu->pm[loc_pc+1];
		inst = (unsigned short) (lsb | (msb << 8));

		printf("%lu:\t%lx: %x %x\n", mcu->id, loc_pc, lsb, msb);

		loc_pc += MSIM_Is32(inst) ? 4 : 2;
	}
	return 0;
}

int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 unsigned char *pm, unsigned long pm_size,
		 unsigned char *dm, unsigned long dm_size,
		 FILE *fp)
{
	unsigned int i;
	char mcu_found = 0;

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++)
		if (!strcmp(init_funcs[i].partno, mcu_name)) {
			if (init_funcs[i].f(mcu, pm, pm_size, dm, dm_size))
				return -1;
			else
				mcu_found = 1;
		}

	if (!mcu_found) {
		fprintf(stderr, "Microcontroller AVR %s is not supported!\n",
				mcu_name);
		return -1;
	}

	if (load_progmem(mcu, fp)) {
		fprintf(stderr, "Program memory cannot be loaded from a "
				"file!\n");
		return -1;
	}
	return 0;
}

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp)
{
	IHexRecord rec, mem_rec;

	if (!fp) {
		fprintf(stderr, "Cannot read from the filestream\n");
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
			printf("Checksum is not correct: 0x%X (memory) != "
			       "0x%X (file)\nFile record:\n",
			       mem_rec.checksum, rec.checksum);
			Print_IHexRecord(&rec);
			printf("Memory record:\n");
			Print_IHexRecord(&mem_rec);
			return -1;
		}
	}
	return 0;
}

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu, enum MSIM_AVRSREGFlag flag,
			 unsigned char set_f)
{
	unsigned char v;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
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
		fprintf(stderr, "MCU is null");
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

	printf("Valid parts are:\n");
	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++)
		printf("%-10s= %s\n", init_funcs[i].partno, init_funcs[i].name);
}
