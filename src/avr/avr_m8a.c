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

/* ATmega8A-specific functions implemented here. */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/m8/m8a.h"
#include "mcusim/avr/sim/mcu_init.h"
#include "mcusim/avr/sim/timer.h"
#include "mcusim/bit/private/macro.h"

#define NOT_CONNECTED		0xFFU
#define COMPARE_MATCH		75
#define SET_TO_BOTTOM		76
#define COMP_MATCH_UPCNT	77
#define COMP_MATCH_DOWNCNT	78

/* Two arbitrary constants to mark two distinct output compare channels
 * of the microcontroller. */
#define A_CHAN			79
#define B_CHAN			80

static uint8_t init_portd;	/* PORTD (buffer) */
static uint8_t init_pind;	/* PIND (buffer) */
static uint8_t init_portb;	/* PORTB (buffer) */
static uint8_t init_pinb;	/* PINB (buffer) */

static uint8_t ucsra_buf;	/* USART Control and Status A (buffer) */
static uint8_t ucsrc_buf;	/* USART Control and Status C (buffer) */
static uint8_t ubrrh_buf;	/* USART Baud Rate High (buffer) */
static uint8_t ubrrl_buf;	/* USART Baud Rate Low (buffer) */
static uint8_t udr_buf;		/* USART Data Register */

static uint8_t spmcr_buf;	/* SPM Control Register */
static uint8_t spmen_cycles = 0; /* Clean SPMEN bit in this number of cycles */
static uint8_t spmen_clear = 0;	/* Flag to clear SPMEN in # of cycles */

static void update_watched(struct MSIM_AVR *mcu);

static void tick_usart(struct MSIM_AVR *mcu);
#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
	static void usart_transmit(struct MSIM_AVR *mcu);
	static void usart_receive(struct MSIM_AVR *mcu);
#endif

int
MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args)
{
	int rc = mcu_init(&ORIG_M8A, mcu, args);
	int r;

	do {
		if (rc != 0) {
			break;
		}

		/* I/O ports have internal pull-up resistors */
		DM(PORTB) = 0xFF;
		DM(PORTC) = 0xFF;
		DM(PORTD) = 0xFF;

		/* Set USART registers */
		DM(UCSRA) = 0x20;
		DM(UCSRC) = 0x82;

		/* Keep previous value of SPMCR */
		if (mcu->spmcsr != NULL) {
			spmcr_buf = *mcu->spmcsr;
		}

		update_watched(mcu);

		/* Set USART registers */
		ubrrh_buf = 0;

		/* Create a pseudo-terminal for this MCU */
		mcu->pty.master_fd = -1;
		mcu->pty.slave_fd = -1;
		mcu->pty.slave_name[0] = 0;

		r = MSIM_PTY_Open(&mcu->pty);
		if (r == 0) {
			snprintf(mcu->log, sizeof mcu->log, "USART is "
			         "available via: %s", mcu->pty.slave_name);
			MSIM_LOG_INFO(mcu->log);
		}
	} while (0);

	return rc;
}

int
MSIM_M8AUpdate(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf)
{
	tick_usart(mcu);

	/* Update watched values after all of the peripherals. */
	update_watched(mcu);

	return 0;
}

int
MSIM_M8AResetSPM(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf)
{
	if (mcu->spmcsr != NULL) {
		(*mcu->spmcsr) = (uint8_t)((*mcu->spmcsr) &
		                           (uint8_t)(~(1<<SPMEN)));
		/* Generate SPM_RDY interrupt */
		if ((*mcu->spmcsr>>SPMIE)&1U) {
			mcu->intr.irq[SPM_RDY_vect_num-1] = 1;
		}
	}
	return 0;
}

