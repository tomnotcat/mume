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
#include "mume-x11-backwin.h"
#include "mume-debug.h"
#include "mume-frontend.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-oset.h"
#include "mume-window.h"
#include "mume-x11-backend.h"
#include "mume-x11-cursor.h"
#include "mume-x11-util.h"
#include MUME_ASSERT_H

#define _x11_backwin_super_class mume_backwin_class
#define _x11_backwin_super_meta_class mume_backwin_meta_class

struct _x11_backwin {
    const char _[MUME_SIZEOF_BACKWIN];
    void *backend;
    Window window;
    cairo_surface_t *surface;
    int managed;
};

struct _x11_backwin_class {
    const char _[MUME_SIZEOF_BACKWIN_CLASS];
    void (*handle_event)(void *self, XEvent *xevent);
    void (*handle_key_press)(void *self, XKeyEvent *xkey);
    void (*handle_key_release)(void *self, XKeyEvent *xkey);
    void (*handle_button_press)(void *self, XButtonEvent *xbutton);
    void (*handle_button_release)(void *self, XButtonEvent *xbutton);
    void (*handle_motion_notify)(void *self, XMotionEvent *xmotion);
    void (*handle_enter_notify)(void *self, XCrossingEvent *xcrossing);
    void (*handle_leave_notify)(void *self, XCrossingEvent *xcrossing);
    void (*handle_focus_in)(void *self, XFocusChangeEvent *xfocus);
    void (*handle_focus_out)(void *self, XFocusChangeEvent *xfocus);
    void (*handle_reparent)(void *self, XReparentEvent *xreparent);
    void (*handle_expose)(void *self, XExposeEvent *xexpose);
    void (*handle_map)(void *self, XMapEvent *xmap);
    void (*handle_unmap)(void *self, XUnmapEvent *xunmap);
    void (*handle_configure)(void *self, XConfigureEvent *xconfigure);
    void (*handle_client)(void *self, XClientMessageEvent *xclient);
    void (*handle_selection_request)(
        void *self, XSelectionRequestEvent *xrequest);
    void (*handle_selection_notify)(
        void *self, XSelectionEvent *xselection);
    void (*handle_selection_clear)(
        void *self, XSelectionClearEvent *xclear);
};

MUME_STATIC_ASSERT(sizeof(struct _x11_backwin) ==
                   MUME_SIZEOF_X11_BACKWIN);

MUME_STATIC_ASSERT(sizeof(struct _x11_backwin_class) ==
                   MUME_SIZEOF_X11_BACKWIN_CLASS);

static void* _x11_backwin_ctor(
    struct _x11_backwin *self, int mode, va_list *app)
{
    if (!_mume_ctor(_x11_backwin_super_class(), self, mode, app))
        return NULL;

    self->backend = va_arg(*app, void*);
    self->window = va_arg(*app, Window);
    self->managed = va_arg(*app, int);
    self->surface = NULL;

    mume_x11_backend_bind(self->backend, self);

    return self;
}

static void* _x11_backwin_dtor(struct _x11_backwin *self)
{
    Display *display;

    mume_x11_backend_unbind(self->backend, self);
    display = mume_x11_backend_get_display(self->backend);

    if (self->surface)
        cairo_surface_destroy(self->surface);

    if (self->managed)
        XDestroyWindow(display, self->window);

    return _mume_dtor(_x11_backwin_super_class(), self);
}

static void _x11_backwin_attach_changed(
    struct _x11_backwin *self, void *window)
{
    Display *display;
    int pref_width, pref_height;
    int min_width, min_height;
    int max_width, max_height;

    if (NULL == window)
        return;

    display = mume_x11_backend_get_display(self->backend);

    mume_window_size_hint(window, &pref_width, &pref_height,
                          &min_width, &min_height,
                          &max_width, &max_height);

    mume_x11_set_sizehint(
        display, self->window, pref_width, pref_height,
        min_width, min_height, max_width, max_height);
}

static int _x11_backwin_equal(
    struct _x11_backwin *self, struct _x11_backwin *other)
{
    return self->window == other->window;
}

