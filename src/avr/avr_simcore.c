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

/* Configuration file (default name). */
#define CFG_FILE		"mcusim.conf"
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

typedef int (*init_func)(struct MSIM_AVR *mcu, struct MSIM_InitArgs *args);

/* Function to process interrupt request according to the order */
static int pass_irqs(struct MSIM_AVR *mcu);
static int handle_irq(struct MSIM_AVR *mcu);
/* Function to setup AVR instance. */
static int setup_avr(struct MSIM_AVR *mcu, const char *mcu_name,
                     uint8_t *pm, uint32_t pm_size,
                     uint8_t *dm, uint32_t dm_size,
                     uint8_t *mpm, FILE *fp);
static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val);
static int set_lock(struct MSIM_AVR *m, uint8_t val);
static void print_config(struct MSIM_AVR *m);

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

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp);

int MSIM_AVR_Simulate(struct MSIM_AVR *mcu, uint8_t frm_test)
{
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	uint64_t tick = 0;	/* # of cycles sinse start */
	uint8_t tick_ovf = 0;	/* cycles overflow flag */
	int rc = 0;

	if (vcd->regs[0].i >= 0) {
		/* Open a VCD file if there are registers to dump. */
		rc = MSIM_AVR_VCDOpen(mcu);
		if (rc != 0) {
			snprintf(LOG, LOGSZ, "failed to open a VCD file %s",
			         vcd->dump_file);
			MSIM_LOG_FATAL(LOG);
			return -1;
		}
	}
	if (frm_test) {
		mcu->state = AVR_RUNNING;
	}

	/* Main simulation loop. */
	while (1) {
		rc = MSIM_AVR_SimStep(mcu, &tick, &tick_ovf, frm_test);
		if (rc != 0) {
			rc = rc == 2 ? 0 : rc;
			break;
		}
	}

	/* We may need to close a previously initialized VCD dump. */
	MSIM_AVR_VCDClose(mcu);

	return rc;
}

