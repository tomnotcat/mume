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
#ifndef MUME_SDL_BACKEND_H
#define MUME_SDL_BACKEND_H

#include "mume-backend.h"
#include "mume-sdl-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SDL_BACKEND (MUME_SIZEOF_BACKEND + \
                                 sizeof(void*))

#define MUME_SIZEOF_SDL_BACKEND_CLASS (MUME_SIZEOF_BACKEND_CLASS)

musdl_public const void* mume_sdl_backend_class(void);

#define mume_sdl_backend_meta_class mume_backend_meta_class

musdl_public const void* mume_backend_class_sym(void);

MUME_END_DECLS

#endif  /* MUME_SDL_BACKEND_H */
