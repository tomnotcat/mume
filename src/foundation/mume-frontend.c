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
#include "mume-frontend.h"
#include "mume-backwin.h"
#include "mume-debug.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-olist.h"
#include "mume-time.h"
#include "mume-urgnmgr.h"
#include "mume-window.h"
#include MUME_ASSERT_H

enum _pointer_grab_mask_e {
    _POINTER_GRAB_LEAVE = 0x0001,
    _POINTER_GRAB_HOVER = 0x0002,
    _POINTER_GRAB_LBUTTON  = 0x0004,
    _POINTER_GRAB_MBUTTON  = 0x0008,
    _POINTER_GRAB_RBUTTON  = 0x0010,
    _POINTER_GRAB_XBUTTON1 = 0x0020,
    _POINTER_GRAB_XBUTTON2 = 0x0040,
    _POINTER_GRAB_ACTIVELY = 0x0080
};

#define _POINTER_GRAB_BUTTONS \
    (_POINTER_GRAB_LBUTTON |  \
     _POINTER_GRAB_MBUTTON |  \
     _POINTER_GRAB_RBUTTON |  \
     _POINTER_GRAB_XBUTTON1 | \
     _POINTER_GRAB_XBUTTON2)

#define _frontend_super_class mume_object_class
#define _frontend_super_meta_class mume_meta_class

#define _frontend_has_popup(_self) \
    ((_self)->popups && mume_octnr_size((_self)->popups) > 0)

struct _frontend {
    const char _[MUME_SIZEOF_OBJECT];
    void *input_focus;
    void *pointer_owner;
    void *popups;
    unsigned int pointer_grab;
    int last_click_x;
    int last_click_y;
    int last_click_num;
    void *last_click_win;
    mume_timeval_t last_click_tv;
};

struct _frontend_class {
    const char _[MUME_SIZEOF_CLASS];
    void (*handle_keydown)(
        void *self, void *bwin, int x, int y, int state, int keysym);
    void (*handle_keyup)(
        void *self, void *bwin, int x, int y, int state, int keysym);
    void (*handle_buttondown)(
        void *self, void *bwin, int x, int y, int state, int button);
    void (*handle_buttonup)(
        void *self, void *bwin, int x, int y, int state, int button);
    void (*handle_mousemotion)(
        void *self, void *bwin, int x, int y, int state);
    void (*handle_mouseenter)(
        void *self, void *bwin, int x, int y, int state, int mode);
    void (*handle_mouseleave)(
        void *self, void *bwin, int x, int y, int state, int mode);
    void (*handle_focusin)(void *self, void *bwin, int mode);
    void (*handle_focusout)(void *self, void *bwin, int mode);
    void (*handle_map)(void *self, void *bwin);
    void (*handle_unmap)(void *self, void *bwin);
    void (*handle_geometry)(
        void *self, void *bwin, int x, int y, int width, int height);
    void (*handle_expose)(
        void *self, void *bwin, const mume_rect_t *rect);
    void (*handle_close)(void *self, void *bwin);
    void (*handle_quit)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _frontend) == MUME_SIZEOF_FRONTEND);
MUME_STATIC_ASSERT(sizeof(struct _frontend_class) ==
                   MUME_SIZEOF_FRONTEND_CLASS);

static void* _frontend_ctor(
    struct _frontend *self, int mode, va_list *app)
{
    if (!_mume_ctor(_frontend_super_class(), self, mode, app))
        return NULL;

    self->input_focus = NULL;
    self->pointer_owner = NULL;
    self->popups = NULL;
    self->pointer_grab = 0;
    self->last_click_x = 0;
    self->last_click_y = 0;
    self->last_click_num = 0;
    self->last_click_win = NULL;

    return self;
}

static void* _frontend_dtor(struct _frontend *self)
{
    mume_delete(self->popups);
    return _mume_dtor(_frontend_super_class(), self);
}

static unsigned int _pointer_grab_mask(int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        return _POINTER_GRAB_LBUTTON;
    case MUME_BUTTON_MIDDLE:
        return _POINTER_GRAB_MBUTTON;
    case MUME_BUTTON_RIGHT:
        return _POINTER_GRAB_RBUTTON;
    case MUME_BUTTON_X1:
        return _POINTER_GRAB_XBUTTON1;
    case MUME_BUTTON_X2:
        return _POINTER_GRAB_XBUTTON2;
    }

    return 0;
}

