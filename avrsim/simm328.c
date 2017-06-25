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
#include <string.h>

/* We would like to include headers specific to the ATMega328P
 * microcontroller. */
#define _SFR_ASM_COMPAT 1
#define __AVR_ATmega328__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"

#define FLASHSTART		0x0000
#define RAMSIZE			2048
#define E2START			0x0000
#define E2SIZE			1024

int MSIM_M328Init(struct MSIM_AVR *mcu,
		  uint8_t *pm, uint32_t pm_size,
		  uint8_t *dm, uint32_t dm_size)
{
	int r;

	r = MSIM_M328PInit(mcu, pm, pm_size, dm, dm_size);
	strcpy(mcu->name, "atmega328");
	mcu->signature[0] = SIGNATURE_0;
	mcu->signature[1] = SIGNATURE_1;
	mcu->signature[2] = SIGNATURE_2;
	return r;
}
