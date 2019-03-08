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
struct MSIM_PTY_Thread {
	pthread_mutex_t mutex;		/* Lock before accessing any fields */
	pthread_t thread;		/* Current thread handle */
	uint8_t stop_thr;		/* Flag to exit the thread */
	uint8_t buf[MSIM_PTY_BUFSIZE];	/* Buffer to read/write pty data */
	uint32_t len;			/* Length of the data in buffer */
};

/* A single pseudo-terminal (with master and slave parts) and additional data
 * to handle a separate thread to read from the master part of pty. */
struct MSIM_PTY {
	char slave_name[128];
	int32_t master_fd;
	int32_t slave_fd;
	struct MSIM_PTY_Thread read_thr;
};

#if defined(WITH_POSIX) && defined(WITH_POSIX_PTY)

int MSIM_PTY_Open(struct MSIM_PTY *pty);
int MSIM_PTY_Close(struct MSIM_PTY *pty);
int MSIM_PTY_Write(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len);
int MSIM_PTY_Read(struct MSIM_PTY *pty, uint8_t *buf, uint32_t len);

#else

#define MSIM_PTY_Open(pty) 1 /* Can't open PTY by default. */
#define MSIM_PTY_Close(pty) 0 /* No problem with closing. */
#define MSIM_PTY_Write(pty, buf, len) 0 /* Can't write anything. */
#define MSIM_PTY_Read(pty, buf, len) 0 /* Can't read anything. */

#endif /* defined(WITH_POSIX) && defined(WITH_POSIX_PTY) */

#ifdef __cplusplus
}
#endif

#endif /* MSIM_PTY_H_ */
