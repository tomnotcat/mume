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
#ifndef MUME_READER_DOCVIEW_H
#define MUME_READER_DOCVIEW_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_DOCVIEW (MUME_SIZEOF_SCROLLVIEW +   \
                             sizeof(void*) * 4 +        \
                             sizeof(int) * 14)

#define MUME_SIZEOF_DOCVIEW_CLASS (MUME_SIZEOF_SCROLLVIEW_CLASS)

murdr_public const void* mume_docview_class(void);

murdr_public const void* mume_docview_meta_class(void);

murdr_public void* mume_docview_new(
    void *parent, int x, int y, int width, int height);

murdr_public void mume_docview_set_doc(void *self, void *doc);

murdr_public void* mume_docview_get_doc(const void *self);

murdr_public int mume_docview_count_pages(const void *self);

murdr_public int mume_docview_first_visible(const void *self);

murdr_public int mume_docview_page_from(const void *self, int y);

murdr_public void mume_docview_client_to_page(
    const void *self, int pageno, int *x, int *y);

murdr_public void mume_docview_page_to_client(
    const void *self, int pageno, int *x, int *y);

murdr_public void mume_docview_set_zoom(void *self, float zoom);

murdr_public float mume_docview_get_zoom(const void *self);

murdr_public void mume_docview_zoom_by(
    void *self, float factor, int x, int y);

murdr_public void mume_docview_set_rotate(void *self, int rotate);

murdr_public int mume_docview_get_rotate(const void *self);

murdr_public void mume_docview_rotate_by(void *self, int rotate);

murdr_public mume_type_t* mume_typeof_docview_theme(void);

MUME_END_DECLS

#endif /* MUME_READER_DOCVIEW_H */
