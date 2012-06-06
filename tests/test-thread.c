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
#include "mume-base.h"
#include "test-util.h"

static mume_mutex_t *_mutex;
static mume_sem_t *_sem1;
static mume_sem_t *_sem2;

static void _mutex_proc(void *param)
{
    int n;
    mume_mutex_lock(_mutex);
    n = *(int*)param;
    mume_sleep_msec(10);
    n += 1;
    *(int*)param = n;
    mume_mutex_unlock(_mutex);
}

static void _sem_proc(void *param)
{
    mume_sem_wait(_sem1);
    ++(*(int*)param);
    mume_sem_post(_sem2);
}

void test_thread_mutex(void)
{
    int i;
    int val = 0;
    mume_thread_t **t[10];
    _mutex = mume_mutex_new();
    test_assert(mume_mutex_trylock(_mutex));
    test_assert(!mume_mutex_trylock(_mutex));
    mume_mutex_unlock(_mutex);
    for (i = 0; i < COUNT_OF(t); ++i) {
        t[i] = mume_thread_new(_mutex_proc, &val);
    }

    for (i = 0; i < COUNT_OF(t); ++i) {
        mume_thread_join(t[i]);
        mume_thread_delete(t[i]);
    }
    mume_mutex_delete(_mutex);
    test_assert(COUNT_OF(t) == val);
}

void test_thread_semaphore(void)
{
    int i;
    int val = 0;
    mume_thread_t **t[10];
    _sem1 = mume_sem_new();
    _sem2 = mume_sem_new();
    test_assert(!mume_sem_trywait(_sem1));
    test_assert(!mume_sem_timedwait(_sem1, 10));
    test_assert(mume_sem_post(_sem1));
    test_assert(mume_sem_post(_sem1));
    test_assert(mume_sem_trywait(_sem1));
    test_assert(mume_sem_timedwait(_sem1, 10));
    test_assert(!mume_sem_trywait(_sem1));
    test_assert(!mume_sem_timedwait(_sem1, 10));
    for (i = 0; i < COUNT_OF(t); ++i) {
        t[i] = mume_thread_new(_sem_proc, &val);
    }

    for (i = 0; i < COUNT_OF(t); ++i) {
        test_assert(mume_sem_post(_sem1));
        test_assert(mume_sem_wait(_sem2));
        test_assert(val == i + 1);
    }

    for (i = 0; i < COUNT_OF(t); ++i) {
        mume_thread_join(t[i]);
        mume_thread_delete(t[i]);
    }
    mume_sem_delete(_sem1);
    mume_sem_delete(_sem2);
}
