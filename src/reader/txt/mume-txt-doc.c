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
#include "mume-txt-doc.h"

#define _TXT_DOC_PAGE_LINE_COUNT 50

/* Default page size. */
#define _TXT_DOC_PAGE_WIDTH 612
#define _TXT_DOC_PAGE_HEIGHT 792
#define _TXT_DOC_PAGE_WIDTH_INC 100
#define _TXT_DOC_PAGE_HEIGHT_INC 100

/* Blank region in each page. */
#define _TXT_DOC_LEFT_BLANK 50
#define _TXT_DOC_RIGHT_BLANK 50
#define _TXT_DOC_TOP_BLANK 50
#define _TXT_DOC_BOTTOM_BLANK 50
#define _TXT_DOC_HBLANK (_TXT_DOC_LEFT_BLANK + _TXT_DOC_RIGHT_BLANK)
#define _TXT_DOC_VBLANK (_TXT_DOC_TOP_BLANK + _TXT_DOC_BOTTOM_BLANK)

#define _txt_doc_super_class mume_docdoc_class

struct _line_info {
    size_t offset;
    size_t length;
    char *text;
};

struct _txt_doc {
    const char _[MUME_SIZEOF_DOCDOC];
    mume_stream_t *stm;
    mume_vector_t *lines;
    mume_rect_t *media_boxes;     /* Media boxes buffer. */
};

static void _line_info_destruct(void *obj, void *p)
{
    struct _line_info *line = obj;
    free(line->text);
}

static void _txt_doc_reset(struct _txt_doc *self)
{
    self->stm = NULL;
    self->lines = NULL;
    self->media_boxes = NULL;
}

static void _txt_doc_clear(struct _txt_doc *self)
{
    mume_stream_close(self->stm);
    mume_vector_delete(self->lines);
    free(self->media_boxes);

    _txt_doc_reset(self);
}

static int _txt_doc_line_count(const struct _txt_doc *self)
{
    if (self->lines)
        return mume_vector_size(self->lines);

    return 0;
}

static void _txt_doc_page_line_range(
    const struct _txt_doc *self, int pageno, int *begin, int *end)
{
    if (begin)
        *begin = pageno * _TXT_DOC_PAGE_LINE_COUNT;

    if (end) {
        *end = (pageno + 1) * _TXT_DOC_PAGE_LINE_COUNT;
        if (*end > _txt_doc_line_count(self))
            *end = _txt_doc_line_count(self);
    }
}

static const char* _txt_doc_get_line_text(
    const struct _txt_doc *self, int lineno, int *len)
{
    struct _line_info *line;

    assert(lineno < _txt_doc_line_count(self));
    line = mume_vector_at(self->lines, lineno);

    if (len)
        *len = line->length;

    if (line->text)
        return line->text;

    line->text = malloc_abort(line->length);
    mume_stream_seek(self->stm, line->offset);
    mume_stream_read(self->stm, line->text, line->length);

    return line->text;
}

static mume_resobj_charfmt_t* _txt_doc_get_charfmt(
    const struct _txt_doc *self)
{
    mume_resobj_charfmt_t *cf = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "docview", "txtdoc"),
        _mume_typeof_resobj_charfmt());

    if (NULL == cf)
        mume_warning(("Txt doc get charfmt failed\n"));

    return cf;
}

static void* _txt_doc_ctor(
    struct _txt_doc *self, int mode, va_list *app)
{
    if (!_mume_ctor(_txt_doc_super_class(), self, mode, app))
        return NULL;

    _txt_doc_reset(self);
    return self;
}

static void* _txt_doc_dtor(struct _txt_doc *self)
{
    _txt_doc_clear(self);
    return _mume_dtor(_txt_doc_super_class(), self);
}

static int _txt_doc_load(struct _txt_doc *self, mume_stream_t *stm)
{
    char buf[4096];
    size_t i, count, offset;
    struct _line_info *line;

    self->stm = stm;
    mume_stream_reference(stm);
    self->lines = mume_vector_new(
        sizeof(struct _line_info), _line_info_destruct, NULL);

    /* Get line info. */
    i = count = offset = 0;
    line = mume_vector_push_back(self->lines);
    line->offset = offset;
    line->length = 0;
    line->text = NULL;

    while ((count = mume_stream_read(stm, buf, sizeof(buf)))) {
        for (i = 0; i < count; ++i) {
            if (buf[i] == '\n' || buf[i] == '\r') {
                /* The line length not include '\n'. */
                line->length = offset + i - line->offset;
                line = mume_vector_push_back(self->lines);
                if (buf[i] == '\r' && i < count && buf[i + 1] == '\n')
                    ++i;

                line->offset = offset + i + 1;
                line->length = count - i - 1;
                line->text = NULL;
            }
        }

        offset += count;
    }

    if (0 == line->length)
        mume_vector_pop_back(self->lines);

    count = mume_docdoc_count_pages(self);
    self->media_boxes = malloc_abort(sizeof(mume_rect_t) * count);
    for (i = 0; i < count; ++i)
        self->media_boxes[i] = mume_rect_infinite;

    return 1;
}

