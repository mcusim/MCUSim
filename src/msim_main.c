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
#include "mcusim/avr/sim/macro.h"

/* Utility files to store MCU memories. */
#define FLASH_FILE		".mcusim.flash"

/* Default GDB RSP port */
#define GDB_RSP_PORT		12750

/* Command line options */
#define CLI_OPTIONS		":c:"
#define VERSION_OPT		7576
#define PRINT_USAGE_OPT		7580
#define CONF_FILE_OPT		7581
/* END Local macro definitions */

/* Long command line options */
static struct MSIM_OPT_Option longopts[] = {
	{ "version", MSIM_OPT_NO_ARGUMENT, NULL, VERSION_OPT },
	{ "help", MSIM_OPT_NO_ARGUMENT, NULL, PRINT_USAGE_OPT },
	{ "conf", MSIM_OPT_REQUIRED_ARGUMENT, NULL, CONF_FILE_OPT },
};

/* AVR MCU descriptor */
static struct MSIM_AVR avr_mcu;
static struct MSIM_AVR *mcu = &avr_mcu;
static struct MSIM_CFG conf;

/* Prototypes of the local functions */
static void print_usage(void);
static void print_short_usage(void);
#ifdef WITH_POSIX
static void dump_flash_handler(int s);
#endif
/* END Prototypes of the local functions */

int main(int argc, char *argv[])
{
	int c, rc;
	char *conf_file = NULL;

	conf.mcu_freq = 0;
	conf.trap_at_isr = 0;
	conf.firmware_test = 0;
	conf.rsp_port = GDB_RSP_PORT;

	/* Adjust logging level in the debug version. */
#ifdef DEBUG
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif
#ifdef WITH_POSIX
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
#endif

	MSIM_CFG_PrintVersion();

	/* Interpret command ling arguments */
	c = MSIM_OPT_Getopt_long(argc, argv, CLI_OPTIONS, longopts, NULL);
	while (c != -1) {
		switch (c) {
		case ':':		/* Missing operand */
			snprintf(LOG, LOGSZ, "-%c requires operand",
			         MSIM_OPT_optopt);
			MSIM_LOG_FATAL(LOG);
			return 1;
		case '?':		/* Unknown option */
			snprintf(LOG, LOGSZ, "unknown option: -%c",
			         MSIM_OPT_optopt);
			MSIM_LOG_FATAL(LOG);
			return 1;
		case 'c':
			conf_file = MSIM_OPT_optarg;
			break;
		case CONF_FILE_OPT:
			conf_file = MSIM_OPT_optarg;
			break;
		case VERSION_OPT:
			print_short_usage();
			return 2;
		case PRINT_USAGE_OPT:
			print_usage();
			return 2;
		default:
			snprintf(LOG, LOGSZ, "unknown option: -%c",
			         MSIM_OPT_optopt);
			MSIM_LOG_WARN(LOG);
			break;
		}
		c = MSIM_OPT_Getopt_long(argc, argv, CLI_OPTIONS,
		                         longopts, NULL);
	}

	do {
		/* Initialize AVR */
		rc = MSIM_AVR_Init(mcu, &conf, conf_file);
		if (rc != 0) {
			break;
		}

		/* Prepare and run AVR simulation */
		if (conf.firmware_test == 0) {
			snprintf(LOG, LOGSZ, "waiting for incoming GDB "
			         "connections at localhost:%d...",
			         conf.rsp_port);
			MSIM_LOG_INFO(LOG);
			MSIM_AVR_RSPInit(mcu, (uint16_t)conf.rsp_port);
		}

		rc = MSIM_AVR_Simulate(mcu, conf.firmware_test);

		MSIM_PTY_Close(&mcu->pty);
		MSIM_AVR_LUACleanModels();
		if (conf.firmware_test == 0) {
			MSIM_AVR_RSPClose();
		}

		if (MSIM_AVR_DumpFlash(mcu, FLASH_FILE) != 0) {
			MSIM_LOG_ERROR("failed to dump to: " FLASH_FILE);
		}
	} while (0);

	return rc;
}

static void print_short_usage(void)
{
	printf("Usage: mcusim --help\n");
}

static void print_usage(void)
{
	/* Print usage and options */
	printf("Usage: mcusim [options]\n"
	       "Options:\n"
	       "  -c <config_file>     Run with this configuration file.\n"
	       "  --conf <config_file> Run with this configuration file.\n"
	       "  --help               Print this message.\n"
	       "  --version            Print version.\n");
}

#ifdef WITH_POSIX
static void dump_flash_handler(int s)
{
	int rc;

	rc = MSIM_AVR_DumpFlash(mcu, FLASH_FILE);
	if (rc != 0) {
		MSIM_LOG_ERROR("failed to dump memory to: " FLASH_FILE);
	}
}
#endif
