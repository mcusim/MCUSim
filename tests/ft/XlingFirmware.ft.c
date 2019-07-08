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
	{ 0x0240, TEST_PREF ".dm_0_0240.hex" },
	{ 0x0242, TEST_PREF ".dm_1_0242.hex" },
	{ 0x0244, TEST_PREF ".dm_2_0244.hex" },
	{ 0x0245, TEST_PREF ".dm_3_0245.hex" },
	{ 0x0247, TEST_PREF ".dm_4_0247.hex" },
	{ 0x024B, TEST_PREF ".dm_5_024B.hex" },
	{ 0x024E, TEST_PREF ".dm_6_024E.hex" },
	{ 0x024F, TEST_PREF ".dm_7_024F.hex" },
	{ 0x0250, TEST_PREF ".dm_8_0250.hex" },
	{ 0x024C, TEST_PREF ".dm_9_024C.hex" },
	{ 0x024D, TEST_PREF ".dm_10_024D.hex" },
	{ 0x024E, TEST_PREF ".dm_11_024E.hex" },
	{ 0x024F, TEST_PREF ".dm_12_024F.hex" },
	{ 0x0250, TEST_PREF ".dm_13_0250.hex" },
	{ 0x0259, TEST_PREF ".dm_14_0259.hex" },
	{ 0x025F, TEST_PREF ".dm_15_025F.hex" },
	{ 0x0262, TEST_PREF ".dm_16_0262.hex" },
	{ 0x0269, TEST_PREF ".dm_17_0269.hex" },
	{ 0x026F, TEST_PREF ".dm_18_026F.hex" },
	{ 0x0277, TEST_PREF ".dm_19_0277.hex" },
	{ 0x028F, TEST_PREF ".dm_20_028F.hex" },
	{ 0x02B4, TEST_PREF ".dm_21_02B4.hex" },
	{ 0x0313, TEST_PREF ".dm_22_0313.hex" },
	{ 0x033D, TEST_PREF ".dm_23_033D.hex" },
};
/* Index of the datamem check point */
static uint32_t dmcp = 0;

/*
 * Perform AVR simulation with occasional stops to compare actual
 * data memory with HEX dumps (obtained from the Atmel Studio 7 with
 * MemoryLogger plugin).
 */
static void
check_datamem(void **state)
{
	/* Compare the whole datamem (0x900 for m328p) */
	const size_t memsz = mcu->ramend + 1;

	/* Initialize data memory */
	if (dm_checkpoints[dmcp].pc == 0U) {
		snprintf(LOG, LOGSZ, "init datamem: step=%" PRIu32 ", pc=0x%"
		         PRIx32 ", size=0x%" PRIx64 " (%" PRIu64 ")",
		         dmcp, mcu->pc, memsz, memsz);
		MSIM_LOG_INFO(LOG);

		MSIM_AVR_LoadDataMem(mcu, dm_checkpoints[dmcp].dump);
		dmcp++;
	}

	do {
		int rc = MSIM_AVR_SimStep(mcu, FRM_TEST);

		/* Stop if there was an error during simulation step. */
		if (rc != 0) {
			MSIM_LOG_ERROR("simulation step error!");
			assert_int_equal(rc, 0);
		}

		/* Check data memory against a dump */
		if (mcu->pc == dm_checkpoints[dmcp].pc) {
			/*
			 * Data memory can be uninitialized if we didn't have
			 * a dump file for PC == 0x0000.
			 */
			if (dmcp == 0U) {
				snprintf(LOG, LOGSZ, "init datamem: step=%"
				         PRIu32 ", pc=0x%" PRIx32 ", size=0x%"
				         PRIx64 " (%" PRIu64 ")",
				         dmcp, mcu->pc, memsz, memsz);
				MSIM_LOG_INFO(LOG);

				MSIM_AVR_LoadDataMem(
				        mcu, dm_checkpoints[dmcp].dump);
				dmcp++;
				continue;
			}

			snprintf(LOG, LOGSZ, "checking datamem: step=%" PRIu32
			         ", pc=0x%" PRIx32 ", size=0x%" PRIx64 " (%"
			         PRIu64 ")", dmcp, mcu->pc, memsz, memsz);
			MSIM_LOG_INFO(LOG);

			/* Load DM from a file to the 'control MCU */
			MSIM_AVR_LoadDataMem(ctl, dm_checkpoints[dmcp].dump);

			/*
			 * Compare data memories of the 'current' and
			 * 'control' MCUs.
			 */
			assert_memory_equal(mcu->dm, ctl->dm, memsz);

			dmcp++;
		}
	} while (dmcp < ARRSZ(dm_checkpoints));
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
		cmocka_unit_test_setup(check_datamem, restore_mcu),
	};

	do {
		/* No output from MCUSim */
		MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);

		/* Read config file */
		rc = MSIM_CFG_Read(&conf, CONF_FILE);
		if (rc != 0) {
			break;
		}

		/* Force firmware test option */
		conf.firmware_test = 1;

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
