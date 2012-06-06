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
#include "thunk.h"
#include "mume-debug.h"

typedef unsigned int dword;

#if defined(_M_IX86)
# pragma pack(push, 1)

struct win32_thunk_s {
    dword mov;                          /* mov dword ptr [esp+0x4] */
    dword ptr;
    char jmp;                           /* jmp win_proc */
    dword relproc;                      /* relative jmp */
};

win32_thunk_t* win32_create_thunk(uintptr_t proc, uintptr_t arg)
{
    win32_thunk_t *thk;
    thk = (win32_thunk_t*)HeapAlloc(GetProcessHeap(),
                                    HEAP_GENERATE_EXCEPTIONS,
                                    sizeof(win32_thunk_t));
    mume_exit_if(NULL == thk, NULL, ("alloc thunk failed\n"));
    /* C7 44 24 0C */
    thk->mov = 0x042444C7;
    thk->ptr = (dword)arg;
    thk->jmp = 0xe9;
    thk->relproc = (dword)(proc - ((uintptr_t)thk +
                                   sizeof(win32_thunk_t)));
    /* write block from data cache and
       flush from instruction cache */
    FlushInstructionCache(GetCurrentProcess(),
                          thk, sizeof(win32_thunk_t));
    return thk;
}

void* win32_thunk_code(win32_thunk_t *thk)
{
    return thk;
}

void win32_destroy_thunk(win32_thunk_t *thk)
{
    HeapFree(GetProcessHeap(), 0, thk);
}

# pragma pack(pop)
#else  /* not _M_IX86 */
# error only X86 supported
#endif  /* not _M_IX86 */
