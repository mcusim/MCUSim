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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/hex/ihex.h"
#include "mcusim/log.h"
#include "mcusim/config.h"
#include "mcusim/bit/private/macro.h"
#include "mcusim/avr/sim/private/macro.h"
#include "mcusim/avr/sim/private/io_macro.h"

#define FLASH_FILE		".mcusim.flash"
#define EEP_FILE		".mcusim.eeprom"

/* Macros to read and update AVR status register (SREG) */
#define UPDATE_SREG(mcu, flag, set_f) do {				\
	if ((set_f) == 0) {						\
		*mcu->sreg &= (uint8_t)~(1<<(flag));			\
	} else {							\
		*mcu->sreg |= (uint8_t)(1<<(flag));			\
	}								\
} while (0)
#define READ_SREG(mcu, flag) ((uint8_t)((*mcu->sreg>>(flag))&1))

#define IS_MCU_ACTIVE(mcu) 	(((mcu)->state == AVR_RUNNING) ||	\
                                 ((mcu)->state == AVR_MSIM_STEP))

typedef int (*init_func)(MSIM_AVR *mcu, MSIM_InitArgs *args);

/* Function to process interrupt request according to the order */
static int	pass_irqs(struct MSIM_AVR *);
static int	handle_irq(struct MSIM_AVR *);

/* Function to setup AVR instance. */
static int	set_fuse(MSIM_AVR *, uint32_t, uint8_t);
static int	set_lock(MSIM_AVR *, uint8_t);
static void	print_config(MSIM_AVR *);
static int	load_mem8(MSIM_AVR *, const char *, uint8_t *, const char *);
static int	load_mem16(MSIM_AVR *, const char *, uint16_t *, const char *);
static int	setup_avr(MSIM_AVR *, const char *,
                          uint8_t *, uint32_t, uint8_t *, uint32_t,
                          uint8_t *, const char *);

/* Init function per AVR chip */
struct init_func_info {
	char partno[20];
	char name[20];
	init_func f;
};

/* Init functions for the supported AVR MCUs. */
static struct init_func_info init_funcs[] = {
	{ "m8",		"ATmega8",	MSIM_M8AInit },
	{ "m8a",	"ATmega8A",	MSIM_M8AInit },
	{ "m328",	"ATmega328",	MSIM_M328Init },
	{ "m328p",	"ATmega328P",	MSIM_M328PInit },
//	{ "m2560",	"ATmega2560",	MSIM_M2560Init }
};

/*
 * Starts the main simualtion loop for the AVR microcontroller.
 *
 * Simulator can be started in firmware test mode, i.e. no debuggers or
 * any external events are necessary to perform a simulation.
 */
int
MSIM_AVR_Simulate(struct MSIM_AVR *mcu, uint8_t ft)
{
	const struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	int rc = 0;

	if (vcd->regs[0].i >= 0) {
		/* Open VCD file if there are registers to dump. */
		rc = MSIM_AVR_VCDOpen(mcu);
		if (rc != 0) {
			snprintf(LOG, LOGSZ, "can't open VCD file: '%s'",
			         vcd->dump_file);
			MSIM_LOG_FATAL(LOG);

			return -1;
		}
	}
	if (ft) {
		mcu->state = AVR_RUNNING;
	}

	/* Main simulation loop. */
	while (1) {
		rc = MSIM_AVR_SimStep(mcu, ft);
		if (rc != 0) {
			rc = (rc == 2) ? 0 : rc;
			break;
		}
	}

	/* We may need to close a previously initialized VCD dump. */
	MSIM_AVR_VCDClose(mcu);

	return rc;
}

