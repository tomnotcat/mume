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
#include "mume-docview.h"
#include "mume-docdoc.h"
#include MUME_CTYPE_H
#include MUME_FLOAT_H

#define _DOCVIEW_ZOOM_IN 1.2f
#define _DOCVIEW_ZOOM_OUT (1.0f / _DOCVIEW_ZOOM_IN)
#define _DOCVIEW_ZOOM_MIN 0.125
#define _DOCVIEW_ZOOM_MAX 64.0

#define _docview_super_class mume_scrollview_class
#define _docview_super_meta_class mume_scrollview_meta_class

#define _docview_is_word_char(c) isalnum(c)

enum _docview_flags_e {
    _DOCVIEW_FLAG_OVERTEXT,
    _DOCVIEW_FLAG_OVERLINK,
    _DOCVIEW_FLAG_CLKLINK,
    _DOCVIEW_FLAG_SELCHAR,
    _DOCVIEW_FLAG_SELWORD,
    _DOCVIEW_FLAG_WORD_BOUND
};

typedef struct _page_text {
    char *texts;
    mume_rect_t *coords;
    int length;
} _page_text_t;

struct _docview {
    const char _[MUME_SIZEOF_SCROLLVIEW];
    void *doc;
    mume_rect_t *page_rects;  /* Page in screen (without border). */
    _page_text_t *page_texts; /* Extracted texts in each page. */
    mume_doclink_t **page_links;   /* Extracted links in each page. */
    mume_rect_t page_border;  /* Border size around each page. */
    int view_width;           /* View width (without page border). */
    int view_height;          /* View height (without page border). */
    int flags;
    float zoom;
    int rotate;
    int first_visible;        /* First visible page. */
    int sel_start_p;
    int sel_start_i;
    int sel_end_p;
    int sel_end_i;
};

struct _docview_class {
    const char _[MUME_SIZEOF_SCROLLVIEW_CLASS];
};

struct _docview_theme {
    mume_resobj_brush_t pagebg;
    mume_resobj_brush_t selbg;
    mume_resobj_brush_t bkgnd;
};

MUME_STATIC_ASSERT(sizeof(struct _docview) == MUME_SIZEOF_DOCVIEW);
MUME_STATIC_ASSERT(sizeof(struct _docview_class) ==
                   MUME_SIZEOF_DOCVIEW_CLASS);

static void _docview_reset(struct _docview *self)
{
    self->doc = NULL;
    self->page_rects = NULL;
    self->page_texts = NULL;
    self->page_links = NULL;
    self->page_border = mume_rect_make(4, 4, 4, 4);
    self->view_width = 0;
    self->view_height = 0;
    self->flags = 0;
    self->zoom = 1.0;
    self->rotate = 0;
    self->first_visible = -1;
    self->sel_start_p = 0;
    self->sel_start_i = 0;
    self->sel_end_p = 0;
    self->sel_end_i = 0;
}

static void _docview_clear(struct _docview *self)
{
    int i, c = mume_docview_count_pages(self);

    free(self->page_rects);

    if (self->page_texts) {
        for (i = 0; i < c; ++i) {
            free(self->page_texts[i].texts);
            free(self->page_texts[i].coords);
        }

        free(self->page_texts);
    }

    if (self->page_links) {
        for (i = 0; i < c; ++i)
            mume_doclink_destroy(self->page_links[i]);

        free(self->page_links);
    }

    if (self->doc)
        mume_refobj_release(self->doc);

    _docview_reset(self);
}

static void _docview_update_page_rects(struct _docview *self)
{
    int i, c;
    mume_rect_t *rect;
    mume_matrix_t ctm;

    self->view_width = 0;
    self->view_height = 0;
    c = mume_docview_count_pages(self);
    for (i = 0; i < c; ++i) {
        rect = self->page_rects + i;

        ctm = mume_docdoc_get_matrix(
            self->doc, i, self->zoom, self->rotate);
        *rect = mume_docdoc_get_mediabox(self->doc, i);
        *rect = mume_rect_transform(*rect, ctm);

        rect->y += self->view_height;
        if (rect->width > self->view_width)
            self->view_width = rect->width;

        self->view_height = rect->y + rect->height;
    }
}

static void _docview_get_view_size(
    const struct _docview *self, int *width, int *height)
{
    int c = mume_docview_count_pages(self);

    if (width) {
        *width = self->view_width;
        if (c > 0)
            *width += self->page_border.x + self->page_border.width;
    }

    if (height) {
        *height = self->view_height;
        *height += (self->page_border.y + self->page_border.height) * c;
    }
}

