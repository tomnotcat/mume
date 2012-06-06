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
    void *main_win;
    void *win;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "reader.xml"));

    main_win = mume_mainform();

    win = test_window_new(main_win, 0, 0, 0, 0);
    mume_mainform_add_view(main_win, win);

    mume_window_map(main_win);
    mume_window_center(main_win, mume_root_window());

    test_run_event_loop(main_win);
}
