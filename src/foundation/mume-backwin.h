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
#ifndef MUME_FOUNDATION_BACKWIN_H
#define MUME_FOUNDATION_BACKWIN_H

#include "mume-refobj.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_BACKWIN (MUME_SIZEOF_REFOBJ + \
                             sizeof(void*))

#define MUME_SIZEOF_BACKWIN_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                   sizeof(voidf*) * 14)

mume_public const void* mume_backwin_class(void);

mume_public const void* mume_backwin_meta_class(void);

/* Get/Set the window attached to this backwin. */
mume_public void mume_backwin_set_attached(void *self, void *window);

mume_public void* mume_backwin_get_attached(const void *self);

/* Selector for notify the backwin its attached window changed. */
mume_public void _mume_backwin_attach_changed(
    const void *clazz, void *self, void *window);

#define mume_backwin_attach_changed(_self, _window) \
    _mume_backwin_attach_changed(NULL, _self, _window)

/* Selector for check if the two underlying backwins equals. */
mume_public int _mume_backwin_equal(
    const void *clazz, void *self, const void *other);

#define mume_backwin_equal(_self, _other) \
    _mume_backwin_equal(NULL, _self, _other)

/* Selector for map the backwin. */
mume_public void _mume_backwin_map(const void *clazz, void *self);

#define mume_backwin_map(_self) _mume_backwin_map(NULL, _self)

/* Selector for unmap the backwin. */
mume_public void _mume_backwin_unmap(const void *clazz, void *self);

#define mume_backwin_unmap(_self) _mume_backwin_unmap(NULL, _self)

/* Selector for check whether the backwin is mapped. */
mume_public int _mume_backwin_is_mapped(
    const void *clazz, void *self);

#define mume_backwin_is_mapped(_self) \
    _mume_backwin_is_mapped(NULL, _self)

/* Selector for set/get the geometry of the backwin. */
mume_public void _mume_backwin_set_geometry(
    const void *clazz, void *self, int x, int y, int w, int h);

#define mume_backwin_set_geometry(_self, _x, _y, _w, _h) \
    _mume_backwin_set_geometry(NULL, _self, _x, _y, _w, _h)

mume_public void _mume_backwin_get_geometry(
    const void *clazz, void *self, int *x, int *y, int *w, int *h);

#define mume_backwin_get_geometry(_self, _x, _y, _w, _h) \
    _mume_backwin_get_geometry(NULL, _self, _x, _y, _w, _h)

/* Selector for set cursor of the backwin. */
mume_public void _mume_backwin_set_cursor(
    const void *clazz, void *self, void *cursor);

#define mume_backwin_set_cursor(_self, _cursor) \
    _mume_backwin_set_cursor(NULL, _self, _cursor)

/* Selector for raise/lower the z-order of the backwin. */
mume_public void _mume_backwin_raise(const void *clazz, void *self);

#define mume_backwin_raise(_self) _mume_backwin_raise(NULL, _self)

mume_public void _mume_backwin_lower(const void *clazz, void *self);

#define mume_backwin_lower(_self) _mume_backwin_lower(NULL, _self)

/* Selector for begin/end paint to the backwin. */
mume_public cairo_t* _mume_backwin_begin_paint(
    const void *clazz, void *self);

#define mume_backwin_begin_paint(_self) \
    _mume_backwin_begin_paint(NULL, _self)

mume_public void _mume_backwin_end_paint(
    const void *clazz, void *self, cairo_t *cr);

#define mume_backwin_end_paint(_self, _cr) \
    _mume_backwin_end_paint(NULL, _self, _cr)

/* Selector for grab/ungrab the mouse pointer. */
mume_public void _mume_backwin_grab_pointer(
    const void *clazz, void *self);

#define mume_backwin_grab_pointer(_self) \
    _mume_backwin_grab_pointer(NULL, _self)

mume_public void _mume_backwin_ungrab_pointer(
    const void *clazz, void *self);

#define mume_backwin_ungrab_pointer(_self) \
    _mume_backwin_ungrab_pointer(NULL, _self)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_BACKWIN_H */
