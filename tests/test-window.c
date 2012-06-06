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

static void _test_map_unmap(void)
{
    void *win;

    win = mume_window_new(
        mume_root_window(), 10, 20, 100, 100);

    test_assert(!mume_window_is_mapped(win));

    mume_window_map(win);
    test_assert(mume_window_is_mapped(win));

    mume_window_unmap(win);
    test_assert(!mume_window_is_mapped(win));

    /* Test with backwin. */
    win = test_setup_toplevel_window(win);

    mume_window_map(win);
    test_assert(mume_window_is_mapped(win));

    mume_window_unmap(win);
    test_assert(!mume_window_is_mapped(win));

    win = test_teardown_toplevel_window(win);

    mume_delete(win);
}

static void _test_translate_coord(void)
{
    void *win;
    void *c1, *c2, *c3, *c4;
    int x, y;

    win = mume_window_new(
        mume_root_window(), 10, 20, 100, 100);

    x = 10, y = 10;
    mume_window_move_to(win, 30, 40);
    mume_translate_coords(win, NULL, &x, &y);
    test_assert(40 == x && y == 50);

    mume_translate_coords(NULL, win, &x, &y);
    test_assert(10 == x && 10 == y);

    c1 = mume_window_new(win, 10, 20, 50, 50);
    c2 = mume_window_new(c1, 5, 10, 40, 40);
    c3 = mume_window_new(c2, 5, 5, 30, 20);
    c4 = mume_window_new(c2, 10, 0, 20, 20);

    x = 5, y = 10;
    mume_translate_coords(c4, c3, &x, &y);
    test_assert(10 == x && 5 == y);

    mume_translate_coords(c3, c4, &x, &y);
    test_assert(5 == x && 10 == y);

    mume_translate_coords(c4, c1, &x, &y);
    test_assert(20 == x && 20 == y);

    mume_translate_coords(c1, c3, &x, &y);
    test_assert(10 == x && 5 == y);

    mume_delete(win);
}

static int _check_stack_order(void **ss, void *pnt)
{
    unsigned int cc, i = 0;
    void **cs = mume_query_children(pnt, &cc, 0);

    if (cs) {
        for (i = 0; i < cc; ++i)
            if (ss[i] != cs[i])
                break;

        mume_free_children(pnt, cs);
    }

    return i == cc;
}

static void _test_stack_order(void)
{
    void *win;
    void *c1, *c2, *c3, *c4;
    void *stack[4];

    win = mume_window_new(
        mume_root_window(), 0, 0, 100, 100);
    c1 = mume_window_new(win, 0, 0, 100, 100);
    c2 = mume_window_new(win, 0, 0, 100, 100);
    c3 = mume_window_new(win, 0, 0, 100, 100);
    c4 = mume_window_new(win, 0, 0, 100, 100);

    stack[0] = c1;
    stack[1] = c2;
    stack[2] = c3;
    stack[3] = c4;

    test_assert(_check_stack_order(stack, win));

    mume_window_raise(c2);
    stack[1] = c3;
    stack[2] = c4;
    stack[3] = c2;
    test_assert(_check_stack_order(stack, win));

    mume_window_lower(c4);
    stack[0] = c4;
    stack[1] = c1;
    stack[2] = c3;
    test_assert(_check_stack_order(stack, win));

    mume_delete(win);
}

static void _test_window_focus(void)
{
    int i, j, k;
    void *cw1[4];
    void *cw2[4][4];
    void *all[22];
    void *it, *win, *odd;

    win = mume_window_new(mume_root_window(), 0, 0, 0, 0);
    odd = mume_window_new(mume_root_window(), 0, 0, 0, 0);
    for (i = 0; i < 4; ++i) {
        cw1[i] = mume_window_new(win, 0, 0, 0, 0);

        for (j = 0; j < 4; ++j) {
            cw2[i][j] = mume_window_new(cw1[i], 0, 0, 0, 0);
        }
    }

    /* Iterate window tree. */
    all[0] = mume_root_window();
    all[1] = win;
    k = 2;
    for (i = 0; i < 4; ++i) {
        all[k++] = cw1[i];

        for (j = 0; j < 4; ++j) {
            all[k++] = cw2[i][j];
        }
    }

    it = mume_root_window();
    for (i = 0; i < 22; ++i) {
        mume_window_map(all[i]);
        test_assert(it == all[i]);
        it = mume_window_next_node(it, 1);
    }

    test_assert(odd == it);
    it = mume_window_last_leaf(win);
    for (i = 21; i >= 0; --i) {
        test_assert(it == all[i]);
        it = mume_window_prev_node(it, 1);
    }

    test_assert(NULL == it);

    /* Iterate auto focus windows. */
    mume_window_focusable(mume_root_window(), 1);
    mume_window_focusable(win, 1);
    mume_window_focusable(odd, 1);
    mume_window_focusable(cw1[0], 1);
    mume_window_focusable(cw1[1], 1);
    mume_window_focusable(cw1[3], 1);
    mume_window_focusable(cw2[1][0], 1);
    mume_window_focusable(cw2[1][3], 1);
    mume_window_focusable(cw2[2][0], 1);
    mume_window_focusable(cw2[2][2], 1);
    mume_window_focusable(cw2[3][1], 1);
    mume_window_focusable(cw2[3][2], 1);
    all[0] = cw1[0];
    all[1] = cw1[1];
    all[2] = cw2[1][0];
    all[3] = cw2[1][3];
    all[4] = cw2[2][0];
    all[5] = cw2[2][2];
    all[6] = cw1[3];
    all[7] = cw2[3][1];
    all[8] = cw2[3][2];
    test_assert(mume_window_next_focus(win, NULL) == all[0]);

    it = all[1];
    for (i = 2; i < 9; ++i) {
        it = mume_window_next_focus(win, it);
        test_assert(it == all[i]);
    }

    it = mume_window_next_focus(win, it);
    test_assert(it == all[0]);
    it = mume_window_next_focus(win, it);
    test_assert(it == all[1]);
    test_assert(mume_window_prev_focus(win, NULL) == all[8]);

    it = all[7];
    for (i = 6; i >= 0; --i) {
        it = mume_window_prev_focus(win, it);
        test_assert(it == all[i]);
    }

    it = mume_window_prev_focus(win, it);
    test_assert(it == all[8]);

    it = mume_window_prev_focus(win, it);
    test_assert(it == all[7]);
}

void all_tests(void)
{
    test_run(_test_map_unmap);
    test_run(_test_translate_coord);
    test_run(_test_stack_order);
    test_run(_test_window_focus);
}
