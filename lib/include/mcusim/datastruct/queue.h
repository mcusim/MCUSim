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
#ifndef MSIM_DATASTRUCT_QUEUE_H_
#define MSIM_DATASTRUCT_QUEUE_H_ 1

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Basic queue routines return codes */
typedef enum MSIM_QResult {
	QRES_SUCCESS = INT16_MIN,
	QRES_INVALID_INPUT,
	QRES_ALLOC_FAIL,
	QRES_QUEUE_EMPTY,
	QRES_QUEUE_FULL,
} MSIM_QResult_t;

/* Basic FIFO queue */
typedef struct MSIM_Queue {
	uint32_t capacity;		/* queue capacity */
	uint32_t head;			/* queue's first element */
	uint32_t tail;			/* queue's last element */
	uint32_t size;			/* actual number of elements in
					   the queue */
	void **base_arr;		/* queue base array */
} MSIM_Queue_t;

MSIM_QResult_t MSIM_InitQueue(MSIM_Queue_t *q,
			      void **q_arr,
			      uint32_t q_arr_size);

MSIM_QResult_t MSIM_Enqueue(MSIM_Queue_t *q, void *elem);

MSIM_QResult_t MSIM_Dequeue(MSIM_Queue_t *q, void **elem);

bool MSIM_QueueHasNext(const MSIM_Queue_t *q);

bool MSIM_IsQueueEmpty(const MSIM_Queue_t *q);

#ifdef __cplusplus
}
#endif

#endif /* MSIM_DATASTRUCT_QUEUE_H_ */
