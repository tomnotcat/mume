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
#include "mume-class.h"
#include "mume-debug.h"
#include "mume-error.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include MUME_ASSERT_H

static int _object_message(mume_object_t *obj, mume_message_t *msg)
{
    return 0;
}

const mume_class_t* mume_object_class2(void)
{
    static const mume_class_t *clazz;
    if (NULL == clazz) {
        clazz = mume_register_class(
            "object",
            sizeof(mume_class_t),
            NULL,
            NULL,
            NULL,
            0,
            _object_message);
    }

    return clazz;
}

mume_object_t mume_new2(const mume_class_t *clazz, ...)
{
    mume_object_t object;
    mume_message_t message;

    assert(clazz->data_size > 0 && clazz->message);

    object.clazz = clazz;
    object.data = malloc_abort(clazz->data_size);

    MUME_MSG_OBJECT_INITIALIZE_init(&message);
    if (mume_send_message(&object, &message)) {
        free(object.data);
        object.data = NULL;
    }

    mume_message_finalize(&message);

    return object;
}

void mume_delete2(mume_object_t object)
{
    mume_message_t message;

    MUME_MSG_OBJECT_FINALIZE_init(&message);
    if (mume_send_message(&object, &message)) {
        mume_error(("Finalize object error: %s\n",
                    mume_get_errstr()));
    }
    else {
        free(object.data);
    }

    mume_message_finalize(&message);
}
