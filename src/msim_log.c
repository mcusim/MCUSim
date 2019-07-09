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

/* MCUSim logging functions */
#include <stdint.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"

static enum MSIM_LOG_Level log_level = MSIM_LOG_LVLINFO;

void
MSIM_LOG_Log(enum MSIM_LOG_Level lvl, const char *lvlmsg,
             const char *file, uint32_t line, const char *msg)
{
	if (log_level >= lvl) {
		printf("%s %s[%d]: %s\n", lvlmsg, file, line, msg);
	}
}

void
MSIM_LOG_SetLevel(enum MSIM_LOG_Level level)
{
	log_level = level;
}

enum MSIM_LOG_Level
MSIM_LOG_GetLevel(void) {
	return log_level;
}

void
MSIM_LOG_PrintMarkers(void)
{
	MSIM_LOG_INFO("Markers: (--) informational, (DD) debug, "
	              "(WW) warning, (EE) error, (!!) fatal.");
}
