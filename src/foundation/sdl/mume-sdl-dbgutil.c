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
#include "mume-sdl-dbgutil.h"
#include "mume-debug.h"
#include "mume-string.h"

static const char* _sdl_get_mod_name(int mod)
{
    static char buffer[256];
    int values[] = {
        KMOD_LSHIFT,
        KMOD_RSHIFT,
        KMOD_LCTRL,
        KMOD_RCTRL,
        KMOD_LALT,
        KMOD_RALT,
        KMOD_LMETA,
        KMOD_RMETA,
        KMOD_NUM,
        KMOD_CAPS,
        KMOD_MODE,
        KMOD_RESERVED
    };

    const char *strings[] = {
        "LSHIFT",
        "RSHIFT",
        "LCTRL",
        "RCTRL",
        "LALT",
        "RALT",
        "LMETA",
        "RMETA",
        "NUM",
        "CAPS",
        "MODE",
        "RESERVED"
    };

    return mume_mask_to_string(
        buffer, sizeof(buffer) / sizeof(buffer[0]),
        mod, ",", values, strings,
        sizeof(values) / sizeof(values[0]));
}

static const char* _sdl_get_button_name(int button)
{
    switch (button) {
    case SDL_BUTTON_LEFT:
        return "LEFT";

    case SDL_BUTTON_MIDDLE:
        return "MIDDLE";

    case SDL_BUTTON_RIGHT:
        return "RIGHT";

    case SDL_BUTTON_WHEELUP:
        return "WHEELUP";

    case SDL_BUTTON_WHEELDOWN:
        return "WHEELDOWN";

    case SDL_BUTTON_X1:
        return "X1";

    case SDL_BUTTON_X2:
        return "X2";
    }

    return "UNKNOWN";
}

static const char* _sdl_get_button_state(int state)
{
    static char buffer[256];
    int values[] = {
        SDL_BUTTON_LMASK,
        SDL_BUTTON_MMASK,
        SDL_BUTTON_RMASK,
        SDL_BUTTON_X1MASK,
        SDL_BUTTON_X2MASK
    };

    const char *strings[] = {
        "LMASK",
        "MMASK",
        "RMASK",
        "X1MASK",
        "X2MASK"
    };

    return mume_mask_to_string(
        buffer, sizeof(buffer) / sizeof(buffer[0]),
        state, ",", values, strings,
        sizeof(values) / sizeof(values[0]));
}

static const char* _sdl_get_active_name(int state)
{
    static char buffer[256];
    int values[] = {
        SDL_APPMOUSEFOCUS,
        SDL_APPINPUTFOCUS,
        SDL_APPACTIVE
    };

    const char *strings[] = {
        "APPMOUSEFOCUS",
        "APPINPUTFOCUS",
        "APPACTIVE"
    };

    return mume_mask_to_string(
        buffer, sizeof(buffer) / sizeof(buffer[0]),
        state, ",", values, strings,
        sizeof(values) / sizeof(values[0]));
}

void mume_sdl_print_event(SDL_Event *event)
{
    static char *event_names[] = {
        "NOEVENT",
        "ACTIVEEVENT",
        "KEYDOWN",
        "KEYUP",
        "MOUSEMOTION",
        "MOUSEBUTTONDOWN",
        "MOUSEBUTTONUP",
        "JOYAXISMOTION",
        "JOYBALLMOTION",
        "JOYHATMOTION",
        "JOYBUTTONDOWN",
        "JOYBUTTONUP",
        "QUIT",
        "SYSWMEVENT",
        "RESERVEDA",
        "RESERVEDB",
        "VIDEORESIZE",
        "VIDEOEXPOSE",
        "RESERVED2",
        "RESERVED3",
        "RESERVED4",
        "RESERVED5",
        "RESERVED6",
        "RESERVED7",
        "USEREVENT",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        ""
    };

    switch (event->type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        mume_trace2(
            ("%s: [%s] %s %d %d\n",
             event_names[event->type],
             _sdl_get_mod_name(event->key.keysym.mod),
             SDL_GetKeyName(event->key.keysym.sym),
             event->key.keysym.sym,
             event->key.keysym.unicode));
        break;

    case SDL_MOUSEMOTION:
        mume_trace3(
            ("%s: [%d:%d] [%s] (%d, %d) (%d, %d)\n",
             event_names[event->type],
             event->motion.which,
             event->motion.state,
             _sdl_get_button_state(event->motion.state),
             event->motion.x, event->motion.y,
             event->motion.xrel, event->motion.yrel));
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        mume_trace2(
            ("%s: [%d:%d] [%s] %s (%d, %d)\n",
             event_names[event->type],
             event->button.which,
             event->button.state,
             _sdl_get_button_state(SDL_GetMouseState(NULL, NULL)),
             _sdl_get_button_name(event->button.button),
             event->button.x, event->button.y));
        break;

    case SDL_ACTIVEEVENT:
        mume_trace1(
            ("%s: %d [%s]\n",
             event_names[event->type],
             event->active.gain,
             _sdl_get_active_name(event->active.state)));
        break;

    default:
        mume_trace0(("%s\n", event_names[event->type]));
        break;
    }
}
