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
#ifndef MUME_FOUNDATION_WINDOW_H
#define MUME_FOUNDATION_WINDOW_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_WINDOW (MUME_SIZEOF_OBJECT +    \
                            sizeof(void*) * 4 +     \
                            sizeof(int) * 4 +       \
                            sizeof(unsigned int))

#define MUME_SIZEOF_WINDOW_CLASS (MUME_SIZEOF_CLASS + \
                                  sizeof(voidf*) * 25)

#define MUME_WINDOW_MAX_WIDTH (65535)
#define MUME_WINDOW_MAX_HEIGHT (65535)
#define MUME_WINDOW_NOTIFY_LAST 0

enum mume_paint_mode_e {
    /* Paint to invalid (dirty) region */
    MUME_PM_INVALID = 0,
    /* Paint to visible area, clip window and children */
    MUME_PM_VISIBLE,
    /* Paint to window, clip window */
    MUME_PM_WINDOW,
    /* Paint to target, no clip */
    MUME_PM_TARGET
};

mume_public const void* mume_window_class(void);

mume_public const void* mume_window_meta_class(void);

mume_public void* mume_window_new(
    void *parent, int x, int y, int width, int height);

/* Map/Unmap window, the window will not be visible
 * unless it and all its ancestors have been mapped. */
mume_public void _mume_window_map(void *self, int sync);

mume_public void _mume_window_unmap(void *self, int sync);

#define mume_window_map(_self) _mume_window_map(_self, 1)

#define mume_window_unmap(_self) _mume_window_unmap(_self, 1)

mume_public int mume_window_is_mapped(const void *self);

/* Check if all the window's ancestors are mapped. */
mume_public int mume_is_ancestors_mapped(const void *self);

mume_public void mume_map_children(void *self);

mume_public void mume_unmap_children(void *self);

/* Enable/Disable mouse and keyboard input to the
 * specified window. */
mume_public void mume_window_enable(void *self, int able);

mume_public int mume_window_is_enabled(const void *self);

/* Change the parent of a window. <x>, <y> are the
 * coordinates relative to the new parent. */
mume_public void mume_window_reparent(
    void *self, void *parent, int x, int y);

mume_public int mume_window_is_ancestor(
    const void *self, const void *other);

/* Functions for iterate the windows tree. */
mume_public void* mume_window_parent(const void *self);

mume_public void* mume_window_sibling(const void *self, int x);

mume_public void* mume_window_last_leaf(const void *self);

mume_public void* mume_window_next_node(const void *self, int down);

mume_public void* mume_window_prev_node(const void *self, int up);

#define mume_window_first_child(_self) \
    mume_window_child_at(_self, 0)

#define mume_window_last_child(_self) \
    mume_window_child_at(_self, -1)

#define mume_window_next_sibling(_self) \
    mume_window_sibling(_self, 1)

#define mume_window_prev_sibling(_self) \
    mume_window_sibling(_self, -1)

/* A focusable window will gain keyboard focus through
 * focus navigation key TAB. */
mume_public void mume_window_focusable(void *self, int able);

mume_public int mume_window_is_focusable(const void *self);

/* Find the next/prev window that will gain focus
 * after <cur>, under the common ancestor <anc>. */
mume_public void* mume_window_next_focus(
    const void *anc, const void *cur);

mume_public void* mume_window_prev_focus(
    const void *anc, const void *cur);

/* Set/Get backwin of the specified window. The backwin is
 * an abstract window of the underlying backend. */
mume_public void mume_window_set_backwin(void *self, void *bwin);

mume_public void* mume_window_get_backwin(const void *self);

/* Seek the window's host backwin. <x>, <y> will receive
 * the coordinates of the window relative to the target. */
mume_public void* mume_window_seek_backwin(
    const void *self, int *x, int *y);

/* Synchronize states, geometry, etc. of the window with
 * its attached backwin.
 *
 * <update> :
 *   if nonzero, change the windows's data according
 *   to the backwin, otherwise, change the backwin's
 *   data according to the window.
 */
mume_public void mume_window_sync_backwin(void *self, int update);

/* Query children of a window, the returned array
 * is in current stack order, from bottom to top.
 *
 * <num> :
 *   if not NULL, receive the children number.
 * <alter> :
 *   whether the returned array will be altered, if <alter>
 *   is zero, the returned array may directly point to the
 *   internal children array.
 *
 * User should call mume_free_children to free the
 * children array no matter what <alter> is passed.
 */
