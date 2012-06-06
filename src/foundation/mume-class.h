/* Mume Reader - a full featured reading environment.
 *
 * Copyright © 2012 Soft Flag, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MUME_FOUNDATION_CLASS_H
#define MUME_FOUNDATION_CLASS_H

#include "mume-message.h"

MUME_BEGIN_DECLS

#define MUME_OBJECT_MESSAGE_LIST(_) \
    _(OBJECT_INITIALIZE)            \
    _(OBJECT_FINALIZE)              \
    _(OBJECT_ADDREF)                \
    _(OBJECT_RELEASE)               \
    _(OBJECT_REFCOUNT)

enum mume_object_message_e {
    MUME_OBJECT_MESSAGE_FIRST,
    MUME_OBJECT_MESSAGE_LIST(MUME_DEFINE_MSGID)
    MUME_OBJECT_MESSAGE_LAST
};

/* Initialize/Finalize an object.
 * Return 0 on success. */
MUME_DEFINE_MESSAGE0(MUME_MSG_OBJECT_INITIALIZE)
MUME_DEFINE_MESSAGE0(MUME_MSG_OBJECT_FINALIZE)

/* Reference operations, an object that support reference
 * must implement these messages.
 * ADDREF, RELEASE return non-zero on success.
 * REFCOUNT return the reference count. */
MUME_DEFINE_MESSAGE0(MUME_MSG_OBJECT_ADDREF)
MUME_DEFINE_MESSAGE0(MUME_MSG_OBJECT_RELEASE)
MUME_DEFINE_MESSAGE0(MUME_MSG_OBJECT_REFCOUNT)

typedef struct mume_class mume_class_t;
typedef struct mume_object mume_object_t;

struct mume_class {
    const char *class_name;
    const mume_class_t *super_class;
    size_t data_size;
    int (*message)(mume_object_t *obj, mume_message_t *msg);
};

struct mume_object {
    const mume_class_t *clazz;
    void *data;
};

mume_public const mume_class_t* mume_object_class2(void);

mume_public mume_object_t mume_new2(const mume_class_t *clazz, ...);

mume_public void mume_delete2(mume_object_t object);

mume_public mume_object_t mume_clone2(const mume_object_t object);

#define mume_class_of2(_object) ((_object)->clazz)

#define mume_size_of2(_object) \
    (mume_class_of(_object)->data_size)

mume_public int mume_is_ancestor2(
    const mume_class_t *c1, const mume_class_t *c2);

mume_public int mume_is_a2(
    const mume_object_t object, const mume_class_t *clazz);

mume_public int mume_is_of2(
    const mume_object_t object, const mume_class_t *clazz);

#define mume_class_name2(_clazz) ((_clazz)->class_name)

#define mume_super_class2(_clazz) ((_clazz)->super_class)

#define mume_class_data_size(_clazz) ((_clazz)->data_size)

#define mume_class_message(_clazz, _obj, _msg) \
    ((_clazz)->message(_obj, _msg))

MUME_END_DECLS

#endif /* !MUME_FOUNDATION_CLASS_H */
