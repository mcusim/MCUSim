/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Utility macro for AVR MCU.
 */
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

/* Helps to access data memory (general purpose registers, I/O registers or
 * SRAM) of the AVR MCU. */
#define DM(v)			(mcu->dm[v])

#define LOG			(mcu->log)
#define LOGSZ			(MSIM_AVR_LOGSZ)
#define TICKS_MAX		(UINT64_MAX)
#define ARR_LEN(a)		(sizeof(a)/sizeof(a[0]))

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
#define SR(mcu, flag) ((uint8_t)((*mcu->sreg>>(flag))&1))

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
	if (!mcu->in_mcinst && (cond)) {				\
		/* It is the first cycle of a multi-cycle instruction */\
		mcu->in_mcinst = 1;					\
		mcu->ic_left = cycl;					\
		return;							\
	}								\
	if (mcu->in_mcinst && mcu->ic_left) {				\
		/* Skip intermediate cycles */				\
		if (--mcu->ic_left) {					\
			return;						\
		}							\
	}								\
	mcu->in_mcinst = 0;						\
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
