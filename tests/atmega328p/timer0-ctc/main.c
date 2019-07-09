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
#define F_CPU 16000000UL

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

void timer0_init(void);

int
main(void)
{
	/* Set PIN5 and PIN6 as output */
	SET_BIT(DDRD, PD5);
	SET_BIT(DDRD, PD6);

	timer0_init();
	sei();

	/* road to infinit */
	while (1);

	return 0;
}

void
timer0_init(void)
{
	/* Compare match toggle output, channel A*/
	SET_BIT(TCCR0A, COM0A0);
	CLEAR_BIT(TCCR0A, COM0A1);

	/* Compare match toggle output, channel B*/
	SET_BIT(TCCR0A, COM0B0);
	CLEAR_BIT(TCCR0A, COM0B1);

	/* Waveform generation mode CTC enable */ //WGM02 = 0, WGM01 = 1, WGM00 = 0
	CLEAR_BIT(TCCR0B, WGM02);
	SET_BIT(TCCR0A, WGM01);
	CLEAR_BIT(TCCR0A, WGM00);

	/* Enable overflow interrupt */
	SET_BIT(TIMSK0, TOIE0); //TOIE0 or TOIE?

	/* Start timer, prescaler to clk/64 */
	SET_BIT(TCCR0B, CS00);
	SET_BIT(TCCR0B, CS01);
	CLEAR_BIT(TCCR0B, CS02);

	/* Set duty cycle, count from 0 | output freq = 625 Hz*/
	OCR0A = 199;
	OCR0B = 199;
}

ISR(TIMER0_OVF_vect)
{
	/* Update duty cycle */
	OCR0A = 199;
	OCR0B = 199;
}
