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
 *	This file contains the functional description of the microcontrollers
 *	provided by MCUSim.
 */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "mcusim/mcusim.h"
#include "mcusim/config.h"
#include "mcusim/log.h"

/* Local macros */
#define LOG			(mcu->log)
#define LOGSZ			MSIM_AVR_LOGSZ

/* Default GDB RSP port */
#define GDB_RSP_PORT		12750

static void setup_ports_load(ARGS);
static void ports_to_unknown_state(ARGS);
static void ports_not_changed(ARGS);

static int setup_avr(struct MSIM_AVR *mcu, struct MSIM_CFG *cfg);
static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val);
static int set_lock(struct MSIM_AVR *m, uint8_t val);

void MSIM_CM(ARGS)
{
	Digital_State_t *rst;		/* Storage for reset value */
	Digital_State_t *rst_old;	/* Previous reset value */
	Digital_State_t *clk;		/* Storage for clock value */
	Digital_State_t *clk_old;	/* Previous clock value */
	struct MSIM_AVR *mcu;		/* AVR MCU descriptor */
	struct MSIM_CFG *cfg;		/* Configuration of the simulator */
	char *conf_file;
	int conf_rc;

	if (INIT == 1) {
#ifdef DEBUG
		/* Adjust logging level in the debug version. */
		MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif

		/* Allocate memory for the even-driven model structures. */
		cm_event_alloc(0, sizeof(Digital_State_t));
		cm_event_alloc(1, sizeof(Digital_State_t));

		clk = (Digital_State_t *) cm_event_get_ptr(0, 0);
		rst = (Digital_State_t *) cm_event_get_ptr(1, 0);
		clk_old = clk;
		rst_old = rst;

		/* Allocate memory for static variables. */
		STATIC_VAR(mcu) = malloc(sizeof(struct MSIM_AVR));
		STATIC_VAR(cfg) = malloc(sizeof(struct MSIM_CFG));

		mcu = STATIC_VAR(mcu);
		cfg = STATIC_VAR(cfg);

		/* Set up capacitive load values. */
		setup_ports_load(mif_private);
		LOAD(clk) = PARAM(clk_load);
		if (!PORT_NULL(reset)) {
			LOAD(reset) = PARAM(reset_load);
		}

		/* Load MCUSim configuration file */
		conf_file = PARAM(config_file);
		conf_rc = MSIM_CFG_Read(cfg, conf_file);
		if (conf_rc != 0) {
			snprintf(LOG, LOGSZ, "cannot load configuration "
			         "file: %s", conf_file);
			MSIM_LOG_FATAL(LOG);
		} else {
			snprintf(LOG, LOGSZ, "using config file: %s",
			         conf_file);
			MSIM_LOG_INFO(LOG);
		}
	} else {
		mcu = STATIC_VAR(mcu);
		cfg = STATIC_VAR(cfg);

		clk = (Digital_State_t *) cm_event_get_ptr(0, 0);
		clk_old = (Digital_State_t *) cm_event_get_ptr(0, 1);
		rst = (Digital_State_t *) cm_event_get_ptr(1, 0);
		rst_old = (Digital_State_t *) cm_event_get_ptr(1, 1);
	}

	if (ANALYSIS == TRANSIENT) {
		/* Update clock and reset values. */
		*clk = INPUT_STATE(clk);
		if (PORT_NULL(reset)) {
			*rst = ZERO;
			*rst_old = ZERO;
		} else {
			*rst = INPUT_STATE(reset);
		}

		if ((*clk != *clk_old) && (*clk == ONE)) {
			/* Rise of the clock value. */
		} else {
			/* Fall of the clock value. */
			ports_not_changed(mif_private);
		}
	} else {
		/* Unsupported type of analysis: output without delays */
		ports_to_unknown_state(mif_private);
	}
}

static void setup_ports_load(ARGS)
{
	for (uint32_t i = 0; i < PORT_SIZE(Ain); i++) {
		LOAD(Ain[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Bin); i++) {
		LOAD(Bin[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Cin); i++) {
		LOAD(Cin[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Din); i++) {
		LOAD(Din[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Ein); i++) {
		LOAD(Ein[i]) = PARAM(input_load);
	}
	for (uint32_t i = 0; i < PORT_SIZE(Fin); i++) {
		LOAD(Fin[i]) = PARAM(input_load);
	}
}

static void ports_to_unknown_state(ARGS)
{
	for (uint32_t i = 0; i < PORT_SIZE(Aout); i++) {
		OUTPUT_STATE(Aout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Aout[i]) = HI_IMPEDANCE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Bout); i++) {
		OUTPUT_STATE(Bout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Bout[i]) = HI_IMPEDANCE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Cout); i++) {
		OUTPUT_STATE(Cout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Cout[i]) = HI_IMPEDANCE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Dout); i++) {
		OUTPUT_STATE(Dout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Dout[i]) = HI_IMPEDANCE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Eout); i++) {
		OUTPUT_STATE(Eout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Eout[i]) = HI_IMPEDANCE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Fout); i++) {
		OUTPUT_STATE(Fout[i]) = UNKNOWN;
		OUTPUT_STRENGTH(Fout[i]) = HI_IMPEDANCE;
	}
}

static void ports_not_changed(ARGS)
{
	for (uint32_t i = 0; i < PORT_SIZE(Aout); i++) {
		OUTPUT_CHANGED(Aout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Bout); i++) {
		OUTPUT_CHANGED(Bout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Cout); i++) {
		OUTPUT_CHANGED(Cout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Dout); i++) {
		OUTPUT_CHANGED(Dout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Eout); i++) {
		OUTPUT_CHANGED(Eout[i]) = FALSE;
	}
	for (uint32_t i = 0; i < PORT_SIZE(Fout); i++) {
		OUTPUT_CHANGED(Fout[i]) = FALSE;
	}
}

static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val)
{
	int rc = 0;

	if (m->set_fusef == NULL) {
		MSIM_LOG_WARN("cannot modify fuse (MCU-specific function is "
		              "not available)");
	}
	m->set_fusef(m, fuse, val);
	return rc;
}

static int set_lock(struct MSIM_AVR *m, uint8_t val)
{
	int rc = 0;

	if (m->set_lockf == NULL) {
		MSIM_LOG_WARN("cannot modify lock bits (MCU-specific function "
		              "is not available)");
	}
	m->set_lockf(m, val);
	return rc;
}

static int setup_avr(struct MSIM_AVR *mcu, struct MSIM_CFG *cfg)
{
	return 0;
}
