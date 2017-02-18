/*
 * mcusim - Interactive simulator for microcontrollers.
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
#include <string.h>

/*
 * We would like to include headers specific to the
 * ATMega8A microcontroller.
 */
#define __AVR_ATmega8A__ 1

#include "avr/io.h"
#include "avr/sim/sim.h"
#include "avr/sim/m8a.h"

int m8a_init(struct avr *mcu)
{
	if (!mcu) {
		fprintf(stderr, "MCU should not be NULL\n");
		return -1;
	}

	strcpy(mcu->name, "atmega8a");

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

	/*
	 * Set ATmega8A lock bits to the default values
	 * according to the datasheet. ATmega8A has 6 lock bits only.
	 *
	 * They are:
	 * 5     4     3     2     1   0
	 * BLB12 BLB11 BLB02 BLB01 LB2 LB1
	 *
	 * Default value means:
	 *	- no memory lock features enabled;
	 *	- no restrictions for SPM or Load Program Memory (LPM)
	 *	  instruction accessing the Application section;
	 *	- no restrictions for SPM or LPM accessing
	 *	  the Boot Loader section.
	 */
	mcu->lockbits = 0x3F;

	/*
	 * Set ATmega8A fuse bits to the default values. ATmega8A has
	 * two fuse bytes, high and low.
	 *
	 * The high fuse byte is:
	 * 7        6     5     4     3      2       1       0
	 * RSTDISBL WDTON SPIEN CKOPT EESAVE BOOTSZ1 BOOTSZ0 BOOTRST
	 *
	 * The low fuse byte is:
	 * 7        6     5    4    3      2      1      0
	 * BODLEVEL BODEN SUT1 SUT0 CKSEL3 CKSEL2 CKSEL1 CKSEL0
	 *
	 * Default value for high byte means:
	 *	- boot sector in program memory is 1024 words,
	 *	  from 0xC00-0xFFF,
	 *	  application sector is 3072 words,
	 *	  from 0x000-0xBFF;
	 *	- ...
	 *
	 * Default value for low byte means:
	 *	- ...
	 *
	 */
	mcu->fuse[1] = 0xD9; /* high */
	mcu->fuse[0] = 0xE1; /* low */

	return 0;
}

int m8a_prog_mem(struct avr *mcu, uint16_t *mem, uint32_t size)
{
	return -1;
}

int m8a_data_mem(struct avr *mcu, uint8_t *mem, uint32_t size)
{
	return -1;
}
