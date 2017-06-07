/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/bootloader.h"

#define MSIM_VERSION_MAJOR	1
#define MSIM_VERSION_MINOR	0
#define MSIM_VERSION_PATCH	0
#define MSIM_VERSION_META	"a"

typedef uint32_t MSIM_AVRFlashAddr_t;

enum MSIM_AVRState {
	AVR_RUNNING = INT16_MIN,
	AVR_STOPPED,
	AVR_SLEEPING
};

enum MSIM_AVRClkSource {
	AVR_INT_CLK = INT16_MIN,
	AVR_EXT_CLK
};

enum MSIM_AVRSREGFlag {
	AVR_SREG_CARRY = INT16_MIN,
	AVR_SREG_ZERO,
	AVR_SREG_NEGATIVE,
	AVR_SREG_TWOSCOM_OF,
	AVR_SREG_SIGN,
	AVR_SREG_HALF_CARRY,
	AVR_SREG_BITCOPY_ST,
	AVR_SREG_GLOB_INT
};

/* Instance of the AVR microcontroller. */
struct MSIM_AVR {
	uint32_t id;			/* ID of a simulated AVR MCU */

	char name[20];			/* Name of the MCU */
	uint8_t signature[3];		/* Signature of the MCU */
	uint16_t spm_pagesize;		/* For devices with bootloader support,
					   the flash pagesize (in bytes) to be
					   used for Self Programming Mode (SPM)
					   instruction. */
	uint32_t flashstart;		/* The first byte address in flash
					   program space, in bytes. */
	uint32_t flashend;		/* The last byte address in flash
					   program space, in bytes. */
	struct MSIM_AVRBootloader *boot_loader;
	uint32_t ramstart;
	uint32_t ramend;
	uint32_t ramsize;
	uint16_t e2start;		/* The first EEPROM address */
	uint16_t e2end;			/* The last EEPROM address */
	uint16_t e2size;
	uint16_t e2pagesize;		/* The size of the EEPROM page */
	uint8_t lockbits;
	uint8_t fuse[6];

	enum MSIM_AVRState state;
	enum MSIM_AVRClkSource clk_source;
	uint32_t freq;			/* Frequency we're currently
					   working at, in kHz */
	MSIM_AVRFlashAddr_t pc;		/* Current program counter register */
	MSIM_AVRFlashAddr_t reset_pc;	/* This is a value used to jump to
					   at reset time. */
	MSIM_AVRFlashAddr_t ivt;	/* Address of Interrupt Vectors Table
					   in program memory. */

	uint8_t *sp_high;		/* SPH in the data memory */
	uint8_t *sp_low;		/* SPL in the data memory */
	uint8_t *sreg;			/* Points directly to SREG placed
					   in data section. */

	uint8_t *prog_mem;		/* Flash memory. This memory
					   section could contain
					   a bootloader. */
	uint8_t *data_mem;		/* General purpose registers,
					   IO registers and SRAM */
	uint32_t pm_size;		/* Actual size of the program memory. */
	uint32_t dm_size;		/* Actual size of the data memory. */


	uint32_t sfr_off;		/* Offset to the AVR special function
					   registers. */
	int16_t io_addr[IO_REGS];	/* I/O registers addresses */
};

#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/simm8a.h"
#include "mcusim/avr/sim/simm328p.h"

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
