/*
 * Copyright 2017-2018 The MCUSim Project.
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

/* glibc (starting from 2.2) requires _XOPEN_SOURCE >= 600 to expose
 * definitions for POSIX.1-2001 base specification plus XSI extension and
 * C99 definitions.
 *
 * This definition is required to let 'sigaction' structure and required
 * functions to be defined on GNU/Linux. */
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <signal.h>
#include "mcusim/mcusim.h"
#include "mcusim/getopt.h"
#include "mcusim/config.h"

/* Local macro definitions */
#define FUSE_EXT		2
#define FUSE_HIGH		1
#define FUSE_LOW		0
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))
#define CLEAR(byte, bit)	((byte)&=(~(1<<(bit))))
#define SET(byte, bit)		((byte)|=(1<<(bit)))
#define LOG			(mcu.log)
#define LOGSZ			MSIM_AVR_LOGSZ

/* Default GDB RSP port */
#define GDB_RSP_PORT		12750

/* Utility files to store MCU memories. */
#define FLASH_FILE		".mcusim.flash"
#define EEP_FILE		".mcusim.eeprom"

/* Configuration file (default name). */
#define CFG_FILE		"mcusim.conf"

/* Command line options */
#define CLI_OPTIONS		":c:"
#define VERSION_OPT		7576
#define PRINT_USAGE_OPT		7580
#define CONF_FILE_OPT		7581
/* END Local macro definitions */

/* Long command line options */
static struct option longopts[] = {
	{ "version", no_argument, NULL, VERSION_OPT },
	{ "help", no_argument, NULL, PRINT_USAGE_OPT },
	{ "conf", required_argument, NULL, CONF_FILE_OPT },
};

/* AVR MCU descriptor */
static struct MSIM_AVR mcu;
static struct MSIM_CFG conf;

/* Prototypes of the local functions */
static void print_usage(void);
static void print_short_usage(void);
static void print_config(struct MSIM_AVR *m);
static void print_version(void);
static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val);
static int set_lock(struct MSIM_AVR *m, uint8_t val);
static void dump_flash_handler(int s);
/* END Prototypes of the local functions */