static mume_list_t* _create_crossing_list(
    const void *win, const void *anc)
{
    mume_list_t *l = mume_list_new(NULL, NULL);
    mume_list_node_t *n;

    while ((win = mume_window_parent(win)) && win != anc) {
        n = mume_list_push_front(l, sizeof(const void*));
        *(const void**)mume_list_data(n) = win;
    }

    return l;
}

static void* _least_common_ancestor(const void *win1, const void *win2)
{
    void *root_window = mume_root_window();
    mume_list_t *l1 = _create_crossing_list(win1, root_window);
    mume_list_t *l2 = _create_crossing_list(win2, root_window);
    mume_list_node_t *n1 = mume_list_front(l1);
    mume_list_node_t *n2 = mume_list_front(l2);
    void *r = root_window;

    while (n1 && n2) {
        if (*(void**)mume_list_data(n1) != *(void**)mume_list_data(n2))
            break;

        r = *(void**)mume_list_data(n1);
        n1 = mume_list_next(n1);
        n2 = mume_list_next(n2);
    }

    mume_list_delete(l1);
    mume_list_delete(l2);

    return r;
}

static void _pointer_leave_between(
    void *inf, void *anc, mume_event_t *event, int *x, int *y)
{
    int tx, ty;

    mume_window_get_geometry(inf, &tx, &ty, NULL, NULL);
    *x += tx;
    *y += ty;

    inf = mume_window_parent(inf);
    while (inf != anc) {
        event->crossing.window = inf;
        event->crossing.x = *x;
        event->crossing.y = *y;
        _mume_post_event(*event, 0);

        mume_window_get_geometry(inf, &tx, &ty, NULL, NULL);
        *x += tx;
        *y += ty;

        inf = mume_window_parent(inf);
    }
}

static void _pointer_enter_between(
    void *anc, void *inf, mume_event_t *event, int *x, int *y)
{
    mume_list_t *l;
    mume_list_node_t *n;
    int tx, ty;

    l = _create_crossing_list(inf, anc);
    n = mume_list_front(l);
    while (n) {
        anc = *(void**)mume_list_data(n);
        mume_window_get_geometry(anc, &tx, &ty, NULL, NULL);
        *x -= tx;
        *y -= ty;

        event->crossing.window = anc;
        event->crossing.x = *x;
        event->crossing.y = *y;
        _mume_post_event(*event, 0);

        n = mume_list_next(n);
    }

    mume_window_get_geometry(inf, &tx, &ty, NULL, NULL);
    *x -= tx;
    *y -= ty;

    mume_list_delete(l);
}

/* Crossing mouse pointer from one window to another,
   the rules for generating event is similar to xlib. */
