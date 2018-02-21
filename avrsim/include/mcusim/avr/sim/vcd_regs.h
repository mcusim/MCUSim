/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

/*
 * It's a common situation whether one AVR MCU has a set of registers, but
 * another one - doesn't. Memory offset of the unavailable registers
 * will be marked as NO_REG.
 */
#define NO_REG		-1

/* Status and stack registers */
#ifndef SREG
#define SREG		NO_REG
#endif
#ifndef SPH
#define SPH		NO_REG
#endif
#ifndef SPL
#define SPL		NO_REG
#endif

/*
 * General purpose I/O ports.
 */
/* Port A registers */
#ifndef PORTA
#define PORTA		NO_REG
#endif
#ifndef DDRA
#define DDRA		NO_REG
#endif
#ifndef PINA
#define PINA		NO_REG
#endif
/* Port B registers */
#ifndef PORTB
#define PORTB		NO_REG
#endif
#ifndef DDRB
#define DDRB		NO_REG
#endif
#ifndef PINB
#define PINB		NO_REG
#endif
/* Port C registers */
#ifndef PORTC
#define PORTC		NO_REG
#endif
#ifndef DDRC
#define DDRC		NO_REG
#endif
#ifndef PINC
#define PINC		NO_REG
#endif
/* Port D registers */
#ifndef PORTD
#define PORTD		NO_REG
#endif
#ifndef DDRD
#define DDRD		NO_REG
#endif
#ifndef PIND
#define PIND		NO_REG
#endif
/* Port E registers */
#ifndef PORTE
#define PORTE		NO_REG
#endif
#ifndef DDRE
#define DDRE		NO_REG
#endif
#ifndef PINE
#define PINE		NO_REG
#endif
/* Port F registers */
#ifndef PORTF
#define PORTF		NO_REG
#endif
#ifndef DDRF
#define DDRF		NO_REG
#endif
#ifndef PINF
#define PINF		NO_REG
#endif
/* Port G registers */
#ifndef PORTG
#define PORTG		NO_REG
#endif
#ifndef DDRG
#define DDRG		NO_REG
#endif
#ifndef PING
#define PING		NO_REG
#endif
/*
 * END General purpose I/O ports.
 */

#ifndef EIND
#define EIND		NO_REG
#endif
#ifndef RAMPZ
#define RAMPZ		NO_REG
#endif
#ifndef SPMCSR
#define SPMCSR		NO_REG
#endif
#ifndef MCUCR
#define MCUCR		NO_REG
#endif
#ifndef MCUSR
#define MCUSR		NO_REG
#endif
#ifndef SMCR
#define SMCR		NO_REG
#endif
#ifndef OCDR
#define OCDR		NO_REG
#endif
#ifndef ACSR
#define ACSR		NO_REG
#endif
#ifndef SPDR
#define SPDR		NO_REG
#endif
#ifndef SPSR
#define SPSR		NO_REG
#endif
#ifndef SPCR
#define SPCR		NO_REG
#endif
#ifndef GPIOR2
#define GPIOR2		NO_REG
#endif
#ifndef GPIOR1
#define GPIOR1		NO_REG
#endif
#ifndef GPIOR0
#define GPIOR0		NO_REG
#endif

    /* Timer/Counter0 */
#ifndef OCR0B
#define OCR0B		NO_REG
#endif
#ifndef OCR0A
#define OCR0A		NO_REG
#endif
#ifndef TCNT0
#define TCNT0		NO_REG
#endif
#ifndef TCCR0B
#define TCCR0B		NO_REG
#endif
#ifndef TCCR0A
#define TCCR0A		NO_REG
#endif
/* END Timer/Counter0 */

#ifndef GTCCR
#define GTCCR		NO_REG
#endif
#ifndef EEARH
#define EEARH		NO_REG
#endif
#ifndef EEARL
#define EEARL		NO_REG
#endif
#ifndef EEDR
#define EEDR		NO_REG
#endif
#ifndef EECR
#define EECR		NO_REG
#endif
#ifndef EIMSK
#define EIMSK		NO_REG
#endif
#ifndef EIFR
#define EIFR		NO_REG
#endif
#ifndef PCIFR
#define PCIFR		NO_REG
#endif

