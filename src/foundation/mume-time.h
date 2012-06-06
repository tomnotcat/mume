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
#ifndef MUME_FOUNDATION_TIME_H
#define MUME_FOUNDATION_TIME_H

#include "mume-common.h"

MUME_BEGIN_DECLS

struct mume_timeval_s {
    int tv_sec;
    int tv_usec;
};

#define MUME_USECS_PER_MSEC 1000
#define MUME_USECS_PER_SEC 1000000
#define MUME_MSECS_PER_SEC 1000

mume_public void mume_gettimeofday(mume_timeval_t *tv);

mume_public void mume_sleep(const mume_timeval_t *tv);

mume_public void mume_sleep_msec(int msec);

mume_public void mume_timeval_normalize(mume_timeval_t *tv);

mume_public mume_timeval_t mume_timeval_make(
    int sec, int msec, int usec);

mume_public int mume_timeval_cmp(
    const mume_timeval_t *x, const mume_timeval_t *y);

mume_public mume_timeval_t mume_timeval_add(
    const mume_timeval_t *x, const mume_timeval_t *y);

mume_public mume_timeval_t mume_timeval_sub(
    const mume_timeval_t *x, const mume_timeval_t *b);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TIME_H */
