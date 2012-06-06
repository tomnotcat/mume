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
#include "mupdf/fitz.h"
#include "mupdf/mupdf.h"
#include "mume-pdf-doc.h"

#define _pdf_doc_super_class mume_docdoc_class

struct _pdf_doc {
    const char _[MUME_SIZEOF_DOCDOC];
    pdf_xref *xref;
    fz_glyph_cache *glyph_cache;
    pdf_page **pages;         /* Loaded pages. */
    fz_display_list **disps;  /* Extracted display lists. */
    fz_rect *media_boxes;     /* Original media boxes from PDF. */
    int *page_rotates;        /* Original page rotates from PDF. */
};

static void _pdf_doc_reset(struct _pdf_doc *self)
{
    self->xref = NULL;
    self->glyph_cache = NULL;
    self->pages = NULL;
    self->disps = NULL;
    self->media_boxes = NULL;
    self->page_rotates = NULL;
}

static void _pdf_doc_clear(struct _pdf_doc *self)
{
    int i, c = mume_docdoc_count_pages(self);

    if (self->pages) {
        for (i = 0; i < c; ++i) {
            if (self->pages[i])
                pdf_free_page(self->pages[i]);
        }

        free(self->pages);
    }

    if (self->disps) {
        for (i = 0; i < c; ++i) {
            if (self->disps[i])
                fz_free_display_list(self->disps[i]);
        }

        free(self->disps);
    }

    if (self->glyph_cache)
        fz_free_glyph_cache(self->glyph_cache);

    if (self->xref)
        pdf_free_xref(self->xref);

    free(self->media_boxes);
    free(self->page_rotates);

    _pdf_doc_reset(self);
}

static pdf_page* _pdf_doc_get_page(struct _pdf_doc *self, int pageno)
{
    if (self->pages[pageno])
        return self->pages[pageno];

    if (NULL == self->pages[pageno]) {
        fz_error err = pdf_load_page(
            &self->pages[pageno], self->xref, pageno);

        if (err) {
            mume_error(("pdf_load_page(%d): %d\n", pageno, err));
        }
    }

    return self->pages[pageno];
}

static fz_display_list* _pdf_doc_get_list(
    struct _pdf_doc *self, int pageno)
{
    pdf_page *page;

    if (self->disps[pageno])
        return self->disps[pageno];

    page = _pdf_doc_get_page(self, pageno);
    if (NULL == self->disps[pageno]) {
        fz_error err;
        fz_device *mdev;

        self->disps[pageno] = fz_new_display_list();
        mdev = fz_new_list_device(self->disps[pageno]);
        err = pdf_run_page(self->xref, page, mdev, fz_identity);

        if (err) {
            mume_error(("pdf_run_page(%d): %d\n", pageno, err));
        }

        fz_free_device(mdev);
    }

    return self->disps[pageno];
}

static int _pdf_doc_find_page_no(
    struct _pdf_doc *self, fz_obj *dest)
{
    if (fz_is_dict(dest)) {
        /* The destination is linked from a Go-To action's D array. */
        fz_obj * D = fz_dict_gets(dest, "D");
        if (D && fz_is_array(D))
            dest = D;
    }

    if (fz_is_array(dest))
        dest = fz_array_get(dest, 0);

    if (fz_is_int(dest))
        return fz_to_int(dest);

    return pdf_find_page_number(self->xref, dest);
}

static mume_tocitem_t* _pdf_doc_build_toc_tree(
    struct _pdf_doc *self, mume_tocitem_t *parent, pdf_outline *entry)
{
    int pageno;
    mume_tocitem_t *first = NULL;
    mume_tocitem_t *node = NULL;

    for (; entry; entry = entry->next) {
        if (entry->link && PDF_LINK_GOTO == entry->link->kind)
            pageno = _pdf_doc_find_page_no(self, entry->link->dest);
        else
            pageno = 0;

        node = mume_tocitem_create(
            parent, node, entry->title, pageno);

        if (NULL == first)
            first = node;

        if (entry->child)
            _pdf_doc_build_toc_tree(self, node, entry->child);
    }