/* Performs a single simulation cycle. */
int
MSIM_AVR_SimStep(MSIM_AVR *mcu, uint8_t ft)
{
	uint64_t *tick = &mcu->tick;
	uint8_t *tovf = &mcu->tovf;
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	struct MSIM_AVRConf cnf;
	int rc = 0;

	do {
		/*
		 * The main simulation loop can be terminated by setting
		 * the MCU state to AVR_MSIM_STOP. It's likely to be done by
		 * a command from a remote GDB client.
		 */
		if ((mcu->ic_left == 0) && (mcu->state == AVR_MSIM_STOP)) {
			if (MSIM_LOG_ISDEBUG) {
				snprintf(LOG, LOGSZ, "simulation terminated "
				         "(stopped mcu), pc=0x%06"
				         PRIx32, mcu->pc);
				MSIM_LOG_DEBUG(LOG);
			}

			rc = 2;
			break;
		}
		if ((mcu->ic_left == 0) && (mcu->state == AVR_MSIM_TESTFAIL)) {
			MSIM_LOG_WARN("simulation terminated (failed test)");
			rc = 1;
			break;
		}

		/* Wait for request from GDB in MCU stopped mode */
		if (!ft && !mcu->ic_left &&
		                (mcu->state == AVR_STOPPED) &&
		                MSIM_AVR_RSPHandle(mcu)) {
			snprintf(LOG, LOGSZ, "handling message from GDB RSP "
			         "client failed: pc=0x%06" PRIx32, mcu->pc);
			MSIM_LOG_FATAL(LOG);

			rc = 1;
			break;
		}

		/* Update timers */
		if (IS_MCU_ACTIVE(mcu)) {
			MSIM_AVR_TMRUpdate(mcu);
		}

		/*
		 * Tick MCU periferals.
		 *
		 * NOTE: It is important to tick MCU peripherals before
		 * updating Lua models. One of the reasons is an accessing
		 * mechanism of the registers which share the same I/O
		 * location (UBRRH/UCSRC of ATmega8A for example).
		 */
		if ((mcu->tick_perf != NULL) && IS_MCU_ACTIVE(mcu)) {
			mcu->tick_perf(mcu, &cnf);
		}

		/* Tick peripherals written in Lua */
		if (IS_MCU_ACTIVE(mcu)) {
			MSIM_AVR_LUATickModels(mcu);
		}

		/* Dump registers to VCD */
		if (vcd->dump && !(*tovf) && IS_MCU_ACTIVE(mcu)) {
			MSIM_AVR_VCDDumpFrame(mcu, *tick);
		}

		/* Test scope of a program counter */
		if (mcu->pc > (mcu->flashend>>1)) {
			snprintf(LOG, LOGSZ, "program counter is out of flash "
			         "memory: pc=0x%06" PRIx32 ", flashend=0x%06"
			         PRIx32, mcu->pc, mcu->flashend >> 1);
			MSIM_LOG_FATAL(LOG);

			rc = 1;
			break;
		}
		if ((mcu->pc + 2) >= mcu->pm_size) {
			snprintf(LOG, LOGSZ, "program counter is out of scope: "
			         "pc=0x%06" PRIx32 ", pm_size=0x%06" PRIx32,
			         mcu->pc, mcu->pm_size);
			MSIM_LOG_FATAL(LOG);

			rc = 1;
			break;
		}

		/*
		 * Decode next instruction.
		 *
		 * It's usually hard to say in which state the MCU registers
		 * will be between neighbor cycles of a multi-cycle instruction.
		 * This discussion may be taken into account:
		 *
		 * https://electronics.stackexchange.com/questions/132171/
		 * 	what-happens-to-avr-registers-during-multi-
		 * 	cycle-instructions
		 *
		 * But this change of LSB and MSB mentioned above can be
		 * MCU-specific and not a general way of how it really works.
		 * Detailed information can be obtained directly from Atmel,
		 * but there is no intention to do this in order not to
		 * unveil their secrets. However, any details they're ready
		 * to share are highly welcome.
		 *
		 * Simulator doesn't guarantee anything special here either.
		 * The only thing you may rely on is the instruction which
		 * will be completed _after all_ of these cycles required to
		 * finish it.
		 */
		if ((mcu->ic_left || IS_MCU_ACTIVE(mcu)) && MSIM_AVR_Step(mcu)) {
			snprintf(mcu->log, sizeof mcu->log, "decoding "
			         "instruction failed: pc=0x%06" PRIx32,
			         mcu->pc);
			MSIM_LOG_FATAL(mcu->log);

			rc = 1;
			break;
		}

		if (mcu->ic_left || IS_MCU_ACTIVE(mcu)) {
			MSIM_AVR_IOSyncPinx(mcu);
		}

		/*
		 * Provide and handle IRQs.
		 *
		 * It's important to understand an interrupt may occur during
		 * execution of a multi-cycle instruction. This instruction
		 * is completed before the interrupt is served (according to
		 * the multiple AVR datasheets).
		 *
		 * It means that we may provide IRQs, but will have to wait
		 * required number of cycles to serve them.
		 */
		pass_irqs(mcu);
		if (READ_SREG(mcu, SR_GLOBINT) && (!mcu->ic_left) &&
		                (!mcu->intr.exec_main) && IS_MCU_ACTIVE(mcu)) {
			handle_irq(mcu);
		}

		/*
		 * All cycles of a single instruction from a main program
		 * have to be performed.
		 */
		if (mcu->ic_left == 0) {
			mcu->intr.exec_main = 0;
		}

		/*
		 * Increment cycles or print a warning message in case of
		 * the maximum amount of cycles reached (extremely unlikely
		 * if a compiler supports 'uint64_t').
		 */
		if (IS_MCU_ACTIVE(mcu)) {
			if ((*tick) < TICKS_MAX) {
				(*tick)++;
			} else {
				*tovf = 1;
				MSIM_LOG_WARN("maximum cycles logged!");
			}
		}

		/* Halt MCU after a single step performed */
		if (!mcu->ic_left && mcu->state == AVR_MSIM_STEP) {
			mcu->state = AVR_STOPPED;
		}
	} while (0);

	return rc;
}

