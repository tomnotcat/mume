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
#include "mume-events.h"

mume_event_t mume_make_empty_event(void)
{
    mume_event_t result;
    result.any.type = 0;
    result.any.window = NULL;
    return result;
}

mume_event_t mume_make_create_event(
    void *parent, void *window, int x, int y, int w, int h)
{
    mume_event_t result;
    result.create.type = MUME_EVENT_CREATE;
    result.create.parent = parent;
    result.create.window = window;
    result.create.x = x;
    result.create.y = y;
    result.create.width = w;
    result.create.height = h;
    return result;
}

mume_event_t mume_make_destroy_event(void *event, void *window)
{
    mume_event_t result;
    result.destroy.type = MUME_EVENT_DESTROY;
    result.destroy.event = event;
    result.destroy.window = window;
    return result;
}

mume_event_t mume_make_map_event(void *event, void *window)
{
    mume_event_t result;
    result.map.type = MUME_EVENT_MAP;
    result.map.event = event;
    result.map.window = window;
    return result;
}

mume_event_t mume_make_unmap_event(void *event, void *window)
{
    mume_event_t result;
    result.unmap.type = MUME_EVENT_UNMAP;
    result.unmap.event = event;
    result.unmap.window = window;
    return result;
}

mume_event_t mume_make_reparent_event(
    void *event, void *window, void *parent, int x, int y)
{
    mume_event_t result;
    result.reparent.type = MUME_EVENT_REPARENT;
    result.reparent.event = event;
    result.reparent.window = window;
    result.reparent.parent = parent;
    result.reparent.x = x;
    result.reparent.y = y;
    return result;
}

mume_event_t mume_make_move_event(
    void *event, void *window, int x, int y, int ox, int oy)
{
    mume_event_t result;
    result.move.type = MUME_EVENT_MOVE;
    result.move.event = event;
    result.move.window = window;
    result.move.x = x;
    result.move.y = y;
    result.move.old_x = ox;
    result.move.old_y = oy;
    return result;
}

mume_event_t mume_make_resize_event(
    void *event, void *window, int width, int height,
    int old_width, int old_height)
{
    mume_event_t result;
    result.resize.type = MUME_EVENT_RESIZE;
    result.resize.event = event;
    result.resize.window = window;
    result.resize.width = width;
    result.resize.height = height;
    result.resize.old_width = old_width;
    result.resize.old_height = old_height;
    return result;
}

mume_event_t mume_make_sizehint_event(
    void *event, int pref_width, int pref_height,
    int min_width, int min_height, int max_width, int max_height)
{
    mume_event_t result;
    result.sizehint.type = MUME_EVENT_SIZEHINT;
    result.sizehint.event = event;
    result.sizehint.pref_width = pref_width;
    result.sizehint.pref_height = pref_height;
    result.sizehint.min_width = min_width;
    result.sizehint.min_height = min_height;
    result.sizehint.max_width = max_width;
    result.sizehint.max_height = max_height;
    return result;
}

mume_event_t mume_make_command_event(
    void *event, void *window, int command)
{
    mume_event_t result;
    result.command.type = MUME_EVENT_COMMAND;
    result.command.event = event;
    result.command.window = window;
    result.command.command = command;
    return result;
}

mume_event_t mume_make_notify_event(
    void *event, void *window, int code, void *data)
{
    mume_event_t result;
    result.notify.type = MUME_EVENT_NOTIFY;
    result.notify.event = event;
    result.notify.window = window;
    result.notify.code = code;
    result.notify.data = data;
    return result;
}

mume_event_t mume_make_scroll_event(
    void *event, void *window, int hitcode, int position)
{
    mume_event_t result;
    result.scroll.type = MUME_EVENT_SCROLL;
    result.scroll.event = event;
    result.scroll.window = window;
    result.scroll.hitcode = hitcode;
    result.scroll.position = position;
    return result;
}

mume_event_t mume_make_close_event(void *event)
{
    mume_event_t result;
    result.close.type = MUME_EVENT_CLOSE;
    result.close.window = event;
    return result;
}

mume_event_t mume_make_quit_event(void)
{
    mume_event_t result;
    result.any.type = MUME_EVENT_QUIT;
    result.any.window = NULL;
    return result;
}