    return first;
}

static mume_rect_t _pdf_doc_get_link_dest_rect(
    struct _pdf_doc *self, pdf_link *link)
{
    mume_rect_t rect = mume_rect_empty;
    fz_obj *dest = link->dest;
    fz_obj *obj = fz_array_get(dest, 1);
    const char *type = fz_to_name(obj);

    if (strcmp(type, "XYZ") == 0) {
        /* NULL values for the coordinates mean: keep the
         * current position. */
        if (!fz_is_null(fz_array_get(dest, 2)))
            rect.x = round(fz_to_real(fz_array_get(dest, 2)));

        if (!fz_is_null(fz_array_get(dest, 3)))
            rect.y = round(fz_to_real(fz_array_get(dest, 3)));
    }
    else if (strcmp(type, "FitR") == 0) {
        double x0, y0, x1, y1;

        x0 = fz_to_real(fz_array_get(dest, 2));
        y0 = fz_to_real(fz_array_get(dest, 5));
        x1 = fz_to_real(fz_array_get(dest, 4));
        y1 = fz_to_real(fz_array_get(dest, 3));

        rect.x = round(x0);
        rect.y = round(y0);
        rect.width = round(x1 - x0);
        rect.height = round(y1 - y0);
    }
    else if (strcmp(type, "FitH") == 0 || strcmp(type, "FitBH") == 0) {
        rect.y = round(fz_to_real(fz_array_get(dest, 2)));
    }

    return rect;
}

static const char* _pdf_doc_get_link_dest_value(
    struct _pdf_doc *self, pdf_link *link)
{
    /*
    char *path = NULL;
    fz_obj *obj;

    switch (link->kind) {
    case PDF_LINK_URI:
        path = str::conv::FromPdf(link->dest);
        if (IsRelativeURI(path)) {
            obj = fz_dict_gets(engine->_xref->trailer, "Root");
            obj = fz_dict_gets(fz_dict_gets(obj, "URI"), "Base");
            ScopedMem<TCHAR> base(obj ? str::conv::FromPdf(obj) : NULL);
            if (!str::IsEmpty(base.Get())) {
                ScopedMem<TCHAR> uri(str::Join(base, path));
                free(path);
                path = uri.StealData();
            }
        }
        break;

    case PDF_LINK_LAUNCH:
        // note: we (intentionally) don't support the /Win specific Launch parameters
        path = FilespecToPath(link->dest);
        if (path && str::Eq(GetType(), "LaunchEmbedded") && str::EndsWithI(path, _T(".pdf"))) {
            free(path);
            obj = dest();
            path = str::Format(_T("%s:%d:%d"), engine->FileName(), fz_to_num(obj), fz_to_gen(obj));
        }
        break;
    case PDF_LINK_ACTION:
        obj = fz_dict_gets(link->dest, "S");
        if (fz_is_name(obj) && str::Eq(fz_to_name(obj), "GoToR"))
            path = FilespecToPath(fz_dict_gets(link->dest, "F"));
        break;
    }

    return path;
    */
    return "hello";
}

static fz_bbox _mume_rect_to_fz_bbox(mume_rect_t r)
{
    fz_bbox bbox;
    bbox.x0 = r.x;
    bbox.y0 = r.y;
    bbox.x1 = r.x + r.width;
    bbox.y1 = r.y + r.height;
    return bbox;
}

static mume_rect_t _fz_bbox_to_mume_rect(fz_bbox b)
{
    mume_rect_t rect;
    rect.x = b.x0;
    rect.y = b.y0;
    rect.width = b.x1 - b.x0;
    rect.height = b.y1 - b.y0;
    return rect;
}

static mume_rect_t _fz_rect_to_mume_rect(fz_rect r)
{
    mume_rect_t rect;
    rect.x = round(r.x0);
    rect.y = round(r.y0);
    rect.width = round(r.x1 - r.x0);
    rect.height = round(r.y1 - r.y0);
    return rect;
}

