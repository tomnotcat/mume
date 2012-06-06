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
#include "mume-window.h"
#include "mume-backwin.h"
#include "mume-cursor.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-refobj.h"
#include "mume-userdata.h"
#include "mume-urgnmgr.h"
#include "mume-vector.h"
#include MUME_ASSERT_H

#define _window_super_class mume_object_class
#define _window_super_meta_class mume_meta_class

/* User data key for window id. */
static const mume_user_data_key_t _window_id_key;

/* User data key for window text. */
static const mume_user_data_key_t _window_text_key;

/* User data key for window cursor. */
static const mume_user_data_key_t _window_cursor_key;

enum _window_flags_e {
    MUME_WF_MAPPED,
    MUME_WF_DISABLED,
    MUME_WF_FOCUSABLE,
    MUME_WF_FOCUSCOPE,
    MUME_WF_LOCKDIRTY
};

struct _window {
    const char _[MUME_SIZEOF_OBJECT];
    struct _window *parent;
    int x, y, width, height;
    unsigned int flags;
    void *backwin;
    mume_vector_t *children;
    mume_user_data_t *user_data;
};

struct _window_class {
    const char _[MUME_SIZEOF_CLASS];
    void (*handle_event)(void *self, mume_event_t *event);
    void (*handle_key_down)(
        void *self, int x, int y, int state, int keysym);
    void (*handle_key_up)(
        void *self, int x, int y, int state, int keysym);
    void (*handle_button_down)(
        void *self, int x, int y, int state, int button);
    void (*handle_button_up)(
        void *self, int x, int y, int state, int button);
    void (*handle_button_dblclk)(
        void *self, int x, int y, int state, int button);
    void (*handle_button_tplclk)(
        void *self, int x, int y, int state, int button);
    void (*handle_mouse_motion)(
        void *self, int x, int y, int state);
    void (*handle_mouse_enter)(
        void *self, int x, int y, int state, int mode, int detail);
    void (*handle_mouse_leave)(
        void *self, int x, int y, int state, int mode, int detail);
    void (*handle_focus_in)(void *self, int mode, int detail);
    void (*handle_focus_out)(void *self, int mode, int detail);
    void (*handle_expose)(
        void *self, int x, int y, int width, int height, int count);
    void (*handle_create)(
        void *self, void *window, int x, int y, int width, int height);
    void (*handle_destroy)(void *self, void *window);
    void (*handle_map)(void *self, void *window);
    void (*handle_unmap)(void *self, void *window);
    void (*handle_reparent)(
        void *self, void *window, void *parent, int x, int y);
    void (*handle_move)(
        void *self, void *window, int x, int y, int old_x, int old_y);
    void (*handle_resize)(
        void *self, void *window, int w, int h, int old_w, int old_h);
    void (*handle_sizehint)(
        void *self, int *pref_w, int *pref_h,
        int *min_w, int *min_h, int *max_w, int *max_h);
    void (*handle_command)(void *self, void *window, int command);
    void (*handle_notify)(
        void *self, void *window, int code, void *data);
    void (*handle_scroll)(
        void *self, void *window, int hitcode, int position);
    void (*handle_close)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _window) == MUME_SIZEOF_WINDOW);
MUME_STATIC_ASSERT(sizeof(struct _window_class) ==
                   MUME_SIZEOF_WINDOW_CLASS);

static int _window_test_ancestors_flag(const void *_self, int flag)
{
    const struct _window *self = _self;
    const struct _window *parent = self->parent;
    while (parent) {
        if (!mume_test_flag(parent->flags, flag))
            return 0;
        parent = parent->parent;
    }
    return 1;
}

static int _window_index(const struct _window *self, int skip, int reverse)
{
    const struct _window *parent = self->parent;
    const struct _window **children;
    int cc = (int)mume_vector_size(parent->children);
    children = (const struct _window**)mume_vector_front(parent->children);
    if (reverse) {
        cc -= skip;
        while (cc-- > 0) {
            if (children[cc] == self)
                return cc;
        }
    }
    else {
        for (; skip < cc; ++skip) {
            if (children[skip] == self)
                return skip;
        }
    }
    /* Should never reach here. */
    assert(0);
    return cc;
}

static void _setup_parent(
    struct _window *self, struct _window *parent)
{
    assert(NULL == self->parent && parent);
    if (NULL == parent->children) {
        parent->children = mume_vector_new(
            sizeof(struct _window*), NULL, NULL);
    }
    *(struct _window**)mume_vector_push_back(parent->children) = self;
    self->parent = parent;
    /****
    if (mume_get_input_focus() == self) {
        void *fswin;
        fswin = _mume_lookup_focus_scope(win);
        _mume_set_logical_focus(fswin, win);
    }
    ****/
}

static void _teardown_parent(struct _window *self)
{
    /****void *fswin;****/
    assert(self->parent);
    /****
    fswin = _mume_lookup_focus_scope(win);
    if (_mume_get_logical_focus(fswin) == win)
        _mume_set_logical_focus(fswin, NULL);
    ****/
    mume_vector_erase(
        self->parent->children, _window_index(self, 0, 1), 1);
    self->parent = NULL;
}

static void _foreach_children(
    const void *_win, void (*func)(void*, int), int value, int reverse)
{
    const struct _window *win = _win;
    if (win->children) {
        void **children;
        size_t cc = mume_vector_size(win->children);
        children = (void**)mume_vector_front(win->children);
        if (reverse) {
            while (cc--)
                func(children[cc], value);
        }
        else {
            size_t i;
            for (i = 0; i < cc; ++i)
                func(children[i], value);
        }
    }
}

static void _restack_window(struct _window *win, int from, int to)
{
    void **children;

    children = mume_vector_front(win->parent->children);
    assert(win == children[from]);

    if (from > to) {
        /* lower window */
        while (from-- > to) {
            children[from + 1] = children[from];
        }
    }
    else {
        /* raise window */
        while (from++ < to) {
            children[from - 1] = children[from];
        }
    }

    children[to] = win;
}

