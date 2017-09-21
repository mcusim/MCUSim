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
#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/sim/peripheral_lua.h"

#ifdef LUA51_FOUND
#	include "lua.h"
#	include "lualib.h"
#	include "lauxlib.h"

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

int MSIM_LoadLuaPeripherals(const char *file)
{
	static char path[4096];
	unsigned int i, ci;
	FILE *f;

	f = fopen(file, "r");
	if (f == NULL) {
		fprintf(stderr, "Cannot load Lua peripherals from: %s\n", file);
		return -1;
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
			fprintf(stderr, "Cannot load peripheral file: %s, "
					"reason: %s\n", path,
					lua_tostring(MSIM_LuaStates[i], -1));
			fclose(f);
			return -1;
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
	return 0;
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
			fprintf(stderr, "Error running function "
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
#else /* LUA51_FOUND is not defined */

int MSIM_LoadLuaPeripherals(const char *file)
{
	fprintf(stderr, "Lua peripherals are not supported!\n");
	return 0;
}

void MSIM_CleanLuaPeripherals(void)
{
	return;
}

void MSIM_TickLuaPeripherals(struct MSIM_AVR *mcu)
{
	return;
}

#endif /* LUA51_FOUND */
