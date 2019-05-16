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

void timer2_init(void);

static volatile int ocr2_up;

int main(void)
{
	timer2_init();
	sei();
	while (1) {
	}
	return 0;
}

void timer2_init(void)
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
