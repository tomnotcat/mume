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
#ifndef MUME_FOUNDATION_DEBUG_H
#define MUME_FOUNDATION_DEBUG_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#ifndef _VAL
# define _VAL(_x) #_x
#endif  /* _VAL */
#ifndef _STR
# define _STR(_x) _VAL(_x)
#endif  /* _STR */

#define MUME_STATIC_ASSERT1(COND, MSG) \
    typedef char static_assertion_##MSG[(COND)?1:-1]
#define MUME_STATIC_ASSERT3(X, L) MUME_STATIC_ASSERT1(X, at_line_##L)
#define MUME_STATIC_ASSERT2(X, L) MUME_STATIC_ASSERT3(X, L)
#define MUME_STATIC_ASSERT(X) MUME_STATIC_ASSERT2(X, __LINE__)

enum mume_log_level_e {
    MUME_LOG_ABORT = 0,
    MUME_LOG_ERROR = 1,
    MUME_LOG_WARNING = 2,
    MUME_LOG_DEBUG = 3,
    MUME_LOG_TRACE0 = 4,
    MUME_LOG_TRACE1 = 5,
    MUME_LOG_TRACE2 = 6,
    MUME_LOG_TRACE3 = 7,
    MUME_LOG_LAST
};

mume_public mume_logger_t* mume_logger_create(int level);

mume_public void mume_logger_destroy(mume_logger_t *logger);

/* Set the log level of the specified logger. */
mume_public void mume_logger_set_level(
    mume_logger_t *logger, int level);

/* Enter/Leave a logging region. */
mume_public void mume_logger_enter(
    mume_logger_t *logger, int level,
    const char *file, const char *func, int line);

mume_public void mume_logger_leave(void);

#define mume_logger_enter_inplace(_log, _level) \
    mume_logger_enter(_log, _level, __FILE__, \
                       __FUNCTION__, __LINE__)

/* Print a log message to current logging region. */
mume_public void mume_logger_print(const char *format, ...);

/* Utility macros for logging. */
#define mume_trace0(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_TRACE0, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_trace1(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_TRACE1, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_trace2(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_TRACE2, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_trace3(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_TRACE3, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_debug(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_DEBUG, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_warning(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_WARNING, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_error(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_ERROR, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
    } while (0)

#define mume_abort(_msg) \
    do { \
        mume_logger_enter(NULL, MUME_LOG_ABORT, __FILE__, \
                           __FUNCTION__, __LINE__); \
        mume_logger_print _msg; \
        mume_logger_leave(); \
        exit(1); \
    } while(0)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_DEBUG_H */
