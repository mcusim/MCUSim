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
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#include "mcusim/thread/pool.h"
#include "mcusim/datastruct/queue.h"

#define POSITIVE		75
#define LOW32			0x00000000FFFFFFFF
#define TASKS_SEM		"mcusim_threadpool_signal_tasks_semaphore"

/* Private prototypes */
static void *process_task(void *arg);

/* Thread function processes tasks from the queue */
static void *process_task(void *arg)
{
	if (arg == NULL) {
		fprintf(stderr, "Thread function called with "
				"NULL argument!\n");
		pthread_exit(NULL);
	}

	struct MSIM_ThreadPool *pool;
	struct MSIM_ThreadPoolQueue *pq;
	struct MSIM_ThreadPoolTask *task;
	void *(*task_func)(void *arg);
	void *task_func_arg;
	uint8_t keep_alive;
	enum MSIM_QResult rq;

	pool = (struct MSIM_ThreadPool *) arg;
	pq = pool->q;
	pthread_mutex_lock(&pq->rw_mutex);
	keep_alive = pool->keep_alive;
	pthread_mutex_unlock(&pq->rw_mutex);

	#ifdef DEBUG
	printf("Thread function to process tasks running\n");
	#endif
	while (keep_alive) {
		/* Wait for a task to be available */
		#ifdef DEBUG
		printf("Waiting for a task...\n");
		#endif
		sem_wait(pool->tasks_sem);

		/* Retrieve the task */
		#ifdef DEBUG
		printf("Task is available!\n");
		#endif
		pthread_mutex_lock(&pq->rw_mutex);
		keep_alive = pool->keep_alive;
		rq = MSIM_Dequeue(pq->q, (void **) &task);
		pthread_mutex_unlock(&pq->rw_mutex);

		if (!keep_alive)
			pthread_exit(NULL);
		if (rq != QRES_SUCCESS)
			continue;
		task_func = task->func;
		task_func_arg = task->arg;
		task_func(task_func_arg);
	}
	pthread_exit(NULL);
}

enum MSIM_ThreadPoolRes MSIM_InitThreadPool(
		struct MSIM_ThreadPool *pool,
		struct MSIM_ThreadPoolQueue *q,
		uint32_t nthreads,
		pthread_t *threads)
{
	if (pool == NULL) {
		fprintf(stderr, "Thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}
	if (q == NULL) {
		fprintf(stderr, "Queue of thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}
	if (nthreads <= 0) {
		fprintf(stderr, "Number of threads in thread pool should "
				"be positive!\n");
		return THREADPOOL_INVALID_INPUT;
	}
	if (threads == NULL) {
		fprintf(stderr, "Array of threads in thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}

	int rc;
	uint32_t i;
	pthread_attr_t attr;

	/*
	 * Initialize semaphore to signal threads that there is a
	 * task to process.
	 */
	pool->tasks_sem = sem_open(TASKS_SEM, O_CREAT, 0600, 0);
	if (pool->tasks_sem == SEM_FAILED)
		/*
		 * Do not leave thread pool partially initialized!
		 */
		return THREADPOOL_SEM_CREATE_ERR;

	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pool->q = q;
	pool->nthreads = nthreads;
	pool->threads = threads;
	pthread_mutex_lock(&q->rw_mutex);
	pool->keep_alive = POSITIVE;
	pthread_mutex_unlock(&q->rw_mutex);
	for (i = 0; i < nthreads; i++) {
		rc = pthread_create(&threads[i], &attr, process_task, pool);
		if (rc)
			/*
			 * Do not leave thread pool partially initialized!
			 */
			return THREADPOOL_THREAD_CREATE_ERR;
	}

	pthread_attr_destroy(&attr);
	return THREADPOOL_SUCCESS;
}

enum MSIM_ThreadPoolRes MSIM_DestroyThreadPool(
		struct MSIM_ThreadPool *pool)
{
	if (pool == NULL) {
		fprintf(stderr, "Thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}

	int rc;
	uint32_t i;
	void *status;

	/* Tell all threads to finish their work */
	pthread_mutex_lock(&pool->q->rw_mutex);
	pool->keep_alive = 0;
	pthread_mutex_unlock(&pool->q->rw_mutex);
	for (i = 0; i < pool->nthreads; i++)
		sem_post(pool->tasks_sem);

	/* Wait for all threads, one by one */
	for (i = 0; i < pool->nthreads; i++) {
		rc = pthread_join(pool->threads[i], &status);
		if (rc)
			fprintf(stderr, "Thread hasn't been finished "
					"successfully!\n");
	}
	/* Destroy threads semaphore */
	rc = sem_close(pool->tasks_sem);
	if (rc)
		fprintf(stderr, "Threads semaphore hasn't been "
				"closed successfully!\n");

	pool->q = NULL;
	pool->nthreads = 0;
	pool->threads = NULL;
	return THREADPOOL_SUCCESS;
}

enum MSIM_ThreadPoolRes MSIM_InitThreadPoolQueue(
		struct MSIM_ThreadPoolQueue *pool_queue,
		struct MSIM_Queue *q)
{
	if (pool_queue == NULL) {
		fprintf(stderr, "Queue of thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}
	if (q == NULL) {
		fprintf(stderr, "Raw queue of thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}

	pool_queue->q = q;
	pthread_mutex_init(&pool_queue->rw_mutex, NULL);
	return THREADPOOL_SUCCESS;
}

enum MSIM_ThreadPoolRes MSIM_DestroyThreadPoolQueue(
		struct MSIM_ThreadPoolQueue *pool_queue)
{
	if (pool_queue == NULL) {
		fprintf(stderr, "Queue of thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}

	pool_queue->q = NULL;
	pthread_mutex_destroy(&pool_queue->rw_mutex);
	return THREADPOOL_SUCCESS;
}

enum MSIM_ThreadPoolRes MSIM_SubmitThreadPool(
		struct MSIM_ThreadPool *pool,
		struct MSIM_ThreadPoolTask *task)
{
	if (pool == NULL) {
		fprintf(stderr, "Thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}
	if (task == NULL) {
		fprintf(stderr, "Task for thread pool is NULL!\n");
		return THREADPOOL_INVALID_INPUT;
	}

	enum MSIM_QResult res;

	pthread_mutex_lock(&pool->q->rw_mutex);
	res = MSIM_Enqueue(pool->q->q, task);
	pthread_mutex_unlock(&pool->q->rw_mutex);
	sem_post(pool->tasks_sem);

	if (res != QRES_SUCCESS)
		return THREADPOOL_INVALID_INPUT;
	return THREADPOOL_SUCCESS;
}
