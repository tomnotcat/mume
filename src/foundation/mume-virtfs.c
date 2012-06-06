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
#include "mume-virtfs.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-stream.h"
#include "mume-string.h"
#include MUME_ASSERT_H
#include MUME_LIMITS_H
#include MUME_STDIO_H
#include MUME_STDLIB_H
#include MUME_PHYSFS_H

#define _MOUNT_DIR_LEN 16
#define _MAX_PATH_LEN 256

struct mume_virtfs_s {
    const char *mount;
    int refcount;
};

typedef struct _virtfs_stream_s {
    mume_stream_t base;
    mume_virtfs_t *vfs;
    PHYSFS_File *file;
} _virtfs_stream_t;

static int _dirsgot;
static char _basedir[_MAX_PATH_LEN];
static char _userdir[_MAX_PATH_LEN];
static char _dirsep;
static mume_virtfs_t *_writefs;

static int _physfs_reference(int add)
{
    /* FIXME: need multi thread mutex in this place */
    static int physfs_refcount;
    if (add) {
        if (1 == ++physfs_refcount) {
            if (!PHYSFS_init("mume")) {
                mume_error(("init physfs failed: %s\n",
                            PHYSFS_getLastError()));
            }
        }
    }
    else if (0 == --physfs_refcount) {
        if (!PHYSFS_deinit()) {
            mume_error(("deinit physfs failed: %s\n",
                         PHYSFS_getLastError()));
        }
    }
    return physfs_refcount;
}

static void _physfs_getdirs(void)
{
    size_t cpc;
    if (_dirsgot)
        return;
    _physfs_reference(1);
    cpc = strcpy_c(
        _basedir, _MAX_PATH_LEN, PHYSFS_getBaseDir());
    if (cpc > 0 && '/' == _basedir[cpc - 1])
        _basedir[cpc - 1] = '\0';
    cpc = strcpy_c(
        _userdir, _MAX_PATH_LEN, PHYSFS_getUserDir());
    if (cpc > 0 && '/' == _userdir[cpc - 1])
        _userdir[cpc - 1] = '\0';
    _dirsep = PHYSFS_getDirSeparator()[0];
    _physfs_reference(0);
    _dirsgot = 1;
}

static int _change_writedir(mume_virtfs_t *vfs)
{
    const char *path = EXTRA_OF(mume_virtfs_t, vfs);
    if (_writefs == vfs)
        return 1;
    if (PHYSFS_setWriteDir(path)) {
        _writefs = vfs;
        return 1;
    }
    mume_warning(("PHYSFS_setWriteDir failed: %s\n",
                  PHYSFS_getLastError()));
    return 0;
}

static inline size_t _sint64_to_uint32(PHYSFS_sint64 n)
{
    if (n < 0 || n > UINT_MAX) {
        mume_error(("can't convert sint64 to uint32: %d\n", n));
        return 0;
    }
    return n;
}

static const char* _convert_to_abspath(
    char *buf, size_t len, const char* path)
{
#define _ERR_OUTOF_BUFFER() \
    mume_error(("out of path buffer: %u: %s\n", len, path))
#define _ERR_INVALID_PATH() \
    mume_error(("invalid path: %s\n", path))
#define _IS_DIRSEP_CHAR(_C) ('/' == (_C) || '\\' == (_C))
    char sep = mume_virtfs_dirsep();
    char *beg = buf;
    char *end = buf + len;
    char *res = buf;
    const char *dir = path;
    const char *it;
#if BUILD_UNIX
    if ('/' != *dir) {
        if (!strcpy_s(buf, len, mume_virtfs_basedir())) {
            _ERR_OUTOF_BUFFER();
            return NULL;
        }
        res = buf + strlen(buf);
    }
#elif BUILD_WIN32
    if (':' == dir[1] && '\\' == dir[2]) {
        if (!strcpy_s(buf, 3, dir)) {
            _ERR_OUTOF_BUFFER();
            return NULL;
        }
        dir += 3;
        beg = buf + 2;
        res = buf + 3;
    }
    else {
        if (!strcpy_s(buf, len, mume_virtfs_basedir())) {
            _ERR_OUTOF_BUFFER();
            return NULL;
        }
        beg = buf + 2;
        res = buf + strlen(buf);
    }
#else
# error UNKNOWN SYSTEM
#endif
    if (res != buf && res[-1] != sep) {
        *res++ = sep;
    }
    it = dir;
    while (*it) {
        if (*it == '.' && dir == it) {
            if (_IS_DIRSEP_CHAR(it[1])) {
                /* './': current dir */
                it += 2;
                dir = it;
            }
            else if (it[1] == '.' && _IS_DIRSEP_CHAR(it[2])) {
                /* '../': parent dir */
                it += 3;
                dir = it;
                if (res != beg && res[-1] == sep)
                    --res;
                while (res != beg && res[-1] != sep)
                    --res;
            }
            else {
                /* invalid dir */
                return NULL;
            }
        }
        else if (res != end) {
            if (_IS_DIRSEP_CHAR(*it)) {
                dir = it + 1;
            }
            *res++ = *it;
            ++it;
        }
        else {
            _ERR_OUTOF_BUFFER();
            return NULL;
        }
    }

    if (res == end) {
        _ERR_OUTOF_BUFFER();
        return NULL;
    }
    *res = '\0';
    return buf;
#undef _IS_DIRSEP_CHAR
#undef _ERR_INVALID_PATH
#undef _ERR_OUTOF_BUFFER
}

