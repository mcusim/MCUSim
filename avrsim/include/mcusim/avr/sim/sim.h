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
#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

#include <stdint.h>

#include "mcusim/avr/sim/bootloader.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Indexes of the AVR I/O registers addresses in the MSIM_AVR structure */
#define SREG_ADDRI		0x3F
#define SPH_ADDRI		0x3E
#define SPL_ADDRI		0x3D
/*	Reserved		0x3C */
#define GICR_ADDRI		0x3B
#define GIFR_ADDRI		0x3A
#define TIMSK_ADDRI		0x39
#define TIFR_ADDRI		0x38
#define SPMCR_ADDRI		0x37
#define TWCR_ADDRI		0x36
#define MCUCR_ADDRI		0x35
#define MCUCSR_ADDRI		0x34
#define TCCR0_ADDRI		0x33
#define TCNT0_ADDRI		0x32	/* Timer/Counter0 (8 Bits) */
#define OSCCAL_ADDRI		0x31	/* Oscillator Calibration Register */
#define SFIOR_ADDRI		0x30
#define TCCR1A_ADDRI		0x2F
#define TCCR1B_ADDRI		0x2E
#define TCNT1H_ADDRI		0x2D	/* Timer/Counter1 – Counter Register
					   High byte */
#define TCNT1L_ADDRI		0x2C	/* Timer/Counter1 – Counter Register
					   Low byte */
#define OCR1AH_ADDRI		0x2B	/* Timer/Counter1 – Output Compare
					   Register A High byte */
#define OCR1AL_ADDRI		0x2A	/* Timer/Counter1 – Output Compare
					   Register A Low byte */
#define OCR1BH_ADDRI		0x29	/* Timer/Counter1 – Output Compare
					   Register B High byte */
#define OCR1BL_ADDRI		0x28	/* Timer/Counter1 – Output Compare
					   Register B Low byte */
#define ICR1H_ADDRI		0x27	/* Timer/Counter1 – Input Capture
					   Register High byte */
#define ICR1L_ADDRI		0x26	/* Timer/Counter1 – Input Capture
					   Register Low byte */
#define TCCR2_ADDRI		0x25
#define TCNT2_ADDRI		0x24	/* Timer/Counter2 (8 Bits) */
#define OCR2_ADDRI		0x23	/* Timer/Counter2 Output Compare
					   Register */
#define ASSR_ADDRI		0x22
#define WDTCR_ADDRI		0x21
#define UBRRH_ADDRI		0x20	/* Refer to the USART description
					   for details on how to access
					   UBRRH and UCSRC. */
#define UCSRC_ADDRI		0x20
#define EEARH_ADDRI		0x1F
#define EEARL_ADDRI		0x1E
#define EEDR_ADDRI		0x1D	/* EEPROM Data Register */
#define EECR_ADDRI		0x1C
/*	Reserved		0x1B */
/*	Reserved		0x1A */
/*	Reserved		0x19 */
#define PORTB_ADDRI		0x18
#define DDRB_ADDRI		0x17
#define PINB_ADDRI		0x16
#define PORTC_ADDRI		0x15
#define DDRC_ADDRI		0x14
#define PINC_ADDRI		0x13
#define PORTD_ADDRI		0x12
#define DDRD_ADDRI		0x11
#define PIND_ADDRI		0x10
#define SPDR_ADDRI		0x0F	/* SPI Data Register */
#define SPSR_ADDRI		0x0E
#define SPCR_ADDRI		0x0D
#define UDR_ADDRI		0x0C	/* USART I/O Data Register */
#define UCSRA_ADDRI		0x0B
#define UCSRB_ADDRI		0x0A
#define UBRRL_ADDRI		0x09	/* USART Baud Rate Register Low byte */
#define ACSR_ADDRI		0x08
#define ADMUX_ADDRI		0x07
#define ADCSRA_ADDRI		0x06
#define ADCH_ADDRI		0x05	/* ADC Data Register High byte */
#define ADCL_ADDRI		0x04	/* ADC Data Register Low byte */
#define TWDR_ADDRI		0x03	/* Two-wire Serial Interface Data
					   Register */
#define TWAR_ADDRI		0x02
#define TWSR_ADDRI		0x01
#define TWBR_ADDRI		0x00	/* Two-wire Serial Interface Bit
					   Rate Register */

typedef uint32_t MSIM_AVRFlashAddr_t;

enum MSIM_AVRState {
	AVR_RUNNING = INT16_MIN,
	AVR_STOPPED,
	AVR_SLEEPING
};

enum MSIM_AVRClkSource {
	AVR_INT_CLK = INT16_MIN,
	AVR_EXT_CLK
};

enum MSIM_AVRSREGFlag {
	AVR_SREG_CARRY = INT16_MIN,
	AVR_SREG_ZERO,
	AVR_SREG_NEGATIVE,
	AVR_SREG_TWOSCOM_OF,
	AVR_SREG_SIGN,
	AVR_SREG_HALF_CARRY,
	AVR_SREG_BITCOPY_ST,
	AVR_SREG_GLOB_INT
};

/* Instance of the AVR microcontroller. */
struct MSIM_AVR {
	uint32_t id;			/* ID of a simulated AVR MCU */

	char name[20];			/* Name of the MCU */
	uint8_t signature[3];		/* Signature of the MCU */
	uint16_t spm_pagesize;		/* For devices with bootloader support,
					   the flash pagesize (in bytes) to be
					   used for Self Programming Mode (SPM)
					   instruction. */
	uint32_t flashstart;		/* The first byte address in flash
					   program space, in bytes. */
	uint32_t flashend;		/* The last byte address in flash
					   program space, in bytes. */
	struct MSIM_AVRBootloader *boot_loader;
	uint32_t ramstart;
	uint32_t ramend;
	uint32_t ramsize;
	uint16_t e2start;		/* The first EEPROM address */
	uint16_t e2end;			/* The last EEPROM address */
	uint16_t e2size;
	uint16_t e2pagesize;		/* The size of the EEPROM page */
	uint8_t lockbits;
	uint8_t fuse[6];

	enum MSIM_AVRState state;
	enum MSIM_AVRClkSource clk_source;
	uint32_t freq;			/* Frequency we're currently
					   working at, in kHz */
	MSIM_AVRFlashAddr_t pc;		/* Current program counter register */
	MSIM_AVRFlashAddr_t reset_pc;	/* This is a value used to jump to
					   at reset time. */
	MSIM_AVRFlashAddr_t ivt;	/* Address of Interrupt Vectors Table
					   in program memory. */

	uint8_t *sp_high;		/* SPH in the data memory */
	uint8_t *sp_low;		/* SPL in the data memory */
	uint8_t *sreg;			/* Points directly to SREG placed
					   in data section. */

	uint8_t *prog_mem;		/* Flash memory. This memory
					   section could contain
					   a bootloader. */
	uint8_t *data_mem;		/* General purpose registers,
					   IO registers and SRAM */
	uint32_t pm_size;		/* Actual size of the program memory. */
	uint32_t dm_size;		/* Actual size of the data memory. */


	uint32_t sfr_off;		/* Offset to the AVR special function
					   registers. */
	int16_t io_addr[64];		/* Addresses of the I/O ports of
					   the MCU. */
};

#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/simm8a.h"

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
