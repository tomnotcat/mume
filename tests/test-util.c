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
#include "test-util.h"

const char *test_name;

void test_fail(const char *file, int line,
               const char *fcn, const char *msg)
{
    fprintf(stderr, "%s: %s:%d: %s: %s\n",
            test_name, file, line, fcn, msg);
    abort();
}

void* test_setup_toplevel_window(void *win)
{
    void *bwin;
    void *ctnr;
    int x, y, width, height;

    mume_window_get_geometry(win, &x, &y, &width, &height);
    bwin = mume_backend_create_backwin(
        mume_backend(), MUME_BACKWIN_NORMAL,
        mume_root_backwin(), x, y, width, height);

    if (bwin) {
        mume_window_set_backwin(win, bwin);
        mume_window_sync_backwin(win, 0);
        mume_refobj_release(bwin);
        return win;
    }

    ctnr = mume_window_new(
        mume_root_window(), x, y, width, height);

    mume_window_reparent(win, ctnr, 0, 0);

    if (mume_window_is_mapped(win))
        mume_window_map(ctnr);

    return ctnr;
}

void* test_teardown_toplevel_window(void *win)
{
    void *bwin;
    void *ctnr;
    void **cs;
    int x, y;

    bwin = mume_window_get_backwin(win);
    if (bwin) {
        mume_window_set_backwin(win, NULL);
        return win;
    }

    ctnr = win;
    cs = mume_query_children(ctnr, NULL, 0);
    win = cs[0];
    mume_free_children(ctnr, cs);
    mume_window_get_geometry(ctnr, &x, &y, NULL, NULL);
    mume_window_reparent(
        win, mume_window_parent(ctnr), x, y);
    mume_delete(ctnr);

    return win;
}

int test_is_break_event(mume_event_t *evt, void *main)
{
    if (evt->any.window != main &&
        evt->any.window != mume_root_window())
    {
        return 0;
    }

    if (MUME_EVENT_CLOSE == evt->type)
        return 1;

    if (MUME_EVENT_KEYDOWN == evt->type &&
        evt->key.keysym == MUME_KEY_ESCAPE)
        return 1;

    return 0;
}

void test_run_event_loop(void *main)
{
    mume_event_t evt;
    void *bwin = mume_window_get_backwin(main);
    void *owin = main;

    if (NULL == bwin)
        main = test_setup_toplevel_window(main);

    while (mume_wait_event(&evt)) {
        if (test_is_break_event(&evt, owin))
            break;

        mume_disp_event(&evt);
    }

    if (NULL == bwin)
        test_teardown_toplevel_window(main);
}

/* test_window class. */
#define _test_window_super_class mume_window_class

struct _test_window {
    const char _[MUME_SIZEOF_WINDOW];
    mume_color_t color;
};

static void* _test_window_ctor(
    struct _test_window *self, int mode, va_list *app)
{
    if (!_mume_ctor(_test_window_super_class(), self, mode, app))
        return NULL;

    self->color = mume_color_make(0, 0, 0);
    return self;
}

static void _test_window_handle_expose(
    struct _test_window *self, int x, int y, int w, int h, int c)
{
    cairo_t *cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    cairo_rectangle(cr, x, y, w, h);
    cairo_set_source_rgb(
        cr, mume_color_rval(&self->color),
        mume_color_gval(&self->color),
        mume_color_bval(&self->color));
    cairo_fill(cr);

    mume_window_end_paint(self, cr);
}

static const void* _test_window_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_window_meta_class(),
        "test window",
        _test_window_super_class(),
        sizeof(struct _test_window),
        MUME_PROP_END,
        _mume_ctor, _test_window_ctor,
        _mume_window_handle_expose,
        _test_window_handle_expose,
        MUME_FUNC_END);
}

void* test_window_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(_test_window_class(), parent, x, y, w, h);
}

void test_window_set_color(void *_self, mume_color_t color)
{
    struct _test_window *self = _self;
    assert(mume_is_of(_self, _test_window_class()));
    self->color = color;
}