static fz_matrix _mume_matrix_to_fz_matrix(mume_matrix_t m)
{
    fz_matrix rm;
    rm.a = m.a;
    rm.b = m.b;
    rm.c = m.c;
    rm.d = m.d;
    rm.e = m.e;
    rm.f = m.f;
    return rm;
}

static int _pdf_stream_read(fz_stream *stm, unsigned char *buf, int len)
{
    return mume_stream_read(stm->state, buf, len);
}

static void _pdf_stream_seek(fz_stream *stm, int offset, int whence)
{
    size_t pos = 0;
    if (0 == whence) {
        pos = offset;
    }
    else if (1 == whence) {
        pos = mume_stream_tell(stm->state) + offset;
    }
    else if (2 == whence) {
        pos = mume_stream_length(stm->state) - offset;
    }

    if (mume_stream_seek(stm->state, pos)) {
        stm->pos = pos;
        stm->rp = stm->bp;
        stm->wp = stm->bp;
    }
    else {
        mume_error(("Seek failed: %u\n", pos));
    }
}

static void _pdf_stream_close(fz_stream *stm)
{
    mume_stream_close(stm->state);
}

static void* _pdf_doc_ctor(
    struct _pdf_doc *self, int mode, va_list *app)
{
    if (!_mume_ctor(_pdf_doc_super_class(), self, mode, app))
        return NULL;

    _pdf_doc_reset(self);
    return self;
}

static void* _pdf_doc_dtor(struct _pdf_doc *self)
{
    _pdf_doc_clear(self);
    return _mume_dtor(_pdf_doc_super_class(), self);
}

static int _pdf_doc_load(struct _pdf_doc *self, mume_stream_t *stm)
{
    fz_error error;
    fz_stream *fzstm;
    fz_rect *mbox;
    fz_obj *page_obj, *box_obj;
    int i, c;

    _pdf_doc_clear(self);

    fzstm = fz_new_stream(stm, _pdf_stream_read, _pdf_stream_close);
    mume_stream_reference(stm);
    fzstm->seek = _pdf_stream_seek;
    error = pdf_open_xref_with_stream(&self->xref, fzstm, NULL);
    fz_close(fzstm);
    if (error) {
        mume_error(("Read xref failed\n", error));
        return 0;
    }

    assert(!pdf_needs_password(self->xref));

    /* Load meta information. */
    error = pdf_load_page_tree(self->xref);
    if (error) {
        mume_error(("Cannot load page tree\n"));
        return 0;
    }

    c = pdf_count_pages(self->xref);
    self->glyph_cache = fz_new_glyph_cache();
    self->pages = calloc_abort(c, sizeof(pdf_page*));
    self->disps = calloc_abort(c, sizeof(fz_display_list*));
    self->media_boxes = malloc_abort(c * sizeof(fz_rect));
    self->page_rotates = malloc_abort(c * sizeof(int));

    /* Extract each pages' media box and rotation. */
    for (i = 0; i < c; ++i) {
        mbox = self->media_boxes + i;
        page_obj = self->xref->page_objs[i];
        if (!page_obj) {
            *mbox = fz_empty_rect;
            continue;
        }

        box_obj = fz_dict_gets(page_obj, "MediaBox");
        *mbox = pdf_to_rect(box_obj);
        if (fz_is_empty_rect(*mbox)) {
            fz_warn("Cannot find page bounds, guessing page bounds.");
            mbox->x1 = 612;
            mbox->y1 = 792;
        }

        box_obj = fz_dict_gets(page_obj, "CropBox");
        if (fz_is_array(box_obj))
            *mbox = fz_intersect_rect(*mbox, pdf_to_rect(box_obj));

        self->page_rotates[i] = fz_to_int(
            fz_dict_gets(page_obj, "Rotate"));

        if (self->page_rotates[i] % 90)
            self->page_rotates[i] = 0;
    }

    return 1;
}

