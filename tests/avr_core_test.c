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
//#include <sys/io.h>

#include "avr/sim/sim.h"
#include "avr/sim/simcore.h"
#include "avr/sim/bootloader.h"

#include "minunit.h"
#include "tools/math/math.h"
//#include "arch/x86/tsc.h"

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