static void
update_watched(struct MSIM_AVR *mcu)
{
	init_portd = DM(PORTD);
	init_pind = DM(PIND);
	init_portb = DM(PORTB);
	init_pinb = DM(PINB);

	/* NOTE: The UBRRH register shares the same I/O location as the
	 * UCSRC Register (24.10. Accessing UBRRH/UCSRC Registers). It means
	 * that URSEL (MSB bit) should be checked additionally to understand
	 * which register should be updated */
	if (((DM(UBRRH)>>URSEL)&1) == 0) {
		ubrrh_buf = DM(UBRRH);
	} else {
		ucsrc_buf = DM(UCSRC);
	}
	ubrrl_buf = DM(UBRRL);
	ucsra_buf = DM(UCSRA);
	udr_buf = DM(UDR);

	/* Update SPMCR value and reset SPMEN bit if necessary */
	if (mcu->spmcsr != NULL) {
		if (spmen_clear == 1U) {
			if (spmen_cycles == 0U) {
				(*mcu->spmcsr) = (uint8_t)((*mcu->spmcsr) &
				                           (uint8_t)(~(1<<SPMEN)));
				spmen_clear = 0;
				/* Generate SPM_RDY interrupt */
				if ((*mcu->spmcsr>>SPMIE)&1U) {
					mcu->intr.irq[SPM_RDY_vect_num-1] = 1;
				}
			} else {
				spmen_cycles--;
			}
		}
		if (IS_RISE(spmcr_buf, *mcu->spmcsr, SPMEN)) {
			spmen_cycles = 4;
			spmen_clear = 1;
		}
		spmcr_buf = *mcu->spmcsr;
	}
}

static void
tick_usart(struct MSIM_AVR *mcu)
{
	/* You may think about this function as a programmable prescaler or
	 * baud rate generator.
	 *
	 * It is running at the system clock (Fosc) and is loaded with the
	 * 12-bit UBRR register value each time the counter has counted down
	 * to zero or when the UBRRL register is written.
	 * (see 24.3.1. Internal Clock Generation... for details) */
	uint32_t *rx_ticks = &mcu->usart.rx_ticks;
	uint32_t *tx_ticks = &mcu->usart.tx_ticks;
	uint32_t *baud = &mcu->usart.baud;
	uint32_t *rx_presc = &mcu->usart.rx_presc;
	uint32_t *tx_presc = &mcu->usart.tx_presc;
	uint8_t mult = 1;

	if ((ubrrl_buf != DM(UBRRL)) || (*tx_ticks == 0U)) {
		/* Load a new baud rate value */
		if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
			/* There is a UBRRH value stored in data memory after
			 * the last tick of the AVR decoder. */
			if (DM(UBRRH) != ubrrh_buf) {
				*baud = (uint32_t)((DM(UBRRH)&0x0F)<<8) |
				        (uint32_t)DM(UBRRL);
			} else {
				*baud = (uint32_t)((ubrrh_buf&0x0F)<<8) |
				        (uint32_t)DM(UBRRL);
			}
		} else {
			/* There is a UCSRC value stored in data memory after
			 * the last tick of the AVR decoder. */
			*baud = (uint32_t)((ubrrh_buf&0x0F)<<8) |
			        (uint32_t)DM(UBRRL);
		}

		/* Update Rx clock prescaler (and ticks?) */
		*rx_presc = *baud+1;
		*rx_ticks = *rx_presc;

		if (((DM(UCSRC)>>UMSEL)&1) == 0U) {
			if (((DM(UCSRA)>>U2X)&1) == 0U) {
				mult = 16; /* Asynchronous Normal mode */
			} else {
				mult = 8; /* Asynchronous Double Speed mode */
			}
		} else {
			mult = 1; /* Synchronous mode */
			MSIM_LOG_WARN("USART synchronous mode is not "
			              "supported yet, Txclk=Fosc/(UBRR+1)");
		}
		/* Update Tx clock prescaler (and ticks?) */
		*tx_presc = mult*(*baud+1);
		*tx_ticks = *tx_presc;
	}

	/* UDR has been written with UDRE flag set. It means that the Transmit
	 * Data Buffer Register (TXB) is a destination for data stored in the
	 * UDR Register location. */
	if ((IS_WRIT(mcu, UDR)) && (IS_SET(DM(UCSRA), UDRE) == 1U)) {
		mcu->usart.txb = DM(UDR);
		/* Clear UDRE flag */
		DM(UCSRA) = (uint8_t)(DM(UCSRA)&(uint8_t)(~(1<<UDRE)));
	}

	if (IS_READ(mcu, UDR)) {
		DM(UCSRA) = (uint8_t)(DM(UCSRA)&(uint8_t)(~(1<<RXC)));
	}

	/* Count-down Rx ticks */
	if (*rx_ticks > 0) {
		(*rx_ticks)--;
	}
	if ((*rx_ticks == 0U) && (((DM(UCSRB)>>RXEN)&1) == 1U)) {
		/* Generate Rx clock */
#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
		usart_receive(mcu);
#endif
		*rx_ticks = *rx_presc;
	} else {
		/* USART Rx inactive, do nothing */
	}

	/* Count-down Tx ticks */
	if (*tx_ticks > 0) {
		(*tx_ticks)--;
	}
	if ((*tx_ticks == 0U) && (((DM(UCSRB)>>TXEN)&1) == 1U)) {
		/* Generate Tx clock */
#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
		usart_transmit(mcu);
#endif
	} else {
		/* USART Tx inactive, do nothing */
	}
}

