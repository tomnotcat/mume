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

static void _test_check_ext(void)
{
    int type;
    void *ftc = mume_filetc_new();
    type = mume_filetc_check_ext(ftc, TESTS_DATA_DIR "/test.pdf");
    test_assert(MUME_FILETYPE_PDF == type);
    type = mume_filetc_check_ext(ftc, TESTS_DATA_DIR "/test.pDf");
    test_assert(MUME_FILETYPE_PDF == type);
    type = mume_filetc_check_ext(ftc, TESTS_DATA_DIR "/test.txt");
    test_assert(MUME_FILETYPE_TXT == type);
    type = mume_filetc_check_ext(ftc, TESTS_DATA_DIR "/test.TxT");
    test_assert(MUME_FILETYPE_TXT == type);
    mume_delete(ftc);
}

static void _test_check_magic(void)
{
    int type;
    mume_stream_t *stm;
    mume_virtfs_t *vfs;
    void *ftc = mume_filetc_new();
    vfs = mume_virtfs_create(TESTS_DATA_DIR);
    test_assert(vfs);
    stm = mume_virtfs_open_read(vfs, "test.pdf");
    test_assert(stm);
    type = mume_filetc_check_magic(ftc, stm);
    test_assert(MUME_FILETYPE_PDF == type);
    mume_stream_close(stm);
    stm = mume_virtfs_open_read(vfs, "test.txt");
    test_assert(stm);
    type = mume_filetc_check_magic(ftc, stm);
    test_assert(MUME_FILETYPE_TXT == type);
    mume_stream_close(stm);
    mume_virtfs_destroy(vfs);
    mume_delete(ftc);
}

void all_tests(void)
{
    test_run(_test_check_ext);
    test_run(_test_check_magic);
}
