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
#include "mume-docdoc.h"

#define _docdoc_super_class mume_refobj_class
#define _docdoc_super_meta_class mume_refobj_meta_class

struct _docdoc {
    const char _[MUME_SIZEOF_REFOBJ];
};

struct _docdoc_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    int (*load)(void *self, mume_stream_t *stm);
    const char* (*title)(void *self);
    int (*count_pages)(void *self);
    mume_rect_t (*get_mediabox)(void *self, int pageno);
    mume_matrix_t (*get_matrix)(
        void *self, int pageno, float zoom, int rotate);
    int (*text_length)(void *self, int pageno);
    void (*extract_text)(void *self, int pageno,
                         char *tbuf, mume_rect_t *rbuf);
    void (*render_page)(void *self, cairo_t *cr, int x, int y,
                        int p, mume_matrix_t m, mume_rect_t r);
    mume_tocitem_t* (*get_toc_tree)(void *self);
    mume_doclink_t* (*get_page_links)(void *self, int pageno);
};

MUME_STATIC_ASSERT(sizeof(struct _docdoc) == MUME_SIZEOF_DOCDOC);
MUME_STATIC_ASSERT(sizeof(struct _docdoc_class) ==
                   MUME_SIZEOF_DOCDOC_CLASS);

static int _docdoc_load(void *self, mume_stream_t *stm)
{
    return 0;
}

static const char* _docdoc_title(void *self)
{
    return NULL;
}

static int _docdoc_count_pages(void *self)
{
    return 0;
}

static mume_rect_t _docdoc_get_mediabox(void *self, int pageno)
{
    return mume_rect_empty;
}

static mume_matrix_t _docdoc_get_matrix(
    void *self, int pageno, float zoom, int rotate)
{
    return mume_matrix_identity;
}

static int _docdoc_text_length(void *self, int pageno)
{
    return 0;
}

static void _docdoc_extract_text(
    void *self, int pageno, char *tbuf, mume_rect_t *rbuf)
{
}

static void _docdoc_render_page(
    void *self, cairo_t *cr, int x, int y,
    int p, mume_matrix_t m, mume_rect_t r)
{
}

static mume_tocitem_t* _docdoc_get_toc_tree(void *self)
{
    return NULL;
}

static mume_doclink_t* _docdoc_get_page_links(void *self, int pageno)
{
    return NULL;
}

static void* _docdoc_class_ctor(
    struct _docdoc_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_docdoc_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_docdoc_load)
            *(voidf**)&self->load = method;
        else if (selector == (voidf*)_mume_docdoc_title)
            *(voidf**)&self->title = method;
        else if (selector == (voidf*)_mume_docdoc_count_pages)
            *(voidf**)&self->count_pages = method;
        else if (selector == (voidf*)_mume_docdoc_get_mediabox)
            *(voidf**)&self->get_mediabox = method;
        else if (selector == (voidf*)_mume_docdoc_get_matrix)
            *(voidf**)&self->get_matrix = method;
        else if (selector == (voidf*)_mume_docdoc_text_length)
            *(voidf**)&self->text_length = method;
        else if (selector == (voidf*)_mume_docdoc_extract_text)
            *(voidf**)&self->extract_text = method;
        else if (selector == (voidf*)_mume_docdoc_render_page)
            *(voidf**)&self->render_page = method;
        else if (selector == (voidf*)_mume_docdoc_get_toc_tree)
            *(voidf**)&self->get_toc_tree = method;
        else if (selector == (voidf*)_mume_docdoc_get_page_links)
            *(voidf**)&self->get_page_links = method;
    }

    return self;
}

const void* mume_docdoc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_docdoc_meta_class(),
        "docdoc",
        _docdoc_super_class(),
        sizeof(struct _docdoc),
        MUME_PROP_END,
        _mume_docdoc_load, _docdoc_load,
        _mume_docdoc_title, _docdoc_title,
        _mume_docdoc_count_pages,
        _docdoc_count_pages,
        _mume_docdoc_get_mediabox,
        _docdoc_get_mediabox,
        _mume_docdoc_get_matrix,
        _docdoc_get_matrix,
        _mume_docdoc_text_length,
        _docdoc_text_length,
        _mume_docdoc_extract_text,
        _docdoc_extract_text,
        _mume_docdoc_render_page,
        _docdoc_render_page,
        _mume_docdoc_get_toc_tree,
        _docdoc_get_toc_tree,
        _mume_docdoc_get_page_links,
        _docdoc_get_page_links,
        MUME_FUNC_END);
}

