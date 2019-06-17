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
 *
 * Functions to parse and save MCUSim configuration.
 */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/private/macro.h"

/* Compare string with a string literal */
#define CMPL(s, l, len) (strncmp((s), (l),				\
                         ARR_LEN(l) < (len) ? ARR_LEN(l) : (len)))

static int
read_lines(struct MSIM_CFG *cfg, char *buf, uint32_t buflen,
           FILE *file, const char *filename);
static int
read_line(struct MSIM_CFG *cfg, char *parm, char *val,
          uint32_t plen, uint32_t vlen);

static void
parse_bool(char *buf, uint32_t len, uint8_t *val);

int
MSIM_CFG_Read(struct MSIM_CFG *cfg, const char *filename)
{
	char buf[4096];
	uint32_t buflen = sizeof buf/sizeof buf[0];
	int rc = 0;

#ifdef DEBUG
	/* Adjust logging level in the debug version. */
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
#endif

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		rc = 1;
	} else {
		cfg->lua_models_num = 0;
		cfg->dump_regs_num = 0;
		cfg->has_lockbits = 0;
		cfg->has_efuse = 0;
		cfg->has_hfuse = 0;
		cfg->has_lfuse = 0;
		cfg->has_firmware_file = 0;
		cfg->firmware_test = 0;
		cfg->reset_flash = 1;
		rc = read_lines(cfg, buf, buflen, f, filename);
	}

	if (f != NULL) {
		fclose(f);
	}
	return rc;
}

int
MSIM_CFG_PrintVersion(void)
{
#ifndef DEBUG
	printf("MCU Simulator %s\nBuild Date: %s %s (UTC)\n",
	       MSIM_VERSION, MSIM_BUILD_DATE, MSIM_BUILD_TIME);
#else
	/* Adjust logging level in the debug version. */
	MSIM_LOG_SetLevel(MSIM_LOG_LVLDEBUG);
	printf("MCU Simulator %s (dbg)\nBuild Date: %s %s (UTC)\n",
	       MSIM_VERSION, MSIM_BUILD_DATE, MSIM_BUILD_TIME);
#endif
	MSIM_LOG_INFO("Copyright 2017-2019 The MCUSim Project");
	MSIM_LOG_INFO("Documentation: https://trac.mcusim.org");
	MSIM_LOG_INFO("Bugs: https://trac.mcusim.org/newticket");
	MSIM_LOG_PrintMarkers();

	return 0;
}

static int
read_lines(struct MSIM_CFG *cfg, char *buf, uint32_t buflen,
           FILE *file, const char *filename)
{
	const uint32_t len = 4096;
	uint32_t line = 1U;
	char parm[len];
	char val[len];
	char *str;
	int rc = 0;

	while (1) {
		str = fgets(buf, (int) buflen, file);
		if (str == NULL) {
			if (feof(file) == 0) {
				snprintf(buf, buflen, "cannot read line #%"
				         PRIu32 " from %s", line, filename);
				MSIM_LOG_ERROR(buf);
				rc = 1;
			} else {
				rc = 0;
			}
			break;
		} else {
			line++;
		}

		/* Skip comment line in the configuration file. */
		if ((buf[0] == '#') || (buf[0] == '\n') || (buf[0] == '\r')) {
			continue;
		}
		/* Try to parse a configuration line. */
		rc = sscanf(buf, "%4096s %4096s", parm, val);
		if (rc != 2) {
			snprintf(buf, buflen, "incorrect format of line #%"
			         PRIu32 " from %s", line, filename);
			MSIM_LOG_DEBUG(buf);
			continue;
		} else { /* Line is correct. */
			rc = read_line(cfg, parm, val, len, len);
			if (rc != 0) {
				snprintf(buf, buflen, "cannot read option at "
				         "line #%" PRIu32 " from %s",
				         line, filename);
				MSIM_LOG_ERROR(buf);
				break;
			}
		}
	}
	return rc;
}

static int
read_line(struct MSIM_CFG *cfg, char *parm, char *val,
          uint32_t plen, uint32_t vlen)
{
	const uint32_t buflen = 4096;
	char buf[buflen];
	int rc = 0;
	int cmp_rc;

	if (CMPL(parm, "mcu", plen) == 0) {
		cmp_rc = sscanf(val, "%64s", &cfg->mcu[0]);
		if (cmp_rc != 1) {
			rc = 2;
		}
	} else if (CMPL(parm, "mcu_freq", plen) == 0) {
		cmp_rc = sscanf(val, "%" SCNu64, &cfg->mcu_freq);
		if (cmp_rc != 1) {
			rc = 2;
		}
	} else if (CMPL(parm, "mcu_lockbits", plen) == 0) {
		cmp_rc = sscanf(val, "0x%" SCNx8, &cfg->mcu_lockbits);
		if (cmp_rc == 1) {
			cfg->has_lockbits = 1;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "mcu_efuse", plen) == 0) {
		cmp_rc = sscanf(val, "0x%" SCNx8, &cfg->mcu_efuse);
		if (cmp_rc == 1) {
			cfg->has_efuse = 1;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "mcu_hfuse", plen) == 0) {
		cmp_rc = sscanf(val, "0x%" SCNx8, &cfg->mcu_hfuse);
		if (cmp_rc == 1) {
			cfg->has_hfuse = 1;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "mcu_lfuse", plen) == 0) {
		cmp_rc = sscanf(val, "0x%" SCNx8, &cfg->mcu_lfuse);
		if (cmp_rc == 1) {
			cfg->has_lfuse = 1;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "firmware_file", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s", &cfg->firmware_file[0]);
		if (cmp_rc == 1) {
			cfg->has_firmware_file = 1;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "firmware_test", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s", buf);
		if (cmp_rc == 1) {
			parse_bool(buf, buflen, &cfg->firmware_test);
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "reset_flash", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s", buf);
		if (cmp_rc == 1) {
			parse_bool(buf, buflen, &cfg->reset_flash);
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "lua_model", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s",
		                &cfg->lua_models[cfg->lua_models_num][0]);
		if (cmp_rc == 1) {
			cfg->lua_models_num++;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "vcd_file", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s", &cfg->vcd_file[0]);
		if (cmp_rc != 1) {
			rc = 2;
		}
	} else if (CMPL(parm, "dump_reg", plen) == 0) {
		cmp_rc = sscanf(val, "%16s",
		                &cfg->dump_regs[cfg->dump_regs_num][0]);
		if (cmp_rc == 1) {
			cfg->dump_regs_num++;
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "rsp_port", plen) == 0) {
		uint32_t port;
		cmp_rc = sscanf(val, "%" SCNu32, &port);
		if (cmp_rc == 1) {
			if ((port <= 1024) || (port >= 65536)) {
				MSIM_LOG_WARN("GDB RSP port should be in "
				              "[1025, 65535]");
			} else {
				cfg->rsp_port = port;
			}
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "trap_at_isr", plen) == 0) {
		cmp_rc = sscanf(val, "%4096s", buf);
		if (cmp_rc == 1) {
			parse_bool(buf, buflen, &cfg->trap_at_isr);
		} else {
			rc = 2;
		}
	} else {
		rc = 1;
		snprintf(buf, buflen, "unknown option %s", parm);
		MSIM_LOG_ERROR(buf);
	}

	return rc;
}

static void
parse_bool(char *buf, uint32_t len, uint8_t *val)
{
	if (CMPL(buf, "no", len) == 0) {
		*val = 0;
	}
	if (CMPL(buf, "yes", len) == 0) {
		*val = 1;
	}
}
