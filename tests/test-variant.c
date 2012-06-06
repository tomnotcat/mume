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

static void _test_type_int(void)
{
    int vals[] = { 1, -100, 20, -8, 17, 0, 64 };
    int i;
    void *var = mume_variant_new(MUME_TYPE_INT);

    test_assert(mume_variant_get_int(var) == 0);

    for (i = 0; i < COUNT_OF(vals); ++i) {
        mume_variant_set_int(var, vals[i]);
        test_assert(mume_variant_get_int(var) == vals[i]);
    }

    test_assert(mume_variant_get_type(var) == MUME_TYPE_INT);
    mume_delete(var);
}

static void _test_type_float(void)
{
    float vals[] = { 1.14, -99.28, 19.0001, -27, 0.0, 32.23 };
    void *var = mume_variant_new(MUME_TYPE_FLOAT);
    int i;

    test_assert(fabs(mume_variant_get_float(var)) < 0.0001);

    for (i = 0; i < COUNT_OF(vals); ++i) {
        mume_variant_set_float(var, vals[i]);
        test_assert(
            fabs(mume_variant_get_float(var) - vals[i]) < 0.0001);
    }

    test_assert(mume_variant_get_type(var) == MUME_TYPE_FLOAT);
    mume_delete(var);
}

static void _test_type_double(void)
{
    double vals[] = { 1.00000014, -99.20000008, 19.00010000001,
                      -27.0000023001, 0.0, 32.233421323423 };
    void *var = mume_variant_new(MUME_TYPE_DOUBLE);
    int i;

    test_assert(abs(mume_variant_get_double(var)) < 0.0001);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        mume_variant_set_double(var, vals[i]);
        test_assert(abs(
            mume_variant_get_double(var) - vals[i]) < 0.0000000001);
    }

    test_assert(mume_variant_get_type(var) == MUME_TYPE_DOUBLE);
    mume_delete(var);
}

static void _test_type_string(void)
{
    const char* vals[] = { "a", "hello", "world", "more string" };
    void *var = mume_variant_new(MUME_TYPE_STRING);
    int i;

    test_assert(mume_variant_get_string(var) == NULL);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        mume_variant_set_string(var, vals[i]);
        test_assert(mume_variant_get_string(var) != vals[i]);
        test_assert(0 == strcmp(
            mume_variant_get_string(var), vals[i]));
        mume_variant_set_static_string(var, vals[i]);
        test_assert(mume_variant_get_string(var) == vals[i]);
    }

    test_assert(mume_variant_get_type(var) == MUME_TYPE_STRING);
    mume_delete(var);
}

static void _test_type_object(void)
{
    void* obj;
    void *var = mume_variant_new(MUME_TYPE_OBJECT);
    int i;

    test_assert(mume_variant_get_object(var) == NULL);
    for (i = MUME_TYPE_INT; i <= MUME_TYPE_OBJECT; ++i) {
        obj = mume_variant_new(i);

        if (MUME_TYPE_STRING == i) {
            mume_variant_set_string(obj, "dummy");
        }
        else if (MUME_TYPE_OBJECT == i) {
            mume_variant_set_object(obj, obj);
            mume_variant_set_object(obj, obj);
        }

        mume_variant_set_object(var, obj);
        test_assert(mume_variant_get_object(var) != obj);
        test_assert(0 == mume_compare(
            mume_variant_get_object(var), obj));
        mume_variant_set_static_object(var, obj);
        test_assert(mume_variant_get_object(var) == obj);
        mume_delete(obj);
    }

    test_assert(mume_variant_get_type(var) == MUME_TYPE_OBJECT);
    mume_delete(var);
}

static void _test_type_convert(void)
{
    void *var;

    var = mume_variant_new(MUME_TYPE_INT);

    mume_variant_set_int(var, -10);
    test_assert(mume_variant_convert(var, MUME_TYPE_INT));
    test_assert(mume_variant_convert(var, MUME_TYPE_STRING));
    test_assert(mume_variant_convert(var, MUME_TYPE_STRING));
    test_assert(!mume_variant_convert(var, MUME_TYPE_OBJECT));
    test_assert(0 == strcmp(mume_variant_get_string(var), "-10"));
    test_assert(mume_variant_convert(var, MUME_TYPE_FLOAT));
    test_assert(fabs(mume_variant_get_float(var) + 10) < 0.0001);
    test_assert(!mume_variant_convert(var, MUME_TYPE_OBJECT));

    mume_variant_set_float(var, 3.141592);
    test_assert(mume_variant_convert(var, MUME_TYPE_FLOAT));
    test_assert(mume_variant_convert(var, MUME_TYPE_STRING));
    test_assert(0 == strcmp(
        mume_variant_get_string(var), "3.141592"));
    test_assert(mume_variant_convert(var, MUME_TYPE_DOUBLE));
    test_assert(fabs(
        mume_variant_get_double(var) - 3.141592) < 0.0001);
    test_assert(!mume_variant_convert(var, MUME_TYPE_OBJECT));

    mume_variant_set_double(var, 1.000000000001);
    test_assert(mume_variant_convert(var, MUME_TYPE_DOUBLE));
    test_assert(mume_variant_convert(var, MUME_TYPE_STRING));
    test_assert(0 == strcmp(
        mume_variant_get_string(var), "1.000000000001"));
    test_assert(mume_variant_convert(var, MUME_TYPE_INT));
    test_assert(mume_variant_get_int(var) == 1);
    test_assert(!mume_variant_convert(var, MUME_TYPE_OBJECT));

    mume_delete(var);

    var = mume_variant_new(MUME_TYPE_OBJECT);
    test_assert(!mume_variant_convert(var, MUME_TYPE_INT));
    test_assert(!mume_variant_convert(var, MUME_TYPE_FLOAT));
    test_assert(!mume_variant_convert(var, MUME_TYPE_DOUBLE));
    test_assert(!mume_variant_convert(var, MUME_TYPE_STRING));
    test_assert(mume_variant_convert(var, MUME_TYPE_OBJECT));
    mume_delete(var);
}

void all_tests(void)
{
    _test_type_int();
    _test_type_float();
    _test_type_double();
    _test_type_string();
    _test_type_object();
    _test_type_convert();
}
