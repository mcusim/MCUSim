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

#define IOBIT(_io, _bit)	 { .reg=(_io), .bit=(_bit), .mask=1 }
#define IOBITS(_io, _bit, _mask) { .reg=(_io), .bit=(_bit), .mask=(_mask) }
#define IOBYTE(_io)		 { .reg=(_io), .bit=0, .mask=0xFF }
#define IONOBIT()		 { .reg=0, .bit=0, .mask=0 }
#define IONOBYTE()		 { .reg=0, .bit=0, .mask=0 }
#define NB			 IONOBIT()
#define IONOBITA()		 { NB }
#define IONOBYTEA()		 { NB }
#define NOWGM()			 { .top=0, .bottom=0, .size=0, .kind=0 }
#define NOINTV()		 { .enable=IONOBIT(), .raised=IONOBIT(), \
				   .vector=0 }
#define NOCOMP()		 { .owner=0, .com=IONOBIT(), .pin=IONOBIT(), \
				   .iv=NOINTV(), .ocr=IONOBITA() }
#define NOWGMA()		 { NOWGM() }
#define NOINTVA()		 { NOINTV() }
#define NOCOMPA()		 { NOCOMP() }

#define UPD_ATNONE		MSIM_AVR_TMR_UPD_ATNONE
#define UPD_ATMAX		MSIM_AVR_TMR_UPD_ATMAX
#define UPD_ATTOP		MSIM_AVR_TMR_UPD_ATTOP
#define UPD_ATBOTTOM		MSIM_AVR_TMR_UPD_ATBOTTOM
#define UPD_ATIMMEDIATE		MSIM_AVR_TMR_UPD_ATIMMEDIATE

#define WGM_NONE		MSIM_AVR_TMR_WGM_None
#define WGM_NORMAL		MSIM_AVR_TMR_WGM_Normal
#define WGM_CTC			MSIM_AVR_TMR_WGM_CTC
#define WGM_PWM			MSIM_AVR_TMR_WGM_PWM
#define WGM_FASTPWM8		MSIM_AVR_TMR_WGM_FastPWM8
#define WGM_FASTPWM9		MSIM_AVR_TMR_WGM_FastPWM9
#define WGM_FASTPWM10		MSIM_AVR_TMR_WGM_FastPWM10
#define WGM_FASTPWM		MSIM_AVR_TMR_WGM_FastPWM
#define WGM_PCPWM8		MSIM_AVR_TMR_WGM_PCPWM8
#define WGM_PCPWM9		MSIM_AVR_TMR_WGM_PCPWM9
#define WGM_PCPWM10		MSIM_AVR_TMR_WGM_PCPWM10
#define WGM_PCPWM		MSIM_AVR_TMR_WGM_PCPWM
#define WGM_PFCPWM		MSIM_AVR_TMR_WGM_PFCPWM

#define COM_DISC		MSIM_AVR_TMR_COM_DISC
#define COM_TGONCM		MSIM_AVR_TMR_COM_TGONCM
#define COM_CLONCM		MSIM_AVR_TMR_COM_CLONCM
#define COM_STONCM		MSIM_AVR_TMR_COM_STONCM
#define COM_CLONCM_STATBOT	MSIM_AVR_TMR_COM_CLONCM_STATBOT
#define COM_STONCM_CLATBOT	MSIM_AVR_TMR_COM_STONCM_CLATBOT
#define	COM_CLONUP_STONDOWN	MSIM_AVR_TMR_COM_CLONUP_STONDOWN
#define	COM_STONUP_CLONDOWN	MSIM_AVR_TMR_COM_STONUP_CLONDOWN

#define IS_IONOBIT(b)	((b.reg==0U)&&(b.bit==0U)&&(b.mask==0U))
#define IS_IONOBYTE(b)	((b.reg==0U)&&(b.bit==0U)&&(b.mask==0U))

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
