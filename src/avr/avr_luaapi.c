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
 * Device models defined as Lua scripts can be used during a scheme
 * simulation in order to substitute important parts (external RAM,
 * displays, etc.) connected to the simulated microcontroller.
 *
 * This file provides basic functions to load, run and unload these models.
 */
#include <stdio.h>
#include <stdint.h>
#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/luaapi.h"
#include "mcusim/log.h"
#ifdef LUA_FOUND
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int flua_MSIM_SetState(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned int s = (unsigned int)lua_tointeger(L, 2);

	mcu->state = (enum MSIM_AVR_State)s;
	return 0; /* Number of results */
}

int flua_MSIM_Freq(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	lua_pushinteger(L, (long)mcu->freq);
	return 1; /* Number of results */
}

int flua_MSIM_Print(lua_State *L)
{
	const char *msg = (const char *)lua_tostring(L, 1);
	MSIM_LOG_INFO(msg);
	return 0;
}

int flua_AVR_RegBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);

	/* Model's trying to read something else. */
	if (reg >= mcu->regs) {
		fprintf(stderr, "[e] Model %s is reading a bit of unknown "
		        "register: %u\n", "N/A", reg);
		lua_pushnil(L);
		return 1;
	}
	/* Model's trying to read unknown bit. */
	if (bit >= 8) {
		fprintf(stderr, "[e] Model %s is reading unknown "
		        "bit of a register: %u\n", "N/A", bit);
		lua_pushnil(L);
		return 1;
	}
	lua_pushboolean(L, (mcu->dm[reg]&(1<<bit))>>bit);
	return 1;
}

int flua_AVR_IOBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);

	/* Model's trying to read something else.
	 * NOTE: We may have no I/O registers at all! */
	if (mcu->io_regs == 0 ||
	                io_reg >= (mcu->sfr_off+mcu->io_regs) ||
	                io_reg < mcu->sfr_off) {
		fprintf(stderr, "[e] Model %s is reading a bit of unknown I/O"
		        " register: %u\n", "N/A", io_reg);
		lua_pushnil(L);
		return 1;
	}
	/* Model's trying to read unknown bit. */
	if (bit >= 8) {
		fprintf(stderr, "[e] Model %s is reading unknown "
		        "bit: %u\n", "N/A", bit);
		lua_pushnil(L);
		return 1;
	}
	lua_pushboolean(L, (mcu->dm[io_reg]&(1<<bit))>>bit);
	return 1;
}

int flua_AVR_ReadReg(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);

	/* Model's trying to read something else. */
	if (reg >= mcu->regs) {
		fprintf(stderr, "[e] Model %s is reading unknown "
		        "register: %u\n", "N/A", reg);
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, mcu->dm[reg]);
	return 1;
}

int flua_AVR_ReadIO(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);

	/* Model's trying to read something else.
	 * NOTE: We may have no I/O registers at all! */
	if (mcu->io_regs == 0 ||
	                io_reg >= (mcu->sfr_off+mcu->io_regs) ||
	                io_reg < mcu->sfr_off) {
		fprintf(stderr, "[e] Model %s is reading unknown I/O "
		        "register: %u\n", "N/A", io_reg);
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, mcu->dm[io_reg]);
	return 1;
}

int flua_AVR_SetRegBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);
	unsigned char val = (unsigned char)lua_tointeger(L, 4);

	/* Model's trying to write something else. */
	if (reg >= mcu->regs) {
		fprintf(stderr, "[e] Model %s is writing a bit of unknown "
		        "register: %u\n", "N/A", reg);
		return 0;
	}
	/* Model's trying to write unknown bit. */
	if (bit >= 8) {
		fprintf(stderr, "[e] Model %s is writing unknown "
		        "bit of a register: %u\n", "N/A", bit);
		return 0;
	}
	if (val&1) {
		mcu->dm[reg] |= (unsigned char)(1<<bit);
	} else {
		mcu->dm[reg] &= (unsigned char)(~(1<<bit));
	}
	return 0;
}

int flua_AVR_SetIOBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);
	unsigned char val = (unsigned char)lua_tointeger(L, 4);

	/* Model's trying to write something else.
	 * NOTE: We may have no I/O registers at all! */
	if (mcu->io_regs == 0 ||
	                io_reg >= (mcu->sfr_off+mcu->io_regs) ||
	                io_reg < mcu->sfr_off) {
		fprintf(stderr, "[e] Model %s is writing a bit of unknown I/O"
		        "register: %u\n", "N/A", io_reg);
		return 0;
	}
	/* Model's trying to write unknown bit. */
	if (bit >= 8) {
		fprintf(stderr, "[e] Model %s is writing unknown "
		        "bit: %u\n", "N/A", bit);
		return 0;
	}
	if (val&1) {
		mcu->dm[io_reg] |= (unsigned char)(1<<bit);
	} else {
		mcu->dm[io_reg] &= (unsigned char)(~(1<<bit));
	}
	return 0;
}

int flua_AVR_WriteReg(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char val = (unsigned char)lua_tointeger(L, 3);

	/* Model's trying to write something else. */
	if (reg >= mcu->regs) {
		fprintf(stderr, "[e] Model %s is writing unknown "
		        "register: %u\n", "N/A", reg);
		return 0;
	}
	mcu->dm[reg] = val;
	return 0;
}

int flua_AVR_WriteIO(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char val = (unsigned char)lua_tointeger(L, 3);

	/* Model's trying to write something else.
	 * NOTE: We may have no I/O registers at all! */
	if (mcu->io_regs == 0 ||
	                io_reg >= (mcu->sfr_off+mcu->io_regs) ||
	                io_reg < mcu->sfr_off) {
		fprintf(stderr, "[e] Model %s is writing unknown I/O"
		        "register: %u\n", "N/A", io_reg);
		return 0;
	}
	mcu->dm[io_reg] = val;
	return 0;
}

#endif /* LUA_FOUND */
