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

/* There are declarations for a simple, thread-safe queue (TSQ). */
#ifndef MSIM_TSQ_H_
#define MSIM_TSQ_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* Size of a queue. */
#define MSIM_TSQ_SIZE		65536
/* Size of an element in the queue. */
#define MSIM_TSQ_ELEMSZ		1024

/* Return codes of the queue functions. */
#define MSIM_TSQ_OK		0
#define MSIM_TSQ_ERR		75
#define MSIM_TSQ_NOMORE		76
#define MSIM_TSQ_TOOBIG		77
#define MSIM_TSQ_NOTINIT	78

/* Structure to describe a thread-safe queue.
 *
 * Its size is (MSIM_TSQ_SIZE * MSIM_TSQ_ELEMSZ) of bytes at least. It is
 * better to allocate it statically unless a proper justification provided.
 *
 * length		Actual number of elements in the queue.
 * head			Index of the head element.
 * tail			Index of the tail element.
 * init			Flag to mark queue as initialized one.
 * array		Array which is the queue based upon. */
struct MSIM_TSQ {
	pthread_mutex_t mutex;
	pthread_cond_t got_elem;
	pthread_cond_t got_space;
	uint32_t length;
	uint32_t head;
	uint32_t tail;
	uint32_t init;
	uint8_t array[MSIM_TSQ_SIZE][MSIM_TSQ_ELEMSZ];
};

/* Initializes queue before any usage.
 *
 * This function is not thread-safe.
 *
 * Returns:
 * MSIM_TSQ_OK		If a queue was initialized correctly.
 * MSIM_TSQ_ERR		If an unknown error occurred. */
int MSIM_TSQ_Init(struct MSIM_TSQ *q);

/* Destroys any resources associated with the queue.
 *
 * Note that it does not de-allocate memory - owner of the queue should do
 * this instead. This function is not thread-safe.
 *
 * Returns:
 * MSIM_TSQ_OK		If the resources were destroyed correctly.
 * MSIM_TSQ_ERR		If an unknown error occurred.
 * MSIM_TSQ_NOTINIT	If a queue was not initialized before. */
int MSIM_TSQ_Destroy(struct MSIM_TSQ *q);

/* Add element 'e' of length 'len' to the tail of the queue.
 *
 * Size of the element should not be larger then MSIM_TSQ_ELEMSZ. The calling
 * thread will be blocked until the queue can accommodate a new element.
 *
 * Returns:
 * MSIM_TSQ_OK		If element was enqueued correctly.
 * MSIM_TSQ_TOOBIG	If element is too big.
 * MSIM_TSQ_NOTINIT	If the queue was destroyed while a calling thread
 * 			blocked. */
int MSIM_TSQ_Enqb(struct MSIM_TSQ *q, uint8_t *e, uint32_t len);

/* Obtain the head element of the queue and put it into 'e'.
 *
 * Given element 'e' should be large enough to accommodate MSIM_TSQ_ELEMSZ
 * bytes at most. The calling thread will be blocked until the queue has
 * any element to dequeue.
 *
 * Returns:
 * MSIM_TSQ_OK		If element was dequeued correctly.
 * MSIM_TSQ_ERR		If element cannot handle MSIM_TSQ_ELEMSZ of bytes.
 * MSIM_TSQ_NOTINIT	If the queue was destroyed while a calling thread
 * 			blocked. */
int MSIM_TSQ_Deqb(struct MSIM_TSQ *q, uint8_t *e, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_TSQ_H_ */
