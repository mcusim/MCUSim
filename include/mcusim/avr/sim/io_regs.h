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

/*
 * Macro definitions of the AVR I/O registers which compilement existing ones
 * from mcusim/avr/io.h.
 */

/* NOTE: Memory offset of unavailable I/O registers marked as NO_REG. */
#define NO_REG		-1

#ifndef SREG
	#define SREG		NO_REG
	#define SREG_RES	0x00
	#define SREG_MASK	0x00
#endif
#ifndef SPH
	#define SPH		NO_REG
	#define SPH_RES		0x00
	#define SPH_MASK	0x00
#endif
#ifndef SPL
	#define SPL		NO_REG
	#define SPL_RES		0x00
	#define SPL_MASK	0x00
#endif
#ifndef PORTA
	#define PORTA		NO_REG
	#define PORTA_RES	0x00
	#define PORTA_MASK	0x00
#endif
#ifndef DDRA
	#define DDRA		NO_REG
	#define DDRA_RES	0x00
	#define DDRA_MASK	0x00
#endif
#ifndef PINA
	#define PINA		NO_REG
	#define PINA_RES	0x00
	#define PINA_MASK	0x00
#endif
#ifndef PORTB
	#define PORTB		NO_REG
	#define PORTB_RES	0x00
	#define PORTB_MASK	0x00
#endif
#ifndef DDRB
	#define DDRB		NO_REG
	#define DDRB_RES	0x00
	#define DDRB_MASK	0x00
#endif
#ifndef PINB
	#define PINB		NO_REG
	#define PINB_RES	0x00
	#define PINB_MASK	0x00
#endif
#ifndef PORTC
	#define PORTC		NO_REG
	#define PORTC_RES	0x00
	#define PORTC_MASK	0x00
#endif
#ifndef DDRC
	#define DDRC		NO_REG
	#define DDRC_RES	0x00
	#define DDRC_MASK	0x00
#endif
#ifndef PINC
	#define PINC		NO_REG
	#define PINC_RES	0x00
	#define PINC_MASK	0x00
#endif
#ifndef PORTD
	#define PORTD		NO_REG
	#define PORTD_RES	0x00
	#define PORTD_MASK	0x00
#endif
#ifndef DDRD
	#define DDRD		NO_REG
	#define DDRD_RES	0x00
	#define DDRD_MASK	0x00
#endif
#ifndef PIND
	#define PIND		NO_REG
	#define PIND_RES	0x00
	#define PIND_MASK	0x00
#endif
#ifndef PORTE
	#define PORTE		NO_REG
	#define PORTE_RES	0x00
	#define PORTE_MASK	0x00
#endif
#ifndef DDRE
	#define DDRE		NO_REG
	#define DDRE_RES	0x00
	#define DDRE_MASK	0x00
#endif
#ifndef PINE
	#define PINE		NO_REG
	#define PINE_RES	0x00
	#define PINE_MASK	0x00
#endif
#ifndef PORTF
	#define PORTF		NO_REG
	#define PORTF_RES	0x00
	#define PORTF_MASK	0x00
#endif
#ifndef DDRF
	#define DDRF		NO_REG
	#define DDRF_RES	0x00
	#define DDRF_MASK	0x00
#endif
#ifndef PINF
	#define PINF		NO_REG
	#define PINF_RES	0x00
	#define PINF_MASK	0x00
#endif
#ifndef PORTG
	#define PORTG		NO_REG
	#define PORTG_RES	0x00
	#define PORTG_MASK	0x00
#endif
#ifndef DDRG
	#define DDRG		NO_REG
	#define DDRG_RES	0x00
	#define DDRG_MASK	0x00
#endif
#ifndef PING
	#define PING		NO_REG
	#define PING_RES	0x00
	#define PING_MASK	0x00
#endif
#ifndef EIND
	#define EIND		NO_REG
	#define EIND_RES	0x00
	#define EIND_MASK	0x00
#endif
#ifndef RAMPZ
	#define RAMPZ		NO_REG
	#define RAMPZ_RES	0x00
	#define RAMPZ_MASK	0x00
#endif
#ifndef RAMPY
	#define RAMPY		NO_REG
	#define RAMPY_RES	0x00
	#define RAMPY_MASK	0x00
#endif
#ifndef RAMPX
	#define RAMPX		NO_REG
	#define RAMPX_RES	0x00
	#define RAMPX_MASK	0x00
#endif
#ifndef RAMPD
	#define RAMPD		NO_REG
	#define RAMPD_RES	0x00
	#define RAMPD_MASK	0x00
#endif
#ifndef SPMCSR
	#define SPMCSR		NO_REG
	#define SPMCSR_RES	0x00
	#define SPMCSR_MASK	0x00
#endif
#ifndef MCUCR
	#define MCUCR		NO_REG
	#define MCUCR_RES	0x00
	#define MCUCR_MASK	0x00
#endif
#ifndef MCUSR
	#define MCUSR		NO_REG
	#define MCUSR_RES	0x00
	#define MCUSR_MASK	0x00
#endif
#ifndef SMCR
	#define SMCR		NO_REG
	#define SMCR_RES	0x00
	#define SMCR_MASK	0x00
#endif
#ifndef OCDR
	#define OCDR		NO_REG
	#define OCDR_RES	0x00
	#define OCDR_MASK	0x00
#endif
#ifndef ACSR
	#define ACSR		NO_REG
	#define ACSR_RES	0x00
	#define ACSR_MASK	0x00
#endif
#ifndef SPDR
	#define SPDR		NO_REG
	#define SPDR_RES	0x00
	#define SPDR_MASK	0x00
#endif
#ifndef SPSR
	#define SPSR		NO_REG
	#define SPSR_RES	0x00
	#define SPSR_MASK	0x00
#endif
#ifndef SPCR
	#define SPCR		NO_REG
	#define SPCR_RES	0x00
	#define SPCR_MASK	0x00
#endif
#ifndef GPIOR2
	#define GPIOR2		NO_REG
	#define GPIOR2_RES	0x00
	#define GPIOR2_MASK	0x00
#endif
#ifndef GPIOR1
	#define GPIOR1		NO_REG
	#define GPIOR1_RES	0x00
	#define GPIOR1_MASK	0x00
#endif
#ifndef GPIOR0
	#define GPIOR0		NO_REG
	#define GPIOR0_RES	0x00
	#define GPIOR0_MASK	0x00
#endif
#ifndef OCR0B
	#define OCR0B		NO_REG
	#define OCR0B_RES	0x00
	#define OCR0B_MASK	0x00
#endif
#ifndef OCR0A
	#define OCR0A		NO_REG
	#define OCR0A_RES	0x00
	#define OCR0A_MASK	0x00
#endif
#ifndef TCNT0
	#define TCNT0		NO_REG
	#define TCNT0_RES	0x00
	#define TCNT0_MASK	0x00
