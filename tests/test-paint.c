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
#include "mume-gui.h"
#include "test-util.h"

static void *g_win;
static void *g_c1, *g_c2, *g_c3, *g_c4, *g_c5;

static void _map_all(void)
{
    mume_window_map(g_win);
    mume_map_children(g_win);
    mume_window_map(g_c4);
}

static void _unmap_all(void)
{
    mume_window_unmap(g_win);
    mume_unmap_children(g_win);
    mume_window_unmap(g_c4);
}

static void _reset_pos(void)
{
    mume_window_set_geometry(
        g_win, 10, 10, 100, 100);
    mume_window_set_geometry(
        g_c1, 10, 20, 40, 40);
    mume_window_set_geometry(
        g_c2, -10, -20, 40, 60);
    mume_window_set_geometry(
        g_c3, 70, 60, 40, 50);
    mume_window_set_geometry(
        g_c4, -10, -10, 30, 40);
    mume_window_set_geometry(
        g_c5, 20, 30, 60, 50);
}

static int _check_update_regions(
    void *win, const mume_rect_t *rects, int count)
{
    const cairo_region_t *urgn;
    cairo_region_t *rgn;
    int result = 0;
    urgn = mume_window_get_urgn(win);
    if (NULL == urgn)
        return 0 == count;
    rgn = cairo_region_create_rectangles(rects, count);
    if (cairo_region_equal(urgn, rgn))
        result = 1;
    cairo_region_destroy(rgn);
    return result;
}

static int _check_update_region1(
    void *win, int xx1, int yy1, int ww1, int hh1)
{
    mume_rect_t rect;
    rect.x = xx1;
    rect.y = yy1;
    rect.width = ww1;
    rect.height = hh1;
    return _check_update_regions(win, &rect, 1);
}

static int _check_update_region2(
    void *win,
    int xx1, int yy1, int ww1, int hh1,
    int xx2, int yy2, int ww2, int hh2)
{
    mume_rect_t rects[2];
    rects[0].x = xx1;
    rects[0].y = yy1;
    rects[0].width = ww1;
    rects[0].height = hh1;
    rects[1].x = xx2;
    rects[1].y = yy2;
    rects[1].width = ww2;
    rects[1].height = hh2;
    return _check_update_regions(win, rects, 2);
}

static int _check_update_region4(
    void *win,
    int xx1, int yy1, int ww1, int hh1,
    int xx2, int yy2, int ww2, int hh2,
    int xx3, int yy3, int ww3, int hh3,
    int xx4, int yy4, int ww4, int hh4)
{
    mume_rect_t rects[4];
    rects[0].x = xx1;
    rects[0].y = yy1;
    rects[0].width = ww1;
    rects[0].height = hh1;
    rects[1].x = xx2;
    rects[1].y = yy2;
    rects[1].width = ww2;
    rects[1].height = hh2;
    rects[2].x = xx3;
    rects[2].y = yy3;
    rects[2].width = ww3;
    rects[2].height = hh3;
    rects[3].x = xx4;
    rects[3].y = yy4;
    rects[3].width = ww4;
    rects[3].height = hh4;
    return _check_update_regions(win, rects, 4);
}

static int _check_update_region6(
    void *win,
    int xx1, int yy1, int ww1, int hh1,
    int xx2, int yy2, int ww2, int hh2,
    int xx3, int yy3, int ww3, int hh3,
    int xx4, int yy4, int ww4, int hh4,
    int xx5, int yy5, int ww5, int hh5,
    int xx6, int yy6, int ww6, int hh6)
{
    mume_rect_t rects[6];
    rects[0].x = xx1;
    rects[0].y = yy1;
    rects[0].width = ww1;
    rects[0].height = hh1;
    rects[1].x = xx2;
    rects[1].y = yy2;
    rects[1].width = ww2;
    rects[1].height = hh2;
    rects[2].x = xx3;
    rects[2].y = yy3;
    rects[2].width = ww3;
    rects[2].height = hh3;
    rects[3].x = xx4;
    rects[3].y = yy4;
    rects[3].width = ww4;
    rects[3].height = hh4;
    rects[4].x = xx5;
    rects[4].y = yy5;
    rects[4].width = ww5;
    rects[4].height = hh5;
    rects[5].x = xx6;
    rects[5].y = yy6;
    rects[5].width = ww6;
    rects[5].height = hh6;
    return _check_update_regions(win, rects, 6);
}