static void _window_handle_event(void *self, mume_event_t *event)
{
    assert(self == event->any.window);

    switch (event->type) {
    case MUME_EVENT_KEYDOWN:
        _mume_window_handle_key_down(
            NULL, self, event->key.x, event->key.y,
            event->key.state, event->key.keysym);
        break;

    case MUME_EVENT_KEYUP:
        _mume_window_handle_key_up(
            NULL, self, event->key.x, event->key.y,
            event->key.state, event->key.keysym);
        break;

    case MUME_EVENT_BUTTONDOWN:
        _mume_window_handle_button_down(
            NULL, self, event->button.x, event->button.y,
            event->button.state, event->button.button);
        break;

    case MUME_EVENT_BUTTONUP:
        _mume_window_handle_button_up(
            NULL, self, event->button.x, event->button.y,
            event->button.state, event->button.button);
        break;

    case MUME_EVENT_BUTTONDBLCLK:
        _mume_window_handle_button_dblclk(
            NULL, self, event->button.x, event->button.y,
            event->button.state, event->button.button);
        break;

    case MUME_EVENT_BUTTONTPLCLK:
        _mume_window_handle_button_tplclk(
            NULL, self, event->button.x, event->button.y,
            event->button.state, event->button.button);
        break;

    case MUME_EVENT_MOUSEMOTION:
        _mume_window_handle_mouse_motion(
            NULL, self, event->motion.x, event->motion.y,
            event->motion.state);
        break;

    case MUME_EVENT_MOUSEENTER:
        _mume_window_handle_mouse_enter(
            NULL, self, event->crossing.x, event->crossing.y,
            event->crossing.state, event->crossing.mode,
            event->crossing.detail);
        break;

    case MUME_EVENT_MOUSELEAVE:
        _mume_window_handle_mouse_leave(
            NULL, self, event->crossing.x, event->crossing.y,
            event->crossing.state, event->crossing.mode,
            event->crossing.detail);
        break;

    case MUME_EVENT_FOCUSIN:
        _mume_window_handle_focus_in(
            NULL, self, event->focus.mode, event->focus.detail);
        break;

    case MUME_EVENT_FOCUSOUT:
        _mume_window_handle_focus_out(
            NULL, self, event->focus.mode, event->focus.detail);
        break;

    case MUME_EVENT_EXPOSE:
        _mume_window_handle_expose(
            NULL, self, event->expose.x, event->expose.y,
            event->expose.width, event->expose.height,
            event->expose.count);
        break;

    case MUME_EVENT_CREATE:
        _mume_window_handle_create(
            NULL, self, event->create.window,
            event->create.x, event->create.y,
            event->create.width, event->create.height);
        break;

    case MUME_EVENT_DESTROY:
        _mume_window_handle_destroy(
            NULL, self, event->create.window);
        break;

    case MUME_EVENT_MAP:
        _mume_window_handle_map(
            NULL, self, event->map.window);
        break;

    case MUME_EVENT_UNMAP:
        _mume_window_handle_unmap(
            NULL, self, event->unmap.window);
        break;

    case MUME_EVENT_REPARENT:
        _mume_window_handle_reparent(
            NULL, self, event->reparent.window,
            event->reparent.parent,
            event->reparent.x, event->reparent.y);
        break;

    case MUME_EVENT_MOVE:
        _mume_window_handle_move(
            NULL, self, event->move.window,
            event->move.x, event->move.y,
            event->move.old_x, event->move.old_y);
        break;

    case MUME_EVENT_RESIZE:
        _mume_window_handle_resize(
            NULL, self, event->resize.window,
            event->resize.width, event->resize.height,
            event->resize.old_width, event->resize.old_height);
        break;

    case MUME_EVENT_SIZEHINT:
        _mume_window_handle_sizehint(
            NULL, self,
            &event->sizehint.pref_width,
            &event->sizehint.pref_height,
            &event->sizehint.min_width,
            &event->sizehint.min_height,
            &event->sizehint.max_width,
            &event->sizehint.max_height);
        break;

    case MUME_EVENT_COMMAND:
        _mume_window_handle_command(
            NULL, self, event->command.window,
            event->command.command);
        break;

    case MUME_EVENT_NOTIFY:
        _mume_window_handle_notify(
            NULL, self, event->notify.window,
            event->notify.code, event->notify.data);
        break;

    case MUME_EVENT_SCROLL:
        _mume_window_handle_scroll(
            NULL, self, event->scroll.window,
            event->scroll.hitcode, event->scroll.position);
        break;

    case MUME_EVENT_CLOSE:
        _mume_window_handle_close(NULL, self);
        break;

    case MUME_EVENT_QUIT:
        assert(0);
        break;

    default:
        mume_warning(("Unknown event: %d\n", event->type));
    }
}

static void _window_handle_key_down(
    void *self, int x, int y, int state, int keysym)
{
    if (state & MUME_MOD_CONTROL) {
        int command = MUME_COMMAND_NONE;

        switch (keysym) {
        case MUME_KEY_C:
            if (MUME_COMMAND_NONE == command)
                command = MUME_COMMAND_COPY;
            /* Go through. */
        case MUME_KEY_X:
            if (MUME_COMMAND_NONE == command)
                command = MUME_COMMAND_CUT;
            /* Go through. */
        case MUME_KEY_V:
            if (MUME_COMMAND_NONE == command)
                command = MUME_COMMAND_PASTE;

            mume_post_event(
                mume_make_command_event(self, self, command));
            break;
        }
    }
}

static void _window_handle_key_up(
    void *self, int x, int y, int state, int keysym)
{
}

static void _window_handle_button_down(
    void *self, int x, int y, int state, int button)
{
}

static void _window_handle_button_up(
    void *self, int x, int y, int state, int button)
{
}

static void _window_handle_button_dblclk(
    void *self, int x, int y, int state, int button)
{
}

static void _window_handle_button_tplclk(
    void *self, int x, int y, int state, int button)
{
}

static void _window_handle_mouse_motion(
    void *self, int x, int y, int state)
{
}

static void _window_handle_mouse_enter(
    void *self, int x, int y, int state, int mode, int detail)
{
}

static void _window_handle_mouse_leave(
    void *self, int x, int y, int state, int mode, int detail)
{
}

static void _window_handle_focus_in(void *self, int mode, int detail)
{
}

static void _window_handle_focus_out(void *self, int mode, int detail)
{
}

static void _window_handle_expose(
    void *self, int x, int y, int width, int height, int count)
{
    cairo_t *cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (cr) {
        cairo_rectangle(cr, x, y, width, height);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_fill(cr);
        mume_window_end_paint(self, cr);
    }
}

static void _window_handle_create(
    void *self, void *window, int x, int y, int width, int height)
{
}

static void _window_handle_destroy(void *self, void *window)
{
}

static void _window_handle_map(void *self, void *window)
{
}

static void _window_handle_unmap(void *self, void *window)
{
}

static void _window_handle_reparent(
    void *self, void *window, void *parent, int x, int y)
{
}

static void _window_handle_move(
    void *self, void *window, int x, int y, int old_x, int old_y)
{
}

static void _window_handle_resize(
    void *self, void *window, int w, int h, int old_w, int old_h)
{
}

static void _window_handle_sizehint(
    void *self, int *pref_w, int *pref_h,
    int *min_w, int *min_h, int *max_w, int *max_h)
{
}

static void _window_handle_command(
    struct _window *self, void *window, int command)
{
    if (self->parent) {
        mume_event_t event;

        event = mume_make_command_event(
            self->parent, window, command);

        mume_send_event(&event);
    }
}

static void _window_handle_notify(
    struct _window *self, void *window, int code, void *data)
{
    if (self->parent) {
        mume_event_t event;

        event = mume_make_notify_event(
            self->parent, window, code, data);

        mume_send_event(&event);
    }
}

static void _window_handle_scroll(
    void *self, void *window, int hitcode, int position)
{
}

static void _window_handle_close(void *self)
{
}

static void* _window_ctor(
    struct _window *self, int mode, va_list *app)
{
    struct _window *parent;

    if (!_mume_ctor(_window_super_class(), self, mode, app))
        return NULL;

    parent = va_arg(*app, void*);
    self->parent = NULL;
    self->x = va_arg(*app, int);
    self->y = va_arg(*app, int);
    self->width = va_arg(*app, int);
    self->height = va_arg(*app, int);
    self->flags = 0;
    self->backwin = NULL;
    self->children = NULL;
    self->user_data = NULL;

    if (parent) {
        mume_event_t event;

        assert(mume_is_of(parent, mume_window_class()));

        _setup_parent(self, parent);

        /* Send create event to parent. */
        event = mume_make_create_event(
            self->parent, self, self->x, self->y,
            self->width, self->height);

        mume_disp_event(&event);
    }

    /****
    _update_focus_scope(self);
    ****/

    return self;
}

