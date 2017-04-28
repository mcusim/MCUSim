/*
 * MCUSim - Interactive simulator for microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mcusim/datastruct/queue.h"

MSIM_QResult_t MSIM_InitQueue(MSIM_Queue_t *q,
			      void **q_arr,
			      uint32_t q_arr_size)
{
	if (q_arr_size <= 0) {
		fprintf(stderr, "Array size should be positive\n");
		return QRES_INVALID_INPUT;
	}

	memset(q_arr, 0, q_arr_size);
	q->base_arr = q_arr;
	q->size = 0;
	q->capacity = q_arr_size;
	q->head = 0;
	q->tail = 0;
	return QRES_SUCCESS;
}

MSIM_QResult_t MSIM_Enqueue(MSIM_Queue_t *q, void *elem)
{
	if (q->size == q->capacity) {
		fprintf(stderr, "Queue is full\n");
		return QRES_QUEUE_FULL;
	}

	q->base_arr[q->tail] = elem;
	q->tail++;
	q->size++;
	/* tail is behind highest index in array */
	if (q->tail == q->capacity)
		q->tail = 0;

	return QRES_SUCCESS;
}

MSIM_QResult_t MSIM_Dequeue(MSIM_Queue_t *q, void **elem)
{
	if (q->size == 0) {
		fprintf(stderr, "Queue is empty\n");
		return QRES_QUEUE_EMPTY;
	}

	*elem = q->base_arr[q->head];
	q->head++;
	q->size--;
	/* head is behind highest index in array */
	if (q->head == q->capacity)
		q->head = 0;

	return QRES_SUCCESS;
}

bool MSIM_QueueHasNext(const MSIM_Queue_t *q)
{
	if (q == NULL)
		return false;

	if (q->size > 0) {
		return true;
	} else {
		return false;
	}
}

bool MSIM_IsQueueEmpty(const MSIM_Queue_t *q)
{
	if (q == NULL)
		return true;

	if (q->size > 0) {
		return false;
	} else {
		return true;
	}
}
