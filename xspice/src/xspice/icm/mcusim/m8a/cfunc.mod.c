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
#include "mcusim/bit/macro.h"
#include "mcusim/avr/sim/m8/m8a.h"
#include "mcusim/avr/sim/macro.h"

#define MCU_MODEL		"m8a"
#define TICKS_MAX		UINT64_MAX

/* Macro to convert Digital_State_t to a bit in uint8_t. */
#define TO_BIT(ds, b) do {		\
	switch ((ds)) {			\
	case ZERO:			\
		(b) = 0;		\
		break;			\
	case ONE:			\
		(b) = 1;		\
		break;			\
	default:			\
		(b) = rand()&1;		\
		break;			\
	}				\
} while (0)

static struct MSIM_AVR _mcu;
static struct MSIM_CFG _cfg;
static uint64_t _tick;
static uint8_t _tick_ovf;

static void setup_ports_load(ARGS);
static void ports_not_changed(ARGS);

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
		MSIM_AVR_Init(mcu, cfg, PARAM(config_file));
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
					UPDATE_BIT(&pval, i, b);
				}
			}
			DM(PINB) = pval & (~DM(DDRB));

			pval = DM(PINC);
			for (uint32_t i = 0; i < PORT_SIZE(Cin); i++) {
				if ((PORT_NULL(Cin) == 0) &&
				    (INPUT_STRENGTH(Cin[i]) != HI_IMPEDANCE)) {
					TO_BIT(INPUT_STATE(Cin[i]), b);
					UPDATE_BIT(&pval, i, b);
				}
			}
			DM(PINC) = pval & (~DM(DDRC));

			pval = DM(PIND);
			for (uint32_t i = 0; i < PORT_SIZE(Din); i++) {
				if ((PORT_NULL(Din) == 0) &&
				    (INPUT_STRENGTH(Din[i]) != HI_IMPEDANCE)) {
					TO_BIT(INPUT_STATE(Din[i]), b);
					UPDATE_BIT(&pval, i, b);
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
