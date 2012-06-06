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

static void _test_read(mume_virtfs_t *vfs)
{
    mume_stream_t *stm;
    char buffer[32];
    const char *content = "hello world\n你好世界";
    test_assert(vfs);
    stm = mume_virtfs_open_read(vfs, "test-base-virtfs.txt");
    test_assert(stm);
    test_assert(strlen(content) + 1 ==
              mume_stream_read(stm, buffer, 32));
    test_assert(0 == strncmp(
        buffer, content, strlen(content)));
    mume_stream_close(stm);
}

static void _test_write(mume_virtfs_t *vfs)
{
    mume_stream_t *stm;
    char buffer[64];
    const char *filename = "test-base-virtfs-new.txt";
    const char *content1 = "hello world\n你好世界";
    const char *content2 = "this is append\n";
    test_assert(vfs);
    test_assert(!mume_virtfs_exists(vfs, filename) ||
              mume_virtfs_delete(vfs, filename));
    /* write */
    stm = mume_virtfs_open_write(vfs, filename);
    if (NULL == stm) {
        mume_warning(("file system not writable!\n"));
        return;
    }
    test_assert(strlen(content1) ==
              mume_stream_write(stm, content1, strlen(content1)));
    mume_stream_close(stm);
    test_assert(mume_virtfs_exists(vfs, filename));
    /* append */
    stm = mume_virtfs_open_append(vfs, filename);
    test_assert(stm);
    test_assert(strlen(content2) ==
              mume_stream_write(stm, content2, strlen(content2)));
    mume_stream_close(stm);
    /* read */
    stm = mume_virtfs_open_read(vfs, filename);
    test_assert(stm);
    test_assert(strlen(content1) + strlen(content2) ==
              mume_stream_read(stm, buffer, 64));
    test_assert(0 == strncmp(
        buffer, content1, strlen(content1)));
    test_assert(0 == strncmp(
        buffer + strlen(content1), content2, strlen(content2)));
    mume_stream_close(stm);
    test_assert(mume_virtfs_delete(vfs, filename));
    test_assert(!mume_virtfs_exists(vfs, filename));
}

void test_virtfs_native(void)
{
    mume_virtfs_t *vfs;

    test_assert(mume_virtfs_dirlen("/home/test.txt") == 6);
    test_assert(mume_virtfs_dirlen("hello.txt") == 0);
    test_assert(mume_virtfs_dirlen("a/b/hello.txt") == 4);
    test_assert(mume_virtfs_dirlen("/world") == 1);

    mume_debug(("base dir: %s\nuser dir: %s\n",
                mume_virtfs_basedir(),
                mume_virtfs_userdir()));

    vfs = mume_virtfs_create(TESTS_DATA_DIR);

    _test_read(vfs);
    _test_write(vfs);

    mume_virtfs_destroy(vfs);
}

void test_virtfs_zip(void)
{
    mume_virtfs_t *vfs;
    vfs = mume_virtfs_create(TESTS_DATA_DIR "/test-base-virtfs.zip");
    _test_read(vfs);
    mume_virtfs_destroy(vfs);
}