#endif
#ifndef TCCR0B
	#define TCCR0B		NO_REG
	#define TCCR0B_RES	0x00
	#define TCCR0B_MASK	0x00
#endif
#ifndef TCCR0A
	#define TCCR0A		NO_REG
	#define TCCR0A_RES	0x00
	#define TCCR0A_MASK	0x00
#endif
#ifndef TCCR0
	#define TCCR0		NO_REG
	#define TCCR0_RES	0x00
	#define TCCR0_MASK	0x00
#endif
#ifndef GTCCR
	#define GTCCR		NO_REG
	#define GTCCR_RES	0x00
	#define GTCCR_MASK	0x00
#endif
#ifndef EEARH
	#define EEARH		NO_REG
	#define EEARH_RES	0x00
	#define EEARH_MASK	0x00
#endif
#ifndef EEARL
	#define EEARL		NO_REG
	#define EEARL_RES	0x00
	#define EEARL_MASK	0x00
#endif
#ifndef EEDR
	#define EEDR		NO_REG
	#define EEDR_RES	0x00
	#define EEDR_MASK	0x00
#endif
#ifndef EECR
	#define EECR		NO_REG
	#define EECR_RES	0x00
	#define EECR_MASK	0x00
#endif
#ifndef EIMSK
	#define EIMSK		NO_REG
	#define EIMSK_RES	0x00
	#define EIMSK_MASK	0x00
#endif
#ifndef EIFR
	#define EIFR		NO_REG
	#define EIFR_RES	0x00
	#define EIFR_MASK	0x00
#endif
#ifndef PCIFR
	#define PCIFR		NO_REG
	#define PCIFR_RES	0x00
	#define PCIFR_MASK	0x00
#endif
#ifndef TIFR5
	#define TIFR5		NO_REG
	#define TIFR5_RES	0x00
	#define TIFR5_MASK	0x00
#endif
#ifndef TIFR4
	#define TIFR4		NO_REG
	#define TIFR4_RES	0x00
	#define TIFR4_MASK	0x00
#endif
#ifndef TIFR3
	#define TIFR3		NO_REG
	#define TIFR3_RES	0x00
	#define TIFR3_MASK	0x00
#endif
#ifndef TIFR2
	#define TIFR2		NO_REG
	#define TIFR2_RES	0x00
	#define TIFR2_MASK	0x00
#endif
#ifndef TIFR1
	#define TIFR1		NO_REG
	#define TIFR1_RES	0x00
	#define TIFR1_MASK	0x00
#endif
#ifndef TIFR0
	#define TIFR0		NO_REG
	#define TIFR0_RES	0x00
	#define TIFR0_MASK	0x00
#endif
#ifndef TIFR
	#define TIFR		NO_REG
	#define TIFR_RES	0x00
	#define TIFR_MASK	0x00
#endif
#ifndef GICR
	#define GICR		NO_REG
	#define GICR_RES	0x00
	#define GICR_MASK	0x00
#endif
#ifndef GIFR
	#define GIFR		NO_REG
	#define GIFR_RES	0x00
	#define GIFR_MASK	0x00
#endif
#ifndef TIMSK
	#define TIMSK		NO_REG
	#define TIMSK_RES	0x00
	#define TIMSK_MASK	0x00
#endif
#ifndef SPMCR
	#define SPMCR		NO_REG
	#define SPMCR_RES	0x00
	#define SPMCR_MASK	0x00
#endif
#ifndef TWCR
	#define TWCR		NO_REG
	#define TWCR_RES	0x00
	#define TWCR_MASK	0x00
#endif
#ifndef MCUCSR
	#define MCUCSR		NO_REG
	#define MCUCSR_RES	0x00
	#define MCUCSR_MASK	0x00
#endif
#ifndef OSCCAL
	#define OSCCAL		NO_REG
	#define OSCCAL_RES	0x00
	#define OSCCAL_MASK	0x00
#endif
#ifndef SFIOR
	#define SFIOR		NO_REG
	#define SFIOR_RES	0x00
	#define SFIOR_MASK	0x00
#endif
#ifndef TCCR1A
	#define TCCR1A		NO_REG
	#define TCCR1A_RES	0x00
	#define TCCR1A_MASK	0x00
#endif
#ifndef TCCR1B
	#define TCCR1B		NO_REG
	#define TCCR1B_RES	0x00
	#define TCCR1B_MASK	0x00
#endif
#ifndef TCNT1H
	#define TCNT1H		NO_REG
	#define TCNT1H_RES	0x00
	#define TCNT1H_MASK	0x00
#endif
#ifndef TCNT1L
	#define TCNT1L		NO_REG
	#define TCNT1L_RES	0x00
	#define TCNT1L_MASK	0x00
#endif
#ifndef OCR1AH
	#define OCR1AH		NO_REG
	#define OCR1AH_RES	0x00
	#define OCR1AH_MASK	0x00
#endif
#ifndef OCR1AL
	#define OCR1AL		NO_REG
	#define OCR1AL_RES	0x00
	#define OCR1AL_MASK	0x00
#endif
#ifndef OCR1BH
	#define OCR1BH		NO_REG
	#define OCR1BH_RES	0x00
	#define OCR1BH_MASK	0x00
#endif
#ifndef OCR1BL
	#define OCR1BL		NO_REG
	#define OCR1BL_RES	0x00
	#define OCR1BL_MASK	0x00
#endif
#ifndef ICR1H
	#define ICR1H		NO_REG
	#define ICR1H_RES	0x00
	#define ICR1H_MASK	0x00
#endif
#ifndef ICR1L
	#define ICR1L		NO_REG
	#define ICR1L_RES	0x00
	#define ICR1L_MASK	0x00
#endif
#ifndef TCCR2
	#define TCCR2		NO_REG
	#define TCCR2_RES	0x00
	#define TCCR2_MASK	0x00
#endif
#ifndef TCNT2
	#define TCNT2		NO_REG
	#define TCNT2_RES	0x00
	#define TCNT2_MASK	0x00
#endif
#ifndef OCR2
	#define OCR2		NO_REG
	#define OCR2_RES	0x00
	#define OCR2_MASK	0x00
#endif
#ifndef ASSR
	#define ASSR		NO_REG
	#define ASSR_RES	0x00
	#define ASSR_MASK	0x00
#endif
#ifndef WDTCR
	#define WDTCR		NO_REG
	#define WDTCR_RES	0x00
	#define WDTCR_MASK	0x00
#endif
#ifndef UBRRH
	#define UBRRH		NO_REG
	#define UBRRH_RES	0x00
	#define UBRRH_MASK	0x00
#endif
#ifndef UCSRC
	#define UCSRC		NO_REG
	#define UCSRC_RES	0x00
	#define UCSRC_MASK	0x00
#endif
#ifndef UDR
	#define UDR		NO_REG
	#define UDR_RES	0x00
	#define UDR_MASK	0x00
