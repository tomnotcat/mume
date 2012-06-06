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
#include "msgproc.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-vector.h"
#include "mume-events.h"
#include "mume-frontend.h"
#include "backend.h"
#include "dbgutil.h"
#include "target.h"
#include "thunk.h"
#include MUME_ASSERT_H

#ifndef VK_0
#define VK_0	'0'
#define VK_1	'1'
#define VK_2	'2'
#define VK_3	'3'
#define VK_4	'4'
#define VK_5	'5'
#define VK_6	'6'
#define VK_7	'7'
#define VK_8	'8'
#define VK_9	'9'
#define VK_A	'A'
#define VK_B	'B'
#define VK_C	'C'
#define VK_D	'D'
#define VK_E	'E'
#define VK_F	'F'
#define VK_G	'G'
#define VK_H	'H'
#define VK_I	'I'
#define VK_J	'J'
#define VK_K	'K'
#define VK_L	'L'
#define VK_M	'M'
#define VK_N	'N'
#define VK_O	'O'
#define VK_P	'P'
#define VK_Q	'Q'
#define VK_R	'R'
#define VK_S	'S'
#define VK_T	'T'
#define VK_U	'U'
#define VK_V	'V'
#define VK_W	'W'
#define VK_X	'X'
#define VK_Y	'Y'
#define VK_Z	'Z'
#endif /* VK_0 */

/* These keys haven't been defined, but were experimentally determined */
#define VK_SEMICOLON	0xBA
#define VK_EQUALS	0xBB
#define VK_COMMA	0xBC
#define VK_MINUS	0xBD
#define VK_PERIOD	0xBE
#define VK_SLASH	0xBF
#define VK_GRAVE	0xC0
#define VK_LBRACKET	0xDB
#define VK_BACKSLASH	0xDC
#define VK_RBRACKET	0xDD
#define VK_APOSTROPHE	0xDE
#define VK_BACKTICK	0xDF
#define VK_OEM_102	0xE2

static win32_target_t *_win32_createwin_data = NULL;
static int VK_keymap[CHKEY_LAST];
static int Arrows_keymap[4];
static int _mouse_button_pressed;
static HWND _mouse_hover_window;

#define EXTENDED_KEYMASK (1<<24)

static HKL hLayoutUS = NULL;