static void _x11_backwin_map(struct _x11_backwin *self)
{
    Display *display;

    if (!self->managed)
        return;

    display = mume_x11_backend_get_display(self->backend);
    XMapWindow(display, self->window);
}

static void _x11_backwin_unmap(struct _x11_backwin *self)
{
    Display *display;

    if (!self->managed)
        return;

    display = mume_x11_backend_get_display(self->backend);
    XUnmapWindow(display, self->window);
}

static int _x11_backwin_is_mapped(struct _x11_backwin *self)
{
    Display *display;
    XWindowAttributes xattr;

    if (!self->managed)
        return 1;

    display = mume_x11_backend_get_display(self->backend);
    XGetWindowAttributes(display, self->window, &xattr);

    return xattr.map_state == IsViewable ||
            xattr.map_state == IsUnviewable;
}

static void _x11_backwin_set_geometry(
    struct _x11_backwin *self, int x, int y, int w, int h)
{
    Display *display;
    XWindowChanges changes;

    if (!self->managed)
        return;

    display = mume_x11_backend_get_display(self->backend);
    changes.x = x;
    changes.y = y;
    changes.width = w;
    changes.height = h;

    XConfigureWindow(display, self->window,
                     CWX | CWY | CWWidth | CWHeight,
                     &changes);
    /*
    XMoveResizeWindow(self->display, self->window,
                      x, y, width, height);
    */
}

static void _x11_backwin_get_geometry(
    struct _x11_backwin *self, int *x, int *y, int *w, int *h)
{
    Display *display;
    Window root_win;
    int tx, ty;
    unsigned int tw, th;
    unsigned int border_width;
    unsigned int depth;

    display = mume_x11_backend_get_display(self->backend);
    XSync(display, False);
    XGetGeometry(display, self->window, &root_win,
                 &tx, &ty, &tw, &th, &border_width, &depth);

    if (x)
        *x = tx;

    if (y)
        *y = ty;

    if (w)
        *w = tw;

    if (h)
        *h = th;
}

static void _x11_backwin_set_cursor(
    struct _x11_backwin *self, void *cursor)
{
    Display *display;
    Cursor xcursor = None;

    if (!self->managed)
        return;

    display = mume_x11_backend_get_display(self->backend);

    if (cursor)
        xcursor = mume_x11_cursor_get_entity(cursor);

    XDefineCursor(display, self->window, xcursor);
}

static void _x11_backwin_raise(struct _x11_backwin *self)
{
}

static void _x11_backwin_lower(struct _x11_backwin *self)
{
}

static cairo_t* _x11_backwin_begin_paint(struct _x11_backwin *self)
{
    Display *display;
    int screen, width, height;

    if (!self->managed)
        return NULL;

    display = mume_x11_backend_get_display(self->backend);
    screen = mume_x11_backend_get_screen(self->backend);
    _x11_backwin_get_geometry(self, NULL, NULL, &width, &height);

    if (NULL == self->surface) {
        self->surface = cairo_xlib_surface_create(
            display, self->window,
            DefaultVisual(display, screen),
            width, height);

        if (NULL == self->surface) {
            mume_error(("cairo_xlib_surface_create(%d, %d)\n",
                        width, height));

            return NULL;
        }
    }
    else {
        cairo_xlib_surface_set_size(self->surface, width, height);
    }

    return cairo_create(self->surface);
}

static void _x11_backwin_end_paint(
    struct _x11_backwin *self, cairo_t *cr)
{
    assert(self->managed);

    cairo_destroy(cr);
    cairo_surface_flush(self->surface);
}

static void _x11_backwin_grab_pointer(struct _x11_backwin *self)
{
    Display *display;
    unsigned int event_mask = ButtonPressMask | ButtonReleaseMask |
                              PointerMotionMask | EnterWindowMask |
                              LeaveWindowMask;

    display = mume_x11_backend_get_display(self->backend);

    XGrabPointer(display, self->window, False,
                 event_mask, GrabModeAsync, GrabModeAsync,
                 None, None, CurrentTime);
}

