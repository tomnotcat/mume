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
#include "mume-sdl-backwin.h"
#include "mume-debug.h"
#include "mume-events.h"
#include "mume-frontend.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-sdl-backend.h"
#include "mume-sdl-cursor.h"
#include MUME_ASSERT_H

#define _sdl_backwin_super_class mume_backwin_class
#define _sdl_backwin_super_meta_class mume_backwin_meta_class

struct _sdl_backwin {
    const char _[MUME_SIZEOF_BACKWIN];
    void *backend;
    SDL_Surface *surface;
};

struct _sdl_backwin_class {
    const char _[MUME_SIZEOF_BACKWIN_CLASS];
    void (*handle_event)(void *self, SDL_Event *event);
    void (*handle_key_down)(void *self, SDL_KeyboardEvent *key);
    void (*handle_key_up)(void *self, SDL_KeyboardEvent *key);
    void (*handle_mouse_motion)(
        void *self, SDL_MouseMotionEvent *motion);
    void (*handle_button_down)(
        void *self, SDL_MouseButtonEvent *button);
    void (*handle_button_up)(
        void *self, SDL_MouseButtonEvent *button);
    void (*handle_user_event)(void *self, SDL_UserEvent *user);
    void (*handle_active_event)(void *self, SDL_ActiveEvent *active);
    void (*handle_video_resize)(void *self, SDL_ResizeEvent *resize);
    void (*handle_video_expose)(void *self, SDL_ExposeEvent *expose);
    void (*handle_quit)(void *self, SDL_QuitEvent *quit);
};

MUME_STATIC_ASSERT(sizeof(struct _sdl_backwin) ==
                   MUME_SIZEOF_SDL_BACKWIN);

MUME_STATIC_ASSERT(sizeof(struct _sdl_backwin_class) ==
                   MUME_SIZEOF_SDL_BACKWIN_CLASS);

static void* _sdl_backwin_ctor(
    struct _sdl_backwin *self, int mode, va_list *app)
{
    if (!_mume_ctor(_sdl_backwin_super_class(), self, mode, app))
        return NULL;

    self->backend = va_arg(*app, void*);
    self->surface = va_arg(*app, SDL_Surface*);

    return self;
}

static void* _sdl_backwin_dtor(struct _sdl_backwin *self)
{
    SDL_FreeSurface(self->surface);
    return _mume_dtor(_sdl_backwin_super_class(), self);
}

static int _sdl_backwin_is_mapped(struct _sdl_backwin *self)
{
    return 1;
}

static void _sdl_backwin_get_geometry(
    struct _sdl_backwin *self, int *x, int *y, int *w, int *h)
{
    if (x)
        *x = 0;

    if (y)
        *y = 0;

    if (w)
        *w = self->surface->w;

    if (h)
        *h = self->surface->h;
}

static void _sdl_backwin_set_cursor(
    struct _sdl_backwin *self, void *cursor)
{
    static SDL_Cursor *sdl_defcursor;
    SDL_Cursor *sdl_cursor;

    if (NULL == sdl_defcursor)
        sdl_defcursor = SDL_GetCursor();

    if (cursor)
        sdl_cursor = mume_sdl_cursor_get_entity(cursor);
    else
        sdl_cursor = sdl_defcursor;

    SDL_SetCursor(sdl_cursor);
}

static cairo_t* _sdl_backwin_begin_paint(struct _sdl_backwin *self)
{
    cairo_t *cr;
    SDL_LockSurface(self->surface);
    cr = cairosdl_create(self->surface);
    /*cairo_scale(cr, sback->screen->w, sback->screen->h);*/
    return cr;
}

static void _sdl_backwin_end_paint(
    struct _sdl_backwin *self, cairo_t *cr)
{
    cairo_status_t status;

    status = cairo_status(cr);
    if (status != CAIRO_STATUS_SUCCESS) {
        mume_abort(("cairo error: %s\n",
                    cairo_status_to_string(status)));
    }

    cairosdl_destroy(cr);
    SDL_UnlockSurface(self->surface);
    SDL_Flip(self->surface);
}

