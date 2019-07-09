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

/*
 * glibc (starting from 2.2) requires _XOPEN_SOURCE >= 600 to expose
 * definitions for POSIX.1-2001 base specification plus XSI extension and
 * C99 definitions.
 *
 * This definition is required to let 'sigaction' structure and required
 * functions to be defined on GNU/Linux.
 */
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
#include "mcusim/avr/sim/private/macro.h"

#define FLASH_FILE		".mcusim.flash"
#define GDB_RSP_PORT		12750

/* Command line options */
#define CLI_OPTIONS		":c:"
#define VERSION_OPT		7576
#define PRINT_USAGE_OPT		7580
#define CONF_FILE_OPT		7581

/* Long command line options */
static struct MSIM_OPT_Option longopts[] = {
	{ "version", MSIM_OPT_NO_ARGUMENT, NULL, VERSION_OPT },
	{ "help", MSIM_OPT_NO_ARGUMENT, NULL, PRINT_USAGE_OPT },
	{ "conf", MSIM_OPT_REQUIRED_ARGUMENT, NULL, CONF_FILE_OPT },
};

static struct MSIM_AVR avr_mcu;
static struct MSIM_AVR *mcu = &avr_mcu;
static struct MSIM_CFG conf;

static void	print_usage(void);
static void	print_short_usage(void);
static void	dump_flash_handler(int s);

int
main(int argc, char *argv[])
{
	int c, rc;
	char *conf_file = NULL;

	conf.mcu_freq = 0;
	conf.trap_at_isr = 0;
	conf.firmware_test = 0;
	conf.rsp_port = GDB_RSP_PORT;

#ifdef DEBUG
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif

	/* Set up signals handlers. */
	int signals[] = { SIGABRT, SIGKILL, SIGQUIT, SIGSEGV, SIGTERM };
	struct sigaction dmpflash_act;

	memset(&dmpflash_act, 0, sizeof dmpflash_act);
	sigemptyset(&dmpflash_act.sa_mask);
	dmpflash_act.sa_handler = dump_flash_handler;

	for (uint32_t i = 0; i < ARRSZ(signals); i++) {
		sigaction(signals[i], &dmpflash_act, NULL);
	}

	MSIM_CFG_PrintVersion();

	/* Raad command line arguments */
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
		/* Read config file */
		rc = MSIM_CFG_Read(&conf, conf_file);
		if (rc != 0) {
			break;
		}

		/* Initialize AVR */
		rc = MSIM_AVR_Init(mcu, &conf);
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
			MSIM_AVR_RSPClose(mcu);
		}

		if (MSIM_AVR_SaveProgMem(mcu, FLASH_FILE) != 0) {
			MSIM_LOG_ERROR("failed to dump to: " FLASH_FILE);
		}
		break;
	} while (0);

	return rc;
}

static void
print_short_usage(void)
{
	printf("Usage: mcusim --help\n");
}

static void
print_usage(void)
{
	/* Print usage and options */
	printf("Usage: mcusim [options]\n"
	       "Options:\n"
	       "  -c <config_file>     Run with this configuration file.\n"
	       "  --conf <config_file> Run with this configuration file.\n"
	       "  --help               Print this message.\n"
	       "  --version            Print version.\n");
}

static void
dump_flash_handler(int s)
{
	int rc;

	rc = MSIM_AVR_SaveProgMem(mcu, FLASH_FILE);
	if (rc != 0) {
		MSIM_LOG_ERROR("failed to dump memory to: " FLASH_FILE);
	}
}
