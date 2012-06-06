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
#include "mume-stream.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include MUME_ASSERT_H
#include MUME_STDARG_H
#include MUME_STDIO_H
#include MUME_STDLIB_H
#include MUME_STRING_H

typedef struct _memory_stream_s {
    mume_stream_t base;
    char *buf;
    size_t len;
    size_t cur;
    void (*des)(void*);
} _memory_stream_t;

typedef struct _file_stream_s {
    mume_stream_t base;
    FILE *fp;
} _file_stream_t;

static char* _stream_printf(
    char *buf, int *len, const char *fmt, va_list ap)
{
    char *newbuf;
    size_t buflen;
    int result;

    result = vsnprintf(buf, *len, fmt, ap);
    if (result >= 0 && result < *len) {
        *len = result;
        return buf;
    }

    buflen = (size_t)result + 1;
    newbuf = malloc_abort(buflen);
    result = vsnprintf(newbuf, buflen, fmt, ap);
    if (result >= 0 && (size_t)result < buflen) {
        *len = result;
        return newbuf;
    }

    mume_error(("Format string failed: %s: %d\n", fmt, result));

    *len = 0;
    free(newbuf);
    return NULL;
}

static size_t _memory_stream_length(void *self)
{
    return ((_memory_stream_t*)self)->len;
}

static int _memory_stream_eof(void *self)
{
    _memory_stream_t *stm = self;
    return stm->cur == stm->len;
}

static size_t _memory_stream_tell(void *self)
{
    return ((_memory_stream_t*)self)->cur;
}

static int _memory_stream_seek(void *self, size_t pos)
{
    _memory_stream_t *stm = self;
    if (pos <= stm->len) {
        stm->cur = pos;
        return 1;
    }
    return 0;
}

static size_t _memory_stream_read(
    void *self, void *data, size_t len)
{
    _memory_stream_t *stm = self;
    if (stm->len - stm->cur < len)
        len = stm->len - stm->cur;
    memcpy(data, stm->buf + stm->cur, len);
    stm->cur += len;
    return len;
}

static size_t _memory_stream_write(
    void *self, const void *data, size_t len)
{
    _memory_stream_t *stm = self;
    if (stm->len - stm->cur < len)
        len = stm->len - stm->cur;
    memcpy(stm->buf + stm->cur, data, len);
    stm->cur += len;
    return len;
}

static void _memory_stream_close(void *self)
{
    _memory_stream_t *stm = self;
    if (stm->des)
        stm->des(stm->buf);
    free(stm);
}

static size_t _file_stream_length(void *self)
{
    FILE *fp = ((_file_stream_t*)self)->fp;
    long len, cur = ftell(fp);
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    return len;
}

static int _file_stream_eof(void *self)
{
    return feof(((_file_stream_t*)self)->fp);
}

static size_t _file_stream_tell(void *self)
{
    return ftell(((_file_stream_t*)self)->fp);
}

static int _file_stream_seek(void *self, size_t pos)
{
    return 0 == fseek(((_file_stream_t*)self)->fp, pos, SEEK_SET);
}

static size_t _file_stream_read(
    void *self, void *data, size_t len)
{
    return fread(data, 1, len, ((_file_stream_t*)self)->fp);
}

static size_t _file_stream_write(
    void *self, const void *data, size_t len)
{
    return fwrite(data, 1, len, ((_file_stream_t*)self)->fp);
}

static void _file_stream_close(void *self)
{
    fclose(((_file_stream_t*)self)->fp);
    free(self);
}

/* This byteorder stuff was lifted from SDL. */
#define MUME_LITTLE_ENDIAN  1234
#define MUME_BIG_ENDIAN  4321

#if defined(__i386__) || defined(__ia64__) || defined(_M_IX86) || \
    defined(_M_IA64) || (defined(__alpha__) || defined(__alpha)) || \
    defined(__arm__) || defined(ARM) || \
    (defined(__mips__) && defined(__MIPSEL__)) || defined(__SYMBIAN32__) \
    || defined(__x86_64__) || defined(__LITTLE_ENDIAN__)
# define MUME_BYTEORDER MUME_LITTLE_ENDIAN
#else
# define MUME_BYTEORDER MUME_BIG_ENDIAN
#endif

static void _swap16(void *val)
{
    uint16_t x = *(uint16_t*)val;
    *(uint16_t*)val = (x << 8) | (x >> 8);
}

static void _swap32(void *val)
{
    uint32_t x = *(uint32_t*)val;
    *(uint32_t*)val = (x << 24) | ((x << 8) & 0x00FF0000) |
                      ((x >> 8) & 0x0000FF00) | (x >> 24);
}

static void _swap64(void *val)
{
    uint64_t x = *(uint64_t*)val;
    uint32_t hi, lo;
    lo = (uint32_t)(x & 0xFFFFFFFF);
    x >>= 32;
    hi = (uint32_t)(x & 0xFFFFFFFF);
    _swap32(&hi);
    _swap32(&lo);
    *(uint64_t*)val = ((uint64_t)lo << 32) | hi;
}

static void (*_swap_fcns[])(void*) = {
    _swap16, _swap32, _swap64
};