static void _sdl_backwin_handle_event(
    struct _sdl_backwin *self, SDL_Event *event)
{
    switch (event->type) {
    case SDL_KEYDOWN:
        mume_sdl_backwin_handle_key_down(self, &event->key);
        break;

    case SDL_KEYUP:
        mume_sdl_backwin_handle_key_up(self, &event->key);
        break;

    case SDL_MOUSEMOTION:
        mume_sdl_backwin_handle_mouse_motion(self, &event->motion);
        break;

    case SDL_MOUSEBUTTONDOWN:
        mume_sdl_backwin_handle_button_down(self, &event->button);
        break;

    case SDL_MOUSEBUTTONUP:
        mume_sdl_backwin_handle_button_up(self, &event->button);
        break;

    case SDL_USEREVENT:
        mume_sdl_backwin_handle_user_event(self, &event->user);
        break;

    case SDL_ACTIVEEVENT:
        mume_sdl_backwin_handle_active_event(self, &event->active);
        break;

    case SDL_VIDEORESIZE:
        mume_sdl_backwin_handle_video_resize(self, &event->resize);
        break;

    case SDL_VIDEOEXPOSE:
        mume_sdl_backwin_handle_video_expose(self, &event->expose);
        break;

    case SDL_QUIT:
        mume_sdl_backwin_handle_quit(self, &event->quit);
        break;
    }
}

static void _sdl_backwin_handle_key_down(
    struct _sdl_backwin *self, SDL_KeyboardEvent *key)
{
    int mouse, x, y, state, keysym;

    mouse = SDL_GetMouseState(&x, &y);
    state = mume_sdl_translate_state(key->keysym.mod, mouse);
    keysym = mume_sdl_translate_keysym(key->keysym.sym);

    mume_frontend_handle_keydown(
        mume_frontend(), self, x, y, state, keysym);
}

static void _sdl_backwin_handle_key_up(
    struct _sdl_backwin *self, SDL_KeyboardEvent *key)
{
    int mouse, x, y, state, keysym;

    mouse = SDL_GetMouseState(&x, &y);
    state = mume_sdl_translate_state(key->keysym.mod, mouse);
    keysym = mume_sdl_translate_keysym(key->keysym.sym);

    mume_frontend_handle_keyup(
        mume_frontend(), self, x, y, state, keysym);
}

static void _sdl_backwin_handle_mouse_motion(
    struct _sdl_backwin *self, SDL_MouseMotionEvent *motion)
{
    int state = mume_sdl_translate_state(
        SDL_GetModState(), motion->state);

    mume_frontend_handle_mousemotion(
        mume_frontend(), self, motion->x, motion->y, state);
}

static void _sdl_backwin_handle_button_down(
    struct _sdl_backwin *self, SDL_MouseButtonEvent *button)
{
    int state, button1;

    state = mume_sdl_translate_state(
        SDL_GetModState(), SDL_GetMouseState(NULL, NULL));

    button1 = mume_sdl_translate_button(button->button);

    mume_frontend_handle_buttondown(
        mume_frontend(), self,
        button->x, button->y, state, button1);
}

static void _sdl_backwin_handle_button_up(
    struct _sdl_backwin *self, SDL_MouseButtonEvent *button)
{
    int state, button1;

    state = mume_sdl_translate_state(
        SDL_GetModState(), SDL_GetMouseState(NULL, NULL));

    button1 = mume_sdl_translate_button(button->button);

    mume_frontend_handle_buttonup(
        mume_frontend(), self,
        button->x, button->y, state, button1);
}

static void _sdl_backwin_handle_user_event(
    struct _sdl_backwin *self, SDL_UserEvent *user)
{
}

static void _sdl_backwin_handle_active_event(
    struct _sdl_backwin *self, SDL_ActiveEvent *active)
{
    if (SDL_APPMOUSEFOCUS & active->state) {
        int x, y, state;

        state = mume_sdl_translate_state(
            SDL_GetModState(), SDL_GetMouseState(&x, &y));

        if (active->gain) {
            mume_frontend_handle_mouseenter(
                mume_frontend(), self,
                x, y, state, MUME_NOTIFY_NORMAL);
        }
        else {
            mume_frontend_handle_mouseleave(
                mume_frontend(), self,
                x, y, state, MUME_NOTIFY_NORMAL);
        }
    }

