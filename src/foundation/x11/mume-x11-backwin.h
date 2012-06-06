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
#ifndef MUME_X11_BACKWIN_H
#define MUME_X11_BACKWIN_H

#include "mume-backwin.h"
#include "mume-x11-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_X11_BACKWIN (MUME_SIZEOF_BACKWIN + \
                                 sizeof(void*) * 2 +   \
                                 sizeof(int) * 2)

#define MUME_SIZEOF_X11_BACKWIN_CLASS (MUME_SIZEOF_BACKWIN_CLASS + \
                                       sizeof(voidf*) * 19)

mux11_public const void* mume_x11_backwin_class(void);

mux11_public const void* mume_x11_backwin_meta_class(void);

mux11_public void* mume_x11_backwin_new(
    void *backend, Window window, int managed);

mux11_public void* mume_x11_backwin_get_backend(const void *self);

mux11_public Window mume_x11_backwin_get_xwindow(const void *self);

mux11_public void mume_x11_set_sizehint(
    Display *display, Window window, int min_width, int min_height,
    int fit_width, int fit_height, int max_width, int max_height);

mux11_public Window mume_x11_create_window(
    Display *display, int type, Window parent,
    int x, int y, unsigned int width, unsigned int height,
    unsigned int clazz, int eventmask);

mux11_public void _mume_x11_backwin_handle_event(
    const void *clazz, void *self, XEvent *xevent);

#define mume_x11_backwin_handle_event(_self, _xevent) \
    _mume_x11_backwin_handle_event(NULL, _self, _xevent)

mux11_public void _mume_x11_backwin_handle_key_press(
    const void *clazz, void *self, XKeyEvent *xkey);

#define mume_x11_backwin_handle_key_press(_self, _xkey) \
    _mume_x11_backwin_handle_key_press(NULL, _self, _xkey)

mux11_public void _mume_x11_backwin_handle_key_release(
    const void *clazz, void *self, XKeyEvent *xkey);

#define mume_x11_backwin_handle_key_release(_self, _xkey) \
    _mume_x11_backwin_handle_key_release(NULL, _self, _xkey)

mux11_public void _mume_x11_backwin_handle_button_press(
    const void *clazz, void *self, XButtonEvent *xbutton);

#define mume_x11_backwin_handle_button_press(_self, _xbutton) \
    _mume_x11_backwin_handle_button_press(NULL, _self, _xbutton)

mux11_public void _mume_x11_backwin_handle_button_release(
    const void *clazz, void *self, XButtonEvent *xbutton);

#define mume_x11_backwin_handle_button_release(_self, _xbutton) \
    _mume_x11_backwin_handle_button_release(NULL, _self, _xbutton)

mux11_public void _mume_x11_backwin_handle_motion_notify(
    const void *clazz, void *self, XMotionEvent *xmotion);

#define mume_x11_backwin_handle_motion_notify(_self, _xmotion) \
    _mume_x11_backwin_handle_motion_notify(NULL, _self, _xmotion)

mux11_public void _mume_x11_backwin_handle_enter_notify(
    const void *clazz, void *self, XCrossingEvent *xcrossing);

#define mume_x11_backwin_handle_enter_notify(_self, _xcrossing) \
    _mume_x11_backwin_handle_enter_notify(NULL, _self, _xcrossing)

mux11_public void _mume_x11_backwin_handle_leave_notify(
    const void *clazz, void *self, XCrossingEvent *xcrossing);

#define mume_x11_backwin_handle_leave_notify(_self, _xcrossing) \
    _mume_x11_backwin_handle_leave_notify(NULL, _self, _xcrossing)

mux11_public void _mume_x11_backwin_handle_focus_in(
    const void *clazz, void *self, XFocusChangeEvent *xfocus);

#define mume_x11_backwin_handle_focus_in(_self, _xfocus) \
    _mume_x11_backwin_handle_focus_in(NULL, _self, _xfocus)

mux11_public void _mume_x11_backwin_handle_focus_out(
    const void *clazz, void *self, XFocusChangeEvent *xfocus);

#define mume_x11_backwin_handle_focus_out(_self, _xfocus) \
    _mume_x11_backwin_handle_focus_out(NULL, _self, _xfocus)

mux11_public void _mume_x11_backwin_handle_reparent(
    const void *clazz, void *self, XReparentEvent *xreparent);

#define mume_x11_backwin_handle_reparent(_self, _xreparent) \
    _mume_x11_backwin_handle_reparent(NULL, _self, _xreparent)

mux11_public void _mume_x11_backwin_handle_expose(
    const void *clazz, void *self, XExposeEvent *xexpose);

#define mume_x11_backwin_handle_expose(_self, _xexpose) \
    _mume_x11_backwin_handle_expose(NULL, _self, _xexpose)

mux11_public void _mume_x11_backwin_handle_map(
    const void *clazz, void *self, XMapEvent *xmap);

#define mume_x11_backwin_handle_map(_self, _xmap) \
    _mume_x11_backwin_handle_map(NULL, _self, _xmap)

mux11_public void _mume_x11_backwin_handle_unmap(
    const void *clazz, void *self, XUnmapEvent *xunmap);

#define mume_x11_backwin_handle_unmap(_self, _xunmap) \
    _mume_x11_backwin_handle_unmap(NULL, _self, _xunmap)

mux11_public void _mume_x11_backwin_handle_configure(
    const void *clazz, void *self, XConfigureEvent *xconfigure);

#define mume_x11_backwin_handle_configure(_self, _xconfigure) \
    _mume_x11_backwin_handle_configure(NULL, _self, _xconfigure)

mux11_public void _mume_x11_backwin_handle_client(
    const void *clazz, void *self, XClientMessageEvent *xclient);

#define mume_x11_backwin_handle_client(_self, _xclient) \
    _mume_x11_backwin_handle_client(NULL, _self, _xclient)

mux11_public void _mume_x11_backwin_handle_selection_request(
    const void *clazz, void *self, XSelectionRequestEvent *xrequest);

#define mume_x11_backwin_handle_selection_request(_self, _xrequest) \
    _mume_x11_backwin_handle_selection_request(NULL, _self, _xrequest)

mux11_public void _mume_x11_backwin_handle_selection_notify(
    const void *clazz, void *self, XSelectionEvent *xselection);

#define mume_x11_backwin_handle_selection_notify(_self, _xselection) \
    _mume_x11_backwin_handle_selection_notify(NULL, _self, _xselection)

mux11_public void _mume_x11_backwin_handle_selection_clear(
    const void *clazz, void *self, XSelectionClearEvent *xclear);

#define mume_x11_backwin_handle_selection_clear(_self, _xclear) \
    _mume_x11_backwin_handle_selection_clear(NULL, _self, _xclear)

MUME_END_DECLS

#endif  /* MUME_X11_BACKWIN_H */
