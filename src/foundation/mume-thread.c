/* Mume Reader - a full featured reading environment.
 *
 * Copyright © 2012 Soft Flag, Inc.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version
 * 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "mume-thread.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include MUME_ERRNO_H
#include MUME_STDLIB_H
#include MUME_STRING_H

typedef struct _thread_param_s {
    void (*proc)(void *param);
    void *param;
} _thread_param_t;

#ifdef HAVE_WINDOWS_H

static DWORD WINAPI _win32_thread_proc(LPVOID param)
{
    _thread_param_t *p = param;
    p->proc(p->param);
    free(p);
    return 0;
}

mume_thread_t* mume_thread_new(void (*proc)(void*), void *param)
{
    HANDLE h;
    DWORD tid = 0;
    _thread_param_t *p;
    p = malloc_struct(_thread_param_t);
    p->proc = proc;
    p->param = param;
    h = CreateThread(NULL, 0, _win32_thread_proc, p, 0, &tid);
    if (NULL == h) {
        mume_error(NULL, ("CreateThread: %d\n", GetLastError()));
        free(p);
        return NULL;
    }
    return h;
}

void mume_thread_delete(mume_thread_t *t)
{
    if (!CloseHandle(t)) {
        mume_error(NULL, ("CloseHandle: %d\n", GetLastError()));
    }
}

mume_mutex_t* mume_mutex_new(void)
{
    CRITICAL_SECTION *cs = malloc_struct(CRITICAL_SECTION);
    InitializeCriticalSection(cs);
    return cs;
}

void mume_mutex_delete(mume_mutex_t *mtx)
{
    DeleteCriticalSection(cs);
    free(mtx);
}

void mume_mutex_lock(mume_mutex_t *mtx)
{
    EnterCriticalSection(mtx);
}

int mume_mutex_trylock(mume_mutex_t *mtx)
{
    return TryEnterCriticalSection(mtx);
}

void mume_mutex_unlock(mume_mutex_t *mtx)
{
    LeaveCriticalSection(mtx);
}

mume_sem_t* mume_sem_new(void)
{
    assert(0);
}

void mume_sem_delete(mume_sem_t *sem)
{
    assert(0);
}

int mume_sem_post(mume_sem_t *sem)
{
    assert(0);
}

int mume_sem_wait(mume_sem_t *sem)
{
    assert(0);
}

int mume_sem_trywait(mume_sem_t *sem)
{
    assert(0);
}

int mume_sem_timedwait(mume_sem_t *sem, int wait)
{
    assert(0);
}

#elif HAVE_PTHREAD_H

#include <pthread.h>
#include <semaphore.h>
#include MUME_SYS_TIME_H

static void* _pthread_proc(void *param)
{
    _thread_param_t *p = param;
    p->proc(p->param);
    free(p);
    return 0;
}

mume_thread_t* mume_thread_new(void (*proc)(void*), void *param)
{
    int err;
    pthread_t *t = malloc_struct(pthread_t);
    _thread_param_t *p;
    p = malloc_struct(_thread_param_t);
    p->proc = proc;
    p->param = param;
    if ((err = pthread_create(t, NULL, &_pthread_proc, p))) {
        mume_error(("pthread_create: %s\n", strerror(err)));
        free(p);
        return NULL;
    }
    return t;
}

void mume_thread_delete(mume_thread_t *t)
{
    pthread_detach(*(pthread_t*)t);
    free(t);
}

void mume_thread_join(mume_thread_t *t)
{
    pthread_join(*(pthread_t*)t, NULL);
}

mume_mutex_t* mume_mutex_new(void)
{
    int err;
    pthread_mutex_t *mtx = malloc_struct(pthread_mutex_t);
    if ((err = pthread_mutex_init(mtx, NULL))) {
        free(mtx);
        mume_error(("pthread_mutex_init: %d\n", strerror(err)));
        return NULL;
    }
    return mtx;
}

void mume_mutex_delete(mume_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_destroy(mtx))) {
        mume_error(("pthread_mutex_destroy: %s\n", strerror(err)));
    }
    free(mtx);
}

void mume_mutex_lock(mume_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_lock(mtx)))
        mume_error(("pthread_mutex_lock: %s\n", strerror(err)));
}

int mume_mutex_trylock(mume_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_trylock(mtx))) {
        mume_debug(("pthread_mutex_trylock: %s\n", strerror(err)));
        return 0;
    }
    return 1;
}

void mume_mutex_unlock(mume_mutex_t *mtx)
{
    int err;
    if ((err = pthread_mutex_unlock(mtx)))
        mume_error(("pthread_mutex_unlock: %s\n", strerror(err)));
}

mume_sem_t* mume_sem_new(void)
{
    int err;
    sem_t *sem = malloc_struct(sem_t);
    if ((err = sem_init(sem, 0, 0))) {
        mume_error(("sem_init: %s\n", strerror(errno)));
        free(sem);
        return NULL;
    }
    return sem;
}

void mume_sem_delete(mume_sem_t *sem)
{
    if (sem_destroy(sem))
        mume_error(("sem_destroy: %s\n", strerror(errno)));
    free(sem);
}

int mume_sem_post(mume_sem_t *sem)
{
    if (sem_post(sem)) {
        mume_error(("sem_post: %s\n", strerror(errno)));
        return 0;
    }
    return 1;
}

int mume_sem_wait(mume_sem_t *sem)
{
    if (sem_wait(sem)) {
        mume_error(("sem_wait: %s\n", strerror(errno)));
        return 0;
    }
    return 1;
}

int mume_sem_trywait(mume_sem_t *sem)
{
    if (sem_trywait(sem)) {
        mume_debug(("sem_trywait: %s\n", strerror(errno)));
        return 0;
    }
    return 1;
}

int mume_sem_timedwait(mume_sem_t *sem, int wait)
{
    struct timeval now;
    struct timespec abstime;
    gettimeofday(&now, NULL);
    abstime.tv_nsec = now.tv_usec * 1000 + (wait % 1000) * 1000000;
    abstime.tv_sec = now.tv_sec + wait / 1000;
    if (sem_timedwait(sem, &abstime)) {
        mume_debug(("sem_timedwait: %s\n", strerror(errno)));
        return 0;
    }
    return 1;
}

#else  /* !HAVE_WINDOWS_H && !HAVE_PTHREAD_H */
# error Unknown thread implementation
#endif
