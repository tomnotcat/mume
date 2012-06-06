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
#ifndef MUME_FOUNDATION_TABCTRL_H
#define MUME_FOUNDATION_TABCTRL_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_TABCTRL (MUME_SIZEOF_WINDOW +   \
                             sizeof(unsigned int) + \
                             sizeof(int) * 2 +      \
                             sizeof(void*) * 2)

#define MUME_SIZEOF_TABCTRL_CLASS (MUME_SIZEOF_WINDOW_CLASS)

enum mume_tabctrl_e {
    MUME_TABCTRL_TOP,
    MUME_TABCTRL_BOTTOM,
    MUME_TABCTRL_LEFT,
    MUME_TABCTRL_RIGHT
};

mume_public const void* mume_tabctrl_class(void);

#define mume_tabctrl_meta_class mume_window_meta_class

mume_public void* mume_tabctrl_new(
    void *parent, int x, int y, int w, int h, int type);

mume_public void mume_tabctrl_enable_close(void *self, int able);

mume_public void mume_tabctrl_insert_tab(
    void *self, int index, void *window);

mume_public void mume_tabctrl_remove_tab(void *self, int index);

mume_public int mume_tabctrl_tab_count(const void *self);

mume_public void* mume_tabctrl_tab_at(const void *self, int index);

static inline int mume_tabctrl_append_tab(void *self, void *window)
{
    int count = mume_tabctrl_tab_count(self);
    mume_tabctrl_insert_tab(self, count, window);
    return count;
}

mume_public int mume_tabctrl_tab_from_point(
    void *self, int x, int y);

mume_public int mume_tabctrl_tab_from_window(
    const void *self, void *window);

mume_public void mume_tabctrl_select_tab(void *self, int index);

mume_public int mume_tabctrl_get_selected(const void *self);

mume_public void mume_tabctrl_insert_tool(
    void *self, int index, cairo_surface_t *icon);

mume_public void mume_tabctrl_remove_tool(void *self, int index);

mume_public int mume_tabctrl_tool_count(const void *self);

mume_public void* mume_tabctrl_tool_at(const void *self, int index);

static inline int mume_tabctrl_append_tool(
    void *self, cairo_surface_t *icon)
{
    int count = mume_tabctrl_tool_count(self);
    mume_tabctrl_insert_tool(self, count, icon);
    return count;
}

mume_public int mume_tabctrl_tool_from_point(
    void *self, int x, int y);

mume_public int mume_tabctrl_tool_from_window(
    const void *self, void *window);

mume_public void mume_tabctrl_set_window(
    void *self, void *item, void *window);

mume_public void* mume_tabctrl_get_window(
    const void *self, const void *item);

mume_public void mume_tabctrl_set_text(
    void *self, void *item, const char *text);

mume_public const char* mume_tabctrl_get_text(
    const void *self, const void *item);

mume_public void mume_tabctrl_set_icon(
    void *self, void *item, cairo_surface_t *icon);

mume_public cairo_surface_t* mume_tabctrl_get_icon(
    const void *self, const void *item);

mume_public void mume_tabctrl_set_data(
    void *self, void *item, void *data);

mume_public void* mume_tabctrl_get_data(
    const void *self, const void *item);

mume_public mume_type_t* mume_typeof_tabctrl_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TABCTRL_H */
