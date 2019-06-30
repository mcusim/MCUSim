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
 */
#ifndef MSIM_AVR_IO_H_
#define MSIM_AVR_IO_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* I/O register of the AVR microcontroller */
typedef struct MSIM_AVR_IOReg {
	char name[16];
	int32_t off;		/* Register address (in data space) */
	uint8_t *addr;		/* Pointer to the register in DM */
	uint8_t reset;		/* Value after MCU reset */
	uint8_t mask;		/* Access mask (1 - R/W or W, 0 - R) */
} MSIM_AVR_IOReg;

/*
 * It provides a way to access bits of the AVR I/O register (or fuse byte)
 * in MCU-agnostic way:
 *
 *         (DM(reg) >> bit) & mask
 */
typedef struct MSIM_AVR_IOBit {
	uint32_t reg;		/* Register address (offset in data space) */
	uint32_t mask;		/* Bit mask */
	uint8_t bit;		/* Index of a bit in the register */
	uint8_t mbits;		/* Number of mask bits */
} MSIM_AVR_IOBit, MSIM_AVR_IOFuse;

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_IO_H_ */
