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
 *
 * There are some declarations and functions to pair a master pseudo-terminal
 * device (in POSIX terms) with USART within a simulated AVR microcontroller.
 */
#ifdef MSIM_POSIX_PTY

/* glibc (starting from 2.2) requires _XOPEN_SOURCE >= 600 to expose
 * definitions for POSIX.1-2001 base specification plus XSI extension
 * and C99 definitions. */
#define _XOPEN_SOURCE		600

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "mcusim/log.h"
#include "mcusim/mcusim.h"
#include "mcusim/avr/sim/pty.h"

int MSIM_AVR_PTYOpen(struct MSIM_AVR *mcu)
{
	int32_t masterfd = -1;
	int32_t slavefd = -1;
	char *slavedevice;
	int pty_err = 0;

	masterfd = posix_openpt(O_RDWR|O_NOCTTY);
	mcu->pty->master_fd = masterfd;

	if ((masterfd == -1) || (grantpt(masterfd) == -1) ||
	                (unlockpt(masterfd) == -1) ||
	                ((slavedevice = ptsname(masterfd)) == NULL)) {
		pty_err = 1;
		MSIM_LOG_ERROR("failed to setup pty master device");
	} else {
#ifdef DEBUG
		snprintf(mcu->log, sizeof mcu->log, "slave pty device "
		         "is: %s", slavedevice);
		MSIM_LOG_DEBUG(mcu->log);
#endif
		slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
		mcu->pty->slave_fd = slavefd;
		snprintf(mcu->pty->slave_name, sizeof mcu->pty->slave_name,
		         "%s", slavedevice);
		if (slavefd < 0) {
			pty_err = 1;
			snprintf(mcu->log, sizeof mcu->log, "cannot "
			         "open pty slave device: %s", slavedevice);
			MSIM_LOG_ERROR(mcu->log);
		}
	}
	return pty_err;
}

int MSIM_AVR_PTYClose(struct MSIM_AVR *mcu)
{
	if (mcu->pty->slave_fd >= 0) {
		close(mcu->pty->slave_fd);
	}
	if (mcu->pty->master_fd >= 0) {
		close(mcu->pty->master_fd);
	}
	return 0;
}

ssize_t MSIM_AVR_PTYWrite(struct MSIM_AVR *mcu, uint8_t *buf, uint32_t len)
{
	return write(mcu->pty->master_fd, buf, len);
}

ssize_t MSIM_AVR_PTYRead(struct MSIM_AVR *mcu, uint8_t *buf, uint32_t len)
{
	return read(mcu->pty->master_fd, buf, len);
}

#endif /* MSIM_POSIX_PTY */

