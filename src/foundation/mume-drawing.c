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
#include "mume-drawing.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-resmgr-private.h"
#include "mume-debug.h"
#include "mume-text-layout.h"
#include MUME_ASSERT_H

static inline void _mume_bitblt_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2)
{
    /* Similar to BitBlt in win32 GDI. */
    if (w1 > 0 && h1 > 0) {
        cairo_matrix_t matrix;
        cairo_matrix_init_translate(
            &matrix, x2 - x1, y2 - y1);
        cairo_pattern_set_matrix(p, &matrix);
        cairo_rectangle(cr, x1, y1, w1, h1);
        cairo_fill(cr);
    }
}

static inline void _mume_stretch_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2, int w2, int h2)
{
    /* Similar to StretchBlt in win32 GDI. */
    if (w1 > 0 && h1 > 0 && w2 > 0 && h2 > 0) {
        cairo_matrix_t matrix;
        double hr = (double)w2 / (double)w1;
        double vr = (double)h2 / (double)h1;
        cairo_matrix_init_translate(
            &matrix, (double)x2 - x1 * hr, (double)y2 - y1 * vr);
        cairo_matrix_scale(&matrix, hr, vr);
        cairo_pattern_set_matrix(p, &matrix);
        cairo_rectangle(cr, x1, y1, w1, h1);
        cairo_fill(cr);
    }
}

void mume_bitblt_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2)
{
    cairo_matrix_t om;
    cairo_pattern_get_matrix(p, &om);
    cairo_save(cr);
    cairo_new_path(cr);
    _mume_bitblt_image(cr, x1, y1, w1, h1, p, x2, y2);
    cairo_restore(cr);
    cairo_pattern_set_matrix(p, &om);
}

void mume_stretch_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2, int w2, int h2)
{
    cairo_matrix_t om;
    cairo_pattern_get_matrix(p, &om);
    cairo_save(cr);
    cairo_new_path(cr);
    _mume_stretch_image(
        cr, x1, y1, w1, h1, p, x2, y2, w2, h2);
    cairo_restore(cr);
    cairo_pattern_set_matrix(p, &om);
}

void mume_draw_resobj_image(
    cairo_t *cr, mume_resobj_image_t *img,
    int x, int y, int width, int height)
{
    cairo_pattern_t *p;
    const mume_rect_t *r = &img->rect;
    const mume_rect_t *m = &img->margin;

    assert(img->p0);

    p = cairo_pattern_create_for_surface(img->p0);
    cairo_save(cr);
    cairo_new_path(cr);
    cairo_set_source(cr, p);
    if ((width <= r->width && height <= r->height)
        || mume_rect_is_null(*m))
    {
        _mume_stretch_image(
            cr, x, y, width, height,
            p, r->x, r->y, r->width, r->height);
    }
    else {
        /* top-left corner */
        _mume_bitblt_image(
            cr, x, y, m->x, m->y, p, r->x, r->y);

        /* top-right corner */
        _mume_bitblt_image(
            cr, x + width - m->width, y, m->width, m->y,
            p, r->x + r->width - m->width, r->y);

        /* bottom-left corner */
        _mume_bitblt_image(
            cr, x, y + height - m->height, m->x, m->height,
            p, r->x, r->y + r->height - m->height);

        /* bottom-right corner */
        _mume_bitblt_image(
            cr, x + width - m->width,
            y + height - m->height,
            m->width, m->height,
            p, r->x + r->width - m->width,
            r->y + r->height - m->height);

        /* top side */
        _mume_stretch_image(
            cr, x + m->x, y,
            width - m->x - m->width, m->y,
            p, r->x + m->x, r->y,
            r->width - m->x - m->width, m->y);

        /* bottom side */
        _mume_stretch_image(
            cr, x + m->x, y + height - m->height,
            width - m->x - m->width, m->height,
            p, r->x + m->x, r->y + r->height - m->height,
            r->width - m->x - m->width, m->height);

        /* left side */
        _mume_stretch_image(
            cr, x, y + m->y,
            m->x, height - m->y - m->height,
            p, r->x, r->y + m->y,
            m->x, r->height - m->y - m->height);

        /* right side */
        _mume_stretch_image(
            cr, x + width - m->width, y + m->y,
            m->width, height - m->y - m->height,
            p, r->x + r->width - m->width, r->y + m->y,
            m->width, r->height - m->y - m->height);

        /* center stretch. */
        _mume_stretch_image(
            cr, x + m->x, y + m->y,
            width - m->x - m->width,
            height - m->y - m->height,
            p, r->x + m->x, r->y + m->y,
            r->width - m->x - m->width,
            r->height - m->y - m->height);
    }

    cairo_restore(cr);
    cairo_pattern_destroy(p);
}