#endif
#ifndef UCSRA
	#define UCSRA		NO_REG
	#define UCSRA_RES	0x00
	#define UCSRA_MASK	0x00
#endif
#ifndef UCSRB
	#define UCSRB		NO_REG
	#define UCSRB_RES	0x00
	#define UCSRB_MASK	0x00
#endif
#ifndef UBRRL
	#define UBRRL		NO_REG
	#define UBRRL_RES	0x00
	#define UBRRL_MASK	0x00
#endif
#ifndef ADMUX
	#define ADMUX		NO_REG
	#define ADMUX_RES	0x00
	#define ADMUX_MASK	0x00
#endif
#ifndef ADCSRA
	#define ADCSRA		NO_REG
	#define ADCSRA_RES	0x00
	#define ADCSRA_MASK	0x00
#endif
#ifndef ADCH
	#define ADCH		NO_REG
	#define ADCH_RES	0x00
	#define ADCH_MASK	0x00
#endif
#ifndef ADCL
	#define ADCL		NO_REG
	#define ADCL_RES	0x00
	#define ADCL_MASK	0x00
#endif
#ifndef TWDR
	#define TWDR		NO_REG
	#define TWDR_RES	0x00
	#define TWDR_MASK	0x00
#endif
#ifndef TWAR
	#define TWAR		NO_REG
	#define TWAR_RES	0x00
	#define TWAR_MASK	0x00
#endif
#ifndef TWSR
	#define TWSR		NO_REG
	#define TWSR_RES	0x00
	#define TWSR_MASK	0x00
#endif
#ifndef TWBR
	#define TWBR		NO_REG
	#define TWBR_RES	0x00
	#define TWBR_MASK	0x00
#endif
#ifndef UDR3
	#define UDR3		NO_REG
	#define UDR3_RES	0x00
	#define UDR3_MASK	0x00
#endif
#ifndef UBRR3H
	#define UBRR3H		NO_REG
	#define UBRR3H_RES	0x00
	#define UBRR3H_MASK	0x00
#endif
#ifndef UBRR3L
	#define UBRR3L		NO_REG
	#define UBRR3L_RES	0x00
	#define UBRR3L_MASK	0x00
#endif
#ifndef UCSR3C
	#define UCSR3C		NO_REG
	#define UCSR3C_RES	0x00
	#define UCSR3C_MASK	0x00
#endif
#ifndef UCSR3B
	#define UCSR3B		NO_REG
	#define UCSR3B_RES	0x00
	#define UCSR3B_MASK	0x00
#endif
#ifndef UCSR3A
	#define UCSR3A		NO_REG
	#define UCSR3A_RES	0x00
	#define UCSR3A_MASK	0x00
#endif
#ifndef OCR5CH
	#define OCR5CH		NO_REG
	#define OCR5CH_RES	0x00
	#define OCR5CH_MASK	0x00
#endif
#ifndef OCR5CL
	#define OCR5CL		NO_REG
	#define OCR5CL_RES	0x00
	#define OCR5CL_MASK	0x00
#endif
#ifndef OCR5BH
	#define OCR5BH		NO_REG
	#define OCR5BH_RES	0x00
	#define OCR5BH_MASK	0x00
#endif
#ifndef OCR5BL
	#define OCR5BL		NO_REG
	#define OCR5BL_RES	0x00
	#define OCR5BL_MASK	0x00
#endif
#ifndef OCR5AH
	#define OCR5AH		NO_REG
	#define OCR5AH_RES	0x00
	#define OCR5AH_MASK	0x00
#endif
#ifndef OCR5AL
	#define OCR5AL		NO_REG
	#define OCR5AL_RES	0x00
	#define OCR5AL_MASK	0x00
#endif
#ifndef ICR5H
	#define ICR5H		NO_REG
	#define ICR5H_RES	0x00
	#define ICR5H_MASK	0x00
#endif
#ifndef ICR5L
	#define ICR5L		NO_REG
	#define ICR5L_RES	0x00
	#define ICR5L_MASK	0x00
#endif
#ifndef TCNT5H
	#define TCNT5H		NO_REG
	#define TCNT5H_RES	0x00
	#define TCNT5H_MASK	0x00
#endif
#ifndef TCNT5L
	#define TCNT5L		NO_REG
	#define TCNT5L_RES	0x00
	#define TCNT5L_MASK	0x00
#endif
#ifndef TCCR5C
	#define TCCR5C		NO_REG
	#define TCCR5C_RES	0x00
	#define TCCR5C_MASK	0x00
#endif
#ifndef TCCR5B
	#define TCCR5B		NO_REG
	#define TCCR5B_RES	0x00
	#define TCCR5B_MASK	0x00
#endif
#ifndef TCCR5A
	#define TCCR5A		NO_REG
	#define TCCR5A_RES	0x00
	#define TCCR5A_MASK	0x00
#endif
#ifndef PORTL
	#define PORTL		NO_REG
	#define PORTL_RES	0x00
	#define PORTL_MASK	0x00
#endif
#ifndef DDRL
	#define DDRL		NO_REG
	#define DDRL_RES	0x00
	#define DDRL_MASK	0x00
#endif
#ifndef PINL
	#define PINL		NO_REG
	#define PINL_RES	0x00
	#define PINL_MASK	0x00
#endif
#ifndef PORTK
	#define PORTK		NO_REG
	#define PORTK_RES	0x00
	#define PORTK_MASK	0x00
#endif
#ifndef DDRK
	#define DDRK		NO_REG
	#define DDRK_RES	0x00
	#define DDRK_MASK	0x00
#endif
#ifndef PINK
	#define PINK		NO_REG
	#define PINK_RES	0x00
	#define PINK_MASK	0x00
#endif
#ifndef PORTJ
	#define PORTJ		NO_REG
	#define PORTJ_RES	0x00
	#define PORTJ_MASK	0x00
#endif
#ifndef DDRJ
	#define DDRJ		NO_REG
	#define DDRJ_RES	0x00
	#define DDRJ_MASK	0x00
#endif
#ifndef PINJ
	#define PINJ		NO_REG
	#define PINJ_RES	0x00
	#define PINJ_MASK	0x00
#endif
#ifndef PORTH
	#define PORTH		NO_REG
	#define PORTH_RES	0x00
	#define PORTH_MASK	0x00
#endif
#ifndef DDRH
	#define DDRH		NO_REG
	#define DDRH_RES	0x00
	#define DDRH_MASK	0x00
#endif
#ifndef PINH
	#define PINH		NO_REG
	#define PINH_RES	0x00
	#define PINH_MASK	0x00
#endif
#ifndef UDR2
	#define UDR2		NO_REG
	#define UDR2_RES	0x00
	#define UDR2_MASK	0x00
#endif
#ifndef UBRR2H
	#define UBRR2H		NO_REG
	#define UBRR2H_RES	0x00
	#define UBRR2H_MASK	0x00
