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

void timer2_init(void);

static volatile int cmp_count;

int
main(void)
{
	/* Configure Timer/Counter2 before start. Global Interrupt Enable
	 * bit is cleared by default (SREG I bit = 0). */
	timer2_init();

	/* Enable interrupts. */
	sei();
	/* Do nothing while waiting for the Timer/Counter2 interrupts. */
	while (1) {
	}
	return 0;
}

void
timer2_init(void)
{
	/* CTC mode (WGM21:0 = 2), enable output on the pin */
	TCCR2 |= (1<<WGM21);
	TCCR2 &= ~(1<<WGM20);
	DDRB |= (1<<PB3);
	/* Toggle OC2 (or PB3) on compare match: COM21:0 = 1 */
	TCCR2 &= ~(1<<COM21);
	TCCR2 |= (1<<COM20);

	cmp_count = 0;
	TCNT2 = 0;			/* Counter to 0 */
	OCR2 = 175;			/* Select duty cycle */
	TIMSK |= (1<<OCIE2);		/* Enable OCIE2 interrupt */

	/* Start timer, prescaler to Fclk_io/8: CS21:0 = 2 */
	TCCR2 &= ~(1<<CS22);
	TCCR2 |= (1<<CS21);
	TCCR2 &= ~(1<<CS20);
}

ISR(TIMER2_COMP_vect)
{
	if (cmp_count < 3) {
		cmp_count++;
	} else {
		/* Update generated frequency after 4 interrupts */
		OCR2 = 67;
	}
}
