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
 * There are some declarations and functions to pair a master pseudo-terminal
 * device (in POSIX terms) with USART within a simulated AVR microcontroller.
 */
#ifndef MSIM_PTY_H_
#define MSIM_PTY_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <pthread.h>

/* Size of the buffers to read/write data from/to pty. */
#define MSIM_PTY_BUFSIZE	16384

/* Thread with buffer to read/write data from/to pty. */
typedef struct MSIM_PTY_Thread {
	pthread_mutex_t mutex;		/* Lock before accessing any fields */
	pthread_t thread;		/* Current thread handle */
	uint8_t stop_thr;		/* Flag to exit the thread */
	uint8_t buf[MSIM_PTY_BUFSIZE];	/* Buffer to read/write pty data */
	uint32_t len;			/* Length of the data in buffer */
} MSIM_PTY_Thread;

/* A single pseudo-terminal (with master and slave parts) and additional data
 * to handle a separate thread to read from the master part of pty. */
typedef struct MSIM_PTY {
	char slave_name[128];
	int32_t master_fd;
	int32_t slave_fd;
	struct MSIM_PTY_Thread read_thr;
} MSIM_PTY;

int MSIM_PTY_Open(struct MSIM_PTY *pty);
int MSIM_PTY_Close(struct MSIM_PTY *pty);
int MSIM_PTY_Write(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len);
int MSIM_PTY_Read(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_PTY_H_ */