static void _frontend_change_pointer_owner(
    struct _frontend *self, void *win,
    int x, int y, int state, int mode)
{
    mume_event_t event;
    void *root_window = mume_root_window();
    void *pointer_owner = mume_frontend_get_pointer_owner(self);
    void *bwin;
    void *cursor;

    assert(!self->pointer_grab);
    if (NULL == win) {
        win = mume_window_from_point(root_window, x, y);
        if (win) {
            mume_translate_coords(root_window, win, &x, &y);
        }
        else {
            win = root_window;
        }
    }

    if (pointer_owner == win)
        return;

    if (mume_window_is_ancestor(pointer_owner, win)) {
        /* From inferior to ancestor. */
        /* Leave the old mouse owner. */
        mume_translate_coords(win, pointer_owner, &x, &y);
        event.crossing.type = MUME_EVENT_MOUSELEAVE;
        event.crossing.window = pointer_owner;
        event.crossing.x = x;
        event.crossing.y = y;
        event.crossing.state = state;
        event.crossing.mode = mode;
        event.crossing.detail = MUME_NOTIFY_ANCESTOR;
        _mume_post_event(event, 0);
        /* Leave the windows between old and new mouse owner. */
        event.crossing.detail = MUME_NOTIFY_VIRTUAL;
        _pointer_leave_between(pointer_owner, win, &event, &x, &y);
        /* Enter the new mouse owner. */
        event.crossing.type = MUME_EVENT_MOUSEENTER;
        event.crossing.x = x;
        event.crossing.y = y;
        event.crossing.window = win;
        event.crossing.detail = MUME_NOTIFY_INFERIOR;
        _mume_post_event(event, 0);
    }
    else if (mume_window_is_ancestor(win, pointer_owner)) {
        /* From ancestor to inferior. */
        /* Leave the old mouse owner. */
        mume_translate_coords(win, pointer_owner, &x, &y);
        event.crossing.type = MUME_EVENT_MOUSELEAVE;
        event.crossing.window = pointer_owner;
        event.crossing.x = x;
        event.crossing.y = y;
        event.crossing.state = state;
        event.crossing.mode = mode;
        event.crossing.detail = MUME_NOTIFY_INFERIOR;
        _mume_post_event(event, 0);
        /* Enter the windows between old and new mouse owner. */
        event.crossing.type = MUME_EVENT_MOUSEENTER;
        event.crossing.detail = MUME_NOTIFY_VIRTUAL;
        _pointer_enter_between(pointer_owner, win, &event, &x, &y);
        /* Enter the new mouse owner. */
        event.crossing.window = win;
        event.crossing.x = x;
        event.crossing.y = y;
        event.crossing.detail = MUME_NOTIFY_ANCESTOR;
        _mume_post_event(event, 0);
    }
    else {
        /* From to different window tree. */
        void *anc = _least_common_ancestor(pointer_owner, win);
        /* Leave the old mouse owner. */
        mume_translate_coords(win, pointer_owner, &x, &y);
        event.crossing.type = MUME_EVENT_MOUSELEAVE;
        event.crossing.window = pointer_owner;
        event.crossing.x = x;
        event.crossing.y = x;
        event.crossing.state = state;
        event.crossing.mode = mode;
        event.crossing.detail = MUME_NOTIFY_NONLINEAR;
        _mume_post_event(event, 0);
        /* Leave the windows between old mouse owner
           and common ancestor. */
        event.crossing.detail = MUME_NOTIFY_NONLINEARVIRTUAL;
        _pointer_leave_between(pointer_owner, anc, &event, &x, &y);
        /* Enter the windows between common ancestor
           and new mouse owner. */
        event.crossing.type = MUME_EVENT_MOUSEENTER;
        _pointer_enter_between(anc, win, &event, &x, &y);
        /* Enter the new mouse owner. */
        event.crossing.window = win;
        event.crossing.x = x;
        event.crossing.y = y;
        event.crossing.detail = MUME_NOTIFY_NONLINEAR;
        _mume_post_event(event, 0);
    }

    self->pointer_owner = win;

    /* Change cursor. */
    bwin = mume_window_seek_backwin(self->pointer_owner, NULL, NULL);
    cursor = mume_window_seek_cursor(self->pointer_owner);

    mume_backwin_set_cursor(bwin, cursor);
}

static void* _frontend_popup_from_point(
    struct _frontend *self, void *win, int x, int y)
{
    void *root, *it;
    mume_rect_t r;

    root = mume_root_window();
    mume_translate_coords(win, root, &x, &y);

    mume_octnr_foreach(self->popups, it, win) {
        mume_window_get_geometry(
            win, NULL, NULL, &r.width, &r.height);

        r.x = r.y = 0;
        mume_translate_coords(win, root, &r.x, &r.y);

        if (mume_rect_inside(r, x, y))
            return win;
    }

    return NULL;
}

static void _frontend_close_popups(struct _frontend *self)
{
    void *it, *win;

    /* Notify all the popup windows to close. */
    mume_octnr_foreach(self->popups, it, win) {
        mume_post_event(mume_make_close_event(win));
    }
}

static void _frontend_handle_keydown(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int keysym)
{
    mume_event_t event;
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    event.key.type = MUME_EVENT_KEYDOWN;
    event.key.window = mume_frontend_get_keyboard_owner(self);
    event.key.x = x;
    event.key.y = y;
    event.key.state = state;
    event.key.keysym = keysym;
    _mume_post_event(event, 0);
}

