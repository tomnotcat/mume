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
#ifndef MUME_FOUNDATION_DBGUTIL_H
#define MUME_FOUNDATION_DBGUTIL_H

#include "mume-common.h"

MUME_BEGIN_DECLS

/* Dump the event to the log output. */
mume_public void mume_dump_event(const mume_event_t *event);

/* Dump the rectangles contained in the region to the log output. */
mume_public void mume_dump_region(const cairo_region_t *rgn);

/* Get the name of the specified key. */
mume_public const char* mume_key_name(int key);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_DBGUTIL_H */
