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

/* Reset values and access masks of the ATmega328/328P's I/O registers. */

#define WDTON		4	/* Fuse bit */

#define SREG		_SFR_IO8(0x3F)
#define SREG_RES	0x00
#define SREG_MASK	0xFF
#define SPH		_SFR_IO8(0x3E)
#define SPH_RES		0x00
#define SPH_MASK	0xFF
#define SPL		_SFR_IO8(0x3D)
#define SPL_RES		0x00
#define SPL_MASK	0xFF

#define PORTB_RES	0x00
#define PORTB_MASK	0xFF
#define DDRB_RES	0x00
#define DDRB_MASK	0xFF
#define PINB_RES	0x00
#define PINB_MASK	0xFF /* See 14.2.2 Toggling the Pin */
#define PORTC_RES	0x00
#define PORTC_MASK	0x7F /* 0111 1111 */
#define DDRC_RES	0x00
#define DDRC_MASK	0x7F /* 0111 1111 */
#define PINC_RES	0x00
#define PINC_MASK	0x7F /* See 14.2.2 Toggling the Pin */
#define PORTD_RES	0x00
#define PORTD_MASK	0xFF
#define DDRD_RES	0x00
#define DDRD_MASK	0xFF
#define PIND_RES	0x00
#define PIND_MASK	0xFF /* See 14.2.2 Toggling the Pin */
#define SPMCSR_RES	0x00
#define SPMCSR_MASK	0xBF /* 1011 1111 */
#define MCUCR_RES	0x00
#define MCUCR_MASK	0x73 /* 0111 0011 */
#define MCUSR_RES	0x02 /* 0000 0010 - External reset by default */
#define MCUSR_MASK	0x0F /* 0000 1111 */
#define SMCR_RES	0x00
#define SMCR_MASK	0x0F /* 0000 1111 */
#define ACSR_RES	0x00
#define ACSR_MASK	0xDF /* 1101 1111 */
#define SPDR_RES	0x00
#define SPDR_MASK	0xFF
#define SPSR_RES	0x00
#define SPSR_MASK	0x01 /* 0000 0001 */
#define SPCR_RES	0x00
#define SPCR_MASK	0xFF
#define GPIOR2_RES	0x00
#define GPIOR2_MASK	0xFF
#define GPIOR1_RES	0x00
#define GPIOR1_MASK	0xFF
#define GPIOR0_RES	0x00
#define GPIOR0_MASK	0xFF
#define OCR0B_RES	0x00
#define OCR0B_MASK	0xFF
#define OCR0A_RES	0x00
#define OCR0A_MASK	0xFF
#define TCNT0_RES	0x00
#define TCNT0_MASK	0xFF
#define TCCR0B_RES	0x00
#define TCCR0B_MASK	0xCF /* 1100 1111 */
#define TCCR0A_RES	0x00
#define TCCR0A_MASK	0xF3 /* 1111 0011 */
#define GTCCR_RES	0x00
#define GTCCR_MASK	0x83 /* 1000 0011 */
#define EEARH_RES	0x00
#define EEARH_MASK	0x03 /* 0000 0011 - It looks like a mistake in
				datasheet. EEAR9 bit should be read/written as
				a part of address. */
