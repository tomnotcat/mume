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
#ifndef MUME_FOUNDATION_LIST_H
#define MUME_FOUNDATION_LIST_H

#include "mume-common.h"

MUME_BEGIN_DECLS

typedef struct mume_list_node_s mume_list_node_t;

struct mume_list_node_s {
    mume_list_node_t *prev;
    mume_list_node_t *next;
};

struct mume_list_s {
    mume_list_node_t *front;
    mume_list_node_t *back;
    size_t count;
    mume_desfcn_t *eltdes;
    void *param;
};

mume_public mume_list_t* mume_list_ctor(
    mume_list_t *lst, mume_desfcn_t *eltdes, void *param);
mume_public mume_list_t* mume_list_dtor(
    mume_list_t *lst);
mume_public mume_list_node_t* mume_list_insert(
    mume_list_t *lst, mume_list_node_t *pos, size_t size);
mume_public mume_list_node_t* mume_list_shift(
    mume_list_t *lst, mume_list_node_t *a, mume_list_node_t *b);
mume_public void mume_list_erase(
    mume_list_t *lst, mume_list_node_t *node);
mume_public void mume_list_clear(mume_list_t *lst);

mume_public mume_list_node_t* mume_list_find_object(
    const mume_list_t *self, void *object);

#define mume_list_new(_eltdes, _param) \
    mume_list_ctor(malloc_struct(mume_list_t), _eltdes, _param)
#define mume_list_delete(_lst) \
    free(mume_list_dtor(_lst))

#define mume_list_front(_lst) ((mume_list_node_t*)(_lst)->front)
#define mume_list_back(_lst) ((mume_list_node_t*)(_lst)->back)
#define mume_list_prev(_node) ((mume_list_node_t*)(_node)->prev)
#define mume_list_next(_node) ((mume_list_node_t*)(_node)->next)
#define mume_list_data(_node) \
    ((void*)((char*)(_node) + sizeof(mume_list_node_t)))
#define mume_list_size(_lst) ((size_t)(_lst)->count)
#define mume_list_empty(_lst) (0 == (_lst)->front)

#define mume_list_foreach(_lst, _nd, _obj) \
    for (_nd = mume_list_front(_lst); \
         (_obj = (_nd ? (typeof(_obj))mume_list_data(_nd) : NULL)); \
         _nd = mume_list_next(_nd))

#define mume_list_foreach_safe(_lst, _nd, _obj, _tn) \
    for (_nd = mume_list_front(_lst), \
         _tn = _nd ? mume_list_next(_nd) : NULL; \
         (_obj = (_nd ? (typeof(_obj))mume_list_data(_nd) : NULL)); \
         _nd = _tn, _tn = _nd ? mume_list_next(_nd) : NULL)

static inline mume_list_node_t* mume_list_push_front(
    mume_list_t *lst, size_t size)
{
    return mume_list_insert(lst, lst->front, size);
}

static inline void mume_list_pop_front(mume_list_t *lst)
{
    mume_list_erase(lst, lst->front);
}

static inline mume_list_node_t* mume_list_push_back(
    mume_list_t *lst, size_t size)
{
    return mume_list_insert(lst, 0, size);
}

static inline void mume_list_pop_back(mume_list_t *lst)
{
    mume_list_erase(lst, lst->back);
}

MUME_END_DECLS

#endif /* MUME_FOUNDATION_LIST_H */