/* Entry point of the simulator */
int main(int argc, char *argv[])
{
	extern char *optarg;
	FILE *fp;
	int c, rc, rc2, conf_rc = 1;
	uint32_t i, j;
	uint32_t dump_regs;
	char *conf_file;
	struct MSIM_AVR_VCD *vcd = &mcu.vcd;
	const uint32_t dflen = sizeof vcd->dump_file/sizeof vcd->dump_file[0];

	conf.mcu_freq = 0;
	conf.trap_at_isr = 0;
	conf.firmware_test = 0;
	conf.rsp_port = GDB_RSP_PORT;

	/* Adjust logging level in the debug version. */
#ifdef DEBUG
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif

	/* Set up signals handlers. */
	struct sigaction dmpflash_act;
	memset(&dmpflash_act, 0, sizeof dmpflash_act);
	dmpflash_act.sa_handler = dump_flash_handler;
	dmpflash_act.sa_flags = 0;
	sigemptyset(&dmpflash_act.sa_mask);
	sigaction(SIGABRT, &dmpflash_act, NULL);
	sigaction(SIGKILL, &dmpflash_act, NULL);
	sigaction(SIGQUIT, &dmpflash_act, NULL);
	sigaction(SIGSEGV, &dmpflash_act, NULL);
	sigaction(SIGTERM, &dmpflash_act, NULL);

	print_version();

	/* Interpret command ling arguments */
	c = getopt_long(argc, argv, CLI_OPTIONS, longopts, NULL);
	while (c != -1) {
		switch (c) {
		case ':':		/* Missing operand */
			snprintf(LOG, LOGSZ, "-%c requires operand", optopt);
			MSIM_LOG_FATAL(LOG);
			return 1;
		case '?':		/* Unknown option */
			snprintf(LOG, LOGSZ, "unknown option: -%c", optopt);
			MSIM_LOG_FATAL(LOG);
			return 1;
		case 'c':
			conf_file = optarg;
			conf_rc = MSIM_CFG_Read(&conf, optarg);
			break;
		case CONF_FILE_OPT:
			conf_file = optarg;
			conf_rc = MSIM_CFG_Read(&conf, optarg);
			break;
		case VERSION_OPT:
			print_short_usage();
			return 2;
		case PRINT_USAGE_OPT:
			print_usage();
			return 2;
		default:
			snprintf(LOG, LOGSZ, "unknown option: -%c", optopt);
			MSIM_LOG_WARN(LOG);
			break;
		}
		c = getopt_long(argc, argv, CLI_OPTIONS, longopts, NULL);
	}

	/* Try to load a configuration file if it has not been loaded yet. */
	if (conf_rc != 0) {
		/* Try to load from the working directory first.*/
		conf_rc = MSIM_CFG_Read(&conf, CFG_FILE);
		if (conf_rc != 0) {
			/* Try to load a system-wide file at least. */
			conf_rc = MSIM_CFG_Read(&conf, MSIM_CFG_FILE);
			if (conf_rc != 0) {
				MSIM_LOG_ERROR("cannot load any of the "
				               "configuration files");
				return 3;
			} else {
				snprintf(LOG, LOGSZ, "using config file: %s",
				         MSIM_CFG_FILE);
				MSIM_LOG_INFO(LOG);
			}
		} else {
			MSIM_LOG_INFO("using config file: " CFG_FILE);
		}
	} else {
		snprintf(LOG, LOGSZ, "using config file: %s", conf_file);
		MSIM_LOG_INFO(LOG);
	}

	/* Try to open a firmware file */
	if (conf.reset_flash == 1U) {
		/* Firmware file has the priority over the utility file. */
		if (conf.has_firmware_file == 1) {
			fp = fopen(conf.firmware_file, "r");
			if (fp == NULL) {
				snprintf(LOG, LOGSZ, "failed to read firmware "
				         "file: %s", conf.firmware_file);
				MSIM_LOG_FATAL(LOG);
				return 1;
			} else {
				snprintf(LOG, LOGSZ, "firmware file used: %s",
				         conf.firmware_file);
				MSIM_LOG_INFO(LOG);
			}
		} else {
			MSIM_LOG_FATAL("no firmware file specified in the "
			               "configuration");
			return 1;
		}
	} else {
		/* Try to load the utility file first. */
		fp = fopen(FLASH_FILE, "r");
		if (fp == NULL) {
			MSIM_LOG_DEBUG("failed to read: " FLASH_FILE);
			if (conf.has_firmware_file == 1) {
				fp = fopen(conf.firmware_file, "r");
				if (fp == NULL) {
					snprintf(LOG, LOGSZ, "failed to read "
					         "firmware file: %s",
					         conf.firmware_file);
					MSIM_LOG_FATAL(LOG);
					return 1;
				} else {
					snprintf(LOG, LOGSZ, "using firmware "
					         "file: %s", conf.firmware_file);
					MSIM_LOG_INFO(LOG);
				}
			} else {
				MSIM_LOG_FATAL("no firmware file specified "
				               "in the configuration");
				return 1;
			}
		} else {
			MSIM_LOG_DEBUG("using firmware file: " FLASH_FILE);
		}
	}

	/* Initialize MCU as one of AVR models */
	mcu.intr.trap_at_isr = conf.trap_at_isr;
	strncpy(vcd->dump_file, conf.vcd_file, dflen - 1);
	if (MSIM_AVR_Init(&mcu, conf.mcu, NULL, MSIM_AVR_PMSZ, NULL,
	                  MSIM_AVR_DMSZ, NULL, fp) != 0) {
		snprintf(LOG, LOGSZ, "MCU model %s cannot be initialized",
		         conf.mcu);
		MSIM_LOG_FATAL(LOG);
		return 1;
	}
	fclose(fp);

	/* Apply memory modifications */
	if (conf.has_lockbits == 1) {
		set_lock(&mcu, conf.mcu_lockbits);
	}
	if (conf.has_efuse == 1) {
		set_fuse(&mcu, FUSE_EXT, conf.mcu_efuse);
	}
	if (conf.has_hfuse == 1) {
		set_fuse(&mcu, FUSE_HIGH, conf.mcu_hfuse);
	}
	if (conf.has_lfuse == 1) {
		set_fuse(&mcu, FUSE_LOW, conf.mcu_lfuse);
	}

	/* Select registers to be dumped */
	dump_regs = 0;
	for (i = 0; i < conf.dump_regs_num; i++) {
		for (j = 0; j < MSIM_AVR_DMSZ; j++) {
			char *bit;
			char *pos;
			size_t len;
			int bitn, cr, bit_cr;

			if (mcu.ioregs[j].off < 0) {
				continue;
			}
			len = strlen(mcu.ioregs[j].name);
			if (len == 0) {
				continue;
			}

			pos = strstr(mcu.ioregs[j].name, conf.dump_regs[i]);
			cr = strncmp(mcu.ioregs[j].name, conf.dump_regs[i], len);

			/* Do we have a 16-bit register mentioned or an exact
			 * match of the register names? */
			if ((cr != 0) && (pos != NULL)) {
				if (mcu.ioregs[j].name[len-1] == 'H') {
					vcd->regs[dump_regs].i = (int32_t)j;
				}
				if (mcu.ioregs[j].name[len-1] == 'L') {
					vcd->regs[dump_regs].reg_lowi =
					        (int32_t)j;
				}

				if ((vcd->regs[dump_regs].i >= 0) &&
				                (vcd->regs[dump_regs].
				                 reg_lowi >= 0)) {
					vcd->regs[dump_regs].n = -1;
					strncpy(vcd->regs[dump_regs].name,
					        mcu.ioregs[j].name, sizeof
					        vcd->regs[dump_regs].name);
					vcd->regs[dump_regs].name[len-1] = 0;

					dump_regs++;
					break;
				}
			} else if (cr != 0) {
				continue;
			} else {
				/* Do we have a bit index suffix? */
				bit = len < sizeof conf.dump_regs[0]/
				      sizeof conf.dump_regs[0][0]
				      ? &conf.dump_regs[i][len] : NULL;
				bit_cr = sscanf(bit, "%d", &bitn);
				if (bit_cr != 1) {
					bitn = -1;
				}

				/* Set index of a register to be dumped */
				vcd->regs[dump_regs].i = (int32_t)j;
				vcd->regs[dump_regs].n = (int8_t)bitn;
				strncpy(vcd->regs[dump_regs].name,
				        mcu.ioregs[j].name,
				        sizeof vcd->regs[dump_regs].name);

				dump_regs++;
				break;
			}
		}
	}

	/* Try to set required frequency */
	if (conf.mcu_freq > mcu.freq) {
		snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%" PRIu64
		         " kHz is above maximum %lu.%lu kHz",
		         conf.mcu_freq/1000U, conf.mcu_freq%1000U,
		         mcu.freq/1000UL, mcu.freq%1000UL);
		MSIM_LOG_WARN(LOG);
	} else if (conf.mcu_freq > 0U) {
		mcu.freq = (uint32_t)conf.mcu_freq;
	} else {
		snprintf(LOG, LOGSZ, "clock frequency %" PRIu64 ".%"
		         PRIu64 " kHz cannot be selected as clock source",
		         conf.mcu_freq/1000U, conf.mcu_freq%1000U);
		MSIM_LOG_WARN(LOG);
	}

	/* Print MCU configuration */
	print_config(&mcu);

	/* Load Lua peripherals if it is required */
	for (uint32_t k = 0; k < conf.lua_models_num; k++) {
		char *lua_model = &conf.lua_models[k][0];
		if (MSIM_AVR_LUALoadModel(&mcu, lua_model)) {
			MSIM_LOG_FATAL("loading Lua model failed");
		}
	}

	/* Prepare and run AVR simulation */
	if (conf.firmware_test == 0) {
		snprintf(LOG, LOGSZ, "Waiting for incoming GDB "
		         "connections at localhost:%d...", conf.rsp_port);
		MSIM_LOG_INFO(LOG);
		MSIM_AVR_RSPInit(&mcu, (int)conf.rsp_port);
	}

	rc = MSIM_AVR_Simulate(&mcu, 0, mcu.flashend+1, conf.firmware_test);

	MSIM_PTY_Close(&mcu.pty);
	MSIM_AVR_LUACleanModels();
	if (conf.firmware_test == 0) {
		MSIM_AVR_RSPClose();
	}

	rc2 = MSIM_AVR_DumpFlash(&mcu, FLASH_FILE);
	if (rc2 != 0) {
		MSIM_LOG_ERROR("failed to dump memory to: " FLASH_FILE);
	}

	return rc;
}

