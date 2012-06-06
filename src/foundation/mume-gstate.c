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
#include "mume-gstate.h"
#include "mume-backend.h"
#include "mume-backwin.h"
#include "mume-class.h"
#include "mume-clipboard.h"
#include "mume-clsmgr.h"
#include "mume-cursor.h"
#include "mume-datasrc.h"
#include "mume-dbgutil.h"
#include "mume-debug.h"
#include "mume-events.h"
#include "mume-frontend.h"
#include "mume-list.h"
#include "mume-memory.h"
#include "mume-oset.h"
#include "mume-resmgr.h"
#include "mume-text-layout.h"
#include "mume-thread.h"
#include "mume-timer.h"
#include "mume-types.h"
#include "mume-urgnmgr.h"
#include "mume-window.h"
#include MUME_ASSERT_H

struct _datafmt_info {
    const char *format;
    void *object;
};

struct _gstate {
    void *frontend;
    void *backend;
    void *urgnmgr;
    void *cursors[MUME_NUMCURSORS];
    void *clipboard;
    void *text_layout;
    mume_oset_t *datafmts;
    mume_resmgr_t *resmgr;
    mume_list_t *event_list;
    mume_mutex_t *event_mutex;
    mume_timerq_t *timer_queue;
    void *root_window;
    int blocking;
};

const char *_mume_argv0;
struct _gstate *_mume_gstate;
mume_object_t _mume_clsmgr;

static void _datafmt_info_destruct(void *obj, void *p)
{
    mume_delete(((struct _datafmt_info*)obj)->object);
}

static int _extract_dirty_event(void)
{
    int i, n;
    mume_event_t event;
    mume_rect_t rect;
    const cairo_region_t *rgn;
    if (!mume_urgnmgr_pop_urgn(_mume_gstate->urgnmgr))
        return 0;

    rgn = mume_urgnmgr_last_rgn(_mume_gstate->urgnmgr);
    n = cairo_region_num_rectangles(rgn);
    event.expose.type = MUME_EVENT_EXPOSE;
    event.expose.window = (void*)mume_urgnmgr_last_win(
        _mume_gstate->urgnmgr);

    for (i = 0; i < n; ++i) {
        cairo_region_get_rectangle(rgn, i, &rect);
        event.expose.x = rect.x;
        event.expose.y = rect.y;
        event.expose.width = rect.width;
        event.expose.height = rect.height;
        event.expose.count = n - i - 1;
        memcpy(mume_list_data(mume_list_push_back(
            _mume_gstate->event_list, sizeof(mume_event_t))),
               &event, sizeof(mume_event_t));
    }

    return 1;
}

void mume_initialize(const char *argv0)
{
    _mume_argv0 = argv0;

    if (NULL == _mume_clsmgr.data)
        _mume_clsmgr = mume_clsmgr_new();
}

int mume_gui_initialize(void *frontend, void *backend)
{
    void *bwin;

    assert(mume_is_of(frontend, mume_frontend_class()));
    assert(mume_is_of(backend, mume_backend_class()));

    if (_mume_gstate || NULL == backend)
        return 0;

    _mume_gstate = malloc_abort(sizeof(struct _gstate));
    _mume_gstate->frontend = frontend;
    _mume_gstate->backend = backend;
    _mume_gstate->urgnmgr = mume_urgnmgr_new();
    memset(_mume_gstate->cursors, 0, sizeof(_mume_gstate->cursors));
    _mume_gstate->clipboard = mume_backend_clipboard(backend);
    _mume_gstate->text_layout = mume_text_layout_new();
    _mume_gstate->datafmts = mume_oset_new(
        _mume_type_string_compare, _datafmt_info_destruct, NULL);
    _mume_gstate->resmgr = mume_resmgr_new();
    _mume_gstate->event_list = mume_list_new(NULL, NULL);
    _mume_gstate->event_mutex = mume_mutex_new();
    _mume_gstate->timer_queue = mume_timerq_new();
    _mume_gstate->root_window = mume_window_new(NULL, 0, 0, 0, 0);
    _mume_gstate->blocking = 0;

    bwin = mume_backend_root_backwin(backend);
    mume_window_set_backwin(_mume_gstate->root_window, bwin);
    mume_window_sync_backwin(_mume_gstate->root_window, 1);

    return 1;
}

void mume_gui_uninitialize(void)
{
    if (_mume_gstate) {
        int i;
        mume_delete(_mume_gstate->root_window);
        mume_delete(_mume_gstate->text_layout);
        mume_resmgr_delete(_mume_gstate->resmgr);
        mume_timerq_delete(_mume_gstate->timer_queue);
        mume_mutex_delete(_mume_gstate->event_mutex);
        mume_list_delete(_mume_gstate->event_list);

        if (_mume_gstate->clipboard)
            mume_refobj_release(_mume_gstate->clipboard);

        mume_oset_delete(_mume_gstate->datafmts);

        for (i = 0; i < COUNT_OF(_mume_gstate->cursors); ++i)
            mume_delete(_mume_gstate->cursors[i]);

        mume_delete(_mume_gstate->urgnmgr);
        free(_mume_gstate);
        _mume_gstate = NULL;
    }
}