static void _win32_init_keymap(void)
{
	int	i;
#ifndef _WIN32_WCE
	char	current_layout[KL_NAMELENGTH];

	GetKeyboardLayoutName(current_layout);
	//printf("Initial Keyboard Layout Name: '%s'\n", current_layout);

	hLayoutUS = LoadKeyboardLayout("00000409", KLF_NOTELLSHELL);

	if (!hLayoutUS) {
		//printf("Failed to load US keyboard layout. Using current.\n");
		hLayoutUS = GetKeyboardLayout(0);
	}
	LoadKeyboardLayout(current_layout, KLF_ACTIVATE);
#else
#if _WIN32_WCE >=420
	TCHAR	current_layout[KL_NAMELENGTH];

	GetKeyboardLayoutName(current_layout);
	//printf("Initial Keyboard Layout Name: '%s'\n", current_layout);

	hLayoutUS = LoadKeyboardLayout(L"00000409", 0);

	if (!hLayoutUS) {
		//printf("Failed to load US keyboard layout. Using current.\n");
		hLayoutUS = GetKeyboardLayout(0);
	}
	LoadKeyboardLayout(current_layout, 0);
#endif // _WIN32_WCE >=420
#endif
	/* Map the VK keysyms */
	for (i = 0; i< mume_countof(VK_keymap); ++i)
		VK_keymap[i] = CHKEY_UNKNOWN;

	VK_keymap[VK_BACK] = CHKEY_BACKSPACE;
	VK_keymap[VK_TAB] = CHKEY_TAB;
	VK_keymap[VK_CLEAR] = CHKEY_CLEAR;
	VK_keymap[VK_RETURN] = CHKEY_RETURN;
	VK_keymap[VK_PAUSE] = CHKEY_PAUSE;
	VK_keymap[VK_ESCAPE] = CHKEY_ESCAPE;
	VK_keymap[VK_SPACE] = CHKEY_SPACE;
	VK_keymap[VK_APOSTROPHE] = CHKEY_QUOTE;
	VK_keymap[VK_COMMA] = CHKEY_COMMA;
	VK_keymap[VK_MINUS] = CHKEY_MINUS;
	VK_keymap[VK_PERIOD] = CHKEY_PERIOD;
	VK_keymap[VK_SLASH] = CHKEY_SLASH;
	VK_keymap[VK_0] = CHKEY_0;
	VK_keymap[VK_1] = CHKEY_1;
	VK_keymap[VK_2] = CHKEY_2;
	VK_keymap[VK_3] = CHKEY_3;
	VK_keymap[VK_4] = CHKEY_4;
	VK_keymap[VK_5] = CHKEY_5;
	VK_keymap[VK_6] = CHKEY_6;
	VK_keymap[VK_7] = CHKEY_7;
	VK_keymap[VK_8] = CHKEY_8;
	VK_keymap[VK_9] = CHKEY_9;
	VK_keymap[VK_SEMICOLON] = CHKEY_SEMICOLON;
	VK_keymap[VK_EQUALS] = CHKEY_EQUALS;
	VK_keymap[VK_LBRACKET] = CHKEY_LEFTBRACKET;
	VK_keymap[VK_BACKSLASH] = CHKEY_BACKSLASH;
	VK_keymap[VK_OEM_102] = CHKEY_LESS;
	VK_keymap[VK_RBRACKET] = CHKEY_RIGHTBRACKET;
	VK_keymap[VK_GRAVE] = CHKEY_BACKQUOTE;
	VK_keymap[VK_BACKTICK] = CHKEY_BACKQUOTE;
	VK_keymap[VK_A] = CHKEY_A;
	VK_keymap[VK_B] = CHKEY_B;
	VK_keymap[VK_C] = CHKEY_C;
	VK_keymap[VK_D] = CHKEY_D;
	VK_keymap[VK_E] = CHKEY_E;
	VK_keymap[VK_F] = CHKEY_F;
	VK_keymap[VK_G] = CHKEY_G;
	VK_keymap[VK_H] = CHKEY_H;
	VK_keymap[VK_I] = CHKEY_I;
	VK_keymap[VK_J] = CHKEY_J;
	VK_keymap[VK_K] = CHKEY_K;
	VK_keymap[VK_L] = CHKEY_L;
	VK_keymap[VK_M] = CHKEY_M;
	VK_keymap[VK_N] = CHKEY_N;
	VK_keymap[VK_O] = CHKEY_O;
	VK_keymap[VK_P] = CHKEY_P;
	VK_keymap[VK_Q] = CHKEY_Q;
	VK_keymap[VK_R] = CHKEY_R;
	VK_keymap[VK_S] = CHKEY_S;
	VK_keymap[VK_T] = CHKEY_T;
	VK_keymap[VK_U] = CHKEY_U;
	VK_keymap[VK_V] = CHKEY_V;
	VK_keymap[VK_W] = CHKEY_W;
	VK_keymap[VK_X] = CHKEY_X;
	VK_keymap[VK_Y] = CHKEY_Y;
	VK_keymap[VK_Z] = CHKEY_Z;
	VK_keymap[VK_DELETE] = CHKEY_DELETE;

	VK_keymap[VK_NUMPAD0] = CHKEY_KP0;
	VK_keymap[VK_NUMPAD1] = CHKEY_KP1;
	VK_keymap[VK_NUMPAD2] = CHKEY_KP2;
	VK_keymap[VK_NUMPAD3] = CHKEY_KP3;
	VK_keymap[VK_NUMPAD4] = CHKEY_KP4;
	VK_keymap[VK_NUMPAD5] = CHKEY_KP5;
	VK_keymap[VK_NUMPAD6] = CHKEY_KP6;
	VK_keymap[VK_NUMPAD7] = CHKEY_KP7;
	VK_keymap[VK_NUMPAD8] = CHKEY_KP8;
	VK_keymap[VK_NUMPAD9] = CHKEY_KP9;
	VK_keymap[VK_DECIMAL] = CHKEY_KP_PERIOD;
	VK_keymap[VK_DIVIDE] = CHKEY_KP_DIVIDE;
	VK_keymap[VK_MULTIPLY] = CHKEY_KP_MULTIPLY;
	VK_keymap[VK_SUBTRACT] = CHKEY_KP_MINUS;
	VK_keymap[VK_ADD] = CHKEY_KP_PLUS;

	VK_keymap[VK_UP] = CHKEY_UP;
	VK_keymap[VK_DOWN] = CHKEY_DOWN;
	VK_keymap[VK_RIGHT] = CHKEY_RIGHT;
	VK_keymap[VK_LEFT] = CHKEY_LEFT;
	VK_keymap[VK_INSERT] = CHKEY_INSERT;
	VK_keymap[VK_HOME] = CHKEY_HOME;
	VK_keymap[VK_END] = CHKEY_END;
	VK_keymap[VK_PRIOR] = CHKEY_PAGEUP;
	VK_keymap[VK_NEXT] = CHKEY_PAGEDOWN;

	VK_keymap[VK_F1] = CHKEY_F1;
	VK_keymap[VK_F2] = CHKEY_F2;
	VK_keymap[VK_F3] = CHKEY_F3;
	VK_keymap[VK_F4] = CHKEY_F4;
	VK_keymap[VK_F5] = CHKEY_F5;
	VK_keymap[VK_F6] = CHKEY_F6;
	VK_keymap[VK_F7] = CHKEY_F7;
	VK_keymap[VK_F8] = CHKEY_F8;
	VK_keymap[VK_F9] = CHKEY_F9;
	VK_keymap[VK_F10] = CHKEY_F10;
	VK_keymap[VK_F11] = CHKEY_F11;
	VK_keymap[VK_F12] = CHKEY_F12;
	VK_keymap[VK_F13] = CHKEY_F13;
	VK_keymap[VK_F14] = CHKEY_F14;
	VK_keymap[VK_F15] = CHKEY_F15;

	VK_keymap[VK_NUMLOCK] = CHKEY_NUMLOCK;
	VK_keymap[VK_CAPITAL] = CHKEY_CAPSLOCK;
	VK_keymap[VK_SCROLL] = CHKEY_SCROLLOCK;
	VK_keymap[VK_RSHIFT] = CHKEY_RSHIFT;
	VK_keymap[VK_LSHIFT] = CHKEY_LSHIFT;
	VK_keymap[VK_RCONTROL] = CHKEY_RCTRL;
	VK_keymap[VK_LCONTROL] = CHKEY_LCTRL;
	VK_keymap[VK_RMENU] = CHKEY_RALT;
	VK_keymap[VK_LMENU] = CHKEY_LALT;
	VK_keymap[VK_RWIN] = CHKEY_RSUPER;
	VK_keymap[VK_LWIN] = CHKEY_LSUPER;

	VK_keymap[VK_HELP] = CHKEY_HELP;
#ifdef VK_PRINT
	VK_keymap[VK_PRINT] = CHKEY_PRINT;
#endif
	VK_keymap[VK_SNAPSHOT] = CHKEY_PRINT;
	VK_keymap[VK_CANCEL] = CHKEY_BREAK;
	VK_keymap[VK_APPS] = CHKEY_MENU;

	Arrows_keymap[3] = 0x25;
	Arrows_keymap[2] = 0x26;
	Arrows_keymap[1] = 0x27;
	Arrows_keymap[0] = 0x28;
}

