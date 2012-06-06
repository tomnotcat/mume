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
#ifndef MUME_FOUNDATION_SERIALIZE_H
#define MUME_FOUNDATION_SERIALIZE_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SERIALIZE (MUME_SIZEOF_OBJECT +     \
                               sizeof(void*) * 2 +      \
                               sizeof(size_t) * 2 +     \
                               sizeof(unsigned int))

#define MUME_SIZEOF_SERIALIZE_CLASS (MUME_SIZEOF_CLASS)

mume_public const void* mume_serialize_class(void);

#define mume_serialize_meta_class mume_meta_class

#define mume_serialize_new() mume_new(mume_serialize_class())

mume_public void mume_serialize_register(
    void *self, const void *clazz);

mume_public int mume_serialize_out(void *self, mume_stream_t *stm);

mume_public int mume_serialize_in(void *self, mume_stream_t *stm);

#define mume_serialize_write_to_file(_self, _file) \
    mume_process_file_stream( \
        mume_serialize_out, _self, _file, MUME_OM_WRITE)

#define mume_serialize_read_from_file(_self, _file) \
    mume_process_file_stream( \
        mume_serialize_in, _self, _file, MUME_OM_READ)

mume_public void mume_serialize_set_static_object(
    void *self, const char *name, const void *object);

mume_public void mume_serialize_set_object(
    void *self, const char *name, const void *object);

mume_public const void* mume_serialize_get_object(
    const void *self, const char *name);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_SERIALIZE_H */
