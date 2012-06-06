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
#ifndef MUME_FOUNDATION_TEXT_LAYOUT_H
#define MUME_FOUNDATION_TEXT_LAYOUT_H

#include "mume-object.h"

MUME_BEGIN_DECLS

enum mume_text_layout_format_e {
    MUME_TLF_LEFT    = 0x00000000,
    MUME_TLF_TOP     = 0x00000000,
    MUME_TLF_RIGHT   = 0x00000001,
    MUME_TLF_BOTTOM  = 0x00000002,
    MUME_TLF_CENTER  = 0x00000004,
    MUME_TLF_VCENTER = 0x00000008,
    MUME_TLF_WORDBREAK  = 0x00000010,
    MUME_TLF_SINGLELINE = 0x00000020,
    MUME_TLF_EXPANDTABS = 0x00000040,
    MUME_TLF_NOCLIP     = 0x00000080,
    MUME_TLF_CALCRECT   = 0x00000100,
    MUME_TLF_DRAWTEXT   = 0x00000200
};

mume_public const void* mume_text_layout_class(void);

#define mume_text_layout_meta_class mume_meta_class

#define mume_text_layout_new() mume_new(mume_text_layout_class())

mume_public void mume_text_layout_reset(void *self);

mume_public void mume_text_layout_add_text(
    void *self, cairo_font_face_t *face, const char *text, int length);

mume_public void mume_text_layout_perform(
    void *self, int font_size, cairo_t *cr,
    mume_rect_t *rect, unsigned int format);

mume_public const char* mume_text_layout_get_texts(const void *self);

mume_public const mume_rect_t* mume_text_layout_get_rects(const void *self);

mume_public int mume_text_layout_get_length(const void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TEXT_LAYOUT_H */
