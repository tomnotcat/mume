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
#ifndef MUME_FOUNDATION_THREAD_H
#define MUME_FOUNDATION_THREAD_H

#include "mume-common.h"

MUME_BEGIN_DECLS

mume_public mume_thread_t* mume_thread_new(
    void (*proc)(void*), void *param);

mume_public void mume_thread_delete(mume_thread_t *t);

mume_public void mume_thread_join(mume_thread_t *t);

mume_public mume_mutex_t* mume_mutex_new(void);

mume_public void mume_mutex_delete(mume_mutex_t *mtx);

mume_public void mume_mutex_lock(mume_mutex_t *mtx);

mume_public int mume_mutex_trylock(mume_mutex_t *mtx);

mume_public void mume_mutex_unlock(mume_mutex_t *mtx);

mume_public mume_sem_t* mume_sem_new(void);

mume_public void mume_sem_delete(mume_sem_t *sem);

mume_public int mume_sem_post(mume_sem_t *sem);

mume_public int mume_sem_wait(mume_sem_t *sem);

mume_public int mume_sem_trywait(mume_sem_t *sem);

mume_public int mume_sem_timedwait(mume_sem_t *sem, int wait);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_THREAD_H */