int MSIM_AVR_SimStep(struct MSIM_AVR *mcu, uint64_t *tick, uint8_t *tick_ovf,
                     uint8_t frm_test)
{
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	struct MSIM_AVRConf cnf;
	int rc = 0;

	do {
		/* The main simulation loop can be terminated by setting
		 * MCU state to AVR_MSIM_STOP. The primary (and maybe only)
		 * source of this state setting is a command from debugger. */
		if ((mcu->ic_left == 0) && (mcu->state == AVR_MSIM_STOP)) {
			if (MSIM_LOG_ISDEBUG) {
				snprintf(mcu->log, sizeof mcu->log,
				         "simulation terminated (stopped mcu),"
				         " pc=0x%06" PRIX32, mcu->pc);
				MSIM_LOG_DEBUG(mcu->log);
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
		if (!frm_test && !mcu->ic_left &&
		                (mcu->state == AVR_STOPPED) &&
		                MSIM_AVR_RSPHandle()) {
			snprintf(mcu->log, sizeof mcu->log, "handling message "
			         "from GDB RSP client failed: pc=0x%06" PRIX32,
			         mcu->pc);
			MSIM_LOG_FATAL(mcu->log);
			rc = 1;
			break;
		}

		/* Update timers */
		MSIM_AVR_TMRUpdate(mcu);

		/* Tick MCU periferals.
		 *
		 * NOTE: It is important to tick MCU peripherals before
		 * updating Lua models. One of the reasons is an accessing
		 * mechanism of the registers which share the same I/O
		 * location (UBRRH/UCSRC of ATmega8A for example). */
		if (mcu->tick_perf != NULL) {
			mcu->tick_perf(mcu, &cnf);
		}

		/* Tick peripherals written in Lua */
		MSIM_AVR_LUATickModels(mcu);

		/* Dump registers to VCD */
		if (vcd->dump && !(*tick_ovf)) {
			MSIM_AVR_VCDDumpFrame(mcu, *tick);
		}

		/* Test scope of a program counter within flash size */
		if ((mcu->pc>>1) > (mcu->flashend>>1)) {
			snprintf(mcu->log, sizeof mcu->log, "program counter "
			         "is out of flash memory: pc=0x%06" PRIX32
			         ", flashend=0x%06" PRIX32,
			         mcu->pc>>1, mcu->flashend>>1);
			MSIM_LOG_FATAL(mcu->log);
			rc = 1;
			break;
		}
		/* Test scope of a program counter */
		if ((mcu->pc+2) >= mcu->pm_size) {
			snprintf(mcu->log, sizeof mcu->log, "program counter "
			         "is out of scope: pc=0x%06" PRIX32 ", pm_size"
			         "=0x%06" PRIX32, mcu->pc, mcu->pm_size);
			MSIM_LOG_FATAL(mcu->log);
			rc = 1;
			break;
		}

		/* Decode next instruction. It's usually hard to say in
		 * which state the MCU registers will be between neighbor
		 * cycles of a multi-cycle instruction. This talk may be
		 * taken into account:
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
		 * finish it. */
		if ((mcu->ic_left || mcu->state == AVR_RUNNING ||
		                mcu->state == AVR_MSIM_STEP) &&
		                MSIM_AVR_Step(mcu)) {
			snprintf(mcu->log, sizeof mcu->log, "decoding "
			         "instruction failed: pc=0x%06" PRIX32,
			         mcu->pc);
			MSIM_LOG_FATAL(mcu->log);
			rc = 1;
			break;
		}

		/* Provide and handle IRQs.
		 *
		 * It's important to understand an interrupt may occur during
		 * execution of a multi-cycle instruction. This instruction
		 * is completed before the interrupt is served (according to
		 * the multiple AVR datasheets).
		 *
		 * It means that we may provide IRQs, but will have to wait
		 * required number of cycles to serve them. */
		pass_irqs(mcu);
		if (READ_SREG(mcu, SR_GLOBINT) &&
		                (!mcu->ic_left) && (!mcu->intr.exec_main) &&
		                (mcu->state == AVR_RUNNING ||
		                 mcu->state == AVR_MSIM_STEP)) {
			handle_irq(mcu);
		}

		/* Halt MCU after a single step performed */
		if (!mcu->ic_left && mcu->state == AVR_MSIM_STEP) {
			mcu->state = AVR_STOPPED;
		}

		/* All cycles of a single instruction from a main program
		 * have to be performed. */
		if (mcu->ic_left == 0) {
			mcu->intr.exec_main = 0;
		}

		/* Increment ticks or print a warning message in case of
		 * maximum amount of ticks reached (extremely unlikely
		 * if compiler supports "unsigned long long" type). */
		if ((*tick) == TICKS_MAX) {
			*tick_ovf = 1;
			MSIM_LOG_WARN("maximum amount of ticks reached");
			if (vcd->dump != NULL) {
				MSIM_LOG_WARN("recording VCD stopped");
			}
		} else {
			(*tick)++;
		}
	} while (0);

	return rc;
}

int MSIM_AVR_Init(struct MSIM_AVR *mcu, struct MSIM_CFG *conf,
                  const char *conf_file)
{
	FILE *fp;
	struct MSIM_AVR_VCD *vcd = &mcu->vcd;
	const uint32_t dflen = sizeof vcd->dump_file/sizeof vcd->dump_file[0];
	uint32_t dump_regs;
	int rc = 0;

	/* Try to load a configuration file. */
	rc = MSIM_CFG_Read(conf, conf_file);
	if (rc != 0) {
		if (conf_file != NULL) {
			snprintf(LOG, LOGSZ, "failed to open config: %s",
			         conf_file);
			MSIM_LOG_ERROR(LOG);
		}

		/* Try to load from the current working directory.*/
		rc = MSIM_CFG_Read(conf, CFG_FILE);
		if (rc != 0) {
			/* Try to load a system-wide file at least. */
			rc = MSIM_CFG_Read(conf, MSIM_CFG_FILE);
			if (rc != 0) {
				MSIM_LOG_ERROR("can't load any configuration");
				rc = 1;
			} else {
				MSIM_LOG_INFO("using config: " MSIM_CFG_FILE);
			}
		} else {
			MSIM_LOG_INFO("using config: " CFG_FILE);
		}
	} else {
		snprintf(LOG, LOGSZ, "using config: %s", conf_file);
		MSIM_LOG_INFO(LOG);
	}

	do {
		if (rc != 0) {
			/* We weren't able to load any config file. */
			break;
		}

		/* Try to open a firmware file */
		if (conf->reset_flash == 1U) {
			/* Firmware file has the priority. */
			if (conf->has_firmware_file == 1) {
				fp = fopen(conf->firmware_file, "r");
				if (fp == NULL) {
					snprintf(LOG, LOGSZ, "failed to read "
					         "firmware: %s",
					         conf->firmware_file);
					MSIM_LOG_FATAL(LOG);
					rc = 1;
					break;
				} else {
					snprintf(LOG, LOGSZ, "firmware: %s",
					         conf->firmware_file);
					MSIM_LOG_INFO(LOG);
				}
			} else {
				MSIM_LOG_FATAL("missing firmware in config");
				rc = 1;
				break;
			}
		} else {
			/* Utility file has the priority. */
			fp = fopen(FLASH_FILE, "r");
			if (fp == NULL) {
				MSIM_LOG_DEBUG("failed to read: " FLASH_FILE);

				if (conf->has_firmware_file == 1) {
					fp = fopen(conf->firmware_file, "r");
					if (fp == NULL) {
						snprintf(LOG, LOGSZ, "failed "
						         "to read: %s",
						         conf->firmware_file);
						MSIM_LOG_FATAL(LOG);
						rc = 1;
						break;
					} else {
						snprintf(LOG, LOGSZ, "using "
						         "firmware: %s",
						         conf->firmware_file);
						MSIM_LOG_INFO(LOG);
					}
				} else {
					MSIM_LOG_FATAL("missing firmware in "
					               "config");
					rc = 1;
					break;
				}
			} else {
				MSIM_LOG_DEBUG("using firmware: " FLASH_FILE);
			}
		}

		/* Initialize MCU as one of the AVR models */
		mcu->intr.trap_at_isr = conf->trap_at_isr;
		if (setup_avr(mcu, conf->mcu, NULL,
		                MSIM_AVR_PMSZ, NULL,
		                MSIM_AVR_DMSZ, NULL, fp) != 0) {
			snprintf(LOG, LOGSZ, "%s can't be initialized",
			         conf->mcu);
			MSIM_LOG_FATAL(LOG);
			rc = 1;
			break;
		}
		fclose(fp);

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

static int setup_avr(struct MSIM_AVR *mcu, const char *mcu_name,
                     uint8_t *pm, uint32_t pm_size,
                     uint8_t *dm, uint32_t dm_size,
                     uint8_t *mpm, FILE *fp)
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

	if (load_progmem(mcu, fp)) {
		MSIM_LOG_FATAL("program memory can't be loaded from a file");
		return -1;
	}
	mcu->state = AVR_STOPPED;
	mcu->read_from_mpm = 0;
	return 0;
}

static int load_progmem(struct MSIM_AVR *mcu, FILE *fp)
{
	IHexRecord rec, mem_rec;
	uint32_t base, addr;
	uint8_t *pm;

	if (fp == NULL) {
		MSIM_LOG_FATAL("can't read program memory from a file");
		return -1;
	}

	/* Copy HEX data to program memory of the MCU */
	pm = mcu->pm;
	base = 0;
	addr = 0;
	while (MSIM_IHEX_ReadRec(&rec, fp) == IHEX_OK) {
		/* Should base address be re-calculated? */
		if (rec.address < addr) {
			base += addr;
		}

		switch (rec.type) {
		case IHEX_TYPE_00:	/* Data */
			addr = rec.address;
			memcpy(pm+base+addr, rec.data, (uint16_t)rec.dataLen);
			break;
		case IHEX_TYPE_01:	/* End of File */
			break;
		default:		/* Other types, unlikely occured */
			continue;
		}
	}

	/* Verify checksum of the loaded data */
	rewind(fp);
	pm = mcu->pm;
	base = 0;
	addr = 0;
	while (MSIM_IHEX_ReadRec(&rec, fp) == IHEX_OK) {
		/* Should base address be re-calculated? */
		if (rec.address < addr) {
			base += addr;
		}
		if (rec.type == IHEX_TYPE_01) {
			break;
		}
		if (rec.type != IHEX_TYPE_00) {
			continue;
		}

		addr = rec.address;
		memcpy(mem_rec.data, pm+base+addr, (uint16_t)rec.dataLen);
		mem_rec.address = rec.address;
		mem_rec.dataLen = rec.dataLen;
		mem_rec.type = rec.type;
		mem_rec.checksum = 0;

		mem_rec.checksum = MSIM_IHEX_CalcChecksum(&mem_rec);
		if (mem_rec.checksum != rec.checksum) {
			snprintf(LOG, LOGSZ, "IHEX record checksum is not "
			         "correct: 0x%X (memory) != 0x%X (file)",
			         mem_rec.checksum, rec.checksum);
			MSIM_LOG_FATAL(LOG);
			MSIM_LOG_FATAL("file record:");
			MSIM_IHEX_PrintRec(&rec);
			MSIM_LOG_FATAL("memory record:");
			MSIM_IHEX_PrintRec(&mem_rec);

			return -1;
		}
	}
	return 0;
}

static int handle_irq(struct MSIM_AVR *mcu)
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
		mcu->pc = mcu->intr.ivt+(i*2);

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

static int pass_irqs(struct MSIM_AVR *mcu)
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
		for (uint32_t k = 0; k < ARR_LEN(vec); k++) {
			if (IS_NOINTV(vec[k])) {
				break;
			}

			en = IOBIT_RD(mcu, &vec[k]->enable);
			rai = IOBIT_RD(mcu, &vec[k]->raised);
			if ((en == 1U) && (rai == 1U)) {
				mcu->intr.irq[vec[k]->vector-1] = 1;
				IOBIT_WR(mcu, &vec[k]->raised, 0);
			}
		}

		/* Interrupts of the output compare channels */
		for (uint32_t k = 0; k < ARR_LEN(tmr->comp); k++) {
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
				mcu->intr.irq[comp->iv.vector-1] = 1;
				IOBIT_WR(mcu, &comp->iv.raised, 0);
			}
		}
	}

	return rc;
}