#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
static void
usart_transmit(struct MSIM_AVR *mcu)
{
	uint8_t buf[2];
	uint32_t buf_len = 1;
	uint8_t ucsz;
	uint8_t err = 0;
	int written;

	/* Find how many bits to transmit */
	if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
		/* There is a UBRRH value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)((uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		                 (uint8_t)(((ucsrc_buf>>UCSZ1)&1U)<<1) |
		                 (uint8_t)((ucsrc_buf>>UCSZ0)&1U));
	} else {
		/* There is a UCSRC value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)((uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		                 (uint8_t)(((DM(UCSRC)>>UCSZ1)&1U)<<1) |
		                 (uint8_t)((DM(UCSRC)>>UCSZ0)&1U));
	}

	buf[1] = 0;
	switch (ucsz) {
	case 0:			/* 5-bit */
		buf[0] = mcu->usart.txb&0x1F;
		break;
	case 1:			/* 6-bit */
		buf[0] = mcu->usart.txb&0x3F;
		break;
	case 2:			/* 7-bit */
		buf[0] = mcu->usart.txb&0x7F;
		break;
	case 3:			/* 8-bit */
		buf[0] = mcu->usart.txb;
		break;
	case 7:			/* 9-bit */
		/* NOTE: Should all other bits of buf[1] be filled from the
		 * next portion of USART transmit data (and not with zeroes)?*/
		buf[0] = mcu->usart.txb;
		buf[1] = (DM(UCSRB)>>TXB8)&1;
		buf_len = 2;
		break;
	default:
		snprintf(mcu->log, sizeof mcu->log, "these bits to select "
		         "USART character size are reserved: UCSZ=0x%" PRIX8,
		         ucsz);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0) && (IS_CLEAR(DM(UCSRA), UDRE) == 1)) {
		if (mcu->pty.master_fd >= 0) {
			written = MSIM_PTY_Write(&mcu->pty, buf, buf_len);
			if (written != (int)buf_len) {
				snprintf(mcu->log, sizeof mcu->log, "failed "
				         "to feed PTY master with USART data, "
				         "slave=%s", mcu->pty.slave_name);
				MSIM_LOG_ERROR(mcu->log);
			}
#ifdef DEBUG
			snprintf(mcu->log, sizeof mcu->log, "USART -> 0x%02"
			         PRIX8 ", pc=0x%06" PRIX32, buf[0], mcu->pc);
			MSIM_LOG_DEBUG(mcu->log);
#endif
			/* Set UDRE flag */
			DM(UCSRA) |= (1<<UDRE);
			/* Should TXC be cleared here? */
			DM(UCSRA) |= (1<<TXC);
		} else {
			MSIM_LOG_DEBUG("cannot feed PTY master with USART "
			               "data: master_fd < 0");
		}
	}
}

