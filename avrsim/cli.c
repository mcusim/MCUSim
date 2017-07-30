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
#include "mcusim/avr/sim/sim.h"

#define MAX_LINESZ		4096
#define MAX_CMDSZ		128

typedef int (*cli_func)(struct MSIM_AVR *mcu, const char *cmd,
			unsigned short cmdl);

/* Supported commands */
static int clif_quit(struct MSIM_AVR *mcu, const char *cmd,
		      unsigned short cmdl);
static int clif_kill_simulation(struct MSIM_AVR *mcu, const char *cmd,
				 unsigned short cmdl);
static int clif_run_simulation(struct MSIM_AVR *mcu, const char *cmd,
				unsigned short cmdl);

static int clif_next_inst(struct MSIM_AVR *mcu, const char *cmd,
			   unsigned short cmdl);
static int clif_execute_until(struct MSIM_AVR *mcu, const char *cmd,
			       unsigned short cmdl);
static int clif_where(struct MSIM_AVR *mcu, const char *cmd,
		       unsigned short cmdl);

static int clif_list_insts(struct MSIM_AVR *mcu, const char *cmd,
			    unsigned short cmdl);
static int clif_set_listsize(struct MSIM_AVR *mcu, const char *cmd,
			      unsigned short cmdl);
static int clif_show_listsize(struct MSIM_AVR *mcu, const char *cmd,
			       unsigned short cmdl);
/* END Supported commands */

static cli_func commands[] = {
	/* Start and stop commands */
	clif_quit,
	clif_kill_simulation,
	clif_run_simulation,
	/* END Start and stop commands */

	/* Line execution commands */
	clif_next_inst,
	clif_execute_until,
	clif_where,
	/* END Line execution commands */

	/* Disassembled code commands */
	clif_list_insts,
	clif_set_listsize,
	clif_show_listsize
	/* END Disassembled code commands */
};

int MSIM_InterpretCommands(struct MSIM_AVR *mcu)
{
	char lbuf[MAX_LINESZ], quit_f;
	unsigned short i, cmd_len;

	quit_f = 0;
	printf("(" SIM_NAME ") ");
	while (fgets(lbuf, sizeof(lbuf), stdin) != NULL) {
		for (i = 0; i < sizeof(lbuf); i++)
			if (lbuf[i] == '\n') {
				lbuf[i] = 0;
				cmd_len = i > 0 ? i-1 : 0;
				break;
			}

		for (i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
			if (commands[i](mcu, lbuf, cmd_len)) {
				break;
			} else if (!strcmp(lbuf, "quit") ||
				   !strcmp(lbuf, "q")) {
				quit_f++;
				break;
			}
		}
		if (quit_f)
			break;
		printf("(" SIM_NAME ") ");
	}

	if (feof(stdin))
		printf("End of stdin reached\n");
	return 0;
}
