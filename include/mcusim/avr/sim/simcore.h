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
