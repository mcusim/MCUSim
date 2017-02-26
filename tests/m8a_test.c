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
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "minunit.h"
#include "avr/sim/sim.h"
#include "avr/sim/bootloader.h"
#include "tools/gis/ihex.h"

#define SUITE_NAME		"Atmel ATMega8A tests"

#define PROGMEM_FILE		"avr-progmem.hex"

/* Tests counter */
int tests_run = 0;

static struct avr m8a;
static struct avr_bootloader bldr;
static uint16_t prog_mem[4096];
static uint8_t data_mem[1120];

/* Test functions prototypes */
int m8a_initialized(void);
int progmem_loaded(void);

int m8a_initialized(void)
{
	m8a.boot_loader = &bldr;

	_mu_assert(m8a_init(&m8a) == 0);

	_mu_test(strcmp(m8a.name, "atmega8a") == 0);
	_mu_test(m8a.spm_pagesize == 64);

	/*
	 * Expected clock source is an internal RC oscillator
	 * which is working at 1 MHz.
	 */
	_mu_test(m8a.freq == 1000);
	_mu_test(m8a.clk_source == AVR_INT_CLK);

	/*
	 * Program memory configuration.
	 */
	_mu_test(m8a.flashstart == 0x0000);
	_mu_test(m8a.flashend == 0x1FFF);
	_mu_test(m8a.boot_loader->start == 0xC00);
	_mu_test(m8a.boot_loader->end == 0xFFF);
	_mu_test(m8a.boot_loader->size == 1024);

	/*
	 * RAM configuration.
	 */
	_mu_test(m8a.ramstart == 0x0060);
	_mu_test(m8a.ramend == 0x045F);
	_mu_test(m8a.ramsize == 1024);

	/*
	 * EEPROM memory configuration.
	 */
	_mu_test(m8a.e2start == 0);
	_mu_test(m8a.e2end == 0x01FF);
	_mu_test(m8a.e2size == 512);
	_mu_test(m8a.e2pagesize == 4);

	/*
	 * Reset vector
	 */
	_mu_test(m8a.reset_pc == 0x000);

	_mu_test(!m8a_set_datamem(&m8a, data_mem, sizeof(data_mem)/
						 sizeof(data_mem[0])));

	return 0;
}

int progmem_loaded(void)
{
	FILE *fp;
	IHexRecord rec;

	fp = fopen(PROGMEM_FILE, "r");
	_mu_assert(fp);

	/*
	 * Print HEX file dedicated to be burned into a simulated
	 * ATmega8A.
	 */
	while (Read_IHexRecord(&rec, fp) == IHEX_OK) {
		/*
		Print_IHexRecord(&rec);
		printf("\n");
		*/
	}
	rewind(fp);

	/*
	 * Assign program memory to the MCU.
	 */
	_mu_assert(!m8a_set_progmem(&m8a, prog_mem, sizeof(prog_mem)/
				  		    sizeof(prog_mem[0])));

	/*
	 * Load program memory from the HEX file.
	 */
	_mu_test(!m8a_load_progmem(&m8a, fp));

	fclose(fp);
	return 0;
}

int all_tests(void)
{
	_mu_verify(m8a_initialized);
	_mu_verify(progmem_loaded);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}
