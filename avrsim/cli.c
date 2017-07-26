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

/* Supported commands */
static void clif_next_inst(struct MSIM_AVR *mcu);
/* END Supported commands */

static int is_prefix(const char *str, const char *pre);

typedef void (*cmd_func)(struct MSIM_AVR *mcu);

struct cli_command {
	char cmd[MAX_CMDSZ];
	cmd_func func;
};

static struct cli_command commands[] = {
	{ "quit", NULL }, /* can be executed directly */
	{ "next instruction", clif_next_inst },
	{ "ni", clif_next_inst }
};

int MSIM_InterpretCommands(struct MSIM_AVR *mcu)
{
	char lbuf[MAX_LINESZ];
	uint32_t i;

	printf("(" SIM_NAME ") ");

	while (fgets(lbuf, sizeof(lbuf), stdin) != NULL) {
		for (i = 0; i < sizeof(lbuf); i++)
			if (lbuf[i] == '\n')
				lbuf[i] = 0;

		for (i = 0; i < sizeof(commands); i++) {
			if (commands[i].func != NULL &&
			    is_prefix(lbuf, commands[i].cmd)) {
				commands[i].func(mcu);
			} else if (strcmp(lbuf, "quit") == 0) {
				goto end_cmd_loop;
			}
		}
		printf("(" SIM_NAME ") ");
	}

end_cmd_loop:
	if (feof(stdin))
		printf("End of stdin reached\n");
	return 0;
}

static void clif_next_inst(struct MSIM_AVR *mcu)
{
	printf("Command: next instruction\n");
}

static int is_prefix(const char *str, const char *pre)
{
	return strncmp(pre, str, strlen(pre)) == 0 ? 1 : 0;
}
