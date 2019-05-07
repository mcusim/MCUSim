#define F_CPU			16000000UL
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

#define DHT_PORT_OUT		PORTB
#define DHT_PORT_IN		PINB
#define DHT_PIN			PB0

void timer0_init(void);

static volatile unsigned char intn = 0;

int main(void)
{
	timer0_init();
	sei();
	while (1) {
	}

	return 0;
}

void timer0_init(void)
{
	TCNT0 = 0;			/* Counter to 0 */
	TIMSK |= (1<<TOIE0);		/* Enable overflow interrupt */

	/* Clock source is clk_io/8 */
	TCCR0 &= (uint8_t)(~(1<<CS02));
	TCCR0 |=  (1<<CS01);
	TCCR0 &= (uint8_t)(~(1<<CS00));
}

ISR(TIMER0_OVF_vect)
{
	if (intn == 255) {
		intn = 0;
	} else {
		intn++;
	}
}
