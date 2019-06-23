/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Unit tests for AVR Instruction Set.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "mcusim/mcusim.h"
#include "mcusim/config.h"
#include "mcusim/avr/sim/private/macro.h"

#define GDB_RSP_PORT		12750
#define CONF_FILE		"mcusim.ut.conf"

static MSIM_AVR orig_mcu;
static MSIM_AVR avr_mcu;
static MSIM_AVR *mcu = &avr_mcu;

static MSIM_CFG conf = {
	.firmware_test = 1,
	.rsp_port = GDB_RSP_PORT,
};

/* SUB - Subtract Without Carry */
static void
check_sub(void **state)
{
	/* 0x1b28 - sub r18, r24 */
	mcu->pm[0] = 0x28;
	mcu->pm[1] = 0x1b;
	DM(18) = 3;
	DM(24) = 1;

	assert_int_equal(mcu->pc, 0x000000);
	assert_int_equal(DM(18), 3);
	assert_int_equal(DM(24), 1);

	MSIM_AVR_Step(mcu);

	assert_int_equal(mcu->pc, 0x000002);
	assert_int_equal(DM(18), 2);
	assert_int_equal(DM(24), 1);

	*mcu = orig_mcu;
}

/* An entry point of the AVR Instruction Set test suite. */
int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(check_sub),
	};
	int rc = 0;

	/* No output from MCUSim */
	MSIM_LOG_SetLevel(MSIM_LOG_LVLNONE);

	/* Initialize AVR MCU */
	rc = MSIM_AVR_Init(mcu, &conf, CONF_FILE);
	orig_mcu = *mcu;

	return rc != 0 ? rc : cmocka_run_group_tests(tests, NULL, NULL);
}