#define EXTKEYPAD(keypad) ((scancode & 0x100)?(mvke):(keypad))

static int _win32_map_virtual_key(int scancode, int vkey)
{
#ifndef _WIN32_WCE
	int	mvke  = MapVirtualKeyEx(scancode & 0xFF, 1, hLayoutUS);
#else
	int	mvke  = MapVirtualKey(scancode & 0xFF, 1);
#endif

	switch(vkey) {
		/* These are always correct */
		case VK_DIVIDE:
		case VK_MULTIPLY:
		case VK_SUBTRACT:
		case VK_ADD:
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
		/* These are already handled */
		case VK_LCONTROL:
		case VK_RCONTROL:
		case VK_LSHIFT:
		case VK_RSHIFT:
		case VK_LMENU:
		case VK_RMENU:
		case VK_SNAPSHOT:
		case VK_PAUSE:
			return vkey;
	}	
	switch(mvke) {
		/* Distinguish between keypad and extended keys */
		case VK_INSERT: return EXTKEYPAD(VK_NUMPAD0);
		case VK_DELETE: return EXTKEYPAD(VK_DECIMAL);
		case VK_END:    return EXTKEYPAD(VK_NUMPAD1);
		case VK_DOWN:   return EXTKEYPAD(VK_NUMPAD2);
		case VK_NEXT:   return EXTKEYPAD(VK_NUMPAD3);
		case VK_LEFT:   return EXTKEYPAD(VK_NUMPAD4);
		case VK_CLEAR:  return EXTKEYPAD(VK_NUMPAD5);
		case VK_RIGHT:  return EXTKEYPAD(VK_NUMPAD6);
		case VK_HOME:   return EXTKEYPAD(VK_NUMPAD7);
		case VK_UP:     return EXTKEYPAD(VK_NUMPAD8);
		case VK_PRIOR:  return EXTKEYPAD(VK_NUMPAD9);
	}
	return mvke ? mvke : vkey;
}

