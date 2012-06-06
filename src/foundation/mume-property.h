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
#ifndef MUME_FOUNDATION_PROPERTY_H
#define MUME_FOUNDATION_PROPERTY_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_PROPERTY (MUME_SIZEOF_OBJECT +  \
                              sizeof(int) * 2 +     \
                              sizeof(char*) +       \
                              sizeof(unsigned int))

#define MUME_SIZEOF_PROPERTY_CLASS (MUME_SIZEOF_CLASS)

enum mume_property_e {
    MUME_PROP_READABLE  = 1 << 0,
    MUME_PROP_WRITABLE  = 1 << 1,
    MUME_PROP_CONSTRUCT = 1 << 2,
    MUME_PROP_CONSTRUCT_ONLY = 1 << 3,
    MUME_PROP_STATIC_NAME    = 1 << 4
};

#define MUME_PROP_READWRITE (MUME_PROP_READABLE | MUME_PROP_WRITABLE)

mume_public const void* mume_property_class(void);

#define mume_property_meta_class mume_meta_class

mume_public void* mume_property_new(
    int type, const char *name, int id, unsigned int flags);

mume_public void* mume_property_key(void *self, const char *name);

mume_public int mume_property_get_type(const void *self);

mume_public const char* mume_property_get_name(const void *self);

mume_public int mume_property_get_id(const void *self);

mume_public unsigned int mume_property_get_flags(const void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_PROPERTY_H */
