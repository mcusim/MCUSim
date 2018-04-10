/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

#include "mcusim/cli.h"
#include "mcusim/getopt.h"
#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/simcore.h"
#include "mcusim/avr/sim/bootloader.h"
#include "mcusim/avr/sim/peripheral_lua.h"
#include "mcusim/avr/sim/gdb_rsp.h"
#include "mcusim/avr/sim/interrupt.h"
#include "mcusim/avr/sim/vcd_dump.h"

/* Limits of statically allocated MCU memory */
#define PMSZ			256*1024	/* Program memory limit */
#define DMSZ			64*1024		/* Data memory limit */
#define PM_PAGESZ		1024		/* PM page size */

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

static struct option longopts[] = {
	{ "dump-regs", required_argument, NULL, DUMP_REGS_OPT },
	{ "version", no_argument, NULL, VERSION_OPT },
	{ "rsp-port", required_argument, NULL, GDB_RSP_PORT_OPT },
	{ "trap-at-isr", no_argument, NULL, TRAP_AT_ISR_OPT }
};
/* END Command line options */

static struct MSIM_AVR mcu;		/* Central MCU descriptor */
static struct MSIM_AVRBootloader bls;	/* Bootloader section */
static struct MSIM_AVRInt intr;		/* Interrupts and IRQs */
static struct MSIM_VCDDetails vcdd;	/* Details about registers to dump */

/* Statically allocated memory for MCU */
static unsigned char pm[PMSZ];			/* Program memory */
static unsigned char pmp[PM_PAGESZ];		/* Page buffer (PM) */
static unsigned char dm[DMSZ];			/* Data memory */
static unsigned char mpm[PMSZ];			/* Match points memory */

/* VCD dump options */
static char vcd_regs[VCD_REGS_MAX][16];		/* MCU registers to dump */
static unsigned int vcd_rn;			/* Number of registers */
static char print_regs;				/* Print available registers
						   which can be dumped */

/* MCU memory operations */
static struct MSIM_MemOp memops[MAX_MEMOPS];
static unsigned short memops_num;

static void print_usage(void);
static void print_config(const struct MSIM_AVR *m);
static void parse_dump(char *cmd);
static int parse_rsp_port(const char *opt);
static int parse_memop(char *opt);
static unsigned int parse_freq(const char *opt);
static int apply_memop(struct MSIM_AVR *m, struct MSIM_MemOp *mo);
static int set_fuse(struct MSIM_AVR *m, struct MSIM_MemOp *mo,
                    unsigned int fuse_n);
static int set_lock(struct MSIM_AVR *m, struct MSIM_MemOp *mo);

