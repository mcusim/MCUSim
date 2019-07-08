/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MSIM_AVR_SIMCORE_H_
#define MSIM_AVR_SIMCORE_H_ 1

#ifndef MSIM_MAIN_HEADER_H_
	#error "Please, include mcusim/mcusim.h instead of this header."
#endif

#include <stdio.h>
#include <stdint.h>
#include "mcusim/mcusim.h"
#include "mcusim/config.h"
#include "mcusim/avr/sim/init.h"

#ifdef __cplusplus
extern "C" {
#endif

int	MSIM_AVR_Init(MSIM_AVR *mcu, MSIM_CFG *conf);
int	MSIM_AVR_Simulate(MSIM_AVR *mcu, uint8_t ft);
int	MSIM_AVR_SimStep(MSIM_AVR *mcu, uint8_t ft);
int	MSIM_AVR_SaveProgMem(MSIM_AVR *mcu, const char *f);
int	MSIM_AVR_LoadProgMem(MSIM_AVR *mcu, const char *f);
int	MSIM_AVR_LoadDataMem(MSIM_AVR *mcu, const char *f);

void	MSIM_AVR_StackPush(MSIM_AVR *mcu, uint8_t val);
uint8_t	MSIM_AVR_StackPop(MSIM_AVR *mcu);

void	MSIM_AVR_PrintParts(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIMCORE_H_ */
