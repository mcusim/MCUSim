#define F_CPU			16000000UL

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

void timer1_init(void);

static volatile int ocr1a_up;

int main(void)
{
	timer1_init();
	sei();

	while (1) {
	}
	return 0;
}

void timer1_init(void)
{
	uint8_t tccr1b;

	/* Clear OC1A and OC1B on Compare Match and set OC1A and OC1B at
	 * BOTTOM (non-inverting mode): COM1A1:0 = 2, COM1B1:0 = 2. */
	TCCR1A |= (1<<COM1A1);
	TCCR1A &= (uint8_t)(~(1<<COM1A0));
	TCCR1A |= (1<<COM1B1);
	TCCR1A &= (uint8_t)(~(1<<COM1B0));

	/* Fast PWM mode with OCR1A value as a TOP,
	 * enable PWM waveform output */
	TCCR1B |= (1<<WGM13)|(1<<WGM12);
	TCCR1A |= (1<<WGM11)|(1<<WGM10);
	DDRB |= (1<<PB1)|(1<<PB2);

	/* Set initial values */
	TCNT1 = 0x0000U;		/* Counter to 0 */
	OCR1A = 0x05FFU;		/* Select duty cycle */
	OCR1B = 0x0400U;
	ocr1a_up = 1;			/* Increment OCR value by default? */
	TIMSK |= (1<<TOIE1);		/* Enable OVF interrupt */
	//TIMSK |= (1<<OCF1A);		/* Enable Output Compare A Match */
	//TIMSK |= (1<<OCF1B);		/* Enable Output Compare B Match */

	/* Start timer, prescaler to Fclk_io/8: CS12:0 = 2 */
	tccr1b = TCCR1B;
	tccr1b &= (uint8_t)(~(1<<CS12));
	tccr1b |= (1<<CS11);
	tccr1b &= (uint8_t)(~(1<<CS10));
	TCCR1B = tccr1b;
}

ISR(TIMER1_OVF_vect)
{
	if (ocr1a_up == 1) {
		OCR1B += 0x0010;
		if (OCR1B >= 0x05FF) {
			ocr1a_up = 0;
		}
	} else {
		OCR1B -= 0x0010;
		if (OCR1B == 0x0000) {
			ocr1a_up = 1;
		}
	}
}