static int _win32_get_vk(WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
        case VK_CONTROL:
            if (lParam & EXTENDED_KEYMASK)
                wParam = VK_RCONTROL;
            else
                wParam = VK_LCONTROL;
            break;
        case VK_SHIFT:
            /* EXTENDED trick doesn't work here */
            {
                if (!(GetKeyState(VK_LSHIFT) & 0x8000)) {
                    wParam = VK_LSHIFT;
                }
                else if (!(GetKeyState(VK_RSHIFT) & 0x8000)) {
                    wParam = VK_RSHIFT;
                }
                else {
                    /* Win9x */
                    int sc = HIWORD(lParam) & 0xFF;

                    if (sc == 0x2A)
                        wParam = VK_LSHIFT;
                    else if (sc == 0x36)
                        wParam = VK_RSHIFT;
                    else
                        wParam = VK_LSHIFT;
                }
            }
            break;
        case VK_MENU:
            if (lParam & EXTENDED_KEYMASK)
                wParam = VK_RMENU;
            else
                wParam = VK_LMENU;
            break;
    }
    return (int)wParam;
}

static DWORD _win32_get_message_pos(HWND hwnd, POINT *point)
{
    DWORD msgpos = GetMessagePos();
    POINT tmppt;
    if (NULL == point)
        point = &tmppt;
    point->x = GET_X_LPARAM(msgpos);
    point->y = GET_Y_LPARAM(msgpos);
    ScreenToClient(hwnd, point);
    return MAKELONG(point->x, point->y);
}

static int _win32_get_key_event_state(void)
{
    int state = 0;
    BYTE keystate[256];
    GetKeyboardState(keystate);
    if (keystate[VK_SHIFT] & 0x80)
        state |= CHMOD_SHIFT;
    if (keystate[VK_CONTROL] & 0x80)
        state |= CHMOD_CONTROL;
    if (keystate[VK_MENU] & 0x80)
        state |= CHMOD_ALT;
    if (keystate[VK_CAPITAL] & 0x01)
        state |= CHMOD_CAPS;
    if (keystate[VK_LBUTTON] & 0x80)
        state |= CHMOD_LBUTTON;
    if (keystate[VK_MBUTTON] & 0x80)
        state |= CHMOD_MBUTTON;
    if (keystate[VK_RBUTTON] & 0x80)
        state |= CHMOD_RBUTTON;
    if (keystate[VK_XBUTTON1] & 0x80)
        state |= CHMOD_XBUTTON1;
    if (keystate[VK_XBUTTON2] & 0x80)
        state |= CHMOD_XBUTTON2;
    return state;
}

static int _win32_translate_key(HWND hwnd, WPARAM vkey, UINT scancode)
{
	if ((vkey == VK_RETURN) && (scancode & 0x100)) {
		/* No VK_ code for the keypad enter key */
		return CHKEY_KP_ENTER;
	}
	return VK_keymap[_win32_map_virtual_key(scancode, vkey)];
}

static int _win32_get_mouse_event_state(WPARAM wParam)
{
    int state = 0;
    if (wParam & MK_SHIFT)
        state |= CHMOD_SHIFT;
    if (wParam & MK_CONTROL)
        state |= CHMOD_CONTROL;
    if (GetKeyState(VK_MENU) < 0)
        state |= CHMOD_ALT;
    if (GetKeyState(VK_CAPITAL) & 0x1)
        state |= CHMOD_CAPS;
    if (wParam & MK_LBUTTON)
        state |= CHMOD_LBUTTON;
    if (wParam & MK_MBUTTON)
        state |= CHMOD_MBUTTON;
    if (wParam & MK_RBUTTON)
        state |= CHMOD_RBUTTON;
    if (wParam & MK_XBUTTON1)
        state |= CHMOD_XBUTTON1;
    if (wParam & MK_XBUTTON2)
        state |= CHMOD_XBUTTON2;
    return state;
}

