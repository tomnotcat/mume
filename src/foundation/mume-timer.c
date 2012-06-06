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
#include "mume-timer.h"
#include "mume-debug.h"
#include "mume-heap.h"
#include "mume-memory.h"
#include "mume-vector.h"
#include MUME_ASSERT_H

static int _mume_timer_comp(const void *v1, const void *v2)
{
    /* make the timer heap a min heap based on delay */
    return mume_timeval_cmp(
        &((*(const mume_timer_t**)v2)->expire),
        &((*(const mume_timer_t**)v1)->expire));
}

mume_timerq_t* mume_timerq_ctor(mume_timerq_t *tmrq)
{
    tmrq->timers = mume_vector_new(
        sizeof(mume_timer_t*), NULL, NULL);

    return tmrq;
}

mume_timerq_t* mume_timerq_dtor(mume_timerq_t *tmrq)
{
    mume_vector_delete(tmrq->timers);
    return tmrq;
}

void mume_timerq_schedule(
    mume_timerq_t *tmrq, mume_timer_t *tmr, int interval)
{
    mume_timeval_t now;
    assert(interval > 0);
    *(mume_timer_t**)mume_vector_push_back(tmrq->timers) = tmr;
    mume_gettimeofday(&now);
    tmr->expire = mume_timeval_make(0, interval, 0);
    tmr->expire = mume_timeval_add(&tmr->expire, &now);
    mume_push_heap(mume_vector_front(tmrq->timers),
                    mume_vector_size(tmrq->timers),
                    sizeof(mume_timer_t*), _mume_timer_comp);
}

void mume_timerq_cancel(
    mume_timerq_t *tmrq, mume_timer_t *tmr)
{
    size_t i, count;
    mume_timer_t **tms;
    tms = (mume_timer_t**)mume_vector_front(tmrq->timers);
    count = mume_vector_size(tmrq->timers);
    for (i = 0; i < count; ++i) {
        if (tms[i] != tmr)
            continue;
        if (i != count - 1) {
            /* adjest heap */
            tms[i] = tms[count - 1];
            mume_reheap_down(
                tms, count - 1, sizeof(mume_timer_t*),
                _mume_timer_comp, i);
        }
        mume_vector_pop_back(tmrq->timers);
        break;
    }
}

int mume_timerq_check(mume_timerq_t *tmrq, mume_timeval_t *wait)
{
    mume_timeval_t tv;
    mume_timeval_t ti;
    mume_timer_t *tmr;
    int interval;
    mume_gettimeofday(&tv);
    while (mume_vector_size(tmrq->timers) > 0) {
        tmr = *(mume_timer_t**)mume_vector_front(tmrq->timers);
        ti = mume_timeval_sub(&tmr->expire, &tv);
        if (ti.tv_sec > 0 ||
            (0 == ti.tv_sec && ti.tv_usec >= MUME_USECS_PER_MSEC))
        {
            /* No timer expired. */
            if (wait)
                *wait = ti;
            break;
        }

        interval = tmr->tmrproc(tmr);
        /* timer may be canceled in tmrproc */
        if (0 == mume_vector_size(tmrq->timers) ||
            tmr != *(mume_timer_t**)(
                mume_vector_front(tmrq->timers)))
        {
            continue;
        }

        if (interval > 0) {
            ti = mume_timeval_make(0, interval, 0);
            tmr->expire = mume_timeval_add(&tv, &ti);
            mume_pop_heap(
                mume_vector_front(tmrq->timers),
                mume_vector_size(tmrq->timers),
                sizeof(mume_timer_t*), _mume_timer_comp);
            mume_push_heap(
                mume_vector_front(tmrq->timers),
                mume_vector_size(tmrq->timers),
                sizeof(mume_timer_t*), _mume_timer_comp);
        }
        else {
            mume_timerq_cancel(tmrq, tmr);
        }
    }
    return (int)mume_vector_size(tmrq->timers);
}
