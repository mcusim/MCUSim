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
#ifndef MSIM_CFG_H_
#define MSIM_CFG_H_ 1

#include <stdint.h>
#include "mcusim/avr/sim/vcd.h"
#include "mcusim/avr/sim/lua.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Full path to the default configuration file of MCUSim. */
#define MSIM_CFG_FILE		"@CMAKE_INSTALL_PREFIX@/@MSIM_CONF_DIR@/mcusim.conf"

/* This is a basic structure to describe an MCUSim configuration.
 * See mcusim.conf or mcusim.conf.in for detailed description of the fields. */
struct MSIM_CFG {
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
};

int MSIM_CFG_Read(struct MSIM_CFG *cfg, const char *filename);
int MSIM_CFG_PrintVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_CFG_H_ */
