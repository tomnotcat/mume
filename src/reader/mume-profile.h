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
#ifndef MUME_READER_PROFILE_H
#define MUME_READER_PROFILE_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_PROFILE (MUME_SIZEOF_OBJECT + \
                             sizeof(void*))

#define MUME_SIZEOF_PROFILE_CLASS (MUME_SIZEOF_CLASS)

murdr_public const void* mume_profile_class(void);

#define mume_profile_meta_class mume_meta_class

#define mume_profile_new() mume_new(mume_profile_class())

murdr_public int mume_profile_load(void *self, mume_stream_t *stm);

murdr_public int mume_profile_save(void *self, mume_stream_t *stm);

#define mume_profile_load_from_file(_self, _file) \
    mume_process_file_stream( \
        mume_profile_load, _self, _file, MUME_OM_READ)

#define mume_profile_save_to_file(_self, _file) \
    mume_process_file_stream( \
        mume_profile_save, _self, _file, MUME_OM_WRITE)

murdr_public void mume_profile_set_int(
    void *self, const char *section, const char *name, int value);

murdr_public int mume_profile_get_int(
    void *self, const char *section, const char *name, int defval);

murdr_public void mume_profile_set_float(
    void *self, const char *section, const char *name, float value);

murdr_public float mume_profile_get_float(
    void *self, const char *section, const char *name, float defval);

murdr_public void mume_profile_set_rect(
    void *self, const char *section,
    const char *name, mume_rect_t value);

murdr_public mume_rect_t mume_profile_get_rect(
    void *self, const char *section,
    const char *name, mume_rect_t defval);

MUME_END_DECLS

#endif /* MUME_READER_PROFILE_H */
