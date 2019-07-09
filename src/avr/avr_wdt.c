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

/* A model-independent AVR Watchdog timer. */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/wdt.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"
#include "mcusim/bit/private/macro.h"

int
MSIM_AVR_WDTUpdate(struct MSIM_AVR *mcu)
{
	return 0;
}