#endif
#ifndef UBRR2L
	#define UBRR2L		NO_REG
	#define UBRR2L_RES	0x00
	#define UBRR2L_MASK	0x00
#endif
#ifndef UCSR2C
	#define UCSR2C		NO_REG
	#define UCSR2C_RES	0x00
	#define UCSR2C_MASK	0x00
#endif
#ifndef UCSR2B
	#define UCSR2B		NO_REG
	#define UCSR2B_RES	0x00
	#define UCSR2B_MASK	0x00
#endif
#ifndef UCSR2A
	#define UCSR2A		NO_REG
	#define UCSR2A_RES	0x00
	#define UCSR2A_MASK	0x00
#endif
#ifndef UDR1
	#define UDR1		NO_REG
	#define UDR1_RES	0x00
	#define UDR1_MASK	0x00
#endif
#ifndef UBRR1H
	#define UBRR1H		NO_REG
	#define UBRR1H_RES	0x00
	#define UBRR1H_MASK	0x00
#endif
#ifndef UBRR1L
	#define UBRR1L		NO_REG
	#define UBRR1L_RES	0x00
	#define UBRR1L_MASK	0x00
#endif
#ifndef UCSR1C
	#define UCSR1C		NO_REG
	#define UCSR1C_RES	0x00
	#define UCSR1C_MASK	0x00
#endif
#ifndef UCSR1B
	#define UCSR1B		NO_REG
	#define UCSR1B_RES	0x00
	#define UCSR1B_MASK	0x00
#endif
#ifndef UCSR1A
	#define UCSR1A		NO_REG
	#define UCSR1A_RES	0x00
	#define UCSR1A_MASK	0x00
#endif
#ifndef UDR0
	#define UDR0		NO_REG
	#define UDR0_RES	0x00
	#define UDR0_MASK	0x00
#endif
#ifndef UBRR0H
	#define UBRR0H		NO_REG
	#define UBRR0H_RES	0x00
	#define UBRR0H_MASK	0x00
#endif
#ifndef UBRR0L
	#define UBRR0L		NO_REG
	#define UBRR0L_RES	0x00
	#define UBRR0L_MASK	0x00
#endif
#ifndef UCSR0C
	#define UCSR0C		NO_REG
	#define UCSR0C_RES	0x00
	#define UCSR0C_MASK	0x00
#endif
#ifndef UCSR0B
	#define UCSR0B		NO_REG
	#define UCSR0B_RES	0x00
	#define UCSR0B_MASK	0x00
#endif
#ifndef UCSR0A
	#define UCSR0A		NO_REG
	#define UCSR0A_RES	0x00
	#define UCSR0A_MASK	0x00
#endif
#ifndef TWAMR
	#define TWAMR		NO_REG
	#define TWAMR_RES	0x00
	#define TWAMR_MASK	0x00
#endif
#ifndef OCR2B
	#define OCR2B		NO_REG
	#define OCR2B_RES	0x00
	#define OCR2B_MASK	0x00
#endif
#ifndef OCR2A
	#define OCR2A		NO_REG
	#define OCR2A_RES	0x00
	#define OCR2A_MASK	0x00
#endif
#ifndef TCCR2B
	#define TCCR2B		NO_REG
	#define TCCR2B_RES	0x00
	#define TCCR2B_MASK	0x00
#endif
#ifndef TCCR2A
	#define TCCR2A		NO_REG
	#define TCCR2A_RES	0x00
	#define TCCR2A_MASK	0x00
#endif
#ifndef OCR4CH
	#define OCR4CH		NO_REG
	#define OCR4CH_RES	0x00
	#define OCR4CH_MASK	0x00
#endif
#ifndef OCR4CL
	#define OCR4CL		NO_REG
	#define OCR4CL_RES	0x00
	#define OCR4CL_MASK	0x00
#endif
#ifndef OCR4BH
	#define OCR4BH		NO_REG
	#define OCR4BH_RES	0x00
	#define OCR4BH_MASK	0x00
#endif
#ifndef OCR4BL
	#define OCR4BL		NO_REG
	#define OCR4BL_RES	0x00
	#define OCR4BL_MASK	0x00
#endif
#ifndef OCR4AH
	#define OCR4AH		NO_REG
	#define OCR4AH_RES	0x00
	#define OCR4AH_MASK	0x00
#endif
#ifndef OCR4AL
	#define OCR4AL		NO_REG
	#define OCR4AL_RES	0x00
	#define OCR4AL_MASK	0x00
#endif
#ifndef ICR4H
	#define ICR4H		NO_REG
	#define ICR4H_RES	0x00
	#define ICR4H_MASK	0x00
#endif
#ifndef ICR4L
	#define ICR4L		NO_REG
	#define ICR4L_RES	0x00
	#define ICR4L_MASK	0x00
#endif
#ifndef TCNT4H
	#define TCNT4H		NO_REG
	#define TCNT4H_RES	0x00
	#define TCNT4H_MASK	0x00
#endif
#ifndef TCNT4L
	#define TCNT4L		NO_REG
	#define TCNT4L_RES	0x00
	#define TCNT4L_MASK	0x00
#endif
#ifndef TCCR4C
	#define TCCR4C		NO_REG
	#define TCCR4C_RES	0x00
	#define TCCR4C_MASK	0x00
#endif
#ifndef TCCR4B
	#define TCCR4B		NO_REG
	#define TCCR4B_RES	0x00
	#define TCCR4B_MASK	0x00
#endif
#ifndef TCCR4A
	#define TCCR4A		NO_REG
	#define TCCR4A_RES	0x00
	#define TCCR4A_MASK	0x00
#endif
#ifndef OCR3CH
	#define OCR3CH		NO_REG
	#define OCR3CH_RES	0x00
	#define OCR3CH_MASK	0x00
#endif
#ifndef OCR3CL
	#define OCR3CL		NO_REG
	#define OCR3CL_RES	0x00
	#define OCR3CL_MASK	0x00
#endif
#ifndef OCR3BH
	#define OCR3BH		NO_REG
	#define OCR3BH_RES	0x00
	#define OCR3BH_MASK	0x00
#endif
#ifndef OCR3BL
	#define OCR3BL		NO_REG
	#define OCR3BL_RES	0x00
	#define OCR3BL_MASK	0x00
#endif
#ifndef OCR3AH
	#define OCR3AH		NO_REG
	#define OCR3AH_RES	0x00
	#define OCR3AH_MASK	0x00
#endif
#ifndef OCR3AL
	#define OCR3AL		NO_REG
	#define OCR3AL_RES	0x00
	#define OCR3AL_MASK	0x00
#endif
#ifndef ICR3H
	#define ICR3H		NO_REG
	#define ICR3H_RES	0x00
	#define ICR3H_MASK	0x00
#endif
#ifndef ICR3L
	#define ICR3L		NO_REG
	#define ICR3L_RES	0x00
	#define ICR3L_MASK	0x00
