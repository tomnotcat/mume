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
#ifndef MUME_FOUNDATION_TREEVIEW_H
#define MUME_FOUNDATION_TREEVIEW_H

#include "mume-scrollview.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_TREEVIEW (MUME_SIZEOF_SCROLLVIEW + \
                              sizeof(void*) * 4 + \
                              sizeof(unsigned int))

#define MUME_SIZEOF_TREEVIEW_CLASS (MUME_SIZEOF_SCROLLVIEW_CLASS)

enum mume_treenode_flags_e {
    MUME_TREENODE_STATIC_NAME  = 1 << 0
};

enum mume_treeview_notify_e {
    MUME_TREEVIEW_CONTEXTMENU = MUME_SCROLLVIEW_NOTIFY_LAST,
    MUME_TREEVIEW_NOTIFY_LAST
};

mume_public const void* mume_treeview_class(void);

#define mume_treeview_meta_class mume_scrollview_meta_class

mume_public void* mume_treeview_new(
    void *parent, int x, int y, int width, int height);

/* Get the root node of the treeview. the root node is a "virtual"
 * node that dos not appear in the view, it is only used as an entry
 * to visit other "real" nodes user inserted. */
mume_public void* mume_treeview_root(const void *self);

/* Set/Get the current selected node. */
mume_public void mume_treeview_set_selected(void *self, void *node);

mume_public void* mume_treeview_get_selected(const void *self);

/* Insert a node to the tree view. Return the Inserted node. */
mume_public void* mume_treeview_insert(
    void *self, void *parent, void *prev,
    const char *text, unsigned int flags);

/* Remove a node from the tree view. */
mume_public void mume_treeview_remove(void *self, void *node);

/* Remove all the children node of the specified node. */
mume_public void mume_treeview_remove_children(
    void *self, void *parent);

/* Set/Get the specified node's text. */
mume_public void mume_treeview_set_text(
    void *self, void *node, const char *text);

mume_public void mume_treeview_set_static_text(
    void *self, void *node, const char *text);

mume_public const char* mume_treeview_get_text(
    const void *self, const void *node);

/* Set/Get the specified node's data. */
mume_public void mume_treeview_set_data(
    void *self, void *node, void *data);

mume_public void* mume_treeview_get_data(
    const void *self, const void *node);

/* Expand/Collapse children nodes of the specified node. */
mume_public void mume_treeview_expand(
    void *self, void *node, int recur);

mume_public void mume_treeview_collapse(
    void *self, void *node, int recur);

/* Get the first visible node in the view. This may change after
 * scroll changed or some node operation. */
mume_public void* mume_treeview_first_visible(void *self);

/* Return the node under the specified vertical coordinate.
 * Return NULL if there's none. */
mume_public void* mume_treeview_node_from(const void *self, int y);

/* Scroll the view to ensure the specified node to be visible. */
mume_public void mume_treeview_ensure_visible(
    void *self, const void *node);

mume_public mume_type_t* mume_typeof_treeview_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TREEVIEW_H */
