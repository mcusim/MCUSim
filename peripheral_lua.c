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
#include "mcusim/avr/sim/peripheral_lua.h"
#include "mcusim/avr/sim/peripheral_luaapi.h"
#ifdef LUA_FOUND
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static lua_State *lua_states[LUA_PERIPHERALS];

int MSIM_LoadLuaPeripherals(struct MSIM_AVR *mcu, const char *file)
{
	static char path[4096];
	unsigned int i, j, ci, regs;
	FILE *f;

	f = fopen(file, "r");
	if (f == NULL) {
		fprintf(stderr, "[e]: Cannot load Lua peripherals "
		        "from: %s\n", file);
		return 1;
	}

	i = 0;
	while (fgets(path, sizeof path, f) != NULL) {
		/* Skip comment lines */
		if (path[0] == '#')
			continue;
		for (ci = 0; ci < 4096; ci++)
			if (path[ci] == '\n') {
				path[ci] = 0;
				break;
			}
		/* Initialize Lua */
		lua_states[i] = luaL_newstate();

		/* Load various Lua libraries */
		luaL_openlibs(lua_states[i]);

		/* Load peripheral */
		if (luaL_loadfile(lua_states[i], path) ||
		                lua_pcall(lua_states[i], 0, 0, 0)) {
			fprintf(stderr, "[e]: Cannot load device model : %s, "
			        "reason: %s\n", path,
			        lua_tostring(lua_states[i], -1));
			fclose(f);
			return 1;
		}

		/* Register mcusim API functions */
		lua_pushcfunction(lua_states[i], flua_AVR_IOBit);
		lua_setglobal(lua_states[i], "AVR_IOBit");
		lua_pushcfunction(lua_states[i], flua_AVR_ReadIO);
		lua_setglobal(lua_states[i], "AVR_ReadIO");
		lua_pushcfunction(lua_states[i], flua_AVR_ReadReg);
		lua_setglobal(lua_states[i], "AVR_ReadReg");
		lua_pushcfunction(lua_states[i], flua_AVR_RegBit);
		lua_setglobal(lua_states[i], "AVR_RegBit");
		lua_pushcfunction(lua_states[i], flua_AVR_SetIOBit);
		lua_setglobal(lua_states[i], "AVR_SetIOBit");
		lua_pushcfunction(lua_states[i], flua_AVR_SetRegBit);
		lua_setglobal(lua_states[i], "AVR_SetRegBit");
		lua_pushcfunction(lua_states[i], flua_AVR_WriteIO);
		lua_setglobal(lua_states[i], "AVR_WriteIO");
		lua_pushcfunction(lua_states[i], flua_AVR_WriteReg);
		lua_setglobal(lua_states[i], "AVR_WriteReg");
		lua_pushcfunction(lua_states[i], flua_MSIM_SetState);
		lua_setglobal(lua_states[i], "MSIM_SetState");
		lua_pushcfunction(lua_states[i], flua_MSIM_Freq);
		lua_setglobal(lua_states[i], "MSIM_Freq");

		/* Add registers available for the current MCU model to
		 * the Lua state. */
		regs = sizeof mcu->vcdd->regs/sizeof mcu->vcdd->regs[0];
		for (j = 0; j < regs; j++) {
			if (mcu->vcdd->regs[j].name[0] == 0)
				break;
			if (mcu->vcdd->regs[j].off < 0)
				continue;

			lua_pushinteger(lua_states[i],
			                (int)mcu->vcdd->regs[j].off);
			lua_setglobal(lua_states[i], mcu->vcdd->regs[j].name);
		}

		/* Add available MCU states to the Lua state. */
		lua_pushinteger(lua_states[i], AVR_RUNNING);
		lua_setglobal(lua_states[i], "AVR_RUNNING");
		lua_pushinteger(lua_states[i], AVR_STOPPED);
		lua_setglobal(lua_states[i], "AVR_STOPPED");
		lua_pushinteger(lua_states[i], AVR_SLEEPING);
		lua_setglobal(lua_states[i], "AVR_SLEEPING");
		lua_pushinteger(lua_states[i], AVR_MSIM_STEP);
		lua_setglobal(lua_states[i], "AVR_MSIM_STEP");
		lua_pushinteger(lua_states[i], AVR_MSIM_STOP);
		lua_setglobal(lua_states[i], "AVR_MSIM_STOP");
		lua_pushinteger(lua_states[i], AVR_MSIM_TESTFAIL);
		lua_setglobal(lua_states[i], "AVR_MSIM_TESTFAIL");

		/* Attempt to call configuration function of the
		 * current model */
		lua_getglobal(lua_states[i], "module_conf");
		lua_pushlightuserdata(lua_states[i], mcu);
		if (lua_pcall(lua_states[i], 1, 0, 0) != 0)
			printf("[I]: Model %s doesn't provide configuration "
			       "function: %s\n",
			       path, lua_tostring(lua_states[i], -1));

		i++;
	}
	fclose(f);
	return 0;
}

void MSIM_CleanLuaPeripherals(void)
{
	unsigned int i;
	for (i = 0; i < LUA_PERIPHERALS; i++)
		if (lua_states[i] != NULL)
			lua_close(lua_states[i]);
}

void MSIM_TickLuaPeripherals(struct MSIM_AVR *mcu)
{
	unsigned int i;

	for (i = 0; i < LUA_PERIPHERALS; i++) {
		if (lua_states[i] == NULL)
			continue;

		lua_getglobal(lua_states[i], "module_tick");
		lua_pushlightuserdata(lua_states[i], mcu);
		if (lua_pcall(lua_states[i], 1, 0, 0) != 0)
			fprintf(stderr, "[e]: Error running function "
			        "module_tick(): %s\n",
			        lua_tostring(lua_states[i], -1));
	}
}
#endif /* LUA_FOUND */