static const char* _pdf_doc_title(struct _pdf_doc *self)
{
    if (self->xref) {
        fz_obj *info, *obj;
        info = fz_dict_gets(self->xref->trailer, "Info");
        if (info) {
            obj = fz_dict_gets(info, "Title");
            if (obj)
                return pdf_to_utf8(obj);
        }
    }

    return NULL;
}

static int _pdf_doc_count_pages(const struct _pdf_doc *self)
{
    if (self->xref)
        return pdf_count_pages(self->xref);

    return 0;
}

static mume_rect_t _pdf_doc_get_mediabox(
    struct _pdf_doc *self, int pageno)
{
    mume_rect_t rect;
    rect.x = self->media_boxes[pageno].x0;
    rect.y = self->media_boxes[pageno].y0;
    rect.width = self->media_boxes[pageno].x1 - rect.x;
    rect.height = self->media_boxes[pageno].y1 - rect.y;
    return rect;
}

static mume_matrix_t _pdf_doc_get_matrix(
    struct _pdf_doc *self, int pageno, float zoom, int rotate)
{
    mume_matrix_t m;
    fz_matrix ctm = fz_identity;
    fz_rect mbox = self->media_boxes[pageno];
    int rotation = (self->page_rotates[pageno] + rotate) % 360;
    if (rotation < 0)
        rotation = rotation + 360;

    if (90 == rotation) {
        ctm = fz_concat(ctm, fz_translate(-mbox.x0, -mbox.y0));
    }
    else if (180 == rotation) {
        ctm = fz_concat(ctm, fz_translate(-mbox.x1, -mbox.y0));
    }
    else if (270 == rotation) {
        ctm = fz_concat(ctm, fz_translate(-mbox.x1, -mbox.y1));
    }
    else {
        ctm = fz_concat(ctm, fz_translate(-mbox.x0, -mbox.y1));
    }

    ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
    ctm = fz_concat(ctm, fz_rotate(rotation));

    m.a = ctm.a;
    m.b = ctm.b;
    m.c = ctm.c;
    m.d = ctm.d;
    m.e = ctm.e;
    m.f = ctm.f;
    return m;
}

static int _pdf_doc_text_length(struct _pdf_doc *self, int pageno)
{
    fz_display_list *list;
    fz_text_span *text, *span;
    fz_device *tdev;
    int length = 0;

    list = _pdf_doc_get_list(self, pageno);
    text = fz_new_text_span();
    tdev = fz_new_text_device(text);
    fz_execute_display_list(
        list, tdev, fz_identity, fz_infinite_bbox);

    for (span = text; span; span = span->next) {
        length += span->len;
        if (!span->eol && span->next)
            continue;

        /* End of line ? */
        length += 1;
    }

    fz_free_device(tdev);
    fz_free_text_span(text);

    return length;
}

static void _pdf_doc_extract_text(
    struct _pdf_doc *self, int pageno, char *tbuf, mume_rect_t *rbuf)
{
    fz_display_list *list;
    fz_text_span *text, *span;
    fz_device *tdev;
    int i;

    list = _pdf_doc_get_list(self, pageno);
    text = fz_new_text_span();
    tdev = fz_new_text_device(text);
    fz_execute_display_list(
        list, tdev, fz_identity, fz_infinite_bbox);

    for (span = text; span; span = span->next) {
        for (i = 0; i < span->len; i++) {
            *tbuf = span->text[i].c;

            if (*tbuf < 32)
                *tbuf = '?';

            tbuf++;
            *rbuf++ = _fz_bbox_to_mume_rect(span->text[i].bbox);
        }

        if (!span->eol && span->next)
            continue;

        *tbuf++ = '\n';
        *rbuf++ = mume_rect_empty;
    }

    fz_free_device(tdev);
    fz_free_text_span(text);
}