const char* mume_program_name(void)
{
    return _mume_argv0;
}

struct mume_class* mume_register_class(
    const char *class_name, size_t class_size,
    void (*class_init)(struct mume_class*),
    void (*class_finalize)(struct mume_class*),
    const struct mume_class *super_class, size_t object_size,
    int (*message_proc)(struct mume_object*, struct mume_message*))
{
    return NULL;
}

mume_resmgr_t* mume_resmgr(void)
{
    return _mume_gstate->resmgr;
}

void* mume_urgnmgr(void)
{
    return _mume_gstate->urgnmgr;
}

int mume_metrics(int index)
{
#define _METRICS_RESNS "metrics"
    switch (index) {
    case MUME_GM_CXHSCROLL:
        return mume_resmgr_get_integer(
            _mume_gstate->resmgr, _METRICS_RESNS, "cxhscroll", 14);
    case MUME_GM_CYHSCROLL:
        return mume_resmgr_get_integer(
            _mume_gstate->resmgr, _METRICS_RESNS, "cyhscroll", 14);
    case MUME_GM_CXVSCROLL:
        return mume_resmgr_get_integer(
            _mume_gstate->resmgr, _METRICS_RESNS, "cxvscroll", 14);
    case MUME_GM_CYVSCROLL:
        return mume_resmgr_get_integer(
            _mume_gstate->resmgr, _METRICS_RESNS, "cyvscroll", 14);
    }
#undef _METRICS_RESNS
    mume_warning(("Unknown metrics: %d\n", index));
    return 0;
}

void* mume_cursor(int id)
{
    if (_mume_gstate->cursors[id] || MUME_CURSOR_NONE == id)
        return _mume_gstate->cursors[id];

    _mume_gstate->cursors[id] = mume_backend_create_cursor(
        _mume_gstate->backend, id);

    if (NULL == _mume_gstate->cursors[id])
        mume_warning(("Create cursor failed: %d\n", id));

    return _mume_gstate->cursors[id];
}

void* mume_clipboard(void)
{
    if (NULL == _mume_gstate->clipboard) {
        mume_warning(("No global clipboard available.\n"));
        _mume_gstate->clipboard = mume_clipboard_new();
    }

    return _mume_gstate->clipboard;
}

void* mume_datafmt(const char *format)
{
    mume_oset_node_t *n;
    struct _datafmt_info *p;

    n = mume_oset_find(_mume_gstate->datafmts, &format);
    if (n) {
        p = mume_oset_data(n);
    }
    else {
        n = mume_oset_newnode(sizeof(*p));
        p = mume_oset_data(n);
        p->object = mume_backend_data_format(
            _mume_gstate->backend, format);

        if (NULL == p->object) {
            mume_warning(("mume_backend_data_format(\"%s\")\n",
                          format));
            p->object = mume_new(mume_datafmt_class(), format);
        }

        p->format = mume_datafmt_get_name(p->object);
        mume_oset_insert(_mume_gstate->datafmts, n);
    }

    return p->object;
}

void* mume_text_layout(void)
{
    return _mume_gstate->text_layout;
}

void* mume_frontend(void)
{
    return _mume_gstate->frontend;
}

void* mume_backend(void)
{
    return _mume_gstate->backend;
}

void mume_screen_size(int *width, int *height)
{
    mume_backend_screen_size(_mume_gstate->backend, width, height);
}

void* mume_root_backwin(void)
{
    return mume_backend_root_backwin(_mume_gstate->backend);
}

void* mume_root_window(void)
{
    return _mume_gstate->root_window;
}

void mume_set_input_focus(void *win)
{
    assert(0);
}

void* mume_get_input_focus(void)
{
    assert(0);
    return NULL;
}

void mume_query_pointer(
    const void *window, int *x, int *y, int *state)
{
    mume_backend_query_pointer(_mume_gstate->backend, x, y, state);
    mume_translate_coords(NULL, window, x, y);
}

void mume_grab_pointer(void *window)
{
    void *bwin = mume_window_seek_backwin(window, NULL, NULL);
    mume_backwin_grab_pointer(bwin);
    mume_frontend_set_pointer_owner(_mume_gstate->frontend, window);
}

void mume_ungrab_pointer(void *window)
{
    void *bwin = mume_window_seek_backwin(window, NULL, NULL);
    mume_backwin_ungrab_pointer(bwin);
    mume_frontend_set_pointer_owner(_mume_gstate->frontend, NULL);
}

void mume_open_popup(void *window)
{
    mume_frontend_open_popup(_mume_gstate->frontend, window);
}

