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

void test_time_add(void)
{
#define count 3
    int tvs1[count][3] = {
        {0, -1000, 1000000}, {-4, 3100, -2101000}, {100400, -200004, 2001}
    };
    int tvs2[count][3] = {
        {1, 0, 4000100}, {1, -3000, -1000000}, {301, -5, 4999}
    };
    mume_timeval_t res[count] = {
        {5, 100}, {-6, -1000}, {100500, 998000}
    };
    mume_timeval_t val1;
    mume_timeval_t val2;
    int i;
    for (i = 0; i < count; ++i) {
        val1 = mume_timeval_make(tvs1[i][0], tvs1[i][1], tvs1[i][2]);
        val2 = mume_timeval_make(tvs2[i][0], tvs2[i][1], tvs2[i][2]);
        val1 = mume_timeval_add(&val1, &val2);
        test_assert(val1.tv_sec == res[i].tv_sec &&
                    val1.tv_usec == res[i].tv_usec);
    }
#undef count
}

void test_time_sub(void)
{
#define count 3
    mume_timeval_t tvs1[count] = {
        {0, 0}, {100, 100}, {4000, 0}
    };
    mume_timeval_t tvs2[count] = {
        {10, 2100000}, {200, 5000200}, {1000, 500}
    };
    mume_timeval_t res[count] = {
        {-12, -100000}, {-105, -100}, {2999, 999500}
    };
    int i;
    for (i = 0; i < count; ++i) {
        tvs1[i] = mume_timeval_sub(tvs1 + i, tvs2 + i);
        test_assert(tvs1[i].tv_sec == res[i].tv_sec &&
                  tvs1[i].tv_usec == res[i].tv_usec);
    }
#undef count
}

void test_time_cmp(void)
{
#define count 8
    mume_timeval_t tvs1[count] = {
        {2, 0}, {0, -1}, {100, 20}, {-1, 0}, {2, 1000},
        {0, -2000}, {500, -4000000}, {2, -2000000}
    };
    mume_timeval_t tvs2[count] = {
        {0, 0}, {100, 0}, {100, 20}, {0, -1000000}, {1, 1001000},
        {-1, 0}, {497, -10}, {9, -9000000}
    };
    int res[count] = {
        1, -1, 0, 0, 0,
        1, -1, 0
    };
    int i;
    for (i = 0; i < count; ++i) {
        test_assert(res[i] == mume_timeval_cmp(tvs1 + i, tvs2 + i));
    }
#undef count
}
