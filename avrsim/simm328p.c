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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * We would like to include headers specific to the ATMega328P
 * microcontroller.
 */
#define __AVR_ATmega328P__ 1
#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"

#define FLASHSTART		0x0000
#define RAMSIZE			2048
#define E2START			0x0000
#define E2SIZE			1024

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			  uint8_t fuse_high, uint8_t fuse_low);

int MSIM_M328PInit(struct MSIM_AVR *mcu)
{
	uint32_t i;

	if (!mcu) {
		fprintf(stderr, "MCU should not be NULL\n");
		return -1;
	}

	srand((unsigned int) time(NULL));
	mcu->id = (uint32_t) rand();
	strcpy(mcu->name, "atmega328p");
	mcu->signature[0] = SIGNATURE_0;
	mcu->signature[1] = SIGNATURE_1;
	mcu->signature[2] = SIGNATURE_2;

	/*
	 * Set values according to the header file included
	 * by avr/io.h.
	 */
	mcu->spm_pagesize = SPM_PAGESIZE;
	mcu->flashstart = FLASHSTART;
	mcu->flashend = FLASHEND;
	mcu->ramstart = RAMSTART;
	mcu->ramend = RAMEND;
	mcu->ramsize = RAMSIZE;
	mcu->e2start = E2START;
	mcu->e2end = E2END;
	mcu->e2size = E2SIZE;
	mcu->e2pagesize = E2PAGESIZE;

	mcu->lockbits = 0x3F;

	mcu->sfr_off = __SFR_OFFSET;

	/* Invalidate I/O ports addresses before initialization */
	for (i = 0; i < sizeof(mcu->io_addr)/sizeof(mcu->io_addr[0]); i++)
		mcu->io_addr[i] = -1;

	mcu->io_addr[PINB_ADDRI] = 0x03;
	mcu->io_addr[DDRB_ADDRI] = 0x04;
	mcu->io_addr[PORTB_ADDRI] = 0x05;
	mcu->io_addr[PINC_ADDRI] = 0x06;
	mcu->io_addr[DDRC_ADDRI] = 0x07;
	mcu->io_addr[PORTC_ADDRI] = 0x08;
	mcu->io_addr[PIND_ADDRI] = 0x09;
	mcu->io_addr[DDRD_ADDRI] = 0x0A;
	mcu->io_addr[PORTD_ADDRI] = 0x0B;
	mcu->io_addr[TIFR0_ADDRI] = 0x15;
	mcu->io_addr[TIFR1_ADDRI] = 0x16;
	mcu->io_addr[TIFR2_ADDRI] = 0x17;
	mcu->io_addr[PCIFR_ADDRI] = 0x1B;
	mcu->io_addr[EIFR_ADDRI] = 0x1C;
	mcu->io_addr[EIMSK_ADDRI] = 0x1D;
	mcu->io_addr[GPIOR0_ADDRI] = 0x1E;
	mcu->io_addr[EECR_ADDRI] = 0x1F;
	mcu->io_addr[EEDR_ADDRI] = 0x20;
	mcu->io_addr[EEARL_ADDRI] = 0x21;
	mcu->io_addr[EEARH_ADDRI] = 0x22;
	mcu->io_addr[GTCCR_ADDRI] = 0x23;
	mcu->io_addr[TCCR0A_ADDRI] = 0x24;
	mcu->io_addr[TCCR0B_ADDRI] = 0x25;
	mcu->io_addr[TCNT0_ADDRI] = 0x26;
	mcu->io_addr[OCR0A_ADDRI] = 0x27;
	mcu->io_addr[OCR0B_ADDRI] = 0x28;
	mcu->io_addr[GPIOR1_ADDRI] = 0x2A;
	mcu->io_addr[GPIOR2_ADDRI] = 0x2B;
	mcu->io_addr[SPCR_ADDRI] = 0x2C;
	mcu->io_addr[SPSR_ADDRI] = 0x2D;
	mcu->io_addr[SPDR_ADDRI] = 0x2E;
	mcu->io_addr[ACSR_ADDRI] = 0x30;
	mcu->io_addr[SMCR_ADDRI] = 0x33;
	mcu->io_addr[MCUSR_ADDRI] = 0x34;
	mcu->io_addr[MCUCR_ADDRI] = 0x35;
	mcu->io_addr[SPMCSR_ADDRI] = 0x37;
	mcu->io_addr[SPL_ADDRI] = 0x3D;
	mcu->io_addr[SPH_ADDRI] = 0x3E;
	mcu->io_addr[SREG_ADDRI] = 0x3F;
	/* extended I/O registers */
	/* mcu->io_addr[... */

	if (set_fuse_bytes(mcu, 0x00, 0xD9, 0xE1)) {
		fprintf(stderr, "Fuse bytes cannot be set correctly\n");
		return -1;
	}
	return 0;
}

static int set_fuse_bytes(struct MSIM_AVR *mcu, uint8_t fuse_ext,
			  uint8_t fuse_high, uint8_t fuse_low)
{
	return -1;
}
