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
#ifndef MUME_FOUNDATION_STRING_H
#define MUME_FOUNDATION_STRING_H

#include "mume-common.h"
#include MUME_STRING_H

MUME_BEGIN_DECLS

static inline size_t strcpy_c(
    char *buf, size_t len, const char *str)
{
    size_t cc = 0;
    while (len-- && *str)
        buf[cc++] = *str++;

    return cc;
}

static inline char* strcpy_s(
    char *buf, size_t len, const char *str)
{
    size_t cc = strcpy_c(buf, len, str);
    if (cc < len) {
        buf[cc] = '\0';
        return buf;
    }

    return 0;
}

/* Calculate <count> strings' length, each non-NULL string
 * will add <append> length to the result.
 */
mume_public size_t mume_strslen(int count, size_t append, ...);

/* The following example will make <p1> point to <buf> and copy
 * <s1> to <p1>, make <p2> point to <buf> + strlen(s1) + 1 and
 * copy <s2> to <p2>, make <p3> point to NULL.
 *
 * char buf[32];
 * const char *s1 = "hello";
 * const char *s2 = "world";
 * const char *s3 = NULL;
 * char *p1, *p2, *p3;
 * mume_strscpy(buf, &p1, s1, &p2, s2, &p3, s3, NULL);
 */
mume_public void mume_strscpy(char *buffer, ...);

mume_public const char* mume_mask_to_string(
    char *buffer, size_t buflen, int mask, const char *split,
    int *values, const char **strings, int count);

mume_public char* mume_escape_xml_entities(const char *str);

mume_public char* mume_resume_xml_entities(char *str);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_STRING_H */