    if (SDL_APPINPUTFOCUS & active->state) {
        if (active->gain) {
            mume_frontend_handle_focusin(
                mume_frontend(), self, MUME_NOTIFY_NORMAL);
        }
        else {
            mume_frontend_handle_focusout(
                mume_frontend(), self, MUME_NOTIFY_NORMAL);
        }
    }
}

static void _sdl_backwin_handle_video_resize(
    struct _sdl_backwin *self, SDL_ResizeEvent *resize)
{
    self->surface = SDL_SetVideoMode(
        resize->w, resize->h, 32, SDL_HWSURFACE | SDL_RESIZABLE);

    mume_frontend_handle_geometry(
        mume_frontend(), self, 0, 0, resize->w, resize->h);
}

static void _sdl_backwin_handle_video_expose(
    struct _sdl_backwin *self, SDL_ExposeEvent *expose)
{
    SDL_Flip(self->surface);
}

static void _sdl_backwin_handle_quit(
    struct _sdl_backwin *self, SDL_QuitEvent *quit)
{
    mume_frontend_handle_close(mume_frontend(), self);
    mume_frontend_handle_quit(mume_frontend());
}

static void* _sdl_backwin_class_ctor(
    struct _sdl_backwin_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_sdl_backwin_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_sdl_backwin_handle_event)
            *(voidf**)&self->handle_event = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_key_down)
            *(voidf**)&self->handle_key_down = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_key_up)
            *(voidf**)&self->handle_key_up = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_mouse_motion)
            *(voidf**)&self->handle_mouse_motion = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_button_down)
            *(voidf**)&self->handle_button_down = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_button_up)
            *(voidf**)&self->handle_button_up = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_user_event)
            *(voidf**)&self->handle_user_event = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_active_event)
            *(voidf**)&self->handle_active_event = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_video_resize)
            *(voidf**)&self->handle_video_resize = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_video_expose)
            *(voidf**)&self->handle_video_expose = method;
        else if (selector == (voidf*)_mume_sdl_backwin_handle_quit)
            *(voidf**)&self->handle_quit = method;
    }

    return self;
}

const void* mume_sdl_backwin_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_sdl_backwin_meta_class(),
        "sdl backwin",
        _sdl_backwin_super_class(),
        sizeof(struct _sdl_backwin),
        MUME_PROP_END,
        _mume_ctor, _sdl_backwin_ctor,
        _mume_dtor, _sdl_backwin_dtor,
        _mume_backwin_is_mapped, _sdl_backwin_is_mapped,
        _mume_backwin_get_geometry, _sdl_backwin_get_geometry,
        _mume_backwin_set_cursor, _sdl_backwin_set_cursor,
        _mume_backwin_begin_paint, _sdl_backwin_begin_paint,
        _mume_backwin_end_paint, _sdl_backwin_end_paint,
        _mume_sdl_backwin_handle_event,
        _sdl_backwin_handle_event,
        _mume_sdl_backwin_handle_key_down,
        _sdl_backwin_handle_key_down,
        _mume_sdl_backwin_handle_key_up,
        _sdl_backwin_handle_key_up,
        _mume_sdl_backwin_handle_mouse_motion,
        _sdl_backwin_handle_mouse_motion,
        _mume_sdl_backwin_handle_button_down,
        _sdl_backwin_handle_button_down,
        _mume_sdl_backwin_handle_button_up,
        _sdl_backwin_handle_button_up,
        _mume_sdl_backwin_handle_user_event,
        _sdl_backwin_handle_user_event,
        _mume_sdl_backwin_handle_active_event,
        _sdl_backwin_handle_active_event,
        _mume_sdl_backwin_handle_video_resize,
        _sdl_backwin_handle_video_resize,
        _mume_sdl_backwin_handle_video_expose,
        _sdl_backwin_handle_video_expose,
        _mume_sdl_backwin_handle_quit,
        _sdl_backwin_handle_quit,
        MUME_FUNC_END);
}

