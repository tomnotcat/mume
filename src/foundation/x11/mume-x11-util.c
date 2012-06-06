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
#include "mume-x11-util.h"
#include "mume-events.h"

static int ODD_keymap[256];
static int MISC_keymap[256];

static Bool _timestamp_predicate(
    Display *display, XEvent  *xevent, XPointer arg)
{
    Window window = (Window)arg;
    Atom timestamp_prop_atom;

    timestamp_prop_atom = XInternAtom(
        display, "MUME_TIMESTAMP_PROP", False);

    if (xevent->type == PropertyNotify &&
        xevent->xproperty.window == window &&
        xevent->xproperty.atom == timestamp_prop_atom)
    {
        return True;
    }

    return False;
}

void mume_x11_init_keymap(void)
{
    int i;
    /* Odd keys used in international keyboards */
    for (i=0; i < COUNT_OF(ODD_keymap); ++i)
        ODD_keymap[i] = MUME_KEY_UNKNOWN;

#ifdef XK_dead_circumflex
    /* These X keysyms have 0xFE as the high byte */
    ODD_keymap[XK_dead_circumflex&0xFF] = MUME_KEY_CARET;
#endif
#ifdef XK_ISO_Level3_Shift
    ODD_keymap[XK_ISO_Level3_Shift&0xFF] = MUME_KEY_MODE; /* "Alt Gr" key */
#endif

    /* Map the miscellaneous keys */
    for (i=0; i < COUNT_OF(MISC_keymap); ++i)
        MISC_keymap[i] = MUME_KEY_UNKNOWN;

    /* These X keysyms have 0xFF as the high byte */
    MISC_keymap[XK_BackSpace&0xFF] = MUME_KEY_BACKSPACE;
    MISC_keymap[XK_Tab&0xFF] = MUME_KEY_TAB;
    MISC_keymap[XK_Clear&0xFF] = MUME_KEY_CLEAR;
    MISC_keymap[XK_Return&0xFF] = MUME_KEY_RETURN;
    MISC_keymap[XK_Pause&0xFF] = MUME_KEY_PAUSE;
    MISC_keymap[XK_Escape&0xFF] = MUME_KEY_ESCAPE;
    MISC_keymap[XK_Delete&0xFF] = MUME_KEY_DELETE;

    MISC_keymap[XK_KP_0&0xFF] = MUME_KEY_KP0;/* Keypad 0-9 */
    MISC_keymap[XK_KP_1&0xFF] = MUME_KEY_KP1;
    MISC_keymap[XK_KP_2&0xFF] = MUME_KEY_KP2;
    MISC_keymap[XK_KP_3&0xFF] = MUME_KEY_KP3;
    MISC_keymap[XK_KP_4&0xFF] = MUME_KEY_KP4;
    MISC_keymap[XK_KP_5&0xFF] = MUME_KEY_KP5;
    MISC_keymap[XK_KP_6&0xFF] = MUME_KEY_KP6;
    MISC_keymap[XK_KP_7&0xFF] = MUME_KEY_KP7;
    MISC_keymap[XK_KP_8&0xFF] = MUME_KEY_KP8;
    MISC_keymap[XK_KP_9&0xFF] = MUME_KEY_KP9;
    MISC_keymap[XK_KP_Insert&0xFF] = MUME_KEY_KP0;
    MISC_keymap[XK_KP_End&0xFF] = MUME_KEY_KP1;
    MISC_keymap[XK_KP_Down&0xFF] = MUME_KEY_KP2;
    MISC_keymap[XK_KP_Page_Down&0xFF] = MUME_KEY_KP3;
    MISC_keymap[XK_KP_Left&0xFF] = MUME_KEY_KP4;
    MISC_keymap[XK_KP_Begin&0xFF] = MUME_KEY_KP5;
    MISC_keymap[XK_KP_Right&0xFF] = MUME_KEY_KP6;
    MISC_keymap[XK_KP_Home&0xFF] = MUME_KEY_KP7;
    MISC_keymap[XK_KP_Up&0xFF] = MUME_KEY_KP8;
    MISC_keymap[XK_KP_Page_Up&0xFF] = MUME_KEY_KP9;
    MISC_keymap[XK_KP_Delete&0xFF] = MUME_KEY_KP_PERIOD;
    MISC_keymap[XK_KP_Decimal&0xFF] = MUME_KEY_KP_PERIOD;
    MISC_keymap[XK_KP_Divide&0xFF] = MUME_KEY_KP_DIVIDE;
    MISC_keymap[XK_KP_Multiply&0xFF] = MUME_KEY_KP_MULTIPLY;
    MISC_keymap[XK_KP_Subtract&0xFF] = MUME_KEY_KP_MINUS;
    MISC_keymap[XK_KP_Add&0xFF] = MUME_KEY_KP_PLUS;
    MISC_keymap[XK_KP_Enter&0xFF] = MUME_KEY_KP_ENTER;
    MISC_keymap[XK_KP_Equal&0xFF] = MUME_KEY_KP_EQUALS;

    MISC_keymap[XK_Up&0xFF] = MUME_KEY_UP;
    MISC_keymap[XK_Down&0xFF] = MUME_KEY_DOWN;
    MISC_keymap[XK_Right&0xFF] = MUME_KEY_RIGHT;
    MISC_keymap[XK_Left&0xFF] = MUME_KEY_LEFT;
    MISC_keymap[XK_Insert&0xFF] = MUME_KEY_INSERT;
    MISC_keymap[XK_Home&0xFF] = MUME_KEY_HOME;
    MISC_keymap[XK_End&0xFF] = MUME_KEY_END;
    MISC_keymap[XK_Page_Up&0xFF] = MUME_KEY_PAGEUP;
    MISC_keymap[XK_Page_Down&0xFF] = MUME_KEY_PAGEDOWN;

    MISC_keymap[XK_F1&0xFF] = MUME_KEY_F1;
    MISC_keymap[XK_F2&0xFF] = MUME_KEY_F2;
    MISC_keymap[XK_F3&0xFF] = MUME_KEY_F3;
    MISC_keymap[XK_F4&0xFF] = MUME_KEY_F4;
    MISC_keymap[XK_F5&0xFF] = MUME_KEY_F5;
    MISC_keymap[XK_F6&0xFF] = MUME_KEY_F6;
    MISC_keymap[XK_F7&0xFF] = MUME_KEY_F7;
    MISC_keymap[XK_F8&0xFF] = MUME_KEY_F8;
    MISC_keymap[XK_F9&0xFF] = MUME_KEY_F9;
    MISC_keymap[XK_F10&0xFF] = MUME_KEY_F10;
    MISC_keymap[XK_F11&0xFF] = MUME_KEY_F11;
    MISC_keymap[XK_F12&0xFF] = MUME_KEY_F12;
    MISC_keymap[XK_F13&0xFF] = MUME_KEY_F13;
    MISC_keymap[XK_F14&0xFF] = MUME_KEY_F14;
    MISC_keymap[XK_F15&0xFF] = MUME_KEY_F15;

    MISC_keymap[XK_Num_Lock&0xFF] = MUME_KEY_NUMLOCK;
    MISC_keymap[XK_Caps_Lock&0xFF] = MUME_KEY_CAPSLOCK;
    MISC_keymap[XK_Scroll_Lock&0xFF] = MUME_KEY_SCROLLOCK;
    MISC_keymap[XK_Shift_R&0xFF] = MUME_KEY_RSHIFT;
    MISC_keymap[XK_Shift_L&0xFF] = MUME_KEY_LSHIFT;
    MISC_keymap[XK_Control_R&0xFF] = MUME_KEY_RCTRL;
    MISC_keymap[XK_Control_L&0xFF] = MUME_KEY_LCTRL;
    MISC_keymap[XK_Alt_R&0xFF] = MUME_KEY_RALT;
    MISC_keymap[XK_Alt_L&0xFF] = MUME_KEY_LALT;
    MISC_keymap[XK_Meta_R&0xFF] = MUME_KEY_RMETA;
    MISC_keymap[XK_Meta_L&0xFF] = MUME_KEY_LMETA;
    MISC_keymap[XK_Super_L&0xFF] = MUME_KEY_LSUPER; /* Left "Windows" */
    MISC_keymap[XK_Super_R&0xFF] = MUME_KEY_RSUPER; /* Right "Windows */
    MISC_keymap[XK_Mode_switch&0xFF] = MUME_KEY_MODE; /* "Alt Gr" key */
    MISC_keymap[XK_Multi_key&0xFF] = MUME_KEY_COMPOSE; /* Multi-key compose */

    MISC_keymap[XK_Help&0xFF] = MUME_KEY_HELP;
    MISC_keymap[XK_Print&0xFF] = MUME_KEY_PRINT;
    MISC_keymap[XK_Sys_Req&0xFF] = MUME_KEY_SYSREQ;
    MISC_keymap[XK_Break&0xFF] = MUME_KEY_BREAK;
    MISC_keymap[XK_Menu&0xFF] = MUME_KEY_MENU;
    MISC_keymap[XK_Hyper_R&0xFF] = MUME_KEY_MENU;   /* Windows "Menu" key */
}