static void _x11_backwin_ungrab_pointer(struct _x11_backwin *self)
{
    Display *display;

    display = mume_x11_backend_get_display(self->backend);
    XUngrabPointer(display, CurrentTime);
}

static void _x11_backwin_handle_event(
    struct _x11_backwin *self, XEvent *xevent)
{
    switch (xevent->type) {
    case KeyPress:
        mume_x11_backwin_handle_key_press(self, &xevent->xkey);
        break;

    case KeyRelease:
        mume_x11_backwin_handle_key_release(self, &xevent->xkey);
        break;

    case ButtonPress:
        mume_x11_backwin_handle_button_press(self, &xevent->xbutton);
        break;

    case ButtonRelease:
        mume_x11_backwin_handle_button_release(self, &xevent->xbutton);
        break;

    case MotionNotify:
        mume_x11_backwin_handle_motion_notify(self, &xevent->xmotion);
        break;

    case EnterNotify:
        mume_x11_backwin_handle_enter_notify(self, &xevent->xcrossing);
        break;

    case LeaveNotify:
        mume_x11_backwin_handle_leave_notify(self, &xevent->xcrossing);
        break;

    case FocusIn:
        mume_x11_backwin_handle_focus_in(self, &xevent->xfocus);
        break;

    case FocusOut:
        mume_x11_backwin_handle_focus_out(self, &xevent->xfocus);
        break;

    case ReparentNotify:
        mume_x11_backwin_handle_reparent(self, &xevent->xreparent);
        break;

    case Expose:
    case GraphicsExpose:
        mume_x11_backwin_handle_expose(self, &xevent->xexpose);
        break;

    case MapNotify:
        mume_x11_backwin_handle_map(self, &xevent->xmap);
        break;

    case UnmapNotify:
        mume_x11_backwin_handle_unmap(self, &xevent->xunmap);
        break;

    case ConfigureNotify:
        mume_x11_backwin_handle_configure(self, &xevent->xconfigure);
        break;

    case ClientMessage:
        mume_x11_backwin_handle_client(self, &xevent->xclient);
        break;

    case SelectionRequest:
        mume_x11_backwin_handle_selection_request(
            self, &xevent->xselectionrequest);
        break;

    case SelectionNotify:
        mume_x11_backwin_handle_selection_notify(
            self, &xevent->xselection);
        break;

    case SelectionClear:
        mume_x11_backwin_handle_selection_clear(
            self, &xevent->xselectionclear);
        break;
    }
}

static void _x11_backwin_handle_key_press(
    struct _x11_backwin *self, XKeyEvent *xkey)
{
    if (xkey->keycode) {
        int state = mume_x11_translate_state(xkey->state);
        int key = mume_x11_translate_key(xkey->display, xkey->keycode);

        mume_frontend_handle_keydown(
            mume_frontend(), self, xkey->x, xkey->y, state, key);
    }
}

static void _x11_backwin_handle_key_release(
    struct _x11_backwin *self, XKeyEvent *xkey)
{
    int state = mume_x11_translate_state(xkey->state);
    int key = mume_x11_translate_key(xkey->display, xkey->keycode);

    mume_frontend_handle_keyup(
        mume_frontend(), self, xkey->x, xkey->y, state, key);
}

static void _x11_backwin_handle_button_press(
    struct _x11_backwin *self, XButtonEvent *xbutton)
{
    int state = mume_x11_translate_state(xbutton->state);
    int button = mume_x11_translate_button(xbutton->button);

    mume_frontend_handle_buttondown(
        mume_frontend(), self,
        xbutton->x, xbutton->y, state, button);
}

static void _x11_backwin_handle_button_release(
    struct _x11_backwin *self, XButtonEvent *xbutton)
{
    int state = mume_x11_translate_state(xbutton->state);
    int button = mume_x11_translate_button(xbutton->button);

    mume_frontend_handle_buttonup(
        mume_frontend(), self,
        xbutton->x, xbutton->y, state, button);
}

static void _x11_backwin_handle_motion_notify(
    struct _x11_backwin *self, XMotionEvent *xmotion)
{
    int state = mume_x11_translate_state(xmotion->state);

