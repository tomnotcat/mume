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
#ifndef MUME_FOUNDATION_DIBFCN_H
#define MUME_FOUNDATION_DIBFCN_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_DIB_FILEHEADER_SIZE 14

typedef struct mume_dib_s {
    int width;
    int height;
    void *data;
} mume_dib_t;

mume_public mume_dib_t* mume_dib_create(mume_stream_t *stm);

mume_public void mume_dib_destroy(mume_dib_t *dib);

mume_public cairo_surface_t* mume_dib_create_surface(
    mume_dib_t *dib, mume_color_t *kclr);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_DIBFCN_H */
