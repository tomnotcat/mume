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
#ifndef MUME_FOUNDATION_HEAP_H
#define MUME_FOUNDATION_HEAP_H

#include "mume-common.h"

MUME_BEGIN_DECLS

mume_public void mume_make_heap(
    void *base, size_t num, size_t size, mume_cmpfcn_t *cmp);
mume_public void mume_sort_heap(
    void *base, size_t num, size_t size, mume_cmpfcn_t *cmp);
/* restore the heap property after an insertion */
mume_public void mume_reheap_up(
    void *base, size_t num, size_t size,
    mume_cmpfcn_t *cmp, size_t node);
/* restore the heap property after a deletion */
mume_public void mume_reheap_down(
    void *base, size_t num, size_t size,
    mume_cmpfcn_t *cmp, size_t node);
mume_public void mume_push_heap(
    void *base, size_t num, size_t size, mume_cmpfcn_t *cmp);
mume_public void mume_pop_heap(
    void *base, size_t num, size_t size, mume_cmpfcn_t *cmp);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_HEAP_H */
