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
#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"

#define CLI_OPTIONS		":hm:p:"

#define PROGRAM_MEMORY		8192
#define DATA_MEMORY		1120

static struct MSIM_AVRBootloader bootloader;
static struct MSIM_AVR mcu;

static uint8_t prog_mem[PROGRAM_MEMORY];
static uint8_t data_mem[DATA_MEMORY];

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind, optopt;
	int c;
	char *mcu_model;
	char *prog_path;
	uint8_t errflag = 1;
	FILE *fp;

	while ((c = getopt(argc, argv, CLI_OPTIONS)) != -1) {
		switch (c) {
		case 'm':
			mcu_model = optarg;
			errflag = 0;
			break;
		case 'p':
			prog_path = optarg;
			errflag = 0;
			break;
		case 'h':
			break;
		case ':':		/* missing operand */
			fprintf(stderr, "Option -%c required an operand\n",
					optopt);
			break;
		case '?':		/* unrecognised option */
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			break;
		}
	}
	if (errflag) {
		fprintf(stderr, "Usage: avrsim -m <AVR_MODEL> "
				"-p <PATH_TO_HEX>\n");
		return -2;
	}

	mcu.boot_loader = &bootloader;
	if (MSIM_InitAVR(&mcu, mcu_model,
			 prog_mem, sizeof prog_mem / sizeof prog_mem[0],
			 data_mem, sizeof data_mem / sizeof data_mem[0])) {
		fprintf(stderr, "AVR %s wasn't initialized successfully!\n",
				mcu_model);
		return -1;
	}

	fp = fopen(prog_path, "r");
	if (MSIM_LoadProgmem(&mcu, fp)) {
		fprintf(stderr, "Program memory cannot be loaded from "
				"file: %s!\n", prog_path);
		return -1;
	}
	fclose(fp);

	MSIM_SimulateAVR(&mcu);

	return 0;
}