static void print_usage(void)
{
	/* Print usage and options */
	printf("Usage: mcusim [options]\n"
	       "Options:\n"
	       "  -p <partno|?>              Specify AVR device "
	       "(required).\n"
	       "  -U <memtype>:w:<filename|value>[:<format>]\n"
	       "                             Memory operation "
	       "specification (required).\n"
	       "  -r <filename>              Specify text file with "
	       "simulated modules written in Lua.\n"
	       "  --dump-regs=<reg0,reg1,...,regN|?>\n"
	       "                             Dump specified registers "
	       "into VCD file.\n"
	       "  --help                     Print this message.\n");
	printf("  -P <port>, --rsp-port=<port>\n"
	       "                             Set port to listen to the "
	       "incoming connections from GDB RSP.\n"
	       "  -f <frequency>             MCU frequency, in Hz.\n"
	       "  --trap-at-isr              Enter stopped mode when "
	       "interrupt occured.\n");

	/* Print examples */
	printf("Examples:\n"
	       "  mcusim -p m328p -U flash:w:./dhtc.hex -U "
	       "hfuse:w:0x57:h -r ./lua-modules --dump-regs=PORTB,PORTC\n"
	       "  mcusim -p m8a -U flash:w:./dhtc.hex "
	       "-r ./lua-modules -f 1000000\n\n");
}

