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
#include "mume-backend.h"
#include "mume-debug.h"
#include "mume-dlfcn.h"
#include "mume-frontend.h"
#include MUME_ASSERT_H

#define _backend_super_class mume_refobj_class
#define _backend_super_meta_class mume_refobj_meta_class

struct _backend {
    const char _[MUME_SIZEOF_REFOBJ];
};

struct _backend_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    void (*screen_size)(void *self, int *width, int *height);
    void* (*data_format)(void *self, const char *name);
    void* (*clipboard)(void *self);
    void* (*root_backwin)(void *self);
    void* (*create_backwin)(void *self, int type, void *parent,
                            int x, int y, int w, int h);
    void* (*create_cursor)(void *self, int id);
    int (*handle_event)(void *self, int wait);
    int (*wakeup_event)(void *self);
    void (*query_pointer)(void *self, int *x, int *y, int *state);
};

MUME_STATIC_ASSERT(sizeof(struct _backend) == MUME_SIZEOF_BACKEND);
MUME_STATIC_ASSERT(sizeof(struct _backend_class) ==
                   MUME_SIZEOF_BACKEND_CLASS);

static void* _backend_ctor(
    struct _backend *self, int mode, va_list *app)
{
    return _mume_ctor(_backend_super_class(), self, mode, app);
}

static void* _backend_dtor(struct _backend *self)
{
    return _mume_dtor(_backend_super_class(), self);
}

static void _backend_screen_size(void *self, int *width, int *height)
{
    if (width)
        *width = 0;

    if (height)
        *height = 0;
}

static void* _backend_data_format(void *self, const char *name)
{
    return NULL;
}

static void* _backend_clipboard(void *self)
{
    return NULL;
}

static void* _backend_root_backwin(void *self)
{
    return NULL;
}

static void* _backend_create_backwin(
    void *self, int type, void *parent, int x, int y, int w, int h)
{
    return NULL;
}

static void* _backend_create_cursor(void *self, int id)
{
    return NULL;
}

static int _backend_handle_event(void *self, int wait)
{
    return 0;
}

static int _backend_wakeup_event(void *self)
{
    return 0;
}

static void _backend_query_pointer(
    void *self, int *x, int *y, int *state)
{
    if (x)
        *x = 0;

    if (y)
        *y = 0;

    if (state)
        *state = 0;
}

static void* _backend_class_ctor(
    struct _backend_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_backend_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_backend_screen_size)
            *(voidf**)&self->screen_size = method;
        else if (selector == (voidf*)_mume_backend_data_format)
            *(voidf**)&self->data_format = method;
        else if (selector == (voidf*)_mume_backend_clipboard)
            *(voidf**)&self->clipboard = method;
        else if (selector == (voidf*)_mume_backend_root_backwin)
            *(voidf**)&self->root_backwin = method;
        else if (selector == (voidf*)_mume_backend_create_backwin)
            *(voidf**)&self->create_backwin = method;
        else if (selector == (voidf*)_mume_backend_create_cursor)
            *(voidf**)&self->create_cursor = method;
        else if (selector == (voidf*)_mume_backend_handle_event)
            *(voidf**)&self->handle_event = method;
        else if (selector == (voidf*)_mume_backend_wakeup_event)
            *(voidf**)&self->wakeup_event = method;
        else if (selector == (voidf*)_mume_backend_query_pointer)
            *(voidf**)&self->query_pointer = method;
    }

    return self;
}

const void* mume_backend_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_backend_meta_class(),
        "backend",
        _backend_super_class(),
        sizeof(struct _backend),
        MUME_PROP_END,
        _mume_ctor, _backend_ctor,
        _mume_dtor, _backend_dtor,
        _mume_backend_screen_size, _backend_screen_size,
        _mume_backend_data_format, _backend_data_format,
        _mume_backend_clipboard, _backend_clipboard,
        _mume_backend_root_backwin, _backend_root_backwin,
        _mume_backend_create_backwin, _backend_create_backwin,
        _mume_backend_create_cursor, _backend_create_cursor,
        _mume_backend_handle_event, _backend_handle_event,
        _mume_backend_wakeup_event, _backend_wakeup_event,
        _mume_backend_query_pointer, _backend_query_pointer,
        MUME_FUNC_END);
}

const void* mume_backend_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "backend class",
        _backend_super_meta_class(),
        sizeof(struct _backend_class),
        MUME_PROP_END,
        _mume_ctor, _backend_class_ctor,
        MUME_FUNC_END);
}

void* mume_backend_new_from_dll(
    mume_dlhdl_t *hdl, int width, int height,
    unsigned int flags, void *data)
{
    const void* (*clazz)(void);

    clazz = (const void* (*)(void))(intptr_t)(
        mume_dlsym(hdl, MUME_BACKEND_CLASS_SYM));

    if (NULL == clazz) {
        mume_warning(("No backend class symbol\n"));
        return NULL;
    }

    if (!mume_is_of(clazz(), mume_backend_meta_class())) {
        mume_warning(("Invalid backend class\n"));
        return NULL;
    }

    return mume_new(clazz(), width, height, flags, data);
}

void _mume_backend_screen_size(
    const void *_clazz, void *_self, int *width, int *height)
{
    MUME_SELECTOR_NORETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, screen_size, (_self, width, height));
}

void* _mume_backend_data_format(
    const void *_clazz, void *_self, const char *name)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, data_format, (_self, name));
}

void* _mume_backend_clipboard(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, clipboard, (_self));
}

void* _mume_backend_root_backwin(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, root_backwin, (_self));
}

void* _mume_backend_create_backwin(
    const void *_clazz, void *_self, int type,
    void *parent, int x, int y, int width, int height)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, create_backwin,
        (_self, type, parent, x, y, width, height));
}

void* _mume_backend_create_cursor(
    const void *_clazz, void *_self, int id)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, create_cursor, (_self, id));
}

int _mume_backend_handle_event(
    const void *_clazz, void *_self, int wait)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, handle_event, (_self, wait));
}

int _mume_backend_wakeup_event(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, wakeup_event, (_self));
}

void _mume_backend_query_pointer(
    const void *_clazz, void *_self, int *x, int *y, int *state)
{
    MUME_SELECTOR_NORETURN(
        mume_backend_meta_class(), mume_backend_class(),
        struct _backend_class, query_pointer, (_self, x, y, state));
}
