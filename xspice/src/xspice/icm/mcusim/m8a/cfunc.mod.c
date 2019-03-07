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
 * SUMMARY
 *
 *	This file contains the functional description of the
 *	simulated microcontroller of ATmega8A provided by MCUSim.
 *
 * REFERENCED FILES
 *
 *	Inputs from and outputs to ARGS structure.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include "mcusim/mcusim.h"
#include "mcusim/config.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/m8/m8a.h"

/* Local macros */
#define MCU_MODEL		"m8a"
#define DM(v)			(mcu->dm[v])
#define LOG			(mcu->log)
#define LOGSZ			(MSIM_AVR_LOGSZ)
#define TICKS_MAX		UINT64_MAX

#define FUSE_EXT		2
#define FUSE_HIGH		1
#define FUSE_LOW		0
#define GLOB_INT		7

#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))

/* Macro to convert Digital_State_t to a bit in uint8_t. */
#define TO_BIT(ds, b) do {						\
	switch ((ds)) {							\
	case ZERO:							\
		(b) = 0;						\
		break;							\
	case ONE:							\
		(b) = 1;						\
		break;							\
	default:							\
		(b) = rand()&1;						\
		break;							\
	}								\
} while (0)

/* Macros to read and update AVR status register (SREG) */
#define UPDATE_SREG(mcu, flag, set_f) do {				\
	if ((set_f) == 0) {						\
		*mcu->sreg &= (uint8_t)~(1<<(flag));			\
	} else {							\
		*mcu->sreg |= (uint8_t)(1<<(flag));			\
	}								\
} while (0)
#define READ_SREG(mcu, flag) ((uint8_t)((*mcu->sreg>>(flag))&1))

static struct MSIM_AVR _mcu;
static struct MSIM_CFG _cfg;
static uint64_t _tick;
static uint8_t _tick_ovf;

/* Local functions. */
static void setup_ports_load(ARGS);
static void ports_not_changed(ARGS);
static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val);
static int set_lock(struct MSIM_AVR *m, uint8_t val);
static void print_config(struct MSIM_AVR *m);
static void update_bit(uint8_t *val, uint8_t i, uint8_t b);

/* Local functions to setup and update a microcontroller. */
static int setup_avr(ARGS, struct MSIM_AVR *mcu, struct MSIM_CFG *conf);

