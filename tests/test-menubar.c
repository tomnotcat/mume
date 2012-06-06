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

static void _setup_menuitem(void *item, int count)
{
    int i;
    char buf[256];
    void *sub;

    for (i = 0; i < count; ++i) {
        snprintf(buf, sizeof(buf), "Item %d", i);
        sub = mume_menuitem_new(i, buf);
        mume_menuitem_add_subitem(item, sub);
        mume_refobj_release(sub);
    }
}

static void _setup_menuitems(void *item, ...)
{
    va_list ap;
    int count, i = 0;
    void *sub;

    va_start(ap, item);

    while ((count = va_arg(ap, int)) != -1) {
        sub = mume_menuitem_get_subitem(item, i++);
        _setup_menuitem(sub, count);
    }

    va_end(ap);
}

static void _test_menuitem(void)
{
    void *item;
    void *sub;
    int i;

    item = mume_menuitem_new(0, "root");

    _setup_menuitem(item, 100);
    test_assert(mume_menuitem_count_subitems(item) == 100);

    mume_menuitem_remove_subitems(item, 50, 50);
    test_assert(mume_menuitem_count_subitems(item) == 50);

    for (i = 0; i < 50; ++i) {
        sub = mume_menuitem_get_subitem(item, i);
        _setup_menuitem(sub, 10);
    }

    mume_delete(item);
}

static void _test_menubar(void)
{
    void *win, *bar;
    void *item;
    int height;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    win = mume_ratiobox_new(mume_root_window(), 0, 0, 400, 400);

    bar = mume_menubar_new(win, 0, 0, 400, 0);

    item = mume_menuitem_new(0, "File");
    _setup_menuitem(item, 10);
    _setup_menuitems(item, 10, 5, 2, 1, 0, 40, -1);
    mume_menubar_add_item(bar, item);
    mume_refobj_release(item);

    item = mume_menuitem_new(0, "Edit");
    _setup_menuitem(item, 5);
    _setup_menuitems(item, 0, 5, 20, 1, -1);
    mume_menubar_add_item(bar, item);
    mume_refobj_release(item);

    item = mume_menuitem_new(0, "View");
    _setup_menuitem(item, 10);
    mume_menubar_add_item(bar, item);
    mume_refobj_release(item);

    item = mume_menuitem_new(0, "Help");
    _setup_menuitem(item, 20);
    mume_menubar_add_item(bar, item);
    mume_refobj_release(item);

    mume_window_size_hint(
        bar, NULL, &height, NULL, NULL, NULL, NULL);

    mume_window_resize_to(
        bar, mume_window_width(bar), height);

    mume_ratiobox_setup(win, bar, 0, 0, 1, 0);

    mume_window_center(win, mume_root_window());
    mume_map_children(win);
    mume_window_map(win);

    test_run_event_loop(win);
}

void all_tests(void)
{
    _test_menuitem();
    _test_menubar();
}
