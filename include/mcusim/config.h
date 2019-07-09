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
#ifndef MSIM_CFG_H_
#define MSIM_CFG_H_ 1

#include <stdint.h>
#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/lua.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Full path to the installed configuration file of MCUSim */
#define MSIM_CFG_FILE	"@CMAKE_INSTALL_PREFIX@/@MSIM_CONF_DIR@/mcusim.conf"

/*
 * This is a basic structure to describe an MCUSim configuration.
 * See mcusim.conf or mcusim.conf.in for detailed description of the fields.
 */
typedef struct MSIM_CFG {
	char mcu[64];
	uint64_t mcu_freq;

	uint8_t mcu_lockbits;
	uint8_t has_lockbits;
	uint8_t mcu_efuse;
	uint8_t has_efuse;
	uint8_t mcu_hfuse;
	uint8_t has_hfuse;
	uint8_t mcu_lfuse;
	uint8_t has_lfuse;

	char firmware_file[4096];
	uint8_t has_firmware_file;

	uint8_t reset_flash;
	uint8_t firmware_test;
	uint8_t trap_at_isr;
	uint32_t rsp_port;

	char lua_models[MSIM_AVR_LUAMODELS][4096];
	uint32_t lua_models_num;

	char vcd_file[4096];
	char dump_regs[MSIM_AVR_VCD_REGS][16];
	uint32_t dump_regs_num;
} MSIM_CFG;

int	MSIM_CFG_Read(MSIM_CFG *cfg, const char *f);
int	MSIM_CFG_PrintVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_CFG_H_ */