/* Timer/Counter Interrupt Flag Register */
#ifndef TIFR5
#define TIFR5		NO_REG
#endif
#ifndef TIFR4
#define TIFR4		NO_REG
#endif
#ifndef TIFR3
#define TIFR3		NO_REG
#endif
#ifndef TIFR2
#define TIFR2		NO_REG
#endif
#ifndef TIFR1
#define TIFR1		NO_REG
#endif
#ifndef TIFR0
#define TIFR0		NO_REG
#endif

#ifndef TIFR
#define TIFR		NO_REG
#endif
/* END Timer/Counter Interrupt Flag Register */

#ifndef GICR
#define GICR		NO_REG
#endif
#ifndef GIFR
#define GIFR		NO_REG
#endif
#ifndef TIMSK
#define TIMSK		NO_REG
#endif
#ifndef SPMCR
#define SPMCR		NO_REG
#endif
#ifndef TWCR
#define TWCR		NO_REG
#endif
#ifndef MCUCSR
#define MCUCSR		NO_REG
#endif
#ifndef OSCCAL
#define OSCCAL		NO_REG
#endif
#ifndef SFIOR
#define SFIOR		NO_REG
#endif
#ifndef TCCR1A
#define TCCR1A		NO_REG
#endif
#ifndef TCCR1B
#define TCCR1B		NO_REG
#endif
#ifndef TCNT1H
#define TCNT1H		NO_REG
#endif
#ifndef TCNT1L
#define TCNT1L		NO_REG
#endif
#ifndef OCR1AH
#define OCR1AH		NO_REG
#endif
#ifndef OCR1AL
#define OCR1AL		NO_REG
#endif
#ifndef OCR1BH
#define OCR1BH		NO_REG
#endif
#ifndef OCR1BL
#define OCR1BL		NO_REG
#endif
#ifndef ICR1H
#define ICR1H		NO_REG
#endif
#ifndef ICR1L
#define ICR1L		NO_REG
#endif
#ifndef TCCR2
#define TCCR2		NO_REG
#endif
#ifndef TCNT2
#define TCNT2		NO_REG
#endif
#ifndef OCR2
#define OCR2		NO_REG
#endif
#ifndef ASSR
#define ASSR		NO_REG
#endif
#ifndef WDTCR
#define WDTCR		NO_REG
#endif
#ifndef UBRRH
#define UBRRH		NO_REG
#endif
#ifndef UCSRC
#define UCSRC		NO_REG
#endif
#ifndef UDR
#define UDR		NO_REG
#endif
#ifndef UCSRA
#define UCSRA		NO_REG
#endif
#ifndef UCSRB
#define UCSRB		NO_REG
#endif
#ifndef UBRRL
#define UBRRL		NO_REG
#endif
#ifndef ADMUX
#define ADMUX		NO_REG
#endif
#ifndef ADCSRA
#define ADCSRA		NO_REG
#endif
#ifndef ADCH
#define ADCH		NO_REG
#endif
#ifndef ADCL
#define ADCL		NO_REG
#endif
#ifndef TWDR
#define TWDR		NO_REG
#endif
#ifndef TWAR
#define TWAR		NO_REG
#endif
#ifndef TWSR
#define TWSR		NO_REG
#endif
#ifndef TWBR
#define TWBR		NO_REG
#endif
#ifndef UDR3
#define UDR3		NO_REG
#endif
#ifndef UBRR3H
#define UBRR3H		NO_REG
#endif
#ifndef UBRR3L
#define UBRR3L		NO_REG
#endif
#ifndef UCSR3C
#define UCSR3C		NO_REG
#endif
#ifndef UCSR3B
#define UCSR3B		NO_REG
#endif
#ifndef UCSR3A
#define UCSR3A		NO_REG
#endif
#ifndef OCR5CH
#define OCR5CH		NO_REG
#endif
#ifndef OCR5CL
#define OCR5CL		NO_REG
#endif
#ifndef OCR5BH
#define OCR5BH		NO_REG
#endif
#ifndef OCR5BL
#define OCR5BL		NO_REG
#endif
#ifndef OCR5AH
#define OCR5AH		NO_REG
#endif
#ifndef OCR5AL
#define OCR5AL		NO_REG
#endif
#ifndef ICR5H
#define ICR5H		NO_REG
#endif
#ifndef ICR5L
#define ICR5L		NO_REG
#endif
#ifndef TCNT5H
#define TCNT5H		NO_REG
#endif
#ifndef TCNT5L
#define TCNT5L		NO_REG
#endif
#ifndef TCCR5C
#define TCCR5C		NO_REG
#endif
#ifndef TCCR5B
#define TCCR5B		NO_REG
#endif
#ifndef TCCR5A
#define TCCR5A		NO_REG
#endif
#ifndef PORTL
#define PORTL		NO_REG
#endif
#ifndef DDRL
#define DDRL		NO_REG
#endif
#ifndef PINL
#define PINL		NO_REG
#endif
#ifndef PORTK
#define PORTK		NO_REG
#endif
#ifndef DDRK
#define DDRK		NO_REG
#endif
#ifndef PINK
#define PINK		NO_REG
#endif
#ifndef PORTJ
#define PORTJ		NO_REG
#endif
#ifndef DDRJ
#define DDRJ		NO_REG
#endif
#ifndef PINJ
#define PINJ		NO_REG
#endif
#ifndef PORTH
#define PORTH		NO_REG
#endif
#ifndef DDRH
#define DDRH		NO_REG
#endif
#ifndef PINH
#define PINH		NO_REG
#endif
#ifndef UDR2
#define UDR2		NO_REG
#endif
#ifndef UBRR2H
#define UBRR2H		NO_REG
#endif
#ifndef UBRR2L
#define UBRR2L		NO_REG
#endif
#ifndef UCSR2C
#define UCSR2C		NO_REG
#endif
#ifndef UCSR2B
#define UCSR2B		NO_REG
#endif
#ifndef UCSR2A
#define UCSR2A		NO_REG
#endif
#ifndef UDR1
#define UDR1		NO_REG
#endif
#ifndef UBRR1H
#define UBRR1H		NO_REG
#endif
#ifndef UBRR1L
#define UBRR1L		NO_REG
#endif
#ifndef UCSR1C
#define UCSR1C		NO_REG
#endif
#ifndef UCSR1B
#define UCSR1B		NO_REG
#endif
#ifndef UCSR1A
#define UCSR1A		NO_REG
#endif
#ifndef UDR0
#define UDR0		NO_REG
#endif
#ifndef UBRR0H
#define UBRR0H		NO_REG
#endif
#ifndef UBRR0L
#define UBRR0L		NO_REG
#endif
#ifndef UCSR0C
#define UCSR0C		NO_REG
#endif
#ifndef UCSR0B
#define UCSR0B		NO_REG
#endif
#ifndef UCSR0A
#define UCSR0A		NO_REG
#endif
#ifndef TWAMR
#define TWAMR		NO_REG
#endif
#ifndef OCR2B
#define OCR2B		NO_REG
#endif
#ifndef OCR2A
#define OCR2A		NO_REG
#endif
#ifndef TCCR2B
#define TCCR2B		NO_REG
#endif
#ifndef TCCR2A
#define TCCR2A		NO_REG
#endif
#ifndef OCR4CH
#define OCR4CH		NO_REG
#endif
#ifndef OCR4CL
#define OCR4CL		NO_REG
#endif
#ifndef OCR4BH
#define OCR4BH		NO_REG
#endif
#ifndef OCR4BL
#define OCR4BL		NO_REG
#endif
#ifndef OCR4AH
#define OCR4AH		NO_REG
#endif
#ifndef OCR4AL
#define OCR4AL		NO_REG
#endif
#ifndef ICR4H
#define ICR4H		NO_REG
#endif
#ifndef ICR4L
#define ICR4L		NO_REG
#endif
#ifndef TCNT4H
#define TCNT4H		NO_REG
#endif
#ifndef TCNT4L
#define TCNT4L		NO_REG
#endif
#ifndef TCCR4C
#define TCCR4C		NO_REG
#endif
#ifndef TCCR4B
#define TCCR4B		NO_REG
#endif
#ifndef TCCR4A
#define TCCR4A		NO_REG
#endif
#ifndef OCR3CH
#define OCR3CH		NO_REG
#endif
#ifndef OCR3CL
#define OCR3CL		NO_REG
#endif
#ifndef OCR3BH
#define OCR3BH		NO_REG
#endif
#ifndef OCR3BL
#define OCR3BL		NO_REG
#endif
#ifndef OCR3AH
#define OCR3AH		NO_REG
#endif
#ifndef OCR3AL
#define OCR3AL		NO_REG
#endif
#ifndef ICR3H
#define ICR3H		NO_REG
#endif
#ifndef ICR3L
#define ICR3L		NO_REG
#endif
#ifndef TCNT3H
#define TCNT3H		NO_REG
#endif
#ifndef TCNT3L
#define TCNT3L		NO_REG
#endif
#ifndef TCCR3C
#define TCCR3C		NO_REG
#endif
#ifndef TCCR3B
#define TCCR3B		NO_REG
#endif
#ifndef TCCR3A
#define TCCR3A		NO_REG
#endif
#ifndef OCR1CH
#define OCR1CH		NO_REG
#endif
#ifndef OCR1CL
#define OCR1CL		NO_REG
#endif
#ifndef TCCR1C
#define TCCR1C		NO_REG
#endif
#ifndef DIDR1
#define DIDR1		NO_REG
#endif
#ifndef DIDR0
#define DIDR0		NO_REG
#endif
#ifndef DIDR2
#define DIDR2		NO_REG
#endif
#ifndef ADCSRB
#define ADCSRB		NO_REG
#endif
#ifndef XMCRB
#define XMCRB		NO_REG
#endif
#ifndef XMCRA
#define XMCRA		NO_REG
#endif
#ifndef TIMSK5
#define TIMSK5		NO_REG
#endif
#ifndef TIMSK4
#define TIMSK4		NO_REG
#endif
#ifndef TIMSK3
#define TIMSK3		NO_REG
#endif
#ifndef TIMSK2
#define TIMSK2		NO_REG
#endif
#ifndef TIMSK1
#define TIMSK1		NO_REG
#endif
#ifndef TIMSK0
#define TIMSK0		NO_REG
#endif
#ifndef PCMSK2
#define PCMSK2		NO_REG
#endif
#ifndef PCMSK1
#define PCMSK1		NO_REG
#endif
#ifndef PCMSK0
#define PCMSK0		NO_REG
#endif
#ifndef EICRB
#define EICRB		NO_REG
#endif
#ifndef EICRA
#define EICRA		NO_REG
#endif
#ifndef PCICR
#define PCICR		NO_REG
#endif
#ifndef PRR1
#define PRR1		NO_REG
#endif
#ifndef PRR0
#define PRR0		NO_REG
#endif
#ifndef CLKPR
#define CLKPR		NO_REG
#endif
#ifndef WDTCSR
#define WDTCSR		NO_REG
#endif
#ifndef SPCR0
#define SPCR0		NO_REG
#endif
#ifndef SPSR0
#define SPSR0		NO_REG
#endif
#ifndef DWDR
#define DWDR		NO_REG
#endif
#ifndef PRR
#define PRR		NO_REG
#endif
/*
 * Available MCU registers to include into VCD dump.
 * See mcusim/avr/sim/vcd_dump.h for details.
 *
 * Format: <regname>, <offset_in_ram>, NULL, 0
 */
