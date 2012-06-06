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
#ifndef MUME_BASE_MATH_H
#define MUME_BASE_MATH_H

#include "mume-common.h"
#include MUME_MATH_H

MUME_BEGIN_DECLS

/* Calculate a * b / c, abort if the result overflows. */
mume_public int mume_int_mul_div(int a, int b, int c);

MUME_END_DECLS

#endif  /* MUME_BASE_MATH_H */
