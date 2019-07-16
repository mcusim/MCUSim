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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"

/*
 * Synchronizes bits of the PINx register according to a value in the PORTx
 * register. The only bits which are configured to output will be updated.
 *
 * It's necessary to wait one clock cycle to be able to read back a software
 * assigned pin values from PINx.
 *
 * See figure 14-4, Synchronization when Reading a Software Assigned Pin Value,
 * ATmega328P Datasheet Rev. 8271J – 11/2015.
 */
int
MSIM_AVR_IOSyncPinx(struct MSIM_AVR *mcu)
{
	MSIM_AVR_IOPort *p;
	uint32_t portx, ddrx, pinx;

	for (uint32_t i = 0; i < ARRSZ(mcu->ioports); i++) {
		/* I/O port to work with */
		p = &mcu->ioports[i];

		/* Work with the first N initialized ports */
		if (IS_IONOBYTE(p->port) || IS_IONOBYTE(p->ddr) ||
		                IS_IONOBYTE(p->pin)) {
			break;
		}

		/* Update PINx from a pending value */
		if (p->pending == 1U) {
			IOBIT_WR(mcu, &p->pin, p->ppin);
			p->pending = 0;
		}

		/* Read PORTx, DDRx and PINx values */
		portx = IOBIT_RD(mcu, &p->port);
		ddrx = IOBIT_RD(mcu, &p->ddr);
		pinx = IOBIT_RD(mcu, &p->pin);

		/* Calculate a pending PINx value */
		p->ppin = (uint8_t)((pinx & ~ddrx) | (portx & ddrx));
		p->pending = 1;
	}

	return 0;
}
