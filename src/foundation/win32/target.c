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
#include "target.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "cairo/cairo-win32.h"
#include "backend.h"
#include "icontext.h"
#include "msgproc.h"
#include "thunk.h"

#define WIN32_TARGET_PARAMS(_tgt) \
    win32_target_t *wtgt = (win32_target_t*)_tgt; \
    win32_backend_t *wback = (win32_backend_t*)wtgt->base.back

static void win32_target_destroy(void *tgt)
{
    WIN32_TARGET_PARAMS(tgt);
    if (wback->root != tgt) {
        DestroyWindow(wtgt->hwnd);
        win32_destroy_thunk(wtgt->thunk);
    }
    free(wtgt);
}

static int win32_target_set_geometry(
    void *tgt, int x, int y, int width, int height)
{
    WIN32_TARGET_PARAMS(tgt);
    /* SetWindowPos(wtgt->hwnd, NULL, x, y, width, height,
        SWP_NOZORDER | SWP_NOACTIVATE); */
    return MoveWindow(wtgt->hwnd, x, y, width, height, TRUE);
}

static int win32_target_get_geometry(
    void *tgt, int *x, int *y, int *width, int *height)
{
    WIN32_TARGET_PARAMS(tgt);
    RECT winrect;
    if (GetWindowRect(wtgt->hwnd, &winrect)) {
        if (x)
           *x = winrect.left;
        if (y)
           *y = winrect.top;
        if (width)
           *width = winrect.right - winrect.left;
        if (height)
           *height = winrect.bottom - winrect.top;
        return 1;
    }
    return 0;
}

static cairo_t* win32_target_begin_paint(void *tgt)
{
    WIN32_TARGET_PARAMS(tgt);
    HDC hdc = GetDC(wtgt->hwnd);
    cairo_surface_t *sfc = cairo_win32_surface_create(hdc);
    cairo_t *cro;
    mume_exit_if(NULL == sfc, wback->logger,
                 ("cairo_win32_surface_create failed\n"));
    cro = cairo_create(sfc);
    /* cairo_t have reference to it now, destroy it */
    cairo_surface_destroy(sfc);
    return cro;
}

static void win32_target_end_paint(void *tgt, cairo_t *cro)
{
    WIN32_TARGET_PARAMS(tgt);
    cairo_surface_t *sfc = cairo_get_target(cro);
    HDC hdc = cairo_win32_surface_get_dc(sfc);
    ReleaseDC(wtgt->hwnd, hdc);
    cairo_destroy(cro);
}

mume_target_t* win32_root_target(void *back)
{
    static struct mume_target_i win32_rootgt_impl = {
        &win32_target_destroy,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &win32_target_get_geometry,
        NULL,
        NULL,
        NULL
    };
    win32_backend_t *wback = (win32_backend_t*)back;
    if (NULL == wback->root) {
        win32_target_t *wtgt;
        wtgt = malloc_abort(sizeof(win32_target_t));
        wtgt->base.impl = &win32_rootgt_impl;
        wtgt->base.back = back;
        wtgt->base.param = NULL;
        wtgt->thunk = NULL;
        wtgt->hwnd = GetDesktopWindow();
        wback->root = (mume_target_t*)wtgt;
    }
    return wback->root;
}

mume_target_t* win32_create_target(
    void *back, mume_target_t *pnt,
    int x, int y, int width, int height)
{
    static struct mume_target_i win32_target_impl = {
        &win32_target_destroy,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &win32_target_set_geometry,
        &win32_target_get_geometry,
        NULL,
        NULL,
        &win32_target_begin_paint,
        &win32_target_end_paint,
        &win32_create_imc
    };
    win32_backend_t *wback = (win32_backend_t*)back;
    win32_target_t *wtgt;
    const char *wcls_name = WIN32_WINCLS_NAME;
    wtgt = malloc_abort(sizeof(win32_target_t));
    wtgt->base.impl = &win32_target_impl;
    wtgt->base.back = back;
    wtgt->base.param = NULL;
    /* wtgt->thunk and wtgt->hwnd will be set in winimpl */
    wtgt->thunk = NULL;
    wtgt->hwnd = NULL;
    /* create window */
    win32_createwin_begin(wtgt);
    CreateWindowEx(WS_EX_CONTROLPARENT | WS_EX_APPWINDOW,
                  wcls_name, "mume",
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  x, y, width, height,
                  ((win32_target_t*)pnt)->hwnd,
                  NULL, NULL, NULL);
    win32_createwin_end(wtgt);
    mume_exit_if(NULL == wtgt->hwnd, wback->logger,
                 ("create window failed\n"));
    return (mume_target_t*)wtgt;
}
