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
#ifndef MSIM_AVR_PERIPHERAL_LUA_H_
#define MSIM_AVR_PERIPHERAL_LUA_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "mcusim/avr/sim/sim.h"

/* Maximum number of device models defined as Lua scripts to be loaded
 * during a simulation. */
#define LUA_PERIPHERALS				256

#ifdef LUA_FOUND

/* Load peripherals written in Lua from a given list file. */
int MSIM_LoadLuaPeripherals(struct MSIM_AVR *mcu, const char *);
/* Close previously created Lua states. */
void MSIM_CleanLuaPeripherals(void);
/* Call a "tick" function of the models during each cycle of simulation. */
void MSIM_TickLuaPeripherals(struct MSIM_AVR *mcu);

#else

/* Empty macros to replace function which cannot be implemented without
 * Lua library. */
#define MSIM_LoadLuaPeripherals(mcu, file)	1
#define MSIM_CleanLuaPeripherals(void)
#define MSIM_TickLuaPeripherals(mcu)

#endif /* LUA_FOUND */

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_PERIPHERAL_LUA_H_ */