static void _frontend_handle_keyup(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int keysym)
{
    mume_event_t event;
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    event.key.type = MUME_EVENT_KEYUP;
    event.key.window = mume_frontend_get_keyboard_owner(self);
    event.key.x = x;
    event.key.y = y;
    event.key.state = state;
    event.key.keysym = keysym;
    _mume_post_event(event, 0);
}

static void _frontend_handle_buttondown(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int button)
{
    mume_event_t event;
    void *win, *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    if (self->pointer_grab) {
        win = mume_frontend_get_pointer_owner(self);
    }
    else if (_frontend_has_popup(self)) {
        win = _frontend_popup_from_point(self, twin, x, y);

        if (NULL == win) {
            _frontend_close_popups(self);
            return;
        }
    }
    else {
        win = mume_window_from_point(twin, x, y);
    }

    if (win) {
        mume_translate_coords(twin, win, &x, &y);
    }
    else {
        win = twin;
    }

    event.button.type = MUME_EVENT_BUTTONDOWN;
    event.button.window = win;
    event.button.x = x;
    event.button.y = y;
    event.button.state = state;
    event.button.button = button;
    _mume_post_event(event, 0);

    /* Multiple click. */
    if (win == self->last_click_win &&
        x == self->last_click_x && y == self->last_click_y)
    {
        mume_timeval_t now, elapse, interval;

        mume_gettimeofday(&now);
        elapse = mume_timeval_sub(
            &now, &self->last_click_tv);

        self->last_click_tv = now;
        interval = mume_timeval_make(0, 400, 0);

        if (mume_timeval_cmp(&elapse, &interval) < 0) {
            ++self->last_click_num;
            if ((self->last_click_num % 2) == 0) {
                event.button.type = MUME_EVENT_BUTTONDBLCLK;
                _mume_post_event(event, 0);
            }

            if ((self->last_click_num % 3) == 0) {
                event.button.type = MUME_EVENT_BUTTONTPLCLK;
                _mume_post_event(event, 0);
            }
        }
        else {
            self->last_click_num = 1;
        }
    }
    else {
        self->last_click_num = 1;
        mume_gettimeofday(&self->last_click_tv);
    }

    self->last_click_win = win;
    self->last_click_x = x;
    self->last_click_y = y;

    /* Auto grab pointer. */
    if (!_frontend_has_popup(self)) {
        if (!self->pointer_grab) {
            _frontend_change_pointer_owner(
                self, win, x, y, state, MUME_NOTIFY_GRAB);

            self->pointer_grab |= _POINTER_GRAB_HOVER;
        }

        self->pointer_grab |= _pointer_grab_mask(button);
    }
}

static void _frontend_handle_buttonup(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int button)
{
    mume_event_t event;
    void *win, *twin = mume_backwin_get_attached(bwin);
    int tx = x, ty = y;

    if (self->pointer_grab) {
        win = mume_frontend_get_pointer_owner(self);
    }
    else if (_frontend_has_popup(self)) {
        win = _frontend_popup_from_point(self, twin, x, y);

        if (NULL == win) {
            _frontend_close_popups(self);
            return;
        }
    }
    else {
        win = mume_window_from_point(twin, x, y);
    }

    if (win) {
        mume_translate_coords(twin, win, &tx, &ty);
    }
    else {
        win = twin;
    }

    event.button.type = MUME_EVENT_BUTTONUP;
    event.button.window = win;
    event.button.x = tx;
    event.button.y = ty;
    event.button.state = state;
    event.button.button = button;
    _mume_post_event(event, 0);

    if (self->pointer_grab) {
        self->pointer_grab &= ~_pointer_grab_mask(button);

        if (0 == (_POINTER_GRAB_BUTTONS & self->pointer_grab)) {
            self->pointer_grab = 0;
            win = mume_window_from_point(twin, x, y);

            if (NULL == win)
                win = mume_root_window();

            mume_translate_coords(twin, win, &x, &y);
            _frontend_change_pointer_owner(
                self, win, x, y, state, MUME_NOTIFY_UNGRAB);
        }
    }
}