static void* _window_dtor(struct _window *self)
{
    mume_event_t event;

    _mume_window_unmap(self, 0);
    if (self->children) {
        while (mume_vector_size(self->children))
            mume_delete(*(void**)mume_vector_back(self->children));

        mume_vector_delete(self->children);
    }

    event = mume_make_destroy_event(self, self);
    mume_disp_event(&event);

    if (self->parent) {
        event.destroy.event = self->parent;
        mume_disp_event(&event);

        _teardown_parent(self);
    }

    if (self->backwin)
        mume_window_set_backwin(self, NULL);

    mume_user_data_delete(self->user_data);
    _mume_window_clear_res(self);

    return _mume_dtor(_window_super_class(), self);
}

static void* _window_class_ctor(
    struct _window_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_window_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_window_handle_event)
            *(voidf**)&self->handle_event = method;
        else if (selector == (voidf*)_mume_window_handle_key_down)
            *(voidf**)&self->handle_key_down = method;
        else if (selector == (voidf*)_mume_window_handle_key_up)
            *(voidf**)&self->handle_key_up = method;
        else if (selector == (voidf*)_mume_window_handle_button_down)
            *(voidf**)&self->handle_button_down = method;
        else if (selector == (voidf*)_mume_window_handle_button_up)
            *(voidf**)&self->handle_button_up = method;
        else if (selector == (voidf*)_mume_window_handle_button_dblclk)
            *(voidf**)&self->handle_button_dblclk = method;
        else if (selector == (voidf*)_mume_window_handle_button_tplclk)
            *(voidf**)&self->handle_button_tplclk = method;
        else if (selector == (voidf*)_mume_window_handle_mouse_motion)
            *(voidf**)&self->handle_mouse_motion = method;
        else if (selector == (voidf*)_mume_window_handle_mouse_enter)
            *(voidf**)&self->handle_mouse_enter = method;
        else if (selector == (voidf*)_mume_window_handle_mouse_leave)
            *(voidf**)&self->handle_mouse_leave = method;
        else if (selector == (voidf*)_mume_window_handle_focus_in)
            *(voidf**)&self->handle_focus_in = method;
        else if (selector == (voidf*)_mume_window_handle_focus_out)
            *(voidf**)&self->handle_focus_out = method;
        else if (selector == (voidf*)_mume_window_handle_expose)
            *(voidf**)&self->handle_expose = method;
        else if (selector == (voidf*)_mume_window_handle_create)
            *(voidf**)&self->handle_create = method;
        else if (selector == (voidf*)_mume_window_handle_destroy)
            *(voidf**)&self->handle_destroy = method;
        else if (selector == (voidf*)_mume_window_handle_map)
            *(voidf**)&self->handle_map = method;
        else if (selector == (voidf*)_mume_window_handle_unmap)
            *(voidf**)&self->handle_unmap = method;
        else if (selector == (voidf*)_mume_window_handle_reparent)
            *(voidf**)&self->handle_reparent = method;
        else if (selector == (voidf*)_mume_window_handle_move)
            *(voidf**)&self->handle_move = method;
        else if (selector == (voidf*)_mume_window_handle_resize)
            *(voidf**)&self->handle_resize = method;
        else if (selector == (voidf*)_mume_window_handle_sizehint)
            *(voidf**)&self->handle_sizehint = method;
        else if (selector == (voidf*)_mume_window_handle_command)
            *(voidf**)&self->handle_command = method;
        else if (selector == (voidf*)_mume_window_handle_notify)
            *(voidf**)&self->handle_notify = method;
        else if (selector == (voidf*)_mume_window_handle_scroll)
            *(voidf**)&self->handle_scroll = method;
        else if (selector == (voidf*)_mume_window_handle_close)
            *(voidf**)&self->handle_close = method;
    }

    return self;
}

const void* mume_window_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_window_meta_class(),
        "window",
        _window_super_class(),
        sizeof(struct _window),
        MUME_PROP_END,
        _mume_ctor, _window_ctor,
        _mume_dtor, _window_dtor,
        _mume_window_handle_event,
        _window_handle_event,
        _mume_window_handle_key_down,
        _window_handle_key_down,
        _mume_window_handle_key_up,
        _window_handle_key_up,
        _mume_window_handle_button_down,
        _window_handle_button_down,
        _mume_window_handle_button_up,
        _window_handle_button_up,
        _mume_window_handle_button_dblclk,
        _window_handle_button_dblclk,
        _mume_window_handle_button_tplclk,
        _window_handle_button_tplclk,
        _mume_window_handle_mouse_motion,
        _window_handle_mouse_motion,
        _mume_window_handle_mouse_enter,
        _window_handle_mouse_enter,
        _mume_window_handle_mouse_leave,
        _window_handle_mouse_leave,
        _mume_window_handle_focus_in,
        _window_handle_focus_in,
        _mume_window_handle_focus_out,
        _window_handle_focus_out,
        _mume_window_handle_expose,
        _window_handle_expose,
        _mume_window_handle_create,
        _window_handle_create,
        _mume_window_handle_destroy,
        _window_handle_destroy,
        _mume_window_handle_map,
        _window_handle_map,
        _mume_window_handle_unmap,
        _window_handle_unmap,
        _mume_window_handle_reparent,
        _window_handle_reparent,
        _mume_window_handle_move,
        _window_handle_move,
        _mume_window_handle_resize,
        _window_handle_resize,
        _mume_window_handle_sizehint,
        _window_handle_sizehint,
        _mume_window_handle_command,
        _window_handle_command,
        _mume_window_handle_notify,
        _window_handle_notify,
        _mume_window_handle_scroll,
        _window_handle_scroll,
        _mume_window_handle_close,
        _window_handle_close,
        MUME_FUNC_END);
}

const void* mume_window_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "window class",
        _window_super_meta_class(),
        sizeof(struct _window_class),
        MUME_PROP_END,
        _mume_ctor, _window_class_ctor,
        MUME_FUNC_END);
}

void* mume_window_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_window_class(),
                    parent, x, y, width, height);
}

void _mume_window_map(void *_self, int sync)
{
    struct _window *self = _self;
    mume_event_t event;
    cairo_region_t *rgn = NULL;
    void *umgr;

    assert(mume_is_of(_self, mume_window_class()));

    if (mume_test_flag(self->flags, MUME_WF_MAPPED))
        return;

    if (self->backwin && sync)
        mume_backwin_map(self->backwin);

    umgr = mume_urgnmgr();

    if (mume_is_ancestors_mapped(self)) {
        rgn = mume_window_region_create(self);
        if (self->parent) {
            cairo_region_translate(rgn, self->x, self->y);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, NULL,
                rgn, MUME_URGN_SUBTRACT, 1);
            cairo_region_translate(rgn, -self->x, -self->y);
        }
    }

    mume_add_flag(self->flags, MUME_WF_MAPPED);

    /* Map event. */
    event = mume_make_map_event(self, self);
    mume_disp_event(&event);

    if (rgn) {
        mume_urgnmgr_set_urgn(
            umgr, self, NULL, rgn, MUME_URGN_REPLACE, 1);
        cairo_region_destroy(rgn);
    }

    /* Map child event. */
    if (self->parent) {
        event.map.event = self->parent;
        mume_disp_event(&event);
    }

    /* Mouse event (if needed) */
    /****
         if (!_gstate->pointer_grab &&
         _mume_window_is_ancestor(win, _gstate->pointer_owner))
         {
         _update_pointer_owner();
         }
    ****/
}

