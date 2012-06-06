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
#include "mume-clipboard.h"
#include "mume-datasrc.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _clipboard_super_class mume_refobj_class
#define _clipboard_super_meta_class mume_refobj_meta_class

struct _clipboard {
    const char _[MUME_SIZEOF_REFOBJ];
    void *datasrc;     /* Data source object. */
};

struct _clipboard_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    void (*set_data)(void *self, void *data);
    void* (*get_data)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _clipboard) == MUME_SIZEOF_CLIPBOARD);
MUME_STATIC_ASSERT(sizeof(struct _clipboard_class) ==
                   MUME_SIZEOF_CLIPBOARD_CLASS);

static void _clipboard_set_data(struct _clipboard *self, void *data)
{
    if (self->datasrc)
        mume_refobj_release(self->datasrc);

    self->datasrc = data;

    if (self->datasrc)
        mume_refobj_addref(self->datasrc);
}

static void* _clipboard_get_data(struct _clipboard *self)
{
    if (self->datasrc)
        mume_refobj_addref(self->datasrc);

    return self->datasrc;
}

static void* _clipboard_ctor(
    struct _clipboard *self, int mode, va_list *app)
{
    if (!_mume_ctor(_clipboard_super_class(), self, mode, app))
        return NULL;

    self->datasrc = NULL;
    return self;
}

static void* _clipboard_dtor(struct _clipboard *self)
{
    if (self->datasrc)
        mume_refobj_release(self->datasrc);

    return _mume_dtor(_clipboard_super_class(), self);
}

static void* _clipboard_class_ctor(
    struct _clipboard_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_clipboard_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_clipboard_set_data)
            *(voidf**)&self->set_data = method;
        else if (selector == (voidf*)_mume_clipboard_get_data)
            *(voidf**)&self->get_data = method;
    }

    return self;
}

const void* mume_clipboard_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_clipboard_meta_class(),
        "clipboard",
        _clipboard_super_class(),
        sizeof(struct _clipboard),
        MUME_PROP_END,
        _mume_ctor, _clipboard_ctor,
        _mume_dtor, _clipboard_dtor,
        _mume_clipboard_set_data, _clipboard_set_data,
        _mume_clipboard_get_data, _clipboard_get_data,
        MUME_FUNC_END);
}

const void* mume_clipboard_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "clipboard class",
        _clipboard_super_meta_class(),
        sizeof(struct _clipboard_class),
        MUME_PROP_END,
        _mume_ctor, _clipboard_class_ctor,
        MUME_FUNC_END);
}

void _mume_clipboard_set_data(
    const void *_clazz, void *_self, void *data)
{
    MUME_SELECTOR_NORETURN(
        mume_clipboard_meta_class(), mume_clipboard_class(),
        struct _clipboard_class, set_data, (_self, data));
}

void* _mume_clipboard_get_data(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_clipboard_meta_class(), mume_clipboard_class(),
        struct _clipboard_class, get_data, (_self));
}