/*
 * Initializes AVR MCU into a specific model according to the given
 * configuration file.
 */
int
MSIM_AVR_Init(MSIM_AVR *mcu, MSIM_CFG *conf)
{
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	const uint32_t dflen = ARRSZ(vcd->dump_file);
	uint32_t dump_regs;
	uint8_t stop_reading = 0;
	FILE *fp = NULL;
	char *frm_file = NULL;
	int rc = 0;

	do {
		/* Try to open a firmware file */
		if (conf->reset_flash == 1U) {
			/* Firmware file has the priority. */
			if (conf->has_firmware_file != 1) {
				MSIM_LOG_FATAL("missing firmware in config");
				rc = 1;
				break;
			}

			fp = fopen(conf->firmware_file, "r");
			if (fp == NULL) {
				snprintf(LOG, LOGSZ, "can't read firmware: %s",
				         conf->firmware_file);
				MSIM_LOG_FATAL(LOG);

				rc = 1;
				break;
			} else {
				snprintf(LOG, LOGSZ, "firmware: %s",
				         conf->firmware_file);
				MSIM_LOG_INFO(LOG);

				frm_file = conf->firmware_file;
				fclose(fp);
			}
		} else {
			/* Utility file has the priority. */
			fp = fopen(FLASH_FILE, "r");

			if (fp != NULL) {
				MSIM_LOG_INFO("using firmware: " FLASH_FILE);

				frm_file = FLASH_FILE;
				fclose(fp);

				/* Stop reading firmware files further */
				stop_reading = 1;
			} else {
				MSIM_LOG_WARN("failed to read: " FLASH_FILE);
			}

			if (!stop_reading && (conf->has_firmware_file == 1)) {
				fp = fopen(conf->firmware_file, "r");

				if (fp == NULL) {
					snprintf(LOG, LOGSZ, "can't read: %s",
					         conf->firmware_file);
					MSIM_LOG_FATAL(LOG);

					rc = 1;
					break;
				} else {
					snprintf(LOG, LOGSZ, "using firmware: %s",
					         conf->firmware_file);
					MSIM_LOG_INFO(LOG);

					frm_file = conf->firmware_file;
					fclose(fp);
				}
			}

			if (!stop_reading && (conf->has_firmware_file != 1)) {
				MSIM_LOG_FATAL("missing firmware in config");
				rc = 1;
				break;
			}
		}

		/* Initialize MCU as one of the AVR models */
		mcu->intr.trap_at_isr = conf->trap_at_isr;
		if (setup_avr(mcu, conf->mcu, NULL,
		                MSIM_AVR_PMSZ, NULL,
		                MSIM_AVR_DMSZ, NULL, frm_file) != 0) {
			snprintf(LOG, LOGSZ, "%s can't be initialized",
			         conf->mcu);
			MSIM_LOG_FATAL(LOG);

			rc = 1;
			break;
		}

		/* Select registers to be dumped */
		dump_regs = 0;
		strncpy(vcd->dump_file, conf->vcd_file, dflen - 1);

		for (uint32_t i = 0; i < conf->dump_regs_num; i++) {
			for (uint32_t j = 0; j < MSIM_AVR_DMSZ; j++) {
				char *bit, *pos;
				size_t len;
				int bitn, cr, bit_cr;

				if (mcu->ioregs[j].off < 0) {
					continue;
				}
				len = strlen(mcu->ioregs[j].name);
				if (len == 0) {
					continue;
				}

				pos = strstr(mcu->ioregs[j].name,
				             conf->dump_regs[i]);
				cr = strncmp(mcu->ioregs[j].name,
				             conf->dump_regs[i], len);

				/* Do we have a 16-bit register mentioned or
				 * an exact match of the register names? */
				if ((cr != 0) && (pos != NULL)) {
					if (mcu->ioregs[j].name[len-1] == 'H') {
						vcd->regs[dump_regs].i = (int32_t)j;
					}
					if (mcu->ioregs[j].name[len-1] == 'L') {
						vcd->regs[dump_regs].reg_lowi =
						        (int32_t)j;
					}

					if ((vcd->regs[dump_regs].i >= 0) &&
					                (vcd->regs[dump_regs].
					                 reg_lowi >= 0)) {
						vcd->regs[dump_regs].n = -1;
						strncpy(vcd->regs[dump_regs].name,
						        mcu->ioregs[j].name, sizeof
						        vcd->regs[dump_regs].name);
						vcd->regs[dump_regs].name[len-1] = 0;

						dump_regs++;
						break;
					}
				} else if (cr != 0) {
					continue;
				} else {
					/* Do we have a bit index suffix? */
					bit = len < sizeof conf->dump_regs[0]/
					      sizeof conf->dump_regs[0][0]
					      ? &conf->dump_regs[i][len] : NULL;
					bit_cr = sscanf(bit, "%d", &bitn);
					if (bit_cr != 1) {
						bitn = -1;
					}

					/* Set index of a register to be dumped */
					vcd->regs[dump_regs].i = (int32_t)j;
					vcd->regs[dump_regs].n = (int8_t)bitn;
					strncpy(vcd->regs[dump_regs].name,
					        mcu->ioregs[j].name,
					        sizeof vcd->regs[dump_regs].name);

					dump_regs++;
					break;
				}
			}
		}

		/* Apply memory modifications */
		if (conf->has_lockbits == 1) {
			set_lock(mcu, conf->mcu_lockbits);
		}
		if (conf->has_efuse == 1) {
			set_fuse(mcu, FUSE_EXT, conf->mcu_efuse);
		}
		if (conf->has_hfuse == 1) {
			set_fuse(mcu, FUSE_HIGH, conf->mcu_hfuse);
		}
		if (conf->has_lfuse == 1) {
			set_fuse(mcu, FUSE_LOW, conf->mcu_lfuse);
		}

		/* Try to set required frequency */
		if (conf->mcu_freq > mcu->freq) {
			snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%"
			         PRIu64 " kHz is above maximum %lu.%lu kHz",
			         conf->mcu_freq/1000U, conf->mcu_freq%1000U,
			         mcu->freq/1000UL, mcu->freq%1000UL);
			MSIM_LOG_WARN(LOG);
		} else if (conf->mcu_freq > 0U) {
			mcu->freq = (uint32_t)conf->mcu_freq;
		} else {
			snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%"
			         PRIu64 " kHz cannot be selected as clock "
			         "source",
			         conf->mcu_freq/1000U, conf->mcu_freq%1000U);
			MSIM_LOG_WARN(LOG);
		}

		/* Print MCU configuration */
		print_config(mcu);

		/* Load Lua peripherals if it is required */
		for (uint32_t k = 0; k < conf->lua_models_num; k++) {
			char *lua_model = &conf->lua_models[k][0];
			if (MSIM_AVR_LUALoadModel(mcu, lua_model)) {
				MSIM_LOG_FATAL("loading Lua model failed");
			}
		}

		/* Do we have registers to dump? */
		if (vcd->regs[0].i >= 0) {
			rc = MSIM_AVR_VCDOpen(mcu);
			if (rc != 0) {
				snprintf(LOG, LOGSZ, "failed to open VCD: %s",
				         vcd->dump_file);
				MSIM_LOG_FATAL(LOG);
				rc = 1;
				break;
			}
		}

		/* Force MCU to run in a firmware-test mode. */
		if (conf->firmware_test == 1U) {
			MSIM_LOG_DEBUG("running in \"firmware test\" mode");
			mcu->state = AVR_RUNNING;
		}
	} while (0);

	return rc;
}

