#define F_CPU			16000000UL

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

#define DHT_PORT_OUT		PORTB
#define DHT_PORT_IN		PINB
#define DHT_PIN			PB0

void timer0_init(void);

static volatile unsigned char intn;

int main(void)
{
	intn = 0;

	timer0_init();
	sei();
	while (1) {
	}
	return 0;
}

void timer0_init(void)
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