static size_t _virtfs_stream_length(void *self)
{
    return _sint64_to_uint32(
        PHYSFS_fileLength(((_virtfs_stream_t*)self)->file));
}

static int _virtfs_stream_eof(void *self)
{
    return PHYSFS_eof(((_virtfs_stream_t*)self)->file);
}

static size_t _virtfs_stream_tell(void *self)
{
    return _sint64_to_uint32(
        PHYSFS_tell(((_virtfs_stream_t*)self)->file));
}

static int _virtfs_stream_seek(void *self, size_t pos)
{
    return PHYSFS_seek(((_virtfs_stream_t*)self)->file, pos);
}

static size_t _virtfs_stream_read(
    void *self, void *data, size_t len)
{
    return _sint64_to_uint32(PHYSFS_read(
        ((_virtfs_stream_t*)self)->file, data, 1, len));
}

static size_t _virtfs_stream_write(
    void *self, const void *data, size_t len)
{
    return _sint64_to_uint32(PHYSFS_write(
        ((_virtfs_stream_t*)self)->file, data, 1, len));
}

static void _virtfs_stream_close(void *self)
{
    _virtfs_stream_t *stm = self;
    PHYSFS_close(stm->file);
    mume_virtfs_destroy(stm->vfs);
    free(stm);
}

const char* mume_virtfs_basedir(void)
{
    if (!_dirsgot)
        _physfs_getdirs();
    return _basedir;
}

const char* mume_virtfs_userdir(void)
{
    if (!_dirsgot)
        _physfs_getdirs();
    return _userdir;
}

char mume_virtfs_dirsep(void)
{
    if (!_dirsgot)
        _physfs_getdirs();
    return _dirsep;
}

size_t mume_virtfs_dirlen(const char *name)
{
    char sep = mume_virtfs_dirsep();
    const char *it = name;
    const char *at = it;

    while (*it) {
        if ((*it) == sep)
            at = it;

        ++it;
    }

    if ((*at) == sep)
        ++at;

    return (size_t)(at - name);
}

mume_virtfs_t* mume_virtfs_create(const char *path)
{
    mume_virtfs_t *vfs;
    char absdir[_MAX_PATH_LEN];
    if (!_convert_to_abspath(absdir, _MAX_PATH_LEN, path))
        return NULL;
    vfs = malloc_abort(
        sizeof(mume_virtfs_t) + strlen(absdir) + 1);
    _physfs_reference(1);
    vfs->mount = PHYSFS_getMountPoint(absdir);
    strcpy(EXTRA_OF(mume_virtfs_t, vfs), absdir);
    vfs->refcount = 1;
    if (NULL == vfs->mount) {
        static int _mountid;
        char mount[_MOUNT_DIR_LEN];
        ++_mountid;
        snprintf(mount, _MOUNT_DIR_LEN, "%d", _mountid);
        if (PHYSFS_mount(absdir, mount, 1))
            vfs->mount = PHYSFS_getMountPoint(absdir);
    }

    if (NULL == vfs->mount) {
        free(vfs);
        _physfs_reference(0);
        return NULL;
    }
    return vfs;
}

void mume_virtfs_destroy(mume_virtfs_t *vfs)
{
    if (0 == --vfs->refcount) {
        if (_writefs == vfs) {
            _writefs = NULL;
        }
        free(vfs);
        _physfs_reference(0);
    }
}

mume_virtfs_t* mume_virtfs_reference(mume_virtfs_t *vfs)
{
    ++vfs->refcount;
    return vfs;
}

int mume_virtfs_exists(mume_virtfs_t *vfs, const char *name)
{
    char mount[_MAX_PATH_LEN];
    snprintf(mount, _MAX_PATH_LEN,
             "%s%s", vfs->mount, name);
    return PHYSFS_exists(mount);
}

int mume_virtfs_delete(mume_virtfs_t *vfs, const char *name)
{
    if (_change_writedir(vfs))
        return PHYSFS_delete(name);
    return 0;
}

mume_stream_t* mume_virtfs_open(
    mume_virtfs_t *vfs, const char *name, int mode)
{
    static struct mume_stream_i _impl = {
        _virtfs_stream_length,
        _virtfs_stream_eof,
        _virtfs_stream_tell,
        _virtfs_stream_seek,
        _virtfs_stream_read,
        _virtfs_stream_write,
        _virtfs_stream_close,
    };
    _virtfs_stream_t *stm = malloc_struct(_virtfs_stream_t);
    switch (mode) {
    case MUME_OM_READ:
        {
            char mount[_MAX_PATH_LEN];
            snprintf(mount, _MAX_PATH_LEN,
                     "%s%s", vfs->mount, name);
            stm->file = PHYSFS_openRead(mount);
        }
        break;
    case MUME_OM_WRITE:
        if (_change_writedir(vfs))
            stm->file = PHYSFS_openWrite(name);
        break;
    case MUME_OM_APPEND:
        if (_change_writedir(vfs))
            stm->file = PHYSFS_openAppend(name);
        break;
    default:
        stm->file = NULL;
        assert(0);
    }

    if (NULL == stm->file) {
        free(stm);
        return NULL;
    }

    stm->vfs = mume_virtfs_reference(vfs);
    stm->base.impl = &_impl;
    stm->base.refcount = 0;
    return (mume_stream_t*)stm;
}
