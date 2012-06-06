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
#include "mume-dibfcn.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-stream.h"
#include "mume-gstate.h"
#include MUME_ASSERT_H
#include MUME_STDINT_H

#define DIB_FILEHEADER_SIZE 14

enum _DIB_COMPRESSION_METHOD {
    BI_RGB = 0,
    BI_RLE8,
    BI_RLE4,
    BI_BITFIELDS,
    BI_JPEG,
    BI_PNG
};

typedef struct _BITMAP_HEADER {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t pels_per_meter_x;
    int32_t pels_per_meter_y;
    uint32_t clr_used;
    uint32_t clr_important;
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t cs_type;
    uint32_t end_points[9];
    uint32_t gamma_red;
    uint32_t gamma_green;
    uint32_t gamma_blue;
} BITMAP_HEADER;

typedef struct _RGBA {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} RGBA;

typedef struct _BGRA {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} BGRA;

typedef struct _ARGB {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ARGB;

mume_dib_t* mume_dib_create(mume_stream_t *stm)
{
    mume_dib_t *dib;
    char magic[2];
    uint32_t filesz;
    uint32_t bits_offset;
    BITMAP_HEADER dbh;
    int i, j, k, stride;
    uint8_t *line, *it;
    BGRA color_table[256], *clr;
    RGBA *data;
    /* Bitmap file header */
    mume_stream_read(stm, magic, sizeof(magic));
    if (magic[0] != 'B' || magic[1] != 'M') {
        mume_error(("Invalid dib magic: %c%c\n", magic[0], magic[1]));
        return NULL;
    }
    mume_stream_read_le_uint32(stm, &filesz);
    /* Skip reserved */
    mume_stream_seek(stm, mume_stream_tell(stm) + 4);
    mume_stream_read_le_uint32(stm, &bits_offset);
    /* DIB header */
    mume_stream_read_le_uint32(stm, &dbh.header_size);
    mume_stream_read_le_int32(stm, &dbh.width);
    mume_stream_read_le_int32(stm, &dbh.height);
    mume_stream_read_le_uint16(stm, &dbh.planes);
    mume_stream_read_le_uint16(stm, &dbh.bit_count);
    mume_stream_read_le_uint32(stm, &dbh.compression);
    mume_stream_read_le_uint32(stm, &dbh.size_image);
    mume_stream_read_le_int32(stm, &dbh.pels_per_meter_x);
    mume_stream_read_le_int32(stm, &dbh.pels_per_meter_y);
    mume_stream_read_le_uint32(stm, &dbh.clr_used);
    mume_stream_read_le_uint32(stm, &dbh.clr_important);
    mume_stream_read_le_uint32(stm, &dbh.red_mask);
    mume_stream_read_le_uint32(stm, &dbh.green_mask);
    mume_stream_read_le_uint32(stm, &dbh.blue_mask);
    mume_stream_read_le_uint32(stm, &dbh.alpha_mask);
    mume_stream_read_le_uint32(stm, &dbh.cs_type);
    for (i = 0; i < 9; ++i)
        mume_stream_read_le_uint32(stm, dbh.end_points + i);
    mume_stream_read_le_uint32(stm, &dbh.gamma_red);
    mume_stream_read_le_uint32(stm, &dbh.gamma_green);
    mume_stream_read_le_uint32(stm, &dbh.gamma_blue);
    /* Load color table */
    switch (dbh.bit_count) {
    case 1:
        i = 2;
        break;
    case 4:
        i = 16;
        break;
    case 8:
        i = 256;
        break;
    default:
        i = 0;
    }
    mume_stream_seek(stm, DIB_FILEHEADER_SIZE + dbh.header_size);
    mume_stream_read(stm, color_table, sizeof(BGRA) * i);
    /* Read bits */
    if (dbh.compression != BI_RGB) {
        mume_error(("Compression method not supported: %d\n",
                    dbh.compression));
        return NULL;
    }
    dib = malloc_struct(mume_dib_t);
    dib->width = dbh.width < 0 ? -dbh.width : dbh.width;
    dib->height = dbh.height < 0 ? -dbh.height : dbh.height;
    dib->data = malloc_abort(sizeof(RGBA) * dib->width * dib->height);
    stride = ((dib->width * dbh.bit_count / 8) + 3) & ~3;
    line = malloc_abort(stride);
    mume_stream_seek(stm, bits_offset);
    data = dib->data;
    data += dib->width * dib->height + dib->width;
    for (i = 0; i < dib->height; ++i) {
        mume_stream_read(stm, line, stride);
        it = line;
        data -= dib->width + dib->width;
        for (j = 0; j < dib->width; ++j) {
            if (dbh.bit_count == 1) {
                k = (8 <= dib->width - j) ? 8 : dib->width - j;
                while (k-- > 0) {
                    clr = *it & 0x80 ? &color_table[1] : &color_table[0];
                    data->red = clr->red;
                    data->green = clr->green;
                    data->blue = clr->blue;
                    data->alpha = clr->alpha;
                    ++data;
                    *it <<= 1;
                }
                ++it;
                j += 7;
            }
            else if (dbh.bit_count == 4) {
                clr = &color_table[(*it >> 4) & 0x0f];
                data->red = clr->red;
                data->green = clr->green;
                data->blue = clr->blue;
                data->alpha = clr->alpha;
                ++data;
                clr = &color_table[*it & 0x0f];
                data->red = clr->red;
                data->green = clr->green;
                data->blue = clr->blue;
                data->alpha = clr->alpha;
                ++it;
                ++j;
                ++data;
            }
            else if (dbh.bit_count == 8) {
                clr = &color_table[*it];
                data->red = clr->red;
                data->green = clr->green;
                data->blue = clr->blue;
                data->alpha = clr->alpha;
                ++it;
                ++data;
            }
            else if (dbh.bit_count == 16) {
                uint16_t val = *((uint16_t*)it);
                data->red = ((val >> 10) & 0x1f) << 3;
                data->green = ((val >> 5) & 0x1f) << 3;
                data->blue = (val & 0x1f) << 3;
                data->alpha = 255;
                it += 2;
                ++data;
            }
            else if (dbh.bit_count == 24) {
                data->blue = it[0];
                data->green = it[1];
                data->red = it[2];
                data->alpha = 255;
                it += 3;
                ++data;
            }
            else if (dbh.bit_count == 32) {
                data->blue = it[0];
                data->green = it[1];
                data->red = it[2];
                data->alpha = it[3];
                it += 4;
                ++data;
            }
        }
    }
    free(line);
    return dib;
}

void mume_dib_destroy(mume_dib_t *dib)
{
    free(dib->data);
    free(dib);
}

cairo_surface_t* mume_dib_create_surface(
    mume_dib_t *dib, mume_color_t *kclr)
{
    cairo_surface_t *sfc = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, dib->width, dib->height);
    RGBA *src = dib->data;
    BGRA *dst = (BGRA*)cairo_image_surface_get_data(sfc);
    int i, j;
    for (i = 0; i < dib->height; ++i) {
        for (j = 0; j < dib->width; ++j, ++src, ++dst) {
            dst->red = src->red;
            dst->green = src->green;
            dst->blue = src->blue;
            if (kclr && kclr->r == dst->red &&
                kclr->g == dst->green && kclr->b == dst->blue)
            {
                dst->alpha = 0;
            }
            else {
                dst->alpha = 255;
            }
        }
    }
    cairo_surface_mark_dirty(sfc);
    return sfc;
}