const void* mume_sdl_backwin_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "sdl backwin class",
        _sdl_backwin_super_meta_class(),
        sizeof(struct _sdl_backwin_class),
        MUME_PROP_END,
        _mume_ctor, _sdl_backwin_class_ctor,
        MUME_FUNC_END);
}

void* mume_sdl_backwin_new(void *backend, SDL_Surface *surface)
{
    return mume_new(mume_sdl_backwin_class(),
                    backend, surface);
}

void _mume_sdl_backwin_handle_event(
    const void *_clazz, void *_self, SDL_Event *event)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_event, (_self, event));
}

void _mume_sdl_backwin_handle_key_down(
    const void *_clazz, void *_self, SDL_KeyboardEvent *key)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_key_down, (_self, key));
}

void _mume_sdl_backwin_handle_key_up(
    const void *_clazz, void *_self, SDL_KeyboardEvent *key)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_key_up, (_self, key));
}

void _mume_sdl_backwin_handle_mouse_motion(
    const void *_clazz, void *_self, SDL_MouseMotionEvent *motion)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_mouse_motion,
        (_self, motion));
}

void _mume_sdl_backwin_handle_button_down(
    const void *_clazz, void *_self, SDL_MouseButtonEvent *button)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_button_down,
        (_self, button));
}

void _mume_sdl_backwin_handle_button_up(
    const void *_clazz, void *_self, SDL_MouseButtonEvent *button)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_button_up,
        (_self, button));
}

void _mume_sdl_backwin_handle_user_event(
    const void *_clazz, void *_self, SDL_UserEvent *user)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_user_event,
        (_self, user));
}

void _mume_sdl_backwin_handle_active_event(
    const void *_clazz, void *_self, SDL_ActiveEvent *active)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_active_event,
        (_self, active));
}

void _mume_sdl_backwin_handle_video_resize(
    const void *_clazz, void *_self, SDL_ResizeEvent *resize)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_video_resize,
        (_self, resize));
}

void _mume_sdl_backwin_handle_video_expose(
    const void *_clazz, void *_self, SDL_ExposeEvent *expose)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_video_expose,
        (_self, expose));
}

void _mume_sdl_backwin_handle_quit(
    const void *_clazz, void *_self, SDL_QuitEvent *quit)
{
    MUME_SELECTOR_NORETURN(
        mume_sdl_backwin_meta_class(), mume_sdl_backwin_class(),
        struct _sdl_backwin_class, handle_quit, (_self, quit));
}

int mume_sdl_translate_keysym(int keysym)
{
    /* keysym in mume is the same as SDL */
    return keysym;
}

int mume_sdl_translate_button(int button)
{
    /* button in mume is the same as SDL */
    return button;
}

int mume_sdl_translate_state(int keymod, int mousemod)
{
    int modifiers = 0;

    if (keymod & (KMOD_LSHIFT | KMOD_RSHIFT))
        modifiers |= MUME_MOD_SHIFT;

    if (keymod & (KMOD_LCTRL | KMOD_RCTRL))
        modifiers |= MUME_MOD_CONTROL;

    if (keymod & (KMOD_LALT | KMOD_RALT))
        modifiers |= MUME_MOD_ALT;

    if (keymod & (KMOD_LMETA | KMOD_RMETA))
        modifiers |= MUME_MOD_META;

    if (keymod & KMOD_CAPS)
        modifiers |= MUME_MOD_CAPS;

    if (SDL_BUTTON_LMASK & mousemod)
        modifiers |= MUME_MOD_LBUTTON;

    if (SDL_BUTTON_MMASK & mousemod)
        modifiers |= MUME_MOD_MBUTTON;

    if (SDL_BUTTON_RMASK & mousemod)
        modifiers |= MUME_MOD_RBUTTON;

    if (SDL_BUTTON_X1MASK & mousemod)
        modifiers |= MUME_MOD_XBUTTON1;

    if (SDL_BUTTON_X2MASK & mousemod)
        modifiers |= MUME_MOD_XBUTTON2;

    return modifiers;
}