#define EEARL_RES	0x00
#define EEARL_MASK	0xFF
#define EEDR_RES	0x00
#define EEDR_MASK	0xFF
#define EECR_RES	0x00
#define EECR_MASK	0x3F /* 0011 1111 */
#define EIMSK_RES	0x00
#define EIMSK_MASK	0x03 /* 0000 0011 */
#define EIFR_RES	0x00
#define EIFR_MASK	0x03 /* 0000 0011 */
#define PCIFR_RES	0x00
#define PCIFR_MASK	0x07 /* 0000 0111 */
#define TIFR2_RES	0x00
#define TIFR2_MASK	0x07 /* 0000 0111 */
#define TIFR1_RES	0x00
#define TIFR1_MASK	0x27 /* 0010 0111 */
#define TIFR0_RES	0x00
#define TIFR0_MASK	0x07 /* 0000 0111 */
#define TWCR_RES	0x00
#define TWCR_MASK	0xF5 /* 1111 0101 */
#define OSCCAL_RES	0x00
#define OSCCAL_MASK	0xFF
#define TCCR1A_RES	0x00
#define TCCR1A_MASK	0xF3 /* 1111 0011 */
#define TCCR1B_RES	0x00
#define TCCR1B_MASK	0xDF /* 1101 1111 */
#define TCNT1H_RES	0x00
#define TCNT1H_MASK	0xFF
#define TCNT1L_RES	0x00
#define TCNT1L_MASK	0xFF
#define OCR1AH_RES	0x00
#define OCR1AH_MASK	0xFF
#define OCR1AL_RES	0x00
#define OCR1AL_MASK	0xFF
#define OCR1BH_RES	0x00
#define OCR1BH_MASK	0xFF
#define OCR1BL_RES	0x00
#define OCR1BL_MASK	0xFF
#define ICR1H_RES	0x00
#define ICR1H_MASK	0xFF
#define ICR1L_RES	0x00
#define ICR1L_MASK	0xFF
#define TCNT2_RES	0x00
#define TCNT2_MASK	0xFF
#define ASSR_RES	0x00
#define ASSR_MASK	0x60 /* 0110 0000 */
#define ADMUX_RES	0x00
#define ADMUX_MASK	0xEF /* 1110 1111 */
#define ADCSRA_RES	0x00
#define ADCSRA_MASK	0xFF
#define ADCH_RES	0x00
#define ADCH_MASK	0x00 /* 0000 0000 */
#define ADCL_RES	0x00
#define ADCL_MASK	0x00 /* 0000 0000 */
#define TWDR_RES	0x00
#define TWDR_MASK	0xFF
#define TWAR_RES	0x01
#define TWAR_MASK	0xFF
#define TWSR_RES	0xF8 /* 1111 1000 */
#define TWSR_MASK	0x03 /* 0000 0011 */
#define TWBR_RES	0x00
#define TWBR_MASK	0xFF
#define UDR0_RES	0x00
#define UDR0_MASK	0xFF
#define UBRR0H_RES	0x00
#define UBRR0H_MASK	0x0F /* 0000 1111 */
#define UBRR0L_RES	0x00
#define UBRR0L_MASK	0xFF
#define UCSR0C_RES	0x06 /* 0000 0110 */
#define UCSR0C_MASK	0xFF
#define UCSR0B_RES	0x00
#define UCSR0B_MASK	0xFD /* 1111 1101 */
#define UCSR0A_RES	0x20 /* 0010 0000 */
#define UCSR0A_MASK	0x43 /* 0100 0011 */
#define TWAMR_RES	0x00
#define TWAMR_MASK	0xFE /* 1111 1110 */
#define OCR2B_RES	0x00
#define OCR2B_MASK	0xFF
#define OCR2A_RES	0x00
#define OCR2A_MASK	0xFF
#define TCCR2B_RES	0x00
#define TCCR2B_MASK	0xCF /* 1100 1111 */
#define TCCR2A_RES	0x00
#define TCCR2A_MASK	0xF3 /* 1111 0011 */
#define TCCR1C_RES	0x00
#define TCCR1C_MASK	0xC0 /* 1100 0000 */
#define DIDR1_RES	0x00
#define DIDR1_MASK	0x03 /* 0000 0011 */
#define DIDR0_RES	0x00
#define DIDR0_MASK	0x3F /* 0011 1111 */
#define ADCSRB_RES	0x00
#define ADCSRB_MASK	0x47 /* 0100 0111 */
#define TIMSK2_RES	0x00
#define TIMSK2_MASK	0x07 /* 0000 0111 */
#define TIMSK1_RES	0x00
#define TIMSK1_MASK	0x27 /* 0010 0111 */
#define TIMSK0_RES	0x00
#define TIMSK0_MASK	0x07 /* 0000 0111 */
#define PCMSK2_RES	0x00
#define PCMSK2_MASK	0xFF
#define PCMSK1_RES	0x00
#define PCMSK1_MASK	0x7F /* 0111 1111 */
#define PCMSK0_RES	0x00
#define PCMSK0_MASK	0xFF
#define EICRA_RES	0x00
#define EICRA_MASK	0x0F /* 0000 1111 */
#define PCICR_RES	0x00
#define PCICR_MASK	0x07 /* 0000 0111 */
#define CLKPR_RES	0x03 /* 0000 0011 */
#define CLKPR_MASK	0x8F /* 1000 1111 */
#define WDTCSR_RES	0x08 /* 0000 1000 */
#define WDTCSR_MASK	0xFF
#define PRR_RES		0x00
#define PRR_MASK	0xEF /* 1110 1111 */
