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
#include "mume-list.h"
#include "mume-config.h"
#include "mume-memory.h"
#include MUME_STDLIB_H

static inline void _list_node_disconnect(
    mume_list_t *lst, mume_list_node_t *node)
{
    if (node->prev) {
        node->prev->next = node->next;
    }
    else {
        lst->front = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
    else {
        lst->back = node->prev;
    }
}

mume_list_t* mume_list_ctor(
    mume_list_t *lst, mume_desfcn_t *eltdes, void *param)
{
    lst->front = NULL;
    lst->back = NULL;
    lst->count = 0;
    lst->eltdes = eltdes;
    lst->param = param;
    return lst;
}

mume_list_t* mume_list_dtor(mume_list_t *lst)
{
    if (lst) {
        mume_list_node_t *node = mume_list_front(lst);
        mume_list_node_t *next;
        while (node) {
            next = mume_list_next(node);
            if (lst->eltdes)
                lst->eltdes(mume_list_data(node), lst->param);

            free(node);
            node = next;
        }
    }

    return lst;
}

mume_list_node_t* mume_list_insert(
    mume_list_t *lst, mume_list_node_t *pos, size_t size)
{
    mume_list_node_t *node = malloc_abort(
        sizeof(mume_list_node_t) + size);
    if (pos) {
        node->next = pos;
        node->prev = pos->prev;
        pos->prev = node;
        if (pos == lst->front)
            lst->front = node;
    }
    else {
        /* insert back */
        node->prev = lst->back;
        node->next = NULL;
        if (lst->back)
            lst->back->next = node;
        lst->back = node;
        if (NULL == lst->front)
            lst->front = node;
    }
    lst->count += 1;
    return node;
}

mume_list_node_t* mume_list_shift(
    mume_list_t *lst, mume_list_node_t *a, mume_list_node_t *b)
{
    if (a != b) {
        _list_node_disconnect(lst, a);
        if (b) {
            a->next = b;
            a->prev = b->prev;
            b->prev = a;
            if (b == lst->front)
                lst->front = a;
        }
        else {
            /* Shift to back. */
            a->prev = lst->back;
            a->next = NULL;
            if (lst->back)
                lst->back->next = a;
            lst->back = a;
            if (NULL == lst->front)
                lst->front = a;
        }
    }
    return a;
}

void mume_list_erase(mume_list_t *lst, mume_list_node_t *node)
{
    _list_node_disconnect(lst, node);
    if (lst->eltdes)
        lst->eltdes(mume_list_data(node), lst->param);
    free(node);
    lst->count -= 1;
}

void mume_list_clear(mume_list_t *lst)
{
    mume_list_node_t *n;
    mume_list_node_t *p = mume_list_front(lst);
    while (p) {
        n = mume_list_next(p);
        if (lst->eltdes)
            lst->eltdes(mume_list_data(p), lst->param);
        free(p);
        p = n;
    }
    lst->front = NULL;
    lst->back = NULL;
    lst->count = 0;
}

mume_list_node_t* mume_list_find_object(
    const mume_list_t *self, void *object)
{
    mume_list_node_t *node;
    void **data;

    mume_list_foreach(self, node, data) {
        if (*data == object)
            return node;
    }

    return NULL;
}
