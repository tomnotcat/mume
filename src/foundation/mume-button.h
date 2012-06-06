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
#ifndef MUME_FOUNDATION_BUTTON_H
#define MUME_FOUNDATION_BUTTON_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_BUTTON (MUME_SIZEOF_WINDOW + \
                            sizeof(char*) + \
                            sizeof(unsigned int))

#define MUME_SIZEOF_BUTTON_CLASS (MUME_SIZEOF_WINDOW_CLASS)

mume_public const void* mume_button_class(void);

mume_public const void* mume_button_meta_class(void);

mume_public void* mume_button_new(
    void *parent, int x, int y, const char *text);

mume_public void mume_button_set_text(void *self, const char *text);

mume_public const char* mume_button_get_text(const void *self);

mume_public mume_type_t* mume_typeof_button_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_BUTTON_H */
