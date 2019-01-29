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
#ifndef MSIM_AVR_SIMCORE_H_
#define MSIM_AVR_SIMCORE_H_ 1

#ifndef MSIM_MAIN_HEADER_H_
#error "Please, include mcusim/mcusim.h instead of this header."
#endif

#include <stdio.h>
#include <stdint.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/init.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Main simulation routine. It performs a required number of
 * steps (instructions).
 *
 * Zero number of steps could be used to run an infinite simulation
 * (until the end of the program or first breakpoint appeared).
 * The infinite simulation could be interrupted by the execution process
 * reached the given address. Addresses within the program space are taken
 * into account, only.
 *
 * Simulator can be started in firmware test mode, i.e. no debuggers or
 * any external events are necessary to perform a simulation. */
int MSIM_AVR_Simulate(struct MSIM_AVR *mcu, uint64_t steps, uint64_t addr,
                      uint8_t firmware_test);

/* Initializes an MCU into specific model determined by the given name.
 * It is, generally, a good idea to prepare specific MCU model using this
 * function instead of MSIM_XXXInit() ones. */
int MSIM_AVR_Init(struct MSIM_AVR *mcu, const char *mcu_name,
                  unsigned char *pm, unsigned long pm_size,
                  unsigned char *dm, unsigned long dm_size,
                  unsigned char *mpm, FILE *fp);

/* Functions to work with a stack inside MCU */
void MSIM_AVR_StackPush(struct MSIM_AVR *mcu, unsigned char val);
uint8_t MSIM_AVR_StackPop(struct MSIM_AVR *mcu);

/* Prints supported AVR parts. */
void MSIM_AVR_PrintParts(void);

/* This function dumps a content of AVR flash memory to the 'dump' file.
 *
 * It can be loaded back instead of the regular AVR firmware specified by the
 * configuration file. */
int MSIM_AVR_DumpFlash(struct MSIM_AVR *mcu, const char *dump);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIMCORE_H_ */