int main(int argc, char *argv[])
{
	extern char *optarg;
	FILE *fp;
	int c, rsp_port;
	char *partno, *luap;
	unsigned int i, j, regs;
	unsigned int dump_regs;		/* Number of registers for VCD dump */
	unsigned int freq;
	unsigned char trap_at_isr;	/* Flag to trap debugger if interrupt
					   generated */

	partno = luap = NULL;
	vcd_rn = print_regs = memops_num = 0;
	rsp_port = GDB_RSP_PORT;
	freq = 0;
	trap_at_isr = 0;

	/* Print welcome message */
	printf("INFO: AVRSim %s - Simulator for AVR microcontrollers,\n"
	       "INFO: part of MCUSim <http://www.mcusim.org>\n"
	       "INFO: --------------------------------------\n",
	       MSIM_VERSION);

	while ((c = getopt_long(argc, argv, CLI_OPTIONS,
	                        longopts, NULL)) != -1) {
		switch (c) {
		case 'p':		/* Required. AVR device. */
			partno = optarg;
			break;
		case 'U':		/* Required. Memory operation. */
			if (parse_memop(optarg)) {
				fprintf(stderr, "ERRO: Incorrect memory "
				        "operation specified!\n");
				print_usage();
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
			fprintf(stderr, "WARN: Option -%c requires "
			        "an operand\n", optopt);
			print_usage();
			return 1;
		case '?':		/* Unknown option */
			fprintf(stderr, "WARN: Unknown option: -%c\n",
			        optopt);
			print_usage();
			return 1;
		case DUMP_REGS_OPT:	/* Registers to dump into VCD file */
			parse_dump(optarg);
			break;
		case VERSION_OPT:	/* Print version and usage */
			print_usage();
			return 0;
		case GDB_RSP_PORT_OPT:	/* Set port for incoming connections
					   from GDB RSP */
			rsp_port = parse_rsp_port(optarg);
			break;
		case TRAP_AT_ISR_OPT:	/* Enter stopped mode when interrupt
					   occured */
			trap_at_isr = 1;
			break;
		}
	}

	if (partno != NULL && (!strcmp(partno, "?"))) {
		MSIM_PrintParts();
		return 0;
	}
	if (partno == NULL) {		/* MCU model is necessary! */
		print_usage();
		return 0;
	}

	/* Load Lua peripherals if it is required */
	if (luap)
		MSIM_LoadLuaPeripherals(luap);

	/* Preparing file for program memory */
	fp = NULL;
	for (i = 0; i < memops_num; i++) {
		if (memops[i].format == 'f' &&
		                !strcmp(memops[i].memtype, "flash")) {
			/* Try to open file to read program memory from */
			fp = fopen(memops[i].operand, "r");
			if (!fp) {
				fprintf(stderr, "ERRO: Failed to open file to "
				        "read a content of flash memory "
				        "from: %s\n", memops[i].operand);
				return 1;
			}
			/* Mark this operation as applied one */
			memops[i].format = -1;
		}
	}
	if (fp == NULL) {
		fprintf(stderr, "ERRO: It is necessary to fill program "
		        "memory, i.e. do not forget to "
		        "-U flash:w:<filename>!\n");
		return 1;
	}

	/* Initialize MCU as one of AVR models */
	mcu.bls = &bls;
	mcu.pmp = pmp;
	mcu.intr = &intr;
	mcu.vcdd = &vcdd;
	intr.trap_at_isr = trap_at_isr;
	if (MSIM_InitAVR(&mcu, partno, pm, PMSZ, dm, DMSZ, mpm, fp)) {
		fprintf(stderr, "ERRO: AVR %s cannot be initialized!\n",
		        partno);
		return 1;
	}
	fclose(fp);

	/* Do we have to print available registers only? */
	regs = sizeof mcu.vcdd->regs/sizeof mcu.vcdd->regs[0];
	if (print_regs) {
		for (i = 0; i < regs; i++) {
			/* Print first N registers only */
			if (mcu.vcdd->regs[i].name[0] == 0)
				break;
			if (mcu.vcdd->regs[i].off < 0)
				continue;
			printf("INFO: %s (0x%2lX)\n",
			       mcu.vcdd->regs[i].name, mcu.vcdd->regs[i].off);
		}
		return 0;
	}

	/* Apply memory modifications */
	for (i = 0; i < memops_num; i++)
		if (apply_memop(&mcu, &memops[i])) {
			fprintf(stderr, "ERRO: Memory modification failed: "
			        "memtype=%s, op=%c, val=%s\n",
			        &memops[i].memtype[0], memops[i].operation,
			        &memops[i].operand[0]);
			return 1;
		}

	/* Select registers to be dumped */
	dump_regs = 0;
	for (i = 0; i < vcd_rn; i++) {
		for (j = 0; j < regs; j++) {
			char *bit;
			size_t len = strlen(mcu.vcdd->regs[j].name);
			int bitn, cr, bit_cr;

			if (!len)
				continue;
			cr = strncmp(mcu.vcdd->regs[j].name, vcd_regs[i], len);
			if (cr)
				continue;

			/* Do we have a bit index suffix? */
			bit = len < sizeof vcd_regs[0]/sizeof vcd_regs[0][0]
			      ? &vcd_regs[i][len] : NULL;
			bit_cr = sscanf(bit, "%d", &bitn);
			if (bit_cr != 1)
				bitn = -1;

			/* Set index of a register to be dumped */
			mcu.vcdd->bit[dump_regs].regi = (short)j;
			mcu.vcdd->bit[dump_regs++].n = (short)bitn;
			break;
		}
	}

	/* Try to set required frequency */
	if (freq > mcu.freq)
		fprintf(stderr, "WARN: Frequency %u.%u kHz is above maximum "
		        "possible %lu.%lu kHz for selected clock source\n",
		        freq/1000, freq%1000, mcu.freq/1000, mcu.freq%1000);
	else if (freq > 0)
		mcu.freq = freq;

	/* Print MCU configuration */
	print_config(&mcu);
	printf("INFO: Listening for incoming GDB connections "
	       "at localhost:%d...\n", rsp_port);

	/* Prepare and run AVR simulation */
	MSIM_RSPInit(&mcu, rsp_port);
	MSIM_SimulateAVR(&mcu, 0, mcu.flashend+1);
	MSIM_CleanLuaPeripherals();
	MSIM_RSPClose();
	return 0;
}

static void print_usage(void)
{
	/* Print usage and options */
	printf("INFO: Usage: avrsim [options]\n"
	       "INFO: Options:\n"
	       "INFO:   -p <partno|?>              Specify AVR device "
	       "(required).\n"
	       "INFO:   -U <memtype>:w:<filename|value>[:<format>]\n"
	       "INFO:                              Memory operation "
	       "specification (required).\n"
	       "INFO:   -r <filename>              Specify text file with "
	       "simulated modules written in Lua.\n"
	       "INFO:   --dump-regs=<reg0,reg1,...,regN|?>\n"
	       "INFO:                              Dump specified registers "
	       "into VCD file.\n"
	       "INFO:   --version                  Print this message.\n");
	printf("INFO:   -P <port>, --rsp-port=<port>\n"
	       "INFO:                              Set port to listen to the "
	       "incoming connections from GDB RSP.\n"
	       "INFO:   -f <frequency>             MCU frequency, in Hz.\n"
	       "INFO:   --trap-at-isr              Enter stopped mode when "
	       "interrupt occured.\n");

	/* Print examples */
	printf("INFO: Examples:\n"
	       "INFO:   avrsim -p m328p -U flash:w:./dhtc.hex -U "
	       "hfuse:w:0x57:h -r ./lua-modules --dump-regs=PORTB,PORTC\n"
	       "INFO:   avrsim -p m8a -U flash:w:./dhtc.hex -r ./lua-modules "
	       "-f 1000000\n\n");
}

static void print_config(const struct MSIM_AVR *m)
{
	/*
	 * AVR memory is organized as array of bytes in the simulator, but
	 * it's natural to measure program memory in words because
	 * all AVR instructions are 16- or 32-bits wide.
	 *
	 * It's why all program memory addresses are divided by two before
	 * printing.
	 */
	printf("INFO: Model: %s\n", m->name);
	printf("INFO: Signature: 0x%X%2X%2X\n",
	       m->signature[2], m->signature[1], m->signature[0]);
	printf("INFO: Clock frequency: %lu.%lu kHz\n",
	       m->freq/1000, m->freq%1000);
	printf("INFO: Program memory: 0x%lX-0x%lX\n",
	       m->flashstart/2, m->flashend/2);
	printf("INFO: Bootloader section: 0x%lX-0x%lX\n",
	       m->bls->start/2, m->bls->end/2);
	printf("INFO: Data memory: 0x%lX-0x%lX\n", m->ramstart, m->ramend);
	printf("INFO: EEPROM: 0x%X-0x%X\n", m->e2start, m->e2end);
	printf("INFO: PC: 0x%lX\n", m->pc/2);
	printf("INFO: Reset address: 0x%lX\n", m->intr->reset_pc/2);
	printf("INFO: Interrupt vectors: 0x%lX\n", m->intr->ivt/2);
#ifndef ULLONG_MAX
	printf("INFO: Maximum rises and falls of CLK_IO to dump: %lu\n",
	       ULONG_MAX);
#else
	printf("INFO: Maximum rises and falls of CLK_IO to dump: %llu\n",
	       ULLONG_MAX);
#endif
}

static void parse_dump(char *cmd)
{
	if (cmd == NULL)
		return;
	if (!strcmp(cmd, "?")) {
		/*
		 * User asked to print all registers which can be included
		 * into VCD dump. Set flag and return, no more.
		 */
		print_regs = 1;
		return;
	} else {
		int c;
		char *p;
		/* We've got a register name to parse. Let's do it! */
		for (c = 0; cmd[c] != 0; c++)
			if (cmd[c] == ',')
				cmd[c] = ' ';

		p = cmd;
		while (1) {
			int ret;
			ret = sscanf(p, "%15s", &vcd_regs[vcd_rn][0]);
			if (ret == EOF)
				break;
			if (ret == 1) {
				vcd_rn++;
				p = strstr(p, " ");
				if (p == NULL)
					break;
				p++;
				continue;
			}
			fprintf(stderr, "ERRO: Failed to parse list of "
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
	if (!(rsp_port > 1024 && rsp_port < 65536)) {
		fprintf(stderr, "WARN: GDB RSP port should be within "
		        "(1024, 65536) range! Default port will be used.\n");
		rsp_port = GDB_RSP_PORT;
	}
	return rsp_port;
}

static unsigned int parse_freq(const char *opt)
{
	int freq;

	freq = atoi(opt);
	if (freq <= 0) {
		fprintf(stderr, "WARN: Frequency should be positive!\n");
		return 0;
	}
	return (unsigned int)freq;
}

static int parse_memop(char *opt)
{
	int c, r;

	for (c = 0; opt[c] != 0; c++)
		if (opt[c] == ':')
			opt[c] = ' ';
	r = sscanf(opt, "%16s %1c %4096s %1c",
	           &memops[memops_num].memtype[0],
	           &memops[memops_num].operation,
	           &memops[memops_num].operand[0],
	           &memops[memops_num].format);

	if (r == EOF || r < 3 || r > 4) /* Something went wrong */
		return 1;
	if (r == 3)			/* 'File' format is default one */
		memops[memops_num].format = 'f';
	memops_num++;
	return 0;
}

static int apply_memop(struct MSIM_AVR *m, struct MSIM_MemOp *mo)
{
	if (mo->format < 0)	/* Operation is applied already, skipping */
		return 0;

	if (!strcmp(mo->memtype, "efuse")) {
		return set_fuse(m, mo, FUSE_EXT);
	} else if (!strcmp(mo->memtype, "hfuse")) {
		return set_fuse(m, mo, FUSE_HIGH);
	} else if (!strcmp(mo->memtype, "lfuse")) {
		return set_fuse(m, mo, FUSE_LOW);
	} else if (!strcmp(mo->memtype, "lock")) {
		return set_lock(m, mo);
	} else {
		return 1;
	}
	return 0;
}

static int set_fuse(struct MSIM_AVR *m, struct MSIM_MemOp *mo,
                    unsigned int fuse_n)
{
	unsigned int fusev;

	if (m->set_fusef == NULL) {
		fprintf(stderr, "WARN: Cannot modify fuse, MCU-specific "
		        "function is not available\n");
		return 0;	/* No function to set fuse */
	}
	if (mo->format != 'h') {
		fprintf(stderr, "WARN: Failed to modify fuse, expected "
		        "format is 'h'\n");
		return 1;
	}

	if (sscanf(mo->operand, "0x%2X", &fusev) != 1) {
		fprintf(stderr, "ERRO: Failed to parse fuse value from %s!\n",
		        mo->operand);
		return 1;
	}
	m->set_fusef(m, fuse_n, (unsigned char)fusev);
	mo->format = -1;	/* Operation is applied correctly */
	return 0;
}

static int set_lock(struct MSIM_AVR *m, struct MSIM_MemOp *mo)
{
	unsigned int lockv;

	if (m->set_lockf == NULL) {
		fprintf(stderr, "WARN: Cannot modify lock bits, MCU-specific "
		        "function is not available\n");
		return 0;	/* No function to set lock bits */
	}
	if (mo->format != 'h') {
		fprintf(stderr, "WARN: Failed to modify lock byte, expected "
		        "format is 'h'\n");
		return 1;
	}

	if (sscanf(mo->operand, "0x%2X", &lockv) != 1) {
		fprintf(stderr, "ERRO: Failed to parse lock value from %s!\n",
		        mo->operand);
		return 1;
	}
	m->set_lockf(m, (unsigned char)lockv);
	mo->format = -1;	/* Operation is applied correctly */
	return 0;
}
