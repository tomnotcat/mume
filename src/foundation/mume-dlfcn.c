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
#include "mume-dlfcn.h"
#include "mume-config.h"

#if HAVE_DLFCN_H
#include MUME_DLFCN_H

mume_dlhdl_t* mume_dlopen(const char *file)
{
    return dlopen(file, RTLD_LAZY);
}

void* mume_dlsym(mume_dlhdl_t *hdl, const char *sym)
{
    return dlsym(hdl, sym);
}

const char* mume_dlerror(void)
{
    return dlerror();
}

void mume_dlclose(mume_dlhdl_t *hdl)
{
    dlclose(hdl);
}

#elif HAVE_WINDOWS_H
#include MUME_WINDOWS_H

mume_dlhdl_t* mume_dlopen(const char *file)
{
    return LoadLibrary(file);
}

void* mume_dlsym(mume_dlhdl_t *hdl, const char *sym)
{
    return GetProcAddress(hdl, sym);
}

const char* mume_dlerror(void)
{
    static char buf[256];
    DWORD lasterr = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  0, lasterr, 0, buf,
                  sizeof(buf) / sizeof(buf[0]), 0);
    return buf;
}

void mume_dlclose(mume_dlhdl_t *hdl)
{
    FreeLibrary(hdl);
}

#else /* !HAVE_DLFCN_H && !HAVE_WINDOWS_H */
# error unknown dlfunc
#endif