int mume_x11_translate_notify_mode(int xmode)
{
    return xmode;
}

int mume_x11_translate_key(Display *display, KeyCode kc)
{
    KeySym xsym;
    int key = MUME_KEY_UNKNOWN;
    xsym = XKeycodeToKeysym(display, kc, 0);
    if (xsym) {
        switch (xsym >> 8) {
        case 0x1005FF:
#ifdef SunXK_F36
            if (xsym == SunXK_F36)
                key = MUME_KEY_F11;
#endif
#ifdef SunXK_F37
            if (xsym == SunXK_F37)
                key = MUME_KEY_F12;
#endif
            break;
        case 0x00:/* Latin 1 */
            key = (int)(xsym & 0xFF);
            break;
        case 0x01:/* Latin 2 */
        case 0x02:/* Latin 3 */
        case 0x03:/* Latin 4 */
        case 0x04:/* Katakana */
        case 0x05:/* Arabic */
        case 0x06:/* Cyrillic */
        case 0x07:/* Greek */
        case 0x08:/* Technical */
        case 0x0A:/* Publishing */
        case 0x0C:/* Hebrew */
        case 0x0D:/* Thai */
            /* These are wrong, but it's better than nothing */
            key = (int)(xsym & 0xFF);
            break;
        case 0xFE:
            key = ODD_keymap[xsym & 0xFF];
            break;
        case 0xFF:
            key = MISC_keymap[xsym & 0xFF];
            break;
        }
    }
    else {
        /* X11 doesn't know how to translate the key! */
        switch (kc) {
            /* Caution:
                      These keycodes are from the Microsoft Keyboard
            */
        case 115:
            key = MUME_KEY_LSUPER;
            break;
        case 116:
            key = MUME_KEY_RSUPER;
            break;
        case 117:
            key = MUME_KEY_MENU;
            break;
        default:
            /*
             * no point in an error message; happens for
             * several keys when we get a keymap notify
             */
            break;
        }
    }
    return key;
}

