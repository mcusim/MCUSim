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

/* Reset values and access masks of the ATmega2560's I/O registers. */

#define SREG			_SFR_IO8(0x3F)
#define SREG_RES		0x00
#define SREG_MASK		0xFF
#define SPH			_SFR_IO8(0x3E)
#define SPH_RES			0x21
#define SPH_MASK		0xFF
#define SPL			_SFR_IO8(0x3D)
#define SPL_RES			0xFF
#define SPL_MASK		0xFF

#define UDR3_RES		0x00
#define UDR3_MASK		0xFF
#define UBRR3H_RES		0x00
#define UBRR3H_MASK		0x0F	/* 0000 1111 */
#define UBRR3L_RES		0x00
#define UBRR3L_MASK		0xFF
#define UCSR3C_RES		0x06	/* 0000 0110 */
#define UCSR3C_MASK		0xC7	/* 1100 0111 */
#define UCSR3B_RES		0x06	/* 0000 0110 */
#define UCSR3B_MASK		0xF8	/* 1111 1000 */
#define UCSR3A_RES		0x06	/* 0000 0110 */
#define UCSR3A_MASK		0xE0	/* 1110 0000 */
#define OCR5CH_RES		0x00
#define OCR5CH_MASK		0xFF
#define OCR5CL_RES		0x00
#define OCR5CL_MASK		0xFF
#define OCR5BH_RES		0x00
#define OCR5BH_MASK		0xFF
#define OCR5BL_RES		0x00
#define OCR5BL_MASK		0xFF
#define OCR5AH_RES		0x00
#define OCR5AH_MASK		0xFF
#define OCR5AL_RES		0x00
#define OCR5AL_MASK		0xFF
#define ICR5H_RES		0x00
#define ICR5H_MASK		0xFF
#define ICR5L_RES		0x00
#define ICR5L_MASK		0xFF
#define TCNT5H_RES		0x00
#define TCNT5H_MASK		0xFF
#define TCNT5L_RES		0x00
#define TCNT5L_MASK		0xFF
#define TCCR5C_RES		0x00
#define TCCR5C_MASK		0xE0	/* 1110 0000 */
#define TCCR5B_RES		0x00
#define TCCR5B_MASK		0xDF	/* 1101 1111 */
#define TCCR5A_RES		0x00
#define TCCR5A_MASK		0xFF
#define PORTL_RES		0x00
#define PORTL_MASK		0xFF
#define DDRL_RES		0x00
#define DDRL_MASK		0xFF
#define PINL_RES		0x00
#define PINL_MASK		0xFF
#define PORTK_RES		0x00
#define PORTK_MASK		0xFF
#define DDRK_RES		0x00
#define DDRK_MASK		0xFF
#define PINK_RES		0x00
#define PINK_MASK		0xFF
#define PORTJ_RES		0x00
#define PORTJ_MASK		0xFF
#define DDRJ_RES		0x00
#define DDRJ_MASK		0xFF
#define PINJ_RES		0x00
#define PINJ_MASK		0xFF
#define PORTH_RES		0x00
#define PORTH_MASK		0xFF
#define DDRH_RES		0x00
#define DDRH_MASK		0xFF
#define PINH_RES		0x00
#define PINH_MASK		0xFF
#define UDR1_RES		0x00
#define UDR1_MASK		0xFF
#define UBRR1H_RES		0x00
#define UBRR1H_MASK		0x0F	/* 0000 1111 */
#define UBRR1L_RES		0x00
#define UBRR1L_MASK		0xFF
#define UCSR1C_RES		0x06	/* 0000 0110 */
#define UCSR1C_MASK		0xC7	/* 1100 0111 */
#define UCSR1B_RES		0x06	/* 0000 0110 */
#define UCSR1B_MASK		0xF8	/* 1111 1000 */
#define UCSR1A_RES		0x06	/* 0000 0110 */
#define UCSR1A_MASK		0xE0	/* 1110 0000 */
#define UDR0_RES		0x00
#define UDR0_MASK		0xFF
#define UBRR0H_RES		0x00
#define UBRR0H_MASK		0x0F	/* 0000 1111 */
#define UBRR0L_RES		0x00
#define UBRR0L_MASK		0xFF
#define UCSR0C_RES		0x00
#define UCSR0C_MASK		0xC7	/* 1100 0111 */
#define UCSR0B_RES		0x00
#define UCSR0B_MASK		0xF8	/* 1111 1000 */
#define UCSR0A_RES		0x06	/* 0000 0110 */
#define UCSR0A_MASK		0xF8	/* 1111 1000 */
#define TWAMR_RES		0x00
#define TWAMR_MASK		0xFE	/* 1111 1110 */
#define TWCR_RES		0x00
#define TWCR_MASK		0xF5	/* 1111 0101 */
#define TWDR_RES		0xFF	/* 1111 1111 */
#define TWDR_MASK		0xFF
#define TWAR_RES		0xFE	/* 1111 1110 */
#define TWAR_MASK		0xFF
#define TWSR_RES		0xF8	/* 1111 1000 */
#define TWSR_MASK		0x03	/* 0000 0011 */
#define TWBR_RES		0x00
#define TWBR_MASK		0xFF
#define ASSR_RES		0x00
#define ASSR_MASK		0x60	/* 0110 0000 */
#define OCR2B_RES		0x00
#define OCR2B_MASK		0xFF
#define OCR2A_RES		0x00
#define OCR2A_MASK		0xFF
#define TCNT2_RES		0x00
#define TCNT2_MASK		0xFF
#define TCCR2B_RES		0x00
#define TCCR2B_MASK		0xCF	/* 1100 1111 */
#define TCCR2A_RES		0x00
#define TCCR2A_MASK		0xF3	/* 1111 0011 */
#define OCR4CH_RES		0x00
#define OCR4CH_MASK		0xFF
#define OCR4CL_RES		0x00
#define OCR4CL_MASK		0xFF
#define OCR4BH_RES		0x00
#define OCR4BH_MASK		0xFF
#define OCR4BL_RES		0x00
#define OCR4BL_MASK		0xFF
#define OCR4AH_RES		0x00
#define OCR4AH_MASK		0xFF
#define OCR4AL_RES		0x00
#define OCR4AL_MASK		0xFF
#define ICR4H_RES		0x00
#define ICR4H_MASK		0xFF
#define ICR4L_RES		0x00
#define ICR4L_MASK		0xFF
#define TCNT4H_RES		0x00
#define TCNT4H_MASK		0xFF
#define TCNT4L_RES		0x00
#define TCNT4L_MASK		0xFF
#define TCCR4C_RES		0x00
#define TCCR4C_MASK		0xE0	/* 1110 0000 */
#define TCCR4B_RES		0x00
#define TCCR4B_MASK		0xDF	/* 1101 1111 */
#define TCCR4A_RES		0x00
#define TCCR4A_MASK		0xFF
#define OCR3CH_RES		0x00
#define OCR3CH_MASK		0xFF
#define OCR3CL_RES		0x00
#define OCR3CL_MASK		0xFF
#define OCR3BH_RES		0x00
#define OCR3BH_MASK		0xFF
#define OCR3BL_RES		0x00
#define OCR3BL_MASK		0xFF
#define OCR3AH_RES		0x00
#define OCR3AH_MASK		0xFF
#define OCR3AL_RES		0x00
#define OCR3AL_MASK		0xFF
#define ICR3H_RES		0x00
#define ICR3H_MASK		0xFF
#define ICR3L_RES		0x00
#define ICR3L_MASK		0xFF
#define TCNT3H_RES		0x00
#define TCNT3H_MASK		0xFF
#define TCNT3L_RES		0x00
#define TCNT3L_MASK		0xFF
#define TCCR3C_RES		0x00
#define TCCR3C_MASK		0xE0	/* 1110 0000 */
#define TCCR3B_RES		0x00
#define TCCR3B_MASK		0xDF	/* 1101 1111 */
#define TCCR3A_RES		0x00
#define TCCR3A_MASK		0xFF
#define OCR1CH_RES		0x00
#define OCR1CH_MASK		0xFF
#define OCR1CL_RES		0x00
#define OCR1CL_MASK		0xFF
#define OCR1BH_RES		0x00
#define OCR1BH_MASK		0xFF
#define OCR1BL_RES		0x00
#define OCR1BL_MASK		0xFF
#define OCR1AH_RES		0x00
#define OCR1AH_MASK		0xFF
#define OCR1AL_RES		0x00
#define OCR1AL_MASK		0xFF
#define ICR1H_RES		0x00
#define ICR1H_MASK		0xFF
#define ICR1L_RES		0x00
#define ICR1L_MASK		0xFF
#define TCNT1H_RES		0x00
#define TCNT1H_MASK		0xFF
#define TCNT1L_RES		0x00
#define TCNT1L_MASK		0xFF
#define TCCR1C_RES		0x00
#define TCCR1C_MASK		0xE00	/* 1110 0000 */
#define TCCR1B_RES		0x00
#define TCCR1B_MASK		0xDF	/* 1101 1111 */
#define TCCR1A_RES		0x00
#define TCCR1A_MASK		0xFF
#define DIDR1_RES		0x00
#define DIDR1_MASK		0x03	/* 0000 0011 */
#define DIDR0_RES		0x00
#define DIDR0_MASK		0xFF
#define DIDR2_RES		0x00
#define DIDR2_MASK		0xFF
#define ADMUX_RES		0x00
#define ADMUX_MASK		0xFF
#define ADCSRB_RES		0x00
#define ADCSRB_MASK		0x4F	/* 0100 1111 */
#define ADCSRA_RES		0x00
#define ADCSRA_MASK		0xFF
#define ADCH_RES		0x00
#define ADCH_MASK		0x00
#define ADCL_RES		0x00
#define ADCL_MASK		0x00
#define XMCRB_RES		0x00
#define XMCRB_MASK		0x87	/* 1000 0111 */
#define XMCRA_RES		0x00
#define XMCRA_MASK		0xFF
#define TIMSK5_RES		0x00
#define TIMSK5_MASK		0x2F	/* 0010 1111 */
#define TIMSK4_RES		0x00
#define TIMSK4_MASK		0x2F	/* 0010 1111 */
#define TIMSK3_RES		0x00
#define TIMSK3_MASK		0x2F	/* 0010 1111 */
#define TIMSK2_RES		0x00
#define TIMSK2_MASK		0x07	/* 0000 0111 */
#define TIMSK1_RES		0x00
#define TIMSK1_MASK		0x2F	/* 0010 1111 */
#define TIMSK0_RES		0x00
#define TIMSK0_MASK		0x07	/* 0000 0111 */
#define PCMSK2_RES		0x00
#define PCMSK2_MASK		0xFF
#define PCMSK1_RES		0x00
#define PCMSK1_MASK		0xFF
#define PCMSK0_RES		0x00
#define PCMSK0_MASK		0xFF
#define EICRB_RES		0x00
#define EICRB_MASK		0xFF
#define EICRA_RES		0x00
#define EICRA_MASK		0xFF
#define PCICR_RES		0x00
#define PCICR_MASK		0x07	/* 0000 0111 */
#define OSCCAL_RES		0x00
#define OSCCAL_MASK		0xFF
#define PRR1_RES		0x00
#define PRR1_MASK		0x3F	/* 0011 1111 */
#define PRR0_RES		0x00
#define PRR0_MASK		0xEF	/* 1110 1111 */
#define CLKPR_RES		0x00
#define CLKPR_MASK		0x8F	/* 1000 1111 */
#define WDTCSR_RES		0x00
#define WDTCSR_MASK		0xFF
#define EIND_RES		0x00
#define EIND_MASK		0xFF
#define RAMPZ_RES		0x00
#define RAMPZ_MASK		0xFF
#define SPMCSR_RES		0x00
#define SPMCSR_MASK		0xFF
#define MCUCR_RES		0x00
#define MCUCR_MASK		0x93	/* 1001 0011 */
#define MCUSR_RES		0x00
#define MCUSR_MASK		0x1F	/* 0001 1111 */
#define SMCR_RES		0x00
#define SMCR_MASK		0x0F	/* 0000 1111 */
#define OCDR_RES		0x00
#define OCDR_MASK		0xFF
#define ACSR_RES		0x00
#define ACSR_MASK		0xDF	/* 1101 1111 */
#define SPDR_RES		0x00
#define SPDR_MASK		0xFF
#define SPSR_RES		0x00
#define SPSR_MASK		0x01	/* 0000 0001 */
#define SPCR_RES		0x00
#define SPCR_MASK		0xFF
#define GPIOR2_RES		0x00
#define GPIOR2_MASK		0xFF
#define GPIOR1_RES		0x00
#define GPIOR1_MASK		0xFF
#define OCR0B_RES		0x00
#define OCR0B_MASK		0xFF
#define OCR0A_RES		0x00
#define OCR0A_MASK		0xFF
#define TCNT0_RES		0x00
#define TCNT0_MASK		0xFF
#define TCCR0B_RES		0x00
#define TCCR0B_MASK		0xCF	/* 1100 1111 */
#define TCCR0A_RES		0x00
#define TCCR0A_MASK		0xF3	/* 1111 0011 */
#define GTCCR_RES		0x00
#define GTCCR_MASK		0x83	/* 1000 0011 */
#define EEARH_RES		0x00
#define EEARH_MASK		0x0F	/* 0000 1111 */
#define EEARL_RES		0x00
#define EEARL_MASK		0xFF
#define EEDR_RES		0x00
#define EEDR_MASK		0xFF
#define EECR_RES		0x00
#define EECR_MASK		0x3F	/* 00011 1111 */
#define GPIOR0_RES		0x00
#define GPIOR0_MASK		0xFF
#define EIMSK_RES		0x00
#define EIMSK_MASK		0xFF
#define EIFR_RES		0x00
#define EIFR_MASK		0xFF
#define PCIFR_RES		0x00
#define PCIFR_MASK		0x07	/* 0000 0111 */
#define TIFR5_RES		0x00
#define TIFR5_MASK		0x2F	/* 0010 1111 */
#define TIFR4_RES		0x00
#define TIFR4_MASK		0x2F	/* 0010 1111 */
#define TIFR3_RES		0x00
#define TIFR3_MASK		0x2F	/* 0010 1111 */
#define TIFR2_RES		0x00
#define TIFR2_MASK		0x07	/* 0000 0111 */
#define TIFR1_RES		0x00
#define TIFR1_MASK		0x2F	/* 0010 1111 */
#define TIFR0_RES		0x00
#define TIFR0_MASK		0x07	/* 0000 0111 */
#define PORTG_RES		0x00
#define PORTG_MASK		0x3F	/* 0011 1111 */
#define DDRG_RES		0x00
#define DDRG_MASK		0x3F	/* 0011 1111 */
#define PING_RES		0x00
#define PING_MASK		0x3F	/* 0011 1111 */
#define PORTF_RES		0x00
#define PORTF_MASK		0xFF
#define DDRF_RES		0x00
#define DDRF_MASK		0xFF
#define PINF_RES		0x00
#define PINF_MASK		0xFF
#define PORTE_RES		0x00
#define PORTE_MASK		0xFF
#define DDRE_RES		0x00
#define DDRE_MASK		0xFF
#define PINE_RES		0x00
#define PINE_MASK		0xFF
#define PORTD_RES		0x00
#define PORTD_MASK		0xFF
#define DDRD_RES		0x00
#define DDRD_MASK		0xFF
#define PIND_RES		0x00
#define PIND_MASK		0xFF
#define PORTC_RES		0x00
#define PORTC_MASK		0xFF
#define DDRC_RES		0x00
#define DDRC_MASK		0xFF
#define PINC_RES		0x00
#define PINC_MASK		0xFF
#define PORTB_RES		0x00
#define PORTB_MASK		0xFF
#define DDRB_RES		0x00
#define DDRB_MASK		0xFF
#define PINB_RES		0x00
#define PINB_MASK		0xFF
#define PORTA_RES		0x00
#define PORTA_MASK		0xFF
#define DDRA_RES		0x00
#define DDRA_MASK		0xFF
#define PINA_RES		0x00
#define PINA_MASK		0xFF
