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
 * Utility macro for the AVR I/O registers.
 */
#ifndef MSIM_AVR_IO_MACRO_H_
#define MSIM_AVR_IO_MACRO_H_ 1

#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/private/macro.h"

/* Macros to initialize a bit of AVR I/O register. */
#define IOBIT(io, bit)		{ .reg=(io), .bit=(bit), .mask=1 }
#define IOBITS(io, bit, mask)	{ .reg=(io), .bit=(bit), .mask=(mask) }
#define IOBYTE(io)		{ .reg=(io), .bit=0, .mask=0xFF }
#define IONOBIT()		{ .reg=0, .bit=UINT8_MAX, .mask=0 }
#define IONOBYTE()		{ .reg=0, .bit=UINT8_MAX, .mask=0 }

/* Macro to warn if a bit doesn't belong to the actual I/O register. */
#ifdef DEBUG
#define WARN_IF_NOTIO(b) do {						\
	if (!IS_IO(mcu, b.reg)) {					\
		snprintf(LOG, LOGSZ, "not I/O register: 0x%04"		\
		         PRIX32, b.reg);				\
		MSIM_LOG_DEBUG(LOG);					\
	}								\
} while (0)
#else
#define WARN_IF_NOTIO(b) do {						\
} while (0)
#endif

/* Read bit/bits of the AVR I/O register. */
static inline uint8_t IOBIT_RD(struct MSIM_AVR *mcu, struct MSIM_AVR_IOBit b)
{
	WARN_IF_NOTIO(b);

	return (DM(b.reg) >> b.bit) & b.mask;
}

/* Write bit/bits of the AVR I/O register */
static inline uint8_t IOBIT_WR(struct MSIM_AVR *mcu, struct MSIM_AVR_IOBit b,
                               uint8_t v)
{
	WARN_IF_NOTIO(b);

	uint32_t m = b.mask << b.bit;
	uint8_t val = ((uint8_t)(v << b.bit)) & m;
	WRITE_DS(b.reg, (DM(b.reg) & (~m)) | val);

	return (DM(b.reg) >> b.bit) & b.mask;
}

#endif /* MSIM_AVR_IO_MACRO_H_ */
