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
#include "mume-memory.h"
#include "mume-config.h"
#include MUME_STDIO_H
#include MUME_STDLIB_H
#include MUME_STRING_H

void* malloc_abort(size_t size)
{
    void *p = malloc(size);
    if (NULL == p) {
        if (size) {
            fprintf(stderr, "malloc_abort(%u): failed\n", size);
            abort();
        }
    }
    return p;
}

void* calloc_abort(size_t count, size_t eltsize)
{
    void *cp = calloc(count, eltsize);
    if (NULL == cp) {
        if (count && eltsize) {
            fprintf(stderr, "calloc_abort(%u, %u): failed\n",
                    count, eltsize);
            abort();
        }
    }
    return cp;
}

void* realloc_abort(void *ptr, size_t newsize)
{
    void *np = realloc(ptr, newsize);
    if (np == NULL) {
        if (newsize) {
            fprintf(stderr, "realloc_abort(%u): failed\n", newsize);
            abort();
        }
    }
    return np;
}

char* strdup_abort(const char *s)
{
    if (s) {
        size_t len = strlen(s) + 1;
        return memcpy(malloc_abort(len), s, len);
    }

    return NULL;
}

char* strndup_abort(const char *s, size_t n)
{
    char *p;
    n = strnlen(s, n);
    p = malloc_abort(n + 1);
    p[n] = '\0';
    return (char*)memcpy(p, s, n);
}
