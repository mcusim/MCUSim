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

/* Utility macro for the AVR I/O registers. */
#ifndef MSIM_AVR_IO_MACRO_H_
#define MSIM_AVR_IO_MACRO_H_ 1

#include <inttypes.h>
#include <math.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/private/macro.h"

/* for I/O registers */
#define IOBIT(io, b)		{ .reg=(io), .bit=(b), .mask=1, .mbits=1 }
#define IOBITS(io, b, m, mb)	{ .reg=(io), .bit=(b), .mask=(m), .mbits=(mb) }
#define IOBYTE(io)		{ .reg=(io), .bit=0, .mask=0xFF, .mbits=8 }
#define IONOBIT()		{ .reg=0, .bit=0, .mask=0, .mbits=0 }
#define IONOBYTE()		{ .reg=0, .bit=0, .mask=0, .mbits=0 }
#define NB			IONOBIT()
#define IONOBITA()		{ NB }
#define IONOBYTEA()		{ NB }

/* for fuse bytes */
#define FBIT(io, b)		IOBIT(io, b)
#define FBITS(io, b, m, mb)	IOBITS(io, b, m, mb)
#define FBYTE(io)		IOBYTE(io)
#define FNOBIT()		IONOBIT()
#define FNOBYTE()		IONOBYTE()
#define FNOBITA()		IONOBITA()
#define FNOBYTEA()		IONOBYTEA()

#define NOWGM()			{ .top=0, .bottom=0, .size=0, .kind=0 }
#define NOWGMA()		{ NOWGM() }
#define NOINTV()		{ .enable=NB, .raised=NB, .vector=0 }
#define NOINTVA()		{ NOINTV() }
#define NOCOMP()		{ .com=NB, .pin=NB, .iv=NOINTV(), .ocr={ NB } }
#define NOCOMPA()		{ NOCOMP() }

/* When to update buffered values */
#define UPD_ATNONE		MSIM_AVR_TMR_UPD_ATNONE
#define UPD_ATMAX		MSIM_AVR_TMR_UPD_ATMAX
#define UPD_ATTOP		MSIM_AVR_TMR_UPD_ATTOP
#define UPD_ATBOTTOM		MSIM_AVR_TMR_UPD_ATBOTTOM
#define UPD_ATIMMEDIATE		MSIM_AVR_TMR_UPD_ATIMMEDIATE
#define UPD_ATCM		MSIM_AVR_TMR_UPD_ATCM

/* Timer count direction */
#define CNT_UP			MSIM_AVR_TMR_CNTUP
#define CNT_DOWN		MSIM_AVR_TMR_CNTDOWN

/* Waveform generation mode */
#define WGM_NONE		MSIM_AVR_TMR_WGM_None
#define WGM_NORMAL		MSIM_AVR_TMR_WGM_Normal
#define WGM_CTC			MSIM_AVR_TMR_WGM_CTC
#define WGM_PWM			MSIM_AVR_TMR_WGM_PWM
#define WGM_FASTPWM		MSIM_AVR_TMR_WGM_FastPWM
#define WGM_PCPWM		MSIM_AVR_TMR_WGM_PCPWM
#define WGM_PFCPWM		MSIM_AVR_TMR_WGM_PFCPWM

/* Output compare pin action. */
#define COM_DISC		MSIM_AVR_TMR_COM_DISC
#define COM_TGONCM		MSIM_AVR_TMR_COM_TGONCM
#define COM_CLONCM		MSIM_AVR_TMR_COM_CLONCM
#define COM_STONCM		MSIM_AVR_TMR_COM_STONCM
#define COM_CLONCM_STATBOT	MSIM_AVR_TMR_COM_CLONCM_STATBOT
#define COM_STONCM_CLATBOT	MSIM_AVR_TMR_COM_STONCM_CLATBOT
#define	COM_CLONUP_STONDOWN	MSIM_AVR_TMR_COM_CLONUP_STONDOWN
#define	COM_STONUP_CLONDOWN	MSIM_AVR_TMR_COM_STONUP_CLONDOWN

