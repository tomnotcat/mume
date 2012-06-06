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
#include MUME_STRING_H

static int _is_little_endian(void)
{
    const int n = 1;
    return *(char*)&n;
}

void test_stream_byteorder(void)
{
    mume_stream_t *stm;
    char buf[sizeof(int16_t) + sizeof(uint16_t) +
             sizeof(int32_t) + sizeof(uint32_t) +
             sizeof(int64_t) + sizeof(uint64_t)];
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    stm = mume_memory_stream_open(buf, sizeof(buf), NULL);
    /* little endian */
    mume_stream_write_le_int16(stm, -2);
    mume_stream_write_le_uint16(stm, 0x1234);
    mume_stream_write_le_int32(stm, -4);
    mume_stream_write_le_uint32(stm, 0x12345678);
    mume_stream_write_le_int64(stm, -8);
    mume_stream_write_le_uint64(stm, 0x12345678ABCDEFAB);
    if (!_is_little_endian()) {
        test_assert(0x3412 == *(uint16_t*)(buf + 2));
        test_assert(0x78563412 == *(uint32_t*)(buf + 8));
        test_assert(0xABEFCDAB78563412 == *(uint64_t*)(buf + 20));
    }
    mume_stream_seek(stm, 0);
    i16 = 0;
    mume_stream_read_le_int16(stm, &i16);
    test_assert(-2 == i16);
    u16 = 0;
    mume_stream_read_le_uint16(stm, &u16);
    test_assert(0x1234 == u16);
    i32 = 0;
    mume_stream_read_le_int32(stm, &i32);
    test_assert(-4 == i32);
    u32 = 0;
    mume_stream_read_le_uint32(stm, &u32);
    test_assert(0x12345678 == u32);
    i64 = 0;
    mume_stream_read_le_int64(stm, &i64);
    test_assert(-8 == i64);
    u64 = 0;
    mume_stream_read_le_uint64(stm, &u64);
    test_assert(0x12345678ABCDEFAB == u64);
    /* big endian */
    mume_stream_seek(stm, 0);
    mume_stream_write_be_int16(stm, -2);
    mume_stream_write_be_uint16(stm, 0x1234);
    mume_stream_write_be_int32(stm, -4);
    mume_stream_write_be_uint32(stm, 0x12345678);
    mume_stream_write_be_int64(stm, -8);
    mume_stream_write_be_uint64(stm, 0x12345678ABCDEFAB);
    if (_is_little_endian()) {
        test_assert(0x3412 == *(uint16_t*)(buf + 2));
        test_assert(0x78563412 == *(uint32_t*)(buf + 8));
        test_assert(0xABEFCDAB78563412 == *(uint64_t*)(buf + 20));
    }
    mume_stream_seek(stm, 0);
    i16 = 0;
    mume_stream_read_be_int16(stm, &i16);
    test_assert(-2 == i16);
    u16 = 0;
    mume_stream_read_be_uint16(stm, &u16);
    test_assert(0x1234 == u16);
    i32 = 0;
    mume_stream_read_be_int32(stm, &i32);
    test_assert(-4 == i32);
    u32 = 0;
    mume_stream_read_be_uint32(stm, &u32);
    test_assert(0x12345678 == u32);
    i64 = 0;
    mume_stream_read_be_int64(stm, &i64);
    test_assert(-8 == i64);
    u64 = 0;
    mume_stream_read_be_uint64(stm, &u64);
    test_assert(0x12345678ABCDEFAB == u64);
    mume_stream_close(stm);
}
