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
#include "dbgutil.h"
#include "mume-debug.h"

static const char* _win32_message_to_string(UINT msg)
{
    switch (msg) {
#define _CASE_MSG(x) case x: return #x
        _CASE_MSG(WM_NULL);
        _CASE_MSG(WM_CREATE);
        _CASE_MSG(WM_DESTROY);
        _CASE_MSG(WM_MOVE);
        _CASE_MSG(WM_SIZE);
        _CASE_MSG(WM_ACTIVATE);
        _CASE_MSG(WM_SETFOCUS);
        _CASE_MSG(WM_KILLFOCUS);
        _CASE_MSG(WM_ENABLE);
        _CASE_MSG(WM_SETREDRAW);
        _CASE_MSG(WM_SETTEXT);
        _CASE_MSG(WM_GETTEXT);
        _CASE_MSG(WM_GETTEXTLENGTH);
        _CASE_MSG(WM_PAINT);
        _CASE_MSG(WM_CLOSE);
        _CASE_MSG(WM_QUERYENDSESSION);
        _CASE_MSG(WM_QUERYOPEN);
        _CASE_MSG(WM_ENDSESSION);
        _CASE_MSG(WM_QUIT);
        _CASE_MSG(WM_ERASEBKGND);
        _CASE_MSG(WM_SYSCOLORCHANGE);
        _CASE_MSG(WM_SHOWWINDOW);
        _CASE_MSG(WM_WININICHANGE);
        _CASE_MSG(WM_DEVMODECHANGE);
        _CASE_MSG(WM_ACTIVATEAPP);
        _CASE_MSG(WM_FONTCHANGE);
        _CASE_MSG(WM_TIMECHANGE);
        _CASE_MSG(WM_CANCELMODE);
        _CASE_MSG(WM_SETCURSOR);
        _CASE_MSG(WM_MOUSEACTIVATE);
        _CASE_MSG(WM_CHILDACTIVATE);
        _CASE_MSG(WM_QUEUESYNC);
        _CASE_MSG(WM_GETMINMAXINFO);
        _CASE_MSG(WM_PAINTICON);
        _CASE_MSG(WM_ICONERASEBKGND);
        _CASE_MSG(WM_NEXTDLGCTL);
        _CASE_MSG(WM_SPOOLERSTATUS);
        _CASE_MSG(WM_DRAWITEM);
        _CASE_MSG(WM_MEASUREITEM);
        _CASE_MSG(WM_DELETEITEM);
        _CASE_MSG(WM_VKEYTOITEM);
        _CASE_MSG(WM_CHARTOITEM);
        _CASE_MSG(WM_SETFONT);
        _CASE_MSG(WM_GETFONT);
        _CASE_MSG(WM_SETHOTKEY);
        _CASE_MSG(WM_GETHOTKEY);
        _CASE_MSG(WM_QUERYDRAGICON);
        _CASE_MSG(WM_COMPAREITEM);
        _CASE_MSG(WM_GETOBJECT);
        _CASE_MSG(WM_COMPACTING);
        _CASE_MSG(WM_WINDOWPOSCHANGING);
        _CASE_MSG(WM_WINDOWPOSCHANGED);
        _CASE_MSG(WM_POWER);
        _CASE_MSG(WM_COPYDATA);
        _CASE_MSG(WM_CANCELJOURNAL);
        _CASE_MSG(WM_NOTIFY);
        _CASE_MSG(WM_INPUTLANGCHANGEREQUEST);
        _CASE_MSG(WM_INPUTLANGCHANGE);
        _CASE_MSG(WM_TCARD);
        _CASE_MSG(WM_HELP);
        _CASE_MSG(WM_USERCHANGED);
        _CASE_MSG(WM_NOTIFYFORMAT);
        _CASE_MSG(WM_CONTEXTMENU);
        _CASE_MSG(WM_STYLECHANGING);
        _CASE_MSG(WM_STYLECHANGED);
        _CASE_MSG(WM_DISPLAYCHANGE);
        _CASE_MSG(WM_GETICON);
        _CASE_MSG(WM_SETICON);
        _CASE_MSG(WM_NCCREATE);
        _CASE_MSG(WM_NCDESTROY);
        _CASE_MSG(WM_NCCALCSIZE);
        _CASE_MSG(WM_NCHITTEST);
        _CASE_MSG(WM_NCPAINT);
        _CASE_MSG(WM_NCACTIVATE);
        _CASE_MSG(WM_GETDLGCODE);
        _CASE_MSG(WM_SYNCPAINT);
        _CASE_MSG(WM_NCMOUSEMOVE);
        _CASE_MSG(WM_NCLBUTTONDOWN);
        _CASE_MSG(WM_NCLBUTTONUP);
        _CASE_MSG(WM_NCLBUTTONDBLCLK);
        _CASE_MSG(WM_NCRBUTTONDOWN);
        _CASE_MSG(WM_NCRBUTTONUP);
        _CASE_MSG(WM_NCRBUTTONDBLCLK);
        _CASE_MSG(WM_NCMBUTTONDOWN);
        _CASE_MSG(WM_NCMBUTTONUP);
        _CASE_MSG(WM_NCMBUTTONDBLCLK);
        _CASE_MSG(WM_NCXBUTTONDOWN);
        _CASE_MSG(WM_NCXBUTTONUP);
        _CASE_MSG(WM_NCXBUTTONDBLCLK);
        _CASE_MSG(WM_KEYDOWN);
        _CASE_MSG(WM_KEYUP);
        _CASE_MSG(WM_CHAR);
        _CASE_MSG(WM_DEADCHAR);
        _CASE_MSG(WM_SYSKEYDOWN);
        _CASE_MSG(WM_SYSKEYUP);
        _CASE_MSG(WM_SYSCHAR);
        _CASE_MSG(WM_SYSDEADCHAR);
        _CASE_MSG(WM_KEYLAST);
        _CASE_MSG(WM_IME_STARTCOMPOSITION);
        _CASE_MSG(WM_IME_ENDCOMPOSITION);
        _CASE_MSG(WM_IME_COMPOSITION);
        _CASE_MSG(WM_INITDIALOG);
        _CASE_MSG(WM_COMMAND);
        _CASE_MSG(WM_SYSCOMMAND);
        _CASE_MSG(WM_TIMER);
        _CASE_MSG(WM_HSCROLL);
        _CASE_MSG(WM_VSCROLL);
        _CASE_MSG(WM_INITMENU);
        _CASE_MSG(WM_INITMENUPOPUP);
        _CASE_MSG(WM_MENUSELECT);
        _CASE_MSG(WM_MENUCHAR);
        _CASE_MSG(WM_ENTERIDLE);
        _CASE_MSG(WM_MENURBUTTONUP);
        _CASE_MSG(WM_MENUDRAG);
        _CASE_MSG(WM_MENUGETOBJECT);
        _CASE_MSG(WM_UNINITMENUPOPUP);
        _CASE_MSG(WM_MENUCOMMAND);
        _CASE_MSG(WM_CHANGEUISTATE);
        _CASE_MSG(WM_UPDATEUISTATE);
        _CASE_MSG(WM_QUERYUISTATE);
        _CASE_MSG(WM_CTLCOLORMSGBOX);
        _CASE_MSG(WM_CTLCOLOREDIT);
        _CASE_MSG(WM_CTLCOLORLISTBOX);
        _CASE_MSG(WM_CTLCOLORBTN);
        _CASE_MSG(WM_CTLCOLORDLG);
        _CASE_MSG(WM_CTLCOLORSCROLLBAR);
        _CASE_MSG(WM_CTLCOLORSTATIC);
        _CASE_MSG(WM_MOUSEMOVE);
        _CASE_MSG(WM_LBUTTONDOWN);
        _CASE_MSG(WM_LBUTTONUP);
        _CASE_MSG(WM_LBUTTONDBLCLK);
        _CASE_MSG(WM_RBUTTONDOWN);
        _CASE_MSG(WM_RBUTTONUP);
        _CASE_MSG(WM_RBUTTONDBLCLK);
        _CASE_MSG(WM_MBUTTONDOWN);
        _CASE_MSG(WM_MBUTTONUP);
        _CASE_MSG(WM_MBUTTONDBLCLK);
        _CASE_MSG(WM_MOUSEWHEEL);
        _CASE_MSG(WM_XBUTTONDOWN);
        _CASE_MSG(WM_XBUTTONUP);
        _CASE_MSG(WM_XBUTTONDBLCLK);
        _CASE_MSG(WM_PARENTNOTIFY);
        _CASE_MSG(WM_ENTERMENULOOP);
        _CASE_MSG(WM_EXITMENULOOP);
        _CASE_MSG(WM_NEXTMENU);
        _CASE_MSG(WM_SIZING);
        _CASE_MSG(WM_CAPTURECHANGED);
        _CASE_MSG(WM_MOVING);
        _CASE_MSG(WM_POWERBROADCAST);
        _CASE_MSG(WM_DEVICECHANGE);
        _CASE_MSG(WM_MDICREATE);
        _CASE_MSG(WM_MDIDESTROY);
        _CASE_MSG(WM_MDIACTIVATE);
        _CASE_MSG(WM_MDIRESTORE);
        _CASE_MSG(WM_MDINEXT);
        _CASE_MSG(WM_MDIMAXIMIZE);
        _CASE_MSG(WM_MDITILE);
        _CASE_MSG(WM_MDICASCADE);
        _CASE_MSG(WM_MDIICONARRANGE);
        _CASE_MSG(WM_MDIGETACTIVE);
        _CASE_MSG(WM_MDISETMENU);
        _CASE_MSG(WM_ENTERSIZEMOVE);
        _CASE_MSG(WM_EXITSIZEMOVE);
        _CASE_MSG(WM_DROPFILES);
        _CASE_MSG(WM_MDIREFRESHMENU);
        _CASE_MSG(WM_IME_SETCONTEXT);
        _CASE_MSG(WM_IME_NOTIFY);
        _CASE_MSG(WM_IME_CONTROL);
        _CASE_MSG(WM_IME_COMPOSITIONFULL);
        _CASE_MSG(WM_IME_SELECT);
        _CASE_MSG(WM_IME_CHAR);
        _CASE_MSG(WM_IME_REQUEST);
        _CASE_MSG(WM_IME_KEYDOWN);
        _CASE_MSG(WM_IME_KEYUP);
        _CASE_MSG(WM_MOUSEHOVER);
        _CASE_MSG(WM_MOUSELEAVE);
        _CASE_MSG(WM_NCMOUSEHOVER);
        _CASE_MSG(WM_NCMOUSELEAVE);
        _CASE_MSG(WM_CUT);
        _CASE_MSG(WM_COPY);
        _CASE_MSG(WM_PASTE);
        _CASE_MSG(WM_CLEAR);
        _CASE_MSG(WM_UNDO);
        _CASE_MSG(WM_RENDERFORMAT);
        _CASE_MSG(WM_RENDERALLFORMATS);
        _CASE_MSG(WM_DESTROYCLIPBOARD);
        _CASE_MSG(WM_DRAWCLIPBOARD);
        _CASE_MSG(WM_PAINTCLIPBOARD);
        _CASE_MSG(WM_VSCROLLCLIPBOARD);
        _CASE_MSG(WM_SIZECLIPBOARD);
        _CASE_MSG(WM_ASKCBFORMATNAME);
        _CASE_MSG(WM_CHANGECBCHAIN);
        _CASE_MSG(WM_HSCROLLCLIPBOARD);
        _CASE_MSG(WM_QUERYNEWPALETTE);
        _CASE_MSG(WM_PALETTEISCHANGING);
        _CASE_MSG(WM_PALETTECHANGED);
        _CASE_MSG(WM_HOTKEY);
        _CASE_MSG(WM_PRINT);
        _CASE_MSG(WM_PRINTCLIENT);
        _CASE_MSG(WM_APPCOMMAND);
        _CASE_MSG(WM_HANDHELDFIRST);
        _CASE_MSG(WM_HANDHELDLAST);
        _CASE_MSG(WM_AFXFIRST);
        _CASE_MSG(WM_AFXLAST);
        _CASE_MSG(WM_PENWINFIRST);
        _CASE_MSG(WM_PENWINLAST);
        _CASE_MSG(WM_APP);
#undef _CASE_MSG
    }
    return "UNKNOWN";
}

void win32_print_event(mume_logger_t *logger,
                       HWND hWnd, UINT message,
                       WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        mume_trace1(
            logger,
            ("%x:%s: vk(%d), repeat(%d), scancode(%d)\n",
             hWnd, _win32_message_to_string(message),
             wParam, LOWORD(lParam), HIWORD(lParam) & 0xFF));
        break;
    default:
        mume_trace0(
            logger,
            ("0x%x:%s: 0x%x, 0x%x\n",
             hWnd, _win32_message_to_string(message),
             wParam, lParam));
    }
}