static const char* _txt_doc_title(struct _txt_doc *self)
{
    return NULL;
}

static int _txt_doc_count_pages(const struct _txt_doc *self)
{
    if (self->lines) {
        int count = mume_vector_size(self->lines);
        if (count % _TXT_DOC_PAGE_LINE_COUNT)
            return count / _TXT_DOC_PAGE_LINE_COUNT + 1;

        return count / _TXT_DOC_PAGE_LINE_COUNT;
    }

    return 0;
}

static mume_rect_t _txt_doc_get_mediabox(
    struct _txt_doc *self, int pageno)
{
    int i, e, n;
    const char *t;
    mume_rect_t rr;
    mume_resobj_charfmt_t *cf;
    cairo_font_extents_t font_exts;

    if (!mume_rect_is_infinite(self->media_boxes[pageno]))
        return self->media_boxes[pageno];

    rr.x = 0;
    rr.y = 0;
    rr.width = _TXT_DOC_PAGE_WIDTH;
    rr.height = _TXT_DOC_PAGE_HEIGHT;

    /* Calculate the widest line. */
    cf = _txt_doc_get_charfmt(self);
    if (NULL == cf)
        return rr;

    font_exts = mume_charfmt_font_extents(cf);
    _txt_doc_page_line_range(self, pageno, &i, &e);

    rr.height = font_exts.height * (e - i) + _TXT_DOC_VBLANK;
    rr.height = MAX(rr.height, _TXT_DOC_PAGE_HEIGHT);

    for (; i < e; ++i) {
        if ((t = _txt_doc_get_line_text(self, i, &n))) {
            mume_point_t pt;
            pt = mume_charfmt_text_extents(cf, t, n);
            rr.width = MAX(rr.width, pt.x + _TXT_DOC_HBLANK);
        }
    }

    if (rr.width % _TXT_DOC_PAGE_WIDTH_INC) {
        rr.width -= rr.width % _TXT_DOC_PAGE_WIDTH_INC;
        rr.width += _TXT_DOC_PAGE_WIDTH_INC;
    }

    if (rr.height % _TXT_DOC_PAGE_HEIGHT_INC) {
        rr.height -= rr.height % _TXT_DOC_PAGE_HEIGHT_INC;
        rr.height += _TXT_DOC_PAGE_HEIGHT_INC;
    }

    self->media_boxes[pageno] = rr;
    return rr;
}

static mume_matrix_t _txt_doc_get_matrix(
    struct _txt_doc *self, int pageno, float zoom, int rotate)
{
    mume_matrix_t m = mume_matrix_identity;
    mume_rect_t r = mume_docdoc_get_mediabox(self, pageno);
    int rotation = rotate % 360;

    if (rotation < 0)
        rotation = rotation + 360;

    if (90 == rotation) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(0, -r.height));
    }
    else if (180 == rotation) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-r.width, -r.height));
    }
    else if (270 == rotation) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-r.width, 0));
    }

    m = mume_matrix_concat(m, mume_matrix_scale(zoom, zoom));
    m = mume_matrix_concat(m, mume_matrix_rotate(rotation));

    return m;
}

static int _txt_doc_text_length(struct _txt_doc *self, int pageno)
{
    int i, e, n;
    const char *t;
    int length = 0;

    _txt_doc_page_line_range(self, pageno, &i, &e);
    if (e > i) {
        /* Add '\n' to each middle line. */
        length += e - i - 1;

        for (; i < e; ++i) {
            if ((t = _txt_doc_get_line_text(self, i, &n)))
                length += n;
        }
    }

    return length;
}

static void _txt_doc_extract_text(
    struct _txt_doc *self, int pageno, char *tbuf, mume_rect_t *rbuf)
{
    int i, j, e, n;
    double dx, dy;
    const char *t;
    const mume_rect_t *r;
    void *tl = mume_text_layout();
    mume_resobj_charfmt_t *cf;
    cairo_font_extents_t font_extents;

