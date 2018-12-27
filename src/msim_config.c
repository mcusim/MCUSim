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
#include "mcusim/config.h"
#include "mcusim/log.h"

int MSIM_CFG_Read(struct MSIM_CFG *cfg, const uint8_t *filename)
{
	char buf[4096];
	char parm[4096];
	char val[4096];
	char *str;
	uint32_t buflen = sizeof buf/sizeof buf[0];
	uint32_t line = 1U;
	int rc = 0;

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		snprintf(buf, buflen, "failed to load %s configuration file",
		         filename);
		MSIM_LOG_ERROR(buf);
		rc = 1;
	} else {
		while (1) {
			str = fgets(buf, buflen, f);
			if (str == NULL) {
				if (feof(f) == 0) {
					snprintf(buf, buflen, "cannot read "
					         "line #%" PRIu32 " from %s",
					         line, filename);
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
			if (buf[0] == '#') {
				continue;
			}
			/* Try to parse a configuration line. */
			rc = sscanf(buf, "%4096s %4096s", &parm, &val);
			if (rc != 2) {
#ifdef DEBUG
				snprintf(buf, buflen, "incorrect format of "
				         "line #%" PRIu32 " from %s",
				         line, filename);
				MSIM_LOG_DEBUG(buf);
#endif
				rc = 0;
				continue;
			} else {
				/* Line is correct. */
			}
		}
	}

	if (f != NULL) {
		fclose(f);
	}
	return rc;
}
