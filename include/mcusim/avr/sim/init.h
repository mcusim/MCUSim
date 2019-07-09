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
#ifndef MSIM_AVR_INIT_H_
#define MSIM_AVR_INIT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct MSIM_InitArgs {
	uint8_t *pm;
	uint8_t *dm;
	uint32_t pmsz;
	uint32_t dmsz;
} MSIM_InitArgs;

/* Initialize MCU as ATmega8A */
int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega328 */
int MSIM_M328Init(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega328P */
int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega2560 */
int MSIM_M2560Init(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_INIT_H_ */