static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val)
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

static int set_lock(struct MSIM_AVR *m, uint8_t val)
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

static void print_config(struct MSIM_AVR *m)
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

void MSIM_AVR_StackPush(struct MSIM_AVR *mcu, unsigned char val)
{
	unsigned int sp;

	sp = (unsigned int)((*mcu->spl) | (*mcu->sph<<8));
	mcu->dm[sp--] = val;
	*mcu->spl = (unsigned char) (sp & 0xFF);
	*mcu->sph = (unsigned char) (sp >> 8);
}

uint8_t MSIM_AVR_StackPop(struct MSIM_AVR *mcu)
{
	unsigned int sp;
	unsigned char v;

	sp = (unsigned int)((*mcu->spl) | (*mcu->sph<<8));
	v = mcu->dm[++sp];
	*mcu->spl = (unsigned char)(sp & 0xFF);
	*mcu->sph = (unsigned char)(sp >> 8);
	return v;
}

void MSIM_AVR_PrintParts(void)
{
	uint32_t i;

	for (i = 0; i < sizeof(init_funcs)/sizeof(init_funcs[0]); i++) {
		printf("%s %s\n", init_funcs[i].partno, init_funcs[i].name);
	}
}

int MSIM_AVR_DumpFlash(struct MSIM_AVR *mcu, const char *dump)
{
	FILE *out;
	IHexRecord rec;
	const uint32_t data_t = IHEX_TYPE_00;
	const uint32_t eof_t = IHEX_TYPE_01;
	uint8_t eof_byte;
	uint32_t pmsz, off;
	int rc = 0;

	out = fopen(dump, "w");
	if (out == NULL) {
		snprintf(LOG, LOGSZ, "failed to open %s to dump flash "
		         "memory to", dump);
		MSIM_LOG_ERROR(LOG);
		rc = 1;
	}

	if (rc == 0) {
		/* Calculate actual size of the flash memory. */
		pmsz = mcu->flashend - mcu->flashstart + 1;

		/* Write program memory to the file in 16 bytes records. */
		off = 0;
		for (uint32_t i = 0; i < pmsz; i += 0x10) {
			/* The physical address of the data is computed by
			 * adding this offset to a previously established
			 * base address, thus allowing memory addressing beyond
			 * the 64 kilobyte limit of 16-bit addresses. */
			if (off > 0xFFF0) {
				off = 0x10;
			}
			MSIM_IHEX_NewRec(data_t, off, &mcu->pm[i], 0x10, &rec);
			MSIM_IHEX_WriteRec(&rec, out);
			off += 0x10;
		}
		MSIM_IHEX_NewRec(eof_t, 0, &eof_byte, 0, &rec);
		MSIM_IHEX_WriteRec(&rec, out);
	}
	if (out != NULL) {
		fclose(out);
	}
	return rc;
}
