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
#ifndef MUME_FOUNDATION_VECTOR_H
#define MUME_FOUNDATION_VECTOR_H

#include "mume-common.h"

MUME_BEGIN_DECLS

struct mume_vector_s {
    void *buffer;
    size_t eltsize;
    size_t count;
    size_t total;
    mume_desfcn_t *eltdes;
    void *param;
};

mume_public mume_vector_t* mume_vector_ctor(
    mume_vector_t *vec, size_t eltsize,
    mume_desfcn_t *eltdes, void *param);

mume_public mume_vector_t* mume_vector_dtor(
    mume_vector_t *vec);

/* insert <num> elements before <idx>, return the address of <idx> */
mume_public void* mume_vector_insert(
    mume_vector_t *vec, size_t idx, size_t num);
/* erase <num> elements from <idx>,
   return the buffer (may be reallocated)*/
mume_public void* mume_vector_erase(
    mume_vector_t *vec, size_t idx, size_t num);

#define mume_vector_new(_eltsize, _eltdes, _param) \
    mume_vector_ctor(malloc_struct(mume_vector_t), \
                           _eltsize, _eltdes, _param)

#define mume_vector_delete(_vec) \
    free(mume_vector_dtor(_vec))

#define mume_vector_eltsize(_vec) ((size_t)(_vec)->eltsize)
#define mume_vector_size(_vec) ((size_t)(_vec)->count)
#define mume_vector_empty(_vec) (0 == (_vec)->count)

#define mume_vector_foreach(_vec, _idx, _obj) \
    for (_idx = 0, _obj = (typeof(_obj))(_vec)->buffer; \
         _idx != mume_vector_size(_vec); \
         ++_idx, _obj = (typeof(_obj))((char*)_obj + (_vec)->eltsize))

static inline void mume_vector_clear(mume_vector_t *vec)
{
    mume_vector_erase(vec, 0, vec->count);
}

static inline void* mume_vector_at(mume_vector_t *vec, size_t idx)
{
    return (char*)vec->buffer + idx * vec->eltsize;
}

static inline void* mume_vector_front(mume_vector_t *vec)
{
    return (char*)vec->buffer;
}

static inline void* mume_vector_back(mume_vector_t *vec)
{
    return (char*)vec->buffer + (vec->count - 1) * vec->eltsize;
}

static inline void* mume_vector_push_back(mume_vector_t *vec)
{
    return mume_vector_insert(vec, vec->count, 1);
}

static inline void mume_vector_pop_back(mume_vector_t *vec)
{
    mume_vector_erase(vec, vec->count - 1, 1);
}

static inline void* mume_vector_append(mume_vector_t *vec, size_t n)
{
    return mume_vector_insert(vec, vec->count, n);
}

static inline void* mume_vector_detach(mume_vector_t *vec)
{
    void *r = vec->buffer;
    vec->buffer = NULL;
    vec->count = 0;
    vec->total = 0;
    return r;
}

MUME_END_DECLS

#endif /* MUME_FOUNDATION_VECTOR_H */