void mume_close_popup(void *window)
{
    mume_frontend_close_popup(_mume_gstate->frontend, window);
}

void* mume_pointer_owner(void)
{
    return mume_frontend_get_pointer_owner(_mume_gstate->frontend);
}

void* mume_keyboard_owner(void)
{
    return mume_frontend_get_keyboard_owner(_mume_gstate->frontend);
}

void mume_schedule_timer(mume_timer_t *timer, int interval)
{
    mume_timerq_schedule(
        _mume_gstate->timer_queue, timer, interval);
}

void mume_cancel_timer(mume_timer_t *timer)
{
    mume_timerq_cancel(_mume_gstate->timer_queue, timer);
}

int mume_wait_event(mume_event_t *event)
{
    mume_mutex_lock(_mume_gstate->event_mutex);
    while (mume_list_empty(_mume_gstate->event_list)) {
        mume_timeval_t tv;
        int wait = MUME_WAIT_INFINITE;
        mume_mutex_unlock(_mume_gstate->event_mutex);
        mume_backend_handle_event(_mume_gstate->backend, 0);
        mume_mutex_lock(_mume_gstate->event_mutex);
        if (!mume_list_empty(_mume_gstate->event_list) ||
            _extract_dirty_event())
        {
            break;
        }

        _mume_gstate->blocking = 1;
        mume_mutex_unlock(_mume_gstate->event_mutex);
        if (mume_timerq_check(_mume_gstate->timer_queue, &tv)) {
            wait = tv.tv_sec * MUME_MSECS_PER_SEC +
                   tv.tv_usec / MUME_USECS_PER_MSEC;
        }

        mume_backend_handle_event(_mume_gstate->backend, wait);
        mume_mutex_lock(_mume_gstate->event_mutex);
        _mume_gstate->blocking = 0;
    }

    if (event) {
        memcpy(event, mume_list_data(mume_list_front(
            _mume_gstate->event_list)), sizeof(mume_event_t));
    }

    mume_list_pop_front(_mume_gstate->event_list);
    mume_mutex_unlock(_mume_gstate->event_mutex);
    return 1;
}

int mume_peek_event(mume_event_t *event, int remove)
{
    int result = 1;
    mume_timerq_check(_mume_gstate->timer_queue, NULL);
    mume_mutex_lock(_mume_gstate->event_mutex);
    if (mume_list_empty(_mume_gstate->event_list)) {
        mume_mutex_unlock(_mume_gstate->event_mutex);
        mume_backend_handle_event(_mume_gstate->backend, 0);
        mume_mutex_lock(_mume_gstate->event_mutex);

        if (mume_list_empty(_mume_gstate->event_list) &&
            !_extract_dirty_event())
        {
            result = 0;
        }
    }

    if (result && event) {
        memcpy(event, mume_list_data(mume_list_front(
            _mume_gstate->event_list)), sizeof(mume_event_t));

        if (remove)
            mume_list_pop_front(_mume_gstate->event_list);
    }

    mume_mutex_unlock(_mume_gstate->event_mutex);
    return result;
}

void mume_disp_event(mume_event_t *event)
{
    void *window = event->any.window;

    assert(_mume_gstate && window);

    mume_dump_event(event);

    _mume_window_handle_event(NULL, window, event);
}

int mume_send_event(mume_event_t *event)
{
    /* TODO: implement multi thread send */
    mume_disp_event(event);
    return 1;
}

int mume_send_message(mume_object_t *obj, mume_message_t *msg)
{
    return mume_class_message(mume_class_of2(obj), obj, msg);
}

int _mume_post_event(mume_event_t event, int wakeup)
{
    int result = 0;

    mume_mutex_lock(_mume_gstate->event_mutex);

    if (mume_list_size(_mume_gstate->event_list) < 256) {
        memcpy(mume_list_data(mume_list_push_back(
            _mume_gstate->event_list, sizeof(mume_event_t))),
               &event, sizeof(mume_event_t));

        result = 1;
    }
    else {
        mume_warning(("event queue is full\n"));
    }

    if (wakeup && _mume_gstate->blocking)
        mume_backend_wakeup_event(_mume_gstate->backend);

    mume_mutex_unlock(_mume_gstate->event_mutex);
    return result;
}

void _mume_window_clear_res(void *self)
{
    mume_event_t *event;
    mume_list_node_t *node;
    mume_list_node_t *next;

    /* Remove all events. */
    mume_mutex_lock(_mume_gstate->event_mutex);
    node = mume_list_front(_mume_gstate->event_list);
    while (node) {
        next = mume_list_next(node);
        event = (mume_event_t*)mume_list_data(node);

        if (event->any.window == self)
            mume_list_erase(_mume_gstate->event_list, node);

        node = next;
    }

    mume_mutex_unlock(_mume_gstate->event_mutex);

    assert(NULL == mume_window_get_urgn(self));

    mume_frontend_remove_window(_mume_gstate->frontend, self);
}