mume_public void** mume_query_children(
    const void *self, unsigned int *num, int alter);

mume_public void mume_free_children(
    const void *self, void **children);

/* Set/Get a identifier to a window, the id can be used to
 * look up this window from its parent via mume_find_child.
 */
mume_public void mume_window_set_id(void *self, int id);

mume_public int mume_window_get_id(const void *self);

mume_public void* mume_find_child(const void *self, int id);

/* Set/Get window's text. */
mume_public void mume_window_set_text(void *self, const char *text);

mume_public const char* mume_window_get_text(const void *self);

/* Get the child window by stack order <i>.
 *
 * The index can be negative, indicate a reverse order.
 */
mume_public void* mume_window_child_at(const void *self, int i);

/* Find the immediate child window that under the
 * specified point.
 *
 * Return the child under the point, if the child is
 * mapped or <mapped> is 0.
 *
 * Return <self> if the point is inside <win>, but no
 * child under that point.
 *
 * Otherwise return NULL.
 */
mume_public void* mume_child_from_point(
    const void *self, int x, int y, int mapped);

/* Find the visible and enabled window that under the
 * specified coordinates.
 *
 * Return the mapped and enabled child (or grandchild)
 * under the point if exists.
 *
 * Return <self> if the point is inside <win>, but no child
 * under that point, and <self> is visible and enabled.
 *
 * Otherwise return NULL.
 */
mume_public void* mume_window_from_point(
    const void *self, int x, int y);

/* Get the size hint of the window. Any of the output
 * parameter can be NULL. */
mume_public void mume_window_size_hint(
    void *self, int *min_width, int *min_height,
    int *fit_width, int *fit_height,
    int *max_width, int *max_height);

/* Set/Get the geometry of a window, relative to its parent.
 * Any of the output parameter can be NULL. */
mume_public void _mume_window_set_geometry(
    void *self, int x, int y, int width, int height, int sync);

#define mume_window_set_geometry(_self, _x, _y, _width, _height) \
    _mume_window_set_geometry(_self, _x, _y, _width, _height, 1)

mume_public void mume_window_get_geometry(
    const void *self, int *px, int *py, int *pw, int *ph);

mume_public int mume_window_width(const void *self);

mume_public int mume_window_height(const void *self);

/* Move a window relative to its parent. */
mume_public void mume_window_move_to(void *self, int x, int y);

/* Move a window relative to its current position. */
mume_public void mume_window_move_by(void *self, int x, int y);

/* Resize a window to a fixed size. */
mume_public void mume_window_resize_to(
    void *self, int width, int height);

/* Inflate or deflate a window's size. */
mume_public void mume_window_resize_by(
    void *self, int width, int height);

/* Center a window relative to another window. */
mume_public void mume_window_center(void *self, const void *dest);

/* Translate coordinates from one window to another. */
mume_public void mume_translate_coords(
    const void *src, const void *dest, int *x, int *y);

/* Move a window to the Top/Bottom of the stack. */
mume_public void mume_window_raise(void *self);

mume_public void mume_window_lower(void *self);

/* Restack a set of windows from top to bottom.
 * All the windows must have the same parent.
 */
mume_public void mume_window_restack(
    void **windows, unsigned int num);

/* Add/Remove the specified region to/from the update
 * region of the specified window.
 */
mume_public void mume_invalidate_region(
    void *self, const cairo_region_t *rgn);

mume_public void mume_validate_region(
    void *self, const cairo_region_t *rgn);

/* Add/Remove a rectangle to/from the update region
 * of the specified window.
 *
 * If <rect> is NULL, the entire area of the window
 * is added/removed.
 */
mume_public void mume_invalidate_rect(
    void *self, const mume_rect_t *rect);

mume_public void mume_validate_rect(
    void *self, const mume_rect_t *rect);

/* Get the invalid region extents of the last window that
 * receive the expose event. */
mume_public mume_rect_t mume_current_invalid_rect(void);

/* Create a window's region that clipped by its ancestors
 * and exclude all siblings that stack above the window.
 */
mume_public cairo_region_t* mume_window_region_create(
    const void *self);

/* Exclude the regions of <win>'s siblings that has stack
 * order higher than <stack> from <rgn>.
 */