const void* mume_docdoc_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "docdoc class",
        _docdoc_super_meta_class(),
        sizeof(struct _docdoc_class),
        MUME_PROP_END,
        _mume_ctor, _docdoc_class_ctor,
        MUME_FUNC_END);
}

int _mume_docdoc_load(
    const void *_clazz, void *_self, mume_stream_t *stm)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, load, (_self, stm));
}

const char* _mume_docdoc_title(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, title, (_self));
}

int _mume_docdoc_count_pages(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, count_pages, (_self));
}

mume_rect_t _mume_docdoc_get_mediabox(
    const void *_clazz, void *_self, int pageno)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, get_mediabox, (_self, pageno));
}

mume_matrix_t _mume_docdoc_get_matrix(
    const void *_clazz, void *_self, int pageno, float zoom, int rotate)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, get_matrix,
        (_self, pageno, zoom, rotate));
}

int _mume_docdoc_text_length(const void *_clazz, void *_self, int pageno)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, text_length, (_self, pageno));
}

void _mume_docdoc_extract_text(
    const void *_clazz, void *_self, int pageno,
    char *tbuf, mume_rect_t *rbuf)
{
    MUME_SELECTOR_NORETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, extract_text,
        (_self, pageno, tbuf, rbuf));
}

void _mume_docdoc_render_page(
    const void *_clazz, void *_self, cairo_t *cr, int x, int y,
    int pageno, mume_matrix_t ctm, mume_rect_t rect)
{
    MUME_SELECTOR_NORETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, render_page,
        (_self, cr, x, y, pageno, ctm, rect));
}

mume_tocitem_t* _mume_docdoc_get_toc_tree(
    const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, get_toc_tree, (_self));
}

mume_doclink_t* _mume_docdoc_get_page_links(
    const void *_clazz, void *_self, int pageno)
{
    MUME_SELECTOR_RETURN(
        mume_docdoc_meta_class(), mume_docdoc_class(),
        struct _docdoc_class, get_page_links, (_self, pageno));
}

mume_tocitem_t* mume_tocitem_create(
    mume_tocitem_t *parent, mume_tocitem_t *sibling,
    const char *title, int pageno)
{
    mume_tocitem_t *item;
    size_t size = sizeof(mume_tocitem_t);

    if (title)
        size += strlen(title) + 1;

    item = malloc_abort(size);
    if (title) {
        item->title = EXTRA_OF(mume_tocitem_t, item);
        strcpy((char*)item->title, title);
    }
    else {
        item->title = NULL;
    }

    item->pageno = pageno;
    if (sibling) {
        sibling->next = item;
        item->next = NULL;
    }
    else if (parent) {
        item->next = parent->child;
        parent->child = item;
    }

    item->child = NULL;
    return item;
}

void mume_tocitem_destroy(mume_tocitem_t *self)
{
    if (self->next)
        mume_tocitem_destroy(self->next);

    if (self->child)
        mume_tocitem_destroy(self->child);

    free(self);
}

mume_doclink_t* mume_doclink_create(
    mume_doclink_t *prev, mume_rect_t src_rect, mume_rect_t dest_rect,
    int dest_pageno, const char *dest_value)
{
    mume_doclink_t *link;
    size_t size = sizeof(mume_doclink_t);

    if (dest_value)
        size += strlen(dest_value) + 1;

    link = malloc_abort(size);
    link->src_rect = src_rect;
    link->dest_rect = dest_rect;
    link->dest_pageno = dest_pageno;

    if (dest_value) {
        link->dest_value = EXTRA_OF(mume_doclink_t, link);
        strcpy((char*)link->dest_value, dest_value);
    }
    else {
        link->dest_value = NULL;
    }

    if (prev) {
        assert(NULL == prev->next);
        prev->next = link;
    }

    link->next = NULL;
    return link;
}

void mume_doclink_destroy(mume_doclink_t *self)
{
    mume_doclink_t *next;

    while (self) {
        next = self->next;
        free(self);
        self = next;
    }
}
