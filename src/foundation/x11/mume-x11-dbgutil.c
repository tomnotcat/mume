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
#include "mume-x11-dbgutil.h"
#include "mume-debug.h"
#include "mume-string.h"

static const char* _x11_get_state_name(int state)
{
    static char buffer[256];
    int values[] = {
        Button1Mask,
        Button2Mask,
        Button3Mask,
        Button4Mask,
        Button5Mask,
        ShiftMask,
        LockMask,
        ControlMask,
        Mod1Mask,
        Mod2Mask,
        Mod3Mask,
        Mod4Mask,
        Mod5Mask
    };

    const char *strings[] = {
        "Button1",
        "Button2",
        "Button3",
        "Button4",
        "Button5",
        "Shift",
        "Lock",
        "Control",
        "Mod1",
        "Mod2",
        "Mod3",
        "Mod4",
        "Mod5"
    };

    return mume_mask_to_string(
        buffer, sizeof(buffer) / sizeof(buffer[0]),
        state, ",", values, strings,
        sizeof(values) / sizeof(values[0]));
}

static const char* _x11_get_button_name(int button)
{
    switch (button) {
    case Button1:
        return "Button1";

    case Button2:
        return "Button2";

    case Button3:
        return "Button3";

    case Button4:
        return "Button4";

    case Button5:
        return "Button5";
    }

    return "Unknown";
}

static const char* _x11_get_cross_mode(int mode)
{
    switch (mode) {
    case NotifyNormal:
        return "Normal";

    case NotifyGrab:
        return "Grab";

    case NotifyUngrab:
        return "Ungrab";
    }

    return "Unknown";
}

static const char* _x11_get_cross_detail(int detail)
{
    switch (detail) {
    case NotifyAncestor:
        return "Ancestor";

    case NotifyVirtual:
        return "Virtual";

    case NotifyInferior:
        return "Inferior";

    case NotifyNonlinear:
        return "Nonlinear";

    case NotifyNonlinearVirtual:
        return "NonlinearVirtual";
    }

    return "Unknown";
}

static const char* _x11_get_focus_mode(int mode)
{
    switch (mode) {
    case NotifyNormal:
        return "Normal";

    case NotifyWhileGrabbed:
        return "WhileGrabbed";

    case NotifyGrab:
        return "Grab";

    case NotifyUngrab:
        return "Ungrab";
    }

    return "Unknown";
}

static const char* _x11_get_focus_detail(int detail)
{
    switch (detail) {
    case NotifyAncestor:
        return "Ancestor";

    case NotifyVirtual:
        return "Virtual";

    case NotifyInferior:
        return "Inferior";

    case NotifyNonlinear:
        return "Nonlinear";

    case NotifyNonlinearVirtual:
        return "NonlinearVirtual";

    case NotifyPointer:
        return "Pointer";

    case NotifyPointerRoot:
        return "PointerRoot";

    case NotifyDetailNone:
        return "DetailNone";
    }

    return "Unknown";
}

