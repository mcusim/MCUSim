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
#ifndef MSIM_AVR_INIT_H_
#define MSIM_AVR_INIT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct MSIM_InitArgs {
	uint8_t *pm;
	uint8_t *dm;
	uint32_t pmsz;
	uint32_t dmsz;
} MSIM_InitArgs;

/* Initialize MCU as ATmega8A */
int MSIM_M8AInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega328 */
int MSIM_M328Init(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega328P */
int MSIM_M328PInit(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Initialize MCU as ATmega2560 */
int MSIM_M2560Init(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_INIT_H_ */