static void _docview_update_scroll_size(struct _docview *self)
{
    int width, height;
    _docview_get_view_size(self, &width, &height);
    mume_scrollview_set_size(self, width, height);
}

static mume_rect_t _docview_get_page_rect(
    const struct _docview *self, int pageno, int border)
{
    mume_rect_t r = self->page_rects[pageno];
    mume_rect_t b = self->page_border;
    int cw, vw, mw;

    mume_scrollview_get_client(self, NULL, NULL, &cw, NULL);
    _docview_get_view_size(self, &vw, NULL);

    mw = MAX(cw, vw);
    mw -= b.x + b.width;

    if (mw > r.width)
        r = mume_rect_translate(r, (mw - r.width) / 2, 0);

    r = mume_rect_translate(r, b.x, (b.y + b.height) * pageno + b.y);
    if (border)
        r = mume_rect_inflate(r, b.x, b.y, b.width, b.height);

    return r;
}

static _page_text_t* _docview_get_page_text(
    const struct _docview *self, int pageno)
{
    _page_text_t *r = self->page_texts + pageno;
    if (-1 == r->length) {
        r->length = mume_docdoc_text_length(self->doc, pageno);
        if (r->length > 0) {
            r->texts = malloc_abort(r->length * sizeof(*(r->texts)));
            r->coords = malloc_abort(r->length * sizeof(*(r->coords)));

            mume_docdoc_extract_text(
                self->doc, pageno, r->texts, r->coords);
        }
    }

    return r;
}

static mume_doclink_t* _docview_get_page_links(
    const struct _docview *self, int pageno)
{
    if (NULL == self->page_links[pageno]) {
        self->page_links[pageno] =
                mume_docdoc_get_page_links(self->doc, pageno);
    }

    return self->page_links[pageno];
}

static int _docview_find_closest_glyph(
    struct _docview *self, int pageno, int x, int y, int *inside)
{
    _page_text_t *text;
    mume_rect_t rect;
    mume_matrix_t ctm;
    mume_point_t pt;
    int i, result = -1;
    int dist, maxdist = -1;

    ctm = mume_docdoc_get_matrix(
        self->doc, pageno, self->zoom, self->rotate);
    ctm = mume_matrix_invert(ctm);

    pt.x = x;
    pt.y = y;
    pt = mume_point_transform(pt, ctm);

    text = _docview_get_page_text(self, pageno);
    for (i = 0; i < text->length; ++i) {
        rect = text->coords[i];
        dist = hypot(pt.x - rect.x - rect.width / 2,
                     pt.y - rect.y - rect.height / 2);
        if (maxdist < 0 || dist < maxdist) {
            maxdist = dist;
            result = i;
        }
    }

    if (inside) {
        if (result != -1) {
            rect = text->coords[result];
            *inside = pt.x >= rect.x && pt.x < (rect.x + rect.width) &&
                      pt.y >= rect.y && pt.y < (rect.y + rect.height);
        }
        else {
            *inside = 0;
        }
    }

    return result;
}

static int _docview_find_closest_text(
    struct _docview *self, int x, int y, int *index)
{
    int pageno = mume_docview_page_from(self, y);
    if (NULL == index)
        return pageno;

    mume_docview_client_to_page(self, pageno, &x, &y);
    *index = _docview_find_closest_glyph(self, pageno, x, y, NULL);
    if (-1 != *index) {
        _page_text_t *text;
        mume_matrix_t ctm;
        mume_rect_t rect;

        text = _docview_get_page_text(self, pageno);
        ctm = mume_docdoc_get_matrix(
            self->doc, pageno, self->zoom, self->rotate);
        rect = mume_rect_transform(text->coords[(*index)], ctm);

        /* When over the right half of a glyph, return the index
         * of the next glyph. */
        if (x > rect.x + rect.width / 2)
            ++(*index);
    }

    return pageno;
}

static int _docview_is_over_text(struct _docview *self, int x, int y)
{
    int result, pageno;

    pageno = mume_docview_page_from(self, y);
    mume_docview_client_to_page(self, pageno, &x, &y);
    _docview_find_closest_glyph(self, pageno, x, y, &result);

    return result;
}

static mume_doclink_t* _docview_page_link_from(
    struct _docview *self, int x, int y)
{
    int pageno;
    mume_matrix_t ctm;
    mume_rect_t rect;
    mume_doclink_t *link;

