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

/*
 * AVR opcodes executors.
 */
static void exec_in_out(struct avr *mcu, uint16_t inst,
			uint8_t reg, uint8_t io_loc);
static void exec_cmp_immediate(struct avr *mcu, uint16_t inst);
static void exec_cmp_carry(struct avr *mcu, uint16_t inst);
static void exec_eor(struct avr *mcu, uint16_t inst);
static void exec_load_immediate(struct avr *mcu, uint16_t inst);
static void exec_rel_jump(struct avr *mcu, uint16_t inst);
static void exec_brne(struct avr *mcu, uint16_t inst);

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

void sreg_update_flag(struct avr *mcu, enum avr_sreg_flag flag, uint8_t set_f)
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

uint8_t sreg_flag(struct avr *mcu, enum avr_sreg_flag flag)
{
	uint8_t v, pos;

	if (!mcu) {
		fprintf(stderr, "MCU is null");
		return UINT8_MAX;
	}

	switch (flag) {
	case AVR_SREG_CARRY:
		v = 0x01;
		pos = 0;
		break;
	case AVR_SREG_ZERO:
		v = 0x02;
		pos = 1;
		break;
	case AVR_SREG_NEGATIVE:
		v = 0x04;
		pos = 2;
		break;
	case AVR_SREG_TWOSCOM_OF:
		v = 0x08;
		pos = 3;
		break;
	case AVR_SREG_SIGN:
		v = 0x10;
		pos = 4;
		break;
	case AVR_SREG_HALF_CARRY:
		v = 0x20;
		pos = 5;
		break;
	case AVR_SREG_BITCOPY_ST:
		v = 0x40;
		pos = 6;
		break;
	case AVR_SREG_GLOB_INT:
		v = 0x80;
		pos = 7;
		break;
	}

	return (*mcu->sreg & v) >> pos;
}

static int decode_inst(struct avr *mcu, uint16_t inst)
{
	switch (inst & 0xF000) {
	case 0xC000:
		exec_rel_jump(mcu, inst);
		break;
	case 0x2000:
		switch (inst & 0xFC00) {
		case 0x2400:
			exec_eor(mcu, inst);
			break;
		}
		break;
	case 0xB000:
		exec_in_out(mcu, inst,
			    (inst & 0x01F0) >> 4,
			    (inst & 0x0F) | ((inst & 0x0600) >> 5));
		break;
	case 0xE000:
		exec_load_immediate(mcu, inst);
		break;
	case 0x3000:
		exec_cmp_immediate(mcu, inst);
		break;
	case 0x0000:
		switch (inst) {
		case 0x0000:	/* NOP – No Operation */
			mcu->pc++;
			break;
		default:
			switch (inst & 0xFC00) {
			case 0x0400:
				exec_cmp_carry(mcu, inst);
				break;
			}
			break;
		}
		break;
	case 0xF000:
		switch (inst & 0xFC07) {
		case 0xF401:
			exec_brne(mcu, inst);
			break;
		}
		break;
	}

	return 0;
}

static void exec_eor(struct avr *mcu, uint16_t inst)
{
	/*
	 * EOR - Exclusive OR
	 */
	uint8_t rd, rr;

	rd = (inst & 0x01F0) >> 4;
	rr = (inst & 0x0F) | ((inst & 0x0200) >> 5);

	mcu->data_mem[rd] = mcu->data_mem[rd] ^ mcu->data_mem[rr];
	mcu->pc++;

	sreg_update_flag(mcu, AVR_SREG_ZERO, !mcu->data_mem[rd]);
	sreg_update_flag(mcu, AVR_SREG_NEGATIVE, mcu->data_mem[rd] & 0x80);
	sreg_update_flag(mcu, AVR_SREG_TWOSCOM_OF, 0);
	sreg_update_flag(mcu, AVR_SREG_SIGN, (mcu->data_mem[rd] & 0x80) ^ 0);
}

