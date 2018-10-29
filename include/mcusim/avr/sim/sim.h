/*
 * Copyright (c) 2017, 2018, The MCUSim Contributors
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
 * This is a main header file which contains declarations to describe the
 * whole simulated microcontroller, and it's supposed to be AVR-agnostic.
 * It means that each declaration should be suitable for every available AVR
 * model.
 */
#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

/* Limits of statically allocated MCU memory */
#define MSIM_AVR_PMSZ		(256*1024)	/* for program memory */
#define MSIM_AVR_DMSZ		(64*1024)	/* for data memory */
#define MSIM_AVR_PM_PAGESZ	(1024)		/* for PM page */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/wdt.h"
#include "mcusim/avr/sim/pty.h"
#include "mcusim/avr/sim/usart.h"

/* Forward declaration of the structure to describe AVR microcontroller
 * instance. */
struct MSIM_AVR;

/* Simulated MCU may provide its own implementations of the functions in order
 * to support these features (fuses, locks, timers, IRQs, etc.). */
typedef int (*MSIM_AVR_SetFuse_f)(struct MSIM_AVR *mcu, uint32_t fuse_n,
                                  uint8_t fuse_v);
typedef int (*MSIM_AVR_SetLock_f)(struct MSIM_AVR *mcu, uint8_t lock_v);
typedef int (*MSIM_AVR_TickPerf_f)(struct MSIM_AVR *mcu);
typedef int (*MSIM_AVR_PassIRQs_f)(struct MSIM_AVR *mcu);

/* State of a simulated AVR microcontroller. Some of these states are
 * AVR-native, others - added by the simulator to manipulate a simulation
 * process. */
enum MSIM_AVR_State {
	AVR_RUNNING,
	AVR_STOPPED,
	AVR_SLEEPING,
	AVR_MSIM_STEP,			/* Execute next instruction */
	AVR_MSIM_STOP,			/* Terminate sim (correctly) */
	AVR_MSIM_TESTFAIL		/* Terminate sim (test failure) */
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

enum MSIM_AVR_SREGFlag {
	AVR_SREG_CARRY,
	AVR_SREG_ZERO,
	AVR_SREG_NEGATIVE,
	AVR_SREG_TWOSCOM_OF,
	AVR_SREG_SIGN,
	AVR_SREG_HALF_CARRY,
	AVR_SREG_T_BIT,
	AVR_SREG_GLOB_INT
};

/* Structure to describe a memory operation requested by user. */
struct MSIM_AVR_MemOp {
	char memtype[16];		/* Type of MCU memory */
	char operation;			/* Memory operation */
	char operand[4096];		/* Path to file, value, etc. */
	char format;			/* Optional, value format */
};

/* Instance of the AVR microcontroller. */
struct MSIM_AVR {
	char name[20];			/* Name of the MCU */
	char log[1024];			/* Buffer to print a log message */
	uint8_t signature[3];		/* Signature of the MCU */
	uint8_t xmega;			/* XMEGA flag */
	uint8_t reduced_core;		/* Reduced core flag */

	uint32_t flashstart;		/* First program memory byte */
	uint32_t flashend;		/* Last program memory byte */
	uint32_t ramstart;		/* First on-chip SRAM byte */
	uint32_t ramend;		/* Last on-chip SRAM byte */
	uint32_t ramsize;		/* On-chip SRAM size, in bytes */
	uint32_t e2start;		/* First EEPROM byte */
	uint32_t e2end;			/* Last EEPROM byte */
	uint32_t e2size;		/* EEPROM size, in bytes */
	uint32_t e2pagesize;		/* EEPROM page size, in bytes */
	uint8_t lockbits;		/* Lock bits of the MCU */
	uint8_t fuse[6];		/* Fuse bytes of the MCU */

	uint32_t spm_pagesize;		/* Flash pagesize (in bytes) for SPM */
	uint8_t *spmcsr;		/* SPMCSR Register (address in DM) */

	uint32_t freq;			/* Current MCU frequency, Hz */
	uint32_t pc;			/* Current program counter */
	uint8_t pc_bits;		/* 16-bit PC, 22-bit PC, etc. */
	uint8_t ic_left;		/* Cycles left to finish opcode */
	uint8_t in_mcinst;		/* Multi-cycle instruction flag */

	uint8_t *sreg;			/* SREG address (in DM) */
	uint8_t *sph;			/* SPH address (in DM) */
	uint8_t *spl;			/* SPL address (in DM) */
	uint8_t *eind;			/* Extended indirect reg (in DM) */
	uint8_t *rampz;			/* Extended Z-pointer reg (in DM) */
	uint8_t *rampy;			/* Extended Y-pointer reg (in DM) */
	uint8_t *rampx;			/* Extended X-pointer reg (in DM) */
	uint8_t *rampd;			/* Extended direct reg (in DM) */

	uint8_t pm[MSIM_AVR_PMSZ];	/* Program memory (PM) */
	uint8_t pmp[MSIM_AVR_PM_PAGESZ];/* Page buffer for PM */
	uint8_t mpm[MSIM_AVR_PMSZ];	/* Match points memory, MPM */
	uint8_t dm[MSIM_AVR_DMSZ];	/* Data memory (GP, I/O regs, SRAM) */
	uint32_t pm_size;		/* Size of PM */
	uint32_t dm_size;		/* Size of the data memory */
	uint8_t read_from_mpm;		/* 1 - read opcode from MPM */
	int32_t writ_io[4];		/* I/O regs written (previous tick) */
	int32_t read_io[4];		/* I/O regs read (previous tick) */

	uint32_t sfr_off;		/* Offset to SFR (I/0 registers) */
	uint32_t regs_num;		/* Number of GP registers */
	uint32_t ioregs_num;		/* Number of all I/O registers */

	MSIM_AVR_SetFuse_f set_fusef;	/* Function to set AVR fuse byte */
	MSIM_AVR_SetLock_f set_lockf;	/* Function to set AVR lock byte */
	MSIM_AVR_TickPerf_f tick_perf;	/* Function to tick 8-bit timers */
	MSIM_AVR_PassIRQs_f pass_irqs;	/* Function to set IRQs */

	enum MSIM_AVR_State state;	 /* State of the MCU */
	enum MSIM_AVR_ClkSource clk_source; /* Clock source */

	struct MSIM_AVR_IOReg
		ioregs[MSIM_AVR_DMSZ]; /* Descriptors of the I/O registers */
	struct MSIM_AVR_Bld bls;	/* Bootloader Section */
	struct MSIM_AVR_Int intr;	/* Interrupts and IRQs */
	struct MSIM_AVR_Wdt wdt;	/* Watchdog Timer */
	struct MSIM_AVR_VCDReg vcd[MSIM_AVR_VCD_REGS]; /* VCD (dump) file details */
	struct MSIM_AVR_Pty pty;	/* Details to work with pseudo-term */
	struct MSIM_AVR_Usart usart;	/* Details to work with USART */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
