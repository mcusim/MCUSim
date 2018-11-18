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
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/getopt.h"
#include "mcusim/log.h"

#define FUSE_LOW		0
#define FUSE_HIGH		1
#define FUSE_EXT		2
#define IS_SET(byte, bit)	(((byte)&(1UL<<(bit)))>>(bit))
#define IS_RISE(init, val, bit)	((!((init>>bit)&1)) & ((val>>bit)&1))
#define IS_FALL(init, val, bit)	(((init>>bit)&1) & (!((val>>bit)&1)))
#define CLEAR(byte, bit)	((byte)&=(~(1<<(bit))))
#define SET(byte, bit)		((byte)|=(1<<(bit)))

#define GDB_RSP_PORT		12750		/* Default GDB RSP port */
#define MAX_MEMOPS		16		/* Maximum MCU memory
						   modifications during
						   initialization */

/* Command line options */
#define CLI_OPTIONS		":p:U:r:P:f:"
#define DUMP_REGS_OPT		7575
#define VERSION_OPT		7576
#define GDB_RSP_PORT_OPT	7577
#define TRAP_AT_ISR_OPT		7578
#define SHORT_VERSION_OPT	7579
#define PRINT_USAGE_OPT		7580
#define FIRMWARE_TEST_OPT	7581

/* Long command line options */
static struct option longopts[] = {
	{ "dump-regs", required_argument, NULL, DUMP_REGS_OPT },
	{ "rsp-port", required_argument, NULL, GDB_RSP_PORT_OPT },
	{ "trap-at-isr", no_argument, NULL, TRAP_AT_ISR_OPT },
	{ "version", no_argument, NULL, VERSION_OPT },
	{ "short-version", no_argument, NULL, SHORT_VERSION_OPT },
	{ "help", no_argument, NULL, PRINT_USAGE_OPT },
	{ "firmware-test", no_argument, NULL, FIRMWARE_TEST_OPT }
};

static struct MSIM_AVR mcu;		/* AVR MCU descriptor */

/* VCD dump options */
static char vcd_regs[MSIM_AVR_VCD_REGS][16]; /* MCU registers to dump */
static unsigned int vcd_rn;		/* Number of registers */
static char print_regs;			/* Print available registers
					   which can be dumped */

/* MCU memory operations */
static struct MSIM_AVR_MemOp memops[MAX_MEMOPS];
static unsigned short memops_num;

static void print_usage(void);
static void print_short_usage(void);
static void print_config(const struct MSIM_AVR *m);
static void print_version(void);
static void parse_dump(char *cmd);
static int parse_rsp_port(const char *opt);
static int parse_memop(char *opt);
static unsigned int parse_freq(const char *opt);
static int apply_memop(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo);
static int set_fuse(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo,
                    unsigned int fuse_n);
static int set_lock(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo);

