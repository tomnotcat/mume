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
#include "mume-reader.h"
#include "test-util.h"

void all_tests(void)
{
    mume_stream_t *stm;
    mume_virtfs_t *vfs;
    void *doc;

    vfs = mume_virtfs_create(TESTS_DATA_DIR);
    test_assert(vfs);

    /* pdf */
    stm = mume_virtfs_open_read(vfs, "test.pdf");
    test_assert(stm);
    test_assert(!mume_docmgr_load(mume_docmgr(), 0, stm));
    mume_docmgr_register(
        mume_docmgr(), MUME_FILETYPE_PDF, mume_pdf_doc_class());
    test_assert(mume_docmgr_load(mume_docmgr(), 0, stm));
    mume_stream_close(stm);
    doc = mume_docmgr_load_file(
        mume_docmgr(), TESTS_DATA_DIR "/test.pdf");
    test_assert(doc);
    test_assert(mume_docdoc_count_pages(doc) == 17);

    /* txt */
    stm = mume_virtfs_open_read(vfs, "test.txt");
    test_assert(stm);
    test_assert(!mume_docmgr_load(mume_docmgr(), 0, stm));
    mume_docmgr_register(
        mume_docmgr(), MUME_FILETYPE_TXT, mume_txt_doc_class());
    test_assert(mume_docmgr_load(mume_docmgr(), 0, stm));
    mume_stream_close(stm);
    doc = mume_docmgr_load_file(
        mume_docmgr(), TESTS_DATA_DIR "/test.txt");
    test_assert(doc);

    mume_virtfs_destroy(vfs);
}
