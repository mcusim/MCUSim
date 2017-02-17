/*
 * mcusim - Interactive simulator for microcontrollers.
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

/*
 * We would like to include headers specific to the
 * ATMega8A microcontroller.
 */
#define __AVR_ATmega8A__ 1

#include "avr/io.h"
#include "avr/sim/sim.h"

enum init_state m8a_init(struct avr *mcu)
{
	if (!mcu)
		return NULL_MCU;

	mcu->ramstart = RAMSTART;
	mcu->ramend = RAMEND;
	mcu->ramsize = RAMSIZE;

	return INITIALIZED;
}
