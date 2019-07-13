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
 * Updates separate bits of the PINx register according to a value in
 * the PORTx register. The only bits which are configured to output will be
 * updated.
 */
int
MSIM_AVR_IOUpdatePinx(struct MSIM_AVR *mcu)
{
	MSIM_AVR_IOPort *p;
	uint32_t v, ddr_mask, pinv;

	for (uint32_t i = 0; i < ARRSZ(mcu->ioports); i++) {
		/* I/O port to work with */
		p = &mcu->ioports[i];

		/* Skip uninitialized I/O port */
		if (IS_IONOBYTE(p->port) || IS_IONOBYTE(p->ddr) ||
		                IS_IONOBYTE(p->pin)) {
			continue;
		}

		/* Read port registers */
		v = IOBIT_RD(mcu, &p->port);
		ddr_mask = IOBIT_RD(mcu, &p->ddr);
		pinv = IOBIT_RD(mcu, &p->pin);

		/* Update PINx */
		IOBIT_WR(mcu, &p->pin, pinv | (v & ddr_mask));
	}

	return 0;
}
