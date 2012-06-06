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
    void *tree;
    void *root;
    void *last;
    void *node;
    int i, j, k;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    tree = mume_treeview_new(win, 0, 0, 400, 400);
    mume_ratiobox_setup(win, tree, 0, 0, 1.0, 1.0);

    root = mume_treeview_root(tree);
    last = NULL;

    for (i = 0; i < 10; ++i) {
        char text[32];
        snprintf(text, 32, "Chapter %d", i);
        last = mume_treeview_insert(tree, root, last, text, 0);
        for (j = 0; j < 10; ++j) {
            snprintf(text, 32, "Section %d.%d", i, j);
            node = mume_treeview_insert(tree, last, NULL, text, 0);
            for (k = 0; k < 10; ++k) {
                snprintf(text, 32, "Subsection %d.%d.%d", i, j, k);
                mume_treeview_insert(tree, node, NULL, text, 0);
            }
        }
    }

    mume_treeview_expand(tree, root, 1);
    mume_window_center(win, mume_root_window());
    mume_map_children(win);
    mume_window_map(win);

    test_run_event_loop(win);
}