static void _color_window_handle_event(void *self, mume_event_t *evt)
{
    switch (evt->type) {
    case MUME_EVENT_BUTTONDOWN:
        {
            mume_rect_t rect;
            rect.x = evt->button.x - 40;
            rect.y = evt->button.y - 40;
            rect.width = 80;
            rect.height = 80;
            mume_invalidate_rect(
                evt->button.window, &rect);
            rect.x += rect.width;
            rect.y += rect.height;
            mume_invalidate_rect(
                evt->button.window, &rect);
            mume_validate_rect(
                evt->button.window, &rect);
        }
        break;

    case MUME_EVENT_EXPOSE:
        {
            cairo_t *cr = mume_window_begin_paint(
                evt->expose.window, MUME_PM_INVALID);

            if (cr) {
                static double colors[][3] = {
                    {0.6, 0.2, 0.2},
                    {0.3, 0.3, 0.3},
                    {0.6, 0.6, 0.6},
                    {0.2, 0.3, 0.5},
                    {0.6, 0.3, 0.6},
                    {0.1, 0.6, 0.2},
                    {0.4, 0.2, 0.1},
                    {0.2, 0.1, 0.4}
                };
                static size_t clridx = 0;
                cairo_set_source_rgb(
                    cr,
                    colors[clridx][0],
                    colors[clridx][1],
                    colors[clridx][2]);
                if (0 == evt->expose.count) {
                    if (++clridx >= COUNT_OF(colors))
                        clridx = 0;
                }
                cairo_rectangle(
                    cr, evt->expose.x, evt->expose.y,
                    evt->expose.width, evt->expose.height);
                cairo_fill(cr);
                mume_window_end_paint(evt->expose.window, cr);
            }
        }
        break;
    }
}

static const void* _color_window_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_window_meta_class(),
        "color window",
        mume_window_class(),
        MUME_SIZEOF_WINDOW,
        MUME_PROP_END,
        _mume_window_handle_event,
        _color_window_handle_event,
        MUME_FUNC_END);
}

static void* _color_window_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(_color_window_class(),
                    parent, x, y, width, height);
}

static void _test_empty_region(void)
{
    test_assert(_check_update_regions(g_win, NULL, 0));
    test_assert(_check_update_regions(g_c1, NULL, 0));
    test_assert(_check_update_regions(g_c2, NULL, 0));
    test_assert(_check_update_regions(g_c3, NULL, 0));
    test_assert(_check_update_regions(g_c4, NULL, 0));
    test_assert(_check_update_regions(g_c5, NULL, 0));
}

static void _test_initial_region(void)
{
    test_assert(_check_update_region6(
        g_win,
        30, 0, 70, 20,
        50, 20, 50, 10,
        0, 40, 10, 20,
        80, 30, 20, 30,
        0, 60, 20, 20,
        0, 80, 70, 20));
    test_assert(_check_update_region2(
        g_c1,
        20, 0, 20, 10,
        0, 20, 10, 20));
    test_assert(_check_update_region2(
        g_c2,
        10, 20, 20, 40,
        30, 20, 10, 30));
    test_assert(_check_update_region2(
        g_c3,
        0, 30, 30, 10,
        20, 0, 10, 30));
    test_assert(_check_update_region2(
        g_c4,
        10, 30, 20, 10,
        20, 10, 10, 30));
    test_assert(_check_update_region1(
        g_c5, 0, 0, 60, 50));
}

static void _test_map_unmap(void)
{
    _unmap_all();
    _reset_pos();
    mume_window_map(g_c1);
    mume_window_map(g_c2);
    mume_window_map(g_c5);
    _test_empty_region();
    mume_window_map(g_win);
    mume_window_map(g_c3);
    mume_window_map(g_c4);
    _test_initial_region();
    mume_window_unmap(g_win);
    _test_empty_region();
}

static void _test_move_resize(void)
{
    _unmap_all();
    _map_all();
    _reset_pos();
    _test_initial_region();
    mume_window_move_by(g_c2, 10, 10);
    test_assert(_check_update_region6(
        g_win,
        40, 0, 60, 20,
        50, 20, 50, 10,
        0, 50, 10, 10,
        80, 30, 20, 30,
        0, 60, 20, 20,
        0, 80, 70, 20));
    test_assert(_check_update_region2(
        g_c1,
        30, 0, 10, 10,
        0, 30, 10, 10));
    test_assert(_check_update_region2(
        g_c2,
        0, 10, 40, 30,
        0, 40, 20, 20));
    mume_window_move_by(g_c2, -10, -10);
    mume_window_move_by(g_c1, 10, -10);
    mume_window_resize_by(g_c1, 10, 0);
    test_assert(_check_update_region1(
        g_c1,
        10, 0, 40, 20));
    mume_window_move_by(g_c1, -10, 10);
    mume_window_resize_by(g_c1, -10, 0);
    _test_initial_region();
    mume_window_set_geometry(g_c3, 0, 0, 100, 100);
    test_assert(_check_update_regions(g_win, NULL, 0));
    test_assert(_check_update_regions(g_c1, NULL, 0));
    test_assert(_check_update_regions(g_c2, NULL, 0));
    test_assert(_check_update_region4(g_c3,
                                   20, 0, 80, 30,
                                   0, 30, 20, 70,
                                   80, 30, 20, 70,
                                   20, 80, 60, 20));
    test_assert(_check_update_region1(g_c4, 10, 10, 20, 30));
    test_assert(_check_update_region1(g_c5, 0, 0, 60, 50));
    mume_window_set_geometry(g_c3, 70, 60, 40, 50);
    _test_initial_region();
}