mume_public void mume_window_region_clip_siblings(
    const void *self, cairo_region_t *rgn, int stack);

/* Exclude the regions of <win>'s children from <rgn>. */
mume_public void mume_window_region_clip_children(
    const void *self, cairo_region_t *rgn);

/* Get the update region of a window.
 *
 * User should not destroy or modify the returned
 * region directly.
 */
mume_public const cairo_region_t* mume_window_get_urgn(
    const void *self);

/* Begin paint to a window.
 *
 * When paint finishes, user should call mume_end_paint.
 */
mume_public cairo_t* mume_window_begin_paint(void *self, int mode);

/* Finish paint to a window.
 *
 * After this operation, <cr> should not be used any more.
 */
mume_public void mume_window_end_paint(void *self, cairo_t *cr);

/* Set/Get user defined data to the specified window. */
mume_public void mume_window_set_user_data(
    void *self, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy);

mume_public void* mume_window_get_user_data(
    const void *self, const mume_user_data_key_t *key);

/* Set/Get mouse cursor of the specified window. The <cursor>
 * will be used when the pointer is in the window. If <cursor>
 * is NULL, the parent's cursor will be used.
 */
mume_public void mume_window_set_cursor(void *self, void *cursor);

mume_public void* mume_window_get_cursor(const void *self);

/* Seek the mouse cursor that will be used by the window. */
mume_public void* mume_window_seek_cursor(const void *self);

/* Selectors for handle events. */
mume_public void _mume_window_handle_event(
    const void *clazz, void *self, mume_event_t *event);

mume_public void _mume_window_handle_key_down(
    const void *clazz, void *self,
    int x, int y, int state, int keysym);

mume_public void _mume_window_handle_key_up(
    const void *clazz, void *self,
    int x, int y, int state, int keysym);

mume_public void _mume_window_handle_button_down(
    const void *clazz, void *self,
    int x, int y, int state, int button);

mume_public void _mume_window_handle_button_up(
    const void *clazz, void *self,
    int x, int y, int state, int button);

mume_public void _mume_window_handle_button_dblclk(
    const void *clazz, void *self,
    int x, int y, int state, int button);

mume_public void _mume_window_handle_button_tplclk(
    const void *clazz, void *self,
    int x, int y, int state, int button);

mume_public void _mume_window_handle_mouse_motion(
    const void *clazz, void *self, int x, int y, int state);

mume_public void _mume_window_handle_mouse_enter(
    const void *clazz, void *self,
    int x, int y, int state, int mode, int detail);

mume_public void _mume_window_handle_mouse_leave(
    const void *clazz, void *self,
    int x, int y, int state, int mode, int detail);

mume_public void _mume_window_handle_focus_in(
    const void *clazz, void *self, int mode, int detail);

mume_public void _mume_window_handle_focus_out(
    const void *clazz, void *self, int mode, int detail);

mume_public void _mume_window_handle_expose(
    const void *clazz, void *self,
    int x, int y, int width, int height, int count);

mume_public void _mume_window_handle_create(
    const void *clazz, void *self, void *window,
    int x, int y, int width, int height);

mume_public void _mume_window_handle_destroy(
    const void *clazz, void *self, void *window);

mume_public void _mume_window_handle_map(
    const void *clazz, void *self, void *window);

mume_public void _mume_window_handle_unmap(
    const void *clazz, void *self, void *window);

mume_public void _mume_window_handle_reparent(
    const void *clazz, void *self, void *window,
    void *parent, int x, int y);

mume_public void _mume_window_handle_move(
    const void *clazz, void *self, void *window,
    int x, int y, int old_x, int old_y);

mume_public void _mume_window_handle_resize(
    const void *clazz, void *self, void *window,
    int w, int h, int old_w, int old_h);

mume_public void _mume_window_handle_sizehint(
    const void *clazz, void *self, int *pref_w, int *pref_h,
    int *min_w, int *min_h, int *max_w, int *max_h);

mume_public void _mume_window_handle_command(
    const void *clazz, void *self, void *window, int command);

mume_public void _mume_window_handle_notify(
    const void *clazz, void *self,
    void *window, int code, void *data);

mume_public void _mume_window_handle_scroll(
    const void *clazz, void *self,
    void *window, int hitcode, int position);

mume_public void _mume_window_handle_close(
    const void *clazz, void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_WINDOW_H */
