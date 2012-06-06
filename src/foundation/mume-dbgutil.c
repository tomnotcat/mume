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
#include "mume-dbgutil.h"
#include "mume-debug.h"
#include "mume-string.h"
#include "mume-events.h"
#include "mume-gstate.h"

static const char* _get_state_name(int state)
{
    static char buffer[256];
    int values[] = {
        MUME_MOD_SHIFT,
        MUME_MOD_CONTROL,
        MUME_MOD_ALT,
        MUME_MOD_META,
        MUME_MOD_CAPS,
        MUME_MOD_LBUTTON,
        MUME_MOD_MBUTTON,
        MUME_MOD_RBUTTON,
        MUME_MOD_XBUTTON1,
        MUME_MOD_XBUTTON2
    };

    const char *strings[] = {
        "SHIFT",
        "CONTROL",
        "ALT",
        "META",
        "CAPS",
        "LBUTTON",
        "MBUTTON",
        "RBUTTON",
        "XBUTTON1",
        "XBUTTON2"
    };

    return mume_mask_to_string(
        buffer, sizeof(buffer) / sizeof(buffer[0]),
        state, ",", values, strings,
        sizeof(values) / sizeof(values[0]));
}

static const char* _get_button_name(int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        return "LEFT";

    case MUME_BUTTON_MIDDLE:
        return "MIDDLE";

    case MUME_BUTTON_RIGHT:
        return "RIGHT";

    case MUME_BUTTON_WHEELUP:
        return "WHEELUP";

    case MUME_BUTTON_WHEELDOWN:
        return "WHEELDOWN";

    case MUME_BUTTON_X1:
        return "X1";

    case MUME_BUTTON_X2:
        return "X2";
    }

    return "UNKNOWN";
}

static const char* _get_notify_mode(int mode)
{
    switch (mode) {
    case MUME_NOTIFY_NORMAL:
        return "NORMAL";

    case MUME_NOTIFY_GRAB:
        return "GRAB";

    case MUME_NOTIFY_UNGRAB:
        return "UNGRAB";
    }

    return "UNKNOWN";
}

static const char* _get_notify_detail(int detail)
{
    switch (detail) {
    case MUME_NOTIFY_ANCESTOR:
        return "ANCESTOR";

    case MUME_NOTIFY_VIRTUAL:
        return "VIRTUAL";

    case MUME_NOTIFY_INFERIOR:
        return "INFERIOR";

    case MUME_NOTIFY_NONLINEAR:
        return "NONLINEAR";

    case MUME_NOTIFY_NONLINEARVIRTUAL:
        return "NONLINEARVIRTUAL";
    }

    return "UNKNOWN";
}

void mume_dump_event(const mume_event_t *event)
{
#define DEFINE_EVENT(_NAME) #_NAME,
    static char *event_names[] = {
        MUME_EVENT_LIST(DEFINE_EVENT)
    };
#undef DEFINE_EVENT

    static int serial;

    switch (event->type) {
    case MUME_EVENT_KEYDOWN:
    case MUME_EVENT_KEYUP:
        mume_trace0(
            ("%d: %s(%p): (%d, %d) [%s] %s\n",
             serial, event_names[event->type],
             event->key.window,
             event->key.x, event->key.y,
             _get_state_name(event->key.state),
             mume_key_name(event->key.keysym)));
        break;

    case MUME_EVENT_BUTTONDOWN:
    case MUME_EVENT_BUTTONUP:
        mume_trace1(
            ("%d: %s(%p): (%d, %d) [%s] %s\n",
             serial, event_names[event->type],
             event->button.window,
             event->button.x, event->button.y,
             _get_state_name(event->button.state),
             _get_button_name(event->button.button)));
        break;

    case MUME_EVENT_MOUSEMOTION:
        mume_trace3(
            ("%d: %s(%p): (%d, %d) [%s]\n",
             serial, event_names[event->type],
             event->motion.window,
             event->motion.x, event->motion.y,
             _get_state_name(event->motion.state)));
        break;

    case MUME_EVENT_MOUSEENTER:
    case MUME_EVENT_MOUSELEAVE:
        mume_trace1(
            ("%d: %s(%p): (%d, %d) [%s] [%s:%s]\n",
             serial, event_names[event->type],
             event->crossing.window,
             event->crossing.x, event->crossing.y,
             _get_state_name(event->crossing.state),
             _get_notify_mode(event->crossing.mode),
             _get_notify_detail(event->crossing.detail)));
        break;

    case MUME_EVENT_EXPOSE:
        mume_trace3(
            ("%d: %s(%p): [%d] (%d, %d, %d, %d)\n",
             serial, event_names[event->type],
             event->expose.window, event->expose.count,
             event->expose.x, event->expose.y,
             event->expose.width, event->expose.height));
        break;

    case MUME_EVENT_MOVE:
        mume_trace3(
            ("%d: %s(%p)(%p): (%d, %d) (%d, %d)\n",
             serial, event_names[event->type],
             event->move.event, event->move.window,
             event->move.x, event->move.y,
             event->move.old_x, event->move.old_y));
        break;

    default:
        mume_trace0(
            ("%d: %s(%p)\n",
             serial, event_names[event->type],
             event->any.window));
        break;
    }

    ++serial;
}

