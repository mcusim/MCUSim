/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <pthread.h>

#include "mcusim/thread/pool.h"
#include "mcusim/datastruct/queue.h"

#include "minunit.h"

#define SUITE_NAME		"threadpool_test"
#define THREADPOOL_TASKS	32
#define THREADPOOL_THREADS	16

#define TASKS_TO_PROCESS	32

/* Tests counter */
int tests_run = 0;

/* Test functions prototypes */
int threadpool_should_be_initialized(void);
int task_should_be_submitted(void);

/* Private prototypes */
static void *test_thread_func(void *arg);

/* Private variables */
static uint16_t task_processed = TASKS_TO_PROCESS;
static pthread_mutex_t task_processed_mutex;
static pthread_cond_t task_processed_cv;

int threadpool_should_be_initialized(void)
{
	const uint32_t attempts = 12;
	uint32_t i;

	for (i = 0; i < attempts; i++) {
		pthread_t threads[THREADPOOL_THREADS];
		void *q_array[THREADPOOL_TASKS];
		struct MSIM_Queue q;
		struct MSIM_ThreadPool pool;
		struct MSIM_ThreadPoolQueue pool_queue;
		enum MSIM_ThreadPoolRes res;

		MSIM_InitQueue(&q, q_array, THREADPOOL_TASKS);
		res = MSIM_InitThreadPoolQueue(&pool_queue, &q);
		_mu_test(res == THREADPOOL_SUCCESS);
		_mu_test(pool_queue.q == &q);

		res = MSIM_InitThreadPool(&pool, &pool_queue,
				THREADPOOL_THREADS, threads);

		_mu_test(res == THREADPOOL_SUCCESS);
		_mu_test(pool.q == &pool_queue);
		_mu_test(pool.nthreads == THREADPOOL_THREADS);
		_mu_test(pool.threads == threads);

		MSIM_DestroyThreadPool(&pool);
		MSIM_DestroyThreadPoolQueue(&pool_queue);
	}
	return 0;
}

int task_should_be_submitted(void)
{
	pthread_t threads[THREADPOOL_THREADS];
	void *q_array[THREADPOOL_TASKS];
	struct MSIM_Queue q;
	struct MSIM_ThreadPool pool;
	struct MSIM_ThreadPoolQueue pool_queue;
	struct MSIM_ThreadPoolTask pool_tasks[TASKS_TO_PROCESS];
	int i;

	/* Initialize mutex and conditional variable */
	pthread_mutex_init(&task_processed_mutex, NULL);
	pthread_cond_init(&task_processed_cv, NULL);

	/* Initialize queue and thread pool */
	MSIM_InitQueue(&q, q_array, THREADPOOL_TASKS);
	MSIM_InitThreadPoolQueue(&pool_queue, &q);
	MSIM_InitThreadPool(&pool, &pool_queue, THREADPOOL_THREADS, threads);

	for (i = 0; i < TASKS_TO_PROCESS; i++) {
		pool_tasks[i].func = test_thread_func;
		pool_tasks[i].arg = NULL;
		MSIM_SubmitThreadPool(&pool, &pool_tasks[i]);
	}

	/* Wait for the tasks to be processed */
	pthread_mutex_lock(&task_processed_mutex);
	while (task_processed > 0) {
		pthread_cond_wait(&task_processed_cv, &task_processed_mutex);
	}
	pthread_mutex_unlock(&task_processed_mutex);

	/* Clean up and finish the test */
	MSIM_DestroyThreadPool(&pool);
	MSIM_DestroyThreadPoolQueue(&pool_queue);
	pthread_cond_destroy(&task_processed_cv);
	pthread_mutex_destroy(&task_processed_mutex);
	return 0;
}

int all_tests(void)
{
	_mu_verify(threadpool_should_be_initialized);
	_mu_verify(task_should_be_submitted);
	return 0;
}

char *suite_name(void)
{
	return SUITE_NAME;
}

void setup_tests(void)
{
}

static void *test_thread_func(void *arg)
{
	pthread_mutex_lock(&task_processed_mutex);
	printf("  test_thread_func: task %d finished\n", task_processed);
	task_processed--;
	pthread_cond_broadcast(&task_processed_cv);
	pthread_mutex_unlock(&task_processed_mutex);

	return 0;
}