void MSIM_CM_M8A(ARGS)
{
	Digital_State_t *clk;		/* Storage for clock value */
	Digital_State_t *clk_old;	/* Previous clock value */
	struct MSIM_AVR *mcu;		/* AVR MCU descriptor */
	struct MSIM_CFG *cfg;		/* Configuration of the simulator */
	uint64_t *tick;
	uint8_t *tick_ovf;
	int rc;

	if (INIT) {
		MSIM_CFG_PrintVersion();

		/* Allocate memory for the even-driven model structures. */
		cm_event_alloc(0, sizeof(Digital_State_t));
		cm_event_alloc(1, sizeof(Digital_State_t));

		clk = (Digital_State_t *) cm_event_get_ptr(0, 0);
		clk_old = clk;

		mcu = &_mcu;
		cfg = &_cfg;
		tick = &_tick;
		tick_ovf = &_tick_ovf;

		*tick = 0;
		*tick_ovf = 0;

		/* Set up capacitive load values. */
		setup_ports_load(mif_private);
		LOAD(clk) = PARAM(clk_load);
		if (!PORT_NULL(reset)) {
			LOAD(reset) = PARAM(reset_load);
		}

		/* Set up an instance of ATmega8A. */
		setup_avr(mif_private, mcu, cfg);
	} else {
		mcu = &_mcu;
		cfg = &_cfg;
		tick = &_tick;
		tick_ovf = &_tick_ovf;

		clk = (Digital_State_t *) cm_event_get_ptr(0, 0);
		clk_old = (Digital_State_t *) cm_event_get_ptr(0, 1);
	}

	if (TIME != 0.0) {
		/* Update clock and reset values. */
		*clk = INPUT_STATE(clk);

		if ((*clk != *clk_old) && (*clk == ONE)) {
			Digital_State_t ns;
			uint8_t pval, b;

			/* Obtain input values of ports. */
			pval = DM(PINB);
			for (uint32_t i = 0; i < PORT_SIZE(Bin); i++) {
				if ((PORT_NULL(Bin) == 0) &&
				    (INPUT_STRENGTH(Bin[i]) != HI_IMPEDANCE)) {
					TO_BIT(INPUT_STATE(Bin[i]), b);
					update_bit(&pval, i, b);
				}
			}
			DM(PINB) = pval & (~DM(DDRB));

			pval = DM(PINC);
			for (uint32_t i = 0; i < PORT_SIZE(Cin); i++) {
				if ((PORT_NULL(Cin) == 0) &&
				    (INPUT_STRENGTH(Cin[i]) != HI_IMPEDANCE)) {
					TO_BIT(INPUT_STATE(Cin[i]), b);
					update_bit(&pval, i, b);
				}
			}
			DM(PINC) = pval & (~DM(DDRC));

			pval = DM(PIND);
			for (uint32_t i = 0; i < PORT_SIZE(Din); i++) {
				if ((PORT_NULL(Din) == 0) &&
				    (INPUT_STRENGTH(Din[i]) != HI_IMPEDANCE)) {
					TO_BIT(INPUT_STATE(Din[i]), b);
					update_bit(&pval, i, b);
				}
			}
			DM(PIND) = pval & (~DM(DDRD));

			/* Update the microcontroller */
			MSIM_AVR_SimStep(mcu, tick, tick_ovf,
			                 cfg->firmware_test);

			/* Provide the updated values of the ports. */
			for (uint32_t i = 0; i < PORT_SIZE(Bout); i++) {
				if (((DM(DDRB)>>i)&1) == 0U) {
					OUTPUT_CHANGED(Bout[i]) = FALSE;
				} else {
					ns = ((DM(PORTB)>>i)&1) ? ONE : ZERO;
					OUTPUT_STATE(Bout[i]) = ns;
					OUTPUT_STRENGTH(Bout[i]) = STRONG;
					OUTPUT_DELAY(Bout[i]) = PARAM(clk_delay);
				}
			}
			for (uint32_t i = 0; i < PORT_SIZE(Cout); i++) {
				if (((DM(DDRC)>>i)&1) == 0U) {
					OUTPUT_CHANGED(Cout[i]) = FALSE;
				} else {
					ns = ((DM(PORTC)>>i)&1) ? ONE : ZERO;
					OUTPUT_STATE(Cout[i]) = ns;
					OUTPUT_STRENGTH(Cout[i]) = STRONG;
					OUTPUT_DELAY(Cout[i]) = PARAM(clk_delay);
				}
			}
			for (uint32_t i = 0; i < PORT_SIZE(Dout); i++) {
				if (((DM(DDRD)>>i)&1) == 0U) {
					OUTPUT_CHANGED(Dout[i]) = FALSE;
				} else {
					ns = ((DM(PORTD)>>i)&1) ? ONE : ZERO;
					OUTPUT_STATE(Dout[i]) = ns;
					OUTPUT_STRENGTH(Dout[i]) = STRONG;
					OUTPUT_DELAY(Dout[i]) = PARAM(clk_delay);
				}
			}
		} else {
			ports_not_changed(mif_private);
		}
	} else {
		/* Output without delays */
		if (PORT_NULL(Bout) == 0) {
			for (uint32_t i = 0; i < PORT_SIZE(Bout); i++) {
				OUTPUT_STATE(Bout[i]) = UNKNOWN;
				OUTPUT_STRENGTH(Bout[i]) = HI_IMPEDANCE;
			}
		}
		if (PORT_NULL(Cout) == 0) {
			for (uint32_t i = 0; i < PORT_SIZE(Cout); i++) {
				OUTPUT_STATE(Cout[i]) = UNKNOWN;
				OUTPUT_STRENGTH(Cout[i]) = HI_IMPEDANCE;
			}
		}
		if (PORT_NULL(Dout) == 0) {
			for (uint32_t i = 0; i < PORT_SIZE(Dout); i++) {
				OUTPUT_STATE(Dout[i]) = UNKNOWN;
				OUTPUT_STRENGTH(Dout[i]) = HI_IMPEDANCE;
			}
		}
	}
}