void _mume_window_unmap(void *_self, int sync)
{
    struct _window *self = _self;
    mume_event_t event;
    int mapped;
    void *umgr;

    assert(mume_is_of(_self, mume_window_class()));

    if (!mume_test_flag(self->flags, MUME_WF_MAPPED))
        return;

    if (self->backwin && sync)
        mume_backwin_unmap(self->backwin);

    mapped = mume_is_ancestors_mapped(self);
    umgr = mume_urgnmgr();

    mume_remove_flag(self->flags, MUME_WF_MAPPED);

    /* Mouse event (if needed) */
    /****
         if (win == _gstate->pointer_owner ||
         _mume_window_is_ancestor(_gstate->pointer_owner, win))
         {
         _gstate->pointer_grab = 0;
         _update_pointer_owner();
         }
    ****/

    /* Unmap event. */
    event = mume_make_unmap_event(self, self);
    mume_disp_event(&event);

    if (mapped) {
        mume_urgnmgr_set_urgn(
            umgr, self, NULL, NULL, MUME_URGN_REPLACE, 1);
    }

    /* Child unmap event. */
    if (self->parent) {
        event.unmap.event = self->parent;
        mume_disp_event(&event);

        if (mapped) {
            cairo_region_t *rgn;
            rgn = mume_window_region_create(self);
            cairo_region_translate(rgn, self->x, self->y);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, NULL,
                rgn, MUME_URGN_UNION, 1);
            cairo_region_destroy(rgn);
        }
    }

    /* Focus event (if needed) */
    /****
         if (win == _gstate->input_focus ||
         _mume_window_is_ancestor(_gstate->input_focus, win))
         {
         if (win->pnt)
         mume_set_input_focus(win->pnt);
         }
    ****/
}

int mume_window_is_mapped(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return mume_test_flag(self->flags, MUME_WF_MAPPED);
}

int mume_is_ancestors_mapped(const void *_self)
{
    assert(mume_is_of(_self, mume_window_class()));
    return _window_test_ancestors_flag(_self, MUME_WF_MAPPED);
}

void mume_map_children(void *self)
{
    assert(mume_is_of(self, mume_window_class()));

    /* TODO: update the dirty region only once after
       all the children has been mapped */
    _foreach_children(self, _mume_window_map, 1, 0);
}

void mume_unmap_children(void *self)
{
    assert(mume_is_of(self, mume_window_class()));

    /* TODO: update the dirty region only once after
       all the children has been unmapped */
    _foreach_children(self, _mume_window_unmap, 1, 1);
}

void mume_window_enable(void *_self, int able)
{
    struct _window *self = _self;
    int changed = 0;

    assert(mume_is_of(_self, mume_window_class()));

    if (able) {
        if (mume_test_flag(self->flags, MUME_WF_DISABLED)) {
            changed = 1;
            mume_remove_flag(self->flags, MUME_WF_DISABLED);
        }
    }
    else if (!mume_test_flag(self->flags, MUME_WF_DISABLED)) {
        changed = 1;
        mume_add_flag(self->flags, MUME_WF_DISABLED);
    }

    if (changed && mume_window_is_mapped(self) &&
        mume_is_ancestors_mapped(self))
    {
        cairo_region_t *rgn;
        rgn = mume_window_region_create(self);
        mume_window_region_clip_children(self, rgn);
        mume_urgnmgr_set_urgn(
            mume_urgnmgr(), self, NULL,
            rgn, MUME_URGN_REPLACE, 0);
        cairo_region_destroy(rgn);
    }
}

int mume_window_is_enabled(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return !mume_test_flag(self->flags, MUME_WF_DISABLED);
}

void mume_window_reparent(
    void *_self, void *parent, int x, int y)
{
    mume_event_t event;
    struct _window *self = _self;
    struct _window *tmp = parent;
    int mapped = mume_window_is_mapped(self);

    assert(mume_is_of(_self, mume_window_class()));
    assert(mume_is_of(parent, mume_window_class()));
    assert(self != mume_root_window() && parent);

    while (tmp) {
        if (tmp == self) {
            mume_error(("Invalid parent window.\n"));
            return;
        }

        tmp = tmp->parent;
    }

    event = mume_make_reparent_event(NULL, self, parent, x, y);

    /* Remove from old parent. */
    if (self->parent) {
        if (mapped)
            _mume_window_unmap(self, 0);

        event.reparent.event = self->parent;
        mume_disp_event(&event);

        _teardown_parent(self);
    }

    event.reparent.event = self;
    mume_disp_event(&event);

    /* Add to new parent. */
    _setup_parent(self, parent);

    self->x = x;
    self->y = y;

    if (mapped)
        _mume_window_map(self, 0);

    event.reparent.event = parent;
    mume_disp_event(&event);
}

int mume_window_is_ancestor(const void *_self, const void *_other)
{
    const struct _window *self = _self;
    const struct _window *other = _other;

    assert(mume_is_of(_self, mume_window_class()));
    assert(mume_is_of(_other, mume_window_class()));

    while ((self = self->parent)) {
        if (self == other)
            return 1;
    }

    return 0;
}

void* mume_window_parent(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return self->parent;
}

void* mume_window_sibling(const void *_self, int x)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->parent) {
        int i = _window_index(self, 0, 0);
        return mume_window_child_at(self->parent, i + x);
    }

    return NULL;
}

void* mume_window_last_leaf(const void *_self)
{
    const struct _window *win = _self;
    const struct _window *lc;

    assert(mume_is_of(_self, mume_window_class()));

    while ((lc = mume_window_child_at(win, -1)))
        win = lc;

    return (void*)win;
}

void* mume_window_next_node(const void *_self, int down)
{
    const struct _window *win = _self;
    struct _window *node;

    assert(mume_is_of(_self, mume_window_class()));

    if (down) {
        node = mume_window_first_child(win);
        if (node)
            return node;
    }

    while (win->parent) {
        node = mume_window_next_sibling(win);
        if (node)
            return node;

        win = win->parent;
    }

    return NULL;
}

void* mume_window_prev_node(const void *_self, int up)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->parent) {
        int i = _window_index(self, 0, 0);
        if (i > 0) {
            return mume_window_last_leaf(
                mume_window_child_at(self->parent, i - 1));
        }
        else if (up) {
            return self->parent;
        }
    }

    return NULL;
}

void mume_window_focusable(void *_self, int able)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (able) {
        mume_add_flag(self->flags, MUME_WF_FOCUSABLE);
    }
    else {
        mume_remove_flag(self->flags, MUME_WF_FOCUSABLE);
    }
}

int mume_window_is_focusable(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return mume_test_flag(self->flags, MUME_WF_FOCUSABLE);
}

void* mume_window_next_focus(const void *anc, const void *cur)
{
    const void *it, *end;

    assert(mume_is_of(anc, mume_window_class()));
    assert(!cur || mume_is_of(cur, mume_window_class()));

    if (NULL == cur) {
        cur = mume_window_child_at(anc, 0);
        if (NULL == cur)
            return NULL;

        if (mume_window_is_mapped(cur) &&
            mume_window_is_focusable(cur))
        {
            return (void*)cur;
        }
    }

    assert(mume_window_is_ancestor(cur, anc));
    it = cur;
    end = mume_window_next_node(anc, 0);
    do {
        it = mume_window_next_node(it, 1);
        if (it == end)
            it = mume_window_first_child(anc);

        if (mume_window_is_mapped(it) &&
            mume_window_is_focusable(it))
        {
            return (void*)it;
        }
    }
    while (it != cur);

    return (void*)cur;
}

