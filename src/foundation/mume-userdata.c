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
#include "mume-userdata.h"
#include "mume-config.h"
#include "mume-memory.h"
#include MUME_STDINT_H
#include MUME_STDLIB_H

struct _user_data_record {
    struct rb_node node;
    const mume_user_data_key_t *key;
    void *data;
    mume_destroy_func_t *destroy;
};

mume_user_data_t* mume_user_data_ctor(mume_user_data_t *self)
{
    self->root = NULL;
    return self;
}

mume_user_data_t* mume_user_data_dtor(mume_user_data_t *self)
{
    struct rb_node *node = rb_first((struct rb_root*)self);
    struct rb_node *next;
    struct _user_data_record *u;
    while (node) {
        next = rb_next(node);
        u = (struct _user_data_record*)node;
        if (u->destroy)
            u->destroy(u->data);

        rb_erase(node, (struct rb_root*)self);
        free(node);
        node = next;
    }

    return self;
}

void* mume_user_data_new(void)
{
    return mume_user_data_ctor(
        malloc_abort(sizeof(mume_user_data_t)));
}

void mume_user_data_delete(mume_user_data_t *self)
{
    if (self)
        free(mume_user_data_dtor(self));
}

void mume_user_data_set(
    mume_user_data_t *self, const mume_user_data_key_t *key,
    void *data, void (*destroy)(void*))
{
    struct rb_node **new = &(self->root), *parent = NULL;
    int r;
    struct _user_data_record *u;
    /* Figure out where to put new node */
    while (*new) {
        u = (struct _user_data_record*)(*new);
        r = (intptr_t)key - (intptr_t)u->key;
        parent = *new;
        if (r < 0)
            new = &((*new)->rb_left);
        else if (r > 0)
            new = &((*new)->rb_right);
        else {
            if (u->destroy)
                u->destroy(u->data);

            u->data = data;
            u->destroy = destroy;
            return;
        }
    }
    /* Add new node and rebalance tree. */
    u = malloc_abort(sizeof(*u));
    u->key = key;
    u->data = data;
    u->destroy = destroy;
    rb_link_node(&u->node, parent, new);
    rb_insert_color(&u->node, (struct rb_root*)self);
}

void* mume_user_data_get(
    mume_user_data_t *self, const mume_user_data_key_t *key)
{
    int r;
    struct _user_data_record *u;
    struct rb_node *node = self->root;
    while (node) {
        u = (struct _user_data_record*)node;
        r = (intptr_t)key - (intptr_t)u->key;
        if (r < 0)
            node = node->rb_left;
        else if (r > 0)
            node = node->rb_right;
        else
            return u->data;
    }

    return NULL;
}
