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
#ifndef MSIM_CLI_H_
#define MSIM_CLI_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/avr/sim/sim.h"

/* Structure to describe a memory operation requested by user. */
struct MSIM_MemOp {
	char memtype[16];		/* Type of MCU memory */
	char operation;			/* Memory operation */
	char operand[4096];		/* Path to file, value, etc. */
	char format;			/* Optional, value format */
};

int MSIM_InterpretCommands(struct MSIM_AVR *mcu);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_CLI_H_ */