    pageno = mume_docview_page_from(self, y);
    mume_docview_client_to_page(self, pageno, &x, &y);
    link = _docview_get_page_links(self, pageno);
    ctm = mume_docdoc_get_matrix(
        self->doc, pageno, self->zoom, self->rotate);

    while (link) {
        rect = mume_rect_transform(link->src_rect, ctm);
        if (mume_rect_inside(rect, x, y))
            return link;

        link = link->next;
    }

    return NULL;
}

static void _docview_get_sel_range(
    struct _docview *self, int *head_p, int *head_i,
    int *tail_p, int *tail_i)
{
    if (self->sel_end_p > self->sel_start_p) {
        *head_p = self->sel_start_p;
        *head_i = self->sel_start_i;
        *tail_p = self->sel_end_p;
        *tail_i = self->sel_end_i;
    }
    else if (self->sel_end_p < self->sel_start_p) {
        *head_p = self->sel_end_p;
        *head_i = self->sel_end_i;
        *tail_p = self->sel_start_p;
        *tail_i = self->sel_start_i;
    }
    else {
        *head_p = self->sel_start_p;
        *tail_p = self->sel_end_p;
        *head_i = MIN(self->sel_start_i, self->sel_end_i);
        *tail_i = MAX(self->sel_start_i, self->sel_end_i);
    }

    if (mume_test_flag(self->flags, _DOCVIEW_FLAG_WORD_BOUND)) {
        /* Adjust the selection range to word boundly. */
        _page_text_t *text;
        int i;

        text = _docview_get_page_text(self, *head_p);
        for (i = *head_i; i > 0; --i) {
            if (!_docview_is_word_char(text->texts[i - 1]))
                break;
        }

        *head_i = i;

        if (*tail_p != *head_p)
            text = _docview_get_page_text(self, *tail_p);

        for (i = *tail_i; i < text->length; ++i) {
            if (!_docview_is_word_char(text->texts[i]))
                break;
        }

        *tail_i = i;
    }
}

static void _docview_get_page_sel(
    struct _docview *self, int pageno, int *begin, int *end)
{
    _page_text_t *text;
    int head_p, head_i, tail_p, tail_i;

    text = _docview_get_page_text(self, pageno);
    _docview_get_sel_range(self, &head_p, &head_i, &tail_p, &tail_i);

    if (pageno == head_p) {
        if (head_p != tail_p) {
            *begin = head_i;
            *end = text->length;
        }
        else {
            *begin = head_i;
            *end = tail_i;
        }
    }
    else if (pageno == tail_p) {
        *begin = 0;
        *end = tail_i;
    }
    else if (pageno > head_p && pageno < tail_p)
    {
        *begin = 0;
        *end = text->length;
    }
    else {
        *begin = 0;
        *end = 0;
    }
}

static cairo_region_t* _docview_create_sel_region(
    struct _docview *self, int pageno, int sel_begin, int sel_end)
{
    _page_text_t *text;
    mume_rect_t ra, rc;
    cairo_region_t *rgn;
    mume_matrix_t ctm;
    int i;

    text = _docview_get_page_text(self, pageno);
    rgn = cairo_region_create();
    ctm = mume_docdoc_get_matrix(
        self->doc, pageno, self->zoom, self->rotate);

    ra = mume_rect_empty;
    for (i = sel_begin; i < sel_end; ++i) {
        rc = text->coords[i];

        if (mume_rect_is_empty(ra)) {
            ra = rc;
        }
        else if (text->texts[i] >= 32) {
            if (rc.width > 0 && rc.height > 0) {
                ra.x = MIN(ra.x, rc.x);
                ra.y = MIN(ra.y, rc.y);
                ra.width = MAX(ra.x + ra.width, rc.x + rc.width) - ra.x;
                ra.height = MAX(ra.height, rc.height);
            }
        }

        if (text->texts[i] != '\n' && i + 1 < sel_end)
            continue;

        ra = mume_rect_transform(ra, ctm);
        cairo_region_union_rectangle(rgn, &ra);
        ra = mume_rect_empty;
    }

    return rgn;
}

static void _docview_select_at(
    struct _docview *self, int pageno, int index, int word)
{
    int wb = mume_test_flag(self->flags, _DOCVIEW_FLAG_WORD_BOUND);