#define IS_IONOBIT(b)		(((b).reg==0U)&&((b).bit==0U)&&((b).mask==0U))
#define IS_IONOBYTE(b)		(((b).reg==0U)&&((b).bit==0U)&&((b).mask==0U))
#define IS_IONOBITA(v)		(IS_IONOBIT((v)[0]))
#define IS_IONOBYTEA(v)		(IS_IONOBYTE((v)[0]))
#define IS_NOCOMP(v)		(IS_IONOBIT((v)->com) &&		\
				 IS_IONOBIT((v)->pin) &&		\
				 IS_IONOBIT((v)->ocr[0]) &&		\
				 IS_IONOBIT((v)->iv.enable) &&		\
				 IS_IONOBIT((v)->iv.raised))
#define IS_NOINTV(v)		(IS_IONOBIT((v)->enable) &&		\
				 IS_IONOBIT((v)->raised))
#define IS_NOWGM(v)		(((v)->top==0U) && ((v)->bottom==0U) &&	\
				 ((v)->size==0U) && ((v)->kind==0U))

/* Read bits of the AVR I/O register. */
static inline uint32_t
IOBIT_RD(struct MSIM_AVR *mcu, MSIM_AVR_IOBit *b)
{
	return (DM(b->reg) >> b->bit) & b->mask;
}

/* Read an array of bits of the AVR I/O registers. */
static inline uint32_t
IOBIT_RDA(struct MSIM_AVR *mcu, MSIM_AVR_IOBit *bit, uint32_t len)
{
	uint32_t r = 0;		/* Result */
	uint8_t mb = 0;		/* Mask bits */

	/* Iterate all bits (and masked bits) */
	for (uint32_t i = 0; i < len; i++) {
		if (IS_IONOBIT(bit[i])) {
			break;
		}
		r |= (uint32_t)(IOBIT_RD(mcu, &bit[i]) << mb);
		mb += bit[i].mbits;
	}

	return r;
}

/* Write bits of the AVR I/O register. */
static inline void
IOBIT_WR(struct MSIM_AVR *mcu, MSIM_AVR_IOBit *b, uint32_t v)
{
	uint32_t m = b->mask << b->bit;
	uint8_t val = ((uint8_t)(v << b->bit)) & m;

	WRITE_DS(b->reg, (DM(b->reg) & (~m)) | val);
}

/* Write an array of bits of the AVR I/O registers. */
static inline void
IOBIT_WRA(struct MSIM_AVR *mcu, MSIM_AVR_IOBit *bit, uint32_t len, uint32_t v)
{
	uint32_t mb = 0;	/* Mask bits */

	for (uint32_t i = 0; i < len; i++) {
		if (IS_IONOBIT(bit[i])) {
			break;
		}
		IOBIT_WR(mcu, &bit[i], ((v>>mb) & bit[i].mask));
		mb += bit[i].mbits;
	}
}

/* Compare two definitions of the AVR I/O bits (but not their values!) */
static inline uint8_t
IOBIT_CMP(MSIM_AVR_IOBit *b0, MSIM_AVR_IOBit *b1)
{
	return (uint8_t)((b0 != NULL) && (b1 != NULL) &&
	                 (b0->reg==b1->reg) &&
	                 (b0->bit==b1->bit) &&
	                 (b0->mask==b1->mask) &&
	                 (b0->mbits==b1->mbits)) ? 0U : 1U;
}

static inline uint8_t
IOBIT_CMPA(MSIM_AVR_IOBit *b0, MSIM_AVR_IOBit *b1, uint32_t count)
{
	uint8_t rc = 1; /* Not equal by default */

	do {
		if (count == 0U) {
			rc = 0;
			break;
		}
		for (uint32_t i = 0; i < count; i++) {
			rc = IOBIT_CMP(&b0[i], &b1[i]);
			if (rc != 0U) {
				break;
			}
		}
	} while (0);

	return rc;
}

static inline void
IOBIT_TG(struct MSIM_AVR *mcu, MSIM_AVR_IOBit *b)
{
	IOBIT_WR(mcu, b, ~(IOBIT_RD(mcu, b))&1);
}

#endif /* MSIM_AVR_IO_MACRO_H_ */