static int
setup_avr(struct MSIM_AVR *mcu, const char *mcu_name,
          uint8_t *pm, uint32_t pm_size,
          uint8_t *dm, uint32_t dm_size,
          uint8_t *mpm, const char *progfile)
{
	unsigned int i;
	char mcu_found = 0;
	char log[1024];
	struct MSIM_InitArgs args;

	args.pm = pm;
	args.dm = dm;
	args.pmsz = pm_size;
	args.dmsz = dm_size;

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++) {
		if (!strcmp(init_funcs[i].partno, mcu_name)) {
			if (init_funcs[i].f(mcu, &args)) {
				return -1;
			} else {
				mcu_found = 1;
				break;
			}
		}
	}

	if (!mcu_found) {
		snprintf(log, sizeof log, "MCU not supported: %s", mcu_name);
		MSIM_LOG_FATAL(log);
		return -1;
	}

	if (MSIM_AVR_LoadProgMem(mcu, progfile)) {
		MSIM_LOG_FATAL("program memory can't be loaded from a file");
		return -1;
	}
	mcu->state = AVR_STOPPED;
	mcu->read_from_mpm = 0;

	return 0;
}

static int
handle_irq(struct MSIM_AVR *mcu)
{
	unsigned int i;
	int ret;

	/* Let's try to find IRQ with the highest priority:
	 * i == 0		highest priority
	 * i == AVR_IRQ_NUM-1	lowest priority */
	for (i = 0; i < MSIM_AVR_IRQNUM; i++) {
		if (mcu->intr.irq[i] > 0) {
			break;
		}
	}

	ret = 0;
	if (i != MSIM_AVR_IRQNUM) {
		/* Clear selected IRQ */
		mcu->intr.irq[i] = 0;

		/* Disable interrupts globally.
		 * It is not applicable for the AVR XMEGA cores. */
		if (!mcu->xmega) {
			UPDATE_SREG(mcu, SR_GLOBINT, 0);
		}

		/* Push PC onto the stack */
		MSIM_AVR_StackPush(mcu, (uint8_t)(mcu->pc & 0xFF));
		MSIM_AVR_StackPush(mcu, (uint8_t)((mcu->pc >> 8)&0xFF));
		if (mcu->pc_bits > 16) {
			MSIM_AVR_StackPush(mcu, (uint8_t)((mcu->pc>>16)&0xFF));
		}

		/* Load interrupt vector to PC */
		mcu->pc = mcu->intr.ivt * i;

		/* Switch MCU to step mode if it's necessary */
		if (mcu->intr.trap_at_isr && mcu->state == AVR_RUNNING) {
			mcu->state = AVR_MSIM_STEP;
		}
	} else {
		/* No IRQ, do nothing */
		ret = 2;
	}
	return ret;
}