static void
usart_receive(struct MSIM_AVR *mcu)
{
	uint8_t buf[2];
	uint32_t buf_len = 1;
	uint32_t mask;
	uint8_t ucsz;
	uint8_t err = 0;
	int recv;

	/* Find how many bits to receive */
	if (((DM(UBRRH)>>UMSEL)&1) == 0U) {
		/* There is a UBRRH value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)((uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		                 (uint8_t)(((ucsrc_buf>>UCSZ1)&1U)<<1) |
		                 (uint8_t)((ucsrc_buf>>UCSZ0)&1U));
	} else {
		/* There is a UCSRC value stored in data memory after
		 * the last tick of the AVR decoder. */
		ucsz = (uint8_t)((uint8_t)(((DM(UCSRB)>>UCSZ2)&1U)<<2) |
		                 (uint8_t)(((DM(UCSRC)>>UCSZ1)&1U)<<1) |
		                 (uint8_t)((DM(UCSRC)>>UCSZ0)&1U));
	}

	switch (ucsz) {
	case 0:			/* 5-bit */
		mask = 0x1F;
		break;
	case 1:			/* 6-bit */
		mask = 0x3F;
		break;
	case 2:			/* 7-bit */
		mask = 0x7F;
		break;
	case 3:			/* 8-bit */
		mask = 0xFF;
		break;
	case 7:			/* 9-bit */
		/* NOTE: Should all other bits of buf[1] be filled from the
		 * next portion of USART receive data (and not with zeroes)? */
		mask = 0xFF;
		buf_len = 2;
		break;
	default:
		snprintf(mcu->log, sizeof mcu->log, "these bits to select "
		         "USART character size are reserved: UCSZ=0x%" PRIX8,
		         ucsz);
		MSIM_LOG_ERROR(mcu->log);
		err = 1;
		break;
	}

	if ((err == 0) && (IS_CLEAR(DM(UCSRA), RXC) == 1)) {
		if (mcu->pty.master_fd >= 0) {
			recv = MSIM_PTY_Read(&mcu->pty, buf, buf_len);

			if (recv == (int)buf_len) {
#ifdef DEBUG
				snprintf(mcu->log, sizeof mcu->log, "USART <- "
				         "0x%02" PRIX8 ", mask 0x%02" PRIX32,
				         buf[0], mask);
				MSIM_LOG_DEBUG(mcu->log);
#endif
				DM(UDR) = 0;
				DM(UDR) = (uint8_t)(DM(UDR) |
				                    (uint8_t)(buf[0]&mask));
				DM(UCSRB) = (uint8_t)(DM(UCSRB) &
				                      (uint8_t)(~(1<<TXB8)));
				if ((buf_len == 2U) && ((buf[1]&1) == 1U)) {
					DM(UCSRB) |= (1<<TXB8);
				}
				DM(UCSRA) |= (1<<RXC);
			}
		} else {
			MSIM_LOG_DEBUG("cannot read USART data from PTY "
			               "master: master_fd < 0");
		}
	}
}
#endif /* defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY) */

