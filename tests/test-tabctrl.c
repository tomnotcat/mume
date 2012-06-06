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

#define _mywindow_super_class mume_ratiobox_class

static void _mywindow_handle_command(
    void *self, void *window, int command)
{
    int i, c;
    void *tab, *item;

    tab = window;
    c = mume_tabctrl_tab_count(tab);

    if (command < c) {
        mume_tabctrl_remove_tab(tab, command);
    }
    else if (command == c) {
        i = mume_tabctrl_append_tab(tab, NULL);
        item = mume_tabctrl_tab_at(tab, i);
        mume_tabctrl_set_text(tab, item, "New Tab");
        mume_tabctrl_select_tab(tab, i);
    }
    else if (command == c + 1) {
        i = mume_tabctrl_get_selected(tab);
        if (i != -1)
            mume_tabctrl_remove_tab(tab, i);
    }
    else {
        test_assert(0);
    }
}

static const void* mywindow_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_ratiobox_meta_class(),
        "mywindow",
        _mywindow_super_class(),
        MUME_SIZEOF_RATIOBOX,
        MUME_PROP_END,
        _mume_window_handle_command,
        _mywindow_handle_command,
        MUME_FUNC_END);
}

void all_tests(void)
{
    void *win, *htab, *vtab, *item;
    cairo_surface_t *icon;

    test_assert(mume_resmgr_load(
        mume_resmgr(), TESTS_THEME_DIR "/default", "main.xml"));

    icon = mume_resmgr_get_icon(
        mume_resmgr(), "images", "tabctrl.plus");
    test_assert(icon);

    win = mume_new(mywindow_class(),
                   mume_root_window(), 0, 0, 400, 400);

    htab = mume_tabctrl_new(win, 0, 0, 400, 400, MUME_TABCTRL_TOP);
    mume_tabctrl_enable_close(htab, 1);
    mume_ratiobox_setup(win, htab, 0, 0, 1, 1);
    mume_button_new(htab, 0, 0, "tab1");
    item = mume_tabctrl_tab_at(htab, 0);
    mume_tabctrl_set_text(htab, item, "Tab1");
    mume_tabctrl_append_tool(htab, icon);
    mume_tabctrl_append_tool(htab, NULL);

    vtab = mume_tabctrl_new(htab, 0, 0, 0, 0, MUME_TABCTRL_LEFT);
    mume_tabctrl_enable_close(vtab, 1);
    item = mume_tabctrl_tab_at(htab, 1);
    mume_tabctrl_set_text(htab, item, "Tab2");
    mume_tabctrl_append_tab(htab, NULL);
    item = mume_tabctrl_tab_at(htab, 2);
    mume_tabctrl_set_text(htab, item, "Tab3");
    mume_button_new(vtab, 0, 0, "sub1");
    item = mume_tabctrl_tab_at(vtab, 0);
    mume_tabctrl_set_text(vtab, item, "Sub Tab1");
    mume_tabctrl_append_tab(vtab, NULL);
    item = mume_tabctrl_tab_at(vtab, 1);
    mume_tabctrl_set_text(vtab, item, "Sub Tab2");
    mume_tabctrl_append_tool(vtab, icon);
    mume_tabctrl_append_tool(vtab, NULL);

    mume_window_center(win, mume_root_window());
    mume_window_map(win);
    mume_map_children(win);
    test_run_event_loop(win);
}
