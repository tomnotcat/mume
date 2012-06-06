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
#include "mume-mainform.h"
#include "mume-home-view.h"

#define _MAINFORM_TABCTRL_HEIGHT 32

#define _mainform_super_class mume_ratiobox_class

struct _mainform {
    const char _[MUME_SIZEOF_RATIOBOX];
    void *tabs_bar;
    void *views;
    void *active_view;
};

MUME_STATIC_ASSERT(sizeof(struct _mainform) == MUME_SIZEOF_MAINFORM);

static void* _mainform_ctor(
    struct _mainform *self, int mode, va_list *app)
{
    void *bwin;
    cairo_surface_t *icon;
    int width, height;

    if (!_mume_ctor(_mainform_super_class(), self, mode, app))
        return NULL;

    mume_window_get_geometry(
        self, NULL, NULL, &width, &height);

    /* Tabs bar. */
    self->tabs_bar = mume_tabctrl_new(
        self, 0, 0, width, _MAINFORM_TABCTRL_HEIGHT,
        MUME_TABCTRL_TOP);

    mume_ratiobox_setup(self, self->tabs_bar, 0, 0, 1, 0);

    icon = mume_resmgr_get_icon(
        mume_resmgr(), "images", "tabctrl.plus");

    if (NULL == icon)
        mume_warning(("Get tabctrl new tab icon failed\n"));

    mume_tabctrl_append_tool(self->tabs_bar, icon);
    mume_window_map(self->tabs_bar);

    /* Backwin. */
    bwin = mume_backend_create_backwin(
        mume_backend(), MUME_BACKWIN_NORMAL,
        mume_root_backwin(), 0, 0, 1, 1);

    if (bwin) {
        mume_window_set_backwin(self, bwin);
        mume_window_sync_backwin(self, 0);
        mume_refobj_release(bwin);
    }

    self->views = mume_olist_new(NULL);
    self->active_view = NULL;

    return self;
}

static void* _mainform_dtor(struct _mainform *self)
{
    mume_delete(self->views);
    return _mume_dtor(_mainform_super_class(), self);
}

static void _mainform_handle_sizehint(
    struct _mainform *self, int *pref_w, int *pref_h,
    int *min_w, int *min_h, int *max_w, int *max_h)
{
    *min_w = MUME_MAINFORM_WIDTH / 2;
    *min_h = MUME_MAINFORM_HEIGHT / 2;
}

static void _mainform_handle_command(
    struct _mainform *self, void *window, int command)
{
    if (window == self->tabs_bar) {
        void *p;
        int c = mume_tabctrl_tab_count(self->tabs_bar);

        if (command < c) {
            /* Close tab. */
            p = mume_tabctrl_tab_at(self->tabs_bar, command);
            p = mume_tabctrl_get_window(self->tabs_bar, p);
            mume_mainform_remove_view(self, p);
        }
        else {
            /* User click the "New Tab" button. */
            p = mume_mainform_new_tab(self);
            mume_mainform_set_active(self, p);
        }
    }
}

static void _mainform_handle_close(struct _mainform *self)
{
    /* User close the main window. */
    mume_post_event(mume_make_quit_event());
}

const void* mume_mainform_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_mainform_meta_class(),
        "mainform",
        _mainform_super_class(),
        sizeof(struct _mainform),
        MUME_PROP_END,
        _mume_ctor, _mainform_ctor,
        _mume_dtor, _mainform_dtor,
        _mume_window_handle_sizehint,
        _mainform_handle_sizehint,
        _mume_window_handle_command,
        _mainform_handle_command,
        _mume_window_handle_close,
        _mainform_handle_close,
        MUME_FUNC_END);
}

void* mume_mainform_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(
        mume_mainform_class(), parent, x, y, width, height);
}

void mume_mainform_add_view(void *_self, void *view)
{
    struct _mainform *self = _self;
    int width, height;

    assert(mume_is_of(_self, mume_mainform_class()));
    assert(mume_is_of(view, mume_window_class()));

    mume_window_get_geometry(
        self, NULL, NULL, &width, &height);

    mume_olist_push_back(self->views, view);

    mume_window_reparent(
        view, self, 0, _MAINFORM_TABCTRL_HEIGHT);

    mume_window_resize_to(
        view, width, height - _MAINFORM_TABCTRL_HEIGHT);

    mume_ratiobox_setup(self, view, 0, 0, 1, 1);

    if (mume_tabctrl_append_tab(self->tabs_bar, view) > 0 )
        mume_tabctrl_enable_close(self->tabs_bar, 1);

    if (self->active_view) {
        mume_window_unmap(view);
    }
    else {
        mume_mainform_set_active(self, view);
    }
}

void mume_mainform_remove_view(void *_self, void *view)
{
    struct _mainform *self = _self;
    void *it, *next;
    int i;

    assert(mume_is_of(_self, mume_mainform_class()));

    it = mume_octnr_find(self->views, view);
    if (NULL == it)
        return;

    next = mume_octnr_next(self->views, it);
    if (next)
        next = mume_octnr_value(self->views, next);
    else
        next = mume_olist_back(self->views);

    if (next) {
        mume_mainform_set_active(self, next);
    }
    else {
        mume_mainform_set_active(self, NULL);
    }

    i = mume_tabctrl_tab_from_window(self->tabs_bar, view);
    if (i != -1) {
        mume_tabctrl_remove_tab(self->tabs_bar, i);

        if (1 == mume_tabctrl_tab_count(self->tabs_bar))
            mume_tabctrl_enable_close(self->tabs_bar, 0);
    }

    mume_ratiobox_remove(self, view);
    mume_octnr_erase(self->views, it);
    mume_window_reparent(view, mume_root_window(), 0, 0);
}

void* mume_mainform_new_tab(void *self)
{
    void *view = mume_home_view_new(NULL, 0, 0, 0, 0);
    mume_window_set_text(view, _("New Tab"));
    mume_mainform_add_view(self, view);
    return view;
}

void mume_mainform_set_active(void *_self, void *view)
{
    struct _mainform *self = _self;

    assert(mume_is_of(_self, mume_mainform_class()));
    assert(mume_octnr_find(self->views, view));

    if (self->active_view != view) {
        int i;

        if (self->active_view)
            mume_window_unmap(self->active_view);

        if (view)
            mume_window_map(view);

        self->active_view = view;

        i = mume_tabctrl_tab_from_window(self->tabs_bar, view);
        if (i != -1)
            mume_tabctrl_select_tab(self->tabs_bar, i);
    }
}

void* mume_mainform_get_active(const void *_self)
{
    const struct _mainform *self = _self;
    assert(mume_is_of(_self, mume_mainform_class()));
    return self->active_view;
}
