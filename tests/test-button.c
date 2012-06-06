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
    void *win, *btn;
    int i, w, c = 4;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    /* push button */
    mume_window_get_geometry(win, NULL, NULL, &w, NULL);
    w /= c;
    for (i = 0; i < c; ++i) {
        btn = mume_button_new(win, i * w + 1, 1, "hello button");
        mume_ratiobox_setup(win, btn, i * 1.0 / c, 0, 1.0 / c, 0);
    }

    /* check box */
    for (i = 0; i < c; ++i) {
        btn = mume_button_new(win, i * w + 1, w + 1, "hello button");
        mume_ratiobox_setup(win, btn, i * 1.0 / c, 0, 1.0 / c, 0);
    }

    /* radio button */
    for (i = 0; i < c; ++i) {
        btn = mume_button_new(
            win, i * w + 1, 2 * w + 1, "hello button");
        mume_ratiobox_setup(win, btn, i * 1.0 / c, 0, 1.0 / c, 0);
    }

    mume_window_center(win, mume_root_window());
    mume_map_children(win);
    mume_window_map(win);
    test_run_event_loop(win);
}