int main(int argc, char *argv[])
{
	extern char *optarg;
	FILE *fp;
	int c, rsp_port, sim_retcode;
	char *partno, *luap;
	char log[1024];		/* Buffer to write a log message to. */
	uint32_t i, j;
	uint32_t freq;
	uint32_t dump_regs;	/* # of registers to store in VCD dump. */
	uint8_t trap_at_isr;	/* Flag to trap debugger right before ISR. */
	uint8_t firmware_test;	/* Start simulator without waiting for a
	                           debugger in order to test firmware. */

	partno = NULL;
	luap = NULL;
	vcd_rn = 0;
	print_regs = 0;
	memops_num = 0;
	freq = 0;
	trap_at_isr = 0;
	firmware_test = 0;
	rsp_port = GDB_RSP_PORT;

#ifdef DEBUG
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif

	c = getopt_long(argc, argv, CLI_OPTIONS, longopts, NULL);
	while (c != -1) {
		switch (c) {
		case 'p':		/* Required. AVR device. */
			partno = optarg;
			break;
		case 'U':		/* Required. Memory operation. */
			if (parse_memop(optarg) != 0) {
				MSIM_LOG_FATAL("Incorrect memory operation "
				               "specified!");
				return 1;
			}
			break;
		case 'r':		/* Optional. Lua peripherals file. */
			luap = optarg;
			break;
		case 'P':
			rsp_port = parse_rsp_port(optarg);
			break;
		case 'f':
			freq = parse_freq(optarg);
			break;
		case ':':		/* Missing operand */
			snprintf(log, sizeof log, "Option -%c requires an "
			         "operand", optopt);
			MSIM_LOG_FATAL(log);
			return 1;
		case '?':		/* Unknown option */
			snprintf(log, sizeof log, "Unknown option: -%c",
			         optopt);
			MSIM_LOG_FATAL(log);
			return 1;
		case DUMP_REGS_OPT:	/* Registers to dump into VCD file */
			parse_dump(optarg);
			break;
		case GDB_RSP_PORT_OPT:	/* Port for GDB RSP clients */
			rsp_port = parse_rsp_port(optarg);
			break;
		case TRAP_AT_ISR_OPT:	/* Enter stopped mode when interrupt
					   occured */
			trap_at_isr = 1;
			break;
		case VERSION_OPT:	/* Print version only */
			print_version();
			print_short_usage();
			return 2;
		case SHORT_VERSION_OPT:
			printf("%s\n", MSIM_VERSION);
			return 2;
		case PRINT_USAGE_OPT:
			print_usage();
			return 2;
		case FIRMWARE_TEST_OPT:
			firmware_test = 1;
			break;
		default:
			snprintf(log, sizeof log, "Unknown option: -%c",
			         optopt);
			MSIM_LOG_WARN(log);
			break;
		}
		c = getopt_long(argc, argv, CLI_OPTIONS, longopts, NULL);
	}

	if ((partno != NULL) && (!strcmp(partno, "?"))) {
		MSIM_AVR_PrintParts();
		return 2;
	}
	print_version();
	if (partno == NULL) {
		/* MCU model is necessary! */
		MSIM_LOG_FATAL("Please, specify MCU model (-p option)");
		return 1;
	}

	/* Filling a program memory */
	fp = NULL;
	for (i = 0; i < memops_num; i++) {
		if ((memops[i].format == 'f') &&
		                (!strcmp(memops[i].memtype, "flash"))) {
			/* Try to open file to read program memory from */
			fp = fopen(memops[i].operand, "r");
			if (!fp) {
				snprintf(log, sizeof log, "Failed to open "
				         "file to read a content of flash "
				         "memory from: %s", memops[i].operand);
				MSIM_LOG_FATAL(log);
				return 1;
			}
			/* Mark this operation as applied one */
			memops[i].format = -1;
		}
	}
	if (fp == NULL) {
		MSIM_LOG_FATAL("It is necessary to fill a program memory "
		               "(option -U flash:w:<filename>)");
		return 1;
	}

	/* Initialize MCU as one of AVR models */
	mcu.intr.trap_at_isr = trap_at_isr;
	if (MSIM_AVR_Init(&mcu, partno, NULL, MSIM_AVR_PMSZ, NULL, MSIM_AVR_DMSZ, NULL,
	                  fp) != 0) {
		snprintf(log, sizeof log, "MCU model %s cannot be initialized",
		         partno);
		MSIM_LOG_FATAL(log);
		return 1;
	}
	fclose(fp);

	/* Create a pseudo-terminal for this MCU */
#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
	mcu.pty.master_fd = -1;
	mcu.pty.slave_fd = -1;
	mcu.pty.slave_name[0] = 0;
	MSIM_PTY_Open(&mcu.pty);
#endif

	/* Do we have to print available registers only? */
	if (print_regs != 0) {
		for (i = 0; i < MSIM_AVR_DMSZ; i++) {
			if (mcu.ioregs[i].name[0] == 0) {
				continue;
			}
			if (mcu.ioregs[i].off < 0) {
				continue;
			}
			snprintf(log, sizeof log, "%s (0x%2" PRIX32 ")",
			         mcu.ioregs[i].name,
			         mcu.ioregs[i].off);
			MSIM_LOG_INFO(log);
		}
		return 2;
	}

	/* Apply memory modifications */
	for (i = 0; i < memops_num; i++) {
		if (apply_memop(&mcu, &memops[i]) != 0) {
			snprintf(log, sizeof log, "Memory modification "
			         "failed: memtype=%s, op=%c, val=%s",
			         &memops[i].memtype[0], memops[i].operation,
			         &memops[i].operand[0]);
			MSIM_LOG_FATAL(log);
			return 1;
		}
	}

	/* Select registers to be dumped */
	dump_regs = 0;
	for (i = 0; i < vcd_rn; i++) {
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

			pos = strstr(mcu.ioregs[j].name, vcd_regs[i]);
			cr = strncmp(mcu.ioregs[j].name, vcd_regs[i], len);

			/* Do we have a 16-bit register mentioned or an exact
			 * match of the register names? */
			if ((cr != 0) && (pos != NULL)) {
				if (mcu.ioregs[j].name[len-1] == 'H') {
					mcu.vcd[dump_regs].i = (int32_t)j;
				}
				if (mcu.ioregs[j].name[len-1] == 'L') {
					mcu.vcd[dump_regs].reg_lowi =
					        (int32_t)j;
				}

				if ((mcu.vcd[dump_regs].i >= 0) &&
				                (mcu.vcd[dump_regs].
				                 reg_lowi >= 0)) {
					mcu.vcd[dump_regs].n = -1;
					strncpy(mcu.vcd[dump_regs].name,
					        mcu.ioregs[j].name, sizeof
					        mcu.vcd[dump_regs].name);
					mcu.vcd[dump_regs].name[len-1] = 0;

					dump_regs++;
					break;
				}
			} else if (cr != 0) {
				continue;
			} else {
				/* Do we have a bit index suffix? */
				bit = len < sizeof vcd_regs[0]/
				      sizeof vcd_regs[0][0]
				      ? &vcd_regs[i][len] : NULL;
				bit_cr = sscanf(bit, "%d", &bitn);
				if (bit_cr != 1) {
					bitn = -1;
				}

				/* Set index of a register to be dumped */
				mcu.vcd[dump_regs].i = (int32_t)j;
				mcu.vcd[dump_regs].n = (int8_t)bitn;
				strncpy(mcu.vcd[dump_regs].name,
				        mcu.ioregs[j].name,
				        sizeof mcu.vcd[dump_regs].name);

				dump_regs++;
				break;
			}
		}
	}

	/* Try to set required frequency */
	if (freq > mcu.freq) {
		snprintf(log, sizeof log, "Clock frequency %u.%u kHz is "
		         "above maximum possible %lu.%lu kHz for a selected "
		         "clock source", freq/1000U, freq%1000U,
		         mcu.freq/1000UL, mcu.freq%1000UL);
		MSIM_LOG_WARN(log);
	} else if (freq > 0U) {
		mcu.freq = freq;
	} else {
		snprintf(log, sizeof log, "Clock frequency %u.%u kHz cannot "
		         "be selected as a clock source",
		         freq/1000U, freq%1000U);
		MSIM_LOG_WARN(log);
	}

	/* Print MCU configuration */
	print_config(&mcu);

	/* Load Lua peripherals if it is required */
	if (luap && MSIM_AVR_LUALoadModels(&mcu, luap)) {
		MSIM_LOG_FATAL("Loading Lua device models failed");
		return 1;
	}

	/* Prepare and run AVR simulation */
	if (!firmware_test) {
		snprintf(log, sizeof log, "Waiting for incoming GDB "
		         "connections at localhost:%d...", rsp_port);
		MSIM_LOG_INFO(log);
		MSIM_AVR_RSPInit(&mcu, rsp_port);
	}

	sim_retcode = MSIM_AVR_Simulate(
	                      &mcu, 0, mcu.flashend+1, firmware_test);

