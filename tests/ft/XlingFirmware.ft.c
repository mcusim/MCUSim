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
#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "mcusim/mcusim.h"
#include "mcusim/config.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"

#define TEST_PREF		"files/XlingFirmware"
#define CONF_FILE		TEST_PREF ".ft.conf"

#define GDB_RSP_PORT		12750
#define FRM_TEST		1
#define SREGR			(*mcu->sreg)

#define RESTORE_MCU() do {						\
	_ctl = _orig;							\
	_avr = _orig;							\
} while (0)

/* Structure to keep a PC value and a dump of the data memory. */
struct dm_checkpoint {
	uint32_t pc;
	char dump[1024];
};

static MSIM_AVR _orig;
static MSIM_AVR _ctl;
static MSIM_AVR _avr;
static MSIM_AVR *mcu = &_avr;
static MSIM_AVR *ctl = &_ctl;

static MSIM_CFG conf = {
	.firmware_test = FRM_TEST,
	.rsp_port = GDB_RSP_PORT,
};

/* Check points to stop and check data memory. */
static struct dm_checkpoint dm_checkpoints[] = {
	{ 0x0000, TEST_PREF ".dm_0000.hex" },
	{ 0x04d2, TEST_PREF ".dm_04d2.hex" },
	{ 0x04de, TEST_PREF ".dm_04de.hex" },
	{ 0x04ee, TEST_PREF ".dm_04ee.hex" },
	{ 0x051e, TEST_PREF ".dm_051e.hex" },
	{ 0x0568, TEST_PREF ".dm_0568.hex" },
	{ 0x0626, TEST_PREF ".dm_0626.hex" },
	{ 0x067a, TEST_PREF ".dm_067a.hex" },
};
/* Index of the current DM check point. */
static uint32_t i_dmcp = 0;

/*
 * Perform AVR simulation with occasional stops to compare actual
 * data memory with Intel HEX dumps.
 */
static void
check_simulation(void **state)
{
	/* Only general purpose and I/O registers to compare */
	const size_t memsz = mcu->ramstart;

	/* Initialize data memory */
	MSIM_AVR_LoadDataMem(mcu, dm_checkpoints[i_dmcp].dump);
	i_dmcp++;

	do {
		/* No output from MCUSim */
		MSIM_LOG_SetLevel(MSIM_LOG_LVLNONE);

		int rc = MSIM_AVR_SimStep(mcu, FRM_TEST);

		/* Let test messages be logged properly */
		MSIM_LOG_SetLevel(MSIM_LOG_LVLINFO);

		/* Stop if there was an error during simulation step. */
		assert_int_equal(rc, 0);

		/* Check data memory against a dump */
		if (mcu->pc == dm_checkpoints[i_dmcp].pc) {
			snprintf(LOG, LOGSZ, "checking datamem: pc=0x%" PRIx32
			         " (0x%" PRIx32 "), size=0x%" PRIx64
			         " (%" PRIu64 ")",
			         mcu->pc, mcu->pc >> 1, memsz, memsz);
			MSIM_LOG_INFO(LOG);

			/* Load DM from a file to the control MCU */
			MSIM_AVR_LoadDataMem(ctl, dm_checkpoints[i_dmcp].dump);

			/* Compare data memories */
			assert_memory_equal(mcu->dm, ctl->dm, memsz);

			i_dmcp++;
		}
	} while (i_dmcp < ARRSZ(dm_checkpoints));
}

static int
restore_mcu(void **state)
{
	RESTORE_MCU();
	return 0;
}

int
main(void)
{
	int rc = 0;

	const struct CMUnitTest tests[] = {
		cmocka_unit_test_setup(check_simulation, restore_mcu),
	};

	do {
		/* No output from MCUSim */
		MSIM_LOG_SetLevel(MSIM_LOG_LVLNONE);

		/* Read config file */
		rc = MSIM_CFG_Read(&conf, CONF_FILE);
		if (rc != 0) {
			break;
		}

		/* Initialize AVR MCU */
		rc = MSIM_AVR_Init(mcu, &conf);
		_orig = *mcu;
		_ctl = *mcu;

		/* Don't write any registers to VCD */
		mcu->vcd.regs[0].i = -1;

		/* Force running state */
		mcu->state = AVR_RUNNING;

	} while (0);

	return rc != 0 ? rc : cmocka_run_group_tests(tests, NULL, NULL);
}