    mume_frontend_handle_mousemotion(
        mume_frontend(), self, xmotion->x, xmotion->y, state);
}

static void _x11_backwin_handle_enter_notify(
    struct _x11_backwin *self, XCrossingEvent *xcrossing)
{
    int state = mume_x11_translate_state(xcrossing->state);
    int mode = mume_x11_translate_notify_mode(xcrossing->mode);

    mume_frontend_handle_mouseenter(
        mume_frontend(), self,
        xcrossing->x, xcrossing->y, state, mode);
}

static void _x11_backwin_handle_leave_notify(
    struct _x11_backwin *self, XCrossingEvent *xcrossing)
{
    int state = mume_x11_translate_state(xcrossing->state);
    int mode = mume_x11_translate_notify_mode(xcrossing->mode);

    mume_frontend_handle_mouseleave(
        mume_frontend(), self,
        xcrossing->x, xcrossing->y, state, mode);
}

static void _x11_backwin_handle_focus_in(
    struct _x11_backwin *self, XFocusChangeEvent *xfocus)
{
    mume_frontend_handle_focusin(
        mume_frontend(), self,
        mume_x11_translate_notify_mode(xfocus->mode));
}

static void _x11_backwin_handle_focus_out(
    struct _x11_backwin *self, XFocusChangeEvent *xfocus)
{
    mume_frontend_handle_focusout(
        mume_frontend(), self,
        mume_x11_translate_notify_mode(xfocus->mode));
}

static void _x11_backwin_handle_reparent(
    struct _x11_backwin *self, XReparentEvent *xreparent)
{
}

static void _x11_backwin_handle_expose(
    struct _x11_backwin *self, XExposeEvent *xexpose)
{
    mume_rect_t rect;

    rect.x = xexpose->x;
    rect.y = xexpose->y;
    rect.width = xexpose->width;
    rect.height = xexpose->height;

    mume_frontend_handle_expose(mume_frontend(), self, &rect);
}

static void _x11_backwin_handle_map(
    struct _x11_backwin *self, XMapEvent *xmap)
{
    if (xmap->event == xmap->window)
        mume_frontend_handle_map(mume_frontend(), self);
}

static void _x11_backwin_handle_unmap(
    struct _x11_backwin *self, XUnmapEvent *xunmap)
{
    if (xunmap->event == xunmap->window)
        mume_frontend_handle_unmap(mume_frontend(), self);
}

static void _x11_backwin_handle_configure(
    struct _x11_backwin *self, XConfigureEvent *xconfigure)
{
    Display *display = xconfigure->display;
    Window xwindow = mume_x11_backwin_get_xwindow(self);
    int x = 0, y = 0, width = 0, height = 0;

    XSync(display, False);

    /* Discard all the pending ConfigureNotify of this window */
    while (XCheckTypedWindowEvent(
               display, xwindow, ConfigureNotify,
               (XEvent*)xconfigure));

    assert(xconfigure->event == xconfigure->window);

    if (!xconfigure->send_event && !xconfigure->override_redirect) {
        int tx = 0;
        int ty = 0;
        Window child_window = 0;
        if (XTranslateCoordinates(
                display, xwindow,
                RootWindow(display, DefaultScreen(display)),
                0, 0, &tx, &ty, &child_window))
        {
            x = tx;
            y = ty;
        }
    }
    else
    {
        x = xconfigure->x;
        y = xconfigure->y;
    }

    width = xconfigure->width;
    height = xconfigure->height;

    mume_frontend_handle_geometry(
        mume_frontend(), self, x, y, width, height);
}

static void _x11_backwin_handle_client(
    struct _x11_backwin *self, XClientMessageEvent *xclient)
{
    Atom delete_window = XInternAtom(
        xclient->display, "WM_DELETE_WINDOW", False);

    if (xclient->format == 32 && xclient->data.l[0] == delete_window) {
        /* Window manager try to close a window. */
        mume_frontend_handle_close(mume_frontend(), self);
    }
}

static void _x11_backwin_handle_selection_request(
    struct _x11_backwin *self, XSelectionRequestEvent *xrequest)
{
}

