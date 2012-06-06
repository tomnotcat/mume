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
#include "mume-geometry.h"
#include MUME_FLOAT_H
#include MUME_MATH_H

#define MAX4(a,b,c,d) MAX(MAX(a,b), MAX(c,d))
#define MIN4(a,b,c,d) MIN(MIN(a,b), MIN(c,d))

const mume_matrix_t mume_matrix_identity = { 1, 0, 0, 1, 0, 0 };
const mume_rect_t mume_rect_empty = { 0, 0, 0, 0 };
const mume_rect_t mume_rect_infinite = { 0, 0, -1, -1 };

mume_matrix_t mume_matrix_concat(
    mume_matrix_t one, mume_matrix_t two)
{
    mume_matrix_t dst;
    dst.a = one.a * two.a + one.b * two.c;
    dst.b = one.a * two.b + one.b * two.d;
    dst.c = one.c * two.a + one.d * two.c;
    dst.d = one.c * two.b + one.d * two.d;
    dst.e = one.e * two.a + one.f * two.c + two.e;
    dst.f = one.e * two.b + one.f * two.d + two.f;
    return dst;
}

mume_matrix_t mume_matrix_scale(float sx, float sy)
{
    mume_matrix_t m;
    m.a = sx; m.b = 0;
    m.c = 0; m.d = sy;
    m.e = 0; m.f = 0;
    return m;
}

mume_matrix_t mume_matrix_shear(float h, float v)
{
    mume_matrix_t m;
    m.a = 1; m.b = v;
    m.c = h; m.d = 1;
    m.e = 0; m.f = 0;
    return m;
}

mume_matrix_t mume_matrix_rotate(float theta)
{
    mume_matrix_t m;
    float s;
    float c;

    while (theta < 0)
        theta += 360;

    while (theta >= 360)
        theta -= 360;

    if (fabsf(0 - theta) < FLT_EPSILON) {
        s = 0;
        c = 1;
    }
    else if (fabsf(90.0f - theta) < FLT_EPSILON) {
        s = 1;
        c = 0;
    }
    else if (fabsf(180.0f - theta) < FLT_EPSILON) {
        s = 0;
        c = -1;
    }
    else if (fabsf(270.0f - theta) < FLT_EPSILON) {
        s = -1;
        c = 0;
    }
    else {
        s = sinf(theta * (float)M_PI / 180);
        c = cosf(theta * (float)M_PI / 180);
    }

    m.a = c; m.b = s;
    m.c = -s; m.d = c;
    m.e = 0; m.f = 0;
    return m;
}

mume_matrix_t mume_matrix_translate(float tx, float ty)
{
    mume_matrix_t m;
    m.a = 1; m.b = 0;
    m.c = 0; m.d = 1;
    m.e = tx; m.f = ty;
    return m;
}

mume_matrix_t mume_matrix_invert(mume_matrix_t src)
{
    float det = src.a * src.d - src.b * src.c;
    if (det < -FLT_EPSILON || det > FLT_EPSILON) {
        mume_matrix_t dst;
        float rdet = 1 / det;
        dst.a = src.d * rdet;
        dst.b = -src.b * rdet;
        dst.c = -src.c * rdet;
        dst.d = src.a * rdet;
        dst.e = -src.e * dst.a - src.f * dst.c;
        dst.f = -src.e * dst.b - src.f * dst.d;
        return dst;
    }

    return src;
}

int mume_matrix_is_rectilinear(mume_matrix_t m)
{
    return (fabsf(m.b) < FLT_EPSILON && fabsf(m.c) < FLT_EPSILON) ||
            (fabsf(m.a) < FLT_EPSILON && fabsf(m.d) < FLT_EPSILON);
}

cairo_matrix_t mume_matrix_to_cairo(mume_matrix_t m)
{
    cairo_matrix_t cm;
    cm.xx = m.a; cm.yx = m.b;
    cm.xy = m.c; cm.yy = m.d;
    cm.x0 = m.e; cm.y0 = m.f;
    return cm;
}

