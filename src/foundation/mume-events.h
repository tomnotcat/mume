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
#ifndef MUME_FOUNDATION_EVENTS_H
#define MUME_FOUNDATION_EVENTS_H

#include "mume-common.h"
#include "mume-keysym.h"

MUME_BEGIN_DECLS

#define MUME_EVENT_LIST(_) \
    _(NONE)                \
    _(KEYDOWN)             \
    _(KEYUP)               \
    _(BUTTONDOWN)          \
    _(BUTTONUP)            \
    _(BUTTONDBLCLK)        \
    _(BUTTONTPLCLK)        \
    _(MOUSEMOTION)         \
    _(MOUSEENTER)          \
    _(MOUSELEAVE)          \
    _(FOCUSIN)             \
    _(FOCUSOUT)            \
    _(EXPOSE)              \
    _(CREATE)              \
    _(DESTROY)             \
    _(MAP)                 \
    _(UNMAP)               \
    _(REPARENT)            \
    _(MOVE)                \
    _(RESIZE)              \
    _(SIZEHINT)            \
    _(COMMAND)             \
    _(NOTIFY)              \
    _(SCROLL)              \
    _(CLOSE)               \
    _(QUIT)

#define MUME_DEFINE_EVENT(_E) MUME_EVENT_##_E,

enum mume_event_e {
    MUME_EVENT_LIST(MUME_DEFINE_EVENT)
    MUME_NUM_EVENTS
};

#undef MUME_DEFINE_EVENT

enum mume_modifier_e {
    MUME_MOD_SHIFT    = 0x0001,
    MUME_MOD_CONTROL  = 0x0002,
    MUME_MOD_ALT      = 0x0004,
    MUME_MOD_META     = 0x0008,
    MUME_MOD_CAPS     = 0x0010,
    MUME_MOD_LBUTTON  = 0x0100,
    MUME_MOD_MBUTTON  = 0x0200,
    MUME_MOD_RBUTTON  = 0x0400,
    MUME_MOD_XBUTTON1 = 0x0800,
    MUME_MOD_XBUTTON2 = 0x1000
};

enum mume_button_e {
    MUME_BUTTON_LEFT      = 0x0001,
    MUME_BUTTON_MIDDLE    = 0x0002,
    MUME_BUTTON_RIGHT     = 0x0003,
    MUME_BUTTON_WHEELUP   = 0x0004,
    MUME_BUTTON_WHEELDOWN = 0x0005,
    MUME_BUTTON_X1        = 0x0006,
    MUME_BUTTON_X2        = 0x0007
};

enum mume_notify_mode_e {
    MUME_NOTIFY_NORMAL,
    MUME_NOTIFY_GRAB,
    MUME_NOTIFY_UNGRAB
};

enum mume_notify_detail_e {
    MUME_NOTIFY_ANCESTOR,
    MUME_NOTIFY_VIRTUAL,
    MUME_NOTIFY_INFERIOR,
    MUME_NOTIFY_NONLINEAR,
    MUME_NOTIFY_NONLINEARVIRTUAL
};

#define MUME_COMMAND_LIST(_) \
    _(NONE)                  \
    _(COPY)                  \
    _(CUT)                   \
    _(PASTE)

#define MUME_DEFINE_COMMAND(_C) MUME_COMMAND_##_C,

enum mume_command_e {
    MUME_COMMAND_LIST(MUME_DEFINE_COMMAND)
    MUME_NUM_COMMANDS
};

#undef MUME_DEFINE_COMMAND

/*========================================
 * [Function]
 *  Every event should have at least those
 *  members appear in mume_any_event_t.
 * [Parameter]
 *  type : one of the mume_event_e
 *  receiver : the receiver this event
 *========================================*/
typedef struct mume_any_event_s {
    int type;
    void *window;
} mume_any_event_t;

/*========================================
 * [Function]
 *  Post when keyboard pressed or released.
 * [Parameter]
 *  type : MUME_EVENT_KEYDOWN, MUME_EVENT_KEYUP
 *  window : the receiver this event
 *  x, y : cursor coordinates in event window
 *  state : key or button mask
 *  keysym : virtual key sympol
 *========================================*/
typedef struct mume_key_event_s {
    int type;
    void *window;
    int x, y;
    int state;
    int keysym;
} mume_key_event_t;

/*========================================
 * [Function]
 *  Post when mouse button pressed or released.
 * [Parameter]
 *  type : MUME_EVENT_BUTTONDOWN, MUME_EVENT_BUTTONUP,
 *         MUME_EVENT_BUTTONDBLCLK, MUME_EVENT_BUTTONTPLCLK
 *  window : the receiver this event
 *  x, y : cursor coordinates in event window
 *  state : key or button mask
 *  button : button identifier
 *========================================*/
