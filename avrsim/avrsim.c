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

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/cli.h"
#include "mcusim/getopt.h"

#define CLI_OPTIONS		":hm:p:v"

#define PROGRAM_MEMORY		262144		/* 256 KiB */
#define DATA_MEMORY		65536		/* 64 KiB */
#define PM_PAGESZ		1024		/* 1 KiB, PM page size */

static struct MSIM_AVRBootloader bootloader;
static struct MSIM_AVR mcu;

static unsigned char pm[PROGRAM_MEMORY];
static unsigned char pmp[PM_PAGESZ];
static unsigned char dm[DATA_MEMORY];

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optopt;
	int c;
	char *mm;
	char *prog_path;
	uint8_t errflag = 1, ver = 0;
	FILE *fp;

	while ((c = getopt(argc, argv, CLI_OPTIONS)) != -1) {
		switch (c) {
		case 'm':
			mm = optarg;
			errflag = 0;
			break;
		case 'p':
			prog_path = optarg;
			errflag = 0;
			break;
		case 'h':
			break;
		case 'v':
			errflag = 0;
			ver = 1;
			break;
		case ':':		/* missing operand */
			fprintf(stderr, "Option -%c required an operand\n",
					optopt);
			break;
		case '?':		/* unrecognised option */
			fprintf(stderr, "Unknown option: -%c\n", optopt);
			break;
		}
	}
	if (errflag) {
		fprintf(stderr,
			"usage: avrsim [-v] [-h] [-o <option>] "
			"-m <model> -p <hexfile>\n\nThese options are used "
			"to configure the simulator:\n\n"
			"    use-stdout       Print all status messages to "
			"stdout (default)\n"
			"    use-ipc-queues   Use IPC message queues to "
			"report about status of the simulator and accept "
			"control messages\n"
			"    print-io-regs    Print I/O registers which "
			"values have been changed\n"
			"    print-opcodes    Print next AVR instruction "
			"which will be executed\n\n");
		return -2;
	} else if (ver) {
		printf("avrsim version %d.%d.%d%s\n",
			MSIM_VERSION_MAJOR, MSIM_VERSION_MINOR,
			MSIM_VERSION_PATCH, MSIM_VERSION_META);
		return 0;
	}

	fp = fopen(prog_path, "r");
	mcu.bls = &bootloader;
	mcu.pmp = pmp;
	if (MSIM_InitAVR(&mcu, mm, pm, PROGRAM_MEMORY, dm, DATA_MEMORY, fp)) {
		fprintf(stderr, "AVR %s cannot be initialized!\n", mm);
		return -1;
	}
	fclose(fp);

	MSIM_InterpretCommands(&mcu);

	return 0;
}