#endif
#ifndef TCNT3H
	#define TCNT3H		NO_REG
	#define TCNT3H_RES	0x00
	#define TCNT3H_MASK	0x00
#endif
#ifndef TCNT3L
	#define TCNT3L		NO_REG
	#define TCNT3L_RES	0x00
	#define TCNT3L_MASK	0x00
#endif
#ifndef TCCR3C
	#define TCCR3C		NO_REG
	#define TCCR3C_RES	0x00
	#define TCCR3C_MASK	0x00
#endif
#ifndef TCCR3B
	#define TCCR3B		NO_REG
	#define TCCR3B_RES	0x00
	#define TCCR3B_MASK	0x00
#endif
#ifndef TCCR3A
	#define TCCR3A		NO_REG
	#define TCCR3A_RES	0x00
	#define TCCR3A_MASK	0x00
#endif
#ifndef OCR1CH
	#define OCR1CH		NO_REG
	#define OCR1CH_RES	0x00
	#define OCR1CH_MASK	0x00
#endif
#ifndef OCR1CL
	#define OCR1CL		NO_REG
	#define OCR1CL_RES	0x00
	#define OCR1CL_MASK	0x00
#endif
#ifndef TCCR1C
	#define TCCR1C		NO_REG
	#define TCCR1C_RES	0x00
	#define TCCR1C_MASK	0x00
#endif
#ifndef DIDR1
	#define DIDR1		NO_REG
	#define DIDR1_RES	0x00
	#define DIDR1_MASK	0x00
#endif
#ifndef DIDR0
	#define DIDR0		NO_REG
	#define DIDR0_RES	0x00
	#define DIDR0_MASK	0x00
#endif
#ifndef DIDR2
	#define DIDR2		NO_REG
	#define DIDR2_RES	0x00
	#define DIDR2_MASK	0x00
#endif
#ifndef ADCSRB
	#define ADCSRB		NO_REG
	#define ADCSRB_RES	0x00
	#define ADCSRB_MASK	0x00
#endif
#ifndef XMCRB
	#define XMCRB		NO_REG
	#define XMCRB_RES	0x00
	#define XMCRB_MASK	0x00
#endif
#ifndef XMCRA
	#define XMCRA		NO_REG
	#define XMCRA_RES	0x00
	#define XMCRA_MASK	0x00
#endif
#ifndef TIMSK5
	#define TIMSK5		NO_REG
	#define TIMSK5_RES	0x00
	#define TIMSK5_MASK	0x00
#endif
#ifndef TIMSK4
	#define TIMSK4		NO_REG
	#define TIMSK4_RES	0x00
	#define TIMSK4_MASK	0x00
#endif
#ifndef TIMSK3
	#define TIMSK3		NO_REG
	#define TIMSK3_RES	0x00
	#define TIMSK3_MASK	0x00
#endif
#ifndef TIMSK2
	#define TIMSK2		NO_REG
	#define TIMSK2_RES	0x00
	#define TIMSK2_MASK	0x00
#endif
#ifndef TIMSK1
	#define TIMSK1		NO_REG
	#define TIMSK1_RES	0x00
	#define TIMSK1_MASK	0x00
#endif
#ifndef TIMSK0
	#define TIMSK0		NO_REG
	#define TIMSK0_RES	0x00
	#define TIMSK0_MASK	0x00
#endif
#ifndef PCMSK2
	#define PCMSK2		NO_REG
	#define PCMSK2_RES	0x00
	#define PCMSK2_MASK	0x00
#endif
#ifndef PCMSK1
	#define PCMSK1		NO_REG
	#define PCMSK1_RES	0x00
	#define PCMSK1_MASK	0x00
#endif
#ifndef PCMSK0
	#define PCMSK0		NO_REG
	#define PCMSK0_RES	0x00
	#define PCMSK0_MASK	0x00
#endif
#ifndef EICRB
	#define EICRB		NO_REG
	#define EICRB_RES	0x00
	#define EICRB_MASK	0x00
#endif
#ifndef EICRA
	#define EICRA		NO_REG
	#define EICRA_RES	0x00
	#define EICRA_MASK	0x00
#endif
#ifndef PCICR
	#define PCICR		NO_REG
	#define PCICR_RES	0x00
	#define PCICR_MASK	0x00
#endif
#ifndef PRR1
	#define PRR1		NO_REG
	#define PRR1_RES	0x00
	#define PRR1_MASK	0x00
#endif
#ifndef PRR0
	#define PRR0		NO_REG
	#define PRR0_RES	0x00
	#define PRR0_MASK	0x00
#endif
#ifndef CLKPR
	#define CLKPR		NO_REG
	#define CLKPR_RES	0x00
	#define CLKPR_MASK	0x00
#endif
#ifndef WDTCSR
	#define WDTCSR		NO_REG
	#define WDTCSR_RES	0x00
	#define WDTCSR_MASK	0x00
#endif
#ifndef SPCR0
	#define SPCR0		NO_REG
	#define SPCR0_RES	0x00
	#define SPCR0_MASK	0x00
#endif
#ifndef SPSR0
	#define SPSR0		NO_REG
	#define SPSR0_RES	0x00
	#define SPSR0_MASK	0x00
#endif
#ifndef DWDR
	#define DWDR		NO_REG
	#define DWDR_RES	0x00
	#define DWDR_MASK	0x00
#endif
#ifndef PRR
	#define PRR		NO_REG
	#define PRR_RES		0x00
	#define PRR_MASK	0x00
#endif
#ifndef PCMSK3
	#define PCMSK3		NO_REG
	#define PCMSK3_RES		0x00
	#define PCMSK3_MASK	0x00
#endif
#ifndef PCMSK3
	#define PCMSK3		NO_REG
	#define PCMSK3_RES		0x00
	#define PCMSK3_MASK	0x00
#endif

