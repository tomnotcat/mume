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
#ifndef MUME_SDL_BACKWIN_H
#define MUME_SDL_BACKWIN_H

#include "mume-backwin.h"
#include "mume-sdl-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SDL_BACKWIN (MUME_SIZEOF_BACKWIN + \
                                 sizeof(void*) * 2)

#define MUME_SIZEOF_SDL_BACKWIN_CLASS (MUME_SIZEOF_BACKWIN_CLASS + \
                                       sizeof(voidf*) * 11)

musdl_public const void* mume_sdl_backwin_class(void);

musdl_public const void* mume_sdl_backwin_meta_class(void);

musdl_public void* mume_sdl_backwin_new(
    void *backend, SDL_Surface *surface);

musdl_public void _mume_sdl_backwin_handle_event(
    const void *clazz, void *self, SDL_Event *event);

#define mume_sdl_backwin_handle_event(_self, _event) \
    _mume_sdl_backwin_handle_event(NULL, _self, _event)

musdl_public void _mume_sdl_backwin_handle_key_down(
    const void *clazz, void *self, SDL_KeyboardEvent *key);

#define mume_sdl_backwin_handle_key_down(_self, _key) \
    _mume_sdl_backwin_handle_key_down(NULL, _self, _key)

musdl_public void _mume_sdl_backwin_handle_key_up(
    const void *clazz, void *self, SDL_KeyboardEvent *key);

#define mume_sdl_backwin_handle_key_up(_self, _key) \
    _mume_sdl_backwin_handle_key_up(NULL, _self, _key)

musdl_public void _mume_sdl_backwin_handle_mouse_motion(
    const void *clazz, void *self, SDL_MouseMotionEvent *motion);

#define mume_sdl_backwin_handle_mouse_motion(_self, _motion) \
    _mume_sdl_backwin_handle_mouse_motion(NULL, _self, _motion)

musdl_public void _mume_sdl_backwin_handle_button_down(
    const void *clazz, void *self, SDL_MouseButtonEvent *button);

#define mume_sdl_backwin_handle_button_down(_self, _button) \
    _mume_sdl_backwin_handle_button_down(NULL, _self, _button)

musdl_public void _mume_sdl_backwin_handle_button_up(
    const void *clazz, void *self, SDL_MouseButtonEvent *button);

#define mume_sdl_backwin_handle_button_up(_self, _button) \
    _mume_sdl_backwin_handle_button_up(NULL, _self, _button)

musdl_public void _mume_sdl_backwin_handle_user_event(
    const void *clazz, void *self, SDL_UserEvent *user);

#define mume_sdl_backwin_handle_user_event(_self, _user) \
    _mume_sdl_backwin_handle_user_event(NULL, _self, _user)

musdl_public void _mume_sdl_backwin_handle_active_event(
    const void *clazz, void *self, SDL_ActiveEvent *active);

#define mume_sdl_backwin_handle_active_event(_self, _user) \
    _mume_sdl_backwin_handle_active_event(NULL, _self, _user)

musdl_public void _mume_sdl_backwin_handle_video_resize(
    const void *clazz, void *self, SDL_ResizeEvent *resize);

#define mume_sdl_backwin_handle_video_resize(_self, _resize) \
    _mume_sdl_backwin_handle_video_resize(NULL, _self, _resize)

musdl_public void _mume_sdl_backwin_handle_video_expose(
    const void *clazz, void *self, SDL_ExposeEvent *expose);

#define mume_sdl_backwin_handle_video_expose(_self, _expose) \
    _mume_sdl_backwin_handle_video_expose(NULL, _self, _expose)

musdl_public void _mume_sdl_backwin_handle_quit(
    const void *clazz, void *self, SDL_QuitEvent *quit);

#define mume_sdl_backwin_handle_quit(_self, _quit) \
    _mume_sdl_backwin_handle_quit(NULL, _self, _quit)

musdl_public int mume_sdl_translate_keysym(int keysym);

musdl_public int mume_sdl_translate_button(int button);

musdl_public int mume_sdl_translate_state(int keymod, int mousemod);

MUME_END_DECLS

#endif  /* MUME_SDL_BACKWIN_H */
