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
#ifndef MUME_TEST_UTIL_H
#define MUME_TEST_UTIL_H

#include "mume-gui.h"

MUME_BEGIN_DECLS

extern const char *test_name;

void test_fail(const char *file, int line,
               const char *fcn, const char *msg);

#define test_assert(_cond) \
    do { \
        if (!(_cond)) { \
            test_fail(__FILE__, __LINE__, __func__, \
                      "Failed condition: " #_cond); \
        } \
    } while (0)

#define test_run(_test) \
    do { \
        (_test)(); \
    } while (0)

#define test_decl_run(_test) \
    do { \
        extern void _test(void); \
        (_test)(); \
    } while (0)

void* test_setup_toplevel_window(void *win);

void* test_teardown_toplevel_window(void *win);

int test_is_break_event(mume_event_t *evt, void *main);

void test_run_event_loop(void *main);

void* test_window_new(void *parent, int x, int y, int w, int h);

void test_window_set_color(void *self, mume_color_t color);

MUME_END_DECLS

#endif /* MUME_TEST_UTIL_H */
