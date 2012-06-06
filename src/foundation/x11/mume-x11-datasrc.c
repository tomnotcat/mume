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
#include "mume-x11-datasrc.h"
#include "mume-debug.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-x11-backend.h"
#include "mume-x11-util.h"
#include MUME_ASSERT_H

/* x11 datafmt class. */
#define _x11_datafmt_super_class mume_datafmt_class

struct _x11_datafmt {
    const char _[MUME_SIZEOF_DATAFMT];
    Atom target;
};

struct _x11_datafmt_class {
    const char _[MUME_SIZEOF_DATAFMT_CLASS];
};

static const struct {
    char *name;
    Atom target;
} basic_formats[] = {
    { MUME_DATAFMT_TEXT, XA_STRING },
};

MUME_STATIC_ASSERT(sizeof(struct _x11_datafmt) ==
                   MUME_SIZEOF_X11_DATAFMT);

MUME_STATIC_ASSERT(sizeof(struct _x11_datafmt_class) ==
                   MUME_SIZEOF_X11_DATAFMT_CLASS);

static char* _x11_atom_to_format_name(
    Display *xdisplay, Atom atom, int *nfree)
{
    int i;
    *nfree = 0;
    for (i = 0; i < COUNT_OF(basic_formats); ++i) {
        if (basic_formats[i].target == atom)
            return basic_formats[i].name;
    }

    *nfree = 1;
    return XGetAtomName(xdisplay, atom);
}

static Atom _format_name_to_x11_atom(
    Display *xdisplay, const char *name)
{
    int i;
    for (i = 0; i < COUNT_OF(basic_formats); ++i) {
        if (0 == strcmp(name, basic_formats[i].name))
            return basic_formats[i].target;
    }

    return XInternAtom(xdisplay, name, False);
}

static void* _x11_datafmt_ctor(
    struct _x11_datafmt *self, int mode, va_list *app)
{
    Display *xdisplay;
    const char *name;

    if (!_mume_ctor(_x11_datafmt_super_class(), self, mode, app))
        return NULL;

    name = mume_datafmt_get_name(self);
    xdisplay = va_arg(*app, Display*);
    self->target = _format_name_to_x11_atom(xdisplay, name);

    return self;
}

const void* mume_x11_datafmt_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_datafmt_meta_class(),
        "x11 datafmt",
        _x11_datafmt_super_class(),
        sizeof(struct _x11_datafmt),
        MUME_PROP_END,
        _mume_ctor, _x11_datafmt_ctor,
        MUME_FUNC_END);
}

void* mume_x11_datafmt_new(Display *display, const char *name)
{
    return mume_new(mume_x11_datafmt_class(), name, display);
}

Atom mume_x11_datafmt_get_atom(const void *_self)
{
    const struct _x11_datafmt *self = _self;
    assert(mume_is_of(_self, mume_x11_datafmt_class()));
    return self->target;
}

/* x11 datasrc class. */
#define _x11_datasrc_super_class mume_datasrc_class

struct _x11_datasrc {
    const char _[MUME_SIZEOF_DATASRC];
    Display *display;
    Window window;
    const void **formats;
};

struct _x11_datasrc_class {
    const char _[MUME_SIZEOF_DATASRC_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _x11_datasrc) ==
                   MUME_SIZEOF_X11_DATASRC);

MUME_STATIC_ASSERT(sizeof(struct _x11_datasrc_class) ==
                   MUME_SIZEOF_X11_DATASRC_CLASS);

static int _x11_convert_selection_and_wait(
    Display *display, Atom selection, Atom target,
    Atom property, Window requestor)
{
    Time init_time, cur_time;
    XEvent xevent;

    XConvertSelection(
        display, selection, target,
        property, requestor, CurrentTime);

    init_time = mume_x11_get_server_time(display, requestor);
    cur_time = init_time;

    while (cur_time >= init_time &&
           (cur_time - init_time) < 1000)
    {
        if (XPending(display)) {
            XNextEvent(display, &xevent);
            mume_x11_backend_dispatch(mume_backend(), &xevent);

            if (xevent.type == SelectionNotify &&
                xevent.xselection.selection == selection &&
                xevent.xselection.target == target &&
                xevent.xselection.property == property)
            {
                return 1;
            }
        }

        cur_time = mume_x11_get_server_time(display, requestor);
    }

    return 0;
}

