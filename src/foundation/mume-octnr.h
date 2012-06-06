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
#ifndef MUME_FOUNDATION_OCTNR_H
#define MUME_FOUNDATION_OCTNR_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_OCTNR (MUME_SIZEOF_OBJECT)

#define MUME_SIZEOF_OCTNR_CLASS (MUME_SIZEOF_CLASS + \
                                 sizeof(voidf*) * 10)

mume_public const void* mume_octnr_class(void);

mume_public const void* mume_octnr_meta_class(void);

mume_public void mume_octnr_enumerate(
    const void *self, void (*proc)(void*, void*), void *closure);

mume_public void mume_octnr_append(
    void *self, const void *from, const void *clazz);

mume_public void* _mume_octnr_begin(const void *clazz, void *self);

#define mume_octnr_begin(_self) \
    _mume_octnr_begin(NULL, (void*)(_self))

mume_public void* _mume_octnr_end(const void *clazz, void *self);

#define mume_octnr_end(_self) \
    _mume_octnr_end(NULL, (void*)(_self))

mume_public void* _mume_octnr_next(
    const void *clazz, void *self, void *it);

#define mume_octnr_next(_self, _it) \
    _mume_octnr_next(NULL, (void*)(_self), _it)

mume_public void* _mume_octnr_value(
    const void *clazz, void *self, void *it);

#define mume_octnr_value(_self, _it) \
    _mume_octnr_value(NULL, (void*)(_self), _it)

mume_public void* _mume_octnr_insert(
    const void *clazz, void *self, void *it, void *object);

#define mume_octnr_insert(_self, _it, _object) \
    _mume_octnr_insert(NULL, _self, _it, _object)

mume_public void* _mume_octnr_find(
    const void *clazz, void *self, const void *object);

#define mume_octnr_find(_self, _object) \
    _mume_octnr_find(NULL, _self, _object)

mume_public void* _mume_octnr_erase(
    const void *clazz, void *self, void *it);

#define mume_octnr_erase(_self, _it) \
    _mume_octnr_erase(NULL, _self, _it)

mume_public size_t _mume_octnr_size(const void *clazz, void *self);

#define mume_octnr_size(_self) _mume_octnr_size(NULL, _self)

mume_public void _mume_octnr_clear(const void *clazz, void *self);

#define mume_octnr_clear(_self) _mume_octnr_clear(NULL, _self)

#define mume_octnr_foreach(_ctnr, _it, _obj) \
    for (_it = mume_octnr_begin(_ctnr); \
         (_obj = ((_it != mume_octnr_end(_ctnr)) ? \
                  (void*)mume_octnr_value(_ctnr, _it) : 0)); \
         _it = mume_octnr_next(_ctnr, _it))

MUME_END_DECLS

#endif /* MUME_FOUNDATION_OCTNR_H */
