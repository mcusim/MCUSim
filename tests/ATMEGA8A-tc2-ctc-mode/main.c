#define F_CPU			16000000UL
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

void timer2_init(void);

static volatile int cmp_count;

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
