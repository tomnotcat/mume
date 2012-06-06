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
#include "mume-oset.h"
#include "mume-config.h"
#include "mume-memory.h"
#include MUME_ASSERT_H
#include MUME_STDLIB_H
#include MUME_STRING_H

mume_oset_t* mume_oset_ctor(
    mume_oset_t *set, mume_cmpfcn_t *eltcmp,
    mume_desfcn_t *eltdes, void *param)
{
    set->root = NULL;
    set->count = 0;
    set->eltcmp = eltcmp;
    set->eltdes = eltdes;
    set->param = param;
    return set;
}

mume_oset_t* mume_oset_dtor(mume_oset_t *set)
{
    if (set) {
        mume_oset_node_t *node = mume_oset_first(set);
        mume_oset_node_t *next;
        while (node) {
            next = mume_oset_next(node);
            rb_erase(node, (struct rb_root*)set);
            if (set->eltdes)
                set->eltdes(mume_oset_data(node), set->param);

            free(node);
            node = next;
        }
    }

    return set;
}

int mume_oset_insert(mume_oset_t *set, mume_oset_node_t *node)
{
    struct rb_node **new = &(set->root), *parent = NULL;
    /* Figure out where to put new node */
    while (*new) {
        int result = set->eltcmp(
            mume_oset_data(node), mume_oset_data(*new));
        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return 0;

    }
    /* Add new node and rebalance tree. */
    rb_link_node(node, parent, new);
    rb_insert_color(node, (struct rb_root*)set);
    set->count += 1;
    return 1;
}

void mume_oset_erase(mume_oset_t *set, mume_oset_node_t *node)
{
    rb_erase(node, (struct rb_root*)set);
    if (set->eltdes)
        set->eltdes(mume_oset_data(node), set->param);
    free(node);
    set->count -= 1;
}

mume_oset_node_t* mume_oset_find(
    const mume_oset_t *set, const void *obj)
{
    struct rb_node *node = set->root;
    while (node) {
        int result = set->eltcmp(obj, mume_oset_data(node));
        if (result < 0)
            node = node->rb_left;
        else if (result > 0)
            node = node->rb_right;
        else
            return node;
    }
    return NULL;
}

void mume_oset_clear(mume_oset_t *set)
{
    mume_oset_node_t *n;
    mume_oset_node_t *p = mume_oset_first(set);
    while (p) {
        n = mume_oset_next(p);
        rb_erase(p, (struct rb_root*)set);
        if (set->eltdes)
            set->eltdes(mume_oset_data(p), set->param);
        free(p);
        p = n;
    }
    set->count = 0;
}

mume_oset_node_t* mume_oset_new_name_node(
    const char *name, size_t size)
{
    char **np;
    mume_oset_node_t *nd;
    assert(size >= sizeof(char*));
    nd = mume_oset_newnode(size + strlen(name) + 1);
    np = (char**)mume_oset_data(nd);
    *np = (char*)np + size;
    strcpy(*np, name);
    return nd;
}
