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
#include MUME_STRING_H

static int _int_cmp(const void *v1, const void *v2)
{
    return *(int*)v1 - *(int*)v2;
}

static int _int_cmpr(const void *v1, const void *v2)
{
    return *(int*)v2 - *(int*)v1;
}

static int _str_cmpr(const void *v1, const void *v2)
{
    return strcmp((const char*)v2, (const char*)v1);
}

static void _validate_heap(
    char *elmts, size_t num, size_t eltsize,
    mume_cmpfcn_t *cmp, mume_cmpfcn_t *equ, int reverse)
{
    size_t i;
    for (i = num; i > 0; --i) {
        mume_pop_heap(elmts, i, eltsize, cmp);
        if (reverse) {
            test_assert(equ(elmts + (i - 1) * eltsize, elmts) <= 0);
        }
        else {
            test_assert(equ(elmts + (i - 1) * eltsize, elmts) >= 0);
        }
    }
    mume_make_heap(elmts, num, eltsize, cmp);
}

static void _heap_test(
    char *buf, char *elmts, size_t num, size_t eltsize,
    mume_cmpfcn_t *cmp, mume_cmpfcn_t *cmpr)
{
    size_t i;
    for (i = 0; i < num; ++i) {
        memcpy(buf + i * eltsize, elmts + i * eltsize, eltsize);
        mume_push_heap(buf, i + 1, eltsize, cmp);
    }
    _validate_heap(buf, num, eltsize, cmp, cmp, 0);
    mume_make_heap(elmts, num, eltsize, cmp);
    _validate_heap(elmts, num, eltsize, cmp, cmp, 0);
    mume_sort_heap(elmts, num, eltsize, cmp);
    for (i = 0; i + 1 < num; ++i) {
        test_assert(cmp(elmts + i * eltsize,
                      elmts + (i + 1) * eltsize) <= 0);
    }
    /* reverse sort */
    mume_make_heap(elmts, num, eltsize, cmpr);
    _validate_heap(elmts, num, eltsize, cmpr, cmp, 1);
    mume_sort_heap(elmts, num, eltsize, cmpr);
    for (i = 0; i + 1 < num; ++i) {
        test_assert(cmp(elmts + i * eltsize,
                      elmts + (i + 1) * eltsize) >= 0);
    }
}

void test_heap_int(void)
{
    int buf1[10];
    int ints[10] = {10, 24, -79, 37, 19, 37, 69, -14, 1389, 4};
    _heap_test((char*)buf1, (char*)ints, 10,
               sizeof(int), _int_cmp, _int_cmpr);
}

void test_heap_string(void)
{
    char buf2[8][20];
    char strs[8][20] = {"zero", "hello", "what", "myname",
                        "child", "zero", "parent", "about"};
    _heap_test((char*)buf2, (char*)strs, 8, 20,
               (mume_cmpfcn_t*)strcmp, _str_cmpr);
}
