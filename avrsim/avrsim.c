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

int main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind, optopt;
	int c;
	char *mcu_model;
	char *prog_path;
	uint8_t errflag = 0;

	while ((c = getopt(argc, argv, CLI_OPTIONS)) != -1) {
		switch (c) {
		case 'm':
			mcu_model = optarg;
			break;
		case 'p':
			prog_path = optarg;
			break;
		case 'h':
			errflag++;
			break;
		case ':':		/* missing operand */
			fprintf(stderr, "Option -%c required an operand\n",
					optopt);
			errflag++;
			break;
		case '?':		/* unrecognised option */
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			errflag++;
			break;
		}
	}
	if (errflag) {
		fprintf(stderr, "Usage: -m <AVR_MODEL> -p <PATH_TO_HEX>\n");
		return -2;
	}
	return 0;
}
