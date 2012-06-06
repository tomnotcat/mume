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
#ifndef MUME_FOUNDATION_USER_DATA_H
#define MUME_FOUNDATION_USER_DATA_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#include "mume-rbtree.h"

struct mume_user_data_key_s {
    int unused;
};

struct mume_user_data_s {
    struct rb_node *root;
};

mume_public mume_user_data_t* mume_user_data_ctor(mume_user_data_t *self);

mume_public mume_user_data_t* mume_user_data_dtor(mume_user_data_t *self);

mume_public void* mume_user_data_new(void);

mume_public void mume_user_data_delete(mume_user_data_t *self);

mume_public void mume_user_data_set(
    mume_user_data_t *self, const mume_user_data_key_t *key,
    void *data, void (*destroy)(void*));

mume_public void* mume_user_data_get(
    mume_user_data_t *self, const mume_user_data_key_t *key);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_USER_DATA_H */
