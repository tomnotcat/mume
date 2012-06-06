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
#ifndef MUME_FOUNDATION_DRAWING_H
#define MUME_FOUNDATION_DRAWING_H

#include "mume-common.h"

MUME_BEGIN_DECLS

mume_public void mume_bitblt_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2);

mume_public void mume_stretch_image(
    cairo_t *cr, int x1, int y1, int w1, int h1,
    cairo_pattern_t *p, int x2, int y2, int w2, int h2);

mume_public void mume_draw_resobj_image(
    cairo_t *cr, mume_resobj_image_t *img,
    int x, int y, int width, int height);

mume_public void mume_resobj_brush_fill_preserve(
    cairo_t *cr, mume_resobj_brush_t *br);

mume_public void mume_resobj_brush_fill(
    cairo_t *cr, mume_resobj_brush_t *br);

mume_public void mume_draw_resobj_brush(
    cairo_t *cr, mume_resobj_brush_t *br,
    int x, int y, int width, int height);

mume_public cairo_font_extents_t mume_charfmt_font_extents(
    mume_resobj_charfmt_t *cf);

mume_public mume_point_t mume_charfmt_text_extents(
    mume_resobj_charfmt_t *cf, const char *text, int length);

mume_public void mume_charfmt_draw_text(
    cairo_t *cr, mume_resobj_charfmt_t *cf, unsigned int fmt,
    const char *text, int length, mume_rect_t *rect);

mume_public void mume_cairo_region_to_path(
    cairo_t *cr, const cairo_region_t *rgn);

mume_public mume_rect_t mume_cairo_region_extents(
    const cairo_region_t *rgn);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_DRAWING_H */
