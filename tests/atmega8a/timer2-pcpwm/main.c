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

static volatile int ocr2_up;

int
main(void)
{
	timer2_init();
	sei();
	while (1) {
	}
	return 0;
}

void
timer2_init(void)
{
	/* Non-inverting compare match output mode: COM21:0 = 2 */
	TCCR2 |= (1<<COM21);
	TCCR2 &= ~(1<<COM20);
	/* Phase Correct PWM mode, enable PWM waveform output */
	TCCR2 |= (1<<WGM20);
	TCCR2 &= ~(1<<WGM21);
	DDRB |= (1<<PB3);

	TCNT2 = 0;			/* Counter to 0 */
	OCR2 = 255;			/* Select duty cycle */
	ocr2_up = 0;			/* Increment OCR2? - yes*/
	/* Enable overflow interrupt */
	TIMSK |= (1<<TOIE2);

	/* Start timer, prescaler to Fclk_io/8: CS21:0 = 2 */
	TCCR2 &= ~(1<<CS22);
	TCCR2 |= (1<<CS21);
	TCCR2 &= ~(1<<CS20);
}

ISR(TIMER2_OVF_vect)
{
	if (ocr2_up) {
		OCR2 += (unsigned char)5;
		if (OCR2 == 255) {
			ocr2_up = 0;
		}
	} else {
		OCR2 -= (unsigned char)5;
		if (OCR2 == 0) {
			ocr2_up = 1;
		}
	}
}
