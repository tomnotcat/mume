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
#include "mume-backwin.h"
#include "mume-debug.h"
#include "mume-window.h"
#include MUME_ASSERT_H

#define _backwin_super_class mume_refobj_class
#define _backwin_super_meta_class mume_refobj_meta_class

struct _backwin {
    const char _[MUME_SIZEOF_REFOBJ];
    void *window;
};

struct _backwin_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    void (*attach_changed)(void *self, void *window);
    int (*equal)(void *self, const void *other);
    void (*map)(void *self);
    void (*unmap)(void *self);
    int (*is_mapped)(void *self);
    void (*set_geometry)(void *self, int x, int y, int w, int h);
    void (*get_geometry)(void *self, int *x, int *y, int *w, int *h);
    void (*set_cursor)(void *self, void *cursor);
    void (*raise)(void *self);
    void (*lower)(void *self);
    cairo_t* (*begin_paint)(void *self);
    void (*end_paint)(void *self, cairo_t *cr);
    void (*grab_pointer)(void *self);
    void (*ungrab_pointer)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _backwin) == MUME_SIZEOF_BACKWIN);
MUME_STATIC_ASSERT(sizeof(struct _backwin_class) ==
                   MUME_SIZEOF_BACKWIN_CLASS);

static void* _backwin_ctor(
    struct _backwin *self, int mode, va_list *app)
{
    if (!_mume_ctor(_backwin_super_class(), self, mode, app))
        return NULL;

    self->window = NULL;
    return self;
}

static void* _backwin_dtor(struct _backwin *self)
{
    assert(NULL == self->window);
    return _mume_dtor(_backwin_super_class(), self);
}

static void _backwin_attach_changed(void *self, void *window)
{
}

static int _backwin_equal(void *self, const void *other)
{
    return 0;
}

static void _backwin_map(void *self)
{
}

static void _backwin_unmap(void *self)
{
}

static int _backwin_is_mapped(void *self)
{
    return 0;
}

static void _backwin_set_geometry(
    void *self, int x, int y, int w, int h)
{
}

static void _backwin_get_geometry(
    void *self, int *x, int *y, int *w, int *h)
{
    if (x) *x = 0;
    if (y) *y = 0;
    if (w) *w = 0;
    if (h) *h = 0;
}

static void _backwin_set_cursor(void *self, void *cursor)
{
}

static void _backwin_raise(void *self)
{
}

static void _backwin_lower(void *self)
{
}

static cairo_t* _backwin_begin_paint(void *self)
{
    return NULL;
}

static void _backwin_end_paint(void *self, cairo_t *cr)
{
}

static void _backwin_grab_pointer(void *self)
{
}

static void _backwin_ungrab_pointer(void *self)
{
}

static void* _backwin_class_ctor(
    struct _backwin_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_backwin_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_backwin_attach_changed)
            *(voidf**)&self->attach_changed = method;
        else if (selector == (voidf*)_mume_backwin_equal)
            *(voidf**)&self->equal = method;
        else if (selector == (voidf*)_mume_backwin_map)
            *(voidf**)&self->map = method;
        else if (selector == (voidf*)_mume_backwin_unmap)
            *(voidf**)&self->unmap = method;
        else if (selector == (voidf*)_mume_backwin_is_mapped)
            *(voidf**)&self->is_mapped = method;
        else if (selector == (voidf*)_mume_backwin_set_geometry)
            *(voidf**)&self->set_geometry = method;
        else if (selector == (voidf*)_mume_backwin_get_geometry)
            *(voidf**)&self->get_geometry = method;
        else if (selector == (voidf*)_mume_backwin_set_cursor)
            *(voidf**)&self->set_cursor = method;
        else if (selector == (voidf*)_mume_backwin_raise)
            *(voidf**)&self->raise = method;
        else if (selector == (voidf*)_mume_backwin_lower)
            *(voidf**)&self->lower = method;
        else if (selector == (voidf*)_mume_backwin_begin_paint)
            *(voidf**)&self->begin_paint = method;
        else if (selector == (voidf*)_mume_backwin_end_paint)
            *(voidf**)&self->end_paint = method;
        else if (selector == (voidf*)_mume_backwin_grab_pointer)
            *(voidf**)&self->grab_pointer = method;
        else if (selector == (voidf*)_mume_backwin_ungrab_pointer)
            *(voidf**)&self->ungrab_pointer = method;
    }

    return self;
}

