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
#include "mume-time.h"
#include "mume-config.h"
#include "mume-debug.h"

#if HAVE_SYS_TIME_H

#include MUME_SYS_TIME_H
#include MUME_TIME_H

void mume_gettimeofday(mume_timeval_t *tv)
{
    struct timeval tmp;
    gettimeofday(&tmp, NULL);
    tv->tv_sec = tmp.tv_sec;
    tv->tv_usec = tmp.tv_usec;
}

void mume_sleep(const mume_timeval_t *tv)
{
    struct timespec req;
    req.tv_sec = tv->tv_sec;
    req.tv_nsec = tv->tv_usec * 1000L;
    nanosleep(&req, NULL);
}

void mume_sleep_msec(int msec)
{
    struct timespec req;
    req.tv_sec = msec / MUME_MSECS_PER_SEC;
    req.tv_nsec = (msec % MUME_MSECS_PER_SEC) * 1000000L;
    nanosleep(&req, NULL);
}

#elif HAVE_WINDOWS_H

#include MUME_WINDOWS_H

void mume_gettimeofday(mume_timeval_t *tv)
{
    FILETIME tfile;
    ULARGE_INTEGER _100ns;
    GetSystemTimeAsFileTime (&tfile);
    _100ns.LowPart = tfile.dwLowDateTime;
    _100ns.HighPart = tfile.dwHighDateTime;
    _100ns.QuadPart -= 0x19db1ded53e8000L;
    tv->tv_sec = (int)(_100ns.QuadPart / (10000 * 1000));
    tv->tv_usec = (int)((_100ns.QuadPart % (10000 * 1000)) / 10);
    mume_tv_normalize(tv);
}

void mume_sleep(const mume_timeval_t *tv)
{
    Sleep(tv->tv_sec * MUME_MSECS_PER_SEC +
        tv->tv_usec / MUME_USECS_PER_MSEC +
        (tv->tv_usec % MUME_USECS_PER_MSEC ? 1 : 0));
}

void mume_sleep_msec(int msec)
{
    Sleep(msec);
}

#else /* !HAVE_SYS_TIME_H && !HAVE_WINDOWS_H */
# error Unknown time implementation
#endif

void mume_timeval_normalize(mume_timeval_t *tv)
{
    if ((tv->tv_usec >= MUME_USECS_PER_SEC) ||
        (tv->tv_usec <= -MUME_USECS_PER_SEC))
    {
        tv->tv_sec += tv->tv_usec / MUME_USECS_PER_SEC;
        tv->tv_usec %= MUME_USECS_PER_SEC;
    }
    if (tv->tv_sec >= 1 && tv->tv_usec < 0) {
        --tv->tv_sec;
        tv->tv_usec += MUME_USECS_PER_SEC;
    }
}

mume_timeval_t mume_timeval_make(int sec, int msec, int usec)
{
    mume_timeval_t tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    mume_timeval_normalize(&tv);
    tv.tv_sec += msec / MUME_MSECS_PER_SEC;
    tv.tv_usec += (msec % MUME_MSECS_PER_SEC) * MUME_USECS_PER_MSEC;
    mume_timeval_normalize(&tv);
    return tv;
}

int mume_timeval_cmp(
    const mume_timeval_t *a, const mume_timeval_t *b)
{
    mume_timeval_t tx = *a;
    mume_timeval_t ty = *b;
    mume_timeval_normalize(&tx);
    mume_timeval_normalize(&ty);
    if (tx.tv_sec > ty.tv_sec)
        return 1;
    if (tx.tv_sec < ty.tv_sec)
        return -1;
    if (tx.tv_usec > ty.tv_usec)
        return 1;
    if (tx.tv_usec < ty.tv_usec)
        return -1;
    return 0;
}

mume_timeval_t mume_timeval_add(
    const mume_timeval_t *a, const mume_timeval_t *b)
{
    mume_timeval_t tv;
    tv.tv_sec = a->tv_sec + b->tv_sec;
    tv.tv_usec = a->tv_usec + b->tv_usec;
    mume_timeval_normalize(&tv);
    return tv;
}

mume_timeval_t mume_timeval_sub(
    const mume_timeval_t *a, const mume_timeval_t *b)
{
    mume_timeval_t tv;
    tv.tv_sec = a->tv_sec - b->tv_sec;
    tv.tv_usec = a->tv_usec - b->tv_usec;
    mume_timeval_normalize(&tv);
    return tv;
}
