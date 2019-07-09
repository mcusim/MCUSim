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

void timer1_init(void);

static volatile int ocr_up;
static volatile int compa, compb;

int
main(void)
{
	timer1_init();
	sei();

	while (1) {}
	return 0;
}

void
timer1_init(void)
{
	/* Channel A: Disconnected: COM1A1:0 = 0
	 * Channel B: Non-inverting compare match mode: COM1B1:0 = 2 */
	TCCR1A &= (uint8_t)(~(1<<COM1A1));
	TCCR1A &= (uint8_t)(~(1<<COM1A0));
	TCCR1A |= (1<<COM1B1);
	TCCR1A &= (uint8_t)(~(1<<COM1B0));

	/* Phase Correct PWM (OCR1A as TOP): WGM13:0 = 11 */
	TCCR1B |= (1<<WGM13);
	TCCR1B &= (uint8_t)(~(1<<WGM12));
	TCCR1A |= (1<<WGM11);
	TCCR1A |= (1<<WGM10);

	DDRB |= (1<<PB2);		/* Enable PWM output on PB2 */
	TCNT1 = 0;			/* Counter to 0 */
	OCR1A = 1024;			/* Select duty cycle */
	OCR1B = 896;
	ocr_up = 0;			/* Increment OCR?*/
	TIMSK1 |= (1<<TOIE1);		/* Enable overflow interrupt */
	TIMSK1 |= (1<<OCIE1A);		/* Enable OC interrupt (A channel) */
	TIMSK1 |= (1<<OCIE1B);		/* Enable OC interrupt (B channel) */

	/* Start timer, prescaler to Fclk_io/8: CS12:0 = 2 */
	TCCR1B &= (uint8_t)(~(1<<CS12));
	TCCR1B |= (1<<CS11);
	TCCR1B &= (uint8_t)(~(1<<CS10));
}

ISR(TIMER1_OVF_vect)
{
	if (ocr_up) {
		OCR1B += (uint8_t)128;
		if (OCR1B == 1024) {
			ocr_up = 0;
		}
	} else {
		OCR1B -= (uint8_t)128;
		if (OCR1B == 128) {
			ocr_up = 1;
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	compa = 1;
}

ISR(TIMER1_COMPB_vect)
{
	compb = 1;
}