static int
pass_irqs(struct MSIM_AVR *mcu)
{
	struct MSIM_AVR_TMR *tmr;
	struct MSIM_AVR_TMR_COMP *comp;
	uint32_t en, rai;
	int rc = 0;

	/* Pass interrupts of the timers */
	for (uint32_t i = 0; i < MSIM_AVR_MAXTMRS; i++) {
		tmr = &mcu->timers[i];
		if (IS_IONOBITA(tmr->tcnt)) {
			break;
		}

		/* Timer's owm interrupts */
		struct MSIM_AVR_INTVec *vec[] = { &tmr->iv_ovf, &tmr->iv_ic };
		for (uint32_t k = 0; k < ARRSZ(vec); k++) {
			if (IS_NOINTV(vec[k])) {
				break;
			}

			en = IOBIT_RD(mcu, &vec[k]->enable);
			rai = IOBIT_RD(mcu, &vec[k]->raised);
			if ((en == 1U) && (rai == 1U)) {
				mcu->intr.irq[vec[k]->vector] = 1;
				IOBIT_WR(mcu, &vec[k]->raised, 0);
			}
		}

		/* Interrupts of the output compare channels */
		for (uint32_t k = 0; k < ARRSZ(tmr->comp); k++) {
			comp = &tmr->comp[k];
			if (IS_NOCOMP(comp)) {
				break;
			}
			if (IS_NOINTV(&comp->iv)) {
				break;
			}

			en = IOBIT_RD(mcu, &comp->iv.enable);
			rai = IOBIT_RD(mcu, &comp->iv.raised);
			if ((en == 1U) && (rai == 1U)) {
				mcu->intr.irq[comp->iv.vector] = 1;
				IOBIT_WR(mcu, &comp->iv.raised, 0);
			}
		}
	}

	return rc;
}

