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
 * There are some declarations and functions to work with a pseudo-terminal
 * (in POSIX terms) within the simulated microcontrollers.
 *
 * These functions are generally useful to let an operating system to interact
 * with a simulated MCU using "serial port" backed by a pseudo-terminal.
 */
#if defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY)

/* glibc (starting from 2.2) requires _XOPEN_SOURCE >= 600 to expose
 * definitions for POSIX.1-2001 base specification plus XSI extension
 * and C99 definitions. */
#define _XOPEN_SOURCE		600

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"

/* Thread function to read data from pty and populate a buffer. */
static void *read_from_pty(void *arg);

int MSIM_PTY_Open(struct MSIM_PTY *pty)
{
	int32_t masterfd = -1;
	int32_t slavefd = -1;
	int pty_err = 0;
	char *slavedevice;
	char log[1024];
	pthread_attr_t attr;

	masterfd = posix_openpt(O_RDWR|O_NOCTTY);
	pty->master_fd = masterfd;

	if ((masterfd == -1) || (grantpt(masterfd) == -1) ||
	                (unlockpt(masterfd) == -1) ||
	                ((slavedevice = ptsname(masterfd)) == NULL)) {
		pty_err = 1;
		MSIM_LOG_ERROR("failed to setup pty master device");
	} else {
		snprintf(log, sizeof log, "slave pty device is: %s",
		         slavedevice);
		MSIM_LOG_INFO(log);

		slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
		pty->slave_fd = slavefd;
		snprintf(pty->slave_name, sizeof pty->slave_name, "%s",
		         slavedevice);
		if (slavefd < 0) {
			pty_err = 1;
			snprintf(log, sizeof log, "cannot open pty slave "
			         "device: %s", slavedevice);
			MSIM_LOG_ERROR(log);
		}
	}

	/* Create a thread to read from pty */
	if (pty_err == 0) {
		/* Initialize a basic mutex */
		pthread_mutex_init(&pty->read_thr.mutex, NULL);
		/* Configure thread attributes */
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		/* Create and start a thread to read */
		pthread_create(&pty->read_thr.thread, &attr, read_from_pty,
		               (void *)pty);
		pthread_attr_destroy(&attr);
	}

	return pty_err;
}

int MSIM_PTY_Close(struct MSIM_PTY *pty)
{
	if (pty->slave_fd >= 0) {
		close(pty->slave_fd);
	}
	if (pty->master_fd >= 0) {
		close(pty->master_fd);
	}
	return 0;
}

int MSIM_PTY_Write(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len)
{
	return (int)write(pty->master_fd, buf, len);
}

int MSIM_PTY_Read(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len)
{
	struct MSIM_PTY_Thread *t = &pty->read_thr;
	uint32_t l;

	/* Lock the basic thread mutex */
	pthread_mutex_lock(&t->mutex);

	l = len < t->len ? len : t->len;
	for (uint32_t i = 0; i < l; i++) {
		buf[i] = t->buf[i];
	}
	for (uint32_t i = 0; i < (t->len-l); i++) {
		t->buf[i] = t->buf[i+l];
	}
	t->len -= l;

	/* Unlock the basic thread mutex */
	pthread_mutex_unlock(&t->mutex);

	return (int)l;
}

static void *read_from_pty(void *arg)
{
	struct MSIM_PTY *pty = (struct MSIM_PTY *)arg;
	struct MSIM_PTY_Thread *t = &pty->read_thr;
	uint8_t stop = 0;
	uint8_t buf[16];
	int res = -1;

	while (stop == 0) {
		res = (int)read(pty->master_fd, buf, sizeof buf);

		/* Lock the basic thread mutex */
		pthread_mutex_lock(&t->mutex);
		/* Should thread be terminated? */
		if (t->stop_thr > 0) {
			stop = 1;
		}

		/* Append data to the thread buffer */
		if (res > 0) {
			if ((t->len+(uint32_t)res) > MSIM_PTY_BUFSIZE) {
				/* Not enough space in the thread buffer */
				MSIM_LOG_ERROR("not enough space in the "
				               "thread buffer");
			} else {
				for (uint32_t i = 0; i < (uint32_t)res; i++) {
					t->buf[t->len+i] = buf[i];
				}
				t->len += (uint32_t)res;
			}
		}
		/* Unlock the basic thread mutex */
		pthread_mutex_unlock(&t->mutex);
	}
	pthread_exit(NULL);
}

#endif /* defined(MSIM_POSIX) && defined(MSIM_POSIX_PTY) */