static void setup_ports_load(ARGS)
{
	for (uint32_t i = 0; i < PORT_SIZE(Bin); i++) {
		LOAD(Bin[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Cin); i++) {
		LOAD(Cin[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Din); i++) {
		LOAD(Din[i]) = PARAM(input_load);
	}
}

static void ports_not_changed(ARGS)
{
	for (uint32_t i = 0; i < PORT_SIZE(Bout); i++) {
		OUTPUT_CHANGED(Bout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Cout); i++) {
		OUTPUT_CHANGED(Cout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Dout); i++) {
		OUTPUT_CHANGED(Dout[i]) = FALSE;
	}
}

static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val)
{
	int rc = 0;

	if (m->set_fusef == NULL) {
		MSIM_LOG_WARN("cannot modify fuse (MCU-specific function is "
		              "not available)");
		rc = 1;
	} else {
		m->set_fusef(m, fuse, val);
	}
	return rc;
}

static int set_lock(struct MSIM_AVR *m, uint8_t val)
{
	int rc = 0;

	if (m->set_lockf == NULL) {
		MSIM_LOG_WARN("cannot modify lock bits (MCU-specific function "
		              "is not available)");
		rc = 1;
	} else {
		m->set_lockf(m, val);
	}
	return rc;
}

static int setup_avr(ARGS, struct MSIM_AVR *mcu, struct MSIM_CFG *conf)
{
	FILE *fp;
	int rc = 0;
	char *conf_file;
	uint32_t dump_regs;
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	const uint32_t dflen = sizeof vcd->dump_file/sizeof vcd->dump_file[0];
	const uint32_t mcuname_len = sizeof conf->mcu/sizeof conf->mcu[0];

	/* Try to load a configuration file of MCUSim. */
	conf_file = PARAM(config_file);
	rc = MSIM_CFG_Read(conf, conf_file);
	if (rc != 0) {
		snprintf(LOG, LOGSZ, "failed loading : %s", conf_file);
		MSIM_LOG_FATAL(LOG);
	} else {
		snprintf(LOG, LOGSZ, "using config file: %s", conf_file);
		MSIM_LOG_INFO(LOG);

		/* Only a firmware file will be loaded in XSPICE model. */
		if (conf->has_firmware_file == 1) {
			fp = fopen(conf->firmware_file, "r");
			if (fp == NULL) {
				snprintf(LOG, LOGSZ, "failed to read firmware "
				         "file: %s", conf->firmware_file);
				MSIM_LOG_FATAL(LOG);
				rc = 1;
			} else {
				snprintf(LOG, LOGSZ, "firmware file used: %s",
				         conf->firmware_file);
				MSIM_LOG_INFO(LOG);
			}
		} else {
			MSIM_LOG_FATAL("no firmware file in the config");
			rc = 1;
		}

		/* Initialize MCU as one of the AVR models */
		if (rc == 0U) {
			mcu->intr.trap_at_isr = conf->trap_at_isr;
			vcd->dump_file[0] = 0;
			strncpy(conf->mcu, MCU_MODEL, mcuname_len);
			rc = MSIM_AVR_Init(mcu, conf->mcu, NULL,
			                   MSIM_AVR_PMSZ, NULL,
			                   MSIM_AVR_DMSZ, NULL, fp);
			if (rc != 0) {
				MSIM_LOG_FATAL("MCU model " MCU_MODEL
				               " cannot be initialized");
			}
			fclose(fp);
		}

		/* Select registers to be dumped */
		dump_regs = 0;
		strncpy(vcd->dump_file, conf->vcd_file, dflen - 1);
		for (uint32_t i = 0; i < conf->dump_regs_num; i++) {
			for (uint32_t j = 0; j < MSIM_AVR_DMSZ; j++) {
				char *bit;
				char *pos;
				size_t len;
				int bitn, cr, bit_cr;

				if (mcu->ioregs[j].off < 0) {
					continue;
				}
				len = strlen(mcu->ioregs[j].name);
				if (len == 0) {
					continue;
				}

				pos = strstr(mcu->ioregs[j].name, conf->dump_regs[i]);
				cr = strncmp(mcu->ioregs[j].name, conf->dump_regs[i], len);

				/* Do we have a 16-bit register mentioned or an exact
				 * match of the register names? */
				if ((cr != 0) && (pos != NULL)) {
					if (mcu->ioregs[j].name[len-1] == 'H') {
						vcd->regs[dump_regs].i = (int32_t)j;
					}
					if (mcu->ioregs[j].name[len-1] == 'L') {
						vcd->regs[dump_regs].reg_lowi =
						        (int32_t)j;
					}

					if ((vcd->regs[dump_regs].i >= 0) &&
					                (vcd->regs[dump_regs].
					                 reg_lowi >= 0)) {
						vcd->regs[dump_regs].n = -1;
						strncpy(vcd->regs[dump_regs].name,
						        mcu->ioregs[j].name, sizeof
						        vcd->regs[dump_regs].name);
						vcd->regs[dump_regs].name[len-1] = 0;

						dump_regs++;
						break;
					}
				} else if (cr != 0) {
					continue;
				} else {
					/* Do we have a bit index suffix? */
					bit = len < sizeof conf->dump_regs[0]/
					      sizeof conf->dump_regs[0][0]
					      ? &conf->dump_regs[i][len] : NULL;
					bit_cr = sscanf(bit, "%d", &bitn);
					if (bit_cr != 1) {
						bitn = -1;
					}

					/* Set index of a register to be dumped */
					vcd->regs[dump_regs].i = (int32_t)j;
					vcd->regs[dump_regs].n = (int8_t)bitn;
					strncpy(vcd->regs[dump_regs].name,
					        mcu->ioregs[j].name,
					        sizeof vcd->regs[dump_regs].name);

					dump_regs++;
					break;
				}
			}
		}

		/* Apply memory modifications */
		if (conf->has_lockbits == 1) {
			set_lock(mcu, conf->mcu_lockbits);
		}
		if (conf->has_efuse == 1) {
			set_fuse(mcu, FUSE_EXT, conf->mcu_efuse);
		}
		if (conf->has_hfuse == 1) {
			set_fuse(mcu, FUSE_HIGH, conf->mcu_hfuse);
		}
		if (conf->has_lfuse == 1) {
			set_fuse(mcu, FUSE_LOW, conf->mcu_lfuse);
		}

		/* Try to set required frequency */
		if (conf->mcu_freq > mcu->freq) {
			snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%"
			         PRIu64 " kHz is above maximum %lu.%lu kHz",
			         conf->mcu_freq/1000U, conf->mcu_freq%1000U,
			         mcu->freq/1000UL, mcu->freq%1000UL);
			MSIM_LOG_WARN(LOG);
		} else if (conf->mcu_freq > 0U) {
			mcu->freq = (uint32_t)conf->mcu_freq;
		} else {
			snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%"
			         PRIu64 " kHz cannot be selected as clock "
			         "source",
			         conf->mcu_freq/1000U, conf->mcu_freq%1000U);
			MSIM_LOG_WARN(LOG);
		}

		/* Print MCU configuration */
		print_config(mcu);

		/* Load Lua peripherals if it is required */
		for (uint32_t k = 0; k < conf->lua_models_num; k++) {
			char *lua_model = &conf->lua_models[k][0];
			if (MSIM_AVR_LUALoadModel(mcu, lua_model)) {
				MSIM_LOG_FATAL("loading Lua model failed");
			}
		}

		/* Do we have registers to dump? */
		if (vcd->regs[0].i >= 0) {
			rc = MSIM_AVR_VCDOpen(mcu);
			if (rc != 0) {
				snprintf(LOG, LOGSZ, "failed to open a VCD "
				         "file %s", vcd->dump_file);
				MSIM_LOG_FATAL(LOG);
				rc = 1;
			}
		}

		/* Force MCU to run in a firmware-test mode. */
		if (conf->firmware_test == 1U) {
			MSIM_LOG_DEBUG("running in a firmware test mode");
			mcu->state = AVR_RUNNING;
		}
	}

	return rc;
}