void mume_x11_print_event(XEvent *xevent)
{
    static char *event_names[] = {
        "",
        "",
        "KeyPress",
        "KeyRelease",
        "ButtonPress",
        "ButtonRelease",
        "MotionNotify",
        "EnterNotify",
        "LeaveNotify",
        "FocusIn",
        "FocusOut",
        "KeymapNotify",
        "Expose",
        "GraphicsExpose",
        "NoExpose",
        "VisibilityNotify",
        "CreateNotify",
        "DestroyNotify",
        "UnmapNotify",
        "MapNotify",
        "MapRequest",
        "ReparentNotify",
        "ConfigureNotify",
        "ConfigureRequest",
        "GravityNotify",
        "ResizeRequest",
        "CirculateNotify",
        "CirculateRequest",
        "PropertyNotify",
        "SelectionClear",
        "SelectionRequest",
        "SelectionNotify",
        "ColormapNotify",
        "ClientMessage",
        "MappingNotify"
    };

    switch (xevent->type) {
    case KeyPress:
    case KeyRelease:
        {
            char buffer[256];
            KeySym keysym;
            XComposeStatus compose;
            int count;
            count = XLookupString(
                &xevent->xkey, buffer,
                sizeof(buffer) / sizeof(buffer[0]),
                &keysym, &compose);
            buffer[count] = '\0';
            mume_trace2(
                ("%s(%x-%x-%x): [%s] sym(%d), time(%d)"
                 ", xy(%d, %d), rxy(%d, %d)"
                 ", state(%s), keycode(%d), samesc(%d)"
                 ", serial(%d), send(%d)\n",
                 event_names[xevent->type],
                 xevent->xkey.root,
                 xevent->xkey.window,
                 xevent->xkey.subwindow,
                 buffer, keysym,
                 xevent->xkey.time,
                 xevent->xkey.x, xevent->xkey.y,
                 xevent->xkey.x_root, xevent->xkey.y_root,
                 _x11_get_state_name(xevent->xkey.state),
                 xevent->xkey.keycode,
                 xevent->xkey.same_screen,
                 xevent->xkey.serial,
                 xevent->xkey.send_event));
        }
        break;

    case ButtonPress:
    case ButtonRelease:
        mume_trace2(
            ("%s(%x-%x-%x): time(%d), xy(%d, %d), rxy(%d, %d)"
             ", state(%s), button(%s), samesc(%d)"
             ", serial(%d), send(%d)\n",
             event_names[xevent->type],
             xevent->xbutton.root,
             xevent->xbutton.window,
             xevent->xbutton.subwindow,
             xevent->xbutton.time,
             xevent->xbutton.x, xevent->xbutton.y,
             xevent->xbutton.x_root, xevent->xbutton.y_root,
             _x11_get_state_name(xevent->xbutton.state),
             _x11_get_button_name(xevent->xbutton.button),
             xevent->xbutton.same_screen,
             xevent->xbutton.serial,
             xevent->xbutton.send_event));
        break;

    case MotionNotify:
        mume_trace3(
            ("%s(%x-%x-%x): time(%d), xy(%d, %d), rxy(%d, %d)"
             ", state(%s), hint(%d), samesc(%d)"
             ", serial(%d), send(%d)\n",
             event_names[xevent->type],
             xevent->xmotion.root,
             xevent->xmotion.window,
             xevent->xmotion.subwindow,
             xevent->xmotion.time,
             xevent->xmotion.x, xevent->xmotion.y,
             xevent->xmotion.x_root, xevent->xmotion.y_root,
             _x11_get_state_name(xevent->xmotion.state),
             xevent->xmotion.is_hint,
             xevent->xmotion.same_screen,
             xevent->xmotion.serial,
             xevent->xmotion.send_event));
        break;

    case EnterNotify:
    case LeaveNotify:
        mume_trace1(
            ("%s(%x-%x-%x): time(%d), xy(%d, %d), rxy(%d, %d)"
             ", mode(%s), detail(%s), samesc(%d)"
             ", focus(%d), state(%s), serial(%d)\n",
             event_names[xevent->type],
             xevent->xcrossing.root,
             xevent->xcrossing.window,
             xevent->xcrossing.subwindow,
             xevent->xcrossing.time,
             xevent->xcrossing.x, xevent->xcrossing.y,
             xevent->xcrossing.x_root, xevent->xcrossing.y_root,
             _x11_get_cross_mode(xevent->xcrossing.mode),
             _x11_get_cross_detail(xevent->xcrossing.detail),
             xevent->xcrossing.same_screen,
             xevent->xcrossing.focus,
             _x11_get_state_name(xevent->xcrossing.state),
             xevent->xcrossing.serial));
        break;

    case FocusIn:
    case FocusOut:
        mume_trace1(
            ("%s(%x): mode(%s), detail(%s)"
             ", serial(%d), send(%d)\n",
             event_names[xevent->type],
             xevent->xfocus.window,
             _x11_get_focus_mode(xevent->xfocus.mode),
             _x11_get_focus_detail(xevent->xfocus.detail),
             xevent->xmotion.serial,
             xevent->xmotion.send_event));
        break;

    case Expose:
        mume_trace2(
            ("%s(%x): %d, %d, %d, %d, count(%d)"
             ", serial(%d), send(%d)\n",
             event_names[xevent->type],
             xevent->xexpose.window,
             xevent->xexpose.x,
             xevent->xexpose.y,
             xevent->xexpose.width,
             xevent->xexpose.height,
             xevent->xexpose.count,
             xevent->xexpose.serial,
             xevent->xexpose.send_event));
        break;

    case GraphicsExpose:
        mume_trace1(
            ("%s(%x): %d, %d, %d, %d, count(%d)"
             ", major_code(%d), minor_code(%d)\n",
             event_names[xevent->type],
             xevent->xgraphicsexpose.drawable,
             xevent->xgraphicsexpose.x,
             xevent->xgraphicsexpose.y,
             xevent->xgraphicsexpose.width,
             xevent->xgraphicsexpose.height,
             xevent->xgraphicsexpose.count,
             xevent->xgraphicsexpose.major_code,
             xevent->xgraphicsexpose.minor_code));
        break;

    case NoExpose:
        break;

    default:
        mume_trace0(
            ("%s(%x): serial(%d), send(%d)\n",
             event_names[xevent->type],
             xevent->xany.window,
             xevent->xany.serial,
             xevent->xany.send_event));
        break;
    }
}
