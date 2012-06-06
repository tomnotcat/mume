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
#include "mume-x11-clipboard.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-x11-backend.h"
#include "mume-x11-backwin.h"
#include "mume-x11-datasrc.h"
#include "mume-x11-util.h"
#include MUME_ASSERT_H

#define _x11_clipboard_super_class mume_clipboard_class

struct _x11_clipboard {
    const char _[MUME_SIZEOF_CLIPBOARD];
    void *backend;
    void *clipboard_window;
};

struct _x11_clipboard_class {
    const char _[MUME_SIZEOF_CLIPBOARD_CLASS];
};

struct _x11_clipboard_window {
    const char _[MUME_SIZEOF_X11_BACKWIN];
    void *clipboard;
};

MUME_STATIC_ASSERT(sizeof(struct _x11_clipboard) ==
                   MUME_SIZEOF_X11_CLIPBOARD);

MUME_STATIC_ASSERT(sizeof(struct _x11_clipboard_class) ==
                   MUME_SIZEOF_X11_CLIPBOARD_CLASS);

static Atom* _x11_datafmt_create_atoms(
    const void **formats, int *count)
{
    int i;
    Atom *xatoms = NULL;

    *count = mume_datasrc_count_formats(formats);
    if ((*count) > 0) {
        xatoms = malloc_abort(sizeof(Atom) * (*count));
        for (i = 0; i < (*count); ++i)
            xatoms[i] = mume_x11_datafmt_get_atom(formats[i]);
    }

    return xatoms;
}

static void _x11_clipboard_window_handle_selection_request(
    struct _x11_clipboard_window *self, XSelectionRequestEvent *xrequest)
{
    void *datasrc;
    const void **format;
    Atom target;
    XEvent respond;
    Display *display = xrequest->display;

    datasrc = _mume_clipboard_get_data(
        _x11_clipboard_super_class(), self->clipboard);

    if (NULL == datasrc)
        return;

    format = mume_datasrc_get_formats(datasrc);

    if (xrequest->target != XInternAtom(display, "TARGETS", True)) {

        respond.xselection.property = None;

        while (*format) {
            target = mume_x11_datafmt_get_atom(*format);
            if (xrequest->target == target) {
                void *datarec = mume_datasrc_get_data(datasrc, *format);
                XChangeProperty(xrequest->display,
                                xrequest->requestor,
                                xrequest->property,
                                target,
                                8,
                                PropModeReplace,
                                mume_datarec_get_data(datarec),
                                mume_datarec_get_length(datarec));

                respond.xselection.property = xrequest->property;
                mume_refobj_release(datarec);
                break;
            }

            ++format;
        }
    }
    else {
        /* TARGETS information requested, Write supported TARGETS
         * into the requestor. */
        Atom *xatoms;
        int count;

        xatoms = _x11_datafmt_create_atoms(format, &count);
        if (xatoms) {
            XChangeProperty(
                xrequest->display, xrequest->requestor,
                xrequest->property, XA_ATOM, 32, PropModeReplace,
                (unsigned char*)xatoms, count);

            free(xatoms);
        }

        /* Notify the requestor we have set its requested property. */
        respond.xselection.property = xrequest->property;
    }

    mume_refobj_release(datasrc);

    respond.xselection.type= SelectionNotify;
    respond.xselection.display= xrequest->display;
    respond.xselection.requestor= xrequest->requestor;
    respond.xselection.selection=xrequest->selection;
    respond.xselection.target= xrequest->target;
    respond.xselection.time = xrequest->time;
    XSendEvent(display, xrequest->requestor, 0, 0, &respond);
}

static const void* _x11_clipboard_window_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_backwin_meta_class(),
        "x11 clipboard window",
        mume_x11_backwin_class(),
        sizeof(struct _x11_clipboard_window),
        MUME_PROP_END,
        _mume_x11_backwin_handle_selection_request,
        _x11_clipboard_window_handle_selection_request,
        MUME_FUNC_END);
}

