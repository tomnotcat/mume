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
    void *win, *vspl, *hspl;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    vspl = mume_splitter_new(
        win, 0, 0, 400, 400, MUME_SPLITTER_LEFT);
    mume_splitter_set_size(vspl, 0, 100, 200);
    mume_splitter_set_size(vspl, 1, 100, MUME_SPLITTER_MAX_POS);
    test_assert(mume_splitter_get_pos(vspl) == 100);
    mume_ratiobox_setup(win, vspl, 0, 0, 1, 1);

    mume_button_new(vspl, 0, 0, "left");

    hspl = mume_splitter_new(
        vspl, 0, 0, 0, 0, MUME_SPLITTER_TOP);
    mume_button_new(hspl, 0, 0, "right top");
    mume_splitter_set_size(hspl, 1, 100, 200);

    mume_map_children(win);
    mume_map_children(vspl);
    mume_map_children(hspl);
    mume_window_center(win, mume_root_window());
    mume_window_map(win);

    test_run_event_loop(win);
}
