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
#ifndef MSIM_AVR_PERIPHERAL_LUA_H_
#define MSIM_AVR_PERIPHERAL_LUA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/avr/sim/sim.h"

#define LUA_PERIPHERALS		256

/*
 * Configuration functions to work with peripherals written in Lua.
 */

/* Load peripherals written in Lua from a given list file. */
int MSIM_LoadLuaPeripherals(const char *);

/* Close previously created Lua states. */
void MSIM_CleanLuaPeripherals(void);

void MSIM_TickLuaPeripherals(struct MSIM_AVR *mcu);
/*
 * END Configuration functions to work with peripherals written in Lua.
 */

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_PERIPHERAL_LUA_H_ */
