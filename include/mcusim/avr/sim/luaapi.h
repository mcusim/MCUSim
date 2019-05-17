/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file contains MCUSim API for models written in Lua, i.e. it declares
 * C functions of the simulator which can be called from the models.
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