const void* mume_backwin_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_backwin_meta_class(),
        "backwin",
        _backwin_super_class(),
        sizeof(struct _backwin),
        MUME_PROP_END,
        _mume_ctor, _backwin_ctor,
        _mume_dtor, _backwin_dtor,
        _mume_backwin_attach_changed,
        _backwin_attach_changed,
        _mume_backwin_equal,
        _backwin_equal,
        _mume_backwin_map,
        _backwin_map,
        _mume_backwin_unmap,
        _backwin_unmap,
        _mume_backwin_is_mapped,
        _backwin_is_mapped,
        _mume_backwin_set_geometry,
        _backwin_set_geometry,
        _mume_backwin_get_geometry,
        _backwin_get_geometry,
        _mume_backwin_set_cursor,
        _backwin_set_cursor,
        _mume_backwin_raise,
        _backwin_raise,
        _mume_backwin_lower,
        _backwin_lower,
        _mume_backwin_begin_paint,
        _backwin_begin_paint,
        _mume_backwin_end_paint,
        _backwin_end_paint,
        _mume_backwin_grab_pointer,
        _backwin_grab_pointer,
        _mume_backwin_ungrab_pointer,
        _backwin_ungrab_pointer,
        MUME_FUNC_END);
}

const void* mume_backwin_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "backwin class",
        _backwin_super_meta_class(),
        sizeof(struct _backwin_class),
        MUME_PROP_END,
        _mume_ctor, _backwin_class_ctor,
        MUME_FUNC_END);
}

void mume_backwin_set_attached(void *_self, void *window)
{
    struct _backwin *self = _self;

    assert(mume_is_of(_self, mume_backwin_class()));
    assert(!window || mume_is_of(window, mume_window_class()));

    mume_backwin_attach_changed(self, window);
    self->window = window;
}

void* mume_backwin_get_attached(const void *_self)
{
    const struct _backwin *self = _self;
    assert(mume_is_of(_self, mume_backwin_class()));
    return self->window;
}

void _mume_backwin_attach_changed(
    const void *_clazz, void *_self, void *window)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, attach_changed, (_self, window));
}

int _mume_backwin_equal(
    const void *_clazz, void *_self, const void *other)
{
    MUME_SELECTOR_RETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, equal, (_self, other));
}

void _mume_backwin_map(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, map, (_self));
}

void _mume_backwin_unmap(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, unmap, (_self));
}

int _mume_backwin_is_mapped(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, is_mapped, (_self));
}

void _mume_backwin_set_geometry(
    const void *_clazz, void *_self, int x, int y, int w, int h)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, set_geometry, (_self, x, y, w, h));
}

void _mume_backwin_get_geometry(
    const void *_clazz, void *_self, int *x, int *y, int *w, int *h)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, get_geometry, (_self, x, y, w, h));
}

void _mume_backwin_set_cursor(
    const void *_clazz, void *_self, void *cursor)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, set_cursor, (_self, cursor));
}

void _mume_backwin_raise(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, raise, (_self));
}

void _mume_backwin_lower(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, lower, (_self));
}

cairo_t* _mume_backwin_begin_paint(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, begin_paint, (_self));
}

void _mume_backwin_end_paint(
    const void *_clazz, void *_self, cairo_t *cr)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, end_paint, (_self, cr));
}

void _mume_backwin_grab_pointer(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, grab_pointer, (_self));
}

void _mume_backwin_ungrab_pointer(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_backwin_meta_class(), mume_backwin_class(),
        struct _backwin_class, ungrab_pointer, (_self));
}