static void _proc_keydown(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state, key;
    POINT pos;
    state = _win32_get_key_event_state();
    key = _win32_translate_key(
        wtgt->hwnd, _win32_get_vk(wParam, lParam),
        HIWORD(lParam));
    _win32_get_message_pos(wtgt->hwnd, &pos);
    mume_frontend_handle_keydown(
        wback->front, &wtgt->base,
        pos.x, pos.y, state, key);
}

static void _proc_keyup(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state, key;
    POINT pos;
    state = _win32_get_key_event_state();
    key = _win32_translate_key(
        wtgt->hwnd, _win32_get_vk(wParam, lParam),
        HIWORD(lParam));
    _win32_get_message_pos(wtgt->hwnd, &pos);
    mume_frontend_handle_keyup(
        wback->front, &wtgt->base,
        pos.x, pos.y, state, key);
}

static void _proc_mousemove(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state = _win32_get_mouse_event_state(wParam);
    if (_mouse_hover_window != wtgt->hwnd) {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = wtgt->hwnd;
        if (TrackMouseEvent(&tme))
           _mouse_hover_window = wtgt->hwnd;
    }
    mume_frontend_handle_mousemotion(
        wback->front, &wtgt->base,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), state);
}

static void _proc_mouseleave(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state = _win32_get_key_event_state();
    POINT pos;
    if (_mouse_hover_window == wtgt->hwnd) {
        _mouse_hover_window = NULL;
    }
    _win32_get_message_pos(wtgt->hwnd, &pos);
    mume_frontend_handle_mousemotion(
        wback->front, &wtgt->base,
        pos.x, pos.y, state);
}

static void _proc_buttondown(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state = _win32_get_mouse_event_state(wParam);
    int button = 0;
    if (0 == _mouse_button_pressed)
        SetCapture(wtgt->hwnd);
    switch (message) {
    case WM_LBUTTONDOWN:
        button = CHBTN_LEFT;
        _mouse_button_pressed |= CHMOD_LBUTTON;
        break;
    case WM_MBUTTONDOWN:
        button = CHBTN_MIDDLE;
        _mouse_button_pressed |= CHMOD_MBUTTON;
        break;
    case WM_RBUTTONDOWN:
        button = CHBTN_RIGHT;
        _mouse_button_pressed |= CHMOD_RBUTTON;
        break;
    case WM_XBUTTONDOWN:
        if (HIWORD(wParam) == XBUTTON1) {
            button = CHBTN_X1;
            _mouse_button_pressed |= CHMOD_XBUTTON1;
        }
        else {
            button = CHBTN_X2;
            _mouse_button_pressed |= CHMOD_XBUTTON2;
        }
        break;
    }
    mume_frontend_handle_buttondown(
        wback->front, &wtgt->base,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
        state, button);
}

static void _proc_buttonup(
    win32_backend_t *wback, win32_target_t *wtgt,
    UINT message, WPARAM wParam, LPARAM lParam)
{
    int state = _win32_get_mouse_event_state(wParam);
    int button = 0;
    switch (message) {
    case WM_LBUTTONUP:
        button = CHBTN_LEFT;
        _mouse_button_pressed &= ~CHMOD_LBUTTON;
        break;
    case WM_MBUTTONUP:
        button = CHBTN_MIDDLE;
        _mouse_button_pressed &= ~CHMOD_MBUTTON;
        break;
    case WM_RBUTTONUP:
        button = CHBTN_RIGHT;
        _mouse_button_pressed &= ~CHMOD_RBUTTON;
        break;
    case WM_XBUTTONUP:
        if (HIWORD(wParam) == XBUTTON1) {
            button = CHBTN_X1;
            _mouse_button_pressed &= ~CHMOD_XBUTTON1;
        }
        else {
            button = CHBTN_X2;
            _mouse_button_pressed &= ~CHMOD_XBUTTON2;
        }
        break;
    }
    if (0 == _mouse_button_pressed)
        ReleaseCapture();
    mume_frontend_handle_buttonup(
        wback->front, &wtgt->base,
        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),
        state, button);
}