void* mume_window_prev_focus(const void *anc, const void *cur)
{
    const void *it;

    assert(mume_is_of(anc, mume_window_class()));
    assert(!cur || mume_is_of(cur, mume_window_class()));

    if (NULL == cur) {
        cur = mume_window_last_child(anc);
        if (NULL == cur)
            return NULL;

        cur = mume_window_last_leaf(cur);
        if (mume_window_is_mapped(cur) &&
            mume_window_is_focusable(cur))
        {
            return (void*)cur;
        }
    }

    assert(mume_window_is_ancestor(cur, anc));
    it = cur;
    do {
        it = mume_window_prev_node(it, 1);
        if (it == anc) {
            it = mume_window_last_child(anc);
            it = mume_window_last_leaf(it);
        }

        if (mume_window_is_mapped(it) &&
            mume_window_is_focusable(it))
        {
            return (void*)it;
        }
    }
    while (it != cur);

    return (void*)cur;
}

void mume_window_set_backwin(void *_self, void *bwin)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));
    assert(!bwin || mume_is_of(bwin, mume_backwin_class()));
    assert(!bwin || mume_backwin_get_attached(bwin) == NULL);

    if (self->backwin) {
        mume_backwin_set_attached(self->backwin, NULL);
        mume_refobj_release(self->backwin);
        self->backwin = NULL;
    }

    if (bwin) {
        mume_backwin_set_attached(bwin, self);
        mume_refobj_addref(bwin);
        self->backwin = bwin;

        /****
        _update_focus_scope(win);
        ****/
    }
}

void* mume_window_get_backwin(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return self->backwin;
}

void* mume_window_seek_backwin(const void *_self, int *x, int *y)
{
    const struct _window *win = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (x)
        *x = 0;

    if (y)
        *y = 0;

    while (NULL == win->backwin) {
        if (x)
            *x += win->x;
        if (y)
            *y += win->y;

        win = win->parent;
    }

    return win->backwin;
}

void mume_window_sync_backwin(void *_self, int update)
{
    struct _window *self = _self;
    int x, y, width, height;

    assert(mume_is_of(_self, mume_window_class()));

    if (NULL == self->backwin)
        return;

    if (update) {
        if (mume_backwin_is_mapped(self->backwin)) {
            _mume_window_map(self, 0);
        }
        else {
            _mume_window_unmap(self, 0);
        }

        mume_backwin_get_geometry(
            self->backwin, &x, &y, &width, &height);

        _mume_window_set_geometry(
            self, x, y, width, height, 0);
    }
    else {
        if (mume_window_is_mapped(self)) {
            mume_backwin_map(self->backwin);
        }
        else {
            mume_backwin_unmap(self->backwin);
        }

        mume_window_get_geometry(
            self, &x, &y, &width, &height);

        mume_backwin_set_geometry(
            self->backwin, x, y, width, height);
    }
}

void** mume_query_children(
    const void *_self, unsigned int *num, int alter)
{
    const struct _window *self = _self;
    void **children;

    assert(mume_is_of(_self, mume_window_class()));

    if (NULL == self->children) {
        if (num)
            *num = 0;
        return NULL;
    }

    if (num)
        *num = mume_vector_size(self->children);

    if (alter) {
        children = malloc_abort(sizeof(struct _window*) * (*num));
        memcpy(children, mume_vector_front(self->children),
               sizeof(struct _window*) * (*num));
    }
    else {
        children = (void**)mume_vector_front(self->children);
    }

    return children;
}

void mume_free_children(const void *_self, void **children)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (NULL == self->children ||
        mume_vector_front(self->children) != children)
    {
        free(children);
    }
}

void mume_window_set_id(void *self, int id)
{
    assert(mume_is_of(self, mume_window_class()));
    mume_window_set_user_data(
        self, &_window_id_key, (void*)(intptr_t)id, NULL);
}

int mume_window_get_id(const void *self)
{
    assert(mume_is_of(self, mume_window_class()));
    return (intptr_t)mume_window_get_user_data(self, &_window_id_key);
}

void* mume_find_child(const void *_self, int id)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->children) {
        void **children;
        size_t cc;
        children = mume_vector_front(self->children);
        cc = mume_vector_size(self->children);
        while (cc-- > 0) {
            if (mume_window_get_id(children[cc]) == id)
                return children[cc];
        }
    }

    return NULL;
}

void mume_window_set_text(void *self, const char *text)
{
    assert(mume_is_of(self, mume_window_class()));

    if (text) {
        mume_window_set_user_data(
            self, &_window_text_key, strdup_abort(text), free);
    }
    else {
        mume_window_set_user_data(
            self, &_window_text_key, NULL, NULL);
    }
}

const char* mume_window_get_text(const void *self)
{
    assert(mume_is_of(self, mume_window_class()));
    return mume_window_get_user_data(self, &_window_text_key);
}

void* mume_window_child_at(const void *_self, int i)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->children) {
        int cc = (int)mume_vector_size(self->children);
        if (i < 0) {
            /* Accept negative index. */
            i += cc;
        }
        if (i >= 0 && i < cc)
            return *(void**)mume_vector_at(self->children, i);
    }

    return NULL;
}

void* mume_child_from_point(const void *_self, int x, int y, int mapped)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (x < 0 || y < 0 || x >= self->width || y >= self->height)
        return NULL;

    if (self->children) {
        struct _window **children;
        unsigned int cc;
        children = (struct _window**)mume_vector_front(self->children);
        cc = mume_vector_size(self->children);
        /* In stack order, from top to bottom. */
        while (cc-- > 0) {
            if (x >= children[cc]->x && y >= children[cc]->y &&
                x < children[cc]->x + children[cc]->width &&
                y < children[cc]->y + children[cc]->height)
            {
                if (!mapped || mume_window_is_mapped(children[cc]))
                    return children[cc];
            }
        }
    }

    return (void*)self;
}

void* mume_window_from_point(const void *_self, int x, int y)
{
    const struct _window *win = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (x >= 0 && y >= 0 && x < win->width && y < win->height &&
        mume_window_is_mapped(win) && mume_is_ancestors_mapped(win))
    {
        const struct _window *c;
        while ((c = mume_child_from_point(win, x, y, 1)) &&
               c != win && mume_window_is_mapped(c))
        {
            x -= c->x;
            y -= c->y;
            win = c;
        }

        return (void*)win;
    }

    return NULL;
}

void mume_window_size_hint(
    void *_self, int *pref_width, int *pref_height,
    int *min_width, int *min_height,
    int *max_width, int *max_height)
{
    struct _window *self = _self;
    mume_event_t event;

    assert(mume_is_of(_self, mume_window_class()));

    event = mume_make_sizehint_event(
        self, self->width, self->height, 0, 0,
        MUME_WINDOW_MAX_WIDTH, MUME_WINDOW_MAX_HEIGHT);

    mume_disp_event(&event);

#define _VALUE_FROM_EVENT(_value) \
    if (_value) *(_value) = event.sizehint._value

    _VALUE_FROM_EVENT(pref_width);
    _VALUE_FROM_EVENT(pref_height);
    _VALUE_FROM_EVENT(min_width);
    _VALUE_FROM_EVENT(min_height);
    _VALUE_FROM_EVENT(max_width);
    _VALUE_FROM_EVENT(max_height);

#undef _VALUE_FROM_EVENT
}

