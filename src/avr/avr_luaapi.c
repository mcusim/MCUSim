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

/* Implementation of the MCUSim API for device models written in Lua. */
#include <stdio.h>
#include <stdint.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/luaapi.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int
MSIM_LUAF_SetState(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned int s = (unsigned int)lua_tointeger(L, 2);

	mcu->state = (enum MSIM_AVR_State)s;
	return 0; /* Number of results */
}

int
MSIM_LUAF_Freq(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	lua_pushinteger(L, (long)mcu->freq);
	return 1; /* Number of results */
}

int
MSIM_LUAF_Print(lua_State *L)
{
	const char *msg = (const char *)lua_tostring(L, 1);
	MSIM_LOG_INFO(msg);
	return 0;
}

int
MSIM_LUAF_AVRRegBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);

	/* Model is trying to read something else. */
	if (reg >= mcu->regs_num) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is reading "
		         "bit of unknown register: %u", reg);
		MSIM_LOG_ERROR(mcu->log);
		lua_pushnil(L);
		return 1;
	}
	/* Model is trying to read unknown bit. */
	if (bit >= 8) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is reading "
		         "unknown bit of a register: %u", bit);
		MSIM_LOG_ERROR(mcu->log);
		lua_pushnil(L);
		return 1;
	}
	lua_pushboolean(L, (mcu->dm[reg]&(1<<bit))>>bit);
	return 1;
}

int
MSIM_LUAF_AVRIOBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);

	/* Model is trying to read something else. */
	if ((mcu->ioregs_num == 0) ||
	                (io_reg >= (mcu->sfr_off+mcu->ioregs_num)) ||
	                (io_reg < mcu->sfr_off)) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is reading "
		         "bit of unknown I/O register: %u", io_reg);
		MSIM_LOG_ERROR(mcu->log);
		lua_pushnil(L);
		return 1;
	}
	/* Model is trying to read unknown bit. */
	if (bit >= 8) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is reading "
		         "unknown bit: %u", bit);
		MSIM_LOG_ERROR(mcu->log);
		lua_pushnil(L);
		return 1;
	}
	lua_pushboolean(L, (mcu->dm[io_reg]&(1<<bit))>>bit);
	return 1;
}

int
MSIM_LUAF_AVRReadReg(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);

	/* Model is trying to read something else. */
	if (reg >= mcu->regs_num) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is reading "
		         "unknown register: %u", reg);
		MSIM_LOG_ERROR(mcu->log);
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, mcu->dm[reg]);
	return 1;
}

int
MSIM_LUAF_AVRReadIO(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);

	/* Model is trying to read something else. */
	if ((mcu->ioregs_num == 0) ||
	                (io_reg >= (mcu->sfr_off+mcu->ioregs_num)) ||
	                (io_reg < mcu->sfr_off)) {
		snprintf(LOG, LOGSZ, "lua model is reading unknown I/O "
		         "register: %u", io_reg);
		MSIM_LOG_ERROR(LOG);
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, mcu->dm[io_reg]);
	return 1;
}

int
MSIM_LUAF_AVRReadIO16(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	uint16_t io_high = (uint16_t)lua_tointeger(L, 2);
	uint16_t io_low = (uint16_t)lua_tointeger(L, 3);
	const uint32_t l = mcu->sfr_off;
	const uint32_t h = l + mcu->ioregs_num;

	/* Model is trying to read something else. */
	if ((l == h) || (io_high >= h) || (io_high < l) ||
	                (io_low >= h) || (io_low < l)) {
		snprintf(LOG, LOGSZ, "lua model is reading unknown 16-bit I/O "
		         "register: %u:%u", io_high, io_low);
		MSIM_LOG_ERROR(LOG);
		lua_pushnil(L);
		return 1;
	}

	lua_pushinteger(L, (uint16_t)(mcu->dm[io_high]<<8)|(mcu->dm[io_low]));
	return 1;
}

int
MSIM_LUAF_AVRSetRegBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);
	unsigned char val = (unsigned char)lua_tointeger(L, 4);

	/* Model is trying to write something else. */
	if (reg >= mcu->regs_num) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "bit of unknown register: %u", reg);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	/* Model is trying to write unknown bit. */
	if (bit >= 8) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "unknown bit of a register: %u", bit);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	if (val&1) {
		mcu->dm[reg] |= (unsigned char)(1<<bit);
	} else {
		mcu->dm[reg] &= (unsigned char)(~(1<<bit));
	}
	return 0;
}

int
MSIM_LUAF_AVRSetIOBit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char bit = (unsigned char)lua_tointeger(L, 3);
	unsigned char val = (unsigned char)lua_tointeger(L, 4);

	/* Model is trying to write something else. */
	if ((mcu->ioregs_num == 0) ||
	                (io_reg >= (mcu->sfr_off+mcu->ioregs_num)) ||
	                (io_reg < mcu->sfr_off)) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "bit of unknown I/O register: %u", io_reg);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	/* Model is trying to write unknown bit. */
	if (bit >= 8) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "unknown bit: %u", bit);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	if (val&1) {
		mcu->dm[io_reg] |= (unsigned char)(1<<bit);
	} else {
		mcu->dm[io_reg] &= (unsigned char)(~(1<<bit));
	}
	return 0;
}

int
MSIM_LUAF_AVRWriteReg(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char val = (unsigned char)lua_tointeger(L, 3);

	/* Model is trying to write something else. */
	if (reg >= mcu->regs_num) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "unknown register: %u", reg);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	mcu->dm[reg] = val;
	return 0;
}

int
MSIM_LUAF_AVRWriteIO(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned short io_reg = (unsigned short)lua_tointeger(L, 2);
	unsigned char val = (unsigned char)lua_tointeger(L, 3);

	/* Model is trying to write something else. */
	if ((mcu->ioregs_num == 0) ||
	                (io_reg >= (mcu->sfr_off+mcu->ioregs_num)) ||
	                (io_reg < mcu->sfr_off)) {
		snprintf(mcu->log, sizeof mcu->log, "lua model is writing "
		         "unknown I/O register: %u", io_reg);
		MSIM_LOG_ERROR(mcu->log);
		return 0;
	}
	mcu->dm[io_reg] = val;
	return 0;
}

int
MSIM_LUAF_AVRWriteIO16(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	uint16_t io_high = (uint16_t)lua_tointeger(L, 2);
	uint16_t io_low = (uint16_t)lua_tointeger(L, 3);
	uint16_t val = (uint16_t)lua_tointeger(L, 4);
	const uint32_t l = mcu->sfr_off;
	const uint32_t h = l + mcu->ioregs_num;

	/* Model is trying to write something else. */
	if ((l == h) || (io_high >= h) || (io_high < l)) {
		snprintf(LOG, LOGSZ, "lua model is writing unknown 16-bit I/O "
		         "register: %u:%u", io_high, io_low);
		MSIM_LOG_ERROR(LOG);
		return 0;
	}

	mcu->dm[io_high] = (uint8_t)((val>>8)&0xFF);
	mcu->dm[io_low] = (uint8_t)(val&0xFF);
	return 0;
}
