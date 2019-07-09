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

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

#define DHT_PORT_OUT		PORTB
#define DHT_PORT_IN		PINB
#define DHT_PIN			PB0

void timer0_init(void);

static volatile unsigned char intn = 0;

int
main(void)
{
	timer0_init();
	sei();

	while (1) {}
	return 0;
}

void
timer0_init(void)
{
	/* External clock source on T0 pin, rising edge. */
	TCCR0 |= (1<<CS02)|(1<<CS01)|(1<<CS00);
	TCNT0 = 0;			/* Counter to 0 */
	TIMSK |= (1<<TOIE0);		/* Enable overflow interrupt */
}

ISR(TIMER0_OVF_vect)
{
	if (intn == 255) {
		intn = 0;
	} else {
		intn++;
	}
}