static LRESULT CALLBACK win32_win_proc(HWND hWnd, UINT message,
                                     WPARAM wParam, LPARAM lParam)
{
    win32_target_t *wtgt = (win32_target_t*)hWnd;
    win32_backend_t *wback = (win32_backend_t*)wtgt->base.back;
    win32_print_event(wback->logger, hWnd, message, wParam, lParam);
    switch (message) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        _proc_keydown(wback, wtgt, message, wParam, lParam);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        _proc_keyup(wback, wtgt, message, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        _proc_mousemove(wback, wtgt, message, wParam, lParam);
        break;
    case WM_MOUSELEAVE:
        _proc_mouseleave(wback, wtgt, message, wParam, lParam);
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
        _proc_buttondown(wback, wtgt, message, wParam, lParam);
        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
        _proc_buttonup(wback, wtgt, message, wParam, lParam);
        break;
    case WM_CAPTURECHANGED:
        if (_mouse_button_pressed) {
           /* just send ungrab event */
            _mouse_button_pressed = 0;
           /*
            DWORD msgpos = _win32_get_message_pos(wtgt->hwnd, NULL);
            if (CHMOD_LBUTTON & _mouse_button_pressed)
               _proc_buttonup(wback, wtgt, WM_LBUTTONUP, 0, msgpos);
            if (CHMOD_MBUTTON & _mouse_button_pressed)
               _proc_buttonup(wback, wtgt, WM_MBUTTONUP, 0, msgpos);
            if (CHMOD_RBUTTON & _mouse_button_pressed)
               _proc_buttonup(wback, wtgt, WM_RBUTTONUP, 0, msgpos);
            if (CHMOD_XBUTTON1 & _mouse_button_pressed)
               _proc_buttonup(wback, wtgt, WM_XBUTTONUP,
                              MAKELONG(0, XBUTTON1), msgpos);
            if (CHMOD_XBUTTON2 & _mouse_button_pressed)
               _proc_buttonup(wback, wtgt, WM_XBUTTONUP,
                              MAKELONG(0, XBUTTON2), msgpos);
            */
        }
        break;
    case WM_CLOSE:
        mume_frontend_handle_close(
            wback->front, &wtgt->base);
        break;
    case WM_QUIT:
        mume_frontend_handle_quit(wback->front);
        break;
    }
    return DefWindowProc(wtgt->hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK win32_start_proc(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam)
{
#pragma warning(disable: 4312)
#pragma warning(disable: 4244)
    WNDPROC new_proc;
    WNDPROC old_proc;
    assert(_win32_createwin_data);
    _win32_createwin_data->hwnd = hWnd;
    _win32_createwin_data->thunk = win32_create_thunk(
       (uintptr_t)&win32_win_proc, (uintptr_t)_win32_createwin_data);
    new_proc = (WNDPROC)win32_thunk_code(_win32_createwin_data->thunk);
    old_proc = (WNDPROC)SetWindowLongPtr(
        hWnd, GWLP_WNDPROC, (LONG_PTR)new_proc);
    return new_proc(hWnd, message, wParam, lParam);
}

void win32_initialize(void)
{
    WNDCLASSEX wcex;
    /* window class */
    wcex.cbSize = sizeof(wcex);
    wcex.style  = 0/*CS_HREDRAW | CS_VREDRAW*/ /* | CS_DBLCLKS*/;
    wcex.lpfnWndProc = &win32_start_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = NULL;
    wcex.hIcon = 0; /* LoadIcon(hInstance, (LPCTSTR)IDI_TESTWIN32WND); */
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = NULL; /* (HBRUSH)(COLOR_WINDOW+1); */
    wcex.lpszMenuName = NULL; /* (LPCTSTR)IDC_TESTWIN32WND; */
    wcex.lpszClassName  = WIN32_WINCLS_NAME;
    wcex.hIconSm = NULL; /* LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL); */
    RegisterClassEx(&wcex);

    _win32_init_keymap();
}

void win32_createwin_begin(win32_target_t *wtgt)
{
    assert(NULL == _win32_createwin_data);
    _win32_createwin_data = wtgt;
}

void win32_createwin_end(win32_target_t *wtgt)
{
    assert(wtgt == _win32_createwin_data);
    _win32_createwin_data = NULL;
}
