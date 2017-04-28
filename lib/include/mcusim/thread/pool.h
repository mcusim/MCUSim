/*
 * MCUSim - Interactive simulator for microcontrollers.
 * Copyright (C) 2016-2017 Dmitry Salychev <darkness.bsd@gmail.com>
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
#ifndef MSIM_THREADPOOL_H_
#define MSIM_THREADPOOL_H_ 1

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "mcusim/datastruct/queue.h"

/* Result of the thread pool operations */
enum MSIM_ThreadPoolRes {
	THREADPOOL_SUCCESS = INT16_MIN,
	THREADPOOL_INVALID_INPUT,
	THREADPOOL_THREAD_CREATE_ERR,
	THREADPOOL_SEM_CREATE_ERR,
};

struct MSIM_ThreadPoolTask {
	void *(*func)(void *arg);	/* pointer to the task function */
	void *arg;			/* argument for the task function */
};

struct MSIM_ThreadPoolQueue {
	pthread_mutex_t rw_mutex;	/* queue r/w access lock */
	MSIM_Queue_t *q;		/* shared queue of tasks */
};

struct MSIM_ThreadPool {
	uint32_t nthreads;		/* number of allocated threads */
	uint8_t volatile keep_alive;	/* set to positive - to keep threads
					   alive; queue of the pool should be
					   locked to access the flag */
	sem_t *tasks_sem;		/* semaphore to unlock suspended worker
					   threads */
	pthread_t *threads;		/* array of the threads */
	struct MSIM_ThreadPoolQueue *q;	/* tasks queue of the thread pool */
};

enum MSIM_ThreadPoolRes MSIM_InitThreadPool(
		struct MSIM_ThreadPool *pool,
		struct MSIM_ThreadPoolQueue *q,
		uint32_t nthreads,
		pthread_t *threads);

enum MSIM_ThreadPoolRes MSIM_DestroyThreadPool(
		struct MSIM_ThreadPool *pool);

enum MSIM_ThreadPoolRes MSIM_InitThreadPoolQueue(
		struct MSIM_ThreadPoolQueue *pool_queue,
		struct MSIM_Queue *q);

enum MSIM_ThreadPoolRes MSIM_DestroyThreadPoolQueue(
		struct MSIM_ThreadPoolQueue *pool_queue);

enum MSIM_ThreadPoolRes MSIM_SubmitThreadPool(
		struct MSIM_ThreadPool *pool,
		struct MSIM_ThreadPoolTask *task);

#endif /* MSIM_THREADPOOL_H_ */