static void exec_in_out(struct avr *mcu, uint16_t inst,
			uint8_t reg, uint8_t io_loc)
{
	switch (inst & 0xF800) {
	/*
	 * OUT – Store Register to I/O Location
	 */
	case 0xB800:
		mcu->data_mem[reg] = mcu->data_mem[io_loc];
		break;
	/*
	 * IN - Load an I/O Location to Register
	 */
	case 0xB000:
		mcu->data_mem[io_loc] = mcu->data_mem[reg];
		break;
	}
	mcu->pc++;
}

static void exec_cmp_immediate(struct avr *mcu, uint16_t inst)
{
	/*
	 * CPI – Compare with Immediate
	 */
	uint8_t rd, c, v, buf;

	rd = ((inst & 0xF0) >> 4) + 0x10;
	c = (inst & 0x0F) | ((inst & 0x0F00) >> 4);
	v = mcu->data_mem[rd] - c;
	mcu->pc++;

	buf = (~rd & c) | (c & v) | (v & ~rd);
	sreg_update_flag(mcu, AVR_SREG_CARRY, (buf >> 7) & 0x01);
	sreg_update_flag(mcu, AVR_SREG_ZERO, !v);
	sreg_update_flag(mcu, AVR_SREG_NEGATIVE, v & 0x80);
	sreg_update_flag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~c & ~v) | (~rd & c & v)) >> 7) & 1);
	sreg_update_flag(mcu, AVR_SREG_SIGN,
			 sreg_flag(mcu, AVR_SREG_NEGATIVE) ^
			 sreg_flag(mcu, AVR_SREG_TWOSCOM_OF));
	sreg_update_flag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 0x01);
}

static void exec_cmp_carry(struct avr *mcu, uint16_t inst)
{
	/*
	 * CPC – Compare with Carry
	 */
	uint8_t rd, rr, v, buf;

	rd = (inst & 0x01F0) >> 4;
	rr = (inst & 0x0F) | ((inst & 0x0200) >> 5);
	v = mcu->data_mem[rd] -
	    mcu->data_mem[rr] -
	    sreg_flag(mcu, AVR_SREG_CARRY);
	mcu->pc++;

	buf = (~rd & rr) | (rr & v) | (v & ~rd);
	sreg_update_flag(mcu, AVR_SREG_CARRY, (buf >> 7) & 0x01);
	sreg_update_flag(mcu, AVR_SREG_HALF_CARRY, (buf >> 3) & 0x01);

	sreg_update_flag(mcu, AVR_SREG_NEGATIVE, v & 0x80);
	sreg_update_flag(mcu, AVR_SREG_TWOSCOM_OF,
			 (((rd & ~rr & ~v) | (~rd & rr & v)) >> 7) & 1);
	sreg_update_flag(mcu, AVR_SREG_SIGN,
			 sreg_flag(mcu, AVR_SREG_NEGATIVE) ^
			 sreg_flag(mcu, AVR_SREG_TWOSCOM_OF));
	if (v)
		sreg_update_flag(mcu, AVR_SREG_ZERO, 0);
}

static void exec_load_immediate(struct avr *mcu, uint16_t inst)
{
	/*
	 * LDI – Load Immediate
	 */
	uint8_t rd_off, c;

	rd_off = (inst & 0xF0) >> 4;
	c = (inst & 0x0F) | ((inst & 0x0F00) >> 4);

	mcu->data_mem[0x10 + rd_off] = c;
	mcu->pc++;
}

static void exec_rel_jump(struct avr *mcu, uint16_t inst)
{
	/*
	 * RJMP - Relative Jump
	 */
	mcu->pc += (uint32_t) (inst & 0x0FFF) + 1;
}

static void exec_brne(struct avr *mcu, uint16_t inst)
{
	/*
	 * BRNE – Branch if Not Equal
	 */
	int16_t c;

	if (!sreg_flag(mcu, AVR_SREG_ZERO)) {
		/*
		 * Z == 0, i.e. Rd != Rr
		 */
		c = (int16_t) ((int16_t) (inst << 6)) >> 9;
		mcu->pc += c + 1;
	} else {
		/*
		 * Z == 1, i.e. Rd == Rr
		 */
		mcu->pc++;
	}
}
