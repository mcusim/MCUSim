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
#ifndef SREG
#define SREG		NO_REG
#endif
#ifndef SPH
#define SPH		NO_REG
#endif
#ifndef SPL
#define SPL		NO_REG
#endif

#ifndef PORTB
#define PORTB		NO_REG
#endif
#ifndef DDRB
#define DDRB		NO_REG
#endif
#ifndef PINB
#define PINB		NO_REG
#endif

#ifndef PORTC
#define PORTC		NO_REG
#endif
#ifndef DDRC
#define DDRC		NO_REG
#endif
#ifndef PINC
#define PINC		NO_REG
#endif

#ifndef PORTD
#define PORTD		NO_REG
#endif
#ifndef DDRD
#define DDRD		NO_REG
#endif
#ifndef PIND
#define PIND		NO_REG
#endif

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
	{ "PORTB",	PORTB, NULL, 0 },				\
	{ "DDRB",	DDRB, NULL, 0 },				\
	{ "PINB",	PINB, NULL, 0 },				\
	{ "PORTC",	PORTC, NULL, 0 },				\
	{ "DDRC",	DDRC, NULL, 0 },				\
	{ "PINC",	PINC, NULL, 0 },				\
	{ "PORTD",	PORTD, NULL, 0 },				\
	{ "DDRD",	DDRD, NULL, 0 },				\
	{ "PIND",	PIND, NULL, 0 },				\
	{ "EIND",	EIND, NULL, 0 },				\
	{ "RAMPZ",	RAMPZ, NULL, 0 },				\
	{ "SPMCSR",	SPMCSR, NULL, 0 },				\
	{ "MCUCR",	MCUCR, NULL, 0 },				\
	{ "MCUSR",	MCUSR, NULL, 0 },				\
	{ "SMCR",	SMCR, NULL, 0 },				\
	{ "OCDR",	OCDR, NULL, 0 },				\
	{ "ACSR",	ACSR, NULL, 0 },				\
	{ "SPDR",	SPDR, NULL, 0 },				\
	{ "SPSR",	SPSR, NULL, 0 }					\
}