static void _x11_backwin_handle_selection_notify(
    struct _x11_backwin *self, XSelectionEvent *xselection)
{
}

static void _x11_backwin_handle_selection_clear(
    struct _x11_backwin *self, XSelectionClearEvent *xclear)
{
}

static void* _x11_backwin_class_ctor(
    struct _x11_backwin_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_x11_backwin_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_x11_backwin_handle_event)
            *(voidf**)&self->handle_event = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_key_press)
            *(voidf**)&self->handle_key_press = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_key_release)
            *(voidf**)&self->handle_key_release = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_button_press)
            *(voidf**)&self->handle_button_press = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_button_release)
            *(voidf**)&self->handle_button_release = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_motion_notify)
            *(voidf**)&self->handle_motion_notify = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_enter_notify)
            *(voidf**)&self->handle_enter_notify = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_leave_notify)
            *(voidf**)&self->handle_leave_notify = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_focus_in)
            *(voidf**)&self->handle_focus_in = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_focus_out)
            *(voidf**)&self->handle_focus_out = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_reparent)
            *(voidf**)&self->handle_reparent = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_expose)
            *(voidf**)&self->handle_expose = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_map)
            *(voidf**)&self->handle_map = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_unmap)
            *(voidf**)&self->handle_unmap = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_configure)
            *(voidf**)&self->handle_configure = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_client)
            *(voidf**)&self->handle_client = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_selection_request)
            *(voidf**)&self->handle_selection_request = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_selection_notify)
            *(voidf**)&self->handle_selection_notify = method;
        else if (selector == (voidf*)_mume_x11_backwin_handle_selection_clear)
            *(voidf**)&self->handle_selection_clear = method;
    }

    return self;
}

const void* mume_x11_backwin_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_backwin_meta_class(),
        "x11 backwin",
        _x11_backwin_super_class(),
        sizeof(struct _x11_backwin),
        MUME_PROP_END,
        _mume_ctor, _x11_backwin_ctor,
        _mume_dtor, _x11_backwin_dtor,
        _mume_backwin_attach_changed,
        _x11_backwin_attach_changed,
        _mume_backwin_equal,
        _x11_backwin_equal,
        _mume_backwin_map,
        _x11_backwin_map,
        _mume_backwin_unmap,
        _x11_backwin_unmap,
        _mume_backwin_is_mapped,
        _x11_backwin_is_mapped,
        _mume_backwin_set_geometry,
        _x11_backwin_set_geometry,
        _mume_backwin_get_geometry,
        _x11_backwin_get_geometry,
        _mume_backwin_set_cursor,
        _x11_backwin_set_cursor,
        _mume_backwin_raise,
        _x11_backwin_raise,
        _mume_backwin_lower,
        _x11_backwin_lower,
        _mume_backwin_begin_paint,
        _x11_backwin_begin_paint,
        _mume_backwin_end_paint,
        _x11_backwin_end_paint,
        _mume_backwin_grab_pointer,
        _x11_backwin_grab_pointer,
        _mume_backwin_ungrab_pointer,
        _x11_backwin_ungrab_pointer,
        _mume_x11_backwin_handle_event,
        _x11_backwin_handle_event,
        _mume_x11_backwin_handle_key_press,
        _x11_backwin_handle_key_press,
        _mume_x11_backwin_handle_key_release,
        _x11_backwin_handle_key_release,
        _mume_x11_backwin_handle_button_press,
        _x11_backwin_handle_button_press,
        _mume_x11_backwin_handle_button_release,
        _x11_backwin_handle_button_release,
        _mume_x11_backwin_handle_motion_notify,
        _x11_backwin_handle_motion_notify,
        _mume_x11_backwin_handle_enter_notify,
        _x11_backwin_handle_enter_notify,
        _mume_x11_backwin_handle_leave_notify,
        _x11_backwin_handle_leave_notify,
        _mume_x11_backwin_handle_focus_in,
        _x11_backwin_handle_focus_in,
        _mume_x11_backwin_handle_focus_out,
        _x11_backwin_handle_focus_out,
        _mume_x11_backwin_handle_reparent,
        _x11_backwin_handle_reparent,
        _mume_x11_backwin_handle_expose,
        _x11_backwin_handle_expose,
        _mume_x11_backwin_handle_map,
        _x11_backwin_handle_map,
        _mume_x11_backwin_handle_unmap,
        _x11_backwin_handle_unmap,
        _mume_x11_backwin_handle_configure,
        _x11_backwin_handle_configure,
        _mume_x11_backwin_handle_client,
        _x11_backwin_handle_client,
        _mume_x11_backwin_handle_selection_request,
        _x11_backwin_handle_selection_request,
        _mume_x11_backwin_handle_selection_notify,
        _x11_backwin_handle_selection_notify,
        _mume_x11_backwin_handle_selection_clear,
        _x11_backwin_handle_selection_clear,
        MUME_FUNC_END);
}

