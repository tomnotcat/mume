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
#include "backend.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "msgproc.h"
#include "target.h"
#include "thunk.h"

#define WIN32_BACKEND_PARAMS(_back) \
    win32_backend_t *wback = (win32_backend_t*)(_back)

static void win32_destroy_backend(void *back)
{
    WIN32_BACKEND_PARAMS(back);
    if (wback->root)
        mume_target_destroy(wback->root);
    free(wback);
}

static void win32_screen_size(void *back, int *width, int *height)
{
    if (width)
        *width = GetSystemMetrics(SM_CXSCREEN);
    if (height)
        *height = GetSystemMetrics(SM_CYSCREEN);
}

static int win32_handle_event(void *back, int wait)
{
    WIN32_BACKEND_PARAMS(back);
    int result;
    MSG msg;
    DWORD dwms = (DWORD)wait;
    if (MUME_WAIT_INFINITE == wait)
        dwms = INFINITE;
    MsgWaitForMultipleObjects(0, NULL, FALSE, dwms, QS_ALLINPUT);
    result = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    if (result) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return result;
}

static int win32_wakeup_event(void *back)
{
}

static int win32_query_pointer(
    void *back, int *x, int *y, int *state)
{
    return 0;
}

mume_backend_t* win32_create_backend(
    mume_frontend_t *front, mume_logger_t *logger)
{
    static int initialized = 0;
    static struct mume_backend_i win32_backend_impl = {
        MUME_BACKEND_CURRENT_VER,
        MUME_BACKEND_ID_WIN32,
        _win32_destroy_backend,
        _win32_screen_size,
        win32_root_target,
        win32_create_target,
        _win32_handle_event,
        _win32_wakeup_event,
        _win32_query_pointer,
    };
    win32_backend_t *wback;
    if (!initialized) {
        win32_initialize();
        initialized = 1;
    }
    wback = malloc_abort(sizeof(win32_backend_t));
    wback->base.impl = &win32_backend_impl;
    wback->logger = logger;
    wback->front = front;
    wback->root = NULL;
    return (mume_backend_t*)wback;
}

mume_backend_t* mume_create_backend(
    mume_frontend_t *front, mume_logger_t *logger, ...)
{
    return win32_create_backend(front, logger);
}