    if (self->sel_start_p != pageno || self->sel_start_i != index ||
        self->sel_end_p != pageno || self->sel_end_i != index ||
        (wb && !word) || (!wb && word))
    {
        self->sel_start_p = pageno;
        self->sel_start_i = index;
        self->sel_end_p = pageno;
        self->sel_end_i = index;

        if (word)
            mume_add_flag(self->flags, _DOCVIEW_FLAG_WORD_BOUND);
        else
            mume_remove_flag(self->flags, _DOCVIEW_FLAG_WORD_BOUND);

        mume_invalidate_region(self, NULL);
    }
}

static int _page_selection_compare(
    int page1, int index1, int page2, int index2)
{
    if (page1 > page2)
        return 1;

    if (page1 < page2)
        return -1;

    return index1 - index2;
}

static void _docview_select_to(
    struct _docview *self, int pageno, int index)
{
    if (self->sel_end_p != pageno || self->sel_end_i != index) {
        cairo_region_t *rgn0, *rgn1;
        mume_rect_t rect;
        int i, last, height, dx, dy, sx, sy;
        int begin, end, cmp, enlarged;

        /* Calculate the invalid region. */
        i = MIN(self->sel_end_p, pageno);
        last = MAX(self->sel_end_p, pageno);
        height = mume_window_height(self);
        mume_scrollview_get_scroll(self, &sx, &sy);

        if (mume_docview_first_visible(self) > i)
            i = mume_docview_first_visible(self);

        cmp = _page_selection_compare(
            self->sel_end_p, self->sel_end_i,
            self->sel_start_p, self->sel_start_i);

        if (cmp > 0) {
            enlarged = (_page_selection_compare(
                pageno, index, self->sel_end_p, self->sel_end_i) > 0);
        }
        else if (cmp < 0) {
            enlarged = (_page_selection_compare(
                pageno, index, self->sel_end_p, self->sel_end_i) < 0);
        }
        else {
            enlarged = 1;
        }

        if (enlarged) {
            self->sel_end_p = pageno;
            self->sel_end_i = index;
        }

        rgn0 = cairo_region_create();
        for (; i <= last; ++i) {
            rect = _docview_get_page_rect(self, i, 1);
            rect.x -= sx;
            rect.y -= sy;

            if (rect.y >= height)
                break;

            dx = dy = 0;
            mume_docview_page_to_client(self, i, &dx, &dy);

            _docview_get_page_sel(self, i, &begin, &end);
            rgn1 = _docview_create_sel_region(self, i, begin, end);
            cairo_region_translate(rgn1, dx, dy);
            cairo_region_union(rgn0, rgn1);
            cairo_region_destroy(rgn1);
        }

        self->sel_end_p = pageno;
        self->sel_end_i = index;

        mume_invalidate_region(self, rgn0);
        cairo_region_destroy(rgn0);
    }
}

static char* _docview_copy_selection(struct _docview *self)
{
    _page_text_t *text;
    int head_p, head_i, tail_p, tail_i;
    mume_vector_t *str;
    int i, j, e;
    char *result = NULL;

    _docview_get_sel_range(
        self, &head_p, &head_i, &tail_p, &tail_i);

    str = mume_vector_new(sizeof(char), NULL, NULL);

    for (i = head_p; i <= tail_p; ++i) {
        text = _docview_get_page_text(self, i);
        j = 0;
        e = text->length;

        if (i == head_p)
            j = head_i;

        if (i == tail_p)
            e = tail_i;

        memcpy(mume_vector_append(str, e - j),
               text->texts + j, e - j);
    }

    if (mume_vector_size(str)) {
        *(char*)mume_vector_push_back(str) = '\0';
        result = mume_vector_detach(str);
    }

    mume_vector_delete(str);
    return result;
}

static void* _docview_ctor(
    struct _docview *self, int mode, va_list *app)
{
    if (!_mume_ctor(_docview_super_class(), self, mode, app))
        return NULL;

    mume_scrollview_set_line(self, 64, 64);

    _docview_reset(self);
    return self;
}

static void* _docview_dtor(struct _docview *self)
{
    _docview_clear(self);
    return _mume_dtor(_docview_super_class(), self);
}

static void _docview_handle_key_down(
    struct _docview *self, int x, int y, int state, int keysym)
{
    switch (keysym) {
    case MUME_KEY_LEFT:
        if (state & MUME_MOD_CONTROL)
            mume_docview_rotate_by(self, -90);
        break;

    case MUME_KEY_RIGHT:
        if (state & MUME_MOD_CONTROL)
            mume_docview_rotate_by(self, 90);
        break;
    }