int mume_x11_translate_button(unsigned int xbutton)
{
    return xbutton;
}

int mume_x11_translate_state(unsigned int xstate)
{
    int modifiers = 0;
    if (ShiftMask & xstate)
        modifiers |= MUME_MOD_SHIFT;

    if (ControlMask & xstate)
        modifiers |= MUME_MOD_CONTROL;

    if (Mod1Mask & xstate)
        modifiers |= MUME_MOD_ALT;

    if (Mod4Mask & xstate)
        modifiers |= MUME_MOD_META;

    if (LockMask & xstate)
        modifiers |= MUME_MOD_CAPS;

    if (Button1Mask & xstate)
        modifiers |= MUME_MOD_LBUTTON;

    if (Button2Mask & xstate)
        modifiers |= MUME_MOD_MBUTTON;

    if (Button3Mask & xstate)
        modifiers |= MUME_MOD_RBUTTON;
    /*
    if (Button4Mask & xstate)
        modifiers |= MUME_MOD_WHEELUP;
    if (Button5Mask & xstate)
        modifiers |= MUME_MOD_WHEELDOWN;
    */
    return modifiers;
}

Time mume_x11_get_server_time(Display *display, Window window)
{
    XEvent xevent;
    unsigned char c = 'a';
    Atom timestamp_prop_atom;

    timestamp_prop_atom = XInternAtom(
        display, "MUME_TIMESTAMP_PROP", False);

    XChangeProperty(display, window, timestamp_prop_atom,
                    timestamp_prop_atom,
                    8, PropModeReplace, &c, 1);

    XIfEvent(display, &xevent,
             _timestamp_predicate, (XPointer)window);

    return xevent.xproperty.time;
}