static void _frontend_handle_mousemotion(
    struct _frontend *self, void *bwin, int x, int y, int state)
{
    mume_event_t event;
    void *win, *pointer_owner;
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    if (self->pointer_grab) {
        pointer_owner = mume_frontend_get_pointer_owner(self);
        win = mume_window_from_point(twin, x, y);
        mume_translate_coords(twin, pointer_owner, &x, &y);

        if (self->pointer_grab & _POINTER_GRAB_HOVER) {
            if (win != pointer_owner) {
                event.crossing.type = MUME_EVENT_MOUSELEAVE;
                event.crossing.window = pointer_owner;
                event.crossing.x = x;
                event.crossing.y = y;
                event.crossing.state = state;
                event.crossing.mode = MUME_NOTIFY_NORMAL;
                event.crossing.detail = MUME_NOTIFY_ANCESTOR;
                _mume_post_event(event, 0);

                self->pointer_grab &= ~_POINTER_GRAB_HOVER;
                self->pointer_grab |= _POINTER_GRAB_LEAVE;
            }
        }
        else if (win == pointer_owner) {
            event.crossing.type = MUME_EVENT_MOUSEENTER;
            event.crossing.window = pointer_owner;
            event.crossing.x = x;
            event.crossing.y = y;
            event.crossing.state = state;
            event.crossing.mode = MUME_NOTIFY_NORMAL;
            event.crossing.detail = MUME_NOTIFY_ANCESTOR;
            _mume_post_event(event, 0);

            self->pointer_grab &= ~_POINTER_GRAB_LEAVE;
            self->pointer_grab |= _POINTER_GRAB_HOVER;
        }

        /* For mouse motion event. */
        win = pointer_owner;
    }
    else {
        if (_frontend_has_popup(self)) {
            mume_translate_coords(twin, NULL, &x, &y);
            twin = mume_root_window();
        }

        win = mume_window_from_point(twin, x, y);

        if (win) {
            mume_translate_coords(twin, win, &x, &y);
        }
        else {
            win = twin;
        }

        _frontend_change_pointer_owner(
            self, win, x, y, state, MUME_NOTIFY_NORMAL);
    }

    event.motion.type = MUME_EVENT_MOUSEMOTION;
    event.motion.window = win;
    event.motion.x = x;
    event.motion.y = y;
    event.motion.state = state;
    _mume_post_event(event, 0);
}

static void _frontend_handle_mouseenter(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int mode)
{
    void *win, *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin || self->pointer_grab)
        return;

    win = mume_window_from_point(twin, x, y);
    if (win) {
        mume_translate_coords(twin, win, &x, &y);
    }
    else {
        win = twin;
    }

    _frontend_change_pointer_owner(self, win, x, y, state, mode);
}

static void _frontend_handle_mouseleave(
    struct _frontend *self, void *bwin,
    int x, int y, int state, int mode)
{
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin || self->pointer_grab)
        return;

    mume_translate_coords(twin, mume_root_window(), &x, &y);
    _frontend_change_pointer_owner(self, NULL, x, y, state, mode);
}

static void _frontend_handle_focusin(
    void *self, void *bwin, int mode)
{
}

static void _frontend_handle_focusout(
    void *self, void *bwin, int mode)
{
}

static void _frontend_handle_map(void *self, void *bwin)
{
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    _mume_window_map(twin, 0);
}

static void _frontend_handle_unmap(void *self, void *bwin)
{
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    _mume_window_unmap(twin, 0);
}

static void _frontend_handle_geometry(
    void *self, void *bwin, int x, int y, int width, int height)
{
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    _mume_window_set_geometry(twin, x, y, width, height, 0);
}

static void _frontend_handle_expose(
    void *self, void *bwin, const mume_rect_t *rect)
{
    cairo_region_t *rgn;
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    if (rect) {
        rgn = cairo_region_create_rectangle(rect);
    }
    else {
        rgn = mume_window_region_create(twin);
    }

    mume_urgnmgr_set_urgn(
        mume_urgnmgr(), twin, NULL, rgn, MUME_URGN_UNION, 1);

    cairo_region_destroy(rgn);
}

static void _frontend_handle_close(void *self, void *bwin)
{
    mume_event_t event;
    void *twin = mume_backwin_get_attached(bwin);

    if (NULL == twin)
        return;

    event.close.type = MUME_EVENT_CLOSE;
    event.close.window = twin;
    _mume_post_event(event, 0);
}

static void _frontend_handle_quit(void *self)
{
    assert(0);
}