    cf = _txt_doc_get_charfmt(self);
    if (!mume_resobj_charfmt_is_valid(cf))
        return;

    font_extents = mume_charfmt_font_extents(cf);

    dx = _TXT_DOC_LEFT_BLANK;
    dy = _TXT_DOC_TOP_BLANK;
    _txt_doc_page_line_range(self, pageno, &i, &e);
    for (; i < e; ++i) {
        if ((t = _txt_doc_get_line_text(self, i, &n))) {
            mume_text_layout_reset(tl);
            mume_text_layout_add_text(tl, cf->p->p, t, n);
            mume_text_layout_perform(
                tl, cf->size, NULL, NULL, MUME_TLF_SINGLELINE);

            r = mume_text_layout_get_rects(tl);
            for (j = 0; j < n; ++j) {
                *tbuf = t[j];
                *rbuf = r[j];
                rbuf->x += dx;
                rbuf->y += dy;
                ++tbuf;
                ++rbuf;
            }
        }

        if (i != e - 1) {
            /* Add '\n' to each middle line. */
            *tbuf++ = '\n';
            *rbuf++ = mume_rect_empty;

            dy += font_extents.height;
        }
    }
}

static void _txt_doc_render_page(
    struct _txt_doc *self, cairo_t *cr, int x, int y,
    int pageno, mume_matrix_t ctm, mume_rect_t rect)
{
    const char* t;
    int i, e, n;
    double lx, ly;
    mume_rect_t r;
    mume_resobj_charfmt_t *cf;
    cairo_font_extents_t font_exts;
    cairo_t *mcr;
    cairo_matrix_t matrix;
    cairo_surface_t *img;

    cf = _txt_doc_get_charfmt(self);
    if (!mume_resobj_charfmt_is_valid(cf))
        return;

    img = cairo_image_surface_create(
        CAIRO_FORMAT_RGB24, rect.width, rect.height);

    if (NULL == img) {
        mume_error(("cairo_image_surface_create(%d, %d)\n",
                    rect.width, rect.height));
        return;
    }

    mcr = cairo_create(img);
    if (NULL == mcr) {
        mume_error(("cairo_create failed\n"));
        cairo_surface_destroy(img);
        return;
    }

    matrix = mume_matrix_to_cairo(ctm);
    cairo_translate(mcr, -rect.x, -rect.y);
    cairo_transform(mcr, &matrix);
    cairo_set_source_rgb(mcr, 1, 1, 1);
    cairo_paint(mcr);

    font_exts = mume_charfmt_font_extents(cf);
    _txt_doc_page_line_range(self, pageno, &i, &e);

    lx = _TXT_DOC_LEFT_BLANK;
    ly = _TXT_DOC_TOP_BLANK;
    for (; i < e; ++i) {
        if ((t = _txt_doc_get_line_text(self, i, &n))) {
#define TEXT_FORMAT (MUME_TLF_DRAWTEXT | \
                     MUME_TLF_SINGLELINE | MUME_TLF_NOCLIP)

            r = mume_rect_make(lx, ly, 0, font_exts.height);
            mume_charfmt_draw_text(mcr, cf, TEXT_FORMAT, t, n, &r);
            ly += font_exts.height;
#undef TEXT_FORMAT
        }
    }

    cairo_save(cr);
    cairo_set_source_surface(cr, img, x, y);
    cairo_rectangle(cr, x, y, rect.width, rect.height);
    cairo_fill(cr);
    cairo_restore(cr);

    cairo_surface_destroy(img);
    cairo_destroy(mcr);
}

const void* mume_txt_doc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_docdoc_meta_class(),
        "txt_doc",
        _txt_doc_super_class(),
        sizeof(struct _txt_doc),
        MUME_PROP_END,
        _mume_ctor, _txt_doc_ctor,
        _mume_dtor, _txt_doc_dtor,
        _mume_docdoc_load,
        _txt_doc_load,
        _mume_docdoc_title,
        _txt_doc_title,
        _mume_docdoc_count_pages,
        _txt_doc_count_pages,
        _mume_docdoc_get_mediabox,
        _txt_doc_get_mediabox,
        _mume_docdoc_get_matrix,
        _txt_doc_get_matrix,
        _mume_docdoc_text_length,
        _txt_doc_text_length,
        _mume_docdoc_extract_text,
        _txt_doc_extract_text,
        _mume_docdoc_render_page,
        _txt_doc_render_page,
        MUME_FUNC_END);
}