typedef struct mume_button_event_s {
    int type;
    void *window;
    int x, y;
    int state;
    int button;
} mume_button_event_t;

/*========================================
 * [Function]
 *  Post when mouse logically moved.
 * [Parameter]
 *  type : MUME_EVENT_MOUSEMOTION, MUME_EVENT_MOUSEENTER,
 *         MUME_EVENT_MOUSELEAVE
 *  window : the receiver this event
 *  x, y : cursor coordinates in event window
 *  state : key or button mask
 *========================================*/
typedef struct mume_motion_event_s {
    int type;
    void *window;
    int x, y;
    int state;
} mume_motion_event_t;

/*========================================
 * [Function]
 *  Post when mouse enter/leave a window.
 * [Parameter]
 *  type : MUME_EVENT_MOUSEENTER, MUME_EVENT_MOUSELEAVE
 *  window : the receiver this event
 *  x, y : cursor coordinates in event window
 *  state : key or button mask
 *  mode : one of the mume_notify_mode_e
 *  detail : one of the mume_notify_detail_e
 *========================================*/
typedef struct mume_crossing_event_s {
    int type;
    void *window;
    int x, y;
    int state;
    int mode;
    int detail;
} mume_crossing_event_t;

/*========================================
 * [Function]
 *  Post when a window get/lost keyboard focus.
 * [Parameter]
 *  type : MUME_EVENT_FOCUSIN, MUME_EVENT_FOCUSOUT
 *  window : the receiver this event
 *  mode : one of the mume_notify_mode_e
 *========================================*/
typedef struct mume_focus_event_s {
    int type;
    void *window;
    int mode;
    int detail;
} mume_focus_event_t;

/*========================================
 * [Function]
 *  Generated when all or part of a window
 *  becomes visible and needs to be redrawn.
 * [Parameter]
 *  type : MUME_EVENT_EXPOSE
 *  window : the receiver this event
 *  x, y, width, height : exposed area
 *  count : number of expose events follow
 *========================================*/
typedef struct mume_expose_event_s {
    int type;
    void *window;
    int x, y;
    int width;
    int height;
    int count;
} mume_expose_event_t;

/*========================================
 * [Function]
 *  Send after a window has been created.
 * [Parameter]
 *  type : MUME_EVENT_CREATE
 *  parent : parent of the window
 *  window : the created window
 *  x, y : coordinates relative to parent
 *  width, height : window size
 * [Notice]
 *  This event is only send to the parent
 *  of the created window.
 *========================================*/
typedef struct mume_create_event_s {
    int type;
    void *parent;
    void *window;
    int x, y;
    int width;
    int height;
} mume_create_event_t;

/*========================================
 * [Function]
 *  Send before destroy a window.
 * [Parameter]
 *  type : MUME_EVENT_DESTROY
 *  event : event receiver (can be either the
 *          destroying window or its parent)
 *  window : the destroying window
 *========================================*/
typedef struct mume_destroy_event_s {
    int type;
    void *event;
    void *window;
} mume_destroy_event_t;

/*========================================
 * [Function]
 *  Send after map a window.
 * [Parameter]
 *  type : MUME_EVENT_MAP
 *  event : event receiver (can be either the
 *          mapped window or its parent)
 *  window : the mapped window
 *========================================*/
typedef struct mume_map_event_s {
    int type;
    void *event;
    void *window;
} mume_map_event_t;

/*========================================
 * [Function]
 *  Send after unmap a window.
 * [Parameter]
 *  type : MUME_EVENT_UNMAP
 *  event : event receiver (can be either the
 *          unmapped window or its parent)
 *  window : the unmapped window
 *========================================*/
typedef struct mume_unmap_event_s {
    int type;
    void *event;
    void *window;
} mume_unmap_event_t;

/*========================================
 * [Function]
 *  Send when changing a window's parent,
 *  this event will be sent to the old parent,
 *  the reparented window and the new parent.
 * [Parameter]
 *  type : MUME_EVENT_REPARENT
 *  event : event receiver (can be either the
 *          reparented window or its parent)
 *  window : the reparented window
 *  parent : the new parent
 *  x, y : the reparented window's coordinates
 *         relative to the new parent
 *========================================*/
typedef struct mume_reparent_event_s {
    int type;
    void *event;
    void *window;
    void *parent;
    int x, y;
} mume_reparent_event_t;

/*========================================
 * [Function]
 *  Send after a window's position has changed.
 * [Parameter]
 *  type : MUME_EVENT_MOVE
 *  event : event receiver (can be either the
 *          moved window or its parent)
 *  window : the moved window
 *  x, y : the new (current) position
 *  old_x, old_y : the old position
 *========================================*/
