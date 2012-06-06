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
#include "mume-home-view.h"
#include "mume-bookmgr.h"
#include "mume-bookshelf.h"
#include "mume-gstate.h"
#include "mume-profile.h"

#define _MIN_TREEVIEW_WIDTH 100
#define _MIN_LISTVIEW_WIDTH 100
#define _LAYOUT_WIDTH 800
#define _LAYOUT_HEIGHT 600
#define _SPLITTER_RATIO_POS (0.24)

#define _home_view_super_class mume_ratiobox_class

struct _home_view {
    const char _[MUME_SIZEOF_RATIOBOX];
    void *splitter;
    void *treeview;
    void *listview;
};

MUME_STATIC_ASSERT(sizeof(struct _home_view) ==
                   MUME_SIZEOF_HOME_VIEW);

static void _home_view_update_bookviews(struct _home_view *self)
{
    void *shelf;
    void *root, *node;
    void *bookmgr = mume_bookmgr();

    root = mume_treeview_root(self->treeview);
    node = NULL;

    /* My books. */
    shelf = mume_bookmgr_my_shelf(bookmgr);
    node = mume_treeview_insert(
        self->treeview, root, node,
        mume_bookshelf_get_name(shelf),
        MUME_TREENODE_STATIC_NAME);

    /* Recent reading. */
    shelf = mume_bookmgr_recent_shelf(bookmgr);
    node = mume_treeview_insert(
        self->treeview, root, node,
        mume_bookshelf_get_name(shelf),
        MUME_TREENODE_STATIC_NAME);

    /* Reading history. */
    shelf = mume_bookmgr_history_shelf(bookmgr);
    node = mume_treeview_insert(
        self->treeview, root, node,
        mume_bookshelf_get_name(shelf),
        MUME_TREENODE_STATIC_NAME);
}

static void* _home_view_ctor(
    struct _home_view *self, int mode, va_list *app)
{
    void *label;
    void *resmgr;
    int width, height;
    float ratio_pos;

    if (!_mume_ctor(_home_view_super_class(), self, mode, app))
        return NULL;

    mume_window_get_geometry(
        self, NULL, NULL, &width, &height);

    /* Just a decorator. */
#define _LABEL_HEIGHT 4

    resmgr = mume_resmgr();
    label = mume_label_new(self, 0, 0, width, _LABEL_HEIGHT);

    mume_ratiobox_setup(self, label, 0, 0, 1, 0);

    mume_label_set_bkgnd(
        label, mume_resmgr_get_brush(
            resmgr, "home_view", "top_bg"));

    /* Splitter. */
    self->splitter = mume_splitter_new(
        self, 0, _LABEL_HEIGHT,
        width, height - _LABEL_HEIGHT,
        MUME_SPLITTER_LEFT);

    mume_ratiobox_setup(self, self->splitter, 0, 0, 1, 1);

#undef _STATIC_HEIGHT

    mume_splitter_set_size(
        self->splitter, 0,
        _MIN_TREEVIEW_WIDTH,
        MUME_SPLITTER_MAX_POS);

    mume_splitter_set_size(
        self->splitter, 1,
        _MIN_LISTVIEW_WIDTH,
        MUME_SPLITTER_MAX_POS);

    ratio_pos = mume_profile_get_float(
        mume_profile(),
        "home_view", "splitter_pos",
        _SPLITTER_RATIO_POS);

    mume_splitter_set_ratio_pos(
        self->splitter, ratio_pos);

    /* Treeview. */
    self->treeview = mume_treeview_new(
        self->splitter, 0, 0, 0, 0);

    self->listview = mume_listview_new(
        self->splitter, 0, 0, 0, 0);

    /* Update treeview and listview with data from bookmgr. */
    _home_view_update_bookviews(self);

    mume_map_children(self->splitter);
    mume_map_children(self);

    return self;
}

static void* _home_view_dtor(struct _home_view *self)
{
    return _mume_dtor(_home_view_super_class(), self);
}

static void _home_view_handle_notify(
    struct _home_view *self, void *window, int code, void *data)
{
    if (window == self->splitter) {
        switch (code) {
        case MUME_SPLITTER_POS_CHANGED:
            mume_profile_set_float(
                mume_profile(),
                "home_view", "splitter_pos",
                mume_splitter_get_ratio_pos(window));
            break;
        }
    }
    else if (window == self->treeview) {
        switch (code) {
        case MUME_TREEVIEW_CONTEXTMENU:
            {
                const mume_point_t *pt = data;
                int x, y;
                void *popup;

                x = pt->x;
                y = pt->y;

                mume_translate_coords(
                    self->treeview, mume_root_window(), &x, &y);

                popup = mume_menubar_new_popup(self);
                mume_menubar_load(popup, "home_view_treeview_menu");
                mume_menubar_popup(popup, x, y, NULL, 1);
            }
            break;
        }
    }
}

const void* mume_home_view_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_home_view_meta_class(),
        "home view",
        _home_view_super_class(),
        sizeof(struct _home_view),
        MUME_PROP_END,
        _mume_ctor, _home_view_ctor,
        _mume_dtor, _home_view_dtor,
        _mume_window_handle_notify,
        _home_view_handle_notify,
        MUME_FUNC_END);
}

void* mume_home_view_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(mume_home_view_class(), parent, x, y, w, h);
}
