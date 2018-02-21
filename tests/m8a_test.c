/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/*
 * We would like to include headers specific to the
 * ATMega8A microcontroller.
 */
#ifndef __AVR_ATmega8A__
#define __AVR_ATmega8A__ 1
#endif

#include "mcusim/avr/io.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/hex/ihex.h"

#include "minunit.h"

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

	_mu_assert(m8a_init(&m8a, prog_mem,
			    sizeof(prog_mem) / sizeof(prog_mem[0]), data_mem,
			    sizeof(data_mem) / sizeof(data_mem[0])) == 0);
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

	_mu_test(m8a.reset_pc == 0x000);

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
