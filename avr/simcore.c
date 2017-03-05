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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "avr/sim/sim.h"
#include "avr/sim/bootloader.h"
#include "avr/sim/simcore.h"

static int decode_inst(struct avr *mcu, uint16_t inst);
static void sreg_update_flag(struct avr *mcu, enum avr_sreg_flag flag,
			     uint8_t set_f);

void simulate_avr(struct avr *mcu)
{
	uint16_t inst;

	while (1) {
		inst = mcu->prog_mem[mcu->pc];

		if (decode_inst(mcu, inst)) {
			/*
			 * Unknown instruction.
			 */
			fprintf(stderr, "Unknown instruction: 0x%X\n", inst);
			abort();
		}
	}
}

void sreg_set_flag(struct avr *mcu, enum avr_sreg_flag flag)
{
	sreg_update_flag(mcu, flag, 1);
}

void sreg_clear_flag(struct avr *mcu, enum avr_sreg_flag flag)
{
	sreg_update_flag(mcu, flag, 0);
}

static void sreg_update_flag(struct avr *mcu, enum avr_sreg_flag flag,
			     uint8_t set_f)
{
	uint8_t v;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
		return;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		v = 0x01;
		break;
	case AVR_SREG_ZERO:
		v = 0x02;
		break;
	case AVR_SREG_NEGATIVE:
		v = 0x04;
		break;
	case AVR_SREG_TWOSCOM_OF:
		v = 0x08;
		break;
	case AVR_SREG_SIGN:
		v = 0x10;
		break;
	case AVR_SREG_HALF_CARRY:
		v = 0x20;
		break;
	case AVR_SREG_BITCOPY_ST:
		v = 0x40;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		break;
	}

	if (set_f)
		*mcu->sreg |= v;
	else
		*mcu->sreg &= ~v;
}

static int decode_inst(struct avr *mcu, uint16_t inst)
{
	uint16_t op, r, d;

	switch (inst & 0xF000) {
	/*
	 * RJMP - Relative Jump
	 */
	case 0xC000:
		op = inst & 0x0FFF;
		mcu->pc += (uint32_t) op + 1;
		break;
	case 0x2000:
		switch (inst & 0xFC00) {
		/*
		 * EOR - Exclusive OR
		 */
		case 0x2400:
			d = (inst & 0x01F0) >> 4;
			r = (inst & 0x0F) | ((inst & 0x0200) >> 5);

			mcu->data_mem[d] = mcu->data_mem[d] ^ mcu->data_mem[r];
			mcu->pc++;
			sreg_clear_flag(mcu, AVR_SREG_TWOSCOM_OF);
			/* ... */
			break;
		}
		break;
	case 0xB000:
		switch (inst & 0xF800) {
		/*
		 * OUT – Store Register to I/O Location
		 */
		case 0xB800:
			d = (inst & 0x0F) | ((inst & 0x0600) >> 5);
			r = (inst & 0x01F0) >> 4;

			mcu->data_mem[d] = mcu->data_mem[r];
			mcu->pc++;
			break;
		/*
		 * IN - Load an I/O Location to Register
		 */
		case 0xB000:
			break;
		}
		break;
	}

	return 0;
}
