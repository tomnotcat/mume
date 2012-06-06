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
#ifndef MUME_FOUNDATION_VIRTFS_H
#define MUME_FOUNDATION_VIRTFS_H

#include "mume-common.h"

MUME_BEGIN_DECLS

/*========================================
 * [function]
 *  get the path where application resides.
 *  get the path where user's home directory resides.
 * [return]
 *  the directory, no directory separator at end.
 * [notice]
 *  user should not destroy the returned value.
 *========================================*/
mume_public const char* mume_virtfs_basedir(void);
mume_public const char* mume_virtfs_userdir(void);

/*========================================
 * [function]
 *  get the directory separator of native system.
 * [return]
 *  the directory separator
 * [example]
 *  '\\' on win32, '/' on linux.
 *========================================*/
mume_public char mume_virtfs_dirsep(void);

/*========================================
 * [function]
 *  get the directory part length of a file path.
 * [parameter]
 *  path : the file path name in native format.
 * [return]
 *  the directory part length
 * [example]
 *  "/home/test.txt" will return 6
 *  "c:\dir\file" will return 7
 *========================================*/
mume_public size_t mume_virtfs_dirlen(const char *name);

/*========================================
 * [function]
 *  create/destroy a virtual file system.
 * [parameter]
 *  path : can be a native directory or any
 *         supported archive (usually zip).
 * [return]
 *  the created vfs or NULL for fail.
 *========================================*/
mume_public mume_virtfs_t* mume_virtfs_create(
    const char *path);

mume_public void mume_virtfs_destroy(
    mume_virtfs_t *vfs);

mume_public mume_virtfs_t* mume_virtfs_reference(
    mume_virtfs_t *vfs);

/*========================================
 * [function]
 *  test if a file exists in the virtual file system.
 * [parameter]
 *  name : platform-independent file path
 * [return]
 *  nonzero : file exist
 *  zero : not exist
 *========================================*/
mume_public int mume_virtfs_exists(
    mume_virtfs_t *vfs, const char *name);

/*========================================
 * [function]
 *  delete a file in the virtual file system.
 * [parameter]
 *  name : platform-independent file path
 * [return]
 *  nonzero : success
 *  zero : fail
 *========================================*/
mume_public int mume_virtfs_delete(
    mume_virtfs_t *vfs, const char *name);

/*========================================
 * [function]
 *  open a file from the virtual file system.
 * [parameter]
 *  name : platform-independent file path
 *  mode : one of the mume_open_mode_e,
 * [return]
 *  the opened file or NULL for fail
 * [notice]
 *  MUME_OM_READ: open for read, return NULL
 *    if file not exist.
 *  MUME_OM_WRITE: open for write, create file
 *    if not exist, the file is truncated to zero.
 *  MUME_OM_APPEND: open for write, create file
 *    if not exist, cursor is set to the end of the file.
 *========================================*/
mume_public mume_stream_t* mume_virtfs_open(
    mume_virtfs_t *vfs, const char *name, int mode);

#define mume_virtfs_open_read(_vfs, _name) \
    mume_virtfs_open(_vfs, _name, MUME_OM_READ)

#define mume_virtfs_open_write(_vfs, _name) \
    mume_virtfs_open(_vfs, _name, MUME_OM_WRITE)

#define mume_virtfs_open_append(_vfs, _name) \
    mume_virtfs_open(_vfs, _name, MUME_OM_APPEND)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_VIRTFS_H */
