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
#include "mume-sdl-cursor.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _sdl_cursor_super_class mume_cursor_class

struct _sdl_cursor {
    const char _[MUME_SIZEOF_CURSOR];
    SDL_Cursor *cursor;
};

struct _sdl_cursor_class {
    const char _[MUME_SIZEOF_CURSOR_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _sdl_cursor) ==
                   MUME_SIZEOF_SDL_CURSOR);

MUME_STATIC_ASSERT(sizeof(struct _sdl_cursor_class) ==
                   MUME_SIZEOF_SDL_CURSOR_CLASS);

static void* _sdl_cursor_ctor(
    struct _sdl_cursor *self, int mode, va_list *app)
{
    if (!_mume_ctor(_sdl_cursor_super_class(), self, mode, app))
        return NULL;

    self->cursor = va_arg(*app, SDL_Cursor*);
    return self;
}

static void* _sdl_cursor_dtor(struct _sdl_cursor *self)
{
    SDL_FreeCursor(self->cursor);
    return _mume_dtor(_sdl_cursor_super_class(), self);
}

const void* mume_sdl_cursor_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_sdl_cursor_meta_class(),
        "sdl cursor",
        _sdl_cursor_super_class(),
        sizeof(struct _sdl_cursor),
        MUME_PROP_END,
        _mume_ctor, _sdl_cursor_ctor,
        _mume_dtor, _sdl_cursor_dtor,
        MUME_FUNC_END);
}

SDL_Cursor* mume_sdl_create_cursor(int id)
{
    /* XPM */
    static const char *arrow[] = {
        /* width height num_colors chars_per_pixel */
        "    32    32        3            1",
        /* colors */
        "X c #000000",
        ". c #ffffff",
        "  c None",
        /* pixels */
        "X                               ",
        "XX                              ",
        "X.X                             ",
        "X..X                            ",
        "X...X                           ",
        "X....X                          ",
        "X.....X                         ",
        "X......X                        ",
        "X.......X                       ",
        "X........X                      ",
        "X.....XXXXX                     ",
        "X..X..X                         ",
        "X.X X..X                        ",
        "XX  X..X                        ",
        "X    X..X                       ",
        "     X..X                       ",
        "      X..X                      ",
        "      X..X                      ",
        "       XX                       ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "0,0"
    };

    int i, row, col;
    Uint8 data[4*32];
    Uint8 mask[4*32];
    int hot_x, hot_y;
    const char **image = NULL;

    switch (id) {
    case MUME_CURSOR_ARROW:
        image = arrow;
        break;

    case MUME_CURSOR_HAND:
        break;

    case MUME_CURSOR_IBEAM:
        break;

    case MUME_CURSOR_WAIT:
        break;
    }

    if (NULL == image)
        return NULL;

    i = -1;
    for (row = 0; row < 32; ++row) {
        for (col = 0; col < 32; ++col) {
            if (col % 8) {
                data[i] <<= 1;
                mask[i] <<= 1;
            } else {
                ++i;
                data[i] = mask[i] = 0;
            }
            switch (image[4+row][col]) {
            case 'X':
                data[i] |= 0x01;
                mask[i] |= 0x01;
                break;

            case '.':
                mask[i] |= 0x01;
                break;

            case ' ':
                break;
            }
        }
    }

    sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);

    return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void* mume_sdl_cursor_new(SDL_Cursor *cursor)
{
    return mume_new(mume_sdl_cursor_class(), cursor);
}

SDL_Cursor* mume_sdl_cursor_get_entity(const void *_self)
{
    const struct _sdl_cursor *self = _self;
    assert(mume_is_of(_self, mume_sdl_cursor_class()));
    return self->cursor;
}
