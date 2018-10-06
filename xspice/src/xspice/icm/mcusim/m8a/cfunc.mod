/*
 * Copyright (c) 2018, The MCUSim Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
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
 *	Atmel ATmega8A microcontroller provided by MCUSim.
 */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "mcusim/mcusim.h"

void MSIM_M8AModel(ARGS)
{
	int i;				/* generic loop counter index */
	int size;			/* number of input & output ports */
	Digital_State_t *out;		/* temporary output for buffers */
	Digital_State_t *out_old; 	/* previous output for buffers  */
	Digital_State_t input;		/* temp storage for input bits  */

	/* Retrieve size value... */
	size = PORT_SIZE(Bin);

	/* Setup required state variables */
	if (INIT == 1) {  /* initial pass */
		MSIM_PrintParts();
		/* allocate storage for the outputs */
		cm_event_alloc(0, sizeof(Digital_State_t));

		/* retrieve storage for the outputs */
		out = (Digital_State_t *) cm_event_get_ptr(0,0);
		out_old = out;
	} else {      /* Retrieve previous values */
		/* retrieve storage for the outputs */
		out = (Digital_State_t *) cm_event_get_ptr(0,0);
		out_old = (Digital_State_t *) cm_event_get_ptr(0,1);
	}

	/* Calculate new output value based on inputs */
	*out = ONE;

	for (i = 0; i < size; i++) {
		/* make sure this input isn't floating... */
		if (PORT_NULL(Bin) == FALSE) {
			/* if a 0, set *out low */
			input = INPUT_STATE(Bin[i]);
			if (input == ZERO) {
				*out = ZERO;
				break;
			} else {
				/* if an unknown input, set *out to unknown */
				if (input == UNKNOWN) {
					*out = UNKNOWN;
				}
			}
		} else {
			/* at least one port is floating...output is unknown */
			*out = UNKNOWN;
			break;
		}
	}

	/* Determine analysis type and output appropriate values */
	if (ANALYSIS == DC) {   /** DC analysis...output w/o delays **/
		OUTPUT_STATE(Bout[0]) = *out;
	} else {      /** Transient Analysis **/
		if (*out != *out_old) { /* output value is changing */
			switch (*out) {
			/* fall to zero value */
			case 0:
				OUTPUT_STATE(Bout[0]) = ZERO;
				OUTPUT_DELAY(Bout[0]) = 0.01;
				break;
			/* rise to one value */
			case 1:
				OUTPUT_STATE(Bout[0]) = ONE;
				OUTPUT_DELAY(Bout[0]) = 0.01;
				break;
			/* unknown output */
			default:
				*out = UNKNOWN;
				OUTPUT_STATE(Bout[0]) = UNKNOWN;

				/* based on old value, add rise or fall delay */
				if (0 == *out_old) { /* add rising delay */
					OUTPUT_DELAY(Bout[0]) = 0.01;
				} else { /* add falling delay */
					OUTPUT_DELAY(Bout[0]) = 0.01;
				}
				break;
			}
		} else { /* output value not changing */
			OUTPUT_CHANGED(Bout[0]) = FALSE;
		}
	}
	OUTPUT_STRENGTH(Bout[0]) = STRONG;
}

