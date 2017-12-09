/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
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

#define NO_REG		-1

/*
 * It's a common situation whether one AVR MCU has a set of registers, but
 * another one - doesn't. Memory offset of the unavailable registers
 * will be marked as NO_REG.
 */
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
	{ "TIFR5",	TIFR5, NULL, 0 },				\
	{ "TIFR4",	TIFR4, NULL, 0 },				\
	{ "TIFR3",	TIFR3, NULL, 0 },				\
	{ "TIFR2",	TIFR2, NULL, 0 },				\
	{ "TIFR1",	TIFR1, NULL, 0 },				\
	{ "TIFR0",	TIFR0, NULL, 0 }				\
}
