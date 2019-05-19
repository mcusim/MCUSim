/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

int main(void)
{
	timer1_init();
	sei();

	while (1) {}
	return 0;
}

void timer1_init(void)
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