static void print_config(struct MSIM_AVR *m)
{
	/* AVR memory is organized as array of bytes in the simulator, but
	 * it's natural to measure program memory in 16-bits words because
	 * all AVR instructions are 16- or 32-bits wide. This is why all
	 * program memory addresses are divided by two before printing. */
	uint64_t reset_pc = m->intr.reset_pc>>1;
	uint64_t ivt = m->intr.ivt>>1;
	uint64_t flashstart = m->flashstart>>1;
	uint64_t flashend = m->flashend>>1;
	uint64_t blsstart = m->bls.start>>1;
	uint64_t blsend = m->bls.end>>1;

	snprintf(m->log, LOGSZ, "MCU model: %s (signature %02X%02X%02X)",
	         m->name, m->signature[0], m->signature[1], m->signature[2]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "clock frequency: %" PRIu32 ".%" PRIu32
	         " kHz", m->freq/1000, m->freq%1000);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "fuses: EXT=0x%02X, HIGH=0x%02X, "
	         "LOW=0x%02X", m->fuse[2], m->fuse[1], m->fuse[0]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "lock bits: 0x%02X", m->lockbits);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "reset vector address: 0x%06" PRIX64,
	         reset_pc);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "interrupt vectors table address: "
	         "0x%06" PRIX64, ivt);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "flash section: 0x%06" PRIX64 "-0x%06" PRIX64,
	         flashstart, flashend);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "bootloader section: 0x%06" PRIX64 "-0x%06"
	         PRIX64, blsstart, blsend);
	MSIM_LOG_INFO(m->log);
}

static void update_bit(uint8_t *val, uint8_t i, uint8_t b)
{
	if (b == 0U) {
		*val &= ~(1<<i);
	} else {
		*val |= (1<<i);
	}
}
