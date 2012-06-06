/* Mume Reader - a full featured reading environment.
 *
 * Copyright © 2012 Soft Flag, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MUME_FOUNDATION_ERROR_H
#define MUME_FOUNDATION_ERROR_H

#include "mume-common.h"

MUME_BEGIN_DECLS

mume_public void mume_set_errno(int error);

mume_public int mume_get_errno(void);

mume_public void mume_set_errstr(const char *str);

mume_public const char* mume_get_errstr(void);

mume_public void mume_error_clear(void);

MUME_END_DECLS

#endif /* !MUME_FOUNDATION_ERROR_H */