static void* _x11_clipboard_window_new(void *clipboard, void *backend)
{
    Window xwindow;
    Display *display;
    struct _x11_clipboard_window *window;
    int managed = 1;

    display = mume_x11_backend_get_display(backend);
    xwindow = mume_x11_create_window(
        display, None, MUME_BACKWIN_NORMAL, -10, -10, 10, 10,
        InputOnly, StructureNotifyMask | PropertyChangeMask);

    window = mume_new(_x11_clipboard_window_class(),
                      backend, xwindow, managed);

    window->clipboard = clipboard;

    return window;
}

static void _x11_clipboard_set_data(
    struct _x11_clipboard *self, void *data)
{
    Display *display;
    Window window;
    Atom clipboard_selection;
    Atom clipboard_manager, save_targets;
    Time time;

    _mume_clipboard_set_data(
        _x11_clipboard_super_class(), self, data);

    if (NULL == data)
        return;

    display = mume_x11_backend_get_display(self->backend);
    window = mume_x11_backwin_get_xwindow(self->clipboard_window);

    clipboard_selection = XInternAtom(display, "CLIPBOARD", False);

    time = mume_x11_get_server_time(display, window);

    XSetSelectionOwner(display, XA_PRIMARY, window, time);
    XSetSelectionOwner(display, clipboard_selection, window, time);

    clipboard_manager = XInternAtom(display, "CLIPBOARD_MANAGER", False);
    save_targets = XInternAtom(display, "SAVE_TARGETS", False);

    /* Convert CLIPBOARD selection to CLIPBOARD_MANAGER. */
    if (XGetSelectionOwner(display, clipboard_manager) != None) {
        Atom property_name = None;
        Atom *xatoms;
        int count;
        const void **formats;

        formats = mume_datasrc_get_formats(data);
        xatoms = _x11_datafmt_create_atoms(formats, &count);
        if (xatoms) {
            property_name = XInternAtom(display, "GTK_SELECTION", False);

            XChangeProperty(
                display, window, property_name, XA_ATOM, 32,
                PropModeReplace, (const unsigned char*)xatoms, count);

            free(xatoms);
        }

        XConvertSelection(
            display, clipboard_manager, save_targets,
            property_name, window, time);
    }
    else {
        mume_warning(("Clipboard manager not found!\n"));
    }
}

static void* _x11_clipboard_get_data(struct _x11_clipboard *self)
{
    Display *display;
    Window window;

    display = mume_x11_backend_get_display(self->backend);
    window = mume_x11_backwin_get_xwindow(self->clipboard_window);

    return mume_x11_datasrc_new(display, window);
}

static void* _x11_clipboard_ctor(
    struct _x11_clipboard *self, int mode, va_list *app)
{
    if (!_mume_ctor(_x11_clipboard_super_class(), self, mode, app))
        return NULL;

    self->backend = va_arg(*app, void*);
    self->clipboard_window = _x11_clipboard_window_new(
        self, self->backend);

    return self;
}

static void* _x11_clipboard_dtor(struct _x11_clipboard *self)
{
    mume_refobj_release(self->clipboard_window);
    return _mume_dtor(_x11_clipboard_super_class(), self);
}

const void* mume_x11_clipboard_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_clipboard_meta_class(),
        "xlib clipboard",
        _x11_clipboard_super_class(),
        sizeof(struct _x11_clipboard),
        MUME_PROP_END,
        _mume_ctor, _x11_clipboard_ctor,
        _mume_dtor, _x11_clipboard_dtor,
        _mume_clipboard_set_data, _x11_clipboard_set_data,
        _mume_clipboard_get_data, _x11_clipboard_get_data,
        MUME_FUNC_END);
}

void* mume_x11_clipboard_new(void *backend)
{
    return mume_new(mume_x11_clipboard_class(), backend);
}
