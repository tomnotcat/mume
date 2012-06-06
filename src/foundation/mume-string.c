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
#include "mume-string.h"
#include "mume-memory.h"

#if 0
int strncmp(const char *s1, const char *s2, size_t size)
{
    int i = 0;
    while ((i = (*s1 - *s2)) >= 0) {
        if (i > 0)
            return 1;
        if ('\0' == *s1)
            return 0;
        if (!(--size))
            return 0;
        s1++;
        s2++;
    }
    return -1;
}
#endif

size_t mume_strslen(int count, size_t append, ...)
{
    va_list ap;
    const char *str;
    size_t len = 0;
    va_start(ap, append);
    while (count-- > 0) {
        str = va_arg(ap, const char*);
        if (str)
            len += strlen(str) + append;
    }
    va_end(ap);
    return len;
}

void mume_strscpy(char *buffer, ...)
{
    va_list ap;
    char **dest;
    const char *src;
    size_t len = 0;
    va_start(ap, buffer);
    while ((dest = va_arg(ap, char**))) {
        src = va_arg(ap, const char*);
        if (src) {
            *dest = buffer + len;
            strcpy(*dest, src);
            len += strlen(src) + 1;
        }
        else {
            *dest = NULL;
        }
    }
    va_end(ap);
}

const char* mume_mask_to_string(
    char *buffer, size_t buflen, int mask, const char *split,
    int *values, const char **strings, int count)
{
    int i;
    size_t curpos = 0;
    buffer[0] = '\0';
    for (i = 0; i < count; ++i) {
        if (values[i] & mask) {
            if (buffer[0] && split) {
                curpos += strcpy_c(
                    buffer + curpos, buflen - curpos, split);
            }
            curpos += strcpy_c(
                buffer + curpos, buflen - curpos, strings[i]);
        }
    }
    if (curpos >= buflen)
        --curpos;
    buffer[curpos] = '\0';
    return buffer;
}

char* mume_escape_xml_entities(const char *str)
{
    int i, len;
    const char *rep;
    char *esc = (char*)str;
    size_t j = 0;
    size_t m = 0;

    len = strlen(str);
    for (i = 0; i < len; ++i) {
        switch (str[i]) {
        case '<':
            rep = "&lt;";
            break;

        case '>':
            rep = "&gt;";
            break;

        case '&':
            rep = "&amp;";
            break;

        case '\'':
            rep = "&apos;";
            break;

        case '\"':
            rep = "&quot;";
            break;

        default:
            rep = NULL;
            break;
        }

        if (!rep) {
            if (esc != str) {
                esc = mume_ensure_buffer(
                    esc, &m, j + 1, sizeof(char*));

                esc[j] = str[i];
            }

            ++j;
        }
        else {
            int rep_len = strlen(rep);

            if (esc != str) {
                esc = mume_ensure_buffer(
                    esc, &m, j + rep_len, sizeof(char*));
            }
            else {
                m = j + rep_len;
                esc = malloc_abort(m);
                strncpy(esc, str, j);
            }

            memcpy(esc + j, rep, rep_len);
            j += rep_len;
        }
    }

    if (esc != str) {
        esc = mume_ensure_buffer(
            esc, &m, j + 1, sizeof(char*));

        esc[j] = '\0';
    }

    return esc;
}

char* mume_resume_xml_entities(char *str)
{
    int i, j = 0;
    int len = strlen(str);

    for (i = 0; i < len; ++i) {
        if (str[i] != '&') {
            str[j++] = str[i];
        }
        else {
            if (0 == strncmp(str + i, "&lt;", 4)) {
                str[j++] = '<';
                i += 3;
            }
            else if (0 == strncmp(str + i, "&gt;", 4)) {
                str[j++] = '>';
                i += 3;
            }
            else if (0 == strncmp(str + i, "&amp;", 5)) {
                str[j++] = '&';
                i += 4;
            }
            else if (0 == strncmp(str + i, "&apos;", 5)) {
                str[j++] = '\'';
                i += 4;
            }
            else if (0 == strncmp(str + i, "&quot;", 5)) {
                str[j++] = '\"';
                i += 4;
            }
            else {
                str[j++] = str[i];
            }
        }
    }

    if (i != j)
        str[j] = '\0';

    return str;
}
