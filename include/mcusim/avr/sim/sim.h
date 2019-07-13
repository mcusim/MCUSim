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
#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

#include <stdint.h>
#include <pthread.h>
#include "mcusim/pty.h"
#include "mcusim/tsq.h"
#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/wdt.h"
#include "mcusim/avr/sim/usart.h"
#include "mcusim/avr/sim/interrupt.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/timer.h"

#define MSIM_AVR_PMSZ		(256*1024)	/* Program Memory size */
#define MSIM_AVR_PM_PAGESZ	(1024)		/* PM page size */
#define MSIM_AVR_DMSZ		(64*1024)	/* Data Memory size */
#define MSIM_AVR_LOGSZ		(64*1024)	/* Log buffer size */
#define MSIM_AVR_MAXTMRS	(32)		/* Maximum # of timers */
#define MSIM_AVR_MAXIOPORTS	(32)		/* Maximum # of I/O ports */

#ifdef __cplusplus
extern "C" {
#endif

struct MSIM_AVR;
struct MSIM_AVRConf;

/* Simulated MCU may provide its own implementations of the functions in order
 * to support these features (fuses, locks, timers, IRQs, etc.). */
typedef int (*MSIM_AVRFunc)(struct MSIM_AVR *mcu, struct MSIM_AVRConf *cnf);

/* State of a simulated AVR microcontroller. Some of these states are
 * AVR-native, others - added by the simulator to manipulate a simulation
 * process. */
enum MSIM_AVR_State {
	AVR_RUNNING,
	AVR_STOPPED,
	AVR_SLEEPING,
	AVR_MSIM_STEP,			/* Step (with calling subroutine) */
	AVR_MSIM_STOP,			/* Terminate (correctly) */
	AVR_MSIM_TESTFAIL,		/* Terminate (test failure) */
	AVR_MSIM_STEPOVER,		/* Step (without calling subroutine) */
};

enum MSIM_AVR_ClkSource {
	AVR_INT_CLK,
	AVR_EXT_CLK,
	AVR_LOWP_CRYSTAL_CLK,		/* Low power crystal */
	AVR_FULLSWING_CRYSTAL_CLK,	/* Full swing crystal */
	AVR_LOWFREQ_CRYSTAL_CLK,	/* Low frequency crystal */
	AVR_EXT_LOWF_CRYSTAL_CLK,	/* External low-freq crystal */
	AVR_INT_CAL_RC_CLK,		/* Internal calibrated RC */
	AVR_EXT_RC_CLK,			/* External RC */
	AVR_EXT_CRYSTAL,		/* External crystal/ceramic resonator*/
	AVR_INT_128K_RC_CLK		/* Internal 128kHz RC Oscillator*/
};

/* Configuration to be passed to the MCU-specific functions. */
typedef struct MSIM_AVRConf {
	uint32_t fuse_n;
	uint8_t fuse_v;
	uint8_t lock_v;
} MSIM_AVRConf;

/* Instance of the 8-bit AVR microcontroller */
typedef struct MSIM_AVR {
	char name[20];			/* Name of the MCU */
	char log[MSIM_AVR_LOGSZ];	/* Buffer to print a log message to */
	uint8_t signature[3];		/* Signature of the MCU */

	uint8_t xmega;			/* AVR XMega flag */
	uint8_t reduced_core;		/* Reduced AVR core flag */

	uint64_t tick;			/* Cycles passed sinse reset */
	uint8_t tovf;			/* Cycles overflow flag */

	uint32_t flashstart;		/* First byte of the PM */
	uint32_t flashend;		/* Last byte of the PM */
	uint32_t ramstart;		/* First byte of the on-chip SRAM */
	uint32_t ramend;		/* Last byte of the on-chip SRAM */
	uint32_t ramsize;		/* On-chip SRAM size, in bytes */
	uint32_t e2start;		/* First EEPROM byte */
	uint32_t e2end;			/* Last EEPROM byte */
	uint32_t e2size;		/* EEPROM size, in bytes */
	uint32_t e2pagesize;		/* EEPROM page size, in bytes */

	uint8_t lockbits;		/* Lock bits  */
	uint8_t fuse[6];		/* Fuse bytes */

	uint32_t spm_pagesize;		/* PM page size, in bytes (for SPM) */
	uint8_t *spmcsr;		/* SPMCSR register address */

	uint32_t freq;			/* Clock frequency, in Hz */
	pthread_mutex_t freq_mutex;	/* Lock before accessing frequency */

	uint32_t pc;			/* Program counter, in bytes */
	uint8_t pc_bits;		/* PC bits (16-bit, 22-bit, etc.) */

	uint8_t ic_left;		/* Cycles to finish cur. instruction */
	uint8_t mci;			/* Multi-cycle instruction flag */

	uint8_t *sreg;			/* SREG register pointer */
	uint8_t *sph;			/* SPH register pointer */
	uint8_t *spl;			/* SPL register pointer */
	uint8_t *eind;			/* EIND register pointer */
	uint8_t *rampz;			/* Ext. Z-pointer register pointer */
	uint8_t *rampy;			/* Ext. Y-pointer register pointer */
	uint8_t *rampx;			/* Ext. X-pointer register pointer */
	uint8_t *rampd;			/* Ext. direct register pointer */

	uint16_t pm[MSIM_AVR_PMSZ];	/* Program memory (PM) */
	uint16_t pmp[MSIM_AVR_PMSZ];	/* Page buffer for program memory */
	uint16_t mpm[MSIM_AVR_PMSZ];	/* Match points memory (MPM) */
	uint32_t pm_size;		/* Actual PM size */
	uint8_t read_from_mpm;		/* Read instruction from MPM flag */

	uint8_t dm[MSIM_AVR_DMSZ];	/* Data memory (DM) */
	uint32_t dm_size;		/* Actual DM size */

	uint32_t writ_io[4];		/* I/O written on a previous cycle */
	uint32_t read_io[4];		/* I/O read on a previous cycle */

	uint32_t sfr_off;		/* Offset to I/O registers in DM */
	uint32_t regs_num;		/* # of general purpose registers */
	uint32_t ioregs_num;		/* # of I/O registers */

	MSIM_AVRFunc set_fusef;		/* Configure AVR fuses */
	MSIM_AVRFunc set_lockf;		/* Configure AVR lock bits */
	MSIM_AVRFunc tick_perf;		/* Tick AVR peripherals */
	MSIM_AVRFunc pass_irqs;		/* Provide IRQs */
	MSIM_AVRFunc reset_spm;		/* Reset SPM instruction */

	enum MSIM_AVR_State state;	/* State of the MCU */
	pthread_mutex_t state_mutex;	/* Lock before accessing MCU state */
	enum MSIM_AVR_ClkSource clk_source; /* Current MCU clock source */

	MSIM_AVR_BLD bls;		/* Bootloader section details */
	MSIM_AVR_INT intr;		/* Details to work with IRQs */
	MSIM_AVR_WDT wdt;		/* Watchdog timer of the MCU */
	MSIM_AVR_VCD vcd;		/* Details to work with VCD file */
	MSIM_AVR_USART usart;		/* Details to work with USART */
	MSIM_PTY pty;			/* Details to work with POSIX PTY */

	MSIM_AVR_IOReg ioregs[MSIM_AVR_DMSZ];		/* I/O registers */
	MSIM_AVR_IOPort ioports[MSIM_AVR_MAXIOPORTS];	/* I/O ports */
	MSIM_AVR_TMR timers[MSIM_AVR_MAXTMRS];		/* Timers/counters */
} MSIM_AVR;

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