#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)
	MSIM_PTY_Close(&mcu.pty);
#endif
	MSIM_AVR_LUACleanModels();
	if (!firmware_test) {
		MSIM_AVR_RSPClose();
	}

	return sim_retcode;
}

static void print_usage(void)
{
	/* Print usage and options */
	print_version();
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
	printf("MCUSim %s : Microcontroller-based circuit simulator\n"
	       "        Copyright (c) 2017, 2018, The MCUSim Contributors\n"
	       "        Please find documentation at https://trac.mcusim.org\n"
	       "        Please file your bug-reports at "
	       "https://trac.mcusim.org/newticket\n", MSIM_VERSION);
#else
	printf("MCUSim %s : Microcontroller-based circuit simulator "
	       "(Debug version)\n"
	       "        Copyright (c) 2017, 2018, The MCUSim Contributors\n"
	       "        Please find documentation at https://trac.mcusim.org\n"
	       "        Please file your bug-reports at "
	       "https://trac.mcusim.org/newticket\n", MSIM_VERSION);
#endif
}

static void print_config(const struct MSIM_AVR *m)
{
	/* AVR memory is organized as array of bytes in the simulator, but
	 * it's natural to measure program memory in 16-bits words because
	 * all AVR instructions are 16- or 32-bits wide. This is why all
	 * program memory addresses are divided by two before printing. */
	char log[1024];
	uint64_t reset_pc = m->intr.reset_pc>>1;
	uint64_t ivt = m->intr.ivt>>1;
	uint64_t flashstart = m->flashstart>>1;
	uint64_t flashend = m->flashend>>1;
	uint64_t blsstart = m->bls.start>>1;
	uint64_t blsend = m->bls.end>>1;

	snprintf(log, sizeof log, "MCU model: %s (signature %02X%02X%02X)",
	         m->name, m->signature[0], m->signature[1], m->signature[2]);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "clock frequency: %" PRIu32 ".%" PRIu32
	         " kHz", m->freq/1000, m->freq%1000);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "fuses: EXT=0x%02X, HIGH=0x%02X, "
	         "LOW=0x%02X", m->fuse[2], m->fuse[1], m->fuse[0]);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "lock bits: 0x%02X", m->lockbits);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "reset vector address: 0x%06lX", reset_pc);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "interrupt vectors table address: "
	         "0x%06lX", ivt);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "flash section: 0x%06lX-0x%06lX",
	         flashstart, flashend);
	MSIM_LOG_INFO(log);

	snprintf(log, sizeof log, "bootloader section: 0x%06lX-0x%06lX",
	         blsstart, blsend);
	MSIM_LOG_INFO(log);
}

