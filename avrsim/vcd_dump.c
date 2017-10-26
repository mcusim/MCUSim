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
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/vcd_dump.h"

static void print_reg(char *buf, unsigned int len, unsigned char r);

FILE *MSIM_VCDOpenDump(void *vmcu, const char *dumpname)
{
	FILE *f;
	time_t timer;
	char buf[32];
	struct tm *tm_info;
	unsigned int i, regs;
	struct MSIM_AVR *mcu = (struct MSIM_AVR *)vmcu;
	struct MSIM_VCDRegister *reg;

	regs = sizeof mcu->vcd_regsn/sizeof mcu->vcd_regsn[0];

	f = fopen(dumpname, "w");
	if (!f)
		return NULL;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%S%z", tm_info);

	/* Printing VCD header */
	fprintf(f, "$date %s $end\n", buf);
	fprintf(f, "$version AVRSim %s $end\n", MSIM_VERSION);
	fprintf(f, "$comment It is a dump of simulated %s $end\n", mcu->name);
	fprintf(f, "$timescale 62500 ps $end\n");
	fprintf(f, "$scope module %s $end\n", mcu->name);
	for (i = 0; i < regs; i++) {
		if (mcu->vcd_regsn[i] < 0)
			break;

		reg = &mcu->vcd_regs[mcu->vcd_regsn[i]];
		fprintf(f, "$var reg 8 %s %s $end\n", reg->name, reg->name);
	}
	fprintf(f, "$upscope $end\n");
	fprintf(f, "$enddefinitions $end\n");

	/* Dumping initial register values */
	fprintf(f, "$dumpvars\n");
	for (i = 0; i < regs; i++) {
		if (mcu->vcd_regsn[i] < 0)
			break;

		reg = &mcu->vcd_regs[mcu->vcd_regsn[i]];
		print_reg(buf, sizeof buf, *reg->addr);
		fprintf(f, "b%s %s\n", buf, reg->name);
	}
	fprintf(f, "$end\n");

	return f;
}

void MSIM_VCDDumpFrame(FILE *f, void *vmcu, unsigned long tick)
{
	unsigned int i, regs;
	char buf[32], print_tick;
	struct MSIM_AVR *mcu = (struct MSIM_AVR *)vmcu;
	struct MSIM_VCDRegister *reg;

	regs = sizeof mcu->vcd_regsn/sizeof mcu->vcd_regsn[0];
	print_tick = 1;

	for (i = 0; i < regs; i++) {
		/* First N registers to be dumped only */
		if (mcu->vcd_regsn[i] < 0)
			break;

		reg = &mcu->vcd_regs[mcu->vcd_regsn[i]];
		/* Hasn't it been changed? */
		if (*reg->addr == reg->oldv)
			continue;

		if (print_tick) {
			print_tick = 0;
			fprintf(f, "#%lu\n", tick);
		}
		reg->oldv = *reg->addr;
		print_reg(buf, sizeof buf, *reg->addr);
		fprintf(f, "b%s %s\n", buf, reg->name);
	}
}

static void print_reg(char *buf, unsigned int len, unsigned char r)
{
	unsigned int i, j = 0;
	size_t rbits = (sizeof r)*8;

	if (len < rbits) {
		buf[0] = 0;
		return;
	}
	for (i = 0; i < rbits; i++) {
		if ((r >> (rbits-1-i)) & 1)
			buf[j++] = '1';
		else
			buf[j++] = '0';
	}
	buf[j] = 0;
}
