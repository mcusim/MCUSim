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

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/cli.h"
#include "mcusim/getopt.h"

#define CLI_OPTIONS		"?p:U:"

#define PROGRAM_MEMORY		262144		/* 256 KiB */
#define DATA_MEMORY		65536		/* 64 KiB */
#define PM_PAGESZ		1024		/* 1 KiB, PM page size */

static struct MSIM_AVRBootloader bootloader;
static struct MSIM_AVR mcu;
static unsigned char pm[PROGRAM_MEMORY];
static unsigned char pmp[PM_PAGESZ];
static unsigned char dm[DATA_MEMORY];

static void print_usage(void);

int main(int argc, char *argv[])
{
	extern char *optarg;
	FILE *fp;
	int c, r;
	char *partno, *mopt;
	char mtype[21], mop, mfn[4096];

	partno = mopt = NULL;
	while ((c = getopt(argc, argv, CLI_OPTIONS)) != -1) {
		switch (c) {
		case 'p':		/* Required. AVR device. */
			partno = optarg;
			break;
		case 'U':		/* Required. Memory operation. */
			mopt = optarg;
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
	for (c = 0; mopt[c] != 0; c++)
		if (mopt[c] == ':')
			mopt[c] = ' ';
	r = sscanf(mopt, "%20s %1c %4095s", &mtype[0], &mop, &mfn[0]);
	if (r == EOF || r != 3) {
		print_usage();
		return 0;
	}

	fp = fopen(&mfn[0], "r");
	mcu.bls = &bootloader;
	mcu.pmp = pmp;
	if (MSIM_InitAVR(&mcu, partno,
			 pm, PROGRAM_MEMORY, dm, DATA_MEMORY, fp)) {
		fprintf(stderr, "AVR %s cannot be initialized!\n", partno);
		return -1;
	}
	fclose(fp);

	MSIM_InterpretCommands(&mcu);

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
