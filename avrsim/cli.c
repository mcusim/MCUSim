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
#include "mcusim/avr/sim/simcore.h"

#define MAX_LINESZ		4096
#define MAX_CMDSZ		128

#define INSTRUCTIONS_LISTSZ	10

static unsigned long inst_listsz = INSTRUCTIONS_LISTSZ;

typedef int (*cli_func)(struct MSIM_AVR *mcu, const char *cmd,
			unsigned short cmdl);

/* Supported commands */
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

static cli_func commands[] = {
	/* Start and stop commands */
	/* do not forget about 'quit' command */
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
/* END Supported commands */

int MSIM_InterpretCommands(struct MSIM_AVR *mcu)
{
	char lbuf[MAX_LINESZ], un_cmd;
	unsigned short i, cmd_len;

	un_cmd = 1;
	cmd_len = 0;
	printf("(" SIM_NAME ") ");
	while (fgets(lbuf, sizeof(lbuf), stdin) != NULL) {
		for (i = 0; i < sizeof(lbuf); i++)
			if (lbuf[i] == '\n') {
				lbuf[i] = 0;
				cmd_len = (unsigned short)(i>0?i:0);
				break;
			}

		if (!strncmp("quit", lbuf, cmd_len) ||
		    !strncmp("q", lbuf, cmd_len))
			break;

		for (i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
			if (commands[i](mcu, lbuf, cmd_len)) {
				un_cmd = 0;
				break;
			}
		}
		if (un_cmd)
			/* Unknown command */
			printf("Unknown command\n");
		printf("(" SIM_NAME ") ");
	}

	if (feof(stdin))
		printf("End of stdin reached\n");
	return 0;
}

static int clif_run_simulation(struct MSIM_AVR *mcu, const char *cmd,
			       unsigned short cmdl)
{
	if (cmdl == 0)
		return 0;
	if (strncmp("run", cmd, cmdl) && strncmp("r", cmd, cmdl))
		return 0;

	/* Run infinite simulation which can be stoped by breakpoint, only. */
	return !MSIM_SimulateAVR(mcu, 0, mcu->flashend+1);
}

static int clif_next_inst(struct MSIM_AVR *mcu, const char *cmd,
			  unsigned short cmdl)
{
	unsigned long steps;
	int r;

	if (cmdl == 0)
		return 0;

	steps = 1;
	r = sscanf(cmd, "step %lu", &steps);
	if (r == EOF || r != 1)
		if (strncmp("step", cmd, cmdl) &&
		    strncmp("stepi", cmd, cmdl) &&
		    strncmp("next", cmd, cmdl) &&
		    strncmp("nexti", cmd, cmdl) &&
		    strncmp("s", cmd, cmdl) &&
		    strncmp("si", cmd, cmdl) &&
		    strncmp("n", cmd, cmdl) &&
		    strncmp("ni", cmd, cmdl))
			return 0;

	/* Run fixed steps simulation with the stop address right behind the
	 * end of the program memory address space. */
	return !MSIM_SimulateAVR(mcu, steps == 0 ? 1 : steps, mcu->flashend+1);
}

static int clif_execute_until(struct MSIM_AVR *mcu, const char *cmd,
			      unsigned short cmdl)
{
	unsigned long iaddr;
	int r;

	if (cmdl == 0)
		return 0;

	r = sscanf(cmd, "until 0x%lx", &iaddr);
	if (r == EOF || r != 1) {
		return 0;
	}

	return !MSIM_SimulateAVR(mcu, 0, iaddr);
}

static int clif_where(struct MSIM_AVR *mcu, const char *cmd,
		      unsigned short cmdl)
{
	if (cmdl == 0)
		return 0;
	if (strncmp("where", cmd, cmdl))
		return 0;

	return !MSIM_PrintInstructions(mcu, mcu->pc, mcu->pc, 0);
}

static int clif_list_insts(struct MSIM_AVR *mcu, const char *cmd,
			   unsigned short cmdl)
{
	unsigned long istart_addr, iend_addr;
	int r, args;

	if (cmdl == 0)
		return 0;

	args = 2;
	r = sscanf(cmd, "list 0x%lx,0x%lx", &istart_addr, &iend_addr);
	if (r == EOF || r != args) {
		r = sscanf(cmd, "list 0x%lx", &istart_addr);
		args = 1;
	}

	if (r == EOF || r != args) {
		args = 0;
		if (strncmp("list", cmd, cmdl) &&
		    strncmp("l", cmd, cmdl))
			return 0;
	}

	if (args > 0)
		return !MSIM_PrintInstructions(mcu, istart_addr,
				args == 2 ? iend_addr : mcu->flashend+1,
				args == 2 ? 0 : inst_listsz);
	else
		return !MSIM_PrintInstructions(mcu, mcu->pc, mcu->flashend+1,
				inst_listsz);
}

static int clif_set_listsize(struct MSIM_AVR *mcu, const char *cmd,
			     unsigned short cmdl)
{
	unsigned long lines;
	int r;

	if (cmdl == 0)
		return 0;

	r = sscanf(cmd, "set listsize %lu", &lines);
	if (r == EOF || r != 1) {
		return 0;
	}

	inst_listsz = lines;
	return 1;
}

static int clif_show_listsize(struct MSIM_AVR *mcu, const char *cmd,
			      unsigned short cmdl)
{
	if (cmdl == 0)
		return 0;
	if (strncmp("show listsize", cmd, cmdl))
		return 0;

	printf("Instructions list size: %lu\n", inst_listsz);
	return 1;
}
