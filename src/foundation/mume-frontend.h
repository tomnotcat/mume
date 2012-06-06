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
#ifndef MUME_FOUNDATION_FRONTEND_H
#define MUME_FOUNDATION_FRONTEND_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_FRONTEND (MUME_SIZEOF_OBJECT + \
                              sizeof(void*) * 4 + \
                              sizeof(unsigned int) + \
                              sizeof(int) * 5)

#define MUME_SIZEOF_FRONTEND_CLASS (MUME_SIZEOF_CLASS + \
                                    sizeof(voidf*) * 15)

mume_public const void* mume_frontend_class(void);

mume_public const void* mume_frontend_meta_class(void);

#define mume_frontend_new() mume_new(mume_frontend_class())

mume_public void mume_frontend_set_pointer_owner(
    void *self, void *window);

mume_public void* mume_frontend_get_pointer_owner(const void *self);

mume_public void* mume_frontend_get_keyboard_owner(const void *self);

mume_public void mume_frontend_open_popup(void *self, void *window);

mume_public void mume_frontend_close_popup(void *self, void *window);

mume_public void mume_frontend_remove_window(void *self, void *window);

mume_public void _mume_frontend_handle_keydown(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int keysym);

#define mume_frontend_handle_keydown( \
    _self, _tgt, _x, _y, _state, _keysym) \
    _mume_frontend_handle_keydown( \
        NULL, _self, _tgt, _x, _y, _state, _keysym)

mume_public void _mume_frontend_handle_keyup(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int keysym);

#define mume_frontend_handle_keyup( \
    _self, _tgt, _x, _y, _state, _keysym) \
    _mume_frontend_handle_keyup( \
        NULL, _self, _tgt, _x, _y, _state, _keysym)

mume_public void _mume_frontend_handle_buttondown(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int button);

#define mume_frontend_handle_buttondown( \
    _self, _tgt, _x, _y, _state, _button) \
    _mume_frontend_handle_buttondown( \
        NULL, _self, _tgt, _x, _y, _state, _button)

mume_public void _mume_frontend_handle_buttonup(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int button);

#define mume_frontend_handle_buttonup( \
    _self, _tgt, _x, _y, _state, _button) \
    _mume_frontend_handle_buttonup( \
        NULL, _self, _tgt, _x, _y, _state, _button)

mume_public void _mume_frontend_handle_mousemotion(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state);

#define mume_frontend_handle_mousemotion( \
    _self, _tgt, _x, _y, _state) \
    _mume_frontend_handle_mousemotion( \
        NULL, _self, _tgt, _x, _y, _state)

mume_public void _mume_frontend_handle_mouseenter(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int mode);

#define mume_frontend_handle_mouseenter( \
    _self, _tgt, _x, _y, _state, _mode) \
    _mume_frontend_handle_mouseenter( \
        NULL, _self, _tgt, _x, _y, _state, _mode)

mume_public void _mume_frontend_handle_mouseleave(
    const void *clazz, void *self, void *backwin,
    int x, int y, int state, int mode);

#define mume_frontend_handle_mouseleave( \
    _self, _tgt, _x, _y, _state, _mode) \
    _mume_frontend_handle_mouseleave( \
        NULL, _self, _tgt, _x, _y, _state, _mode)

mume_public void _mume_frontend_handle_focusin(
    const void *clazz, void *self, void *backwin, int mode);

#define mume_frontend_handle_focusin(_self, _tgt, _mode) \
    _mume_frontend_handle_focusin(NULL, _self, _tgt, _mode)

mume_public void _mume_frontend_handle_focusout(
    const void *clazz, void *self, void *backwin, int mode);

#define mume_frontend_handle_focusout(_self, _tgt, _mode) \
    _mume_frontend_handle_focusout(NULL, _self, _tgt, _mode)

mume_public void _mume_frontend_handle_map(
    const void *clazz, void *self, void *backwin);

#define mume_frontend_handle_map(_self, _tgt) \
    _mume_frontend_handle_map(NULL, _self, _tgt)

mume_public void _mume_frontend_handle_unmap(
    const void *clazz, void *self, void *backwin);

#define mume_frontend_handle_unmap(_self, _tgt) \
    _mume_frontend_handle_unmap(NULL, _self, _tgt)

mume_public void _mume_frontend_handle_geometry(
    const void *clazz, void *self, void *backwin,
    int x, int y, int width, int height);

#define mume_frontend_handle_geometry( \
    _self, _tgt, _x, _y, _width, _height) \
    _mume_frontend_handle_geometry( \
        NULL, _self, _tgt, _x, _y, _width, _height)

mume_public void _mume_frontend_handle_expose(
    const void *clazz, void *self, void *backwin,
    const mume_rect_t *rect);

#define mume_frontend_handle_expose(_self, _tgt, _rect) \
    _mume_frontend_handle_expose(NULL, _self, _tgt, _rect)

mume_public void _mume_frontend_handle_close(
    const void *clazz, void *self, void *backwin);

#define mume_frontend_handle_close(_self, _tgt) \
    _mume_frontend_handle_close(NULL, _self, _tgt)

mume_public void _mume_frontend_handle_quit(
    const void *clazz, void *self);

#define mume_frontend_handle_quit(_self) \
    _mume_frontend_handle_quit(NULL, _self)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_FRONTEND_H */
