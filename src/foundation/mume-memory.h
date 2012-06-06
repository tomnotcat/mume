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
#ifndef MUME_FOUNDATION_MEMORY_H
#define MUME_FOUNDATION_MEMORY_H

#include "mume-common.h"

MUME_BEGIN_DECLS

/* Allocate memory, abort program if failed. */
mume_public void* malloc_abort(size_t size);
mume_public void* calloc_abort(size_t count, size_t eltsize);
mume_public void* realloc_abort(void *ptr, size_t newsize);
mume_public char* strdup_abort(const char *s);
mume_public char* strndup_abort(const char *s, size_t n);

#define malloc_struct(_strux) \
    ((_strux*)malloc_abort(sizeof(_strux)))

#define calloc_struct(_count, _strux) \
    ((_strux*)calloc_abort(_count, sizeof(_strux)))

static inline void* mume_ensure_buffer(
    void *buffer, size_t *allocated, size_t nmemb, size_t size)
{
    if (nmemb > *allocated) {
        while (nmemb > *allocated)
            *allocated += (*allocated >> 1) + 32;

        buffer = realloc_abort(buffer, (*allocated) * size);
    }

    return buffer;
}

MUME_END_DECLS

#endif /* MUME_FOUNDATION_MEMORY_H */