#define VCD_DUMP_REGS {							\
	{ "SREG",	SREG, NULL, 0 },				\
	{ "SPH",	SPH, NULL, 0 },					\
	{ "SPL",	SPL, NULL, 0 },					\
									\
	{ "PORTA",	PORTA, NULL, 0 },				\
	{ "DDRA",	DDRA, NULL, 0 },				\
	{ "PINA",	PINA, NULL, 0 },				\
	{ "PORTB",	PORTB, NULL, 0 },				\
	{ "DDRB",	DDRB, NULL, 0 },				\
	{ "PINB",	PINB, NULL, 0 },				\
	{ "PORTC",	PORTC, NULL, 0 },				\
	{ "DDRC",	DDRC, NULL, 0 },				\
	{ "PINC",	PINC, NULL, 0 },				\
	{ "PORTD",	PORTD, NULL, 0 },				\
	{ "DDRD",	DDRD, NULL, 0 },				\
	{ "PIND",	PIND, NULL, 0 },				\
	{ "PORTE",	PORTE, NULL, 0 },				\
	{ "DDRE",	DDRE, NULL, 0 },				\
	{ "PINE",	PINE, NULL, 0 },				\
	{ "PORTF",	PORTF, NULL, 0 },				\
	{ "DDRF",	DDRF, NULL, 0 },				\
	{ "PINF",	PINF, NULL, 0 },				\
	{ "PORTG",	PORTG, NULL, 0 },				\
	{ "DDRG",	DDRG, NULL, 0 },				\
	{ "PING",	PING, NULL, 0 },				\
									\
	{ "EIND",	EIND, NULL, 0 },				\
	{ "RAMPZ",	RAMPZ, NULL, 0 },				\
	{ "SPMCSR",	SPMCSR, NULL, 0 },				\
	{ "MCUCR",	MCUCR, NULL, 0 },				\
	{ "MCUSR",	MCUSR, NULL, 0 },				\
	{ "SMCR",	SMCR, NULL, 0 },				\
	{ "OCDR",	OCDR, NULL, 0 },				\
	{ "ACSR",	ACSR, NULL, 0 },				\
	{ "SPDR",	SPDR, NULL, 0 },				\
	{ "SPSR",	SPSR, NULL, 0 },				\
	{ "SPCR",	SPCR, NULL, 0 },				\
	{ "GPIOR2",	GPIOR2, NULL, 0 },				\
	{ "GPIOR1",	GPIOR1, NULL, 0 },				\
	{ "GPIOR0",	GPIOR0, NULL, 0 },				\
									\
	{ "OCR0B",	OCR0B, NULL, 0 },				\
	{ "OCR0A",	OCR0A, NULL, 0 },				\
	{ "TCNT0",	TCNT0, NULL, 0 },				\
	{ "TCCR0B",	TCCR0B, NULL, 0 },				\
	{ "TCCR0A",	TCCR0A, NULL, 0 },				\
									\
	{ "GTCCR",	GTCCR, NULL, 0 },				\
	{ "EEARH",	EEARH, NULL, 0 },				\
	{ "EEARL",	EEARL, NULL, 0 },				\
	{ "EEDR",	EEDR, NULL, 0 },				\
	{ "EECR",	EECR, NULL, 0 },				\
	{ "EIMSK",	EIMSK, NULL, 0 },				\
	{ "EIFR",	EIFR, NULL, 0 },				\
	{ "PCIFR",	PCIFR, NULL, 0 },				\
									\
	{ "TIFR5",	TIFR5, NULL, 0 },				\
	{ "TIFR4",	TIFR4, NULL, 0 },				\
	{ "TIFR3",	TIFR3, NULL, 0 },				\
	{ "TIFR2",	TIFR2, NULL, 0 },				\
	{ "TIFR1",	TIFR1, NULL, 0 },				\
	{ "TIFR0",	TIFR0, NULL, 0 },				\
	{ "TIFR",	TIFR, NULL, 0 },				\
									\
	{ "GICR",	GICR, NULL, 0 },				\
	{ "GIFR",	GIFR, NULL, 0 },				\
	{ "TIMSK",	TIMSK, NULL, 0 },				\
	{ "SPMCR",	SPMCR, NULL, 0 },				\
	{ "TWCR",	TWCR, NULL, 0 },				\
	{ "MCUCSR",	MCUCSR, NULL, 0 },				\
	{ "OSCCAL",	OSCCAL, NULL, 0 },				\
	{ "SFIOR",	SFIOR, NULL, 0 },				\
	{ "TCCR1A",	TCCR1A, NULL, 0 },				\
	{ "TCCR1B",	TCCR1B, NULL, 0 },				\
	{ "TCNT1H",	TCNT1H, NULL, 0 },				\
	{ "TCNT1L",	TCNT1L, NULL, 0 },				\
	{ "OCR1AH",	OCR1AH, NULL, 0 },				\
	{ "OCR1AL",	OCR1AL, NULL, 0 },				\
	{ "OCR1BH",	OCR1BH, NULL, 0 },				\
	{ "OCR1BL",	OCR1BL, NULL, 0 },				\
	{ "ICR1H",	ICR1H, NULL, 0 },				\
	{ "ICR1L",	ICR1L, NULL, 0 },				\
	{ "TCCR2",	TCCR2, NULL, 0 },				\
	{ "TCNT2",	TCNT2, NULL, 0 },				\
	{ "OCR2",	OCR2, NULL, 0 },				\
	{ "ASSR",	ASSR, NULL, 0 },				\
	{ "WDTCR",	WDTCR, NULL, 0 },				\
	{ "UBRRH",	UBRRH, NULL, 0 },				\
	{ "UCSRC",	UCSRC, NULL, 0 },				\
	{ "UDR",	UDR, NULL, 0 },					\
	{ "UCSRA",	UCSRA, NULL, 0 },				\
	{ "UCSRB",	UCSRB, NULL, 0 },				\
	{ "UBRRL",	UBRRL, NULL, 0 },				\
	{ "ADMUX",	ADMUX, NULL, 0 },				\
	{ "ADCSRA",	ADCSRA, NULL, 0 },				\
	{ "ADCH",	ADCH, NULL, 0 },				\
	{ "ADCL",	ADCL, NULL, 0 },				\
	{ "TWDR",	TWDR, NULL, 0 },				\
	{ "TWAR",	TWAR, NULL, 0 },				\
	{ "TWSR",	TWSR, NULL, 0 },				\
	{ "TWBR",	TWBR, NULL, 0 },				\
									\
	{ "UDR3",	UDR3, NULL, 0 },				\
	{ "UBRR3H",	UBRR3H, NULL, 0 },				\
	{ "UBRR3L",	UBRR3L, NULL, 0 },				\
	{ "UCSR3C",	UCSR3C, NULL, 0 },				\
	{ "UCSR3B",	UCSR3B, NULL, 0 },				\
	{ "UCSR3A",	UCSR3A, NULL, 0 },				\
	{ "OCR5CH",	OCR5CH, NULL, 0 },				\
	{ "OCR5CL",	OCR5CL, NULL, 0 },				\
	{ "OCR5BH",	OCR5BH, NULL, 0 },				\
	{ "OCR5BL",	OCR5BL, NULL, 0 },				\
	{ "OCR5AH",	OCR5AH, NULL, 0 },				\
	{ "OCR5AL",	OCR5AL, NULL, 0 },				\
	{ "ICR5H",	ICR5H, NULL, 0 },				\
	{ "ICR5L",	ICR5L, NULL, 0 },				\
	{ "TCNT5H",	TCNT5H, NULL, 0 },				\
	{ "TCNT5L",	TCNT5L, NULL, 0 },				\
	{ "TCCR5C",	TCCR5C, NULL, 0 },				\
	{ "TCCR5B",	TCCR5B, NULL, 0 },				\
	{ "TCCR5A",	TCCR5A, NULL, 0 },				\
	{ "PORTL",	PORTL, NULL, 0 },				\
	{ "DDRL",	DDRL, NULL, 0 },				\
	{ "PINL",	PINL, NULL, 0 },				\
	{ "PORTK",	PORTK, NULL, 0 },				\
	{ "DDRK",	DDRK, NULL, 0 },				\
	{ "PINK",	PINK, NULL, 0 },				\
	{ "PORTJ",	PORTJ, NULL, 0 },				\
	{ "DDRJ",	DDRJ, NULL, 0 },				\
	{ "PINJ",	PINJ, NULL, 0 },				\
	{ "PORTH",	PORTH, NULL, 0 },				\
	{ "DDRH",	DDRH, NULL, 0 },				\
	{ "PINH",	PINH, NULL, 0 },				\
	{ "UDR2",	UDR2, NULL, 0 },				\
	{ "UBRR2H",	UBRR2H, NULL, 0 },				\
	{ "UBRR2L",	UBRR2L, NULL, 0 },				\
	{ "UCSR2C",	UCSR2C, NULL, 0 },				\
	{ "UCSR2B",	UCSR2B, NULL, 0 },				\
	{ "UCSR2A",	UCSR2A, NULL, 0 },				\
	{ "UDR1",	UDR1, NULL, 0 },				\
	{ "UBRR1H",	UBRR1H, NULL, 0 },				\
	{ "UBRR1L",	UBRR1L, NULL, 0 },				\
	{ "UCSR1C",	UCSR1C, NULL, 0 },				\
	{ "UCSR1B",	UCSR1B, NULL, 0 },				\
	{ "UCSR1A",	UCSR1A, NULL, 0 },				\
	{ "UDR0",	UDR0, NULL, 0 },				\
	{ "UBRR0H",	UBRR0H, NULL, 0 },				\
	{ "UBRR0L",	UBRR0L, NULL, 0 },				\
	{ "UCSR0C",	UCSR0C, NULL, 0 },				\
	{ "UCSR0B",	UCSR0B, NULL, 0 },				\
	{ "UCSR0A",	UCSR0A, NULL, 0 },				\
	{ "TWAMR",	TWAMR, NULL, 0 },				\
	{ "OCR2B",	OCR2B, NULL, 0 },				\
	{ "OCR2A",	OCR2A, NULL, 0 },				\
	{ "TCCR2B",	TCCR2B, NULL, 0 },				\
	{ "TCCR2A",	TCCR2A, NULL, 0 },				\
	{ "OCR4CH",	OCR4CH, NULL, 0 },				\
	{ "OCR4CL",	OCR4CL, NULL, 0 },				\
	{ "OCR4BH",	OCR4BH, NULL, 0 },				\
	{ "OCR4BL",	OCR4BL, NULL, 0 },				\
	{ "OCR4AH",	OCR4AH, NULL, 0 },				\
	{ "OCR4AL",	OCR4AL, NULL, 0 },				\
	{ "ICR4H",	ICR4H, NULL, 0 },				\
	{ "ICR4L",	ICR4L, NULL, 0 },				\
	{ "TCNT4H",	TCNT4H, NULL, 0 },				\
	{ "TCNT4L",	TCNT4L, NULL, 0 },				\
	{ "TCCR4C",	TCCR4C, NULL, 0 },				\
	{ "TCCR4B",	TCCR4B, NULL, 0 },				\
	{ "TCCR4A",	TCCR4A, NULL, 0 },				\
	{ "OCR3CH",	OCR3CH, NULL, 0 },				\
	{ "OCR3CL",	OCR3CL, NULL, 0 },				\
	{ "OCR3BH",	OCR3BH, NULL, 0 },				\
	{ "OCR3BL",	OCR3BL, NULL, 0 },				\
	{ "OCR3AH",	OCR3AH, NULL, 0 },				\
	{ "OCR3AL",	OCR3AL, NULL, 0 },				\
	{ "ICR3H",	ICR3H, NULL, 0 },				\
	{ "ICR3L",	ICR3L, NULL, 0 },				\
	{ "TCNT3H",	TCNT3H, NULL, 0 },				\
	{ "TCNT3L",	TCNT3L, NULL, 0 },				\
	{ "TCCR3C",	TCCR3C, NULL, 0 },				\
	{ "TCCR3B",	TCCR3B, NULL, 0 },				\
	{ "TCCR3A",	TCCR3A, NULL, 0 },				\
	{ "OCR1CH",	OCR1CH, NULL, 0 },				\
	{ "OCR1CL",	OCR1CL, NULL, 0 },				\
	{ "TCCR1C",	TCCR1C, NULL, 0 },				\
	{ "DIDR1",	DIDR1, NULL, 0 },				\
	{ "DIDR0",	DIDR0, NULL, 0 },				\
	{ "DIDR2",	DIDR2, NULL, 0 },				\
	{ "ADCSRB",	ADCSRB, NULL, 0 },				\
	{ "XMCRB",	XMCRB, NULL, 0 },				\
	{ "XMCRA",	XMCRA, NULL, 0 },				\
	{ "TIMSK5",	TIMSK5, NULL, 0 },				\
	{ "TIMSK4",	TIMSK4, NULL, 0 },				\
	{ "TIMSK3",	TIMSK3, NULL, 0 },				\
	{ "TIMSK2",	TIMSK2, NULL, 0 },				\
	{ "TIMSK1",	TIMSK1, NULL, 0 },				\
	{ "TIMSK0",	TIMSK0, NULL, 0 },				\
	{ "PCMSK2",	PCMSK2, NULL, 0 },				\
	{ "PCMSK1",	PCMSK1, NULL, 0 },				\
	{ "PCMSK0",	PCMSK0, NULL, 0 },				\
	{ "EICRB",	EICRB, NULL, 0 },				\
	{ "EICRA",	EICRA, NULL, 0 },				\
	{ "PCICR",	PCICR, NULL, 0 },				\
	{ "PRR1",	PRR1, NULL, 0 },				\
	{ "PRR0",	PRR0, NULL, 0 },				\
	{ "CLKPR",	CLKPR, NULL, 0 },				\
	{ "WDTCSR",	WDTCSR, NULL, 0 },				\
	{ "SPCR0",	SPCR0, NULL, 0 },				\
	{ "SPSR0",	SPSR0, NULL, 0 },				\
	{ "DWDR",	DWDR, NULL, 0 },				\
	{ "PRR",	PRR, NULL, 0 }					\
}
