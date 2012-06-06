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
#include "mume-vector.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include MUME_ASSERT_H
#include MUME_STDLIB_H
#include MUME_STRING_H

mume_vector_t* mume_vector_ctor(
    mume_vector_t *vec, size_t eltsize,
    mume_desfcn_t *eltdes, void *param)
{
    assert(eltsize > 0);
    vec->buffer = NULL;
    vec->eltsize = eltsize;
    vec->count = 0;
    vec->total = 0;
    vec->eltdes = eltdes;
    vec->param = param;
    return vec;
}

mume_vector_t* mume_vector_dtor(mume_vector_t *vec)
{
    if (vec) {
        if (vec->eltdes) {
            char *obj = (char*)vec->buffer;
            while (vec->count--) {
                vec->eltdes(obj, vec->param);
                obj += vec->eltsize;
            }
        }

        free(vec->buffer);
    }

    return vec;
}

void* mume_vector_insert(
    mume_vector_t *vec, size_t idx, size_t num)
{
    size_t newcount = vec->count + num;
    if (newcount > vec->total) {
        vec->total += (vec->total >> 1) + 8;
        if (newcount > vec->total)
            vec->total = newcount;
        vec->buffer = realloc_abort(
            vec->buffer, vec->total * vec->eltsize);
    }
    if (idx < vec->count) {
        memmove((char*)vec->buffer + (idx + num) * vec->eltsize,
                (char*)vec->buffer + idx * vec->eltsize,
                (vec->count - idx) * vec->eltsize);
    }
    vec->count = newcount;
    return (char*)vec->buffer + idx * vec->eltsize;
}

void* mume_vector_erase(
    mume_vector_t *vec, size_t idx, size_t num)
{
    size_t newcount = vec->count - num;
    assert(idx + num <= vec->count);
    if (vec->eltdes) {
        size_t i;
        for (i = idx; i < idx + num; ++i) {
            vec->eltdes((char*)vec->buffer +
                     i * vec->eltsize, vec->param);
        }
    }

    if (idx + num < vec->count) {
        memmove((char*)vec->buffer + idx * vec->eltsize,
                (char*)vec->buffer + (idx + num) * vec->eltsize,
                (vec->count - idx - num) * vec->eltsize);
    }
    vec->count = newcount;
    return vec->buffer;
}
