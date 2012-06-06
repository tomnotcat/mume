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
#ifndef MUME_FOUNDATION_MESSAGE_H
#define MUME_FOUNDATION_MESSAGE_H

#include "mume-common.h"

MUME_BEGIN_DECLS

typedef struct mume_message {
    int id;
    const char *name;
    union {
        char s[32];
        void *d;
    } data;
    size_t rd_ptr;
    size_t wr_ptr;
    void (*finalize)(struct mume_message *msg);
} mume_message_t;

#define MUME_DEFINE_MSGID(MESSAGE) MUME_MSG_##MESSAGE,

#define MUME_DEFINE_MESSAGE0(MESSAGE) \
    static inline const char* MESSAGE##_name(void) \
    { \
        return #MESSAGE; \
    } \
    static inline void MESSAGE##_init(mume_message_t *msg) \
    { \
        msg->id = MESSAGE; \
        msg->name = MESSAGE##_name(); \
        msg->rd_ptr = 0; \
        msg->wr_ptr = 0; \
        msg->finalize = mume_message_default_finalize; \
    }

mume_public void mume_message_default_finalize(mume_message_t *msg);

static inline void mume_message_finalize(mume_message_t *msg)
{
    msg->finalize(msg);
}

MUME_END_DECLS

#endif /* !MUME_FOUNDATION_MESSAGE_H */
