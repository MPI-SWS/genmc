/*
 * Fake definitions to simulate threads.h.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef __THREADS_H__
#define __THREADS_H__

#include <errno.h>
#include <pthread.h>

/* Only basic types for the time being */
typedef pthread_t thrd_t;
typedef pthread_mutex_t mtx_t;

typedef int (*thrd_start_t)(void *);

enum { mtx_plain }; /* Only plain mutexes for now */

enum { thrd_success, thrd_timedout, thrd_busy, thrd_error, thrd_nomem };

static inline __attribute__ ((always_inline))
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	int res = pthread_create(thr, NULL, (void *(*)(void *)) func, arg);
	if (res == 0)
		return thrd_success;
	return res == ENOMEM ? thrd_nomem : thrd_error;
}

static inline __attribute__ ((always_inline))
void thrd_exit(int res)
{
	pthread_exit((void *) (long) res);
}

static inline __attribute__ ((always_inline))
int thrd_join(thrd_t thr, int *res)
{
	void *retval;

	if (pthread_join(thr, &retval) != 0)
		return thrd_error;
	if (res)
		*res = (long) retval;
	return thrd_success;
}

static inline __attribute__ ((always_inline))
void thrd_yield(void)
{
}

static inline __attribute__ ((always_inline))
int mtx_init(mtx_t *mtx, int type)
{
	int res;

	res = pthread_mutex_init(mtx, NULL) == 0 ? thrd_success : thrd_error;
	return res;
}

static inline __attribute__ ((always_inline))
void mtx_destroy(mtx_t *mtx)
{
	pthread_mutex_destroy(mtx);
}

static inline __attribute__ ((always_inline))
int mtx_lock(mtx_t *mtx)
{
	int res = pthread_mutex_lock(mtx);
	if (res == EDEADLK)
		return thrd_busy;
	return res == 0 ? thrd_success : thrd_error;
}

static inline __attribute__ ((always_inline))
int mtx_unlock(mtx_t *mtx)
{
	return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;
}

#endif /* __THREADS_H__ */
