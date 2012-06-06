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
#include "mume-heap.h"
#include "mume-config.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _parent(_node) ((0 == _node) ? 0 : ((_node - 1) / 2))
#define _lchild(_node) (_node + _node + 1)
#define _rchild(_node) (_node + _node + 2)

static void _swap(char *a, char *b, size_t size)
{
    char tmp;
    while (size-- > 0) {
        tmp = *a;
        *a++ = *b;
        *b++ = tmp;
    }
}

void mume_make_heap(void *base, size_t num, size_t size,
                     mume_cmpfcn_t *cmp)
{
    size_t parent = num / 2;
    do {
        mume_reheap_down(base, num, size, cmp, parent);
    } while (parent-- > 0);
}

void mume_sort_heap(void *base, size_t num, size_t size,
                     mume_cmpfcn_t *cmp)
{
    while (num > 1)
        mume_pop_heap(base, num--, size, cmp);
}

void mume_reheap_up(void *base, size_t num, size_t size,
                     mume_cmpfcn_t *cmp, size_t node)
{
    size_t parent = _parent(node);
    assert(node < num);
    while (node > 0 && cmp((const char*)base + node * size,
                           (const char*)base + parent * size) > 0)
    {
        _swap((char*)base + node * size,
              (char*)base + parent * size, size);
        node = parent;
        parent = _parent(node);
    }
}

void mume_reheap_down(void *base, size_t num, size_t size,
                       mume_cmpfcn_t *cmp, size_t node)
{
    size_t lchild = _lchild(node);
    size_t rchild = _rchild(node);
    size_t largest = node;
    assert(node >= 0);
    while (node < num) {
        if (lchild < num &&
            cmp((const char*)base + lchild * size,
                (const char*)base + node * size) > 0)
        {
            largest = lchild;
        }
        if (rchild < num &&
            cmp((const char*)base + rchild * size,
                (const char*)base + largest * size) > 0)
        {
            largest = rchild;
        }
        if (largest != node) {
            _swap((char*)base + node * size,
                  (char*)base + largest * size, size);
            node = largest;
            lchild = _lchild(node);
            rchild = _rchild(node);
        }
        else
            break;
    }
}

void mume_push_heap(void *base, size_t num, size_t size,
                     mume_cmpfcn_t *cmp)
{
    mume_reheap_up(base, num, size, cmp, num - 1);
}

void mume_pop_heap(void *base, size_t num, size_t size,
                    mume_cmpfcn_t *cmp)
{
    if (num > 1) {
        _swap(base, (char*)base + (num - 1) * size, size);
        mume_reheap_down(base, num - 1, size, cmp, 0);
    }
}