static void _pdf_doc_render_page(
    struct _pdf_doc *self, cairo_t *cr, int x, int y,
    int pageno, mume_matrix_t ctm, mume_rect_t rect)
{
    /* TODO: implement a fz_device to rendering directly to cairo. */
    fz_colorspace *colorspace;
    fz_bbox bbox;
    fz_device *idev;
    fz_pixmap *pixmap;
    fz_display_list *list;
    cairo_format_t format;
    cairo_surface_t *surface;
    int stride;

#ifdef _WIN32
    colorspace = fz_device_bgr;
#else
    colorspace = fz_device_rgb;
#endif

    list = _pdf_doc_get_list(self, pageno);
    bbox = _mume_rect_to_fz_bbox(rect);
    pixmap = fz_new_pixmap_with_rect(colorspace, bbox);
    if (NULL == pixmap) {
        mume_error(("fz_new_pixmap_with_rect(%d, %d, %d, %d)\n",
                    bbox.x0, bbox.y0, bbox.x1, bbox.y1));
        return;
    }

    fz_clear_pixmap_with_color(pixmap, 255);
    idev = fz_new_draw_device(self->glyph_cache, pixmap);
    fz_execute_display_list(
        list, idev, _mume_matrix_to_fz_matrix(ctm), bbox);
    fz_free_device(idev);

    /* Create cairo surface. */
    format = CAIRO_FORMAT_ARGB32;
    stride = cairo_format_stride_for_width(format, pixmap->w);
    surface = cairo_image_surface_create_for_data(
        pixmap->samples, format, pixmap->w, pixmap->h, stride);

    if (surface) {
        cairo_set_source_surface(cr, surface, x, y);
        cairo_rectangle(cr, x, y, rect.width, rect.height);
        cairo_fill(cr);
        cairo_surface_destroy(surface);
    }

    fz_drop_pixmap(pixmap);
}

static mume_tocitem_t* _pdf_doc_get_toc_tree(struct _pdf_doc *self)
{
    pdf_outline *outline;
    mume_tocitem_t *item = NULL;

    outline = pdf_load_outline(self->xref);
    if (outline) {
        item = _pdf_doc_build_toc_tree(self, NULL, outline);
        pdf_free_outline(outline);
    }

    return item;
}

static mume_doclink_t* _pdf_doc_get_page_links(
    struct _pdf_doc *self, int pageno)
{
    pdf_page *page;
    pdf_link *link;
    mume_rect_t sr, dr;
    int dp;
    const char *ds;
    mume_doclink_t *first = NULL;
    mume_doclink_t *node = NULL;

    page = _pdf_doc_get_page(self, pageno);
    for (link = page->links; link; link = link->next) {
        sr = _fz_rect_to_mume_rect(link->rect);
        dr = _pdf_doc_get_link_dest_rect(self, link);
        dp = _pdf_doc_find_page_no(self, link->dest);
        ds = _pdf_doc_get_link_dest_value(self, link);
        node = mume_doclink_create(node, sr, dr, dp, ds);

        if (NULL == first)
            first = node;
    }

    return first;
}

const void* mume_pdf_doc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_docdoc_meta_class(),
        "pdf_doc",
        _pdf_doc_super_class(),
        sizeof(struct _pdf_doc),
        MUME_PROP_END,
        _mume_ctor, _pdf_doc_ctor,
        _mume_dtor, _pdf_doc_dtor,
        _mume_docdoc_load,
        _pdf_doc_load,
        _mume_docdoc_title,
        _pdf_doc_title,
        _mume_docdoc_count_pages,
        _pdf_doc_count_pages,
        _mume_docdoc_get_mediabox,
        _pdf_doc_get_mediabox,
        _mume_docdoc_get_matrix,
        _pdf_doc_get_matrix,
        _mume_docdoc_text_length,
        _pdf_doc_text_length,
        _mume_docdoc_extract_text,
        _pdf_doc_extract_text,
        _mume_docdoc_render_page,
        _pdf_doc_render_page,
        _mume_docdoc_get_toc_tree,
        _pdf_doc_get_toc_tree,
        _mume_docdoc_get_page_links,
        _pdf_doc_get_page_links,
        MUME_FUNC_END);
}
