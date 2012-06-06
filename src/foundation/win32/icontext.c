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
#include "icontext.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "target.h"
#include MUME_ASSERT_H

#define WIN32_IMC_PARAMS(_imc) \
    win32_icontext_t *wimc = (win32_icontext_t*)(_imc); \
    win32_target_t *wtgt = (win32_target_t*)wimc->base.tgt

static win32_immfuncs_t* win32_immfuncs_reference(int add)
{
	static HMODULE _immdll;
	static win32_immfuncs_t _funcs;
	static int _refcount;
	if (add) {
		if (++_refcount == 1) {
			_immdll = LoadLibraryW(L"imm32.dll");
			if (_immdll) {
				_funcs.get_context = (win32_imm_get_context*)(
					GetProcAddress(_immdll, "ImmGetContext"));
				assert(_funcs.get_context);
				_funcs.release_context = (win32_imm_release_context*)(
					GetProcAddress(_immdll, "ImmReleaseContext"));
				assert(_funcs.release_context);
				_funcs.get_open_status = (win2_imm_get_open_status*)(
					GetProcAddress(_immdll, "ImmGetOpenStatus"));
				assert(_funcs.get_open_status);
				_funcs.notify_ime = (win32_imm_notify_ime*)(
					GetProcAddress(_immdll, "ImmNotifyIME"));
				assert(_funcs.notify_ime);
				_funcs.set_composition_window = (win32_imm_set_composition_window*)(
					GetProcAddress(_immdll, "ImmSetCompositionWindow"));
				assert(_funcs.set_composition_window);
				_funcs.set_candidate_window = (win32_imm_set_candidate_window*)(
					GetProcAddress(_immdll, "ImmSetCandidateWindow"));
				assert(_funcs.set_candidate_window);
			}
			else {
				_funcs.get_context = NULL;
				_funcs.release_context = NULL;
				_funcs.get_open_status = NULL;
				_funcs.notify_ime = NULL;
				_funcs.set_composition_window = NULL;
				_funcs.set_candidate_window = NULL;
				return NULL;
			}
		}
	}
	else if (--_refcount == 0) {
		if (_immdll)
			FreeLibrary(_immdll);
	}
	return &_funcs;
}

static void win32_imc_destroy(mume_icontext_t *imc)
{
    WIN32_IMC_PARAMS(imc);
	win32_immfuncs_reference(0);
    free(wimc);
}

static void win32_imc_reset(mume_icontext_t *imc)
{
    WIN32_IMC_PARAMS(imc);
    HIMC himc = NULL;
	if (!wimc->func)
		return;
	himc = wimc->func->get_context(wtgt->hwnd);
    if (NULL == himc)
        return;
    if (wimc->func->get_open_status(himc))
        wimc->func->notify_ime(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    wimc->func->release_context(wtgt->hwnd, himc);
}

static void win32_imc_set_focus(mume_icontext_t *imc)
{
}

static void win32_imc_set_location(
    mume_icontext_t *imc, int posx, int posy,
    int excx, int excy, int excw, int exch)
{
    WIN32_IMC_PARAMS(imc);
    COMPOSITIONFORM cf;
    CANDIDATEFORM candf;
    HIMC himc = NULL;
	if (!wimc->func)
		return;
	himc = wimc->func->get_context(wtgt->hwnd);
    if (!himc)
        return;
    cf.dwStyle = CFS_POINT;
    cf.ptCurrentPos.x = posx;
    cf.ptCurrentPos.y = posy;
    wimc->func->set_composition_window(himc, &cf);

    candf.dwIndex = 0;
    candf.dwStyle = CFS_EXCLUDE;
    candf.ptCurrentPos.x = posx;
    candf.ptCurrentPos.y = posy;
    candf.rcArea.left = excx;
    candf.rcArea.top = excy;
    candf.rcArea.right = excx + excw;
    candf.rcArea.bottom = excy + exch;
    wimc->func->set_candidate_window(himc, &candf);
    wimc->func->release_context(wtgt->hwnd, himc);
}

mume_icontext_t* win32_create_imc(mume_target_t *tgt)
{
    static struct mume_icontext_i win32_imc_impl = {
        &win32_imc_destroy,
        &win32_imc_reset,
        &win32_imc_set_focus,
        &win32_imc_set_location
    };
    win32_target_t *wtgt = (win32_target_t*)tgt;
    win32_icontext_t *wimc = malloc_abort(sizeof(win32_icontext_t));
    wimc->base.impl = &win32_imc_impl;
    wimc->base.tgt = tgt;
	wimc->func = win32_immfuncs_reference(1);
    return (mume_icontext_t*)wimc;
}