    _mume_window_handle_key_down(
        _docview_super_class(), self, x, y, state, keysym);
}

static void _docview_handle_button_down(
    struct _docview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        if (_docview_page_link_from(self, x, y))
            mume_add_flag(self->flags, _DOCVIEW_FLAG_CLKLINK);

        if (mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT)) {
            int p, i;
            mume_add_flag(self->flags, _DOCVIEW_FLAG_SELCHAR);
            p = _docview_find_closest_text(self, x, y, &i);
            _docview_select_at(self, p, i, 0);
        }
        else {
            _docview_select_at(self, 0, 0, 0);
        }
        break;

    case MUME_BUTTON_WHEELUP:
        if (state & MUME_MOD_CONTROL) {
            mume_docview_zoom_by(self, _DOCVIEW_ZOOM_IN, x, y);
        }
        break;

    case MUME_BUTTON_WHEELDOWN:
        if (state & MUME_MOD_CONTROL) {
            mume_docview_zoom_by(self, _DOCVIEW_ZOOM_OUT, x, y);
        }
        break;
    }

    _mume_window_handle_button_down(
        _docview_super_class(), self, x, y, state, button);
}

static void _docview_handle_button_up(
    struct _docview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        if (mume_test_flag(self->flags, _DOCVIEW_FLAG_CLKLINK)) {
            mume_doclink_t *link = _docview_page_link_from(self, x, y);
            if (link) {
                mume_debug(("goto: %s\n", link->dest_value));
            }

            mume_remove_flag(self->flags, _DOCVIEW_FLAG_CLKLINK);
        }

        mume_remove_flag(self->flags, _DOCVIEW_FLAG_SELCHAR);
        mume_remove_flag(self->flags, _DOCVIEW_FLAG_SELWORD);
        break;
    }
}

static void _docview_handle_button_dblclk(
    struct _docview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        if (mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT)) {
            int p, i;
            mume_remove_flag(self->flags, _DOCVIEW_FLAG_SELCHAR);
            mume_add_flag(self->flags, _DOCVIEW_FLAG_SELWORD);
            p = _docview_find_closest_text(self, x, y, &i);
            _docview_select_at(self, p, i, 1);
        }
        break;
    }

    _mume_window_handle_button_dblclk(
        _docview_super_class(), self, x, y, state, button);
}

static void _docview_handle_mouse_motion(
    struct _docview *self, int x, int y, int state)
{
    if (!mume_test_flag(self->flags, _DOCVIEW_FLAG_SELCHAR) &&
        !mume_test_flag(self->flags, _DOCVIEW_FLAG_SELWORD))
    {
        if (mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERLINK)) {
            if (!_docview_page_link_from(self, x, y)) {
                mume_remove_flag(self->flags, _DOCVIEW_FLAG_OVERLINK);
                mume_window_set_cursor(self, NULL);
            }
        }
        else {
            if (_docview_page_link_from(self, x, y)) {
                mume_add_flag(self->flags, _DOCVIEW_FLAG_OVERLINK);
                mume_window_set_cursor(
                    self, mume_cursor(MUME_CURSOR_HAND));
            }
        }
    }

    if (mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT)) {
        if (mume_test_flag(self->flags, _DOCVIEW_FLAG_SELCHAR)) {
            int p, i;
            p = _docview_find_closest_text(self, x, y, &i);
            _docview_select_to(self, p, i);
        }
        else if (mume_test_flag(self->flags, _DOCVIEW_FLAG_SELWORD)) {
            int p, i;
            p = _docview_find_closest_text(self, x, y, &i);
            _docview_select_to(self, p, i);
        }
        else if (!_docview_is_over_text(self, x, y)) {
            mume_remove_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT);

            if (!mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERLINK))
                mume_window_set_cursor(self, NULL);
        }
    }
    else {
        if (_docview_is_over_text(self, x, y)) {
            mume_add_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT);

            if (!mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERLINK)) {
                mume_window_set_cursor(
                    self, mume_cursor(MUME_CURSOR_IBEAM));
            }
        }
    }
}

static void _docview_handle_mouse_leave(
    struct _docview *self, int x, int y, int state, int mode, int detail)
{
    mume_remove_flag(self->flags, _DOCVIEW_FLAG_OVERLINK);

