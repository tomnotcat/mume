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
#ifndef MUME_FOUNDATION_FILETC_H
#define MUME_FOUNDATION_FILETC_H

#include "mume-object.h"

/* The file type check, inspired by the "file" command. */

MUME_BEGIN_DECLS

#define MUME_MAX_MAGIC_STRING 64

enum mume_magictype_e {
    MUME_MAGICTYPE_STRING
};

enum mume_filetype_e {
    MUME_FILETYPE_UNKNOWN,
    MUME_FILETYPE_EMPTY,
    MUME_FILETYPE_PDF,
    MUME_FILETYPE_TXT
};

typedef struct _mume_magic {
    size_t offset;
    int type;    /* mume_magictype_e */
    union {
        char s[MUME_MAX_MAGIC_STRING];
    } value;
} mume_magic_t;

mume_public const void* mume_filetc_class(void);

#define mume_filetc_new() mume_new(mume_filetc_class())

mume_public int mume_filetc_check_ext(
    const void *self, const char *name);

mume_public int mume_filetc_check_magic(
    const void *self, mume_stream_t *stm);

mume_public void mume_filetc_add_ext(
    void *self, int type, const char *ext);

mume_public void mume_filetc_add_magic(
    void *self, int type, mume_magic_t magic);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_FILETC_H */