static int _read_number(
    mume_stream_t *stm, void *buf, size_t len, int endian)
{
    assert(2 == len || 4 == len || 8 == len);
    if (len != mume_stream_read(stm, buf, len)) {
        mume_warning(("Read number error: %u\n", len));
        return 0;
    }

    if (endian != MUME_BYTEORDER)
        _swap_fcns[len/4](buf);
    return 1;
}

static int _write_number(
    mume_stream_t *stm, void *buf, size_t len, int endian)
{
    assert(2 == len || 4 == len || 8 == len);
    if (endian != MUME_BYTEORDER) {
        _swap_fcns[len/4](buf);
    }

    if (len != mume_stream_write(stm, buf, len)) {
        mume_warning(("Write number error: %u\n", len));
        return 0;
    }
    return 1;
}

#define _stream_read_number(_stm, _val, _endian) \
    _read_number(_stm, _val, sizeof(*_val), _endian)

#define _stream_write_number(_stm, _val, _endian) \
    _write_number(_stm, &_val, sizeof(_val), _endian)

mume_stream_t* mume_memory_stream_open(
    void *buf, size_t len, void (*des)(void*))
{
    static struct mume_stream_i _impl = {
        _memory_stream_length,
        _memory_stream_eof,
        _memory_stream_tell,
        _memory_stream_seek,
        _memory_stream_read,
        _memory_stream_write,
        _memory_stream_close,
    };
    _memory_stream_t *stm = malloc_struct(_memory_stream_t);
    stm->buf = buf;
    stm->len = len;
    stm->cur = 0;
    stm->des = des;
    stm->base.impl = &_impl;
    stm->base.refcount = 0;
    return (mume_stream_t*)stm;
}

mume_stream_t* mume_file_stream_open(const char *text, int mode)
{
    static struct mume_stream_i _impl = {
        _file_stream_length,
        _file_stream_eof,
        _file_stream_tell,
        _file_stream_seek,
        _file_stream_read,
        _file_stream_write,
        _file_stream_close,
    };
    const char *fm = "";
    FILE *fp;
    _file_stream_t *stm;
    switch (mode) {
    case MUME_OM_READ:
        fm = "r";
        break;
    case MUME_OM_WRITE:
        fm = "w";
        break;
    case MUME_OM_APPEND:
        fm = "a";
        break;
    }

    fp = fopen(text, fm);
    if (NULL == fp) {
        mume_warning(("Open file error: \"%s\" (%s)\n", text, fm));
        return NULL;
    }

    stm = malloc_struct(_file_stream_t);
    stm->fp = fp;
    stm->base.impl = &_impl;
    stm->base.refcount = 0;

    return (mume_stream_t*)stm;
}

int mume_process_file_stream(
    int (*proc)(void*, mume_stream_t*), void *closure,
    const char *file, int mode)
{
    mume_stream_t *stm;
    int result;

    stm = mume_file_stream_open(file, mode);
    if (NULL == stm)
        return 0;

    result = proc(closure, stm);
    mume_stream_close(stm);

    return result;
}

int mume_stream_printf(mume_stream_t *stm, const char *fmt, ...)
{
    char buf[256];
    char *str;
    va_list args;
    int len = sizeof(buf);

    va_start(args, fmt);
    str = _stream_printf(buf, &len, fmt, args);
    if (str) {
        len = mume_stream_write(stm, str, len);
        if (str != buf)
            free(str);
    }

    va_end(args);
    return len;
}

int mume_stream_read_le_int16(
    mume_stream_t *stm, int16_t *val)
{
    return _stream_read_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_read_be_int16(
    mume_stream_t *stm, int16_t *val)
{
    return _stream_read_number(stm, val, MUME_BIG_ENDIAN);
}

int mume_stream_read_le_int32(
    mume_stream_t *stm, int32_t *val)
{
    return _stream_read_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_read_be_int32(
    mume_stream_t *stm, int32_t *val)
{
    return _stream_read_number(stm, val, MUME_BIG_ENDIAN);
}

int mume_stream_read_le_int64(
    mume_stream_t *stm, int64_t *val)
{
    return _stream_read_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_read_be_int64(
    mume_stream_t *stm, int64_t *val)
{
    return _stream_read_number(stm, val, MUME_BIG_ENDIAN);
}

int mume_stream_write_le_int16(
    mume_stream_t *stm, int16_t val)
{
    return _stream_write_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_write_be_int16(
    mume_stream_t *stm, int16_t val)
{
    return _stream_write_number(stm, val, MUME_BIG_ENDIAN);
}

int mume_stream_write_le_int32(
    mume_stream_t *stm, int32_t val)
{
    return _stream_write_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_write_be_int32(
    mume_stream_t *stm, int32_t val)
{
    return _stream_write_number(stm, val, MUME_BIG_ENDIAN);
}

int mume_stream_write_le_int64(
    mume_stream_t *stm, int64_t val)
{
    return _stream_write_number(stm, val, MUME_LITTLE_ENDIAN);
}

int mume_stream_write_be_int64(
    mume_stream_t *stm, int64_t val)
{
    return _stream_write_number(stm, val, MUME_BIG_ENDIAN);
}