static void* _x11_get_clipboard_selection(
    Display *display, Window window, Atom target,
    Atom req_type, int req_format, int *count)
{
    Atom clipboard_selection, data_property;
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *data = 0;

    clipboard_selection = XInternAtom(display, "CLIPBOARD", False);
    data_property = XInternAtom(display, "MUME_DATA", False);

    if (!_x11_convert_selection_and_wait(
            display, clipboard_selection,
            target, data_property, window))
    {
        mume_warning(("Convert selection failed\n"));
        return NULL;
    }

    if (XGetWindowProperty(
            display, window, data_property,
            0, 65536, True, req_type, &type, &format,
            &nitems, &after, &data) != Success)
    {
        mume_warning(("Get property failed\n"));
        return NULL;
    }

    if (NULL == data) {
        mume_warning(("No property data available\n"));
        return NULL;
    }

    if (type != req_type) {
        char *n1, *n2;
        n1 = XGetAtomName(display, req_type);
        n2 = XGetAtomName(display, type);
        mume_warning(("Type disaccord: %s - %s\n", n1, n2));
        XFree(n1);
        XFree(n2);
        XFree(data);
        return NULL;
    }

    if (format != req_format) {
        mume_warning(("Format disaccord: %d - %d\n",
                      req_format, format));
        XFree(data);
        return NULL;
    }

    *count = nitems;
    return data;
}

static const void** _x11_datasrc_get_formats(
    struct _x11_datasrc *self)
{
    Atom targets, *atoms;
    int i, nitems, nfree;
    char *name;

    if (self->formats)
        return self->formats;

    targets = XInternAtom(
        self->display, "TARGETS", False);

    atoms = _x11_get_clipboard_selection(
        self->display, self->window,
        targets, XA_ATOM, 32, &nitems);

    if (NULL == atoms)
        return self->formats;

    self->formats = malloc_abort(sizeof(void*) * (nitems + 1));
    self->formats[nitems] = NULL;

    for (i = 0; i < nitems; ++i) {
        name = _x11_atom_to_format_name(
            self->display, atoms[i], &nfree);

        self->formats[i] = mume_datafmt(name);

        if (nfree)
            XFree(name);
    }

    XFree(atoms);

    return self->formats;
}

static void* _x11_datasrc_get_data(
    struct _x11_datasrc *self, const void *format)
{
    Atom target;
    void *data, *data_copy;
    int length;

    target = mume_x11_datafmt_get_atom(format);
    data = _x11_get_clipboard_selection(
        self->display, self->window, target, target, 8, &length);

    if (NULL == data)
        return NULL;

    data_copy = malloc_abort(length);
    memcpy(data_copy, data, length);
    XFree(data);

    return mume_datarec_new(format, data_copy, length);
}

static void* _x11_datasrc_ctor(
    struct _x11_datasrc *self, int mode, va_list *app)
{
    if (!_mume_ctor(_x11_datasrc_super_class(), self, mode, app))
        return NULL;

    self->display = va_arg(*app, Display*);
    self->window = va_arg(*app, Window);
    self->formats = NULL;

    return self;
}

static void* _x11_datasrc_dtor(struct _x11_datasrc *self)
{
    free(self->formats);
    return _mume_dtor(_x11_datasrc_super_class(), self);
}

const void* mume_x11_datasrc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_datasrc_meta_class(),
        "x11 datasrc",
        _x11_datasrc_super_class(),
        sizeof(struct _x11_datasrc),
        MUME_PROP_END,
        _mume_ctor, _x11_datasrc_ctor,
        _mume_dtor, _x11_datasrc_dtor,
        _mume_datasrc_get_formats,
        _x11_datasrc_get_formats,
        _mume_datasrc_get_data,
        _x11_datasrc_get_data,
        MUME_FUNC_END);
}

void* mume_x11_datasrc_new(Display *display, Window window)
{
    return mume_new(mume_x11_datasrc_class(),
                    NULL, 0, display, window);
}