const void* mume_x11_backwin_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "x11 backwin class",
        _x11_backwin_super_meta_class(),
        sizeof(struct _x11_backwin_class),
        MUME_PROP_END,
        _mume_ctor, _x11_backwin_class_ctor,
        MUME_FUNC_END);
}

void* mume_x11_backwin_new(
    void *backend, Window window, int managed)
{
    return mume_new(mume_x11_backwin_class(),
                    backend, window, managed);
}

void* mume_x11_backwin_get_backend(const void *_self)
{
    const struct _x11_backwin *self = _self;
    assert(mume_is_of(_self, mume_x11_backwin_class()));
    return self->backend;
}

Window mume_x11_backwin_get_xwindow(const void *_self)
{
    const struct _x11_backwin *self = _self;
    assert(mume_is_of(_self, mume_x11_backwin_class()));
    return self->window;
}

void mume_x11_set_sizehint(
    Display *display, Window window, int pref_width, int pref_height,
    int min_width, int min_height, int max_width, int max_height)
{
    XSizeHints *size_hints;

    size_hints = XAllocSizeHints();
    if (NULL == size_hints) {
        mume_error(("XAllocSizeHints\n"));
        return;
    }

    size_hints->flags = PPosition | PBaseSize | PMinSize | PMaxSize;
    size_hints->base_width = pref_width;
    size_hints->base_height = pref_height;
    size_hints->min_width = min_width;
    size_hints->min_height = min_height;
    size_hints->max_width = max_width;
    size_hints->max_height = max_height;

    XSetWMNormalHints(display, window, size_hints);
    XFree(size_hints);
}

Window mume_x11_create_window(
    Display *display, int type, Window parent,
    int x, int y, unsigned int width, unsigned int height,
    unsigned int clazz, int eventmask)
{
    int screen;
    int depth;
    Visual *visual;
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    XSizeHints *size_hints;
    XWMHints *wm_hints;
    XClassHint *class_hints;
    Window window;

    screen = DefaultScreen(display);
    visual = DefaultVisual(display, screen);

    if (clazz != InputOnly) {
        depth = DefaultDepth(display, screen);
        valuemask = CWBackPixel | CWBorderPixel;
        attributes.background_pixel = WhitePixel(display, screen);
        attributes.border_pixel = BlackPixel(display, screen);

        if (MUME_BACKWIN_MENU == type) {
            assert(RootWindow(display, screen) == parent);

            valuemask |= CWSaveUnder | CWOverrideRedirect;
            attributes.save_under = True;
            attributes.override_redirect = True;
        }
    }
    else {
        depth = 0;
        valuemask = 0;
    }

    if (None == parent)
        parent = RootWindow(display, screen);

    window = XCreateWindow(
        display, parent, x, y, width, height, 0, depth,
        clazz, visual, valuemask, &attributes);

    if (clazz != InputOnly) {
        /* Setup standard properties. */
        if (!(size_hints = XAllocSizeHints()))
            mume_abort(("allocating memory failed!\n"));

        if (!(wm_hints = XAllocWMHints()))
            mume_abort(("allocating memory failed!\n"));

        if (!(class_hints = XAllocClassHint()))
            mume_abort(("allocating memory failed!\n"));

        size_hints->flags = PPosition | PSize | PMinSize;
        size_hints->min_width = 0;
        size_hints->min_height = 0;
        wm_hints->initial_state = NormalState;
        wm_hints->input = True;
        /* wm_hints->icon_pixmap = icon_pixmap; */
        wm_hints->flags = StateHint/* | IconPixmapHint*/ | InputHint;
        class_hints->res_name = "mume";
        class_hints->res_class = "mume";

        /* or use XSizeHints, XSetClassHint, XSetWMHints */
        XmbSetWMProperties(display, window, "mume", "mume",
                           NULL, 0, size_hints, wm_hints, class_hints);

        XFree(wm_hints);
        XFree(class_hints);
        XFree(size_hints);
    }

    /* Select event */
    XSelectInput(display, window, eventmask);

    return window;
}

