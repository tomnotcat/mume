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
#include "mume-base.h"
#include "mume-gui.h"
#include "test-util.h"

static mume_list_t *event_list;

static void _event_checker_proc(void *_self, mume_event_t *event)
{
    *(mume_event_t*)mume_list_data(
        mume_list_push_back(event_list, sizeof(*event))) = *event;

    _mume_window_handle_event(
        mume_window_class(), _self, event);
}

static const void* _event_checker_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_window_meta_class(),
        "event check window",
        mume_window_class(),
        MUME_SIZEOF_WINDOW,
        MUME_PROP_END,
        _mume_window_handle_event,
        _event_checker_proc,
        MUME_FUNC_END);
}

static void* _create_event_check_window(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(_event_checker_class(),
                    parent, x, y, width, height);
}

static int _event_checker_check(int x, ...)
{
    va_list args;
    int errnum = 0;
    va_start(args, x);
    while (1) {
        mume_event_t event = va_arg(args, mume_event_t);
        int cmpbits = 0;
        switch (event.any.type) {
        case MUME_EVENT_KEYDOWN:
        case MUME_EVENT_KEYUP:
            cmpbits = sizeof(event.key);
            break;
        case MUME_EVENT_BUTTONDOWN:
        case MUME_EVENT_BUTTONUP:
        case MUME_EVENT_BUTTONDBLCLK:
        case MUME_EVENT_BUTTONTPLCLK:
            cmpbits = sizeof(event.button);
            break;
        case MUME_EVENT_MOUSEMOTION:
            cmpbits = sizeof(event.motion);
            break;
        case MUME_EVENT_MOUSEENTER:
        case MUME_EVENT_MOUSELEAVE:
            cmpbits = sizeof(event.crossing);
            break;
        case MUME_EVENT_FOCUSIN:
        case MUME_EVENT_FOCUSOUT:
            cmpbits = sizeof(event.focus);
            break;
        case MUME_EVENT_EXPOSE:
            cmpbits = sizeof(event.expose);
            break;
        case MUME_EVENT_CREATE:
            cmpbits = sizeof(event.create);
            break;
        case MUME_EVENT_DESTROY:
            cmpbits = sizeof(event.destroy);
            break;
        case MUME_EVENT_MAP:
            cmpbits = sizeof(event.map);
            break;
        case MUME_EVENT_UNMAP:
            cmpbits = sizeof(event.unmap);
            break;
        case MUME_EVENT_REPARENT:
            cmpbits = sizeof(event.reparent);
            break;
        case MUME_EVENT_MOVE:
            cmpbits = sizeof(event.move);
            break;
        case MUME_EVENT_RESIZE:
            cmpbits = sizeof(event.resize);
            break;
        case MUME_EVENT_CLOSE:
            cmpbits = sizeof(event.close);
            break;
        }
        if (event.any.type) {
            mume_event_t *cur;
            assert(mume_list_size(event_list));
            cur = (mume_event_t*)mume_list_data(
                mume_list_front(event_list));
            if (memcmp(&event, cur, cmpbits)) {
                ++errnum;
                mume_dump_event(&event);
                mume_dump_event(cur);
            }
            mume_list_pop_front(event_list);
        }
        else
            break;
    }
    va_end(args);
    return 0 == errnum && mume_list_empty(event_list);
}

static void _test_create_window(void)
{
    void *win = _create_event_check_window(
        mume_root_window(), 0, 0, 100, 100);
    mume_delete(win);
    test_assert(_event_checker_check(
        0,
        mume_make_destroy_event(win, win),
        mume_make_empty_event()));
}

static void _test_create_children(void)
{
    void *win;
    void *c1, *c2, *c3, *c4;
    unsigned int count;
    mume_event_t event;
    win = _create_event_check_window(
        mume_root_window(), 0, 0, 100, 100);
    c1 = _create_event_check_window(win, 0, 0, 200, 200);
    c2 = _create_event_check_window(c1, 0, 0, 100, 100);
    c3 = _create_event_check_window(c2, 0, 0, 50, 50);
    c4 = _create_event_check_window(c2, 0, 0, 0, 0);
    test_assert(_event_checker_check(
        0,
        mume_make_create_event(
            win, c1, 0, 0, 200, 200),
        mume_make_create_event(
            c1, c2, 0, 0, 100, 100),
        mume_make_create_event(
            c2, c3, 0, 0, 50, 50),
        mume_make_create_event(
            c2, c4, 0, 0, 0, 0),
        mume_make_empty_event()));
    mume_free_children(
        c2, mume_query_children(c2, &count, 0));
    mume_free_children(
        c2, mume_query_children(c2, &count, 1));
    test_assert(2 == count);
    mume_window_set_id(c4, 1);
    mume_window_set_id(c3, 2);
    test_assert(mume_find_child(c2, 1) == c4);
    test_assert(mume_find_child(c2, 2) == c3);
    mume_free_children(
        c4, mume_query_children(c4, &count, 0));
    mume_free_children(
        c4, mume_query_children(c4, &count, 1));
    test_assert(0 == count);
    mume_window_reparent(c4, c1, 10, 20);
    test_assert(_event_checker_check(
        0,
        mume_make_reparent_event(c2, c4, c1, 10, 20),
        mume_make_reparent_event(c4, c4, c1, 10, 20),
        mume_make_reparent_event(c1, c4, c1, 10, 20),
        mume_make_empty_event()));
    /* test clearance of event queue after destroy window */
    event.any.type = MUME_EVENT_NONE;
    event.any.window = win;
    mume_post_event(event);
    event.any.window = c1;
    mume_post_event(event);
    event.any.window = c2;
    mume_post_event(event);
    event.any.window = c3;
    mume_post_event(event);
    event.any.window = c4;
    mume_post_event(event);
    mume_delete(win);
    test_assert(_event_checker_check(
        0,
        mume_make_destroy_event(c4, c4),
        mume_make_destroy_event(c1, c4),
        mume_make_destroy_event(c3, c3),
        mume_make_destroy_event(c2, c3),
        mume_make_destroy_event(c2, c2),
        mume_make_destroy_event(c1, c2),
        mume_make_destroy_event(c1, c1),
        mume_make_destroy_event(win, c1),
        mume_make_destroy_event(win, win),
        mume_make_empty_event()));
    /* After destroy a window, all the events related to
       that window must be cleaned.*/
    while (mume_peek_event(&event, 1))
        mume_disp_event(&event);
}