int
MSIM_M8ASetFuse(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf)
{
	uint32_t fuse_n = cnf->fuse_n;
	uint8_t fuse_v = cnf->fuse_v;
	uint8_t cksel, bootsz, ckopt;
	int ret;

	ret = 0;
	if (fuse_n > 1U) {
		snprintf(mcu->log, sizeof mcu->log, "fuse #%u is not "
		         "supported by %s", fuse_n, mcu->name);
		MSIM_LOG_ERROR(mcu->log);
		ret = 1;
	}

	if (ret == 0) {
		mcu->fuse[fuse_n] = fuse_v;
		cksel = mcu->fuse[0]&0xF;
		ckopt = (mcu->fuse[1]>>4)&0x1;

		switch (fuse_n) {
		case FUSE_LOW:
			cksel = fuse_v&0xFU;
			if (cksel == 0U) {
				mcu->clk_source = AVR_EXT_CLK;
			} else if ((cksel >= 1U) && (cksel <= 4U)) {
				mcu->clk_source = AVR_INT_CAL_RC_CLK;
				switch (cksel) {
				case 1:
					mcu->freq = 1000000;	/* max 1 MHz */
					break;
				case 2:
					mcu->freq = 2000000;	/* max 2 MHz */
					break;
				case 3:
					mcu->freq = 4000000;	/* max 4 MHz */
					break;
				case 4:
					mcu->freq = 8000000;	/* max 8 MHz */
					break;
				default:
					/* Shouldn't happen! */
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be within [1,4] "
					         "inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
					break;
				}
			} else if ((cksel >= 5U) && (cksel <= 8U)) {
				mcu->clk_source = AVR_EXT_RC_CLK;
				switch (cksel) {
				case 5:
					/* max 0.9 MHz */
					mcu->freq = 900000;
					break;
				case 6:
					/* max 3 MHz */
					mcu->freq = 3000000;
					break;
				case 7:
					/* max 4 MHz */
					mcu->freq = 8000000;
					break;
				case 8:
					/* max 12 MHz */
					mcu->freq = 12000000;
					break;
				default:
					/* Shouldn't happen! */
					snprintf(mcu->log, sizeof mcu->log,
					         "CKSEL = %" PRIu8 ", but it "
					         "should be within [5,8] "
					         "inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
					break;
				}
			} else if (cksel == 9U) {
				/* 32.768 kHz low frequency crystal */
				mcu->clk_source = AVR_EXT_LOWF_CRYSTAL_CLK;
				mcu->freq = 32768;
			} else if ((cksel >= 10U) && (cksel <= 15U)) {
				mcu->clk_source = AVR_EXT_CRYSTAL;
				cksel = (cksel>>1)&0x7U;
				switch (cksel) {
				case 5:
					/* max 0.9 MHz */
					mcu->freq = 900000;
					break;
				case 6:
					/* max 3 MHz */
					mcu->freq = 3000000;
					break;
				case 7:
					/* max 8 MHz */
					mcu->freq = 8000000;
					break;
				default:
					snprintf(mcu->log, sizeof mcu->log,
					         "(CKSEL>>1) = %" PRIu8 ", "
					         "but it should be within "
					         "[5,7] inclusively", cksel);
					MSIM_LOG_ERROR(mcu->log);
					break;
				}
				if (!ckopt) {
					/* max 16 MHz */
					mcu->freq = 16000000;
				}
			} else {
				/* Shouldn't happen! */
				snprintf(mcu->log, sizeof mcu->log, "CKSEL = %"
				         PRIu8 ", but it should be within "
				         "[0,15] inclusively", cksel);
				MSIM_LOG_ERROR(mcu->log);
			}
			break;
		case FUSE_HIGH:
			bootsz = (fuse_v>>1)&0x3U;
			ckopt = (fuse_v>>4)&0x1U;
			switch (bootsz) {
			case 3:
				mcu->bls.start = 0x1F00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 256;
				break;
			case 2:
				mcu->bls.start = 0x1E00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 512;
				break;
			case 1:
				mcu->bls.start = 0x1C00;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 1024;
				break;
			case 0:
				mcu->bls.start = 0x1800;
				mcu->bls.end = 0x1FFF;
				mcu->bls.size = 2048;
				break;
			default:
				/* Shouldn't happen! */
				snprintf(mcu->log, sizeof mcu->log, "BOOTSZ = "
				         "%" PRIu8 ", but it should be within "
				         "[0,3] inclusively", bootsz);
				MSIM_LOG_ERROR(mcu->log);
				break;
			}

			if ((fuse_v&1U) == 1U) {
				/* BOOTRST is 1(unprogrammed) */
				mcu->intr.reset_pc = 0x0000;
				mcu->pc = 0x0000;
			} else {
				/* BOOTRST is 0(programmed) */
				mcu->intr.reset_pc = mcu->bls.start;
				mcu->pc = mcu->bls.start;
			}

			switch (cksel) {
			case 5:
				mcu->freq = 900000;	/* max 0.9 MHz */
				break;
			case 6:
				mcu->freq = 3000000;	/* max 3 MHz */
				break;
			case 7:
				mcu->freq = 8000000;	/* max 8 MHz */
				break;
			default:
				snprintf(mcu->log, sizeof mcu->log, "CKSEL = %"
				         PRIu8 ", but it should be within "
				         "[5,7] inclusively", cksel);
				MSIM_LOG_ERROR(mcu->log);
				break;
			}
			if (!ckopt) {
				mcu->freq = 16000000;	/* max 16 MHz */
			}

			break;
		default:			/* Should not happen */
			snprintf(mcu->log, sizeof mcu->log, "Unknown fuse = %"
			         PRIu32 ", %s will not be modified",
			         fuse_n, mcu->name);
			MSIM_LOG_ERROR(mcu->log);
			ret = 1;
		}
	}
	return ret;
}

int
MSIM_M8ASetLock(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf)
{
	/* Waiting to be implemented */
	return 0;
}