static int
set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val)
{
	struct MSIM_AVRConf cnf;
	int rc = 0;

	if (m->set_fusef == NULL) {
		MSIM_LOG_WARN("can't write fuse");
	} else {
		cnf.fuse_n = fuse;
		cnf.fuse_v = val;
		m->set_fusef(m, &cnf);
	}

	return rc;
}

static int
set_lock(struct MSIM_AVR *m, uint8_t val)
{
	struct MSIM_AVRConf cnf;
	int rc = 0;

	if (m->set_lockf == NULL) {
		MSIM_LOG_WARN("can't write lock bits");
	} else {
		cnf.lock_v = val;
		m->set_lockf(m, &cnf);
	}

	return rc;
}

static void
print_config(struct MSIM_AVR *m)
{
	/* AVR memory is organized as array of bytes in the simulator, but
	 * it's natural to measure program memory in 16-bits words because
	 * all AVR instructions are 16- or 32-bits wide. This is why all
	 * program memory addresses are divided by two before printing. */
	uint64_t reset_pc = m->intr.reset_pc>>1;
	uint64_t ivt = m->intr.ivt>>1;
	uint64_t flashstart = m->flashstart>>1;
	uint64_t flashend = m->flashend>>1;
	uint64_t blsstart = m->bls.start>>1;
	uint64_t blsend = m->bls.end>>1;

	snprintf(m->log, LOGSZ, "model: %s (%02X%02X%02X)",
	         m->name, m->signature[0], m->signature[1], m->signature[2]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "clock: %" PRIu32 ".%" PRIu32 " kHz",
	         m->freq/1000, m->freq%1000);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "fuses: EXT=0x%02X, HIGH=0x%02X, LOW=0x%02X",
	         m->fuse[2], m->fuse[1], m->fuse[0]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "lock: 0x%02X", m->lockbits);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "reset vector: 0x%06" PRIX64, reset_pc);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "interrupt vectors: 0x%06" PRIX64, ivt);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "flash: 0x%06" PRIX64 "-0x%06" PRIX64,
	         flashstart, flashend);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "bootloader: 0x%06" PRIX64 "-0x%06"
	         PRIX64, blsstart, blsend);
	MSIM_LOG_INFO(m->log);
}