static void* _frontend_class_ctor(
    struct _frontend_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_frontend_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);
        if (selector == (voidf*)_mume_frontend_handle_keydown)
            *(voidf**)&self->handle_keydown = method;
        else if (selector == (voidf*)_mume_frontend_handle_keyup)
            *(voidf**)&self->handle_keyup = method;
        else if (selector == (voidf*)_mume_frontend_handle_buttondown)
            *(voidf**)&self->handle_buttondown = method;
        else if (selector == (voidf*)_mume_frontend_handle_buttonup)
            *(voidf**)&self->handle_buttonup = method;
        else if (selector == (voidf*)_mume_frontend_handle_mousemotion)
            *(voidf**)&self->handle_mousemotion = method;
        else if (selector == (voidf*)_mume_frontend_handle_mouseenter)
            *(voidf**)&self->handle_mouseenter = method;
        else if (selector == (voidf*)_mume_frontend_handle_mouseleave)
            *(voidf**)&self->handle_mouseleave = method;
        else if (selector == (voidf*)_mume_frontend_handle_focusin)
            *(voidf**)&self->handle_focusin = method;
        else if (selector == (voidf*)_mume_frontend_handle_focusout)
            *(voidf**)&self->handle_focusout = method;
        else if (selector == (voidf*)_mume_frontend_handle_map)
            *(voidf**)&self->handle_map = method;
        else if (selector == (voidf*)_mume_frontend_handle_unmap)
            *(voidf**)&self->handle_unmap = method;
        else if (selector == (voidf*)_mume_frontend_handle_geometry)
            *(voidf**)&self->handle_geometry = method;
        else if (selector == (voidf*)_mume_frontend_handle_expose)
            *(voidf**)&self->handle_expose = method;
        else if (selector == (voidf*)_mume_frontend_handle_close)
            *(voidf**)&self->handle_close = method;
        else if (selector == (voidf*)_mume_frontend_handle_quit)
            *(voidf**)&self->handle_quit = method;
    }

    return self;
}

const void* mume_frontend_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_frontend_meta_class(),
        "frontend",
        _frontend_super_class(),
        sizeof(struct _frontend),
        MUME_PROP_END,
        _mume_ctor, _frontend_ctor,
        _mume_dtor, _frontend_dtor,
        _mume_frontend_handle_keydown,
        _frontend_handle_keydown,
        _mume_frontend_handle_keyup,
        _frontend_handle_keyup,
        _mume_frontend_handle_buttondown,
        _frontend_handle_buttondown,
        _mume_frontend_handle_buttonup,
        _frontend_handle_buttonup,
        _mume_frontend_handle_mousemotion,
        _frontend_handle_mousemotion,
        _mume_frontend_handle_mouseenter,
        _frontend_handle_mouseenter,
        _mume_frontend_handle_mouseleave,
        _frontend_handle_mouseleave,
        _mume_frontend_handle_focusin,
        _frontend_handle_focusin,
        _mume_frontend_handle_focusout,
        _frontend_handle_focusout,
        _mume_frontend_handle_map,
        _frontend_handle_map,
        _mume_frontend_handle_unmap,
        _frontend_handle_unmap,
        _mume_frontend_handle_geometry,
        _frontend_handle_geometry,
        _mume_frontend_handle_expose,
        _frontend_handle_expose,
        _mume_frontend_handle_close,
        _frontend_handle_close,
        _mume_frontend_handle_quit,
        _frontend_handle_quit,
        MUME_FUNC_END);
}

const void* mume_frontend_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "frontend class",
        _frontend_super_meta_class(),
        sizeof(struct _frontend_class),
        MUME_PROP_END,
        _mume_ctor, _frontend_class_ctor,
        MUME_FUNC_END);
}

void mume_frontend_set_pointer_owner(void *_self, void *window)
{
    struct _frontend *self = _self;
    int x, y, state;

    assert(mume_is_of(_self, mume_frontend_class()));
    assert(!window || mume_is_of(window, mume_window_class()));

    mume_query_pointer(window, &x, &y, &state);
    self->pointer_grab  = 0;

    if (window) {
        _frontend_change_pointer_owner(
            self, window, x, y, state, MUME_NOTIFY_GRAB);
        self->pointer_grab = _POINTER_GRAB_ACTIVELY;
    }
    else {
        _frontend_change_pointer_owner(
            self, window, x, y, state, MUME_NOTIFY_UNGRAB);
    }
}

