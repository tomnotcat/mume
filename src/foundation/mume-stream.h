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
#ifndef MUME_FOUNDATION_STREAM_H
#define MUME_FOUNDATION_STREAM_H

#include "mume-common.h"
#include MUME_STDINT_H

MUME_BEGIN_DECLS

enum mume_open_mode_e {
    MUME_OM_READ = 1,
    MUME_OM_WRITE,
    MUME_OM_APPEND
};

struct mume_stream_i {
    size_t (*length)(void *self);
    int (*eof)(void *self);
    size_t (*tell)(void *self);
    int (*seek)(void *self, size_t pos);
    size_t (*read)(void *self, void *data, size_t len);
    size_t (*write)(void *self, const void *data, size_t len);
    void (*close)(void *self);
};

struct mume_stream_s {
    struct mume_stream_i *impl;
    int refcount;
};

/* Open a memory stream. */
mume_public mume_stream_t* mume_memory_stream_open(
    void *buf, size_t len, void (*des)(void*));

/* Open a local file stream. */
mume_public mume_stream_t* mume_file_stream_open(
    const char *text, int mode);

/* Open the file, and pass the stream to the specified proc. */
mume_public int mume_process_file_stream(
    int (*proc)(void*, mume_stream_t*), void *closure,
    const char *file, int mode);

static inline size_t mume_stream_length(mume_stream_t *stm)
{
    return stm->impl->length(stm);
}

static inline int mume_stream_eof(mume_stream_t *stm)
{
    return stm->impl->eof(stm);
}

static inline size_t mume_stream_tell(mume_stream_t *stm)
{
    return stm->impl->tell(stm);
}

static inline int mume_stream_seek(mume_stream_t *stm, size_t pos)
{
    return stm->impl->seek(stm, pos);
}

static inline size_t mume_stream_read(
    mume_stream_t *stm, void *data, size_t len)
{
    return stm->impl->read(stm, data, len);
}

static inline size_t mume_stream_write(
    mume_stream_t *stm, const void *data, size_t len)
{
    return stm->impl->write(stm, data, len);
}

static inline void mume_stream_close(mume_stream_t *stm)
{
    if (stm) {
        if (stm->refcount > 0) {
            --stm->refcount;
        }
        else {
            stm->impl->close(stm);
        }
    }
}

static inline void mume_stream_reference(mume_stream_t *stm)
{
    ++stm->refcount;
}

/* Format a string and write the result to the specified stream.
 * the format rule is the same as C standard function printf.
 *
 * Return the number of characters written, or negative when
 * error encounted.
 */
mume_public int mume_stream_printf(
    mume_stream_t *stm, const char *fmt, ...);

/* Help functions to read/write different byte order numbers
 * from/to the stream.
 *
 * "le" means means little endian, "be" means big endian.
 */
mume_public int mume_stream_read_le_int16(
    mume_stream_t *stm, int16_t *val);

#define mume_stream_read_le_uint16(_stm, _val) \
    mume_stream_read_le_int16(_stm, (int16_t*)(_val))

mume_public int mume_stream_read_be_int16(
    mume_stream_t *stm, int16_t *val);

#define mume_stream_read_be_uint16(_stm, _val) \
    mume_stream_read_be_int16(_stm, (int16_t*)(_val))

mume_public int mume_stream_read_le_int32(
    mume_stream_t *stm, int32_t *val);

#define mume_stream_read_le_uint32(_stm, _val) \
    mume_stream_read_le_int32(_stm, (int32_t*)(_val))

mume_public int mume_stream_read_be_int32(
    mume_stream_t *stm, int32_t *val);

#define mume_stream_read_be_uint32(_stm, _val) \
    mume_stream_read_be_int32(_stm, (int32_t*)(_val))

mume_public int mume_stream_read_le_int64(
    mume_stream_t *stm, int64_t *val);

#define mume_stream_read_le_uint64(_stm, _val) \
    mume_stream_read_le_int64(_stm, (int64_t*)(_val))

mume_public int mume_stream_read_be_int64(
    mume_stream_t *stm, int64_t *val);

#define mume_stream_read_be_uint64(_stm, _val) \
    mume_stream_read_be_int64(_stm, (int64_t*)(_val))

mume_public int mume_stream_write_le_int16(
    mume_stream_t *stm, int16_t val);

#define mume_stream_write_le_uint16(_stm, _val) \
    mume_stream_write_le_int16(_stm, (int16_t)(_val))

mume_public int mume_stream_write_be_int16(
    mume_stream_t *stm, int16_t val);

#define mume_stream_write_be_uint16(_stm, _val) \
    mume_stream_write_be_int16(_stm, (int16_t)(_val))

mume_public int mume_stream_write_le_int32(
    mume_stream_t *stm, int32_t val);

#define mume_stream_write_le_uint32(_stm, _val) \
    mume_stream_write_le_int32(_stm, (int32_t)(_val))

mume_public int mume_stream_write_be_int32(
    mume_stream_t *stm, int32_t val);

#define mume_stream_write_be_uint32(_stm, _val) \
    mume_stream_write_be_int32(_stm, (int32_t)(_val))

mume_public int mume_stream_write_le_int64(
    mume_stream_t *stm, int64_t val);

#define mume_stream_write_le_uint64(_stm, _val) \
    mume_stream_write_le_int64(_stm, (int64_t)(_val))

mume_public int mume_stream_write_be_int64(
    mume_stream_t *stm, int64_t val);

#define mume_stream_write_be_uint64(_stm, _val) \
    mume_stream_write_be_int64(_stm, (int64_t)(_val))

MUME_END_DECLS

#endif /* MUME_FOUNDATION_STREAM_H */