    if (mume_pointer_owner() != self &&
        mume_test_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT))
    {
        mume_remove_flag(self->flags, _DOCVIEW_FLAG_OVERTEXT);
        mume_window_set_cursor(self, NULL);
    }
}

static void _docview_handle_expose(
    struct _docview *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    struct _docview_theme *thm;
    int sx, sy, i;
    mume_rect_t rr, r0, r1, r2;
    mume_matrix_t m;
    cairo_region_t *c0, *c1;
    int sel_begin, sel_end;
    cairo_region_t *sel_rgn;

    if (count)
        return;

    thm = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "docview", "theme"),
        mume_typeof_docview_theme());

    if (NULL == thm) {
        mume_warning(("Docview Get theme failed\n"));
        return;
    }

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Docview begin paint failed\n"));
        return;
    }

    /* Invalid rect. */
    rr = mume_current_invalid_rect();

    /* Client rect. */
    mume_scrollview_get_client(
        self, &r0.x, &r0.y, &r0.width, &r0.height);

    c0 = cairo_region_create_rectangle(&r0);

    /* Pages. */
    mume_scrollview_get_scroll(self, &sx, &sy);
    count = mume_docview_count_pages(self);
    for (i = mume_docview_first_visible(self); i < count; ++i) {
        r2 = _docview_get_page_rect(self, i, 0);
        r2 = mume_rect_translate(r2, -sx, -sy);
        r1 = mume_rect_inflate(r2, 2, 2, 2, 2);

        if (r1.y >= r0.height)
            break;

        cairo_region_subtract_rectangle(c0, &r1);
        c1 = cairo_region_create_rectangle(&r1);

        /* Page border. */
        cairo_region_subtract_rectangle(c1, &r2);
        mume_cairo_region_to_path(cr, c1);
        cairo_region_destroy(c1);

        cairo_save(cr);
        cairo_clip(cr);

        mume_draw_resobj_brush(
            cr, &thm->pagebg, r1.x, r1.y, r1.width, r1.height);

        cairo_restore(cr);

        /* Page content. */
        r2 = mume_rect_intersect(r2, rr);
        x = r2.x;
        y = r2.y;
        mume_docview_client_to_page(self, i, &r2.x, &r2.y);

        m = mume_docdoc_get_matrix(
            self->doc, i, self->zoom, self->rotate);
        mume_docdoc_render_page(self->doc, cr, x, y, i, m, r2);

#ifdef DOCVIEW_DEBUG
        {
            int rlx, rly, rls = 50;
            mume_rect_t rlr;
            /* Draw reference lines. */
            rlr = _docview_get_page_rect(self, i, 0);
            rlr = mume_rect_translate(rlr, -sx, -sy);
            cairo_translate(cr, rlr.x, rlr.y);

            for (rlx = 0; rlx < rlr.width; rlx += rls) {
                cairo_move_to(cr, rlx, 0);
                cairo_line_to(cr, rlx, rlr.height);

                for (rly = 0; rly < rlr.height; rly += rls) {
                    cairo_move_to(cr, 0, rly);
                    cairo_line_to(cr, rlr.width, rly);
                }
            }

            cairo_set_source_rgb(cr, 0.5, 0, 0);
            cairo_stroke(cr);
            cairo_translate(cr, -rlr.x, -rlr.y);
        }
#endif

        /* Selection. */
        _docview_get_page_sel(self, i, &sel_begin, &sel_end);
        if (sel_end > sel_begin) {
            int tx = 0, ty = 0;
            cairo_operator_t oo;

            sel_rgn = _docview_create_sel_region(
                self, i, sel_begin, sel_end);
            mume_docview_page_to_client(self, i, &tx, &ty);
            cairo_region_translate(sel_rgn, tx, ty);

            mume_cairo_region_to_path(cr, sel_rgn);
            oo = cairo_get_operator(cr);

            cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_fill_preserve(cr);

            cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
            mume_resobj_brush_fill(cr, &thm->selbg);

            cairo_set_operator(cr, oo);

            cairo_region_destroy(sel_rgn);
        }
    }

    /* Background. */
    mume_cairo_region_to_path(cr, c0);
    cairo_region_destroy(c0);
    cairo_clip(cr);

    mume_draw_resobj_brush(
        cr, &thm->bkgnd, r0.x, r0.y, r0.width, r0.height);

    mume_window_end_paint(self, cr);
}