void _mume_window_set_geometry(
    void *_self, int x, int y, int width, int height, int sync)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->backwin && sync) {
        mume_backwin_set_geometry(
            self->backwin, x, y, width, height);

        return;
    }

    if (self->x != x || self->y != y ||
        self->width != width || self->height != height)
    {
        mume_event_t event;
        int oldx = self->x;
        int oldy = self->y;
        int oldw = self->width;
        int oldh = self->height;
        int mapped = mume_window_is_mapped(self) &&
                     mume_is_ancestors_mapped(self);
        int stack = 0;
        mume_rect_t rect;
        cairo_region_t *rgn = NULL;
        cairo_region_t *oldrgn = NULL;
        cairo_region_t *newrgn = NULL;
        void *umgr = mume_urgnmgr();
        if (mapped && self->parent) {
            stack = _window_index(self, 0, 0);
            oldrgn = mume_window_region_create(self->parent);
            newrgn = cairo_region_copy(oldrgn);
            /* Expose the windows under old region. */
            rect = mume_rect_make(oldx, oldy, oldw, oldh);
            cairo_region_intersect_rectangle(oldrgn, &rect);
            cairo_region_translate(oldrgn, -oldx, -oldy);
            mume_window_region_clip_siblings(self, oldrgn, stack);
            cairo_region_translate(oldrgn, oldx, oldy);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, self,
                oldrgn, MUME_URGN_UNION, 1);
            cairo_region_translate(oldrgn, -oldx, -oldy);
        }

        self->x = x;
        self->y = y;
        self->width = width;
        self->height = height;
        if (mapped && self->parent) {
            /* Obscure the windows under new region. */
            rect = mume_rect_make(x, y, width, height);
            cairo_region_intersect_rectangle(newrgn, &rect);
            cairo_region_translate(newrgn, -x, -y);
            mume_window_region_clip_siblings(self, newrgn, stack);
            cairo_region_translate(newrgn, x, y);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, self,
                newrgn, MUME_URGN_SUBTRACT, 1);
            cairo_region_translate(newrgn, -x, -y);
        }

        if (oldx != x || oldy != y) {
            event.move.type = MUME_EVENT_MOVE;
            event.move.event = self;
            event.move.window = self;
            event.move.x = x;
            event.move.y = y;
            event.move.old_x = oldx;
            event.move.old_y = oldy;
            mume_disp_event(&event);
            if (self->parent) {
                event.move.event = self->parent;
                mume_disp_event(&event);
            }
        }

        if (oldw != width || oldh != height) {
            event.resize.type = MUME_EVENT_RESIZE;
            event.resize.event = self;
            event.resize.window = self;
            event.resize.width = width;
            event.resize.height = height;
            event.resize.old_width = oldw;
            event.resize.old_height = oldh;
            mume_disp_event(&event);
            if (self->parent) {
                event.resize.event = self->parent;
                mume_disp_event(&event);
            }

            if (mapped) {
                /* Resized, redraw the entire window. */
                if (oldrgn && newrgn) {
                    cairo_region_destroy(oldrgn);
                    rgn = newrgn;
                    oldrgn = NULL;
                    newrgn = NULL;
                }
                else {
                    rgn = mume_window_region_create(self);
                }
                mume_urgnmgr_set_urgn(
                    umgr, self, NULL, rgn, MUME_URGN_REPLACE, 1);
                cairo_region_destroy(rgn);
            }
        }

        if (oldrgn && newrgn) {
            /* Calculate the obscured and exposed region. */
            rgn = cairo_region_copy(oldrgn);
            cairo_region_subtract(oldrgn, newrgn);
            mume_urgnmgr_set_urgn(
                umgr, self, NULL, oldrgn, MUME_URGN_SUBTRACT, 1);
            cairo_region_destroy(oldrgn);
            cairo_region_subtract(newrgn, rgn);
            cairo_region_destroy(rgn);
            mume_urgnmgr_set_urgn(
                umgr, self, NULL, newrgn, MUME_URGN_UNION, 1);
            cairo_region_destroy(newrgn);
        }
    }
}

void mume_window_get_geometry(
    const void *_self, int *px, int *py, int *pw, int *ph)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (px) *px = self->x;
    if (py) *py = self->y;
    if (pw) *pw = self->width;
    if (ph) *ph = self->height;
}

int mume_window_width(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return self->width;
}

int mume_window_height(const void *_self)
{
    const struct _window *self = _self;
    assert(mume_is_of(_self, mume_window_class()));
    return self->height;
}

void mume_window_move_to(void *_self, int x, int y)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    mume_window_set_geometry(
        self, x, y, self->width, self->height);
}

void mume_window_move_by(void *_self, int x, int y)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    mume_window_set_geometry(
        self, self->x + x, self->y + y,
        self->width, self->height);
}

void mume_window_resize_to(void *_self, int width, int height)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    mume_window_set_geometry(
        self, self->x, self->y, width, height);
}

void mume_window_resize_by(void *_self, int width, int height)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    mume_window_set_geometry(
        self, self->x, self->y,
        self->width + width, self->height + height);
}

void mume_window_center(void *_self, const void *_dest)
{
    struct _window *self = _self;
    const struct _window *dest = _dest;
    int x = 0, y = 0;

    assert(mume_is_of(_self, mume_window_class()));
    assert(mume_is_of(_dest, mume_window_class()));

    mume_translate_coords(dest, self->parent, &x, &y);
    x += (dest->width - self->width) / 2;
    y += (dest->height - self->height) / 2;

    mume_window_set_geometry(
        self, x, y, self->width, self->height);
}

void mume_translate_coords(
    const void *_src, const void *_dest, int *x, int *y)
{
    const struct _window *src = _src;
    const struct _window *dest = _dest;

    assert(!_src || mume_is_of(_src, mume_window_class()));
    assert(!_dest || mume_is_of(_dest, mume_window_class()));

    while (src) {
        *x += src->x;
        *y += src->y;
        src = src->parent;
    }

    while (dest) {
        *x -= dest->x;
        *y -= dest->y;
        dest = dest->parent;
    }
}

void mume_window_raise(void *_self)
{
    struct _window *self = _self;
    int oldidx, topidx;

    assert(mume_is_of(_self, mume_window_class()));
    assert(self->parent && self->parent->children);

    oldidx = _window_index(self, 0, 0);
    topidx = (int)mume_vector_size(self->parent->children) - 1;
    if (oldidx != topidx) {
        _restack_window(self, oldidx, topidx);
        if (mume_window_is_mapped(self) &&
            mume_is_ancestors_mapped(self))
        {
            void *umgr = mume_urgnmgr();
            cairo_region_t *oldrgn = NULL;
            cairo_region_t *newrgn = NULL;
            newrgn = mume_window_region_create(self);
            oldrgn = cairo_region_copy(newrgn);
            mume_window_region_clip_siblings(self, oldrgn, oldidx);
            cairo_region_subtract(newrgn, oldrgn);
            cairo_region_destroy(oldrgn);
            mume_urgnmgr_set_urgn(
                umgr, self, NULL, newrgn, MUME_URGN_UNION, 1);
            cairo_region_translate(newrgn, self->x, self->y);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, self, newrgn, MUME_URGN_SUBTRACT, 1);
            cairo_region_destroy(newrgn);
        }
    }

    if (self->backwin)
        mume_backwin_raise(self->backwin);
}

