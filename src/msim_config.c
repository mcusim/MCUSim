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

/* Functions to parse and save MCUSim configuration. */
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/private/macro.h"

/* Configuration file from the current working directory */
#define CFG_FILE		"mcusim.conf"

/* Compare string with a string literal */
#define CMPL(s, l, len) (strncmp((s), (l), ARRSZ(l) < (len) ? ARRSZ(l) : (len)))

static int	read_file(MSIM_CFG *cfg, const char *cf);
static int	read_lines(MSIM_CFG *cfg, char *buf, uint32_t len, FILE *file,
                           const char *cf);
static int	read_line(MSIM_CFG *cfg, char *parm, char *val, uint32_t plen,
                          uint32_t vlen);
static void	parse_bool(char *buf, uint32_t len, uint8_t *val);

/*
 * Read a configuration file.
 *
 * If it's not possible to obtain configuration from the given one,
 * there will be attempts to load it from the current working directory or
 * where it's supposed to be installed during compilation.
 */
int
MSIM_CFG_Read(MSIM_CFG *cfg, const char *cf)
{
	const uint32_t len = 64*1024;
	char log[len];
	int rc = 0;

	/* Try to load a configuration file */
	rc = read_file(cfg, cf);

	if (rc != 0) {
		if (cf != NULL) {
			snprintf(log, len, "failed to open config: %s", cf);
			MSIM_LOG_ERROR(log);
		}

		/* Try to load from the current working directory */
		rc = read_file(cfg, CFG_FILE);

		if (rc != 0) {
			/* Try to load a system-wide file at least */
			rc = read_file(cfg, MSIM_CFG_FILE);

			if (rc != 0) {
				MSIM_LOG_ERROR("can't load any config file");
				rc = 1;
			} else {
				MSIM_LOG_INFO("using config: " MSIM_CFG_FILE);
			}
		} else {
			MSIM_LOG_INFO("using config: " CFG_FILE);
		}
	} else {
		snprintf(log, len, "using config: %s", cf);
		MSIM_LOG_INFO(log);
	}

	return rc;
}

static int
read_file(MSIM_CFG *cfg, const char *cf)
{
	FILE *f = fopen(cf, "r");
	const uint32_t buflen = 4096;
	char buf[buflen];
	int rc = 0;

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

		rc = read_lines(cfg, buf, buflen, f, cf);
	}

	if (f != NULL) {
		fclose(f);
	}

	return rc;
}

int
MSIM_CFG_PrintVersion(void)
{
	char buf[1024];

	snprintf(buf, 1024, "MCUSim %s, an XSPICE library with "
	         "microcontrollers.", MSIM_VERSION);
	MSIM_LOG_INFO(buf);
	MSIM_LOG_INFO("Copyright (c) 2017-2019 MCUSim Developers.");
	MSIM_LOG_INFO("");
	MSIM_LOG_INFO("MCUSim comes with ABSOLUTELY NO WARRANTY.");
	MSIM_LOG_INFO("This is free software, and you are welcome to redistribute it");
	MSIM_LOG_INFO("under under the terms of the GNU General Public License as");
	MSIM_LOG_INFO("published by the Free Software Foundation, either version 3");
	MSIM_LOG_INFO("of the License, or (at your option) any later version.");
	MSIM_LOG_INFO("");
	snprintf(buf, 1024, "Build Date: %s %s (UTC)",
	         MSIM_BUILD_DATE, MSIM_BUILD_TIME);
	MSIM_LOG_INFO(buf);
	MSIM_LOG_INFO("Documentation: https://trac.mcusim.org");
	MSIM_LOG_INFO("Bugs: https://trac.mcusim.org/newticket");
	MSIM_LOG_INFO("");

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
		rc = sscanf(buf, "%4095s %4095s", parm, val);
		if (rc != 2) {
			snprintf(buf, buflen, "incorrect format of line #%"
			         PRIu32 " from %s", line, filename);
			MSIM_LOG_DEBUG(buf);
			continue;
		} else {
			/* Line is correct */
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
		cmp_rc = sscanf(val, "%4095s", buf);
		if (cmp_rc == 1) {
			parse_bool(buf, buflen, &cfg->firmware_test);
		} else {
			rc = 2;
		}
	} else if (CMPL(parm, "reset_flash", plen) == 0) {
		cmp_rc = sscanf(val, "%4095s", buf);
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
		cmp_rc = sscanf(val, "%4095s", buf);
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