static void _docview_handle_command(
    struct _docview *self, void *window, int command)
{
    switch (command) {
    case MUME_COMMAND_COPY:
        {
            char *text;
            void *datarec;
            void *datasrc;

            text = _docview_copy_selection(self);
            if (NULL == text)
                break;

            datarec = mume_datarec_new(
                mume_datafmt(MUME_DATAFMT_TEXT),
                text, strlen(text) + 1);
            datasrc = mume_datasrc_new(&datarec, 1);
            mume_clipboard_set_data(mume_clipboard(), datasrc);

            mume_refobj_release(datarec);
            mume_refobj_release(datasrc);
        }
        break;
    }
}

static void _docview_handle_notify(
    struct _docview *self, void *window, int code, void *data)
{
    if (self == window && MUME_SCROLLVIEW_SCROLL == code) {
        const mume_point_t *pt = data;
        int sy;

        mume_scrollview_get_scroll(self, NULL, &sy);
        if (pt->y != sy) {
            /* Recalculate the first visible page. */
            self->first_visible = -1;
        }
    }
}

const void* mume_docview_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_docview_meta_class(),
        "docview",
        _docview_super_class(),
        sizeof(struct _docview),
        MUME_PROP_END,
        _mume_ctor, _docview_ctor,
        _mume_dtor, _docview_dtor,
        _mume_window_handle_key_down,
        _docview_handle_key_down,
        _mume_window_handle_button_down,
        _docview_handle_button_down,
        _mume_window_handle_button_up,
        _docview_handle_button_up,
        _mume_window_handle_button_dblclk,
        _docview_handle_button_dblclk,
        _mume_window_handle_mouse_motion,
        _docview_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _docview_handle_mouse_leave,
        _mume_window_handle_expose,
        _docview_handle_expose,
        _mume_window_handle_command,
        _docview_handle_command,
        _mume_window_handle_notify,
        _docview_handle_notify,
        MUME_FUNC_END);
}

const void* mume_docview_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "docview class",
        _docview_super_meta_class(),
        sizeof(struct _docview_class),
        MUME_PROP_END,
        MUME_FUNC_END);
}

void* mume_docview_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(mume_docview_class(), parent, x, y, w, h);
}

void mume_docview_set_doc(void *_self, void *doc)
{
    struct _docview *self = _self;
    int i, page_count;

    assert(mume_is_of(_self, mume_docview_class()));
    assert(!doc || mume_is_of(doc, mume_docdoc_class()));

    _docview_clear(self);

    self->doc = doc;
    if (NULL == self->doc)
        return;

    mume_refobj_addref(doc);

    page_count = mume_docview_count_pages(self);

    self->page_rects = malloc_abort(
        page_count * sizeof(mume_rect_t));

    self->page_texts = malloc_abort(
        page_count * sizeof(_page_text_t));

    self->page_links = malloc_abort(
        page_count * sizeof(mume_doclink_t*));

    for (i = 0; i < page_count; ++i) {
        self->page_texts[i].texts = NULL;
        self->page_texts[i].coords = NULL;
        self->page_texts[i].length = -1;
        self->page_links[i] = NULL;
    }

    _docview_update_page_rects(self);
    _docview_update_scroll_size(self);
    mume_invalidate_region(self, NULL);
}

void* mume_docview_get_doc(const void *_self)
{
    const struct _docview *self = _self;
    assert(mume_is_of(_self, mume_docview_class()));
    return self->doc;
}

int mume_docview_count_pages(const void *_self)
{
    const struct _docview *self = _self;

    assert(mume_is_of(_self, mume_docview_class()));

    if (self->doc)
        return mume_docdoc_count_pages(self->doc);

    return 0;
}

int mume_docview_first_visible(const void *_self)
{
    struct _docview *self = (struct _docview*)_self;
    int y, i, c;
    mume_rect_t rect;

    assert(mume_is_of(_self, mume_docview_class()));

    if (self->first_visible != -1)
        return self->first_visible;

    self->first_visible = 0;
    c = mume_docview_count_pages(self);
    mume_scrollview_get_scroll(self, NULL, &y);

    for (i = 0; i < c; ++i) {
        rect = _docview_get_page_rect(self, i, 1);

        if (y >= rect.y && y < rect.y + rect.height) {
            self->first_visible = i;
            break;
        }
    }

    return self->first_visible;
}

