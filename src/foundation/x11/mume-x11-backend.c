/*==================================================
 * Copyright 2011-2012 Choostone Software, Icn
 * James Wong, jamone@126.com
 *==================================================*/
#include "mume-x11-backend.h"
#include "mume-debug.h"
#include "mume-events.h"
#include "mume-memory.h"
#include "mume-oset.h"
#include "mume-x11-backwin.h"
#include "mume-x11-clipboard.h"
#include "mume-x11-cursor.h"
#include "mume-x11-datasrc.h"
#include "mume-x11-dbgutil.h"
#include "mume-x11-util.h"
#include MUME_ASSERT_H

#define _x11_backend_super_class mume_backend_class

struct _window_pair {
    Window xwindow;
    void *window;
};

struct _x11_backend {
    const char _[MUME_SIZEOF_BACKEND];
    Display *display;
    int screen;
    int wakeup_pipe[2];
    mume_oset_t *windows;
    void *root_window;
};

struct _x11_backend_class {
    const char _[MUME_SIZEOF_BACKEND_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _x11_backend) ==
                   MUME_SIZEOF_X11_BACKEND);

MUME_STATIC_ASSERT(sizeof(struct _x11_backend_class) ==
                   MUME_SIZEOF_X11_BACKEND_CLASS);

static int _x11_error_handler(Display *display, XErrorEvent *xerror)
{
    char buffer[64];
    XGetErrorText(display, xerror->error_code,
                  buffer, COUNT_OF(buffer));

    mume_error(("%s\n", buffer));
    return 0;
}

static int _window_pair_compare(const void *a, const void *b)
{
    const struct _window_pair *p1 = a;
    const struct _window_pair *p2 = b;
    return (int)p1->xwindow - (int)p2->xwindow;
}

static void* _x11_backend_ctor(
    struct _x11_backend *self, int mode, va_list *app)
{
    Window window;

    XSetErrorHandler(_x11_error_handler);
    mume_x11_init_keymap();

    if (!_mume_ctor(_x11_backend_super_class(), self, mode, app))
        return NULL;

    self->display = XOpenDisplay(NULL);
    self->screen = DefaultScreen(self->display);
    self->windows = mume_oset_new(_window_pair_compare, NULL, 0);
    window = RootWindow(self->display, self->screen);
    self->root_window =  mume_x11_backwin_new(self, window, 0);

    if (0 != pipe(self->wakeup_pipe))
        mume_error(("Open wakeup pipe failed\n"));

    return self;
}

static void* _x11_backend_dtor(struct _x11_backend *self)
{
    mume_refobj_release(self->root_window);
    assert(0 == mume_oset_size(self->windows));

    mume_oset_delete(self->windows);
    close(self->wakeup_pipe[0]);
    close(self->wakeup_pipe[1]);
    XCloseDisplay(self->display);

    return _mume_dtor(_x11_backend_super_class(), self);
}

static void _x11_backend_screen_size(
    struct _x11_backend *self, int *width, int *height)
{
    if (width) {
        *width = DisplayWidth(self->display, self->screen);
    }

    if (height) {
        *height = DisplayHeight(self->display, self->screen);
    }
}

static void* _x11_backend_data_format(
    struct _x11_backend *self, const char *name)
{
    return mume_x11_datafmt_new(self->display, name);
}

static void* _x11_backend_clipboard(struct _x11_backend *self)
{
    return mume_x11_clipboard_new(self);
}

static void* _x11_backend_root_backwin(struct _x11_backend *self)
{
    return self->root_window;
}

static void* _x11_backend_create_backwin(
    struct _x11_backend *self, int type, void *parent,
    int x, int y, int width, int height)
{
    int eventmask =
            ButtonPressMask | ButtonReleaseMask |
            ButtonMotionMask | KeyPressMask | KeyReleaseMask |
            KeymapStateMask | PointerMotionMask | EnterWindowMask |
            LeaveWindowMask | FocusChangeMask | ExposureMask |
            PropertyChangeMask | StructureNotifyMask;

    Window window = mume_x11_create_window(
        self->display, type, mume_x11_backwin_get_xwindow(parent),
        x, y, width, height, InputOutput, eventmask);

    Atom delete_window = XInternAtom(
        self->display, "WM_DELETE_WINDOW", False);

    /* Intercept close window message. */
    XSetWMProtocols(self->display, window, &delete_window, 1);
    XFlush(self->display);

    return mume_x11_backwin_new(self, window, 1);
}

static void* _x11_backend_create_cursor(
    struct _x11_backend *self, int id)
{
    Cursor cursor = mume_x11_create_cursor(self->display, id);
    if (cursor)
        return mume_x11_cursor_new(self->display, cursor);

    return NULL;
}