static void _test_map_unmap(void)
{
    void *win;
    void *c1, *c2, *c3;
    win = _create_event_check_window(
        mume_root_window(), 0, 0, 100, 100);
    c1 = _create_event_check_window(win, 0, 0, 0, 0);
    c2 = _create_event_check_window(win, 20, 30, 40, 50);
    c3 = _create_event_check_window(win, 10, 10, 10, 10);
    /* Clear create events. */
    mume_list_clear(event_list);
    mume_window_map(win);
    mume_window_unmap(win);
    test_assert(_event_checker_check(
        0,
        mume_make_map_event(win, win),
        mume_make_unmap_event(win, win),
        mume_make_empty_event()));
    mume_map_children(win);
    test_assert(_event_checker_check(
        0,
        mume_make_map_event(c1, c1),
        mume_make_map_event(win, c1),
        mume_make_map_event(c2, c2),
        mume_make_map_event(win, c2),
        mume_make_map_event(c3, c3),
        mume_make_map_event(win, c3),
        mume_make_empty_event()));
    mume_unmap_children(win);
    test_assert(_event_checker_check(
        0,
        mume_make_unmap_event(c3, c3),
        mume_make_unmap_event(win, c3),
        mume_make_unmap_event(c2, c2),
        mume_make_unmap_event(win, c2),
        mume_make_unmap_event(c1, c1),
        mume_make_unmap_event(win, c1),
        mume_make_empty_event()));
    mume_delete(win);
}

static void _test_move_resize(void)
{
    void *win, *c1;
    win = _create_event_check_window(
        mume_root_window(), 10, 11, 100, 101);
    c1 = _create_event_check_window(win, 20, 30, 150, 180);
    /* Clear create events. */
    mume_list_clear(event_list);
    mume_window_move_to(win, -40, -45);
    test_assert(_event_checker_check(
        0,
        mume_make_move_event(win, win, -40, -45, 10, 11),
        mume_make_empty_event()));
    mume_window_move_to(c1, -10, -20);
    test_assert(_event_checker_check(
        0,
        mume_make_move_event(c1, c1, -10, -20, 20, 30),
        mume_make_move_event(win, c1, -10, -20, 20, 30),
        mume_make_empty_event()));
    mume_window_move_by(win, 70, 95);
    test_assert(_event_checker_check(
        0,
        mume_make_move_event(win, win, 30, 50, -40, -45),
        mume_make_empty_event()));
    mume_window_resize_to(win, -200, -205);
    test_assert(_event_checker_check(
        0,
        mume_make_resize_event(win, win, -200, -205, 100, 101),
        mume_make_empty_event()));
    mume_window_resize_by(win, 500, 515);
    test_assert(_event_checker_check(
        0,
        mume_make_resize_event(win, win, 300, 310, -200, -205),
        mume_make_empty_event()));
    mume_window_set_geometry(win, 10, 20, 200, 100);
    test_assert(_event_checker_check(
        0,
        mume_make_move_event(win, win, 10, 20, 30, 50),
        mume_make_resize_event(win, win, 200, 100, 300, 310),
        mume_make_empty_event()));
    mume_window_set_geometry(c1, 30, 10, 50, 40);
    test_assert(_event_checker_check(
        0,
        mume_make_move_event(c1, c1, 30, 10, -10, -20),
        mume_make_move_event(win, c1, 30, 10, -10, -20),
        mume_make_resize_event(c1, c1, 50, 40, 150, 180),
        mume_make_resize_event(win, c1, 50, 40, 150, 180),
        mume_make_empty_event()));
    mume_delete(win);
}

void all_tests(void)
{
    event_list = mume_list_new(NULL, NULL);
    test_run(_test_create_window);
    test_run(_test_create_children);
    test_run(_test_map_unmap);
    test_run(_test_move_resize);
    mume_list_delete(event_list);
}
