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
 */
#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/sim/peripheral_lua.h"

#ifdef LUA_FOUND		/* Lua library is defined */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static lua_State *MSIM_LuaStates[LUA_PERIPHERALS];

/* AVRSim specific functions for peripherals. */

/*
 * Copies the value of ch into each of the first count characters of the
 * data memory pointed to by mcu and offset.
 *
 * Lua parameters:
 *	struct MSIM_AVR *mcu;
 *	unsigned long offset;
 *	int ch;
 *	unsigned long count;
 */
static int lua_avr_dmemset(lua_State *L);

/*
 * Reads one byte of the data memory pointed to by mcu and offset.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned long offset;
 */
static int lua_avr_dmemget(lua_State *L);

/*
 * Tests a bit value of a byte at the given address.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned long offset;
 * 	unsigned char bit;
 */
static int lua_avr_isset(lua_State *L);

/*
 * Sets bit of a byte at the given address.
 *
 * Lua parameters:
 * 	struct MSIM_AVR *mcu;
 * 	unsigned long offset;
 * 	unsigned char bit;
 * 	unsigned char val;
 */
static int lua_avr_setbit(lua_State *L);

/* END AVRSim specific functions for peripherals. */

void MSIM_LoadLuaPeripherals(const char *file)
{
	static char path[4096];
	unsigned int i, ci;
	FILE *f;

	f = fopen(file, "r");
	if (f == NULL) {
		fprintf(stderr, "ERRO: Cannot load Lua peripherals from: %s\n",
				file);
		return;
	}

	i = 0;
	while (fgets(path, sizeof path, f) != NULL) {
		for (ci = 0; ci < 4096; ci++)
			if (path[ci] == '\n') {
				path[ci] = 0;
				break;
			}
		/* Initialize Lua */
		MSIM_LuaStates[i] = luaL_newstate();

		/* Load various Lua libraries */
		luaL_openlibs(MSIM_LuaStates[i]);

		/* Load peripheral */
		if (luaL_loadfile(MSIM_LuaStates[i], path) ||
		    lua_pcall(MSIM_LuaStates[i], 0, 0, 0)) {
			fprintf(stderr, "ERRO: Cannot load peripheral "
			        "file: %s, reason: %s\n", path,
			        lua_tostring(MSIM_LuaStates[i], -1));
			fclose(f);
			return;
		}

		/* Register AVRSim specific functions */
		lua_pushcfunction(MSIM_LuaStates[i], lua_avr_dmemset);
		lua_setglobal(MSIM_LuaStates[i], "msim_avr_dmemset");
		lua_pushcfunction(MSIM_LuaStates[i], lua_avr_dmemget);
		lua_setglobal(MSIM_LuaStates[i], "msim_avr_dmemget");
		lua_pushcfunction(MSIM_LuaStates[i], lua_avr_isset);
		lua_setglobal(MSIM_LuaStates[i], "msim_avr_isset");
		lua_pushcfunction(MSIM_LuaStates[i], lua_avr_setbit);
		lua_setglobal(MSIM_LuaStates[i], "msim_avr_setbit");

		i++;
	}
	fclose(f);
}

void MSIM_CleanLuaPeripherals(void)
{
	unsigned int i;
	for (i = 0; i < LUA_PERIPHERALS; i++)
		if (MSIM_LuaStates[i] != NULL)
			lua_close(MSIM_LuaStates[i]);
}

void MSIM_TickLuaPeripherals(struct MSIM_AVR *mcu)
{
	unsigned int i;

	for (i = 0; i < LUA_PERIPHERALS; i++) {
		if (MSIM_LuaStates[i] == NULL)
			continue;

		lua_getglobal(MSIM_LuaStates[i], "module_tick");
		lua_pushlightuserdata(MSIM_LuaStates[i], mcu);
		if (lua_pcall(MSIM_LuaStates[i], 1, 0, 0) != 0)
			fprintf(stderr, "ERRO: Error running function "
			        "module_tick(): %s\n",
			        lua_tostring(MSIM_LuaStates[i], -1));
	}
}

static int lua_avr_dmemset(lua_State *L)
{
	unsigned long i;

	struct MSIM_AVR *mcu = (struct MSIM_AVR *)lua_touserdata(L, 1);
	unsigned long off = (unsigned long)lua_tointeger(L, 2);
	int ch = (int)lua_tointeger(L, 3);
	unsigned long count = (unsigned long)lua_tointeger(L, 4);

	for (i = off; i < off+count; i++)
		mcu->dm[i] = (unsigned char)ch;
	return 0;	/* Number of results */
}

static int lua_avr_dmemget(lua_State *L)
{
	struct MSIM_AVR *mcu = (struct MSIM_AVR *)lua_touserdata(L, 1);
	unsigned long off = (unsigned long)lua_tointeger(L, 2);

	lua_pushinteger(L, mcu->dm[off]);
	return 1;	/* Number of results */
}

static int lua_avr_isset(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned long off = (unsigned long)lua_tointeger(L, 2);
	unsigned char b = (unsigned char)lua_tointeger(L, 3);

	lua_pushboolean(L, mcu->dm[off]&(1<<b));
	return 1;
}

static int lua_avr_setbit(lua_State *L)
{
	struct MSIM_AVR *mcu = lua_touserdata(L, 1);
	unsigned long off = (unsigned long)lua_tointeger(L, 2);
	unsigned char b = (unsigned char)lua_tointeger(L, 3);
	unsigned char v = (unsigned char)lua_tointeger(L, 4);

	if (v)
		mcu->dm[off] |= (unsigned char)(1 << b);
	else
		mcu->dm[off] &= (unsigned char)(~(1 << b));
	return 0;
}
#endif /* LUA_FOUND */