typedef struct mume_move_event_s {
    int type;
    void *event;
    void *window;
    int x, y;
    int old_x, old_y;
} mume_move_event_t;

/*========================================
 * [Function]
 *  Send after a window's size has changed.
 * [Parameter]
 *  type : MUME_EVENT_RESIZE
 *  event : event receiver (can be either the
 *          resized window or its parent)
 *  window : the resized window
 *  width, height : the new (current) size
 *  old_width, old_height : the old size
 *========================================*/
typedef struct mume_resize_event_s {
    int type;
    void *event;
    void *window;
    int width, height;
    int old_width, old_height;
} mume_resize_event_t;

/*========================================
 * [Function]
 *  Send to query a window's size hint.
 * [Parameter]
 *  type : MUME_EVENT_SIZEHINT
 *  event : event receiver
 *  pref_width, pref_height : receive the preferred size.
 *  min_width, min_height : receive the minimum size.
 *  max_width, max_height : receive the maximum size.
 *========================================*/
typedef struct mume_sizehint_event_s {
    int type;
    void *event;
    int pref_width, pref_height;
    int min_width, min_height;
    int max_width, max_height;
} mume_sizehint_event_t;

/*========================================
 * [Function]
 *  Command event generated by button, menu, etc.
 * [Parameter]
 *  type : MUME_EVENT_COMMAND
 *  event : The event receiver
 *  window : The event generator.
 *  command : Command code.
 *========================================*/
typedef struct mume_command_event_s {
    int type;
    void *event;
    void *window;
    int command;
} mume_command_event_t;

/*========================================
 * [Function]
 *  Event generated by window to notify its
 *  data or state change.
 * [Parameter]
 *  type : MUME_EVENT_NOTIFY
 *  event : the event receiver
 *  window : the event generator
 *  code : notify code.
 *  data : user defined data.
 *========================================*/
typedef struct mume_notify_event_s {
    int type;
    void *event;
    void *window;
    int code;
    void *data;
} mume_notify_event_t;

/*========================================
 * [Function]
 *  Scroll event generated by scroll bar, etc.
 * [Parameter]
 *  type : MUME_EVENT_SCROLL
 *  event : The event receiver
 *  window : The event generator.
 *  hitcode : One of mume_scroll_hitcode_e;
 *  position : The new scroll position.
 *========================================*/
typedef struct mume_scroll_event_s {
    int type;
    void *event;
    void *window;
    int hitcode;
    int position;
} mume_scroll_event_t;

/*========================================
 * [Function]
 *  Post when user try to close a window
 * [Parameter]
 *  type : MUME_EVENT_CLOSE
 * [Return]
 *  zero : continue
 *  nonzero : forbid close the window
 *========================================*/
typedef mume_any_event_t mume_close_event_t;

union mume_event_u {
    int type;
    mume_any_event_t any;
    mume_key_event_t key;
    mume_button_event_t button;
    mume_motion_event_t motion;
    mume_crossing_event_t crossing;
    mume_focus_event_t focus;
    mume_expose_event_t expose;
    mume_create_event_t create;
    mume_destroy_event_t destroy;
    mume_map_event_t map;
    mume_unmap_event_t unmap;
    mume_reparent_event_t reparent;
    mume_move_event_t move;
    mume_resize_event_t resize;
    mume_sizehint_event_t sizehint;
    mume_command_event_t command;
    mume_notify_event_t notify;
    mume_scroll_event_t scroll;
    mume_close_event_t close;
};

/* Utility functions for create event. */
mume_public mume_event_t mume_make_empty_event(void);

mume_public mume_event_t mume_make_create_event(
    void *parent, void *window, int x, int y, int w, int h);

mume_public mume_event_t mume_make_destroy_event(
    void *event, void *window);

mume_public mume_event_t mume_make_map_event(
    void *event, void *window);

mume_public mume_event_t mume_make_unmap_event(
    void *event, void *window);

mume_public mume_event_t mume_make_reparent_event(
    void *event, void *window, void *parent, int x, int y);

mume_public mume_event_t mume_make_move_event(
    void *event, void *window, int x, int y, int ox, int oy);

mume_public mume_event_t mume_make_resize_event(
    void *event, void *window, int width, int height,
    int old_width, int old_height);

mume_public mume_event_t mume_make_sizehint_event(
    void *event, int min_width, int min_height,
    int fit_width, int fit_height, int max_width, int max_height);

mume_public mume_event_t mume_make_command_event(
    void *event, void *window, int command);

mume_public mume_event_t mume_make_notify_event(
    void *event, void *window, int code, void *data);

mume_public mume_event_t mume_make_scroll_event(
    void *event, void *window, int hitcode, int position);

mume_public mume_event_t mume_make_close_event(void *event);

mume_public mume_event_t mume_make_quit_event(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_EVENTS_H */
