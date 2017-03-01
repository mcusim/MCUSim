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
#ifndef MSIM_AVR_M8A_H_
#define MSIM_AVR_M8A_H_ 1

#include <stdio.h>

#include "avr/sim/sim.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Addresses of the 64 I/O registers */

#define SREG_ADDR		0x3F
#define SPH_ADDR		0x3E
#define SPL_ADDR		0x3D
/*	Reserved		0x3C */
#define GICR_ADDR		0x3B
#define GIFR_ADDR		0x3A
#define TIMSK_ADDR		0x39
#define TIFR_ADDR		0x38
#define SPMCR_ADDR		0x37
#define TWCR_ADDR		0x36
#define MCUCR_ADDR		0x35
#define MCUCSR_ADDR		0x34
#define TCCR0_ADDR		0x33
#define TCNT0_ADDR		0x32	/* Timer/Counter0 (8 Bits) */
#define OSCCAL_ADDR		0x31	/* Oscillator Calibration Register */
#define SFIOR_ADDR		0x30
#define TCCR1A_ADDR		0x2F
#define TCCR1B_ADDR		0x2E
#define TCNT1H_ADDR		0x2D	/* Timer/Counter1 – Counter Register
					   High byte */
#define TCNT1L_ADDR		0x2C	/* Timer/Counter1 – Counter Register
					   Low byte */
#define OCR1AH_ADDR		0x2B	/* Timer/Counter1 – Output Compare
					   Register A High byte */
#define OCR1AL_ADDR		0x2A	/* Timer/Counter1 – Output Compare
					   Register A Low byte */
#define OCR1BH_ADDR		0x29	/* Timer/Counter1 – Output Compare
					   Register B High byte */
#define OCR1BL_ADDR		0x28	/* Timer/Counter1 – Output Compare
					   Register B Low byte */
#define ICR1H_ADDR		0x27	/* Timer/Counter1 – Input Capture
					   Register High byte */
#define ICR1L_ADDR		0x26	/* Timer/Counter1 – Input Capture
					   Register Low byte */
#define TCCR2_ADDR		0x25
#define TCNT2_ADDR		0x24	/* Timer/Counter2 (8 Bits) */
#define OCR2_ADDR		0x23	/* Timer/Counter2 Output Compare
					   Register */
#define ASSR_ADDR		0x22
#define WDTCR_ADDR		0x21
#define UBRRH_ADDR		0x20	/* Refer to the USART description
					   for details on how to access
					   UBRRH and UCSRC. */
#define UCSRC_ADDR		0x20
#define EEARH_ADDR		0x1F
#define EEARL_ADDR		0x1E
#define EEDR_ADDR		0x1D	/* EEPROM Data Register */
#define EECR_ADDR		0x1C
/*	Reserved		0x1B */
/*	Reserved		0x1A */
/*	Reserved		0x19 */
#define PORTB_ADDR		0x18
#define DDRB_ADDR		0x17
#define PINB_ADDR		0x16
#define PORTC_ADDR		0x15
#define DDRC_ADDR		0x14
#define PINC_ADDR		0x13
#define PORTD_ADDR		0x12
#define DDRD_ADDR		0x11
#define PIND_ADDR		0x10
#define SPDR_ADDR		0x0F	/* SPI Data Register */
#define SPSR_ADDR		0x0E
#define SPCR_ADDR		0x0D
#define UDR_ADDR		0x0C	/* USART I/O Data Register */
#define UCSRA_ADDR		0x0B
#define UCSRB_ADDR		0x0A
#define UBRRL_ADDR		0x09	/* USART Baud Rate Register Low byte */
#define ACSR_ADDR		0x08
#define ADMUX_ADDR		0x07
#define ADCSRA_ADDR		0x06
#define ADCH_ADDR		0x05	/* ADC Data Register High byte */
#define ADCL_ADDR		0x04	/* ADC Data Register Low byte */
#define TWDR_ADDR		0x03	/* Two-wire Serial Interface Data
					   Register */
#define TWAR_ADDR		0x02
#define TWSR_ADDR		0x01
#define TWBR_ADDR		0x00	/* Two-wire Serial Interface Bit
					   Rate Register */

/* Public prototypes */

int m8a_init(struct avr *mcu, uint16_t *pm, uint32_t pm_size,
			      uint8_t *dm, uint32_t dm_size);

int m8a_load_progmem(struct avr *mcu, FILE *fp);

/* END Public prototypes */

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_M8A_H_ */