void _mume_x11_backwin_handle_event(
    const void *_clazz, void *_self, XEvent *xevent)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_event, (_self, xevent));
}

void _mume_x11_backwin_handle_key_press(
    const void *_clazz, void *_self, XKeyEvent *xkey)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_key_press, (_self, xkey));
}

void _mume_x11_backwin_handle_key_release(
    const void *_clazz, void *_self, XKeyEvent *xkey)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_key_release, (_self, xkey));
}

void _mume_x11_backwin_handle_button_press(
    const void *_clazz, void *_self, XButtonEvent *xbutton)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_button_press,
        (_self, xbutton));
}

void _mume_x11_backwin_handle_button_release(
    const void *_clazz, void *_self, XButtonEvent *xbutton)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_button_release,
        (_self, xbutton));
}

void _mume_x11_backwin_handle_motion_notify(
    const void *_clazz, void *_self, XMotionEvent *xmotion)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_motion_notify,
        (_self, xmotion));
}

void _mume_x11_backwin_handle_enter_notify(
    const void *_clazz, void *_self, XCrossingEvent *xcrossing)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_enter_notify,
        (_self, xcrossing));
}

void _mume_x11_backwin_handle_leave_notify(
    const void *_clazz, void *_self, XCrossingEvent *xcrossing)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_leave_notify,
        (_self, xcrossing));
}

void _mume_x11_backwin_handle_focus_in(
    const void *_clazz, void *_self, XFocusChangeEvent *xfocus)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_focus_in, (_self, xfocus));
}

void _mume_x11_backwin_handle_focus_out(
    const void *_clazz, void *_self, XFocusChangeEvent *xfocus)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_focus_out, (_self, xfocus));
}

void _mume_x11_backwin_handle_reparent(
    const void *_clazz, void *_self, XReparentEvent *xreparent)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_reparent,
        (_self, xreparent));
}

void _mume_x11_backwin_handle_expose(
    const void *_clazz, void *_self, XExposeEvent *xexpose)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_expose, (_self, xexpose));
}

void _mume_x11_backwin_handle_map(
    const void *_clazz, void *_self, XMapEvent *xmap)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_map, (_self, xmap));
}

void _mume_x11_backwin_handle_unmap(
    const void *_clazz, void *_self, XUnmapEvent *xunmap)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_unmap, (_self, xunmap));
}

void _mume_x11_backwin_handle_configure(
    const void *_clazz, void *_self, XConfigureEvent *xconfigure)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_configure,
        (_self, xconfigure));
}

void _mume_x11_backwin_handle_client(
    const void *_clazz, void *_self, XClientMessageEvent *xclient)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_client, (_self, xclient));
}

void _mume_x11_backwin_handle_selection_request(
    const void *_clazz, void *_self, XSelectionRequestEvent *xrequest)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_selection_request,
        (_self, xrequest));
}

void _mume_x11_backwin_handle_selection_notify(
    const void *_clazz, void *_self, XSelectionEvent *xselection)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_selection_notify,
        (_self, xselection));
}

void _mume_x11_backwin_handle_selection_clear(
    const void *_clazz, void *_self, XSelectionClearEvent *xclear)
{
    MUME_SELECTOR_NORETURN(
        mume_x11_backwin_meta_class(), mume_x11_backwin_class(),
        struct _x11_backwin_class, handle_selection_clear,
        (_self, xclear));
}
