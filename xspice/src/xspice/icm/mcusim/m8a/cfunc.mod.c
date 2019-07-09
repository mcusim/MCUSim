/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
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
#include "mcusim/bit/private/macro.h"
#include "mcusim/avr/sim/m8/m8a.h"
#include "mcusim/avr/sim/private/macro.h"

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

static void setup_ports_load(ARGS);
static void ports_not_changed(ARGS);

void
MSIM_CM_M8A(ARGS)
{
	Digital_State_t *clk;		/* Storage for clock value */
	Digital_State_t *clk_old;	/* Previous clock value */
	struct MSIM_AVR *mcu;		/* AVR MCU descriptor */
	struct MSIM_CFG *cfg;		/* Configuration of the simulator */

	if (INIT) {
		MSIM_CFG_PrintVersion();

		/* Allocate memory for the even-driven model structures. */
		cm_event_alloc(0, sizeof(Digital_State_t));
		cm_event_alloc(1, sizeof(Digital_State_t));

		clk = (Digital_State_t *) cm_event_get_ptr(0, 0);
		clk_old = clk;

		mcu = &_mcu;
		cfg = &_cfg;

		/* Set up capacitive load values. */
		setup_ports_load(mif_private);
		LOAD(clk) = PARAM(clk_load);
		if (!PORT_NULL(reset)) {
			LOAD(reset) = PARAM(reset_load);
		}

		/* Read config file */
		MSIM_CFG_Read(cfg, PARAM(config_file));

		/* Force 'm8a' model */
		snprintf(cfg->mcu, ARRSZ(cfg->mcu), "m8a");

		/* Set up an instance of the ATmega8A */
		MSIM_AVR_Init(mcu, cfg);
	} else {
		mcu = &_mcu;
		cfg = &_cfg;

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
			MSIM_AVR_SimStep(mcu, cfg->firmware_test);

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

static void
setup_ports_load(ARGS)
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

static void
ports_not_changed(ARGS)
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
