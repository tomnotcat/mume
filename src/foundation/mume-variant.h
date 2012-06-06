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
#ifndef MUME_FOUNDATION_VARIANT_H
#define MUME_FOUNDATION_VARIANT_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_VARIANT (MUME_SIZEOF_OBJECT + \
                             sizeof(int) +        \
                             sizeof(double))

#define MUME_SIZEOF_VARIANT_CLASS (MUME_SIZEOF_CLASS)

enum mume_type_e {
    MUME_TYPE_INT,
    MUME_TYPE_FLOAT,
    MUME_TYPE_DOUBLE,
    MUME_TYPE_STRING,
    MUME_TYPE_OBJECT
};

mume_public const void* mume_variant_class(void);

#define mume_variant_meta_class mume_meta_class

mume_public void* mume_variant_new(int type);

mume_public void mume_variant_reset(void *self, int type);

mume_public int mume_variant_get_type(const void *self);

mume_public void mume_variant_set_int(void *self, int value);

mume_public int mume_variant_get_int(const void *self);

mume_public void mume_variant_set_float(void *self, float value);

mume_public float mume_variant_get_float(const void *self);

mume_public void mume_variant_set_double(void *self, double value);

mume_public double mume_variant_get_double(const void *self);

mume_public void mume_variant_set_string(
    void *self, const char *string);

mume_public void mume_variant_append_string(
    void *self, const char *string, int length);

mume_public void mume_variant_set_static_string(
    void *self, const char *string);

mume_public const char* mume_variant_get_string(const void *self);

mume_public void mume_variant_set_object(
    void *self, const void *object);

mume_public void mume_variant_set_static_object(
    void *self, const void *object);

mume_public const void* mume_variant_get_object(const void *self);

mume_public int mume_variant_convert(void *self, int type);

static inline int mume_type_is_valid(int type)
{
    return (type >= MUME_TYPE_INT && type <= MUME_TYPE_OBJECT);
}

static inline void* mume_variant_new_int(int value)
{
    void *var = mume_variant_new(MUME_TYPE_INT);
    mume_variant_set_int(var, value);
    return var;
}

static inline void* mume_variant_new_float(float value)
{
    void *var = mume_variant_new(MUME_TYPE_FLOAT);
    mume_variant_set_float(var, value);
    return var;
}

static inline void* mume_variant_new_double(double value)
{
    void *var = mume_variant_new(MUME_TYPE_DOUBLE);
    mume_variant_set_double(var, value);
    return var;
}

static inline void* mume_variant_new_string(const char *value)
{
    void *var = mume_variant_new(MUME_TYPE_STRING);
    mume_variant_set_string(var, value);
    return var;
}

static inline void* mume_variant_new_static_string(const char *value)
{
    void *var = mume_variant_new(MUME_TYPE_STRING);
    mume_variant_set_static_string(var, value);
    return var;
}

static inline void* mume_variant_new_object(const void *value)
{
    void *var = mume_variant_new(MUME_TYPE_OBJECT);
    mume_variant_set_object(var, value);
    return var;
}

static inline void* mume_variant_new_static_object(const void *value)
{
    void *var = mume_variant_new(MUME_TYPE_OBJECT);
    mume_variant_set_static_object(var, value);
    return var;
}

MUME_END_DECLS

#endif /* MUME_FOUNDATION_VARIANT_H */
