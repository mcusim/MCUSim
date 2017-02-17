/*
 * We would like to include headers specific to the
 * ATMega8A microcontroller.
 */
#define __AVR_ATmega8A__ 1

#include <avr/io.h>

#include "avr/sim.h"

enum init_state m8a_init(struct avr *mcu)
{
	if (!mcu)
		return NULL_MCU;

	mcu->ramstart = RAMSTART;
	mcu->ramend = RAMEND;
	mcu->ramsize = RAMSIZE;

	return INITIALIZED;
}