static int _x11_backend_handle_event(
    struct _x11_backend *self, int wait)
{
    if (!XPending(self->display)) {
        fd_set rfds;
        int xfd = ConnectionNumber(self->display);
        int nfd = MAX(xfd, self->wakeup_pipe[0]);
        int result = 0;

        FD_ZERO(&rfds);
        FD_SET(xfd, &rfds);
        FD_SET(self->wakeup_pipe[0], &rfds);
        if (MUME_WAIT_INFINITE == wait) {
            result = select(nfd + 1, &rfds, 0, 0, NULL);
        }
        else if (wait > 0) {
            struct timeval tv;
            tv.tv_sec = wait / 1000;
            tv.tv_usec = wait % 1000 * 1000;
            result = select(nfd + 1, &rfds, 0, 0, &tv);
        }

        if (result > 0 && FD_ISSET(self->wakeup_pipe[0], &rfds)) {
            int cmd, n;
            n = read(self->wakeup_pipe[0], &cmd, sizeof(cmd));
            mume_debug(("Wakeup command: %d:%d\n", cmd, n));
        }
    }

    if (XPending(self->display)) {
        XEvent xevent;
        XNextEvent(self->display, &xevent);
        mume_x11_backend_dispatch(self, &xevent);
    }

    return 1;
}

static int _x11_backend_wakeup_event(struct _x11_backend *self)
{
    int cmd = 0;
    return write(self->wakeup_pipe[1], &cmd, sizeof(cmd));
}

static void _x11_backend_query_pointer(
    struct _x11_backend *self, int *x, int *y, int *state)
{
    Window rw, cw;
    int rx = 0, ry = 0, cx = 0, cy = 0;
    unsigned int xstate = 0;

    rw = RootWindow(self->display, self->screen);
    if (!XQueryPointer(self->display, rw, &rw, &cw,
                       &rx, &ry, &cx, &cy, &xstate))
    {
        mume_warning(("XQueryPointer failed\n"));
    }

    if (x)
        *x = rx;

    if (y)
        *y = ry;

    if (state)
        *state = mume_x11_translate_state(xstate);
}

const void* mume_x11_backend_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_backend_meta_class(),
        "x11 backend",
        _x11_backend_super_class(),
        sizeof(struct _x11_backend),
        MUME_PROP_END,
        _mume_ctor, _x11_backend_ctor,
        _mume_dtor, _x11_backend_dtor,
        _mume_backend_screen_size,
        _x11_backend_screen_size,
        _mume_backend_data_format,
        _x11_backend_data_format,
        _mume_backend_clipboard,
        _x11_backend_clipboard,
        _mume_backend_root_backwin,
        _x11_backend_root_backwin,
        _mume_backend_create_backwin,
        _x11_backend_create_backwin,
        _mume_backend_create_cursor,
        _x11_backend_create_cursor,
        _mume_backend_handle_event,
        _x11_backend_handle_event,
        _mume_backend_wakeup_event,
        _x11_backend_wakeup_event,
        _mume_backend_query_pointer,
        _x11_backend_query_pointer,
        MUME_FUNC_END);
}

const void* mume_backend_class_sym(void)
{
    return mume_x11_backend_class();
}

Display* mume_x11_backend_get_display(const void *_self)
{
    const struct _x11_backend *self = _self;
    assert(mume_is_of(_self, mume_x11_backend_class()));
    return self->display;
}

int mume_x11_backend_get_screen(const void *_self)
{
    const struct _x11_backend *self = _self;
    assert(mume_is_of(_self, mume_x11_backend_class()));
    return self->screen;
}

void mume_x11_backend_bind(void *_self, void *window)
{
    struct _x11_backend *self = _self;
    struct _window_pair *pair;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_x11_backend_class()));
    assert(mume_is_of(window, mume_x11_backwin_class()));

    node = mume_oset_newnode(sizeof(*pair));
    pair = mume_oset_data(node);
    pair->window = window;
    pair->xwindow = mume_x11_backwin_get_xwindow(window);

    mume_oset_insert(self->windows, node);
}

void mume_x11_backend_unbind(void *_self, void *window)
{
    struct _x11_backend *self = _self;
    struct _window_pair pair;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_x11_backend_class()));
    assert(mume_is_of(window, mume_x11_backwin_class()));

    pair.xwindow = mume_x11_backwin_get_xwindow(window);
    node = mume_oset_find(self->windows, &pair);
    if (node)
        mume_oset_erase(self->windows, node);
}

void mume_x11_backend_dispatch(void *_self, XEvent *xevent)
{
    struct _x11_backend *self = _self;
    struct _window_pair key;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_x11_backend_class()));

    mume_x11_print_event(xevent);

    if (XFilterEvent(xevent, None))
        return;

    key.xwindow = xevent->xany.window;
    node = mume_oset_find(self->windows, &key);

    if (node) {
        void *window = ((struct _window_pair*)(
            mume_oset_data(node)))->window;

        mume_x11_backwin_handle_event(window, xevent);
    }
    else if (MappingNotify == xevent->type) {
        XRefreshKeyboardMapping(&xevent->xmapping);
    }
}
