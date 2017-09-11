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

#include "mcusim/avr/sim/bootloader.h"

#define SIM_NAME		"avrsim"

#define FUSE_LOW		0
#define FUSE_HIGH		1
#define FUSE_EXT		2

typedef unsigned long MSIM_AVRFlashAddr_t;

/* Device-specific functions to set fuse and lock bytes. */
typedef int (*MSIM_SetFuse_f)(void *mcu,
			      unsigned int fuse_n, unsigned char fuse_v);
typedef int (*MSIM_SetLock_f)(void *mcu, unsigned char lock_v);

enum MSIM_AVRState {
	AVR_RUNNING,
	AVR_STOPPED,
	AVR_SLEEPING
};

enum MSIM_AVRClkSource {
	AVR_INT_CLK,
	AVR_EXT_CLK,
	AVR_LOWP_CRYSTAL_CLK,		/* Low power crystal */
	AVR_FULLSWING_CRYSTAL_CLK,	/* Full swing crystal */
	AVR_LOWFREQ_CRYSTAL_CLK,	/* Low frequency crystal */
	AVR_EXT_LOWF_CRYSTAL_CLK,	/* External low-freq crystal */
	AVR_INT_CAL_RC_CLK,		/* Internal calibrated RC */
	AVR_EXT_RC_CLK,			/* External RC */
	AVR_EXT_CRYSTAL			/* External crystal/ceramic resonator */
};

enum MSIM_AVRSREGFlag {
	AVR_SREG_CARRY,
	AVR_SREG_ZERO,
	AVR_SREG_NEGATIVE,
	AVR_SREG_TWOSCOM_OF,
	AVR_SREG_SIGN,
	AVR_SREG_HALF_CARRY,
	AVR_SREG_T_BIT,
	AVR_SREG_GLOB_INT
};

/* Instance of the AVR microcontroller. */
struct MSIM_AVR {
	unsigned long id;		/* ID of a simulated AVR MCU */
	char name[20];			/* Name of the MCU */
	unsigned char signature[3];	/* Signature of the MCU */

	unsigned long flashstart;	/* The first byte address in PM. */
	unsigned long flashend;		/* The last byte address in PM. */
	unsigned long ramstart;
	unsigned long ramend;
	unsigned long ramsize;
	unsigned int e2start;		/* The first EEPROM address */
	unsigned int e2end;		/* The last EEPROM address */
	unsigned int e2size;
	unsigned int e2pagesize;	/* The size of the EEPROM page */
	unsigned char lockbits;
	unsigned char fuse[6];

	unsigned int spm_pagesize;	/* For devices with bootloader support,
					   the flash pagesize (in bytes) to be
					   used for Self Programming Mode (SPM)
					   instruction. */
	unsigned char *spmcsr;		/* Store Program Memory Control
					   and Status Register (SPMCSR). */

	struct MSIM_AVRBootloader *bls;
	enum MSIM_AVRState state;
	enum MSIM_AVRClkSource clk_source;

	unsigned long freq;		/* Current MCU frequency, Hz*/
	unsigned char pc_bits;		/* 16-bit PC, 22-bit PC, etc. */
	MSIM_AVRFlashAddr_t pc;		/* Current program counter. */
	MSIM_AVRFlashAddr_t reset_pc;	/* Reset program counter. */
	MSIM_AVRFlashAddr_t ivt;	/* Address of Interrupt Vectors Table
					   in program memory. */

	unsigned char *sreg;		/* SREG in the data memory. */
	unsigned char *sph;		/* SP(high) in the data memory. */
	unsigned char *spl;		/* SP(low) in the data memory. */

	unsigned char *eind;		/* Extended indirect register. */
	unsigned char *rampz;		/* Extended Z-pointer register. */
	unsigned char *rampy;		/* Extended Y-pointer register. */
	unsigned char *rampx;		/* Extended X-pointer register. */
	unsigned char *rampd;		/* Extended direct register.
					   NOTE: Let me know if you're aware
					   of AVR MCUs which use this register
					   at darkness.bsd at gmail.com */

	unsigned char *pm;		/* Flash memory (+bootloader). */
	unsigned char *pmp;		/* Page buffer of the flash memory. */
	unsigned char *dm;		/* GP and I/O registers, SRAM. */
	unsigned long pm_size;		/* Actual size of the program memory. */
	unsigned long dm_size;		/* Actual size of the data memory. */


	unsigned int sfr_off;		/* Offset to the AVR special function
					   registers. */
	unsigned int regs;		/* Number of GP registers. */
	unsigned int io_regs;		/* Number of all I/O registers. */

	MSIM_SetFuse_f set_fusef;	/* Function to set AVR fuse byte. */
	MSIM_SetLock_f set_lockf;	/* Function to set AVR lock byte. */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
