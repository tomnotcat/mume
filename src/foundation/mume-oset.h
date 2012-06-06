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
#ifndef MUME_FOUNDATION_OSET_H
#define MUME_FOUNDATION_OSET_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#include "mume-rbtree.h"

typedef struct rb_node mume_oset_node_t;

struct mume_oset_s {
    struct rb_node *root;
    size_t count;
    mume_cmpfcn_t *eltcmp;
    mume_desfcn_t *eltdes;
    void *param;
};

mume_public mume_oset_t* mume_oset_ctor(
    mume_oset_t *set, mume_cmpfcn_t *eltcmp,
    mume_desfcn_t *eltdes, void *param);
mume_public mume_oset_t* mume_oset_dtor(
    mume_oset_t *set);
mume_public int mume_oset_insert(
    mume_oset_t *set, mume_oset_node_t *node);
mume_public void mume_oset_erase(
    mume_oset_t *set, mume_oset_node_t *node);
mume_public mume_oset_node_t* mume_oset_find(
    const mume_oset_t *set, const void *obj);
mume_public void mume_oset_clear(mume_oset_t *set);

#define mume_oset_first(_set) rb_first((struct rb_root*)(_set))
#define mume_oset_last(_set) rb_last((struct rb_root*)(_set))
#define mume_oset_next(_node) rb_next(_node)
#define mume_oset_prev(_node) rb_prev(_node)
#define mume_oset_data(_node) \
    ((void*)((char*)(_node) + sizeof(mume_oset_node_t)))

#define mume_oset_newnode(_datasize) \
    ((mume_oset_node_t*)malloc_abort( \
        sizeof(mume_oset_node_t) + (_datasize)))
#define mume_oset_delnode(_node) free(_node)
#define mume_oset_new(_eltcmp, _eltdes, _param) \
    mume_oset_ctor(malloc_struct(mume_oset_t), \
                   _eltcmp, _eltdes, _param)
#define mume_oset_delete(_set) \
    free(mume_oset_dtor(_set))
#define mume_oset_size(_set) ((size_t)(_set)->count)

mume_public mume_oset_node_t* mume_oset_new_name_node(
    const char *name, size_t size);

#define mume_oset_foreach(_set, _nd, _obj) \
    for (_nd = mume_oset_first(_set); \
         (_obj = (_nd ? (typeof(_obj))mume_oset_data(_nd) : NULL)); \
         _nd = mume_oset_next(_nd))

#define mume_oset_foreach_safe(_oset, _nd, _obj, _tn) \
    for (_nd = mume_oset_first(_set), \
         _tn = _nd ? mume_oset_next(_nd) : NULL; \
         (_obj = (_nd ? (typeof(_obj))mume_oset_data(_nd) : NULL)); \
         _nd = _tn, _tn = _nd ? mume_oset_next(_nd) : NULL)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_OSET_H */
