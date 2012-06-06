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
#ifndef MUME_FOUNDATION_CLSMGR_H
#define MUME_FOUNDATION_CLSMGR_H

#include "mume-class.h"

MUME_BEGIN_DECLS

#define MUME_CLSMGR_MESSAGE_LIST(_) \
    _(CLSMGR_REGISTER)

enum mume_clsmgr_message_e {
    MUME_CLSMGR_MESSAGE_FIRST = MUME_OBJECT_MESSAGE_LAST,
    MUME_CLSMGR_MESSAGE_LIST(MUME_DEFINE_MSGID)
    MUME_CLSMGR_MESSAGE_LAST
};

typedef struct mume_clsmgr {
    int n;
} mume_clsmgr_t;

mume_public const mume_class_t* mume_clsmgr_class(void);

#define mume_clsmgr_new() mume_new2(mume_clsmgr_class())

MUME_END_DECLS

#endif /* !MUME_FOUNDATION_CLSMGR_H */
