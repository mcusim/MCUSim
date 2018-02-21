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
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/math/math.h"

#include "minunit.h"

#define SUITE_NAME		"AVR Simulation Core Test"

#define PROGMEM_FILE		"avr-progmem.hex"

/* Tests counter */
int tests_run = 0;

/*
 * Allocate MCU.
 */
static uint16_t prog_mem[4096];
static uint8_t data_mem[1120];
static struct avr_bootloader bldr;
static struct avr m8a = {
	.boot_loader = &bldr,
};

/* Test functions prototypes */
int simple_io_simulation(void);

int simple_io_simulation(void)
{
	FILE *prog_file;

	_mu_assert(!m8a_init(&m8a, prog_mem,
			     sizeof(prog_mem) / sizeof(prog_mem[0]), data_mem,
			     sizeof(data_mem) / sizeof(data_mem[0])));

	prog_file = fopen(PROGMEM_FILE, "r");
	_mu_assert(prog_file);

	_mu_assert(!m8a_load_progmem(&m8a, prog_file));

	simulate_avr(&m8a);
	return 0;
}

int all_tests(void)
{
	_mu_verify(simple_io_simulation);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}
