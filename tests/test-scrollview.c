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
#include "test-util.h"

void all_tests(void)
{
    void *win;
    void *sb;
    void *sv;
    int x, y, w, h;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    w = mume_metrics(MUME_GM_CXVSCROLL);
    h = mume_metrics(MUME_GM_CYHSCROLL);

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    x = y = 10;
    sb = mume_scrollbar_new(
        win, x, y, 380, h, MUME_SCROLLBAR_TOP);
    test_assert(mume_scrollbar_get_size(sb) == 0);
    test_assert(mume_scrollbar_get_page(sb) == 0);
    test_assert(mume_scrollbar_get_pos(sb) == 0);
    mume_scrollbar_set_pos(sb, 100);
    test_assert(mume_scrollbar_get_pos(sb) == 0);
    mume_scrollbar_set_size(sb, 30);
    mume_scrollbar_set_page(sb, 10);
    mume_scrollbar_set_pos(sb, 15);
    test_assert(mume_scrollbar_get_size(sb) == 30);
    test_assert(mume_scrollbar_get_page(sb) == 10);
    test_assert(mume_scrollbar_get_pos(sb) == 15);
    mume_ratiobox_setup(win, sb, 0, 0, 1, 0);

    y += h + 10;
    sb = mume_scrollbar_new(
        win, x, y, 380, h, MUME_SCROLLBAR_BOTTOM);
    mume_scrollbar_set_size(sb, 10000);
    mume_scrollbar_set_page(sb, 10);
    mume_scrollbar_set_pos(sb, 500);
    test_assert(mume_scrollbar_get_size(sb) == 10000);
    test_assert(mume_scrollbar_get_page(sb) == 10);
    test_assert(mume_scrollbar_get_pos(sb) == 500);
    mume_ratiobox_setup(win, sb, 0, 0, 1, 0);

    y += h + 10;
    sb = mume_scrollbar_new(
        win, x, y, w, 400 - y - 10, MUME_SCROLLBAR_LEFT);
    mume_scrollbar_set_size(sb, 100);
    mume_scrollbar_set_page(sb, 10);
    mume_scrollbar_set_pos(sb, 200);
    test_assert(mume_scrollbar_get_size(sb) == 100);
    test_assert(mume_scrollbar_get_page(sb) == 10);
    test_assert(mume_scrollbar_get_pos(sb) == 90);
    mume_ratiobox_setup(win, sb, 0, 0, 0, 1);

    x += w + 10;
    sb = mume_scrollbar_new(
        win, x, y, w, 400 - y - 10, MUME_SCROLLBAR_RIGHT);
    mume_scrollbar_set_size(sb, 10);
    mume_scrollbar_set_page(sb, 100);
    mume_scrollbar_set_pos(sb, 50);
    test_assert(mume_scrollbar_get_size(sb) == 10);
    test_assert(mume_scrollbar_get_page(sb) == 100);
    test_assert(mume_scrollbar_get_pos(sb) == 0);
    mume_ratiobox_setup(win, sb, 0, 0, 0, 1);

    x += w + 10;
    sv = mume_scrollview_new(
        win, x, y, 400 - x - 10, 400 - y - 10);
    mume_scrollview_set_size(sv, 500, 500);
    mume_scrollview_set_page(sv, 400 - x - 10, 400 - y - 10);
    mume_scrollview_set_line(sv, 50, 50);
    mume_ratiobox_setup(win, sv, 0, 0, 1, 1);

    mume_window_center(win, mume_root_window());
    mume_map_children(win);
    mume_window_map(win);

    test_run_event_loop(win);
}
