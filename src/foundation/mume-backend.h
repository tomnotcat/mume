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
#ifndef MUME_FOUNDATION_BACKEND_H
#define MUME_FOUNDATION_BACKEND_H

#include "mume-refobj.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_BACKEND (MUME_SIZEOF_REFOBJ)

#define MUME_SIZEOF_BACKEND_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                   sizeof(voidf*) * 9)

#define MUME_BACKEND_CLASS_SYM "mume_backend_class_sym"

#define MUME_WAIT_INFINITE 0xFFFFFFFF

enum mume_backend_flags_e {
    MUME_BACKEND_FLAG_CENTERED,
    MUME_BACKEND_FLAG_RESIZABLE,
    MUME_BACKEND_FLAG_FULLSCREEN
};

enum mume_backwin_types_e {
    MUME_BACKWIN_NORMAL,
    MUME_BACKWIN_MENU
};

mume_public const void* mume_backend_class(void);

mume_public const void* mume_backend_meta_class(void);

mume_public void* mume_backend_new_from_dll(
    mume_dlhdl_t *hdl, int width, int height,
    unsigned int flags, void *data);

/* Selector for get the screen size of the backend.
 * For media backend, this return the host window's size.
 */
mume_public void _mume_backend_screen_size(
    const void *clazz, void *self, int *width, int *height);

#define mume_backend_screen_size(_self, _width, _height) \
    _mume_backend_screen_size(NULL, _self, _width, _height)

/* Selector for query data format used by clipboard and DnD. */
mume_public void* _mume_backend_data_format(
    const void *clazz, void *self, const char *name);

#define mume_backend_data_format(_self, _name) \
    _mume_backend_data_format(NULL, _self, _name)

/* Selector for query the clipboard object of the backend. */
mume_public void* _mume_backend_clipboard(
    const void *clazz, void *self);

#define mume_backend_clipboard(_self) \
    _mume_backend_clipboard(NULL, _self)

/* Selector for get the root backwin of the backend. */
mume_public void* _mume_backend_root_backwin(
    const void *clazz, void *self);

#define mume_backend_root_backwin(_self) \
    _mume_backend_root_backwin(NULL, _self)

/* Selector for create a backwin in the backend. */
mume_public void* _mume_backend_create_backwin(
    const void *clazz, void *self, int type,
    void *parent, int x, int y, int width, int height);

#define mume_backend_create_backwin(_self, _type, _parent, \
                                    _x, _y, _width, _height) \
    _mume_backend_create_backwin(NULL, _self, _type, _parent, \
                                 _x, _y, _width, _height)

/* Selector for create a cursor object of the backend. */
mume_public void* _mume_backend_create_cursor(
    const void *clazz, void *self, int id);

#define mume_backend_create_cursor(_self, _id) \
    _mume_backend_create_cursor(NULL, _self, _id)

/* Selector for handle the pending events of the backend. */
mume_public int _mume_backend_handle_event(
    const void *clazz, void *self, int wait);

#define mume_backend_handle_event(_self, _wait) \
    _mume_backend_handle_event(NULL, _self, _wait)

/* Selector for wakeup the blocking of event handling. */
mume_public int _mume_backend_wakeup_event(
    const void *clazz, void *self);

#define mume_backend_wakeup_event(_self) \
    _mume_backend_wakeup_event(NULL, _self)

/* Selector for query the current mouse pointer information. */
mume_public void _mume_backend_query_pointer(
    const void *clazz, void *self, int *x, int *y, int *state);

#define mume_backend_query_pointer(_self, _x, _y, _state) \
    _mume_backend_query_pointer(NULL, _self, _x, _y, _state)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_BACKEND_H */
