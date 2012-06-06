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
#ifndef MUME_READER_DOCMGR_H
#define MUME_READER_DOCMGR_H

/* The docmgr object manage all the loaded (opened) documents. */

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_DOCMGR (MUME_SIZEOF_OBJECT +    \
                            sizeof(void*) +         \
                            sizeof(void*))

#define MUME_SIZEOF_DOCMGR_CLASS (MUME_SIZEOF_CLASS)

murdr_public const void* mume_docmgr_class(void);

#define mume_docmgr_meta_class mume_meta_class

#define mume_docmgr_new() mume_new(mume_docmgr_class())

/* Register a document class.
 *
 * <type> is the file type that <clazz> support.
 * <clazz> must be a class derive from "docdoc class".
 */
murdr_public void mume_docmgr_register(
    void *self, int type, const void *clazz);

/* Load a document from stream.
 *
 * <type> is the file type of the stream, if <type> is NULL,
 * the file type will be determined by its contents.
 *
 * Return the doc object, which is managed by the docmgr.
 */
murdr_public void* mume_docmgr_load(
    void *self, int type, mume_stream_t *stm);

/* Load a document from file. */
murdr_public void* mume_docmgr_load_file(
    void *self, const char *file);

MUME_END_DECLS

#endif /* MUME_READER_DOCMGR_H */
