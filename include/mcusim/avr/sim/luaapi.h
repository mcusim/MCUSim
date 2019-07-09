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

/*
 * MCUSim API for models written in Lua, i.e. functions to be called from
 * the Lua scripts.
 */
#ifndef MSIM_AVR_LUAAPI_H_
#define MSIM_AVR_LUAAPI_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "mcusim/mcusim.h"

/* Reads bit of a general purpose AVR register (from register file).
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short reg;
 * 	unsigned char bit;
 * Returns:
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRRegBit(lua_State *L);

/* Reads bit of an I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short io_reg;
 * 	unsigned char bit;
 * Returns:
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRIOBit(lua_State *L);

/* Reads value of a general purpose AVR register (from register file).
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short reg;
 * Returns:
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRReadReg(lua_State *L);

/* Reads value of an I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short io_reg;
 * Returns:
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRReadIO(lua_State *L);

/* Reads value of a 16-bit I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	uint16_t io_high;
 * 	uint16_t io_low;
 * Returns:
 * 	uint16_t val;
 */
int MSIM_LUAF_AVRReadIO16(lua_State *L);

/* Writes bit of a general purpose AVR register (from register file).
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short reg;
 * 	unsigned char bit;
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRSetRegBit(lua_State *L);

/* Writes bit of an I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short io_reg;
 * 	unsigned char bit;
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRSetIOBit(lua_State *L);

/* Writes value to general purpose AVR register (from register file).
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short reg;
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRWriteReg(lua_State *L);

/* Writes value to I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned short reg;
 * 	unsigned char val;
 */
int MSIM_LUAF_AVRWriteIO(lua_State *L);

/* Writes value to 16-bit I/O AVR register. I/O registers are addressed as
 * data space, i.e. offset of the special function registers should be added.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	uint16_t io_high;
 * 	uint16_t io_low;
 * 	uint16_t val;
 */
int MSIM_LUAF_AVRWriteIO16(lua_State *L);

/* Set state of a simulated AVR microcontroller. This function is helpful to
 * terminate simulation if it's necessary (test failure, etc.).
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned int state;
 */
int MSIM_LUAF_SetState(lua_State *L);

/* Function to retrieve frequency of the simulated microcontroller. It can be
 * helpful while configuring model.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * Returns:
 * 	unsigned long freq;		MCU frequency, in Hz
 */
int MSIM_LUAF_Freq(lua_State *L);

/* Function to print anything from a Lua model. It is supposed to be a
 * replacement of a Lua print() function and based on the MCUSim logging
 * mechanism.
 *
 * Lua parameters:
 * 	const char *message;
 */
int MSIM_LUAF_Print(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_LUAAPI_H_ */
