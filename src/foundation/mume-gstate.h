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
#ifndef MUME_FOUNDATION_GSTATE_H
#define MUME_FOUNDATION_GSTATE_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define _(string) string

enum mume_global_metrics_e {
    MUME_GM_CXHSCROLL,
    MUME_GM_CYHSCROLL,
    MUME_GM_CXVSCROLL,
    MUME_GM_CYVSCROLL
};

struct mume_class;
struct mume_object;
struct mume_message;

/* Initialize the mume library. This must be called before
 * any other operations.
 */
mume_public void mume_initialize(const char *argv0);

/* Initialize the GUI module. This must be called before
 * any other GUI operations.
 */
mume_public int mume_gui_initialize(void *frontend, void *backend);

/* Uninitialize the GUI module. After this operation,
 * user should not call any other GUI operations.
 */
mume_public void mume_gui_uninitialize(void);

/* Get the program name, which is passed to mume_initialize. */
mume_public const char* mume_program_name(void);

/* Register a class in the global class manager.
 * Return the registered class or an existing one. */
mume_public struct mume_class* mume_register_class(
    const char *class_name, size_t class_size,
    void (*class_init)(struct mume_class*),
    void (*class_finalize)(struct mume_class*),
    const struct mume_class *super_class, size_t object_size,
    int (*message_proc)(struct mume_object*, struct mume_message*));

/* Get the global GUI resource manager. */
mume_public mume_resmgr_t* mume_resmgr(void);

/* Get the global update region manager. */
mume_public void* mume_urgnmgr(void);

/* Get the specified GUI metrics or configuration settings. */
mume_public int mume_metrics(int index);

/* Get the shared standard Cursor. */
mume_public void* mume_cursor(int id);

/* Get the clipboard object. */
mume_public void* mume_clipboard(void);

/* Get the data format object. */
mume_public void* mume_datafmt(const char *format);

/* Get the text layout object. */
mume_public void* mume_text_layout(void);

/* Get the current frontend. */
mume_public void* mume_frontend(void);

/* Get the current backend. */
mume_public void* mume_backend(void);

/* Get the screen size. */
mume_public void mume_screen_size(int *width, int *height);

/* Get the root backwin of current backend. */
mume_public void* mume_root_backwin(void);

/* Get the root window. */
mume_public void* mume_root_window(void);

/* Set/Get the keyboard focus window.
 *
 * The window should be mapped and not disabled.
 */
mume_public void mume_set_input_focus(void *win);

mume_public void* mume_get_input_focus(void);

/* Query current mouse pointer coordinates and modifiers.
 *
 * <win> :
 *   Specify the window.
 * <x>, <y> :
 *   Receive the pointer coordinates, relative to the <win>.
 * <state> :
 *   Receive the modifiers mask.
 *
 * Any of the <win>, <x>, <y>, <state> can be NULL.
 */
mume_public void mume_query_pointer(
    const void *window, int *x, int *y, int *state);

/* Grab/Ungrab mouse pointer. */
mume_public void mume_grab_pointer(void *window);

mume_public void mume_ungrab_pointer(void *window);

/* Add/Remove window to the "popup list". */
mume_public void mume_open_popup(void *window);

mume_public void mume_close_popup(void *window);

/* Get the window that currently "owns" the mouse pointer.
 * This is the window that grabs or under the pointer.
 */
mume_public void* mume_pointer_owner(void);

/* Get the window that currently "owns" the keyboard,
 * This is the window that will receive key event.
 */
mume_public void* mume_keyboard_owner(void);

/* Schedule/Calcel a timer.
 *
 * Timer interval is in millisecond. The granularity of the
 * timer is vary from backend to backend, typically 10 ms.
 */
mume_public void mume_schedule_timer(
    mume_timer_t *timer, int interval);

mume_public void mume_cancel_timer(mume_timer_t *timer);

/* Process pending sent events, then wait until a posted event
 * is available for retrieval.
 *
 * If <event> is NULL, this function just wait until the event
 * queue is nonempty or some error occured.
 */
mume_public int mume_wait_event(mume_event_t *event);

/* Process pending sent events, then check the event queue for
 * a posted event and retrieve the event (if any exist).
 *
 * If <event> is NULL, this function just check whether there is
 * event available.
 */
mume_public int mume_peek_event(mume_event_t *event, int remove);

/* Dispatch a event to the destination window. */
mume_public void mume_disp_event(mume_event_t *event);

/* Send a event to a window, wait until the event been processed. */
mume_public int mume_send_event(mume_event_t *event);

/* Send a message to the object, wait until the message
 * has been processed. */
mume_public int mume_send_message(
    struct mume_object *obj, struct mume_message *msg);

/* Post a event to the event queue. */
mume_public int _mume_post_event(mume_event_t event, int wakeup);

#define mume_post_event(_event) _mume_post_event(_event, 1)

/* Clear up all the global resource related to the window
 * (such as update region, pending events). */
mume_public void _mume_window_clear_res(void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_GSTATE_H */