void mume_window_lower(void *_self)
{
    struct _window *self = _self;
    int oldidx;

    assert(mume_is_of(_self, mume_window_class()));
    assert(self->parent && self->parent->children);

    oldidx = _window_index(self, 0, 1);
    if (oldidx != 0) {
        _restack_window(self, oldidx, 0);
        if (mume_window_is_mapped(self) &&
            mume_is_ancestors_mapped(self))
        {
            mume_rect_t rect;
            void *umgr = mume_urgnmgr();
            cairo_region_t *oldrgn = NULL;
            cairo_region_t *newrgn = NULL;
            newrgn = mume_window_region_create(self->parent);
            mume_window_get_geometry(
                self, &rect.x, &rect.y, &rect.width, &rect.height);
            cairo_region_intersect_rectangle(newrgn, &rect);
            cairo_region_translate(newrgn, -self->x, -self->y);
            oldrgn = cairo_region_copy(newrgn);
            mume_window_region_clip_siblings(self, oldrgn, oldidx);
            mume_window_region_clip_siblings(self, newrgn, 0);
            cairo_region_subtract(oldrgn, newrgn);
            cairo_region_destroy(newrgn);
            mume_urgnmgr_set_urgn(
                umgr, self, NULL, oldrgn, MUME_URGN_SUBTRACT, 1);
            cairo_region_translate(oldrgn, self->x, self->y);
            mume_urgnmgr_set_urgn(
                umgr, self->parent, self, oldrgn, MUME_URGN_UNION, 1);
            cairo_region_destroy(oldrgn);
        }
    }

    if (self->backwin)
        mume_backwin_lower(self->backwin);
}

void mume_window_restack(void **windows, unsigned int num)
{
    struct _window **wins = (struct _window**)windows;
    struct _window *parent = wins[0]->parent;
    int i, pos = (int)mume_vector_size(parent->children) - 1;
    int changed = 0;

    for (i = 0; i < num; ++i, --pos) {
        int oldidx;
        assert(wins[i]->parent == parent);
        oldidx = _window_index(wins[i], i, 1);
        if (oldidx != pos) {
            _restack_window(wins[i], oldidx, pos);
            changed = 1;
        }
    }

    if (changed && mume_window_is_mapped(parent) &&
        mume_is_ancestors_mapped(parent))
    {
        cairo_region_t *rgn;
        rgn = mume_window_region_create(parent);
        mume_urgnmgr_set_urgn(
            mume_urgnmgr(), parent, parent,
            rgn, MUME_URGN_REPLACE, 1);
        cairo_region_destroy(rgn);
    }
}

void mume_invalidate_region(
    void *self, const cairo_region_t *rgn)
{
    if (mume_window_is_mapped(self) &&
        mume_is_ancestors_mapped(self))
    {
        cairo_region_t *add;
        add = mume_window_region_create(self);
        if (rgn) {
            cairo_region_intersect(add, rgn);
        }

        mume_urgnmgr_set_urgn(
            mume_urgnmgr(), self, NULL,
            add, MUME_URGN_UNION, 1);
        cairo_region_destroy(add);
    }
}

void mume_validate_region(void *self, const cairo_region_t *rgn)
{
    if (mume_window_is_mapped(self) &&
        mume_is_ancestors_mapped(self))
    {
        if (rgn) {
            mume_urgnmgr_set_urgn(
                mume_urgnmgr(), self, NULL,
                rgn, MUME_URGN_SUBTRACT, 1);
        }
        else {
            mume_urgnmgr_set_urgn(
                mume_urgnmgr(), self, NULL,
                NULL, MUME_URGN_REPLACE, 1);
        }
    }
}

void mume_invalidate_rect(
    void *self, const mume_rect_t *rect)
{
    if (mume_window_is_mapped(self) &&
        mume_is_ancestors_mapped(self))
    {
        cairo_region_t *rgn = NULL;
        if (rect) {
            rgn = cairo_region_create_rectangle(rect);
        }

        mume_invalidate_region(self, rgn);
        cairo_region_destroy(rgn);
    }
}

void mume_validate_rect(
    void *self, const mume_rect_t *rect)
{
    if (mume_window_is_mapped(self) &&
        mume_is_ancestors_mapped(self))
    {
        cairo_region_t *rgn = NULL;
        if (rect) {
            rgn = cairo_region_create_rectangle(rect);
        }

        mume_validate_region(self, rgn);
        cairo_region_destroy(rgn);
    }
}

mume_rect_t mume_current_invalid_rect(void)
{
    mume_rect_t rect = mume_rect_empty;
    const cairo_region_t *rgn;

    rgn = mume_urgnmgr_last_rgn(mume_urgnmgr());
    if (rgn)
        rect = mume_cairo_region_extents(rgn);

    return rect;
}

cairo_region_t* mume_window_region_create(const void *_self)
{
    const struct _window *win = _self;
    mume_rect_t rect;
    cairo_region_t *rgn;
    const struct _window **ss;
    unsigned int sc;
    int i, x = 0, y = 0;

    assert(mume_is_of(_self, mume_window_class()));

    rect.x = 0;
    rect.y = 0;
    rect.width = win->width;
    rect.height = win->height;
    rgn = cairo_region_create_rectangle(&rect);
    while (win->parent) {
        x -= win->x;
        y -= win->y;
        /* Clip ancestors. */
        rect.x = x;
        rect.y = y;
        rect.width = win->parent->width;
        rect.height = win->parent->height;
        cairo_region_intersect_rectangle(rgn, &rect);
        /* Clip siblings. */
        ss = (const struct _window**)mume_query_children(win->parent, &sc, 0);
        if (ss) {
            i = _window_index(win, 0, 0);
            for (++i; i < (int)sc; ++i) {
                if (!mume_window_is_mapped(ss[i]))
                    continue;
                rect.x = x + ss[i]->x;
                rect.y = y + ss[i]->y;
                rect.width = ss[i]->width;
                rect.height = ss[i]->height;
                cairo_region_subtract_rectangle(rgn, &rect);
            }
            mume_free_children(win->parent, (void**)ss);
        }
        win = win->parent;
    }

    return rgn;
}

void mume_window_region_clip_siblings(
    const void *_self, cairo_region_t *rgn, int stack)
{
    const struct _window *self = _self;
    void **ss;
    unsigned int sc;

    assert(mume_is_of(_self, mume_window_class()));
    assert(self->parent);

    ss = mume_query_children(self->parent, &sc, 0);
    if (ss) {
        mume_rect_t rect;
        for (++stack; stack < sc; ++stack) {
            if (!mume_window_is_mapped(ss[stack]))
                continue;
            mume_window_get_geometry(
                ss[stack], &rect.x, &rect.y, &rect.width, &rect.height);
            rect.x -= self->x;
            rect.y -= self->y;
            cairo_region_subtract_rectangle(rgn, &rect);
        }

        mume_free_children(self->parent, ss);
    }
}

void mume_window_region_clip_children(
    const void *self, cairo_region_t *rgn)
{
    void **cs;
    unsigned int cc;

    cs = mume_query_children(self, &cc, 0);
    if (cs) {
        mume_rect_t rect;
        while (cc-- > 0) {
            if (!mume_window_is_mapped(cs[cc]))
                continue;
            mume_window_get_geometry(
                cs[cc], &rect.x, &rect.y, &rect.width, &rect.height);
            cairo_region_subtract_rectangle(rgn, &rect);
        }
        mume_free_children(self, cs);
    }
}

const cairo_region_t* mume_window_get_urgn(const void *self)
{
    return mume_urgnmgr_get_urgn(mume_urgnmgr(), self);
}

