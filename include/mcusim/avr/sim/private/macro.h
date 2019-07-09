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

/* Utility macro for AVR MCU. */
#ifndef MSIM_AVR_MACRO_H_
#define MSIM_AVR_MACRO_H_ 1

#include <inttypes.h>

#include "mcusim/mcusim.h"

#define REG_ZH			0x1F
#define REG_ZL			0x1E

/* Indexes of the fuse bytes. */
#define FUSE_EXT		2
#define FUSE_HIGH		1
#define FUSE_LOW		0

/* Bits of the AVR status register (SREG). */
#define SR_GLOBINT		7
#define SR_TBIT			6
#define SR_HCARRY		5
#define SR_SIGN			4
#define SR_TCOF			3
#define SR_NEG			2
#define SR_ZERO			1
#define SR_CARRY		0

/* Offset to AVR I/O (special function) registers. */
#define SFR			(mcu->sfr_off)

/* Helps to access AVR memories and registers. */
#define PM(v)			(mcu->pm[(v)])
#define DM(v)			(mcu->dm[(v)])
#define IOR(v)			(DM(SFR + (v)))
#define MPM(v)			(mcu->mpm[(v)])

#define LOG			(mcu->log)
#define LOGSZ			(MSIM_AVR_LOGSZ)
#define TICKS_MAX		(UINT64_MAX)
#define ARRSZ(a)		(sizeof(a)/sizeof((a)[0]))

/* Macro to provide a result of writing value to I/O register with its
 * access mask applied.
 *
 * io		Address of an I/O register in data space.
 * v		Value to write to the register.
 */
#define IO(io, v)		((mcu->dm[io])&(~(mcu->ioregs[io].mask))) | \
				((v)&(mcu->ioregs[io].mask))

/* Helps to test whether I/O register of the AVR MCU been written or not. */
#define IS_WRIT(mcu, io)	(((mcu->writ_io[0]) == (io)) || \
				 ((mcu->writ_io[1]) == (io)) || \
				 ((mcu->writ_io[2]) == (io)) || \
				 ((mcu->writ_io[3]) == (io)))
/* Helps to test whether I/O register of the AVR MCU been read or not. */
#define IS_READ(mcu, io)	(((mcu->read_io[0]) == (io)) || \
				 ((mcu->read_io[1]) == (io)) || \
				 ((mcu->read_io[2]) == (io)) || \
				 ((mcu->read_io[3]) == (io)))

/* Helps to test whether location is I/O register or not. */
#define IS_IO(mcu, loc)		((mcu->regs_num <= loc) && \
				 (loc < (mcu->regs_num+mcu->ioregs_num)))

/* Read an AVR status register (SREG). */
#define SR(mcu, flag)		((uint8_t)((*mcu->sreg >> (flag))&1))

/* Update an AVR status register (SREG). */
#define UPDSR(mcu, flag, set_f) do {					\
	if ((set_f) == 0) {						\
		*mcu->sreg &= (uint8_t)~(1<<(flag));			\
	} else {							\
		*mcu->sreg |= (uint8_t)(1<<(flag));			\
	}								\
} while (0)

/* Macro to help opcode function to skip required number of clock cycles
 * required to execute instruction on a real hardware.
 *
 * This is necessary to perform a cycle-accurate simulation because each
 * instruction should be finished within required number of cycles in spite
 * of the fact that major of the AVR instructions occupy 1 clock cycle only.
 *
 * mcu		Pointer to the MCU instance structure.
 * cond		Condition to start skipping cycles.
 * cycl		Number of cycles to skip (this is a number of cycles per
 * 		instruction minus one)
 */
#define SKIP_CYCLES(mcu, cond, cycl) do {				\
	if (!mcu->mci && (cond)) {				\
		/* It is the first cycle of a multi-cycle instruction */\
		mcu->mci = 1;					\
		mcu->ic_left = cycl;					\
		return;							\
	}								\
	if (mcu->mci && mcu->ic_left) {				\
		/* Skip intermediate cycles */				\
		if (--mcu->ic_left) {					\
			return;						\
		}							\
	}								\
	mcu->mci = 0;						\
} while (0)

/* Write value to the data space. Location will be checked against space of
 * I/O registers and access mask will be applied if necessary. */
#ifndef DEBUG
#define WRITE_DS(loc, v) do {						\
	if (IS_IO(mcu, loc)) {						\
		DM(loc) = ((uint8_t)IO(loc, v));			\
		mcu->writ_io[0] = loc;					\
	} else {							\
		DM(loc) = v;						\
	}								\
} while (0)
#endif

/* NOTE: Debug only.
 * Write value to the data space. Location will be checked against space of
 * I/O registers and access mask will be applied if necessary. */
#ifdef DEBUG
#define WRITE_DS(loc, v) do {						\
	if (IS_IO(mcu, loc)) {						\
		if (mcu->ioregs[loc].off < 0) {				\
			snprintf(LOG, LOGSZ, "firmware is trying to "	\
			         "write to unknown I/O register: 0x%04"	\
			         PRIX32, loc);				\
			MSIM_LOG_DEBUG(LOG);				\
		}							\
		DM(loc) = IO(loc, v);					\
		mcu->writ_io[0] = loc;					\
	} else {							\
		DM(loc) = v;						\
	}								\
} while (0)
#endif

#endif /* MSIM_AVR_MACRO_H_ */
