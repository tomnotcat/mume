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
#ifndef CATCH_GUI_BACKENDS_WIN32_ICONTEXT_H
#define CATCH_GUI_BACKENDS_WIN32_ICONTEXT_H

#include "common.h"
#include "mume-icontext.h"

CATCH_BEGIN_DECLS

typedef HIMC WINAPI win32_imm_get_context(HWND);
typedef BOOL WINAPI win32_imm_release_context(HWND, HIMC);
typedef BOOL WINAPI win2_imm_get_open_status(HIMC);
typedef BOOL WINAPI win32_imm_notify_ime(HIMC, DWORD, DWORD, DWORD);
typedef BOOL WINAPI win32_imm_set_composition_window(HIMC, LPCOMPOSITIONFORM);
typedef BOOL WINAPI win32_imm_set_candidate_window(HIMC, LPCANDIDATEFORM);

typedef struct win32_immfuncs_s {
	win32_imm_get_context *get_context;
	win32_imm_release_context *release_context;
	win2_imm_get_open_status *get_open_status;
	win32_imm_notify_ime *notify_ime;
	win32_imm_set_composition_window *set_composition_window;
	win32_imm_set_candidate_window *set_candidate_window;
} win32_immfuncs_t;

struct win32_icontext_s {
    mume_icontext_t base;
	win32_immfuncs_t *func;
};

mume_icontext_t* win32_create_imc(mume_target_t *tgt);

CATCH_END_DECLS

#endif  /* CATCH_GUI_BACKENDS_WIN32_ICONTEXT_H */
