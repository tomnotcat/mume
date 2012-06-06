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
#include "mume-gui.h"
#include "mume-reader.h"
#include "test-util.h"

void all_tests(void)
{
    void *win, *tab, *doc, *view;
    mume_virtfs_t *vfs;
    mume_stream_t *stm;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "reader.xml"));

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    tab = mume_tabctrl_new(win, 5, 5, 390, 390, MUME_TABCTRL_TOP);
    mume_ratiobox_setup(win, tab, 0, 0, 1, 1);
    vfs = mume_virtfs_create(TESTS_DATA_DIR);
    test_assert(vfs);

    /* pdf */
    view = mume_docview_new(tab, 0, 0, 0, 0);
    doc = mume_new(mume_pdf_doc_class());
    stm = mume_virtfs_open_read(vfs, "test.pdf");
    test_assert(stm);
    test_assert(mume_docdoc_load(doc, stm));
    mume_stream_close(stm);
    test_assert(NULL == mume_docview_get_doc(view));
    mume_docview_set_doc(view, NULL);
    mume_docview_set_doc(view, doc);
    mume_refobj_release(doc);

    /* txt */
    view = mume_docview_new(tab, 0, 0, 0, 0);
    doc = mume_new(mume_txt_doc_class());
    stm = mume_virtfs_open_read(vfs, "test.txt");
    test_assert(stm);
    test_assert(mume_docdoc_load(doc, stm));
    mume_stream_close(stm);
    mume_docview_set_doc(view, doc);
    mume_refobj_release(doc);

    mume_window_center(win, mume_root_window());
    mume_window_map(win);
    mume_map_children(win);

    test_run_event_loop(win);

    /* Cleanup. */
    mume_virtfs_destroy(vfs);
}
