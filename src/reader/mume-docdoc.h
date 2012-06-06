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
#ifndef MUME_READER_DOCDOC_H
#define MUME_READER_DOCDOC_H

/* The docdoc object represent a loaded (opened) document. */

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_DOCDOC (MUME_SIZEOF_REFOBJ)

#define MUME_SIZEOF_DOCDOC_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                  sizeof(voidf*) * 10)

typedef struct mume_tocitem_s mume_tocitem_t;
typedef struct mume_doclink_s mume_doclink_t;

struct mume_tocitem_s {
    const char *title;
    int pageno;
    mume_tocitem_t *next;
    mume_tocitem_t *child;
};

struct mume_doclink_s {
    mume_rect_t src_rect;
    mume_rect_t dest_rect;
    int dest_pageno;
    const char *dest_value;
    mume_doclink_t *next;
};

murdr_public const void* mume_docdoc_class(void);

murdr_public const void* mume_docdoc_meta_class(void);

/* Selector for load document. */
murdr_public int _mume_docdoc_load(
    const void *clazz, void *self, mume_stream_t *stm);

#define mume_docdoc_load(_self, _stm) \
    _mume_docdoc_load(NULL, _self, _stm)

/* Selector for get document title. */
murdr_public const char* _mume_docdoc_title(
    const void *clazz, void *self);

#define mume_docdoc_title(_self) \
    _mume_docdoc_title(NULL, _self)

/* Selector for get document page count. */
murdr_public int _mume_docdoc_count_pages(
    const void *clazz, void *self);

#define mume_docdoc_count_pages(_self) \
    _mume_docdoc_count_pages(NULL, _self)

/* Selector for get the specified page's media box. */
murdr_public mume_rect_t _mume_docdoc_get_mediabox(
    const void *clazz, void *self, int pageno);

#define mume_docdoc_get_mediabox(_self, _pageno) \
    _mume_docdoc_get_mediabox(NULL, _self, _pageno)

/* Selector for get the specified page's transform matrix. */
murdr_public mume_matrix_t _mume_docdoc_get_matrix(
    const void *clazz, void *self, int pageno, float zoom, int rotate);

#define mume_docdoc_get_matrix(_self, _pageno, _zoom, _rotate) \
    _mume_docdoc_get_matrix(NULL, _self, _pageno, _zoom, _rotate)

/* Selector for get the text length of the specified page. */
murdr_public int _mume_docdoc_text_length(
    const void *clazz, void *self, int pageno);

#define mume_docdoc_text_length(_self, _pageno) \
    _mume_docdoc_text_length(NULL, _self, _pageno)

/* Selector for extract the text of the specified page. */
murdr_public void _mume_docdoc_extract_text(
    const void *clazz, void *self, int pageno,
    char *tbuf, mume_rect_t *rbuf);

#define mume_docdoc_extract_text(_self, _pageno, _tbuf, _rbuf) \
    _mume_docdoc_extract_text(NULL, _self, _pageno, _tbuf, _rbuf)

/* Selector for render the specified page. */
murdr_public void _mume_docdoc_render_page(
    const void *clazz, void *self, cairo_t *cr, int x, int y,
    int pageno, mume_matrix_t ctm, mume_rect_t rect);

#define mume_docdoc_render_page(_self, _cr, _x, _y, _p, _m, _r) \
    _mume_docdoc_render_page(NULL, _self, _cr, _x, _y, _p, _m, _r)

/* Selector for get the document's table of contexts. */
murdr_public mume_tocitem_t* _mume_docdoc_get_toc_tree(
    const void *clazz, void *self);

#define mume_docdoc_get_toc_tree(_self) \
    _mume_docdoc_get_toc_tree(NULL, _self)

/* Selector for get all the links of the specified page. */
murdr_public mume_doclink_t* _mume_docdoc_get_page_links(
    const void *clazz, void *self, int pageno);

#define mume_docdoc_get_page_links(_self, _pageno) \
    _mume_docdoc_get_page_links(NULL, _self, _pageno)

/* Utility functions. */
murdr_public mume_tocitem_t* mume_tocitem_create(
    mume_tocitem_t *parent, mume_tocitem_t *sibling,
    const char *title, int pageno);

murdr_public void mume_tocitem_destroy(mume_tocitem_t *self);

murdr_public mume_doclink_t* mume_doclink_create(
    mume_doclink_t *prev, mume_rect_t src_rect, mume_rect_t dest_rect,
    int dest_pageno, const char *dest_value);

murdr_public void mume_doclink_destroy(mume_doclink_t *self);

MUME_END_DECLS

#endif /* MUME_READER_DOCDOC_H */