static void _test_stack_order(void)
{
    void *wins[4];
    _unmap_all();
    _map_all();
    _reset_pos();
    _test_initial_region();
    mume_window_raise(g_c1);
    test_assert(_check_update_region6(
        g_win,
        30, 0, 70, 20,
        50, 20, 50, 10,
        0, 40, 10, 20,
        80, 30, 20, 30,
        0, 60, 20, 20,
        0, 80, 70, 20));
    test_assert(_check_update_region1(
        g_c1, 0, 0, 40, 40));
    test_assert(_check_update_region2(
        g_c2,
        10, 20, 30, 20,
        10, 40, 10, 20));
    test_assert(_check_update_region2(
        g_c3,
        0, 30, 30, 10,
        20, 0, 10, 30));
    test_assert(_check_update_region2(
        g_c4,
        10, 30, 20, 10,
        20, 10, 10, 30));
    test_assert(_check_update_region2(
        g_c5,
        0, 30, 30, 20,
        30, 0, 30, 50));
    mume_window_lower(g_c1);
    _test_initial_region();
    mume_window_lower(g_c5);
    test_assert(_check_update_region2(
        g_c1,
        20, 0, 20, 40,
        0, 20, 20, 20));
    test_assert(_check_update_region1(
        g_c2, 10, 20, 30, 40));
    mume_window_lower(g_c2);
    test_assert(_check_update_region1(
        g_c1, 0, 0, 40, 40));
    test_assert(_check_update_region2(
        g_c2,
        10, 20, 30, 20,
        10, 40, 10, 20));
    test_assert(_check_update_region6(
        g_win,
        30, 0, 70, 20,
        50, 20, 50, 10,
        0, 40, 10, 20,
        80, 30, 20, 30,
        0, 60, 20, 20,
        0, 80, 70, 20));
    test_assert(_check_update_region2(
        g_c3,
        0, 30, 30, 10,
        20, 0, 10, 30));
    test_assert(_check_update_region1(
        g_c4, 10, 10, 20, 30));
    test_assert(_check_update_region2(
        g_c5,
        30, 0, 30, 30,
        0, 30, 50, 20));
    mume_window_lower(g_c1);
    mume_window_raise(g_c5);
    _test_initial_region();
    wins[0] = g_c1;
    wins[1] = g_c2;
    wins[2] = g_c3;
    wins[3] = g_c5;
    mume_window_restack(wins, 4);
    test_assert(_check_update_region1(
        g_c1, 0, 0, 40, 40));
    test_assert(_check_update_region2(
        g_c2,
        10, 20, 30, 20,
        10, 40, 10, 20));
    test_assert(_check_update_region2(
        g_c3,
        0, 30, 30, 10,
        20, 0, 10, 30));
    test_assert(_check_update_region1(
        g_c4, 10, 10, 20, 30));
    test_assert(_check_update_region2(
        g_c5,
        30, 0, 30, 30,
        0, 30, 50, 20));
}

static void _test_color_window(void)
{
    mume_window_set_geometry(
        g_win, 0, 0, 800, 600);
    mume_window_set_geometry(
        g_c1, 80, 100, 300, 300);
    mume_window_set_geometry(
        g_c2, -100, -200, 300, 400);
    mume_window_set_geometry(
        g_c3, 500, 300, 400, 350);
    mume_window_set_geometry(
        g_c4, -100, -200, 300, 400);
    mume_window_set_geometry(
        g_c5, 150, 150, 400, 300);
    _map_all();
    mume_window_center(g_win, mume_root_window());
    test_run_event_loop(g_win);
}

void all_tests(void)
{
    g_win = _color_window_new(
        mume_root_window(), 0, 0, 0, 0);
    g_c1 = _color_window_new(g_win, 0, 0, 0, 0);
    g_c2 = _color_window_new(g_win, 0, 0, 0, 0);
    g_c3 = _color_window_new(g_win, 0, 0, 0, 0);
    g_c4 = _color_window_new(g_c3, 0, 0, 0, 0);
    g_c5 = _color_window_new(g_win, 0, 0, 0, 0);
    test_run(_test_map_unmap);
    test_run(_test_move_resize);
    test_run(_test_stack_order);
    test_run(_test_color_window);
    mume_delete(g_win);
    test_run(_test_empty_region);
}