void* mume_frontend_get_pointer_owner(const void *_self)
{
    const struct _frontend *self = _self;

    assert(mume_is_of(_self, mume_frontend_class()));

    if (self->pointer_owner)
        return self->pointer_owner;

    return mume_root_window();
}

void* mume_frontend_get_keyboard_owner(const void *_self)
{
    const struct _frontend *self = _self;

    assert(mume_is_of(_self, mume_frontend_class()));

    if (self->input_focus)
        return self->input_focus;

    return mume_frontend_get_pointer_owner(self);
}

void mume_frontend_open_popup(void *_self, void *window)
{
    struct _frontend *self = _self;

    assert(mume_is_of(_self, mume_frontend_class()));
    assert(mume_is_of(window, mume_window_class()));

    if (NULL == self->popups)
        self->popups = mume_olist_new(NULL);

    mume_olist_push_front(self->popups, window);

    if (1 == mume_octnr_size(self->popups)) {
        mume_backwin_grab_pointer(
            mume_window_seek_backwin(window, NULL, NULL));
    }
}

void mume_frontend_close_popup(void *_self, void *window)
{
    struct _frontend *self = _self;

    assert(mume_is_of(_self, mume_frontend_class()));

    if (self->popups) {
        mume_olist_remove(self->popups, window);

        if (0 == mume_octnr_size(self->popups)) {
            mume_backwin_ungrab_pointer(
                mume_window_seek_backwin(window, NULL, NULL));
        }
    }
}

void mume_frontend_remove_window(void *_self, void *window)
{
    struct _frontend *self = _self;

    assert(mume_is_of(_self, mume_frontend_class()));
    assert(!self->popups || !mume_octnr_find(self->popups, window));

    if (self->input_focus == window)
        self->input_focus = NULL;

    if (self->pointer_owner == window) {
        self->pointer_owner = NULL;
        self->pointer_grab = 0;
    }

    if (self->last_click_win == window)
        self->last_click_win = NULL;
}

void _mume_frontend_handle_keydown(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int keysym)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_keydown,
        (_self, bwin, x, y, state, keysym));
}

void _mume_frontend_handle_keyup(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int keysym)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_keyup,
        (_self, bwin, x, y, state, keysym));
}

void _mume_frontend_handle_buttondown(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_buttondown,
        (_self, bwin, x, y, state, button));
}

void _mume_frontend_handle_buttonup(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_buttonup,
        (_self, bwin, x, y, state, button));
}

void _mume_frontend_handle_mousemotion(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_mousemotion,
        (_self, bwin, x, y, state));
}

void _mume_frontend_handle_mouseenter(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int mode)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_mouseenter,
        (_self, bwin, x, y, state, mode));
}

void _mume_frontend_handle_mouseleave(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int state, int mode)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_mouseleave,
        (_self, bwin, x, y, state, mode));
}

void _mume_frontend_handle_focusin(
    const void *_clazz, void *_self, void *bwin, int mode)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_focusin, (_self, bwin, mode));
}

void _mume_frontend_handle_focusout(
    const void *_clazz, void *_self, void *bwin, int mode)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_focusout, (_self, bwin, mode));
}

void _mume_frontend_handle_map(
    const void *_clazz, void *_self, void *bwin)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_map, (_self, bwin));
}

void _mume_frontend_handle_unmap(
    const void *_clazz, void *_self, void *bwin)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_unmap, (_self, bwin));
}

void _mume_frontend_handle_geometry(
    const void *_clazz, void *_self, void *bwin,
    int x, int y, int width, int height)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_geometry,
        (_self, bwin, x, y, width, height));
}

void _mume_frontend_handle_expose(
    const void *_clazz, void *_self, void *bwin,
    const mume_rect_t *rect)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_expose, (_self, bwin, rect));
}

void _mume_frontend_handle_close(
    const void *_clazz, void *_self, void *bwin)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_close, (_self, bwin));
}

void _mume_frontend_handle_quit(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_frontend_meta_class(), mume_frontend_class(),
        struct _frontend_class, handle_quit, (_self));
}