void mume_resobj_brush_fill_preserve(
    cairo_t *cr, mume_resobj_brush_t *br)
{
    if (br->p) {
        double x1, y1, x2, y2;
        int x, y, width, height;
        cairo_fill_extents(cr, &x1, &y1, &x2, &y2);
        x = x1;
        y = y1;
        width = x2 - x1;
        height = y2 - y1;
        mume_draw_resobj_image(cr, br->p, x, y, width, height);
    }
    else {
        cairo_save(cr);
        cairo_set_source_rgb(
            cr, mume_color_rval(&br->color),
            mume_color_gval(&br->color),
            mume_color_bval(&br->color));
        cairo_fill_preserve(cr);
        cairo_restore(cr);
    }
}

void mume_resobj_brush_fill(
    cairo_t *cr, mume_resobj_brush_t *br)
{
    mume_resobj_brush_fill_preserve(cr, br);
    cairo_new_path(cr);
}

void mume_draw_resobj_brush(
    cairo_t *cr, mume_resobj_brush_t *br,
    int x, int y, int width, int height)
{
    if (br->p) {
        mume_draw_resobj_image(
            cr, br->p, x, y, width, height);
    }
    else {
        cairo_save(cr);
        cairo_set_source_rgb(
            cr, mume_color_rval(&br->color),
            mume_color_gval(&br->color),
            mume_color_bval(&br->color));
        cairo_rectangle(cr, x, y, width, height);
        cairo_fill(cr);
        cairo_restore(cr);
    }
}

cairo_font_extents_t mume_charfmt_font_extents(
    mume_resobj_charfmt_t *cf)
{
    cairo_font_extents_t fs_metrics = { 0, 0, 0, 0, 0 };
    if (mume_resobj_charfmt_is_valid(cf))
        fs_metrics = cairo_font_face_font_extents(cf->p->p, cf->size);

    return fs_metrics;
}

mume_point_t mume_charfmt_text_extents(
    mume_resobj_charfmt_t *cf, const char *text, int length)
{
    mume_point_t point = { 0, 0 };
    void *tl = mume_text_layout();

    if (mume_resobj_charfmt_is_valid(cf)) {
        mume_rect_t rect = mume_rect_empty;

        mume_text_layout_reset(tl);
        mume_text_layout_add_text(tl, cf->p->p, text, length);
        mume_text_layout_perform(
            tl, cf->size, NULL, &rect,
            MUME_TLF_CALCRECT | MUME_TLF_SINGLELINE);

        point.x = rect.width;
        point.y = rect.height;
    }

    return point;
}

void mume_charfmt_draw_text(
    cairo_t *cr, mume_resobj_charfmt_t *cf, unsigned int fmt,
    const char *text, int length, mume_rect_t *rect)
{
    void *tl = mume_text_layout();

    if (mume_resobj_charfmt_is_valid(cf)) {
        if (cr) {
            cairo_save(cr);

            cairo_set_source_rgb(
                cr, mume_color_rval(&cf->color),
                mume_color_gval(&cf->color),
                mume_color_bval(&cf->color));
        }

        mume_text_layout_reset(tl);
        mume_text_layout_add_text(tl, cf->p->p, text, length);
        mume_text_layout_perform(tl, cf->size, cr, rect, fmt);

        if (cr)
            cairo_restore(cr);
    }
}

void mume_cairo_region_to_path(cairo_t *cr, const cairo_region_t *rgn)
{
    mume_rect_t r;
    int i = cairo_region_num_rectangles(rgn);
    while (i-- > 0) {
        cairo_region_get_rectangle(rgn, i, &r);
        cairo_rectangle(cr, r.x, r.y, r.width, r.height);
    }
}

mume_rect_t mume_cairo_region_extents(const cairo_region_t *rgn)
{
    mume_rect_t r0, r1;
    int i = cairo_region_num_rectangles(rgn);

    r0 = mume_rect_empty;
    while (i-- > 0) {
        cairo_region_get_rectangle(rgn, i, &r1);
        r0 = mume_rect_union(r0, r1);
    }

    return r0;
}
