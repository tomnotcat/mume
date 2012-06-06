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
#include "mume-base.h"
#include "test-util.h"
#include MUME_STDLIB_H

static int my_comp(const void *v1, const void *v2)
{
    return *(int*)v1 - *(int*)v2;
}

void test_mume_list(void)
{
    mume_list_t *mylst;
    mume_list_node_t *node = NULL;
    mume_list_node_t *next;
    int i, vals[] = {10, 100, -10, 60, 80, -50};
    mylst = mume_list_new(NULL, NULL);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        node = mume_list_push_back(mylst, sizeof(int));
        *(int*)mume_list_data(node) = vals[i];
    }
    test_assert(COUNT_OF(vals) == mume_list_size(mylst));
    node = mume_list_front(mylst);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        next = mume_list_next(node);
        test_assert(*(int*)mume_list_data(node) == vals[i]);
        mume_list_erase(mylst, node);
        node = next;
    }
    test_assert(0 == mume_list_size(mylst));
    mume_list_delete(mylst);
}

void test_mume_vector(void)
{
    mume_vector_t *myvec;
    int *ptr, count, i;
    int vals[] = {
        -7, -10, 1043, 62, 100
    };
    myvec = mume_vector_new(sizeof(int), NULL, NULL);
    *(int*)mume_vector_push_back(myvec) = 1;
    test_assert(1 == *(int*)mume_vector_front(myvec));
    mume_vector_erase(myvec, 0, 1);
    ptr = (int*)mume_vector_insert(myvec, 0, 4);
    ptr[0] = ptr[1] = ptr[2] = ptr[3] = 2;
    test_assert(4 == mume_vector_size(myvec));
    *(int*)mume_vector_insert(myvec, 0, 1) = 1;
    *(int*)mume_vector_insert(myvec, 0, 1) = 1;
    *(int*)mume_vector_push_back(myvec) = 3;
    *(int*)mume_vector_push_back(myvec) = 3;
    test_assert(8 == mume_vector_size(myvec));
    for (i = 0; i < 2; ++i)
        test_assert(1 == *(int*)mume_vector_at(myvec, i));
    for (i = 2; i < 6; ++i)
        test_assert(2 == *(int*)mume_vector_at(myvec, i));
    for (i = 6; i < 8; ++i)
        test_assert(3 == *(int*)mume_vector_at(myvec, i));
    mume_vector_erase(myvec, 2, 3);
    mume_vector_pop_back(myvec);
    mume_vector_erase(myvec, 0, 1);
    test_assert(3 == mume_vector_size(myvec));
    test_assert(1 == *(int*)mume_vector_at(myvec, 0));
    test_assert(2 == *(int*)mume_vector_at(myvec, 1));
    test_assert(3 == *(int*)mume_vector_at(myvec, 2));
    test_assert(!mume_vector_empty(myvec));
    mume_vector_clear(myvec);
    test_assert(0 == mume_vector_size(myvec));
    test_assert(mume_vector_empty(myvec));
    count = sizeof(vals) / sizeof(int);
    for (i = 0; i < count; ++i)
        *(int*)mume_vector_insert(myvec, 0, 1) = vals[i];
    test_assert(100 == *(int*)mume_vector_front(myvec));
    test_assert(-7 == *(int*)mume_vector_back(myvec));
    mume_vector_clear(myvec);
    for (i = 0; i < count; ++i)
        *(int*)mume_vector_push_back(myvec) = vals[i];
    test_assert(-7 == *(int*)mume_vector_front(myvec));
    test_assert(100 == *(int*)mume_vector_back(myvec));
    test_assert(count == mume_vector_size(myvec));
    mume_vector_delete(myvec);
}

void test_mume_oset(void)
{
    mume_oset_t *myset;
    mume_oset_node_t *node;
    int i, vals[] = {10, 100, -10, 60, 80, -50};
    myset = mume_oset_new(my_comp, NULL, NULL);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        node = mume_oset_newnode(sizeof(int));
        *(int*)mume_oset_data(node) = vals[i];
        test_assert(mume_oset_insert(myset, node));
    }

    for (i = 0; i < COUNT_OF(vals); ++i) {
        test_assert(mume_oset_find(myset, vals + i));
        node = mume_oset_newnode(sizeof(int));
        *(int*)mume_oset_data(node) = vals[i];
        test_assert(!mume_oset_insert(myset, node));
        mume_oset_delnode(node);
    }

    mume_oset_delete(myset);
}
