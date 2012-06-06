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
#ifndef MUME_FOUNDATION_DATASRC_H
#define MUME_FOUNDATION_DATASRC_H

#include "mume-refobj.h"

MUME_BEGIN_DECLS

/* Data format class. */
#define MUME_SIZEOF_DATAFMT (MUME_SIZEOF_OBJECT + \
                             sizeof(char*))

#define MUME_SIZEOF_DATAFMT_CLASS (MUME_SIZEOF_CLASS)

/* Standard data formats. */
#define MUME_DATAFMT_TEXT "text"

mume_public const void* mume_datafmt_class(void);

mume_public const void* mume_datafmt_meta_class(void);

mume_public const char* mume_datafmt_get_name(const void *self);

/* Data record class. */
#define MUME_SIZEOF_DATAREC (MUME_SIZEOF_REFOBJ + \
                             sizeof(void*) * 2 +  \
                             sizeof(size_t))

#define MUME_SIZEOF_DATAREC_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                   sizeof(voidf*) * 3)

mume_public const void* mume_datarec_class(void);

mume_public const void* mume_datarec_meta_class(void);

mume_public void* mume_datarec_new(
    const void *format, void *data, size_t length);

mume_public void* mume_datarec_new_with_text(const char *text);

/* Selector for get the data format of the data record. */
mume_public const void* _mume_datarec_get_format(
    const void *clazz, void *self);

#define mume_datarec_get_format(_self) \
    _mume_datarec_get_format(NULL, _self)

/* Selector for get the data ptr of the data record. */
mume_public const void* _mume_datarec_get_data(
    const void *clazz, void *self);

#define mume_datarec_get_data(_self) \
    _mume_datarec_get_data(NULL, _self)

/* Selector for get the data length of the data record. */
mume_public size_t _mume_datarec_get_length(
    const void *clazz, void *self);

#define mume_datarec_get_length(_self) \
    _mume_datarec_get_length(NULL, _self)

/* Data source class. */
#define MUME_SIZEOF_DATASRC (MUME_SIZEOF_REFOBJ + \
                             sizeof(void**) * 2 + \
                             sizeof(int))

#define MUME_SIZEOF_DATASRC_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                   sizeof(voidf*) * 2)

mume_public const void* mume_datasrc_class(void);

mume_public const void* mume_datasrc_meta_class(void);

mume_public void* mume_datasrc_new(void **datas, int count);

/* Selector for get the available data formats of the data source,
 * the returned array is NULL terminated. */
mume_public const void** _mume_datasrc_get_formats(
    const void *clazz, void *self);

#define mume_datasrc_get_formats(_self) \
    _mume_datasrc_get_formats(NULL, _self)

/* Count the formats returned by mume_datasrc_get_formats. */
mume_public int mume_datasrc_count_formats(const void **formats);

/* Selector for get the data record of the specified format. */
mume_public void* _mume_datasrc_get_data(
    const void *clazz, void *self, const void *format);

#define mume_datasrc_get_data(_self, _format) \
    _mume_datasrc_get_data(NULL, _self, _format)

/* Create a snapshot of current data source. So the following
 * call to visit the data of this datasrc will read data from
 * the cache instead of the original source. */
mume_public void mume_datasrc_snapshot(void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_DATASRC_H */