/* Pushes a value to the head of MCU stack. */
void
MSIM_AVR_StackPush(MSIM_AVR *mcu, uint8_t val)
{
	uint32_t sp;

	sp = (uint32_t)((*mcu->spl) | (*mcu->sph<<8));
	mcu->dm[sp--] = val;
	*mcu->spl = (uint8_t)(sp & 0xFF);
	*mcu->sph = (uint8_t)(sp >> 8);
}

/* Borrows a value from the MCU stack head. */
uint8_t
MSIM_AVR_StackPop(MSIM_AVR *mcu)
{
	uint32_t sp;
	uint8_t v;

	sp = (uint32_t)((*mcu->spl) | (*mcu->sph<<8));
	v = mcu->dm[++sp];
	*mcu->spl = (uint8_t)(sp & 0xFF);
	*mcu->sph = (uint8_t)(sp >> 8);

	return v;
}

/* Prints supported AVR parts. */
void
MSIM_AVR_PrintParts(void)
{
	for (uint32_t i = 0; i < ARRSZ(init_funcs); i++) {
		printf("%s %s\n", init_funcs[i].partno, init_funcs[i].name);
	}
}

/* Populates AVR program memory from the Intel HEX file. */
int
MSIM_AVR_LoadProgMem(MSIM_AVR *mcu, const char *f)
{
	return load_mem16(mcu, f, mcu->pm, "progmem");
}

/* Populates AVR data memory from the Intel HEX file. */
int
MSIM_AVR_LoadDataMem(MSIM_AVR *mcu, const char *f)
{
	return load_mem8(mcu, f, mcu->dm, "datamem");
}

static int
load_mem8(MSIM_AVR *mcu, const char *f, uint8_t *mem, const char *memtype)
{
	FILE *fp = NULL;
	IHexRecord r, mr;
	uint8_t *addr = 0;

	fp = fopen(f, "r");
	if (fp == NULL) {
		snprintf(LOG, LOGSZ, "can't load %s from: '%s'", memtype, f);
		MSIM_LOG_ERROR(LOG);

		return 1;
	}

	/* Copy hex data to the memory */
	while (MSIM_IHEX_ReadRec(&r, fp) == IHEX_OK) {
		switch (r.type) {
		case IHEX_TYPE_00:
			/* Data record */
			addr = mem + r.address;
			memcpy(addr, r.data, (uint16_t)r.dataLen);
			break;
		case IHEX_TYPE_01:
			/* End of File record */
			break;
		default:		/* Other types, unlikely occured */
			continue;
		}
	}

	rewind(fp);
	addr = 0;

	/* Verify checksum of the loaded data */
	while (MSIM_IHEX_ReadRec(&r, fp) == IHEX_OK) {
		/* Stop reading if end-of-file reached */
		if (r.type == IHEX_TYPE_01) {
			break;
		}

		/* Skip all other record types */
		if (r.type != IHEX_TYPE_00) {
			continue;
		}

		addr = mem + r.address;
		memcpy(mr.data, addr, (uint16_t)r.dataLen);
		mr.address = r.address;
		mr.dataLen = r.dataLen;
		mr.type = r.type;
		mr.checksum = 0;

		mr.checksum = MSIM_IHEX_CalcChecksum(&mr);
		if (mr.checksum != r.checksum) {
			snprintf(LOG, LOGSZ, "incorrt IHEX checksum: "
			         "0x%X (mem) != 0x%X (file)",
			         mr.checksum, r.checksum);
			MSIM_LOG_FATAL(LOG);

			MSIM_LOG_FATAL("file record:");
			MSIM_IHEX_PrintRec(&r);
			MSIM_LOG_FATAL("memory record:");
			MSIM_IHEX_PrintRec(&mr);

			return -1;
		}
	}

	fclose(fp);
	return 0;
}

