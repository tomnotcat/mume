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

static const char *_res_path = TESTS_DATA_DIR "/resmgr";
static const char *_font_path = TESTS_THEME_DIR "/default";

static void _test_resmgr_basic(void)
{
    mume_resmgr_t *mgr;
    struct {
        const char *ns;
        const char *nm;
        mume_color_t clr;
    } clrtab[] = {
        {":mycolor", "red", {255, 0, 0}},
        {":mycolor", "green", {0, 255, 0}},
        {"mycolor", "blue", {0, 0, 255}},
        {"mycolor", "trans", {255, 0, 255}}
    };
    struct {
        const char *ns;
        const char *nm;
        mume_rect_t r;
        mume_rect_t m;
    } imgtab[] = {
        {"myimage", "button.normal",
         {0, 0, 20, 23}, {8, 8, 8, 8}},
        {":myimage", "button.hot",
         {0, 23, 20, 23}, {8, 8, 8, 8}},
        {":myimage", "button.pressed",
         {0, 46, 20, 23}, {8, 8, 8, 8}},
        {"myimage", "button.disabled",
         {0, 69, 20, 23}, {8, 8, 8, 8}},
        {"myimage", "button.focused",
         {0, 92, 20, 23}, {8, 8, 8, 8}},
    };
    struct {
        const char *ns;
        const char *nm;
        int size;
    } cftab[] = {
        {"mycharformat", "format1", 13},
        {"mycharformat", "format2", 23},
        {"mycharformat", "format3", 33},
    };
    int i;
    mgr = mume_resmgr_new();
    test_assert(mume_resmgr_load(mgr, _res_path, "main.xml"));
    test_assert(mume_resmgr_load(mgr, _font_path, NULL));
    for (i = 0; i < COUNT_OF(clrtab); ++i) {
        mume_color_t *clr = mume_resmgr_get_color(
            mgr, clrtab[i].ns, clrtab[i].nm);
        test_assert(clr && mume_color_getuint(clr) ==
                  mume_color_getuint(&clrtab[i].clr));
        /* namespace not set properly */
        test_assert(NULL == mume_resmgr_get_color(
            mgr, NULL, clrtab[i].nm));
    }

    for (i = 0; i < 1; ++i) {
        mume_resobj_image_t *img = mume_resmgr_get_image(
            mgr, imgtab[i].ns, imgtab[i].nm);
        test_assert(img && img->p0);
        test_assert(img->rect.x == imgtab[i].r.x &&
                  img->rect.y == imgtab[i].r.y &&
                  img->rect.width == imgtab[i].r.width &&
                  img->rect.height == imgtab[i].r.height);
        test_assert(img->margin.x == imgtab[i].m.x &&
                  img->margin.y == imgtab[i].m.y &&
                  img->margin.width == imgtab[i].m.width &&
                  img->margin.height == imgtab[i].m.height);
        test_assert(NULL == mume_resmgr_get_image(
            mgr, NULL, imgtab[i].nm));
    }

    for (i = 0; i < COUNT_OF(cftab); ++i) {
        mume_resobj_charfmt_t *cf = mume_resmgr_get_charfmt(
            mgr, cftab[i].ns, cftab[i].nm);
        test_assert(cf && cf->p);
        test_assert(cf->size == cftab[i].size);
    }
    mume_resmgr_delete(mgr);
}

static void _test_resmgr_paint(void)
{
    void *win;
    mume_event_t evt;
    mume_resmgr_t *mgr;
    mume_resobj_brush_t *br;
    mume_resobj_charfmt_t *cf;
    const char *brnm[] = {
        "clr1", "clr2", "clr3", "clr4",
        "png1", "png2", "png3", "png4",
        "bmp1", "bmp2", "bmp3", "bmp4",
    };
    const char *cfnm[] = {
        "format1", "format2", "format3",
        "format1", "format2", "format2",
        "format1", "format2",
    };
    unsigned int flags[] = {
        0, MUME_TLF_VCENTER, MUME_TLF_BOTTOM,
        MUME_TLF_VCENTER | MUME_TLF_CENTER,
        MUME_TLF_VCENTER | MUME_TLF_RIGHT,
        MUME_TLF_BOTTOM | MUME_TLF_CENTER,
        MUME_TLF_BOTTOM | MUME_TLF_RIGHT,
        MUME_TLF_VCENTER | MUME_TLF_CENTER | MUME_TLF_SINGLELINE,
    };
    const char *texts[] = {
        "hello\nworld", "hello\nworld", "hello\nworld",
        "hello\nworld, \nsome dummy long text for test",
        "hello\nworld", "hello\nworld", "hello\nworld",
        "hello\nworld, \nsome dummy long text for test",
    };
    mgr = mume_resmgr_new();
    test_assert(mume_resmgr_load(mgr, _res_path, "main.xml"));
    test_assert(mume_resmgr_load(mgr, _font_path, NULL));
    win = mume_window_new(mume_root_window(), 0, 0, 800, 600);
    mume_window_map(win);
    mume_window_center(win, mume_root_window());
    test_setup_toplevel_window(win);

    while (mume_wait_event(&evt)) {
        if (test_is_break_event(&evt, win))
            break;

        if (MUME_EVENT_EXPOSE == evt.type &&
            evt.any.window == win)
        {
            cairo_t *cr;
            int width, height;
            int w, i, c = 4;
            mume_rect_t rect;
            cr = mume_window_begin_paint(win, MUME_PM_INVALID);
            mume_window_get_geometry(win, NULL, NULL, &width, &height);
            w = width / c;
            cairo_rectangle(cr, 0, 0, width, height);
            cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
            cairo_fill(cr);
            for (i = 0; i < COUNT_OF(brnm); ++i) {
                br = mume_resmgr_get_brush(
                    mgr, "mybrush", brnm[i]);
                test_assert(br);
                mume_draw_resobj_brush(
                    cr, br, i % c * w + 1, i / c * w + 1, w - 2, w - 2);
            }

            for (i = 0; i < COUNT_OF(cfnm); ++i) {
                cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
                /* Horizontal middle line. */
                cairo_move_to(cr, i % c * w + 1, i / c * w + w / 2);
                cairo_line_to(cr, i % c * w + w - 1, i / c * w + w / 2);
                /* Vertical middle line. */
                cairo_move_to(cr, i % c * w + w / 2, i / c * w + 1);
                cairo_line_to(cr, i % c * w + w / 2, i / c * w + w - 1);
                cairo_stroke(cr);
                cf = mume_resmgr_get_charfmt(
                    mgr, "mycharformat", cfnm[i]);
                test_assert(cf && cf->p);
                rect = mume_rect_make(
                    i % c * w + 1, i / c * w + 1, w - 2, w - 2);
                mume_charfmt_draw_text(
                    cr, cf,
                    flags[i] | MUME_TLF_CALCRECT | MUME_TLF_DRAWTEXT,
                    texts[i], -1, &rect);
                cairo_rectangle(
                    cr, rect.x, rect.y, rect.width, rect.height);
                cairo_set_source_rgb(cr, 0, 0, 0);
                cairo_stroke(cr);
            }

            mume_window_end_paint(win, cr);
        }
        else {
            mume_disp_event(&evt);
        }
    }
    mume_resmgr_delete(mgr);
}

void all_tests(void)
{
    test_run(_test_resmgr_basic);
    test_run(_test_resmgr_paint);
}
