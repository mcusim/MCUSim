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

/* An MCU-agnostic way to access an I/O port. */
typedef struct MSIM_AVR_IOPort {
	MSIM_AVR_IOBit port;	/* PORTx (in data space) */
	MSIM_AVR_IOBit ddr;	/* DDRx (in data space) */
	MSIM_AVR_IOBit pin;	/* PINx (in data space) */
} MSIM_AVR_IOPort;

int MSIM_AVR_IOUpdatePinx(struct MSIM_AVR *mcu);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_IO_H_ */
