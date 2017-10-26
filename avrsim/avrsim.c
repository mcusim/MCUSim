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
#include <string.h>

#include "mcusim/cli.h"
#include "mcusim/getopt.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/peripheral_lua.h"
#include "mcusim/avr/sim/gdb_rsp.h"

#define CLI_OPTIONS		"?p:U:r:d:"

#define PMSZ			262144		/* 256 KiB */
#define DMSZ			65536		/* 64 KiB */
#define PM_PAGESZ		1024		/* 1 KiB, PM page size */

#define GDB_RSP_PORT		12750

static struct MSIM_AVRBootloader bls;
static struct MSIM_AVR mcu;

/* Statically allocated memory for MCU */
static unsigned char pm[PMSZ];			/* Program memory */
static unsigned char pmp[PM_PAGESZ];		/* Page buffer (PM) */
static unsigned char dm[DMSZ];			/* Data memory */
static unsigned char mpm[PMSZ];			/* Match points memory */

/* VCD dump options */
static char vcd_regs[VCD_REGS_MAX][16];		/* MCU registers to dump */
static unsigned int vcd_rn;			/* Number of registers */
static char print_regs;				/* Print available registers
						   which can be dumped */

static void print_usage(void);
static void parse_dump(char *cmd);

int main(int argc, char *argv[])
{
	extern char *optarg;
	FILE *fp;
	int c, r;
	char mtype[21], mop, mfn[4096];
	char *partno, *mopt, *luap;
	unsigned int i, j, regs, dregs;

	partno = mopt = luap = NULL;
	vcd_rn = 0;
	print_regs = 0;

	while ((c = getopt(argc, argv, CLI_OPTIONS)) != -1) {
		switch (c) {
		case 'p':		/* Required. AVR device. */
			partno = optarg;
			break;
		case 'U':		/* Required. Memory operation. */
			mopt = optarg;
			break;
		case 'r':		/* Optional. Lua peripherals file. */
			luap = optarg;
			break;
		case 'd':		/* Dump options */
			parse_dump(optarg);
			break;
		case '?':		/* Print usage */
			print_usage();
			return 0;
		}
	}

	if (partno != NULL && (!strcmp(partno, "?"))) {
		MSIM_PrintParts();
		return 0;
	}
	if (partno == NULL || mopt == NULL) {
		print_usage();
		return 0;
	}

	/* Parse memory operation */
	for (c = 0; mopt[c] != 0; c++)
		if (mopt[c] == ':')
			mopt[c] = ' ';
	r = sscanf(mopt, "%20s %1c %4095s", &mtype[0], &mop, &mfn[0]);
	if (r == EOF || r != 3) {
		print_usage();
		return 0;
	}

	/* Load Lua peripherals if it is required */
	if (luap != NULL && MSIM_LoadLuaPeripherals(luap))
		return -1;

	/* Initialize MCU as one of AVR models */
	fp = fopen(&mfn[0], "r");
	mcu.bls = &bls;
	mcu.pmp = pmp;
	if (MSIM_InitAVR(&mcu, partno, pm, PMSZ, dm, DMSZ, mpm, fp)) {
		fprintf(stderr, "AVR %s cannot be initialized!\n", partno);
		return -1;
	}
	fclose(fp);

	/* Print available registers only? */
	regs = sizeof mcu.vcd_regs/sizeof mcu.vcd_regs[0];
	if (print_regs) {
		for (i = 0; i < regs; i++) {
			/* Print first N registers only */
			if (mcu.vcd_regs[i].name[0] == 0)
				break;
			printf("%s (0x%2lX)\n", mcu.vcd_regs[i].name,
						mcu.vcd_regs[i].off);
		}
		return 0;
	}

	/* Select registers to be dumped */
	dregs = 0;
	for (i = 0; i < vcd_rn; i++) {
		for (j = 0; j < regs; j++) {
			if (!strcmp(vcd_regs[i], mcu.vcd_regs[j].name)) {
				mcu.vcd_regsn[dregs++] = (short)j;
				break;
			}
		}
	}

	/* Prepare and run AVR simulation */
	MSIM_RSPInit(&mcu, GDB_RSP_PORT);
	MSIM_SimulateAVR(&mcu, 0, mcu.flashend+1);
	MSIM_CleanLuaPeripherals();
	MSIM_RSPClose();
	return 0;
}

static void print_usage(void)
{
	printf("Usage: avrsim [options]\n"
		"Options:\n"
		"  -p <partno>                Required. Specify AVR device.\n"
		"  -U <memtype>:r|w|v:<filename>\n"
		"                             Required. Memory operation "
		"specification.\n"
		"  -?                         Display this usage.\n\n"
		"avrsim version %s, <http://www.mcusim.org>\n", MSIM_VERSION);
}

static void parse_dump(char *cmd)
{
	int ret, c;
	char *p;

	if (cmd == NULL)
		return;
	if (!strcmp(cmd, "ump-regs=?")) {
		print_regs = 1;
		return;
	} else if (strstr(cmd, "ump-regs=") != NULL) {
		for (c = 0; cmd[c] != 0; c++)
			if (cmd[c] == ',')
				cmd[c] = ' ';

		p = cmd+9;
		while (1) {
			ret = sscanf(p, "%s", &vcd_regs[vcd_rn][0]);
			if (ret == EOF)
				break;
			if (ret == 1) {
				vcd_rn++;
				p = strstr(p, " ");
				if (p == NULL)
					break;
				p++;
				continue;
			}
			fprintf(stderr, "Failed to parse list of registers "
					"to dump: %s\n", p);
			break;
		}
		return;
	}
}