void mume_dump_region(const cairo_region_t *rgn)
{
    int i, num;
    mume_rect_t rect;

    num = cairo_region_num_rectangles(rgn);
    for (i = 0; i < num; ++i) {
        cairo_region_get_rectangle(rgn, i, &rect);
        mume_debug(("%d, %d, %d, %d\n",
                    rect.x, rect.y, rect.width, rect.height));
    }
}

const char* mume_key_name(int key)
{
    static const char *keynames[MUME_KEY_LAST];

    if (key < 0 || key >= MUME_KEY_LAST)
        return "unknown";

    if (NULL == keynames[key]) {
        keynames[MUME_KEY_BACKSPACE] = "backspace";
        keynames[MUME_KEY_TAB] = "tab";
        keynames[MUME_KEY_CLEAR] = "clear";
        keynames[MUME_KEY_RETURN] = "return";
        keynames[MUME_KEY_PAUSE] = "pause";
        keynames[MUME_KEY_ESCAPE] = "escape";
        keynames[MUME_KEY_SPACE] = "space";
        keynames[MUME_KEY_EXCLAIM]  = "!";
        keynames[MUME_KEY_QUOTEDBL]  = "\"";
        keynames[MUME_KEY_HASH]  = "#";
        keynames[MUME_KEY_DOLLAR]  = "$";
        keynames[MUME_KEY_AMPERSAND]  = "&";
        keynames[MUME_KEY_QUOTE] = "'";
        keynames[MUME_KEY_LEFTPAREN] = "(";
        keynames[MUME_KEY_RIGHTPAREN] = ")";
        keynames[MUME_KEY_ASTERISK] = "*";
        keynames[MUME_KEY_PLUS] = "+";
        keynames[MUME_KEY_COMMA] = ",";
        keynames[MUME_KEY_MINUS] = "-";
        keynames[MUME_KEY_PERIOD] = ".";
        keynames[MUME_KEY_SLASH] = "/";
        keynames[MUME_KEY_0] = "0";
        keynames[MUME_KEY_1] = "1";
        keynames[MUME_KEY_2] = "2";
        keynames[MUME_KEY_3] = "3";
        keynames[MUME_KEY_4] = "4";
        keynames[MUME_KEY_5] = "5";
        keynames[MUME_KEY_6] = "6";
        keynames[MUME_KEY_7] = "7";
        keynames[MUME_KEY_8] = "8";
        keynames[MUME_KEY_9] = "9";
        keynames[MUME_KEY_COLON] = ":";
        keynames[MUME_KEY_SEMICOLON] = ";";
        keynames[MUME_KEY_LESS] = "<";
        keynames[MUME_KEY_EQUALS] = "=";
        keynames[MUME_KEY_GREATER] = ">";
        keynames[MUME_KEY_QUESTION] = "?";
        keynames[MUME_KEY_AT] = "@";
        keynames[MUME_KEY_LEFTBRACKET] = "[";
        keynames[MUME_KEY_BACKSLASH] = "\\";
        keynames[MUME_KEY_RIGHTBRACKET] = "]";
        keynames[MUME_KEY_CARET] = "^";
        keynames[MUME_KEY_UNDERSCORE] = "_";
        keynames[MUME_KEY_BACKQUOTE] = "`";
        keynames[MUME_KEY_A] = "a";
        keynames[MUME_KEY_B] = "b";
        keynames[MUME_KEY_C] = "c";
        keynames[MUME_KEY_D] = "d";
        keynames[MUME_KEY_E] = "e";
        keynames[MUME_KEY_F] = "f";
        keynames[MUME_KEY_G] = "g";
        keynames[MUME_KEY_H] = "h";
        keynames[MUME_KEY_I] = "i";
        keynames[MUME_KEY_J] = "j";
        keynames[MUME_KEY_K] = "k";
        keynames[MUME_KEY_L] = "l";
        keynames[MUME_KEY_M] = "m";
        keynames[MUME_KEY_N] = "n";
        keynames[MUME_KEY_O] = "o";
        keynames[MUME_KEY_P] = "p";
        keynames[MUME_KEY_Q] = "q";
        keynames[MUME_KEY_R] = "r";
        keynames[MUME_KEY_S] = "s";
        keynames[MUME_KEY_T] = "t";
        keynames[MUME_KEY_U] = "u";
        keynames[MUME_KEY_V] = "v";
        keynames[MUME_KEY_W] = "w";
        keynames[MUME_KEY_X] = "x";
        keynames[MUME_KEY_Y] = "y";
        keynames[MUME_KEY_Z] = "z";
        keynames[MUME_KEY_DELETE] = "delete";

        keynames[MUME_KEY_WORLD_0] = "world 0";
        keynames[MUME_KEY_WORLD_1] = "world 1";
        keynames[MUME_KEY_WORLD_2] = "world 2";
        keynames[MUME_KEY_WORLD_3] = "world 3";
        keynames[MUME_KEY_WORLD_4] = "world 4";
        keynames[MUME_KEY_WORLD_5] = "world 5";
        keynames[MUME_KEY_WORLD_6] = "world 6";
        keynames[MUME_KEY_WORLD_7] = "world 7";
        keynames[MUME_KEY_WORLD_8] = "world 8";
        keynames[MUME_KEY_WORLD_9] = "world 9";
        keynames[MUME_KEY_WORLD_10] = "world 10";
        keynames[MUME_KEY_WORLD_11] = "world 11";
        keynames[MUME_KEY_WORLD_12] = "world 12";
        keynames[MUME_KEY_WORLD_13] = "world 13";
        keynames[MUME_KEY_WORLD_14] = "world 14";
        keynames[MUME_KEY_WORLD_15] = "world 15";
        keynames[MUME_KEY_WORLD_16] = "world 16";
        keynames[MUME_KEY_WORLD_17] = "world 17";
        keynames[MUME_KEY_WORLD_18] = "world 18";
        keynames[MUME_KEY_WORLD_19] = "world 19";
        keynames[MUME_KEY_WORLD_20] = "world 20";
        keynames[MUME_KEY_WORLD_21] = "world 21";
        keynames[MUME_KEY_WORLD_22] = "world 22";
        keynames[MUME_KEY_WORLD_23] = "world 23";
        keynames[MUME_KEY_WORLD_24] = "world 24";
        keynames[MUME_KEY_WORLD_25] = "world 25";
        keynames[MUME_KEY_WORLD_26] = "world 26";
        keynames[MUME_KEY_WORLD_27] = "world 27";
        keynames[MUME_KEY_WORLD_28] = "world 28";
        keynames[MUME_KEY_WORLD_29] = "world 29";
        keynames[MUME_KEY_WORLD_30] = "world 30";
        keynames[MUME_KEY_WORLD_31] = "world 31";
        keynames[MUME_KEY_WORLD_32] = "world 32";
        keynames[MUME_KEY_WORLD_33] = "world 33";
        keynames[MUME_KEY_WORLD_34] = "world 34";
        keynames[MUME_KEY_WORLD_35] = "world 35";
        keynames[MUME_KEY_WORLD_36] = "world 36";
        keynames[MUME_KEY_WORLD_37] = "world 37";
        keynames[MUME_KEY_WORLD_38] = "world 38";
        keynames[MUME_KEY_WORLD_39] = "world 39";
        keynames[MUME_KEY_WORLD_40] = "world 40";
        keynames[MUME_KEY_WORLD_41] = "world 41";
        keynames[MUME_KEY_WORLD_42] = "world 42";
        keynames[MUME_KEY_WORLD_43] = "world 43";
        keynames[MUME_KEY_WORLD_44] = "world 44";
        keynames[MUME_KEY_WORLD_45] = "world 45";
        keynames[MUME_KEY_WORLD_46] = "world 46";
        keynames[MUME_KEY_WORLD_47] = "world 47";
        keynames[MUME_KEY_WORLD_48] = "world 48";
        keynames[MUME_KEY_WORLD_49] = "world 49";
        keynames[MUME_KEY_WORLD_50] = "world 50";
        keynames[MUME_KEY_WORLD_51] = "world 51";
        keynames[MUME_KEY_WORLD_52] = "world 52";
        keynames[MUME_KEY_WORLD_53] = "world 53";
        keynames[MUME_KEY_WORLD_54] = "world 54";
        keynames[MUME_KEY_WORLD_55] = "world 55";
        keynames[MUME_KEY_WORLD_56] = "world 56";
        keynames[MUME_KEY_WORLD_57] = "world 57";
        keynames[MUME_KEY_WORLD_58] = "world 58";
        keynames[MUME_KEY_WORLD_59] = "world 59";
        keynames[MUME_KEY_WORLD_60] = "world 60";
        keynames[MUME_KEY_WORLD_61] = "world 61";
        keynames[MUME_KEY_WORLD_62] = "world 62";
        keynames[MUME_KEY_WORLD_63] = "world 63";
        keynames[MUME_KEY_WORLD_64] = "world 64";
        keynames[MUME_KEY_WORLD_65] = "world 65";
        keynames[MUME_KEY_WORLD_66] = "world 66";
        keynames[MUME_KEY_WORLD_67] = "world 67";
        keynames[MUME_KEY_WORLD_68] = "world 68";
        keynames[MUME_KEY_WORLD_69] = "world 69";
        keynames[MUME_KEY_WORLD_70] = "world 70";
        keynames[MUME_KEY_WORLD_71] = "world 71";
        keynames[MUME_KEY_WORLD_72] = "world 72";
        keynames[MUME_KEY_WORLD_73] = "world 73";
        keynames[MUME_KEY_WORLD_74] = "world 74";
        keynames[MUME_KEY_WORLD_75] = "world 75";
        keynames[MUME_KEY_WORLD_76] = "world 76";
        keynames[MUME_KEY_WORLD_77] = "world 77";
        keynames[MUME_KEY_WORLD_78] = "world 78";
        keynames[MUME_KEY_WORLD_79] = "world 79";
        keynames[MUME_KEY_WORLD_80] = "world 80";
        keynames[MUME_KEY_WORLD_81] = "world 81";
        keynames[MUME_KEY_WORLD_82] = "world 82";
        keynames[MUME_KEY_WORLD_83] = "world 83";
        keynames[MUME_KEY_WORLD_84] = "world 84";
        keynames[MUME_KEY_WORLD_85] = "world 85";
        keynames[MUME_KEY_WORLD_86] = "world 86";
        keynames[MUME_KEY_WORLD_87] = "world 87";
        keynames[MUME_KEY_WORLD_88] = "world 88";
        keynames[MUME_KEY_WORLD_89] = "world 89";
        keynames[MUME_KEY_WORLD_90] = "world 90";
        keynames[MUME_KEY_WORLD_91] = "world 91";
        keynames[MUME_KEY_WORLD_92] = "world 92";
        keynames[MUME_KEY_WORLD_93] = "world 93";
        keynames[MUME_KEY_WORLD_94] = "world 94";
        keynames[MUME_KEY_WORLD_95] = "world 95";

        keynames[MUME_KEY_KP0] = "[0]";
        keynames[MUME_KEY_KP1] = "[1]";
        keynames[MUME_KEY_KP2] = "[2]";
        keynames[MUME_KEY_KP3] = "[3]";
        keynames[MUME_KEY_KP4] = "[4]";
        keynames[MUME_KEY_KP5] = "[5]";
        keynames[MUME_KEY_KP6] = "[6]";
        keynames[MUME_KEY_KP7] = "[7]";
        keynames[MUME_KEY_KP8] = "[8]";
        keynames[MUME_KEY_KP9] = "[9]";
        keynames[MUME_KEY_KP_PERIOD] = "[.]";
        keynames[MUME_KEY_KP_DIVIDE] = "[/]";
        keynames[MUME_KEY_KP_MULTIPLY] = "[*]";
        keynames[MUME_KEY_KP_MINUS] = "[-]";
        keynames[MUME_KEY_KP_PLUS] = "[+]";
        keynames[MUME_KEY_KP_ENTER] = "enter";
        keynames[MUME_KEY_KP_EQUALS] = "equals";

        keynames[MUME_KEY_UP] = "up";
        keynames[MUME_KEY_DOWN] = "down";
        keynames[MUME_KEY_RIGHT] = "right";
        keynames[MUME_KEY_LEFT] = "left";
        keynames[MUME_KEY_DOWN] = "down";
        keynames[MUME_KEY_INSERT] = "insert";
        keynames[MUME_KEY_HOME] = "home";
        keynames[MUME_KEY_END] = "end";
        keynames[MUME_KEY_PAGEUP] = "page up";
        keynames[MUME_KEY_PAGEDOWN] = "page down";

        keynames[MUME_KEY_F1] = "f1";
        keynames[MUME_KEY_F2] = "f2";
        keynames[MUME_KEY_F3] = "f3";
        keynames[MUME_KEY_F4] = "f4";
        keynames[MUME_KEY_F5] = "f5";
        keynames[MUME_KEY_F6] = "f6";
        keynames[MUME_KEY_F7] = "f7";
        keynames[MUME_KEY_F8] = "f8";
        keynames[MUME_KEY_F9] = "f9";
        keynames[MUME_KEY_F10] = "f10";
        keynames[MUME_KEY_F11] = "f11";
        keynames[MUME_KEY_F12] = "f12";
        keynames[MUME_KEY_F13] = "f13";
        keynames[MUME_KEY_F14] = "f14";
        keynames[MUME_KEY_F15] = "f15";

        keynames[MUME_KEY_NUMLOCK] = "numlock";
        keynames[MUME_KEY_CAPSLOCK] = "caps lock";
        keynames[MUME_KEY_SCROLLOCK] = "scroll lock";
        keynames[MUME_KEY_RSHIFT] = "right shift";
        keynames[MUME_KEY_LSHIFT] = "left shift";
        keynames[MUME_KEY_RCTRL] = "right ctrl";
        keynames[MUME_KEY_LCTRL] = "left ctrl";
        keynames[MUME_KEY_RALT] = "right alt";
        keynames[MUME_KEY_LALT] = "left alt";
        keynames[MUME_KEY_RMETA] = "right meta";
        keynames[MUME_KEY_LMETA] = "left meta";
        keynames[MUME_KEY_LSUPER] = "left super";/* "Windows" keys */
        keynames[MUME_KEY_RSUPER] = "right super";
        keynames[MUME_KEY_MODE] = "alt gr";
        keynames[MUME_KEY_COMPOSE] = "compose";

        keynames[MUME_KEY_HELP] = "help";
        keynames[MUME_KEY_PRINT] = "print screen";
        keynames[MUME_KEY_SYSREQ] = "sys req";
        keynames[MUME_KEY_BREAK] = "break";
        keynames[MUME_KEY_MENU] = "menu";
        keynames[MUME_KEY_POWER] = "power";
        keynames[MUME_KEY_EURO] = "euro";
        keynames[MUME_KEY_UNDO] = "undo";
    }

    return keynames[key];
}
