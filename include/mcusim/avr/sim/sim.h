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

/* Instance of the AVR microcontroller.
 *
 * name			Name of the MCU.
 * log			Buffer to print a log message.
 * signature		Signature of the MCU.
 * xmega		Flag of the XMega AVR microcontroller.
 * reduced_core		Flag of the reduced AVR core.
 *
 * flashstart		First byte of the program memory.
 * flashend		Last byte of the program memory.
 * ramstart		First byte of the on-chip SRAM.
 * ramend		Last byte of the on-chip SRAM.
 * ramsize		Size of the on-chip SRAM (in bytes).
 * e2start		First EEPROM byte.
 * e2end		Last EEPROM byte.
 * e2size		EEPROM size (in bytes).
 * e2pagesize		EEPROM page size (in bytes).
 * lockbits		Lock bits of the MCU.
 * fuse			Fuse bytes of the MCU.
 *
 * spm_pagesize		Flash page size (in bytes) for SPM instruction.
 * spmcsr		Address of the SPMCSR register in data memory.
 *
 * freq			Current MCU clock frequency (in Hz).
 * pc			Current program counter (in bytes).
 * pc_bits		Number of PC bits (16-bit, 22-bit, etc.)
 * ic_left		Clock cycles left to finish current instruction.
 * in_mcinst		Multi-cycle instruction flag.
 *
 * sreg			Address of SREG register in data memory.
 * sph			Address of SPH register in data memory.
 * spl			Address of SPL register in data memory.
 * eind			Address of extended indirect register in DM.
 * rampz		Address of extended Z-pointer register in DM.
 * rampy		Address of extended Y-pointer register in DM.
 * rampx		Address of extended X-pointer register in DM.
 * rampd		Address of extended direct register in DM.
 *
 * pm			Program memory.
 * pmp			Page buffer for program memory.
 * mpm			Match points memory. It contains actual instructions
 * 			from program memory which were replaced by breakpoints.
 * dm			Data memory (general purpose registers, I/O registers
 * 			and SRAM).
 * pm_size		Size of the program memory.
 * dm_size		Size of the data memory.
 * read_from_mpm	Flag to read instruction from match points memory.
 * writ_io		I/O registers written on a previous step.
 * read_io		I/O registers read on a previous step.
 *
 * sfr_off		Offset to I/O registers in data memory.
 * regs_num		Number of general purpose registers.
 * ioregs_num		Number of I/O registers.
 *
 * set_fusef		Function to configure AVR fuse bytes.
 * set_lockf		Function to configure AVR lock bits.
 * tick_perf		Function to tick AVR peripherals.
 * pass_irqs		Function to set IRQs according to the flags.
 * reset_spm		Function to reset SPM instruction.
 *
 * state		State of the MCU
 * clk_source		Current MCU clock source
 *
 * ioregs		Descriptors of the I/O registers. Each register can
 * 			be addressed by the same offset as in the data memory.
 * bls			Bootloader section details.
 * intr			Details to work with MCU interrupts and IRQs.
 * wdt			Watchdog timer of the MCU.
 * vcd			Array of I/O registers to be dumped into VCD file.
 * vcd_queue		Queue to keep VCD dump frames.
 * pty			Details to work with POSIX pseudo-terminals.
 * usart		Details to work with USART.
 */
struct MSIM_AVR {
	char name[20];
	char log[MSIM_AVR_LOGSZ];
	uint8_t signature[3];
	uint8_t xmega;
	uint8_t reduced_core;

	uint32_t flashstart;
	uint32_t flashend;
	uint32_t ramstart;
	uint32_t ramend;
	uint32_t ramsize;
	uint32_t e2start;
	uint32_t e2end;
	uint32_t e2size;
	uint32_t e2pagesize;
	uint8_t lockbits;
	uint8_t fuse[6];

	uint32_t spm_pagesize;
	uint8_t *spmcsr;

	uint32_t freq;
	uint32_t pc;
	uint8_t pc_bits;
	uint8_t ic_left;
	uint8_t in_mcinst;

	uint8_t *sreg;
	uint8_t *sph;
	uint8_t *spl;
	uint8_t *eind;
	uint8_t *rampz;
	uint8_t *rampy;
	uint8_t *rampx;
	uint8_t *rampd;

	uint8_t pm[MSIM_AVR_PMSZ];
	uint8_t pmp[MSIM_AVR_PMSZ];
	uint8_t mpm[MSIM_AVR_PMSZ];
	uint8_t dm[MSIM_AVR_DMSZ];
	uint32_t pm_size;
	uint32_t dm_size;
	uint8_t read_from_mpm;
	uint32_t writ_io[4];
	uint32_t read_io[4];

	uint32_t sfr_off;
	uint32_t regs_num;
	uint32_t ioregs_num;

	MSIM_AVR_SetFuse_f set_fusef;
	MSIM_AVR_SetLock_f set_lockf;
	MSIM_AVR_TickPerf_f tick_perf;
	MSIM_AVR_PassIRQs_f pass_irqs;
	MSIM_AVR_ResetSPM_f reset_spm;

	enum MSIM_AVR_State state;
	enum MSIM_AVR_ClkSource clk_source;

	struct MSIM_AVR_IOReg ioregs[MSIM_AVR_DMSZ];
	struct MSIM_AVR_BLD bls;
	struct MSIM_AVR_INT intr;
	struct MSIM_AVR_WDT wdt;
	struct MSIM_AVR_VCD vcd;
	struct MSIM_AVR_USART usart;
	struct MSIM_PTY pty;
};

#ifdef __cplusplus
}
#endif

#endif /* MSIM_AVR_SIM_H_ */