int mume_docview_page_from(const void *_self, int y)
{
    const struct _docview *self = _self;
    int sy, i, c;
    mume_rect_t rect = {0, 0, 0, 0};

    c = mume_docview_count_pages(self);
    mume_scrollview_get_scroll(self, NULL, &sy);
    sy += y;

    for (i = mume_docview_first_visible(self); i < c; ++i) {
        rect = _docview_get_page_rect(self, i, 1);

        if (sy >= rect.y && sy < rect.y + rect.height)
            return i;

        if (rect.y > sy)
            break;
    }

    return sy < rect.y ? 0 : c - 1;
}

void mume_docview_client_to_page(
    const void *self, int pageno, int *x, int *y)
{
    int sx, sy;
    mume_rect_t rect;

    assert(mume_is_of(self, mume_docview_class()));

    mume_scrollview_get_scroll(self, &sx, &sy);
    rect = _docview_get_page_rect(self, pageno, 0);

    *x = sx + *x - rect.x;
    *y = sy + *y - rect.y;
}

void mume_docview_page_to_client(
    const void *self, int pageno, int *x, int *y)
{
    int sx, sy;
    mume_rect_t rect;

    assert(mume_is_of(self, mume_docview_class()));

    mume_scrollview_get_scroll(self, &sx, &sy);
    rect = _docview_get_page_rect(self, pageno, 0);

    *x = rect.x + *x - sx;
    *y = rect.y + *y - sy;
}

void mume_docview_set_zoom(void *_self, float zoom)
{
    struct _docview *self = _self;

    assert(mume_is_of(_self, mume_docview_class()));

    if (zoom < _DOCVIEW_ZOOM_MIN)
        zoom = _DOCVIEW_ZOOM_MIN;
    else if (zoom > _DOCVIEW_ZOOM_MAX)
        zoom = _DOCVIEW_ZOOM_MAX;

    if (fabsf(zoom - self->zoom) > FLT_EPSILON) {
        self->zoom = zoom;
        self->first_visible = -1;
        _docview_update_page_rects(self);
        _docview_update_scroll_size(self);
        mume_invalidate_region(self, NULL);
    }
}

float mume_docview_get_zoom(const void *_self)
{
    const struct _docview *self = _self;
    assert(mume_is_of(_self, mume_docview_class()));
    return self->zoom;
}

void mume_docview_zoom_by(void *self, float factor, int x, int y)
{
    float nz, oz = mume_docview_get_zoom(self);
    int pageno = mume_docview_page_from(self, y);
    int px = x, py = y;

    if (pageno != -1)
        mume_docview_client_to_page(self, pageno, &px, &py);

    mume_docview_set_zoom(self, oz * factor);

    /* Change the viewport, so the original content of x, y
     * will be in the same place. */
    nz = mume_docview_get_zoom(self);
    if (pageno != -1 && nz != oz) {
        mume_rect_t r = _docview_get_page_rect(self, pageno, 1);
        mume_scrollview_set_scroll(
            self, r.x + px * nz / oz - x, r.y + py * nz / oz - y);
    }
}

void mume_docview_set_rotate(void *_self, int rotate)
{
    struct _docview *self = _self;

    assert(mume_is_of(_self, mume_docview_class()));

    rotate -= rotate % 90;
    rotate %= 360;
    if (rotate < 0)
        rotate += 360;

    if (self->rotate != rotate) {
        self->rotate = rotate;
        self->first_visible = -1;
        _docview_update_page_rects(self);
        _docview_update_scroll_size(self);
        mume_invalidate_region(self, NULL);
    }
}

int mume_docview_get_rotate(const void *_self)
{
    const struct _docview *self = _self;
    assert(mume_is_of(_self, mume_docview_class()));
    return self->rotate;
}

void mume_docview_rotate_by(void *self, int rotate)
{
    double rx, ry;
    int or, nr, x, y, w, h;

    mume_scrollview_get_scroll(self, &x, &y);
    mume_scrollview_get_size(self, &w, &h);
    if (w && h) {
        rx = x * 1.0 / w;
        ry = y * 1.0 / h;
    }
    else {
        rx = 1;
        ry = 1;
    }

    or = mume_docview_get_rotate(self);
    mume_docview_set_rotate(self, or + rotate);

    /* Change the viewport, so the original content will
     * in the viewport. */
    nr = mume_docview_get_rotate(self);
    if (or != nr) {
        mume_scrollview_get_size(self, &w, &h);
        mume_scrollview_set_scroll(self, w * rx, h * ry);
    }
}

mume_type_t* mume_typeof_docview_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _docview_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), pagebg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), selbg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), bkgnd);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