static void print_short_usage(void)
{
	printf("Usage: mcusim --help\n");
}

static void print_version(void)
{
#ifndef DEBUG
	printf("MCUSim %s: Microcontroller-based circuit simulator\n"
	       "        Copyright 2017-2018 The MCUSim Project.\n"
	       "        Please find documentation at https://trac.mcusim.org\n"
	       "        Please file your bug-reports at "
	       "https://trac.mcusim.org/newticket\n", MSIM_VERSION);
#else
	printf("MCUSim %s: Microcontroller-based circuit simulator "
	       "(debug)\n"
	       "        Copyright 2017-2018 The MCUSim Project.\n"
	       "        Please find documentation at https://trac.mcusim.org\n"
	       "        Please file your bug-reports at "
	       "https://trac.mcusim.org/newticket\n", MSIM_VERSION);
#endif
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

	snprintf(m->log, LOGSZ, "MCU model: %s (signature %02X%02X%02X)",
	         m->name, m->signature[0], m->signature[1], m->signature[2]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "clock frequency: %" PRIu32 ".%" PRIu32
	         " kHz", m->freq/1000, m->freq%1000);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "fuses: EXT=0x%02X, HIGH=0x%02X, "
	         "LOW=0x%02X", m->fuse[2], m->fuse[1], m->fuse[0]);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "lock bits: 0x%02X", m->lockbits);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "reset vector address: 0x%06lX", reset_pc);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "interrupt vectors table address: "
	         "0x%06lX", ivt);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "flash section: 0x%06lX-0x%06lX",
	         flashstart, flashend);
	MSIM_LOG_INFO(m->log);

	snprintf(m->log, LOGSZ, "bootloader section: 0x%06lX-0x%06lX",
	         blsstart, blsend);
	MSIM_LOG_INFO(m->log);
}

static int set_fuse(struct MSIM_AVR *m, uint32_t fuse, uint8_t val)
{
	if (m->set_fusef == NULL) {
		MSIM_LOG_WARN("cannot modify fuse (MCU-specific function is "
		              "not available)");
		return 0;
	}
	m->set_fusef(m, fuse, val);
	return 0;
}

static int set_lock(struct MSIM_AVR *m, uint8_t val)
{
	if (m->set_lockf == NULL) {
		MSIM_LOG_WARN("cannot modify lock bits (MCU-specific function "
		              "is not available)");
		return 0;
	}
	m->set_lockf(m, val);
	return 0;
}

static void dump_flash_handler(int s)
{
	int rc;

	rc = MSIM_AVR_DumpFlash(&mcu, FLASH_FILE);
	if (rc != 0) {
		MSIM_LOG_ERROR("failed to dump memory to: " FLASH_FILE);
	}
}
