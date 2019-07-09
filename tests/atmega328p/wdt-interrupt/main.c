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
#define F_CPU			16000000UL
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

static void
wdt_init(void);

int
main(void)
{
	/* Initialize WDT */
	wdt_init();

	/*
	 * Let's run an infinite loop in the main program's body.
	 * All of the interesting stuff will be done in the WDT ISR.
	 */
	while (1) {}

	return 0;
}

static void
wdt_init(void)
{
	/* Disable interupts */
	cli();

	/* Prepare WDT to be changed */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* Enable WDT interrupt only(!) with 4s timeout */
	WDTCSR |= (1<<WDIE) | (1<<WDP3);
	WDTCSR &= (uint8_t)(~(1<<WDE));

	/* Enable interupts */
	sei();
}

ISR(WDT_vect)
{
}
