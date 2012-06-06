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
#include "mume-gui.h"
#include "test-util.h"

typedef struct my_timer_args_s {
    mume_timer_t tmr;
    mume_timeval_t expire;
    int id;
    int count;
    int interval;
} my_timer_args_t;

static my_timer_args_t g_tmrs[4];

static int _timer_proc(mume_timer_t *tmr)
{
    my_timer_args_t *ta = CONTAINER_OF(
        tmr, my_timer_args_t, tmr);
    if (0 == ta->count)
        mume_gettimeofday(&ta->expire);

    ++ta->count;
    if (ta->id == 0 && 9 == ta->count)
        mume_cancel_timer(tmr);
    if (ta->id == 1 && 4 == ta->count)
        return 0;
    if (ta->id == 2 && 3 == ta->count)
        mume_cancel_timer(tmr);
    if (ta->id == 3 && 2 == ta->count) {
        mume_event_t event;
        event.type = MUME_EVENT_COMMAND;
        mume_post_event(event);
    }

    return ta->interval;
}

static void test_gui_timer(void)
{
    const int intervals[4] = {
        1, 20, 50, 200
    };
    int i;
    mume_event_t event;
    for (i = 0; i < 4; ++i) {
        mume_timer_ctor(&g_tmrs[i].tmr, _timer_proc, NULL);
        mume_schedule_timer(&g_tmrs[i].tmr, intervals[i]);
        g_tmrs[i].id = i;
        g_tmrs[i].interval = intervals[i];
    }

    while (mume_wait_event(&event)) {
        if (MUME_EVENT_COMMAND == event.type)
            break;
    }
    test_assert(9 == g_tmrs[0].count);
    test_assert(4 == g_tmrs[1].count);
    test_assert(mume_timeval_cmp(
        &g_tmrs[1].expire, &g_tmrs[0].expire) > 0);
    test_assert(3 == g_tmrs[2].count);
    test_assert(mume_timeval_cmp(
        &g_tmrs[2].expire, &g_tmrs[1].expire) > 0);
    test_assert(2 == g_tmrs[3].count);
    test_assert(mume_timeval_cmp(
        &g_tmrs[3].expire, &g_tmrs[2].expire) > 0);
}

void all_tests(void)
{
    test_run(test_gui_timer);
}
