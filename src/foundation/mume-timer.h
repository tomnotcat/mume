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
#ifndef MUME_FOUNDATION_TIMER_H
#define MUME_FOUNDATION_TIMER_H

#include "mume-time.h"
#include "mume-common.h"

MUME_BEGIN_DECLS

struct mume_timer_s {
    mume_timeval_t expire;
    int (*tmrproc)(mume_timer_t *tmr);
    void *data;
};

struct mume_timerq_s {
    mume_vector_t *timers;
};

static inline mume_timer_t* mume_timer_ctor(
    mume_timer_t *tmr, int (*tmrproc)(mume_timer_t*), void *data)
{
    tmr->tmrproc = tmrproc;
    tmr->data = data;
    return tmr;
}

static inline mume_timer_t* mume_timer_dtor(
    mume_timer_t *tmr)
{
    tmr->tmrproc = 0;
    return tmr;
}

#define mume_timer_new(_tmrproc, _data) \
    mume_timer_ctor((mume_timer_t*)malloc_abort( \
        sizeof(mume_timer_t)), _tmrproc, _data)

#define mume_timer_delete(_tmr) \
    free(mume_timer_dtor(_tmr))

#define mume_timer_data(_tmr) ((void*)(_tmr)->data)

mume_public mume_timerq_t* mume_timerq_ctor(
    mume_timerq_t *tmrq);
mume_public mume_timerq_t* mume_timerq_dtor(
    mume_timerq_t *tmrq);
mume_public void mume_timerq_schedule(
    mume_timerq_t *tmrq, mume_timer_t *tmr, int interval);
mume_public void mume_timerq_cancel(
    mume_timerq_t *tmrq, mume_timer_t *tmr);
/* <wait> will be set to the next expire interval,
   return the number of timers in the queue. */
mume_public int mume_timerq_check(
    mume_timerq_t *tmrq, mume_timeval_t *wait);

#define mume_timerq_new() \
    mume_timerq_ctor((mume_timerq_t*)malloc_abort( \
        sizeof(mume_timerq_t)))

#define mume_timerq_delete(_tmrq) \
    free(mume_timerq_dtor(_tmrq))

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TIMER_H */
