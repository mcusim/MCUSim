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

/*
 * There are some declarations and functions to work with a pseudo-terminal
 * (in POSIX terms) within the simulated microcontrollers.
 *
 * These functions are generally useful to let an operating system to interact
 * with a simulated MCU using "serial port" backed by a pseudo-terminal.
 */
#if defined(WITH_POSIX) && defined(WITH_POSIX_PTY)

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 600
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include "mcusim/mcusim.h"

/* Thread function to read data from pty and populate a buffer. */
static void *read_from_pty(void *arg);

int
MSIM_PTY_Open(struct MSIM_PTY *pty)
{
	int32_t masterfd = -1;
	int32_t slavefd = -1;
	int pty_err = 0;
	char *slavedevice;
	char log[1024];
	pthread_attr_t attr;

	masterfd = posix_openpt(O_RDWR|O_NOCTTY);
	pty->master_fd = masterfd;
	if (masterfd == -1) {
		pty_err = 1;
		MSIM_LOG_ERROR("failed to connect pseudo-terminal master "
		               "device and file descriptor");
	}

	if (pty_err == 0) {
		pty_err = grantpt(masterfd);
		if (pty_err == -1) {
			MSIM_LOG_ERROR("failed to change the mode and "
			               "ownership of the slave pseudo-"
			               "terminal device");
		}
	}
	if (pty_err == 0) {
		pty_err = unlockpt(masterfd);
		if (pty_err == -1) {
			MSIM_LOG_ERROR("failed to unlock the slave pseudo-"
			               "terminal device");
		}
	}
	if (pty_err == 0) {
		slavedevice = ptsname(masterfd);
		if (slavedevice == NULL) {
			pty_err = 1;
			MSIM_LOG_ERROR("failed to obtain the name of the "
			               "slave pseudo-terminal device");
		}
	}

	if (pty_err == 0) {
		slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
		pty->slave_fd = slavefd;
		snprintf(pty->slave_name, sizeof pty->slave_name, "%s",
		         slavedevice);
		if (slavefd < 0) {
			pty_err = 1;
			snprintf(log, sizeof log, "cannot open pty slave "
			         "device: %s", slavedevice);
			MSIM_LOG_ERROR(log);
		} else {
			/* Slave device was opened correctly, do nothing */
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

	/* Close master and slave devices in case of error */
	if (pty_err != 0) {
		if (pty->slave_fd >= 0) {
			close(pty->slave_fd);
		}
		if (pty->master_fd >= 0) {
			close(pty->master_fd);
		}
	}
	return pty_err;
}

int
MSIM_PTY_Close(struct MSIM_PTY *pty)
{
	struct MSIM_PTY_Thread *t = &pty->read_thr;
	char log[1024];
	void *status;
	int rc;

	/* Mark the reading thread to be stopped */
	pthread_mutex_lock(&t->mutex);
	t->stop_thr = 1;
	pthread_mutex_unlock(&t->mutex);

	/* Close PTY files */
	if (pty->slave_fd >= 0) {
		close(pty->slave_fd);
	}
	if (pty->master_fd >= 0) {
		close(pty->master_fd);
	}

	/* Wait for the reading thread and clear its mutex */
	rc = pthread_join(t->thread, &status);
	if (rc != 0) {
		snprintf(log, sizeof log, "failed to join a thread (may not "
		         "be created), return code: %d", rc);
		MSIM_LOG_WARN(log);
	}
	pthread_mutex_destroy(&t->mutex);

	return 0;
}

int
MSIM_PTY_Write(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len)
{
	return (int)write(pty->master_fd, buf, len);
}

int
MSIM_PTY_Read(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len)
{
	struct MSIM_PTY_Thread *t = &pty->read_thr;
	uint32_t i, l;

	/* Lock the basic thread mutex */
	pthread_mutex_lock(&t->mutex);

	l = (len < (t->len)) ? len : t->len;
	for (i = 0; i < l; i++) {
		buf[i] = t->buf[i];
	}
	for (i = 0; i < (t->len-l); i++) {
		t->buf[i] = t->buf[i+l];
	}
	t->len -= l;

	/* Unlock the basic thread mutex */
	pthread_mutex_unlock(&t->mutex);

	return (int)l;
}

static void *
read_from_pty(void *arg)
{
	struct MSIM_PTY *pty = (struct MSIM_PTY *)arg;
	struct MSIM_PTY_Thread *t = &pty->read_thr;
	uint8_t stop = 0;
	uint8_t buf[1024];
	int res = -1;

	while (stop == 0U) {
		res = (int)read(pty->master_fd, buf, sizeof buf);

		/* Lock the basic thread mutex */
		pthread_mutex_lock(&t->mutex);
		/* Check the thread termination flag */
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

#endif /* defined(WITH_POSIX) && defined(WITH_POSIX_PTY) */
