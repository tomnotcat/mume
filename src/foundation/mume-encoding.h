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
#ifndef MUME_FOUNDATION_ENCODING_H
#define MUME_FOUNDATION_ENCODING_H

#include "mume-common.h"

/* Text encoding check, from the "file" command code. */

MUME_BEGIN_DECLS

enum mume_encoding_e {
    MUME_ENCODING_UNKNOWN,
    MUME_ENCODING_ASCII,
    MUME_ENCODING_UTF8_BOM,
    MUME_ENCODING_UTF8,
    MUME_ENCODING_UTF16_LE,
    MUME_ENCODING_UTF16_BE,
    MUME_ENCODING_ISO_8859,
    MUME_ENCODING_EXTENDED_ASCII,
    MUME_ENCODING_EBCDIC,
    MUME_ENCODING_INTERNATIONAL_EBCDIC
};

mume_public int mume_encoding_check(
    const unsigned char *buf, size_t nbytes,
    unsigned int **ubuf, size_t *ulen);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_ENCODING_H */
