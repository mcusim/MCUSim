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

/* Reset values and access masks of the ATmega8/8A's I/O registers. */

#define WDTON		6	/* Fuse bit */

#define SREG		_SFR_IO8(0x3F)
#define SREG_RES	0x00
#define SREG_MASK	0xFF
#define SPH		_SFR_IO8(0x3E)
#define SPH_RES		0x00
#define SPH_MASK	0x07 /* 0000 0111 */
#define SPL		_SFR_IO8(0x3D)
#define SPL_RES		0x00
#define SPL_MASK	0xFF

#define PORTB_RES	0x00
#define PORTB_MASK	0xFF
#define DDRB_RES	0x00
#define DDRB_MASK	0xFF
#define PINB_RES	0x00
#define PINB_MASK	0x00
#define PORTC_RES	0x00
#define PORTC_MASK	0x7F /* 0111 1111 */
#define DDRC_RES	0x00
#define DDRC_MASK	0x7F /* 0111 1111 */
#define PINC_RES	0x00
#define PINC_MASK	0x00
#define PORTD_RES	0x00
#define PORTD_MASK	0xFF
#define DDRD_RES	0x00
#define DDRD_MASK	0xFF
#define PIND_RES	0x00
#define PIND_MASK	0x00
#define MCUCR_RES	0x00
#define MCUCR_MASK	0xFF
#define ACSR_RES	0x00
#define ACSR_MASK	0xDF /* 1101 1111 */
#define SPDR_RES	0x00
#define SPDR_MASK	0xFF
#define MCUCSR_RES	0x00
#define MCUCSR_MASK	0x0F /* 0000 1111 */
#define SPSR_RES	0x00
#define SPSR_MASK	0x01 /* 0000 0001 */
#define SPCR_RES	0x00
#define SPCR_MASK	0xFF
#define TCNT0_RES	0x00
#define TCNT0_MASK	0xFF
#define EEARH_RES	0x00
#define EEARH_MASK	0x01 /* 0000 0001 */
#define EEARL_RES	0x00
#define EEARL_MASK	0xFF
#define EEDR_RES	0x00
#define EEDR_MASK	0xFF
#define EECR_RES	0x00
#define EECR_MASK	0x0F /* 0000 1111 */
#define TIFR_RES	0x00
#define TIFR_MASK	0xFD /* 1111 1101 */
#define GICR_RES	0x00
#define GICR_MASK	0xC3 /* 1100 0011 */
#define GIFR_RES	0x00
#define GIFR_MASK	0xC0 /* 1100 0000 */
#define TIMSK_RES	0x00
#define TIMSK_MASK	0xFD /* 1111 1101 */
#define SPMCR_RES	0x00
#define SPMCR_MASK	0x9F /* 1001 1111 */
#define TWCR_RES	0x00
#define TWCR_MASK	0xFD /* 1111 1101 */
#define OSCCAL_RES	0x00
#define OSCCAL_MASK	0xFF
#define SFIOR_RES	0x00
#define SFIOR_MASK	0x0F /* 0000 1111 */
#define TCCR0_RES	0x00
#define TCCR0_MASK	0x07 /* 0000 0111 */
#define TCCR1A_RES	0x00
#define TCCR1A_MASK	0xFF
#define TCCR1B_RES	0x00
#define TCCR1B_MASK	0xDF
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
#define TCCR2_RES	0x00
#define TCCR2_MASK	0xFF
#define TCNT2_RES	0x00
#define TCNT2_MASK	0xFF
#define OCR2_RES	0x00
#define OCR2_MASK	0xFF
#define ASSR_RES	0x00
#define ASSR_MASK	0x0F /* 0000 1111 */
#define WDTCR_RES	0x00
#define WDTCR_MASK	0x1F /* 0001 1111 */
#define UBRRH_RES	0x00
#define UBRRH_MASK	0x8F /* 1000 1111 */
#define UCSRC_RES	0x82
#define UCSRC_MASK	0xFF
#define UDR_RES		0x00
#define UDR_MASK	0xFF
#define UCSRA_RES	0x20
#define UCSRA_MASK	0x43 /* 0100 0011 */
#define UCSRB_RES	0x00
#define UCSRB_MASK	0xFD /* 1111 1101 */
#define UBRRL_RES	0x00
#define UBRRL_MASK	0xFF
#define ADMUX_RES	0x00
#define ADMUX_MASK	0xEF /* 1110 1111 */
#define ADCSRA_RES	0x00
#define ADCSRA_MASK	0xFF
#define ADCH_RES	0x00
#define ADCH_MASK	0x00
#define ADCL_RES	0x00
#define ADCL_MASK	0x00
#define TWDR_RES	0x01
#define TWDR_MASK	0xFF
#define TWAR_RES	0x02
#define TWAR_MASK	0xFF
#define TWSR_RES	0x08
#define TWSR_MASK	0x03 /* 0000 0011 */
#define TWBR_RES	0x00
#define TWBR_MASK	0xFF
