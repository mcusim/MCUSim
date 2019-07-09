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

/* Device models defined as Lua scripts can be used during a scheme
 * simulation in order to substitute important parts (external RAM,
 * displays, etc.) connected to the simulated microcontroller.
 *
 * This file provides basic functions to load, run and unload these models.
 */
#include <stdint.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/luaapi.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static lua_State *lua_states[MSIM_AVR_LUAMODELS];
static uint64_t models_num;

int
MSIM_AVR_LUALoadModel(struct MSIM_AVR *mcu, char *model)
{
	uint8_t err = 0;
	uint64_t i = models_num;

	/* Initialize Lua */
	lua_states[i] = luaL_newstate();
	/* Load various Lua libraries */
	luaL_openlibs(lua_states[i]);

	/* Load peripheral */
	if (luaL_loadfile(lua_states[i], model) ||
	                lua_pcall(lua_states[i], 0, 0, 0)) {
		snprintf(LOG, LOGSZ, "cannot load model: %s, reason: %s",
		         model, lua_tostring(lua_states[i], -1));
		MSIM_LOG_ERROR(LOG);
		err = 1;
	} else {
		models_num++;
		/* Register MCUSim API functions */
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRIOBit);
		lua_setglobal(lua_states[i], "AVR_IOBit");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRReadIO);
		lua_setglobal(lua_states[i], "AVR_ReadIO");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRReadIO16);
		lua_setglobal(lua_states[i], "AVR_ReadIO16");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRReadReg);
		lua_setglobal(lua_states[i], "AVR_ReadReg");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRRegBit);
		lua_setglobal(lua_states[i], "AVR_RegBit");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRSetIOBit);
		lua_setglobal(lua_states[i], "AVR_SetIOBit");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRSetRegBit);
		lua_setglobal(lua_states[i], "AVR_SetRegBit");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRWriteIO);
		lua_setglobal(lua_states[i], "AVR_WriteIO");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRWriteIO16);
		lua_setglobal(lua_states[i], "AVR_WriteIO16");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_AVRWriteReg);
		lua_setglobal(lua_states[i], "AVR_WriteReg");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_SetState);
		lua_setglobal(lua_states[i], "MSIM_SetState");
		lua_pushcfunction(lua_states[i], MSIM_LUAF_Freq);
		lua_setglobal(lua_states[i], "MSIM_Freq");
		/* Override existing Lua functions */
		lua_pushcfunction(lua_states[i], MSIM_LUAF_Print);
		lua_setglobal(lua_states[i], "print");

		/* Add registers available for the current MCU model to
		 * the Lua state. */
		for (uint32_t j = 0; j < MSIM_AVR_DMSZ; j++) {
			if (mcu->ioregs[j].off < 0) {
				continue;
			}
			if (mcu->ioregs[j].name[0] == 0) {
				continue;
			}

			lua_pushinteger(lua_states[i],
			                (int)mcu->ioregs[j].off);
			lua_setglobal(lua_states[i], mcu->ioregs[j].name);
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
		if (lua_pcall(lua_states[i], 1, 0, 0) != 0) {
#ifdef DEBUG
			snprintf(LOG, LOGSZ, "model %s does not provide a "
			         "configuration function: %s", model,
			         lua_tostring(lua_states[i], -1));
			MSIM_LOG_DEBUG(LOG);
#endif
		}
		/* Find a "module_tick" function and push it onto a stack.
		 * This is a cache mechanism of the function value. */
		lua_getglobal(lua_states[i], "module_tick");
	}
	return err;
}

void
MSIM_AVR_LUACleanModels(void)
{
	for (uint64_t i = 0; i < models_num; i++) {
		if (lua_states[i] != NULL) {
			lua_close(lua_states[i]);
		}
	}
	models_num = 0;
}

void
MSIM_AVR_LUATickModels(struct MSIM_AVR *mcu)
{
	for (uint32_t i = 0; i < models_num; i++) {
		if (lua_states[i] == NULL) {
			break;
		}
		/* Push a cached value of the "module_tick" function onto
		 * the stack. */
		lua_pushvalue(lua_states[i], -1);
		/* Push argument to call the "module_tick" function. */
		lua_pushlightuserdata(lua_states[i], mcu);
#ifndef DEBUG
		/* Call the "module_tick" function. */
		lua_call(lua_states[i], 1, 0);
#else
		/* Call the "module_tick" function (protected call). */
		if (lua_pcall(lua_states[i], 1, 0, 0) != 0) {
			snprintf(LOG, LOGSZ, "cannot run module_tick(): %s",
			         lua_tostring(lua_states[i], -1));
			MSIM_LOG_DEBUG(LOG);
		}
#endif
	}
}