/* Macro to initialize array of AVR I/O registers. */
#define AVR_INIT_IOREGS { \
	{ "SREG", SREG, NULL, SREG_RES, SREG_MASK }, \
	{ "SPH", SPH, NULL, SPH_RES, SPH_MASK }, \
	{ "SPL", SPL, NULL, SPL_RES, SPL_MASK }, \
	{ "PORTA", PORTA, NULL, PORTA_RES, PORTA_MASK }, \
	{ "DDRA", DDRA, NULL, DDRA_RES, DDRA_MASK }, \
	{ "PINA", PINA, NULL, PINA_RES, PINA_MASK }, \
	{ "PORTB", PORTB, NULL, PORTB_RES, PORTB_MASK }, \
	{ "DDRB", DDRB, NULL, DDRB_RES, DDRB_MASK }, \
	{ "PINB", PINB, NULL, PINB_RES, PINB_MASK }, \
	{ "PORTC", PORTC, NULL, PORTC_RES, PORTC_MASK }, \
	{ "DDRC", DDRC, NULL, DDRC_RES, DDRC_MASK }, \
	{ "PINC", PINC, NULL, PINC_RES, PINC_MASK }, \
	{ "PORTD", PORTD, NULL, PORTD_RES, PORTD_MASK }, \
	{ "DDRD", DDRD, NULL, DDRD_RES, DDRD_MASK }, \
	{ "PIND", PIND, NULL, PIND_RES, PIND_MASK }, \
	{ "PORTE", PORTE, NULL, PORTE_RES, PORTE_MASK }, \
	{ "DDRE", DDRE, NULL, DDRE_RES, DDRE_MASK }, \
	{ "PINE", PINE, NULL, PINE_RES, PINE_MASK }, \
	{ "PORTF", PORTF, NULL, PORTF_RES, PORTF_MASK }, \
	{ "DDRF", DDRF, NULL, DDRF_RES, DDRF_MASK }, \
	{ "PINF", PINF, NULL, PINF_RES, PINF_MASK }, \
	{ "PORTG", PORTG, NULL, PORTG_RES, PORTG_MASK }, \
	{ "DDRG", DDRG, NULL, DDRG_RES, DDRG_MASK }, \
	{ "PING", PING, NULL, PING_RES, PING_MASK }, \
	{ "EIND", EIND, NULL, EIND_RES, EIND_MASK }, \
	{ "RAMPZ", RAMPZ, NULL, RAMPZ_RES, RAMPZ_MASK }, \
	{ "SPMCSR", SPMCSR, NULL, SPMCSR_RES, SPMCSR_MASK }, \
	{ "MCUCR", MCUCR, NULL, MCUCR_RES, MCUCR_MASK }, \
	{ "MCUSR", MCUSR, NULL, MCUSR_RES, MCUSR_MASK }, \
	{ "SMCR", SMCR, NULL, SMCR_RES, SMCR_MASK }, \
	{ "OCDR", OCDR, NULL, OCDR_RES, OCDR_MASK }, \
	{ "ACSR", ACSR, NULL, ACSR_RES, ACSR_MASK }, \
	{ "SPDR", SPDR, NULL, SPDR_RES, SPDR_MASK }, \
	{ "SPSR", SPSR, NULL, SPSR_RES, SPSR_MASK }, \
	{ "SPCR", SPCR, NULL, SPCR_RES, SPCR_MASK }, \
	{ "GPIOR2", GPIOR2, NULL, GPIOR2_RES, GPIOR2_MASK }, \
	{ "GPIOR1", GPIOR1, NULL, GPIOR1_RES, GPIOR1_MASK }, \
	{ "GPIOR0", GPIOR0, NULL, GPIOR0_RES, GPIOR0_MASK }, \
	{ "OCR0B", OCR0B, NULL, OCR0B_RES, OCR0B_MASK }, \
	{ "OCR0A", OCR0A, NULL, OCR0A_RES, OCR0A_MASK }, \
	{ "TCNT0", TCNT0, NULL, TCNT0_RES, TCNT0_MASK }, \
	{ "TCCR0B", TCCR0B, NULL, TCCR0B_RES, TCCR0B_MASK }, \
	{ "TCCR0A", TCCR0A, NULL, TCCR0A_RES, TCCR0A_MASK }, \
	{ "TCCR0", TCCR0, NULL, TCCR0_RES, TCCR0_MASK }, \
	{ "GTCCR", GTCCR, NULL, GTCCR_RES, GTCCR_MASK }, \
	{ "EEARH", EEARH, NULL, EEARH_RES, EEARH_MASK }, \
	{ "EEARL", EEARL, NULL, EEARL_RES, EEARL_MASK }, \
	{ "EEDR", EEDR, NULL, EEDR_RES, EEDR_MASK }, \
	{ "EECR", EECR, NULL, EECR_RES, EECR_MASK }, \
	{ "EIMSK", EIMSK, NULL, EIMSK_RES, EIMSK_MASK }, \
	{ "EIFR", EIFR, NULL, EIFR_RES, EIFR_MASK }, \
	{ "PCIFR", PCIFR, NULL, PCIFR_RES, PCIFR_MASK }, \
	{ "TIFR5", TIFR5, NULL, TIFR5_RES, TIFR5_MASK }, \
	{ "TIFR4", TIFR4, NULL, TIFR4_RES, TIFR4_MASK }, \
	{ "TIFR3", TIFR3, NULL, TIFR3_RES, TIFR3_MASK }, \
	{ "TIFR2", TIFR2, NULL, TIFR2_RES, TIFR2_MASK }, \
	{ "TIFR1", TIFR1, NULL, TIFR1_RES, TIFR1_MASK }, \
	{ "TIFR0", TIFR0, NULL, TIFR0_RES, TIFR0_MASK }, \
	{ "TIFR", TIFR, NULL, TIFR_RES, TIFR_MASK }, \
	{ "GICR", GICR, NULL, GICR_RES, GICR_MASK }, \
	{ "GIFR", GIFR, NULL, GIFR_RES, GIFR_MASK }, \
	{ "TIMSK", TIMSK, NULL, TIMSK_RES, TIMSK_MASK }, \
	{ "SPMCR", SPMCR, NULL, SPMCR_RES, SPMCR_MASK }, \
	{ "TWCR", TWCR, NULL, TWCR_RES, TWCR_MASK }, \
	{ "MCUCSR", MCUCSR, NULL, MCUCSR_RES, MCUCSR_MASK }, \
	{ "OSCCAL", OSCCAL, NULL, OSCCAL_RES, OSCCAL_MASK }, \
	{ "SFIOR", SFIOR, NULL, SFIOR_RES, SFIOR_MASK }, \
	{ "TCCR1A", TCCR1A, NULL, TCCR1A_RES, TCCR1A_MASK }, \
	{ "TCCR1B", TCCR1B, NULL, TCCR1B_RES, TCCR1B_MASK }, \
	{ "TCNT1H", TCNT1H, NULL, TCNT1H_RES, TCNT1H_MASK }, \
	{ "TCNT1L", TCNT1L, NULL, TCNT1L_RES, TCNT1L_MASK }, \
	{ "OCR1AH", OCR1AH, NULL, OCR1AH_RES, OCR1AH_MASK }, \
	{ "OCR1AL", OCR1AL, NULL, OCR1AL_RES, OCR1AL_MASK }, \
	{ "OCR1BH", OCR1BH, NULL, OCR1BH_RES, OCR1BH_MASK }, \
	{ "OCR1BL", OCR1BL, NULL, OCR1BL_RES, OCR1BL_MASK }, \
	{ "ICR1H", ICR1H, NULL, ICR1H_RES, ICR1H_MASK }, \
	{ "ICR1L", ICR1L, NULL, ICR1L_RES, ICR1L_MASK }, \
	{ "TCCR2", TCCR2, NULL, TCCR2_RES, TCCR2_MASK }, \
	{ "TCNT2", TCNT2, NULL, TCNT2_RES, TCNT2_MASK }, \
	{ "OCR2", OCR2, NULL, OCR2_RES, OCR2_MASK }, \
	{ "ASSR", ASSR, NULL, ASSR_RES, ASSR_MASK }, \
	{ "WDTCR", WDTCR, NULL, WDTCR_RES, WDTCR_MASK }, \
	{ "UBRRH", UBRRH, NULL, UBRRH_RES, UBRRH_MASK }, \
	{ "UCSRC", UCSRC, NULL, UCSRC_RES, UCSRC_MASK }, \
	{ "UDR", UDR, NULL, UDR_RES, UDR_MASK }, \
	{ "UCSRA", UCSRA, NULL, UCSRA_RES, UCSRA_MASK }, \
	{ "UCSRB", UCSRB, NULL, UCSRB_RES, UCSRB_MASK }, \
	{ "UBRRL", UBRRL, NULL, UBRRL_RES, UBRRL_MASK }, \
	{ "ADMUX", ADMUX, NULL, ADMUX_RES, ADMUX_MASK }, \
	{ "ADCSRA", ADCSRA, NULL, ADCSRA_RES, ADCSRA_MASK }, \
	{ "ADCH", ADCH, NULL, ADCH_RES, ADCH_MASK }, \
	{ "ADCL", ADCL, NULL, ADCL_RES, ADCL_MASK }, \
	{ "TWDR", TWDR, NULL, TWDR_RES, TWDR_MASK }, \
	{ "TWAR", TWAR, NULL, TWAR_RES, TWAR_MASK }, \
	{ "TWSR", TWSR, NULL, TWSR_RES, TWSR_MASK }, \
	{ "TWBR", TWBR, NULL, TWBR_RES, TWBR_MASK }, \
	{ "UDR3", UDR3, NULL, UDR3_RES, UDR3_MASK }, \
	{ "UBRR3H", UBRR3H, NULL, UBRR3H_RES, UBRR3H_MASK }, \
	{ "UBRR3L", UBRR3L, NULL, UBRR3L_RES, UBRR3L_MASK }, \
	{ "UCSR3C", UCSR3C, NULL, UCSR3C_RES, UCSR3C_MASK }, \
	{ "UCSR3B", UCSR3B, NULL, UCSR3B_RES, UCSR3B_MASK }, \
	{ "UCSR3A", UCSR3A, NULL, UCSR3A_RES, UCSR3A_MASK }, \
	{ "OCR5CH", OCR5CH, NULL, OCR5CH_RES, OCR5CH_MASK }, \
	{ "OCR5CL", OCR5CL, NULL, OCR5CL_RES, OCR5CL_MASK }, \
	{ "OCR5BH", OCR5BH, NULL, OCR5BH_RES, OCR5BH_MASK }, \
	{ "OCR5BL", OCR5BL, NULL, OCR5BL_RES, OCR5BL_MASK }, \
	{ "OCR5AH", OCR5AH, NULL, OCR5AH_RES, OCR5AH_MASK }, \
	{ "OCR5AL", OCR5AL, NULL, OCR5AL_RES, OCR5AL_MASK }, \
	{ "ICR5H", ICR5H, NULL, ICR5H_RES, ICR5H_MASK }, \
	{ "ICR5L", ICR5L, NULL, ICR5L_RES, ICR5L_MASK }, \
	{ "TCNT5H", TCNT5H, NULL, TCNT5H_RES, TCNT5H_MASK }, \
	{ "TCNT5L", TCNT5L, NULL, TCNT5L_RES, TCNT5L_MASK }, \
	{ "TCCR5C", TCCR5C, NULL, TCCR5C_RES, TCCR5C_MASK }, \
	{ "TCCR5B", TCCR5B, NULL, TCCR5B_RES, TCCR5B_MASK }, \
	{ "TCCR5A", TCCR5A, NULL, TCCR5A_RES, TCCR5A_MASK }, \
	{ "PORTL", PORTL, NULL, PORTL_RES, PORTL_MASK }, \
	{ "DDRL", DDRL, NULL, DDRL_RES, DDRL_MASK }, \
	{ "PINL", PINL, NULL, PINL_RES, PINL_MASK }, \
	{ "PORTK", PORTK, NULL, PORTK_RES, PORTK_MASK }, \
	{ "DDRK", DDRK, NULL, DDRK_RES, DDRK_MASK }, \
	{ "PINK", PINK, NULL, PINK_RES, PINK_MASK }, \
	{ "PORTJ", PORTJ, NULL, PORTJ_RES, PORTJ_MASK }, \
	{ "DDRJ", DDRJ, NULL, DDRJ_RES, DDRJ_MASK }, \
	{ "PINJ", PINJ, NULL, PINJ_RES, PINJ_MASK }, \
	{ "PORTH", PORTH, NULL, PORTH_RES, PORTH_MASK }, \
	{ "DDRH", DDRH, NULL, DDRH_RES, DDRH_MASK }, \
	{ "PINH", PINH, NULL, PINH_RES, PINH_MASK }, \
	{ "UDR2", UDR2, NULL, UDR2_RES, UDR2_MASK }, \
	{ "UBRR2H", UBRR2H, NULL, UBRR2H_RES, UBRR2H_MASK }, \
	{ "UBRR2L", UBRR2L, NULL, UBRR2L_RES, UBRR2L_MASK }, \
	{ "UCSR2C", UCSR2C, NULL, UCSR2C_RES, UCSR2C_MASK }, \
	{ "UCSR2B", UCSR2B, NULL, UCSR2B_RES, UCSR2B_MASK }, \
	{ "UCSR2A", UCSR2A, NULL, UCSR2A_RES, UCSR2A_MASK }, \
	{ "UDR1", UDR1, NULL, UDR1_RES, UDR1_MASK }, \
	{ "UBRR1H", UBRR1H, NULL, UBRR1H_RES, UBRR1H_MASK }, \
	{ "UBRR1L", UBRR1L, NULL, UBRR1L_RES, UBRR1L_MASK }, \
	{ "UCSR1C", UCSR1C, NULL, UCSR1C_RES, UCSR1C_MASK }, \
	{ "UCSR1B", UCSR1B, NULL, UCSR1B_RES, UCSR1B_MASK }, \
	{ "UCSR1A", UCSR1A, NULL, UCSR1A_RES, UCSR1A_MASK }, \
	{ "UDR0", UDR0, NULL, UDR0_RES, UDR0_MASK }, \
	{ "UBRR0H", UBRR0H, NULL, UBRR0H_RES, UBRR0H_MASK }, \
	{ "UBRR0L", UBRR0L, NULL, UBRR0L_RES, UBRR0L_MASK }, \
	{ "UCSR0C", UCSR0C, NULL, UCSR0C_RES, UCSR0C_MASK }, \
	{ "UCSR0B", UCSR0B, NULL, UCSR0B_RES, UCSR0B_MASK }, \
	{ "UCSR0A", UCSR0A, NULL, UCSR0A_RES, UCSR0A_MASK }, \
	{ "TWAMR", TWAMR, NULL, TWAMR_RES, TWAMR_MASK }, \
	{ "OCR2B", OCR2B, NULL, OCR2B_RES, OCR2B_MASK }, \
	{ "OCR2A", OCR2A, NULL, OCR2A_RES, OCR2A_MASK }, \
	{ "TCCR2B", TCCR2B, NULL, TCCR2B_RES, TCCR2B_MASK }, \
	{ "TCCR2A", TCCR2A, NULL, TCCR2A_RES, TCCR2A_MASK }, \
	{ "OCR4CH", OCR4CH, NULL, OCR4CH_RES, OCR4CH_MASK }, \
	{ "OCR4CL", OCR4CL, NULL, OCR4CL_RES, OCR4CL_MASK }, \
	{ "OCR4BH", OCR4BH, NULL, OCR4BH_RES, OCR4BH_MASK }, \
	{ "OCR4BL", OCR4BL, NULL, OCR4BL_RES, OCR4BL_MASK }, \
	{ "OCR4AH", OCR4AH, NULL, OCR4AH_RES, OCR4AH_MASK }, \
	{ "OCR4AL", OCR4AL, NULL, OCR4AL_RES, OCR4AL_MASK }, \
	{ "ICR4H", ICR4H, NULL, ICR4H_RES, ICR4H_MASK }, \
	{ "ICR4L", ICR4L, NULL, ICR4L_RES, ICR4L_MASK }, \
	{ "TCNT4H", TCNT4H, NULL, TCNT4H_RES, TCNT4H_MASK }, \
	{ "TCNT4L", TCNT4L, NULL, TCNT4L_RES, TCNT4L_MASK }, \
	{ "TCCR4C", TCCR4C, NULL, TCCR4C_RES, TCCR4C_MASK }, \
	{ "TCCR4B", TCCR4B, NULL, TCCR4B_RES, TCCR4B_MASK }, \
	{ "TCCR4A", TCCR4A, NULL, TCCR4A_RES, TCCR4A_MASK }, \
	{ "OCR3CH", OCR3CH, NULL, OCR3CH_RES, OCR3CH_MASK }, \
	{ "OCR3CL", OCR3CL, NULL, OCR3CL_RES, OCR3CL_MASK }, \
	{ "OCR3BH", OCR3BH, NULL, OCR3BH_RES, OCR3BH_MASK }, \
	{ "OCR3BL", OCR3BL, NULL, OCR3BL_RES, OCR3BL_MASK }, \
	{ "OCR3AH", OCR3AH, NULL, OCR3AH_RES, OCR3AH_MASK }, \
	{ "OCR3AL", OCR3AL, NULL, OCR3AL_RES, OCR3AL_MASK }, \
	{ "ICR3H", ICR3H, NULL, ICR3H_RES, ICR3H_MASK }, \
	{ "ICR3L", ICR3L, NULL, ICR3L_RES, ICR3L_MASK }, \
	{ "TCNT3H", TCNT3H, NULL, TCNT3H_RES, TCNT3H_MASK }, \
	{ "TCNT3L", TCNT3L, NULL, TCNT3L_RES, TCNT3L_MASK }, \
	{ "TCCR3C", TCCR3C, NULL, TCCR3C_RES, TCCR3C_MASK }, \
	{ "TCCR3B", TCCR3B, NULL, TCCR3B_RES, TCCR3B_MASK }, \
	{ "TCCR3A", TCCR3A, NULL, TCCR3A_RES, TCCR3A_MASK }, \
	{ "OCR1CH", OCR1CH, NULL, OCR1CH_RES, OCR1CH_MASK }, \
	{ "OCR1CL", OCR1CL, NULL, OCR1CL_RES, OCR1CL_MASK }, \
	{ "TCCR1C", TCCR1C, NULL, TCCR1C_RES, TCCR1C_MASK }, \
	{ "DIDR1", DIDR1, NULL, DIDR1_RES, DIDR1_MASK }, \
	{ "DIDR0", DIDR0, NULL, DIDR0_RES, DIDR0_MASK }, \
	{ "DIDR2", DIDR2, NULL, DIDR2_RES, DIDR2_MASK }, \
	{ "ADCSRB", ADCSRB, NULL, ADCSRB_RES, ADCSRB_MASK }, \
	{ "XMCRB", XMCRB, NULL, XMCRB_RES, XMCRB_MASK }, \
	{ "XMCRA", XMCRA, NULL, XMCRA_RES, XMCRA_MASK }, \
	{ "TIMSK5", TIMSK5, NULL, TIMSK5_RES, TIMSK5_MASK }, \
	{ "TIMSK4", TIMSK4, NULL, TIMSK4_RES, TIMSK4_MASK }, \
	{ "TIMSK3", TIMSK3, NULL, TIMSK3_RES, TIMSK3_MASK }, \
	{ "TIMSK2", TIMSK2, NULL, TIMSK2_RES, TIMSK2_MASK }, \
	{ "TIMSK1", TIMSK1, NULL, TIMSK1_RES, TIMSK1_MASK }, \
	{ "TIMSK0", TIMSK0, NULL, TIMSK0_RES, TIMSK0_MASK }, \
	{ "PCMSK2", PCMSK2, NULL, PCMSK2_RES, PCMSK2_MASK }, \
	{ "PCMSK1", PCMSK1, NULL, PCMSK1_RES, PCMSK1_MASK }, \
	{ "PCMSK0", PCMSK0, NULL, PCMSK0_RES, PCMSK0_MASK }, \
	{ "EICRB", EICRB, NULL, EICRB_RES, EICRB_MASK }, \
	{ "EICRA", EICRA, NULL, EICRA_RES, EICRA_MASK }, \
	{ "PCICR", PCICR, NULL, PCICR_RES, PCICR_MASK }, \
	{ "PRR1", PRR1, NULL, PRR1_RES, PRR1_MASK }, \
	{ "PRR0", PRR0, NULL, PRR0_RES, PRR0_MASK }, \
	{ "CLKPR", CLKPR, NULL, CLKPR_RES, CLKPR_MASK }, \
	{ "WDTCSR", WDTCSR, NULL, WDTCSR_RES, WDTCSR_MASK }, \
	{ "SPCR0", SPCR0, NULL, SPCR0_RES, SPCR0_MASK }, \
	{ "SPSR0", SPSR0, NULL, SPSR0_RES, SPSR0_MASK }, \
	{ "DWDR", DWDR, NULL, DWDR_RES, DWDR_MASK }, \
	{ "PRR", PRR, NULL, PRR_RES, PRR_MASK }, \
	{ "PCMSK3", PCMSK3, NULL, PCMSK3_RES, PCMSK3_MASK }, \
}
