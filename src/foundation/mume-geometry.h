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
#ifndef MUME_FOUNDATION_GEOMETRY_H
#define MUME_FOUNDATION_GEOMETRY_H

#include "mume-common.h"

MUME_BEGIN_DECLS

/* The transformation matrix, from "mupdf". */
struct mume_matrix_s {
    float a, b, c, d, e, f;
};

struct mume_point_s {
    int x, y;
};

mume_public extern const mume_matrix_t mume_matrix_identity;
mume_public extern const mume_rect_t mume_rect_empty;
mume_public extern const mume_rect_t mume_rect_infinite;

mume_public mume_matrix_t mume_matrix_concat(
    mume_matrix_t left, mume_matrix_t right);

mume_public mume_matrix_t mume_matrix_scale(float sx, float sy);

mume_public mume_matrix_t mume_matrix_shear(float sx, float sy);

mume_public mume_matrix_t mume_matrix_rotate(float degrees);

mume_public mume_matrix_t mume_matrix_translate(float tx, float ty);

mume_public mume_matrix_t mume_matrix_invert(mume_matrix_t src);

mume_public int mume_matrix_is_rectilinear(mume_matrix_t m);

mume_public cairo_matrix_t mume_matrix_to_cairo(mume_matrix_t m);

mume_public mume_point_t mume_point_transform(
    mume_point_t p, mume_matrix_t m);

mume_public mume_point_t mume_point_from_string(const char *str);

mume_public char* mume_point_to_string(
    mume_point_t p, char *buf, size_t len);

mume_public mume_rect_t mume_rect_make(int x, int y, int w, int h);

mume_public int mume_rect_is_valid(mume_rect_t r);

mume_public int mume_rect_is_empty(mume_rect_t r);

mume_public int mume_rect_is_null(mume_rect_t r);

mume_public int mume_rect_is_infinite(mume_rect_t r);

mume_public int mume_rect_equal(mume_rect_t r1, mume_rect_t r2);

mume_public int mume_rect_inside(mume_rect_t rect, int x, int y);

mume_public mume_rect_t mume_rect_transform(
    mume_rect_t rect, mume_matrix_t matrix);

mume_public mume_rect_t mume_rect_inflate(
    mume_rect_t rect, int left, int top, int right, int bottom);

#define mume_rect_deflate(_rect, _left, _top, _right, _bottom) \
    mume_rect_inflate(_rect, -(_left), -(_top), -(_right), -(_bottom))

mume_public mume_rect_t mume_rect_intersect(mume_rect_t a, mume_rect_t b);

mume_public mume_rect_t mume_rect_union(mume_rect_t a, mume_rect_t b);

#define mume_rect_translate(_rect, _x, _y) \
    mume_rect_transform(_rect, mume_matrix_translate(_x, _y))

MUME_END_DECLS

#endif  /* MUME_FOUNDATION_GEOMETRY_H */
