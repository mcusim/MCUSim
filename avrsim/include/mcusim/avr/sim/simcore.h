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
#ifndef MSIM_AVR_SIMCORE_H_
#define MSIM_AVR_SIMCORE_H_ 1

#include <stdio.h>
#include <stdint.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"

#ifdef __cplusplus
extern "C" {
#endif

void MSIM_SimulateAVR(struct MSIM_AVR *mcu);

int MSIM_InitAVR(struct MSIM_AVR *mcu, const char *mcu_name,
		 uint8_t *pm, uint32_t pm_size,
		 uint8_t *dm, uint32_t dm_size,
		 FILE *fp);

void MSIM_UpdateSREGFlag(struct MSIM_AVR *mcu,
			 enum MSIM_AVRSREGFlag flag,
			 uint8_t set_f);

uint8_t MSIM_ReadSREGFlag(struct MSIM_AVR *mcu,
			  enum MSIM_AVRSREGFlag flag);

void MSIM_StackPush(struct MSIM_AVR *mcu, uint8_t val);

uint8_t MSIM_StackPop(struct MSIM_AVR *mcu);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIMCORE_H_ */