static int
load_mem16(MSIM_AVR *mcu, const char *f, uint16_t *mem, const char *memtype)
{
	FILE *fp = NULL;
	IHexRecord r, mr;
	uint16_t *addr = 0;

	fp = fopen(f, "r");
	if (fp == NULL) {
		snprintf(LOG, LOGSZ, "can't load %s from: '%s'", memtype, f);
		MSIM_LOG_ERROR(LOG);

		return 1;
	}

	/* Copy hex data to the memory */
	while (MSIM_IHEX_ReadRec(&r, fp) == IHEX_OK) {
		switch (r.type) {
		case IHEX_TYPE_00:
			/* Data record */
			addr = mem + (r.address >> 1);
			for (uint32_t j = 0; j < r.dataLen; j += 2) {
				addr[j >> 1] = (uint16_t) (
				                       ((r.data[j+1] << 8) &0xFF00) |
				                       (r.data[j] &0x00FF));
			}

			break;
		case IHEX_TYPE_01:
			/* End of File record */
			break;
		default:		/* Other types, unlikely occured */
			continue;
		}
	}

	rewind(fp);
	addr = 0;

	/* Verify checksum of the loaded data */
	while (MSIM_IHEX_ReadRec(&r, fp) == IHEX_OK) {
		/* Stop reading if end-of-file reached */
		if (r.type == IHEX_TYPE_01) {
			break;
		}

		/* Skip all other record types */
		if (r.type != IHEX_TYPE_00) {
			continue;
		}

		addr = mem + (r.address >> 1);
		for (uint32_t j = 0; j < r.dataLen; j += 2) {
			mr.data[j] = addr[j >> 1] &0xFF;
			mr.data[j + 1] = (addr[j >> 1] >> 8) &0xFF;
		}

		mr.address = r.address;
		mr.dataLen = r.dataLen;
		mr.type = r.type;
		mr.checksum = 0;

		mr.checksum = MSIM_IHEX_CalcChecksum(&mr);
		if (mr.checksum != r.checksum) {
			snprintf(LOG, LOGSZ, "incorrect IHEX checksum: "
			         "0x%X (mem) != 0x%X (file)",
			         mr.checksum, r.checksum);
			MSIM_LOG_FATAL(LOG);

			MSIM_LOG_FATAL("file record:");
			MSIM_IHEX_PrintRec(&r);
			MSIM_LOG_FATAL("memory record:");
			MSIM_IHEX_PrintRec(&mr);

			return -1;
		}
	}

	fclose(fp);
	return 0;
}

/*
 * This function dumps a content of AVR flash memory to the 'dump' file.
 *
 * It can be loaded back instead of the regular AVR firmware specified by the
 * configuration file.
 */
int
MSIM_AVR_SaveProgMem(MSIM_AVR *mcu, const char *f)
{
	const uint32_t data_t = IHEX_TYPE_00;
	const uint32_t eof_t = IHEX_TYPE_01;
	const uint32_t pmsz = (mcu->flashend - mcu->flashstart + 1) >> 1;
	FILE *out = NULL;
	IHexRecord rec;
	uint8_t chunk[16];
	uint8_t eof_byte = 0;
	uint32_t off = 0;
	int rc = 0;

	/* Open dump file to write to */
	out = fopen(f, "w");

	do {
		if (out == NULL) {
			snprintf(LOG, LOGSZ, "failed to open %s to dump flash "
			         "memory to", f);
			MSIM_LOG_ERROR(LOG);

			rc = 1;
			break;
		}

		/* Write program memory to the file in 16 bytes chuncks */
		for (uint32_t i = 0; i < pmsz; i += 8) {
			/* Populate 16 bytes chunk */
			for (uint32_t j = 0; j < 8; j++) {
				chunk[(j << 1)] =
				        (uint8_t)(PM(i + j) & 0xFF);
				chunk[(j << 1) + 1] =
				        (uint8_t)((PM(i + j) >> 8) & 0xFF);
			}

			MSIM_IHEX_NewRec(data_t, off, chunk, 16, &rec);
			MSIM_IHEX_WriteRec(&rec, out);
			off += 16;
		}

		/* Write 'end-of-file' record */
		MSIM_IHEX_NewRec(eof_t, 0, &eof_byte, 0, &rec);
		MSIM_IHEX_WriteRec(&rec, out);

		/* Close dump file */
		fclose(out);
	} while (0);

	return rc;
}