static void parse_dump(char *cmd)
{
	if (cmd == NULL) {
		return;
	}
	if (!strcmp(cmd, "?")) {
		/* User asked to print all registers which can be included
		 * into VCD dump. Set flag and return, no more. */
		print_regs = 1;
		return;
	} else {
		int c;
		char *p;
		/* We've got a register name to parse. Let's do it! */
		for (c = 0; cmd[c] != 0; c++) {
			if (cmd[c] == ',') {
				cmd[c] = ' ';
			}
		}

		p = cmd;
		while (1) {
			int ret;
			ret = sscanf(p, "%15s", &vcd_regs[vcd_rn][0]);
			if (ret == EOF) {
				break;
			}
			if (ret == 1) {
				vcd_rn++;
				p = strstr(p, " ");
				if (p == NULL) {
					break;
				}
				p++;
				continue;
			}
			fprintf(stderr, "[e]: Failed to parse list of "
			        "registers to dump: %s\n", p);
			break;
		}
		return;
	}
}

static int parse_rsp_port(const char *opt)
{
	int rsp_port;

	rsp_port = atoi(opt);
	if (!((rsp_port > 1024) && (rsp_port < 65536))) {
		fprintf(stderr, "[!]: GDB RSP port should be in a "
		        "(1024, 65536) range exclusively! Default port "
		        "will be used.\n");
		rsp_port = GDB_RSP_PORT;
	}
	return rsp_port;
}

static unsigned int parse_freq(const char *opt)
{
	int freq;

	freq = atoi(opt);
	if (freq <= 0) {
		fprintf(stderr, "[!]: Frequency should be positive!\n");
		return 0;
	}
	return (unsigned int)freq;
}

static int parse_memop(char *opt)
{
	int c, r;

	for (c = 0; opt[c] != 0; c++) {
		if (opt[c] == ':') {
			opt[c] = ' ';
		}
	}
	r = sscanf(opt, "%16s %1c %4096s %1c",
	           &memops[memops_num].memtype[0],
	           &memops[memops_num].operation,
	           &memops[memops_num].operand[0],
	           &memops[memops_num].format);

	if ((r==EOF) || (r<3) || (r>4)) {
		/* Something went wrong */
		return 1;
	}
	if (r==3) {
		/* 'File' format is default one */
		memops[memops_num].format = 'f';
	}
	memops_num++;
	return 0;
}

static int apply_memop(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo)
{
	int r;

	if (mo->format < 0) {	/* Operation is applied already, skipping */
		r = 0;
	} else if (!strcmp(mo->memtype, "efuse")) {
		r = set_fuse(m, mo, FUSE_EXT);
	} else if (!strcmp(mo->memtype, "hfuse")) {
		r = set_fuse(m, mo, FUSE_HIGH);
	} else if (!strcmp(mo->memtype, "lfuse")) {
		r = set_fuse(m, mo, FUSE_LOW);
	} else if (!strcmp(mo->memtype, "lock")) {
		r = set_lock(m, mo);
	} else {
		r = 1;
	}
	return r;
}

static int set_fuse(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo,
                    unsigned int fuse_n)
{
	unsigned int fusev;

	if (m->set_fusef == NULL) {
		fprintf(stderr, "[!]: Cannot modify fuse, MCU-specific "
		        "function is not available\n");
		return 0;	/* No function to set fuse */
	}
	if (mo->format != 'h') {
		fprintf(stderr, "[!]: Failed to modify fuse, expected "
		        "format is 'h'\n");
		return 1;
	}

	if (sscanf(mo->operand, "0x%2X", &fusev) != 1) {
		fprintf(stderr, "[e]: Failed to parse fuse value from %s!\n",
		        mo->operand);
		return 1;
	}
	m->set_fusef(m, fuse_n, (unsigned char)fusev);
	mo->format = -1;	/* Operation is applied correctly */
	return 0;
}

static int set_lock(struct MSIM_AVR *m, struct MSIM_AVR_MemOp *mo)
{
	unsigned int lockv;

	if (m->set_lockf == NULL) {
		fprintf(stderr, "[!]: Cannot modify lock bits, MCU-specific "
		        "function is not available\n");
		return 0;	/* No function to set lock bits */
	}
	if (mo->format != 'h') {
		fprintf(stderr, "[!]: Failed to modify lock byte, expected "
		        "format is 'h'\n");
		return 1;
	}

	if (sscanf(mo->operand, "0x%2X", &lockv) != 1) {
		fprintf(stderr, "[e]: Failed to parse lock value from %s!\n",
		        mo->operand);
		return 1;
	}
	m->set_lockf(m, (unsigned char)lockv);
	mo->format = -1;	/* Operation is applied correctly */
	return 0;
}
