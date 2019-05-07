#define F_CPU		16000000UL
#define __AVR_ATmega8A__ 1
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(byte, bit)	((byte)|=(1UL<<(bit)))
#define CLEAR_BIT(byte, bit)	((byte)&=~(1UL<<(bit)))
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))

int main(void)
{
	char cb;

	DDRD = 0;
	SET_BIT(DDRD, PD1);
	SET_BIT(PORTD, PD1);

	cb = 1;
	while (1) {
		if (cb) {
			CLEAR_BIT(PORTD, PD1);
			cb = 0;
		} else {
			SET_BIT(PORTD, PD1);
			cb = 1;
		}
	}

	return 0;
}