cairo_t* mume_window_begin_paint(void *self, int mode)
{
    int x, y;
    void *bwin;
    cairo_t *cr;
    cairo_region_t *rgn;

    switch (mode) {
    case MUME_PM_INVALID:
        {
            void *umgr = mume_urgnmgr();
            rgn = (cairo_region_t*)mume_urgnmgr_last_rgn(umgr);
            if (NULL == rgn ||
                self != mume_urgnmgr_last_win(umgr))
            {
                mume_warning(("Not last update window: %s\n",
                              mume_class_name(mume_class_of(self))));
                return NULL;
            }

            cairo_region_reference(rgn);
        }
        break;

    case MUME_PM_WINDOW:
        rgn = mume_window_region_create(self);
        break;

    case MUME_PM_VISIBLE:
        rgn = mume_window_region_create(self);
        mume_window_region_clip_children(self, rgn);
        break;

    default:
        rgn = NULL;
        break;
    }

    bwin = mume_window_seek_backwin(self, &x, &y);
    cr = mume_backwin_begin_paint(bwin);
    if (cr) {
        cairo_save(cr);
        cairo_translate(cr, x, y);
        if (rgn) {
            int num;
            mume_rect_t rect;
            num = cairo_region_num_rectangles(rgn);
            while (num-- > 0) {
                cairo_region_get_rectangle(rgn, num, &rect);
                cairo_rectangle(cr, rect.x, rect.y,
                                rect.width, rect.height);
            }

            cairo_clip(cr);
        }
    }
    else {
        /*
        mume_warning(("Backwin begin paint failed: %s\n",
                      mume_class_name(mume_class_of(bwin))));
        */
    }

    cairo_region_destroy(rgn);
    return cr;
}

void mume_window_end_paint(void *self, cairo_t *cr)
{
    void *bwin = mume_window_seek_backwin(self, 0, 0);

    cairo_restore(cr);
    mume_backwin_end_paint(bwin, cr);
}

void mume_window_set_user_data(
    void *_self, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy)
{
    struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (NULL == self->user_data)
        self->user_data = mume_user_data_new();

    mume_user_data_set(self->user_data, key, data, destroy);
}

void* mume_window_get_user_data(
    const void *_self, const mume_user_data_key_t *key)
{
    const struct _window *self = _self;

    assert(mume_is_of(_self, mume_window_class()));

    if (self->user_data)
        return mume_user_data_get(self->user_data, key);

    return NULL;
}

void mume_window_set_cursor(void *_self, void *cursor)
{
    struct _window *self = _self;
    void *pointer_owner = mume_pointer_owner();

    assert(mume_is_of(_self, mume_window_class()));
    assert(!cursor || mume_is_of(cursor, mume_cursor_class()));

    mume_window_set_user_data(
        self, &_window_cursor_key, cursor, NULL);

    if (pointer_owner == self ||
        mume_window_is_ancestor(pointer_owner, self))
    {
        cursor = mume_window_seek_cursor(pointer_owner);
        mume_backwin_set_cursor(
            mume_window_seek_backwin(self, NULL, NULL), cursor);
    }
}

void* mume_window_get_cursor(const void *self)
{
    return mume_window_get_user_data(self, &_window_cursor_key);
}

void* mume_window_seek_cursor(const void *_self)
{
    const struct _window *win = _self;
    void *cursor = NULL;

    assert(mume_is_of(_self, mume_window_class()));

    while (win) {
        cursor = mume_window_get_cursor(win);
        if (cursor)
            return cursor;

        win = win->parent;
    }

    return cursor;
}

void _mume_window_handle_event(
    const void *_clazz, void *_self, mume_event_t *event)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_event, (_self, event));
}

void _mume_window_handle_key_down(
    const void *_clazz, void *_self,
    int x, int y, int state, int keysym)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_key_down,
        (_self, x, y, state, keysym));
}

void _mume_window_handle_key_up(
    const void *_clazz, void *_self, int x, int y, int state, int keysym)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_key_up,
        (_self, x, y, state, keysym));
}

void _mume_window_handle_button_down(
    const void *_clazz, void *_self, int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_button_down,
        (_self, x, y, state, button));
}

void _mume_window_handle_button_up(
    const void *_clazz, void *_self, int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_button_up,
        (_self, x, y, state, button));
}

void _mume_window_handle_button_dblclk(
    const void *_clazz, void *_self, int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_button_dblclk,
        (_self, x, y, state, button));
}

void _mume_window_handle_button_tplclk(
    const void *_clazz, void *_self, int x, int y, int state, int button)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_button_tplclk,
        (_self, x, y, state, button));
}

void _mume_window_handle_mouse_motion(
    const void *_clazz, void *_self, int x, int y, int state)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_mouse_motion,
        (_self, x, y, state));
}

void _mume_window_handle_mouse_enter(
    const void *_clazz, void *_self, int x, int y,
    int state, int mode, int detail)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_mouse_enter,
        (_self, x, y, state, mode, detail));
}

void _mume_window_handle_mouse_leave(
    const void *_clazz, void *_self, int x, int y,
    int state, int mode, int detail)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_mouse_leave,
        (_self, x, y, state, mode, detail));
}

void _mume_window_handle_focus_in(
    const void *_clazz, void *_self, int mode, int detail)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_focus_in,
        (_self, mode, detail));
}

void _mume_window_handle_focus_out(
    const void *_clazz, void *_self, int mode, int detail)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_focus_out,
        (_self, mode, detail));
}

void _mume_window_handle_expose(
    const void *_clazz, void *_self, int x, int y,
    int width, int height, int count)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_expose,
        (_self, x, y, width, height, count));
}

void _mume_window_handle_create(
    const void *_clazz, void *_self, void *window,
    int x, int y, int width, int height)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_create,
        (_self, window, x, y, width, height));
}

void _mume_window_handle_destroy(
    const void *_clazz, void *_self, void *window)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_destroy, (_self, window));
}

void _mume_window_handle_map(
    const void *_clazz, void *_self, void *window)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_map, (_self, window));
}

void _mume_window_handle_unmap(
    const void *_clazz, void *_self, void *window)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_unmap, (_self, window));
}

void _mume_window_handle_reparent(
    const void *_clazz, void *_self, void *window,
    void *parent, int x, int y)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_reparent,
        (_self, window, parent, x, y));
}

void _mume_window_handle_move(
    const void *_clazz, void *_self, void *window,
    int x, int y, int old_x, int old_y)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_move,
        (_self, window, x, y, old_x, old_y));
}

void _mume_window_handle_resize(
    const void *_clazz, void *_self, void *window,
    int w, int h, int old_w, int old_h)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_resize,
        (_self, window, w, h, old_w, old_h));
}

void _mume_window_handle_sizehint(
    const void *_clazz, void *_self, int *pref_w, int *pref_h,
    int *min_w, int *min_h, int *max_w, int *max_h)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_sizehint,
        (_self, pref_w, pref_h, min_w, min_h, max_w, max_h));
}

void _mume_window_handle_command(
    const void *_clazz, void *_self, void *window, int command)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_command,
        (_self, window, command));
}

void _mume_window_handle_notify(
    const void *_clazz, void *_self,
    void *window, int code, void *data)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_notify,
        (_self, window, code, data));
}

void _mume_window_handle_scroll(
    const void *_clazz, void *_self,
    void *window, int hitcode, int pos)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_scroll,
        (_self, window, hitcode, pos));
}

void _mume_window_handle_close(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_window_meta_class(), mume_window_class(),
        struct _window_class, handle_close, (_self));
}
