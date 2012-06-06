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
#ifndef MUME_SDL_CURSOR_H
#define MUME_SDL_CURSOR_H

#include "mume-cursor.h"
#include "mume-sdl-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SDL_CURSOR (MUME_SIZEOF_CURSOR + \
                                sizeof(SDL_Cursor*))

#define MUME_SIZEOF_SDL_CURSOR_CLASS (MUME_SIZEOF_CURSOR_CLASS)

musdl_public const void* mume_sdl_cursor_class(void);

#define mume_sdl_cursor_meta_class mume_cursor_meta_class

musdl_public SDL_Cursor* mume_sdl_create_cursor(int id);

musdl_public void* mume_sdl_cursor_new(SDL_Cursor *cursor);

musdl_public SDL_Cursor* mume_sdl_cursor_get_entity(const void *self);

MUME_END_DECLS

#endif  /* MUME_SDL_CURSOR_H */
