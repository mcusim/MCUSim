/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_SIMIO_H_
#define MSIM_AVR_SIMIO_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* Indexes of the AVR I/O registers addresses */
enum {
	TWBR_ADDRI, /* Two-wire Serial Interface Bit Rate Register */
	TWSR_ADDRI,
	TWAR_ADDRI,
	TWDR_ADDRI, /* Two-wire Serial Interface Data Register */
	ADCL_ADDRI, /* ADC Data Register Low byte */
	ADCH_ADDRI, /* ADC Data Register High byte */
	ADCSRA_ADDRI,
	ADMUX_ADDRI,
	ACSR_ADDRI,
	UBRRL_ADDRI, /* USART Baud Rate Register Low byte */
	UCSRB_ADDRI,
	UCSRA_ADDRI,
	UDR_ADDRI, /* USART I/O Data Register */
	SPCR_ADDRI,
	SPSR_ADDRI,
	SPDR_ADDRI, /* SPI Data Register */
	PIND_ADDRI, /* Port D */
	DDRD_ADDRI,
	PORTD_ADDRI,
	PINC_ADDRI, /* Port C */
	DDRC_ADDRI,
	PORTC_ADDRI,
	PINB_ADDRI, /* Port B */
	DDRB_ADDRI,
	PORTB_ADDRI,
	EECR_ADDRI,
	EEDR_ADDRI, /* EEPROM Data Register */
	EEARL_ADDRI,
	EEARH_ADDRI,
	UCSRC_ADDRI,
	UBRRH_ADDRI,
	WDTCR_ADDRI,
	ASSR_ADDRI,
	OCR2_ADDRI, /* Timer/Counter2 Output Compare Register */
	TCNT2_ADDRI, /* Timer/Counter2 (8 Bits) */
	TCCR2_ADDRI,
	ICR1L_ADDRI, /* Timer/Counter1 – Input Capture Register Low byte */
	ICR1H_ADDRI, /* Timer/Counter1 – Input Capture Register High byte */
	OCR1BL_ADDRI, /* Timer/Counter1 – Output Compare Register B Low byte */
	OCR1BH_ADDRI, /* Timer/Counter1 – Output Compare Register B High byte */
	OCR1AL_ADDRI, /* Timer/Counter1 – Output Compare Register A Low byte */
	OCR1AH_ADDRI, /* Timer/Counter1 – Output Compare Register A High byte */
	TCNT1L_ADDRI, /* Timer/Counter1 – Counter Register Low byte */
	TCNT1H_ADDRI, /* Timer/Counter1 – Counter Register High byte */
	TCCR1B_ADDRI,
	TCCR1A_ADDRI,
	SFIOR_ADDRI,
	OSCCAL_ADDRI, /* Oscillator Calibration Register */
	TCNT0_ADDRI, /* Timer/Counter0 (8 Bits) */
	TCCR0_ADDRI,
	MCUCSR_ADDRI,
	MCUCR_ADDRI,
	TWCR_ADDRI,
	SPMCR_ADDRI,
	TIFR_ADDRI,
	TIMSK_ADDRI,
	GIFR_ADDRI,
	GICR_ADDRI,
	SPL_ADDRI,
	SPH_ADDRI,
	SREG_ADDRI,
	TIFR0_ADDRI,
	TIFR1_ADDRI,
	TIFR2_ADDRI,
	PCIFR_ADDRI,
	EIFR_ADDRI,
	EIMSK_ADDRI,
	GPIOR0_ADDRI,
	GPIOR1_ADDRI,
	GPIOR2_ADDRI,
	GTCCR_ADDRI,
	TCCR0A_ADDRI,
	TCCR0B_ADDRI,
	OCR0A_ADDRI,
	OCR0B_ADDRI,
	SMCR_ADDRI,
	MCUSR_ADDRI,
	SPMCSR_ADDRI,

	IO_REGS /* Number of all I/O registers defined above */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIMIO_H_ */
