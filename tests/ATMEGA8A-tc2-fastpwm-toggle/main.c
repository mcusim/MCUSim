#define F_CPU			16000000UL

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

void timer2_init(void);

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
	/* Compare match toggle output mode: COM21:0 = 1 */
	TCCR2 &= ~(1<<COM21);
	TCCR2 |= (1<<COM20);
	/* Fast PWM mode, enable PWM waveform output */
	TCCR2 |= (1<<WGM21)|(1<<WGM20);
	DDRB |= (1<<PB3);

	TCNT2 = 0;			/* Counter to 0 */
	OCR2 = 60;			/* Select duty cycle */
	/* Enable overflow interrupt */
	TIMSK |= (1<<TOIE2);

	/* Start timer, prescaler to Fclk_io/8: CS21:0 = 2 */
	TCCR2 &= ~(1<<CS22);
	TCCR2 |= (1<<CS21);
	TCCR2 &= ~(1<<CS20);
}

ISR(TIMER2_OVF_vect)
{
	OCR2 = 60;			/* Select duty cycle */
}
