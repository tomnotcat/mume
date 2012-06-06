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
#include "mume-sdl-backend.h"
#include "mume-debug.h"
#include "mume-sdl-backwin.h"
#include "mume-sdl-cursor.h"
#include "mume-sdl-dbgutil.h"
#include MUME_ASSERT_H

#define _sdl_backend_super_class mume_backend_class

struct _sdl_backend {
    const char _[MUME_SIZEOF_BACKEND];
    void *root_window;
};

struct _sdl_backend_class {
    const char _[MUME_SIZEOF_BACKEND_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _sdl_backend) ==
                   MUME_SIZEOF_SDL_BACKEND);

MUME_STATIC_ASSERT(sizeof(struct _sdl_backend_class) ==
                   MUME_SIZEOF_SDL_BACKEND_CLASS);

static Uint32 _sdl_timer_callback(Uint32 interval, void *param)
{
    SDL_Event event;

#define SDL_USEREVT_TIMER 1
    event.user.type = SDL_USEREVENT;
    event.user.code = SDL_USEREVT_TIMER;
    SDL_PushEvent(&event);
#undef SDL_USEREVT_TIMER

    return 0;
}

static void* _sdl_backend_ctor(
    struct _sdl_backend *self, int mode, va_list *app)
{
    int width, height;
    unsigned int flags;
    Uint32 sdl_flags = SDL_HWSURFACE;
    SDL_Surface *screen;

    if (!_mume_ctor(_sdl_backend_super_class(), self, mode, app))
        return NULL;

    width = va_arg(*app, int);
    height = va_arg(*app, int);
    flags = va_arg(*app, unsigned int);

    if (mume_test_flag(flags, MUME_BACKEND_FLAG_CENTERED))
        SDL_putenv("SDL_VIDEO_CENTERED=1");

    if (mume_test_flag(flags, MUME_BACKEND_FLAG_RESIZABLE))
        sdl_flags |= SDL_RESIZABLE;

    if (mume_test_flag(flags, MUME_BACKEND_FLAG_FULLSCREEN))
        sdl_flags |= SDL_FULLSCREEN;

    if (-1 == SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
        mume_abort(("SDL_Init error: %s\n", SDL_GetError()));

    screen = SDL_SetVideoMode(width, height, 32, sdl_flags);
    if (NULL == screen)
        mume_abort(("SDL_SetVideoMode error: %s\n", SDL_GetError()));

    self->root_window =  mume_sdl_backwin_new(self, screen);

    SDL_WM_SetCaption("mume", "ICON");
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                        SDL_DEFAULT_REPEAT_INTERVAL);

    return self;
}

static void* _sdl_backend_dtor(struct _sdl_backend *self)
{
    mume_refobj_release(self->root_window);
    SDL_Quit();
    return _mume_dtor(_sdl_backend_super_class(), self);
}

static void _sdl_backend_screen_size(
    struct _sdl_backend *self, int *width, int *height)
{
    SDL_Surface *screen = SDL_GetVideoSurface();

    if (width)
        *width = screen->w;

    if (height)
        *height = screen->h;
}

static void* _sdl_backend_root_backwin(struct _sdl_backend *self)
{
    return self->root_window;
}

static void* _sdl_backend_create_cursor(
    struct _sdl_backend *self, int id)
{
    SDL_Cursor *cursor = mume_sdl_create_cursor(id);
    if (cursor)
        return mume_sdl_cursor_new(cursor);

    return NULL;
}

static int _sdl_backend_handle_event(
    struct _sdl_backend *self, int wait)
{
    int result;
    SDL_Event event;

    if (MUME_WAIT_INFINITE == wait) {
        result = SDL_WaitEvent(&event);
    }
    else {
        result = SDL_PollEvent(&event);
        if (wait > 0 && !SDL_AddTimer(
                wait, _sdl_timer_callback, NULL))
        {
            mume_warning(("Add timer failed: %d\n", wait));
        }
    }

    if (result) {
        mume_sdl_print_event(&event);
        mume_sdl_backwin_handle_event(self->root_window, &event);
    }

    return result;
}

static int _sdl_backend_wakeup_event(struct _sdl_backend *self)
{
    SDL_Event event;

#define SDL_USEREVT_WAKEUP 2
    event.user.type = SDL_USEREVENT;
    event.user.code = SDL_USEREVT_WAKEUP;
#undef SDL_USEREVT_WAKEUP

    return (0 == SDL_PushEvent(&event));
}

static void _sdl_backend_query_pointer(
    struct _sdl_backend *self, int *x, int *y, int *state)
{
    int mouse = SDL_GetMouseState(x, y);

    if (state)
        *state = mume_sdl_translate_state(SDL_GetModState(), mouse);
}

const void* mume_sdl_backend_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_sdl_backend_meta_class(),
        "sdl backend",
        _sdl_backend_super_class(),
        sizeof(struct _sdl_backend),
        MUME_PROP_END,
        _mume_ctor, _sdl_backend_ctor,
        _mume_dtor, _sdl_backend_dtor,
        _mume_backend_screen_size,
        _sdl_backend_screen_size,
        _mume_backend_root_backwin,
        _sdl_backend_root_backwin,
        _mume_backend_create_cursor,
        _sdl_backend_create_cursor,
        _mume_backend_handle_event,
        _sdl_backend_handle_event,
        _mume_backend_wakeup_event,
        _sdl_backend_wakeup_event,
        _mume_backend_query_pointer,
        _sdl_backend_query_pointer,
        MUME_FUNC_END);
}

const void* mume_backend_class_sym(void)
{
    return mume_sdl_backend_class();
}
