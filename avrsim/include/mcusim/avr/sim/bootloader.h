/*
 * mcusim - Interactive simulator for microcontrollers.
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
#ifndef MSIM_AVR_BLDR_H_
#define MSIM_AVR_BLDR_H_ 1

#include <stdint.h>

#include "mcusim/avr/sim/sim.h"

#ifdef __cplusplus
extern "C" {
#endif

struct MSIM_AVRBootloader {
	uint32_t start;		/* The first bootloader address in
				   program memory, in bytes. */
	uint32_t end;		/* The last bootloader address in
				   program memory, in bytes. */
	uint32_t size;		/* The bootloader size, in bytes. */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_BLDR_H_ */
