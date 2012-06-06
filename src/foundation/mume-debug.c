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
#include "mume-debug.h"
#include "mume-config.h"
#include "mume-memory.h"
#include MUME_ASSERT_H
#include MUME_STDARG_H
#include MUME_STDIO_H
#include MUME_STDLIB_H

struct mume_logger_s {
    int level;
};

static mume_logger_t _default_logger = {
    MUME_LOG_LAST
};

/* TODO: Add muti thread logging support */
static struct {
    mume_logger_t *logger;
    mume_mutex_t *mutex;
    int loglevel;
    int curlevel;
} _gstate;

static const char*log_level_strs[MUME_LOG_LAST] = {
    "abort", "error", "warning", "debug",
    "trace0", "trace1", "trace2", "trace3"
};

mume_logger_t* mume_logger_create(int level)
{
    mume_logger_t *logger;
    logger = malloc_abort(sizeof(mume_logger_t));
    logger->level = level;
    return logger;
}

void mume_logger_destroy(mume_logger_t *logger)
{
    free(logger);
}

void mume_logger_set_level(mume_logger_t *logger, int level)
{
    assert(level >= 0 && level <= MUME_LOG_LAST);

    if (NULL == logger)
        logger = &_default_logger;

    logger->level = level;
}

void mume_logger_enter(
    mume_logger_t *logger, int level,
    const char *file, const char *func, int line)
{
    /* currently not support recursive logging */
    /* TODO: Add thread support */
    /* assert(NULL == _gstate.logger); */
    assert(level >= MUME_LOG_ABORT && level < MUME_LOG_LAST);
    assert(file && func && line >= 0);

    if (logger) {
        _gstate.logger = logger;
        _gstate.curlevel = logger->level;
    }
    else {
        _gstate.logger = &_default_logger;
        _gstate.curlevel = _default_logger.level;
    }

    _gstate.loglevel = level;

    mume_logger_print(
        "[%s] %s:%s:%d :", log_level_strs[level], file, func, line);
}

void mume_logger_leave(void)
{
    _gstate.logger = NULL;
    _gstate.curlevel = 0;
}

void mume_logger_print(const char *format, ...)
{
    if (_gstate.loglevel <= _gstate.curlevel) {
        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}
