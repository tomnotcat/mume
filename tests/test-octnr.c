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

int deleted = 0;

static void _test_octnr(void *octnr, int count)
{
    void *obj, *no;
    char buf[256];
    void *it, *ni;
    int i;

    test_assert(mume_octnr_size(octnr) == 0);
    test_assert(mume_octnr_begin(octnr) == mume_octnr_end(octnr));

    for (i = 0; i < count; ++i) {
        snprintf(buf, sizeof(buf), "%d", i);
        obj = mume_property_new(0, buf, i, 0);
        it = mume_octnr_insert(octnr, mume_octnr_end(octnr), obj);
        test_assert(it != mume_octnr_end(octnr));
        test_assert(mume_octnr_value(octnr, it) == obj);
    }

    i = 0;
    obj = NULL;
    it = mume_octnr_begin(octnr);
    while (it != mume_octnr_end(octnr)) {
        obj = mume_octnr_value(octnr, it);
        snprintf(buf, sizeof(buf), "%d", mume_property_get_id(obj));
        test_assert(0 == strcmp(buf, mume_property_get_name(obj)));

        ++i;
        it = mume_octnr_next(octnr, it);
    }

    it = mume_octnr_find(octnr, obj);
    test_assert(it != mume_octnr_end(octnr));
    test_assert(mume_octnr_next(octnr, it) == mume_octnr_end(octnr));
    test_assert(mume_octnr_find(octnr, NULL) == mume_octnr_end(octnr));

    test_assert(count == i);
    test_assert(mume_octnr_size(octnr) == count);

    it = mume_octnr_begin(octnr);
    for (i = 0; i < count / 2; ++i) {
        ni = mume_octnr_next(octnr, it);
        no = mume_octnr_value(octnr, ni);
        it = mume_octnr_erase(octnr, it);
        test_assert(no == mume_octnr_value(octnr, it));
    }

    test_assert(mume_octnr_size(octnr) == count / 2);

    mume_octnr_clear(octnr);
    test_assert(mume_octnr_size(octnr) == 0);
    test_assert(mume_octnr_begin(octnr) == mume_octnr_end(octnr));
}

static void _mydelete(void *obj)
{
    ++deleted;
    mume_delete(obj);
}

void all_tests(void)
{
    void *set = mume_ooset_new(_mydelete);
    void *list = mume_olist_new(_mydelete);
    void *vect = mume_ovector_new(_mydelete);

    _test_octnr(set, 500);
    test_assert(500 == deleted);
    _test_octnr(list, 200);
    test_assert(700 == deleted);
    _test_octnr(vect, 1000);
    test_assert(1700 == deleted);

    mume_delete(set);
    mume_delete(list);
    mume_delete(vect);
}
