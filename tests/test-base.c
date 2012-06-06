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
#include "test-util.h"

const char* all_tests(void)
{
    test_decl_run(test_mume_list);
    test_decl_run(test_mume_vector);
    test_decl_run(test_mume_oset);
    test_decl_run(test_objbase_common);
    test_decl_run(test_objbase_serialize);
    test_decl_run(test_objbase_include);
    test_decl_run(test_types_simple);
    test_decl_run(test_types_compound);
    test_decl_run(test_types_container);
    test_decl_run(test_types_enumeration);
    test_decl_run(test_heap_int);
    test_decl_run(test_heap_string);
    test_decl_run(test_time_add);
    test_decl_run(test_time_sub);
    test_decl_run(test_time_cmp);
    test_decl_run(test_stream_byteorder);
    test_decl_run(test_virtfs_native);
    test_decl_run(test_virtfs_zip);
    test_decl_run(test_thread_mutex);
    test_decl_run(test_thread_semaphore);
    return 0;
}