mume_point_t mume_point_transform(mume_point_t p, mume_matrix_t m)
{
    mume_point_t t;
    t.x = p.x * m.a + p.y * m.c + m.e;
    t.y = p.x * m.b + p.y * m.d + m.f;
    return t;
}

mume_point_t mume_point_from_string(const char *str)
{
    mume_point_t t;

    t.x = strtol(str, (char**)&str, 10);
    if (*str)
        t.y = strtol(str + 1, (char**)&str, 10);
    else
        t.y = 0;

    return t;
}

char* mume_point_to_string(mume_point_t p, char *buf, size_t len)
{
    snprintf(buf, len, "%d, %d", p.x, p.y);
    return buf;
}

mume_rect_t mume_rect_make(int x, int y, int w, int h)
{
    mume_rect_t r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    return r;
}

int mume_rect_is_valid(mume_rect_t r)
{
    return r.width >= 0 && r.height >= 0;
}

int mume_rect_is_empty(mume_rect_t r)
{
    return 0 == r.width || 0 == r.height;
}

int mume_rect_is_null(mume_rect_t r)
{
    return 0 == r.x && 0 == r.y &&
            0 == r.width && 0 == r.height;
}

int mume_rect_is_infinite(mume_rect_t r)
{
    return r.width < 0;
}

int mume_rect_equal(mume_rect_t r1, mume_rect_t r2)
{
    return (r1.x == r2.x && r1.width == r2.width &&
            r1.y == r2.y && r1.height == r2.height);
}

int mume_rect_inside(mume_rect_t rect, int x, int y)
{
    return (x >= rect.x && x < (rect.x + rect.width) &&
            y >= rect.y && y < (rect.y + rect.height));
}

mume_rect_t mume_rect_transform(mume_rect_t r, mume_matrix_t m)
{
    mume_point_t s, t, u, v;
    if (mume_rect_is_infinite(r))
        return r;

    s.x = r.x; s.y = r.y;
    t.x = r.x; t.y = r.y + r.height;
    u.x = r.x + r.width; u.y = r.y + r.height;
    v.x = r.x + r.width; v.y = r.y;
    s = mume_point_transform(s, m);
    t = mume_point_transform(t, m);
    u = mume_point_transform(u, m);
    v = mume_point_transform(v, m);
    r.x = MIN4(s.x, t.x, u.x, v.x);
    r.y = MIN4(s.y, t.y, u.y, v.y);
    r.width = MAX4(s.x, t.x, u.x, v.x) - r.x;
    r.height = MAX4(s.y, t.y, u.y, v.y) - r.y;
    return r;
}

mume_rect_t mume_rect_inflate(
    mume_rect_t rect, int left, int top, int right, int bottom)
{
    rect.x -= left;
    rect.width += left + right;
    rect.y -= top;
    rect.height += top + bottom;
    return rect;
}

mume_rect_t mume_rect_intersect(mume_rect_t a, mume_rect_t b)
{
    mume_rect_t r;
    if (mume_rect_is_infinite(a)) return b;
    if (mume_rect_is_infinite(b)) return a;
    if (mume_rect_is_empty(a)) return mume_rect_empty;
    if (mume_rect_is_empty(b)) return mume_rect_empty;
    r.x = MAX(a.x, b.x);
    r.y = MAX(a.y, b.y);
    r.width = MIN(a.x + a.width, b.x + b.width) - r.x;
    r.height = MIN(a.y + a.height, b.y + b.height) - r.y;
    return (r.width < 0 || r.height < 0) ? mume_rect_empty : r;
}

mume_rect_t mume_rect_union(mume_rect_t a, mume_rect_t b)
{
    mume_rect_t r;
    if (mume_rect_is_infinite(a)) return a;
    if (mume_rect_is_infinite(b)) return b;
    if (mume_rect_is_empty(a)) return b;
    if (mume_rect_is_empty(b)) return a;
    r.x = MIN(a.x, b.x);
    r.y = MIN(a.y, b.y);
    r.width = MAX(a.x + a.width, b.x + b.width) - r.x;
    r.height = MAX(a.y + a.height, b.y + b.height) - r.y;
    return r;
}
