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

/* Main header file which denotes a public API of the MCUSim library. */
#ifndef MSIM_MAIN_HEADER_H_
#define MSIM_MAIN_HEADER_H_ 1

#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/decoder.h"
#include "mcusim/avr/sim/gdb.h"
#include "mcusim/avr/sim/interrupt.h"
#include "mcusim/avr/sim/lua.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/wdt.h"
#include "mcusim/avr/sim/usart.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/timer.h"

#include "mcusim/pty.h"
#include "mcusim/log.h"
#include "mcusim/config.h"

#endif /* MSIM_MAIN_HEADER_H_ */
