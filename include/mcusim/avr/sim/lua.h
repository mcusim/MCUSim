/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_LUA_H_
#define MSIM_AVR_LUA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/avr/sim/sim.h"

/* Maximum number of device models defined as Lua scripts to be loaded
 * during a simulation. */
#define MSIM_AVR_LUAMODELS			256

/* Load peripherals written in Lua from a given list file. */
int MSIM_AVR_LUALoadModel(struct MSIM_AVR *mcu, char *model);
/* Close previously created Lua states. */
void MSIM_AVR_LUACleanModels(void);
/* Call a "tick" function of the models during each cycle of simulation. */
void MSIM_AVR_LUATickModels(struct MSIM_AVR *mcu);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_LUA_H_ */
