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
 * whole simulated microcontroller, and it is supposed to be AVR-agnostic.
 * It means that each declaration should be suitable for every available AVR
 * model.
 */
#ifndef MSIM_AVR_SIM_H_
#define MSIM_AVR_SIM_H_ 1

/* Limits of statically allocated MCU memory */
#define MSIM_AVR_PMSZ		(256*1024)	/* for program memory */
#define MSIM_AVR_DMSZ		(64*1024)	/* for data memory */
#define MSIM_AVR_PM_PAGESZ	(1024)		/* for PM page */
#define MSIM_AVR_LOGSZ		1024		/* log buffer, in bytes */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
#include "mcusim/pty.h"
#include "mcusim/tsq.h"
#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/io.h"
#include "mcusim/avr/sim/wdt.h"
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
typedef int (*MSIM_AVR_ResetSPM_f)(struct MSIM_AVR *mcu);

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

/* Instance of the AVR microcontroller. */
struct MSIM_AVR {
	char name[20]; /* Name of the MCU. */
	char log[MSIM_AVR_LOGSZ]; /* Buffer to print a log message. */
	uint8_t signature[3]; /* Signature of the MCU. */
	uint8_t xmega; /* Flag of the XMega AVR microcontroller. */
	uint8_t reduced_core; /* Flag of the reduced AVR core. */

	uint32_t flashstart; /* First byte of the program memory. */
	uint32_t flashend; /* Last byte of the program memory. */
	uint32_t ramstart; /* First byte of the on-chip SRAM. */
	uint32_t ramend; /* Last byte of the on-chip SRAM. */
	uint32_t ramsize; /* Size of the on-chip SRAM (in bytes). */
	uint32_t e2start; /* First EEPROM byte. */
	uint32_t e2end; /* Last EEPROM byte. */
	uint32_t e2size; /* EEPROM size (in bytes). */
	uint32_t e2pagesize; /* EEPROM page size (in bytes). */
	uint8_t lockbits; /* Lock bits of the MCU. */
	uint8_t fuse[6]; /* Fuse bytes of the MCU. */

	/* Flash page size (in bytes) for SPM instruction. */
	uint32_t spm_pagesize;
	/* Address of the SPMCSR register in data memory. */
	uint8_t *spmcsr;

	uint32_t freq; /* Current MCU clock frequency (in Hz). */
	pthread_mutex_t freq_mutex; /* Lock before accessing frequency. */
	uint32_t pc; /* Current program counter (in bytes). */
	uint8_t pc_bits; /* Number of PC bits (16-bit, 22-bit, etc.) */
	uint8_t ic_left; /* Clock cycles left to finish current instruction. */
	uint8_t in_mcinst; /* Multi-cycle instruction flag. */

	uint8_t *sreg; /* Address of SREG register in data memory. */
	uint8_t *sph; /* Address of SPH register in data memory. */
	uint8_t *spl; /* Address of SPL register in data memory. */
	uint8_t *eind; /* Address of extended indirect register in DM. */
	uint8_t *rampz; /* Address of extended Z-pointer register in DM. */
	uint8_t *rampy; /* Address of extended Y-pointer register in DM. */
	uint8_t *rampx; /* Address of extended X-pointer register in DM. */
	uint8_t *rampd; /* Address of extended direct register in DM. */

	uint8_t pm[MSIM_AVR_PMSZ]; /* Program memory. */
	uint8_t pmp[MSIM_AVR_PMSZ]; /* Page buffer for program memory. */
	/* Match points memory. It contains actual instructions from
	 * the program memory which were replaced by breakpoints. */
	uint8_t mpm[MSIM_AVR_PMSZ];
	uint32_t pm_size; /* Size of the program memory. */
	/* Flag to read instruction from match points memory. */
	uint8_t read_from_mpm;

	/* Data memory (general purpose registers, I/O registers and SRAM). */
	uint8_t dm[MSIM_AVR_DMSZ];
	uint32_t dm_size; /* Size of the data memory. */
	uint32_t writ_io[4]; /* I/O registers written on a previous step. */
	uint32_t read_io[4]; /* I/O registers read on a previous step. */

	uint32_t sfr_off; /* Offset to I/O registers in data memory. */
	uint32_t regs_num; /* Number of general purpose registers. */
	uint32_t ioregs_num; /* Number of I/O registers. */

	MSIM_AVR_SetFuse_f set_fusef; /* Function to configure AVR fuses .*/
	MSIM_AVR_SetLock_f set_lockf; /* Function to configure AVR lock bits.*/
	MSIM_AVR_TickPerf_f tick_perf; /* Function to tick AVR peripherals. */
	MSIM_AVR_PassIRQs_f pass_irqs; /* Function to provide IRQs. */
	MSIM_AVR_ResetSPM_f reset_spm; /* Function to reset SPM instruction. */

	enum MSIM_AVR_State state; /* State of the MCU. */
	pthread_mutex_t state_mutex; /* Lock before accessing MCU state. */
	enum MSIM_AVR_ClkSource clk_source; /* Current MCU clock source. */

	/* Descriptors of the I/O registers. Each register can be addressed by
	 * the same offset as in the data memory. */
	struct MSIM_AVR_IOReg ioregs[MSIM_AVR_DMSZ];
	struct MSIM_AVR_BLD bls; /* Bootloader section details. */
	struct MSIM_AVR_INT intr; /* Details to work with IRQs. */
	struct MSIM_AVR_WDT wdt; /* Watchdog timer of the MCU. */
	struct MSIM_AVR_VCD vcd; /* Details to work with VCD file. */
	struct MSIM_AVR_USART usart; /* Details to work with USART. */
	struct MSIM_PTY pty; /* Details to work with POSIX pseudo-terminals. */
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
