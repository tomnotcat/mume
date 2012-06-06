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
#include "mume-button.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-objbase.h"
#include "mume-resmgr-private.h"
#include "mume-text-layout.h"
#include "mume-types.h"

#define _button_super_class mume_window_class
#define _button_super_meta_class mume_window_meta_class

enum _button_flags_e {
    _BUTTON_FLAG_HOT,
    _BUTTON_FLAG_PRESSED,
    _BUTTON_FLAG_CHECKED
};

struct _button {
    const char _[MUME_SIZEOF_WINDOW];
    char *text;
    unsigned int flags;
};

struct _button_class {
    const char _[MUME_SIZEOF_WINDOW_CLASS];
};

struct _pushbtn_theme {
    mume_widget_bkgnd_t bkgnd;
    mume_widget_texts_t texts;
};

struct _checkbox_theme {
    mume_widget_bkgnd_t uncheck;
    mume_widget_bkgnd_t check;
    mume_widget_bkgnd_t bkgnd;
    mume_widget_texts_t texts;
};

struct _button_theme {
    struct _pushbtn_theme pushbtn;
    struct _checkbox_theme checkbox;
    struct _checkbox_theme radiobtn;
};

MUME_STATIC_ASSERT(sizeof(struct _button) == MUME_SIZEOF_BUTTON);
MUME_STATIC_ASSERT(sizeof(struct _button_class) == MUME_SIZEOF_BUTTON_CLASS);

static void _button_handle_button_down(
    struct _button *self, int x, int y, int state, int button)
{
    if (button != MUME_BUTTON_LEFT)
        return;

    if (mume_test_flag(self->flags, _BUTTON_FLAG_PRESSED))
        return;

    mume_add_flag(self->flags, _BUTTON_FLAG_PRESSED);
    mume_invalidate_region(self, NULL);
}

static void _button_handle_button_up(
    struct _button *self, int x, int y, int state, int button)
{
    if (button != MUME_BUTTON_LEFT)
        return;

    if (!mume_test_flag(self->flags, _BUTTON_FLAG_PRESSED))
        return;

    mume_remove_flag(self->flags, _BUTTON_FLAG_PRESSED);
    mume_invalidate_region(self, NULL);
}

static void _button_handle_mouse_enter(
    struct _button *self, int x, int y, int state, int mode, int detail)
{
    if (mume_test_flag(self->flags, _BUTTON_FLAG_HOT))
        return;

    mume_add_flag(self->flags, _BUTTON_FLAG_HOT);
    mume_invalidate_region(self, NULL);
}

static void _button_handle_mouse_leave(
    struct _button *self, int x, int y, int state, int mode, int detail)
{
    if (mume_test_flag(self->flags, _BUTTON_FLAG_HOT))
        mume_remove_flag(self->flags, _BUTTON_FLAG_HOT);

    if (mume_test_flag(self->flags, _BUTTON_FLAG_PRESSED) &&
        self != mume_pointer_owner())
    {
        mume_remove_flag(self->flags, _BUTTON_FLAG_PRESSED);
    }

    mume_invalidate_region(self, NULL);
}

static void _button_handle_expose(
    struct _button *self, int x, int y, int width, int height, int count)
{
    cairo_t *cr;
    mume_resobj_brush_t *br;
    mume_resobj_charfmt_t *cf;
    struct _button_theme *thm;
    int w, h;

    if (count)
        return;

    thm = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "button", "theme"),
        mume_typeof_button_theme());

    if (NULL == thm) {
        mume_warning(("Get button theme failed\n"));
        return;
    }

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    if (!mume_window_is_enabled(self)) {
        br = &thm->pushbtn.bkgnd.disabled;
        cf = &thm->pushbtn.texts.disabled;
    }
    else if (mume_test_flag(self->flags, _BUTTON_FLAG_PRESSED)) {
        br = &thm->pushbtn.bkgnd.pressed;
        cf = &thm->pushbtn.texts.pressed;
    }
    else if (mume_test_flag(self->flags, _BUTTON_FLAG_HOT)) {
        br = &thm->pushbtn.bkgnd.hot;
        cf = &thm->pushbtn.texts.hot;
    }
    else {
        br = &thm->pushbtn.bkgnd.normal;
        cf = &thm->pushbtn.texts.normal;
    }

    mume_window_get_geometry(self, NULL, NULL, &w, &h);
    mume_draw_resobj_brush(cr, br, 0, 0, w, h);
    if (self->text) {
        mume_rect_t rect = mume_rect_make(0, 0, w, h);
        unsigned int format =
                MUME_TLF_DRAWTEXT | MUME_TLF_WORDBREAK |
                MUME_TLF_CENTER | MUME_TLF_VCENTER;

        mume_charfmt_draw_text(
            cr, cf, format, self->text, -1, &rect);
    }

    mume_window_end_paint(self, cr);
}

static void* _button_ctor(
    struct _button *self, int mode, va_list *app)
{
    if (!_mume_ctor(_button_super_class(), self, mode, app))
        return NULL;

    self->text = va_arg(*app, char*);
    self->flags = 0;

    if (self->text)
        self->text = strdup_abort(self->text);

    return self;
}

static void* _button_dtor(struct _button *self)
{
    free(self->text);
    return _mume_dtor(_button_super_class(), self);
}

const void* mume_button_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_button_meta_class(),
        "button",
        _button_super_class(),
        sizeof(struct _button),
        MUME_PROP_END,
        _mume_ctor, _button_ctor,
        _mume_dtor, _button_dtor,
        _mume_window_handle_button_down,
        _button_handle_button_down,
        _mume_window_handle_button_up,
        _button_handle_button_up,
        _mume_window_handle_mouse_enter,
        _button_handle_mouse_enter,
        _mume_window_handle_mouse_leave,
        _button_handle_mouse_leave,
        _mume_window_handle_expose,
        _button_handle_expose,
        MUME_FUNC_END);
}

const void* mume_button_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "button class",
        _button_super_meta_class(),
        sizeof(struct _button_class),
        MUME_PROP_END,
        MUME_FUNC_END);
}

void* mume_button_new(void *parent, int x, int y, const char *text)
{
    int width = 100;
    int height = 100;
    return mume_new(mume_button_class(),
                    parent, x, y, width, height, text);
}

void mume_button_set_text(void *_self, const char *text)
{
    struct _button *self = _self;
    free(self->text);
    if (text)
        self->text = strdup_abort(text);
    else
        self->text = NULL;

    mume_invalidate_region(self, NULL);
}

const char* mume_button_get_text(const void *_self)
{
    const struct _button *self = _self;
    return self->text;
}

mume_type_t* mume_typeof_button_theme(void)
{
    static void *tp;

    if (!tp) {
        mume_type_t *t1, *t2;
        /* pushbutton */
        MUME_COMPOUND_CREATE(
            t1, struct _pushbtn_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), bkgnd);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_texts(), texts);
        MUME_COMPOUND_FINISH();
        /* checkbox */
        MUME_COMPOUND_CREATE(
            t2, struct _checkbox_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), uncheck);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), check);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), bkgnd);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_texts(), texts);
        MUME_COMPOUND_FINISH();
        /* button */
        MUME_COMPOUND_CREATE(
            tp, struct _button_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(t1, pushbtn);
        MUME_DIRECT_PROPERTY(t2, checkbox);
        MUME_DIRECT_PROPERTY(t2, radiobtn);
        MUME_COMPOUND_FINISH();
        /* tp has reference now */
        mume_type_destroy(t1);
        mume_type_destroy(t2);
    }

    return tp;
}
