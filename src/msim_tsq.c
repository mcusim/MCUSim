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
 * Implementation of a simple and thread-safe queue (TSQ).
 */
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <inttypes.h>
#include "mcusim/tsq.h"
#include "mcusim/log.h"

#define TSQ_INIT	0x7575cafe
#define TSQ_NOTINIT	0xdeadbeef

int MSIM_TSQ_Init(struct MSIM_TSQ *q)
{
	pthread_condattr_t cond_attr;
	int rc;

	/* Prepare attributes for conditional variables. */
	rc = pthread_condattr_init(&cond_attr);
	if (rc != 0) {
		return MSIM_TSQ_ERR;
	}

	/* Initialize a basic queue mutex. */
	rc = pthread_mutex_init(&q->mutex, NULL);
	if (rc != 0) {
		pthread_condattr_destroy(&cond_attr);
		return MSIM_TSQ_ERR;
	}

	/* Initialize conditional variables. */
	rc = pthread_cond_init(&q->got_elem, &cond_attr);
	if (rc != 0) {
		pthread_condattr_destroy(&cond_attr);
		pthread_mutex_destroy(&q->mutex);
		return MSIM_TSQ_ERR;
	}
	rc = pthread_cond_init(&q->got_space, &cond_attr);
	if (rc != 0) {
		pthread_condattr_destroy(&cond_attr);
		pthread_cond_destroy(&q->got_elem);
		pthread_mutex_destroy(&q->mutex);
		return MSIM_TSQ_ERR;
	}

	q->head = 0;
	q->tail = 0;
	q->length = 0;
	q->init = TSQ_INIT;

	return MSIM_TSQ_OK;
}

int MSIM_TSQ_Destroy(struct MSIM_TSQ *q)
{
	int rc;
	char buf[1024];

	/* Lock the basic queue mutex */
	pthread_mutex_lock(&q->mutex);

	if (q->init != TSQ_INIT) {
		return MSIM_TSQ_NOTINIT;
	}
	q->init = TSQ_NOTINIT;

	/* Notify all of the blocked threads about a destroyed queue. */
	pthread_cond_broadcast(&q->got_elem);
	pthread_cond_broadcast(&q->got_space);

	rc = pthread_cond_destroy(&q->got_elem);
	if (rc != 0) {
		MSIM_LOG_WARN("cannot destroy conditional variable");
	}
	rc = pthread_cond_destroy(&q->got_space);
	if (rc != 0) {
		MSIM_LOG_WARN("cannot destroy conditional variable");
	}
	q->head = 0;
	q->tail = 0;
	if (q->length > 0) {
		snprintf(buf, 1024, "destroying queue with length = %" PRIu32,
		         q->length);
		MSIM_LOG_WARN(buf);
	}
	q->length = 0;

	/* Unlock the basic queue mutex */
	pthread_mutex_unlock(&q->mutex);

	rc = pthread_mutex_destroy(&q->mutex);
	if (rc != 0) {
		MSIM_LOG_WARN("cannot destroy mutex");
	}

	return MSIM_TSQ_OK;
}

int MSIM_TSQ_Enqb(struct MSIM_TSQ *q, uint8_t *e, uint32_t len)
{
	int rc = MSIM_TSQ_OK;

	if (len > MSIM_TSQ_ELEMSZ) {
		return MSIM_TSQ_TOOBIG;
	}
	/* Lock the basic queue mutex */
	pthread_mutex_lock(&q->mutex);

	/* Check an overflow of the queue and lock the calling thread until
	 * there will be space for the element to enqueue. */
	if ((q->length > 0) && (q->tail == q->head)) {
		rc = pthread_cond_wait(&q->got_space, &q->mutex);
		/* We may have a queue destroyed - check it first.*/
		if (q->init == TSQ_NOTINIT) {
			rc = MSIM_TSQ_NOTINIT;
		} else {
			rc = rc == 0 ? MSIM_TSQ_OK : MSIM_TSQ_ERR;
		}
	}
	if ((rc == MSIM_TSQ_OK) && (q->init == TSQ_INIT)) {
		memcpy(&q->array[q->tail][0], e, len);
		q->length++;
		if (q->tail == (MSIM_TSQ_SIZE-1)) {
			q->tail = 0;
		} else {
			q->tail++;
		}
		/* Signal about an element available. */
		pthread_cond_signal(&q->got_elem);
	}

	/* Unlock the basic queue mutex */
	pthread_mutex_unlock(&q->mutex);

	return rc;
}

int MSIM_TSQ_Deqb(struct MSIM_TSQ *q, uint8_t *e, uint32_t len)
{
	int rc = MSIM_TSQ_OK;

	if (len == 0U) {
		return MSIM_TSQ_ERR;
	}
	/* Lock the basic queue mutex */
	pthread_mutex_lock(&q->mutex);

	/* Check an emptiness of the queue and lock the calling thread until
	 * there will be an element to dequeue. */
	if ((q->length == 0) && (q->tail == q->head)) {
		rc = pthread_cond_wait(&q->got_elem, &q->mutex);
		/* We may have a queue destroyed - check it first.*/
		if (q->init == TSQ_NOTINIT) {
			rc = MSIM_TSQ_NOTINIT;
		} else {
			rc = rc == 0 ? MSIM_TSQ_OK : MSIM_TSQ_ERR;
		}
	}
	if ((rc == MSIM_TSQ_OK) && (q->init == TSQ_INIT)) {
		memcpy(e, &q->array[q->head][0], len);
		q->length--;
		if (q->head == (MSIM_TSQ_SIZE-1)) {
			q->head = 0;
		} else {
			q->head++;
		}
		/* Signal about a space for element. */
		pthread_cond_signal(&q->got_space);
	}

	/* Unlock the basic queue mutex */
	pthread_mutex_unlock(&q->mutex);

	return rc;
}
