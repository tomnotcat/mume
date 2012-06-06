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
#include "mume-math.h"
#include "mume-debug.h"

int mume_int_mul_div(int a, int b, int c)
{
    int m = a * b;
    if ((a > 0 && b > 0) || (a < 0 && b < 0)) {
        if (m < 0) {
            m = (double)a * b / c;

            if ((c > 0 && m < 0) || (c < 0 && m > 0))
                mume_abort(("mume_int_mul_div(%d, %d, %d)\n", a, b, c));

            return m;
        }
    }
    else if (m > 0) {
        m = (double)a * b / c;

        if ((c > 0 && m > 0) || (c < 0 && m < 0))
            mume_abort(("mume_int_mul_div(%d, %d, %d)\n", a, b, c));

        return m;
    }

    return m / c;
}
