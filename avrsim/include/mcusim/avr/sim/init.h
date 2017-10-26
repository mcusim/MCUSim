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
#ifndef MSIM_AVR_INIT_H_
#define MSIM_AVR_INIT_H_ 1

#ifndef MSIM_AVR_SIMCORE_H_
#	error "Include <mcusim/avr/sim/simcore.h> instead of this file."
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct MSIM_InitArgs {
	unsigned char *pm;
	unsigned char *dm;
	unsigned long pmsz;
	unsigned long dmsz;
};

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
