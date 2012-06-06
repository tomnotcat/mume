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
#include "mume-text-layout.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-resmgr.h"
#include "mume-vector.h"
#include MUME_ASSERT_H
#include MUME_CTYPE_H
#include MUME_MATH_H

#include <cairo/cairo-ft.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-buffer.h>

#define _text_layout_super_class mume_object_class

struct _text_glyph {
    int codepoint;
    int cluster;
    double x_advance;
    double y_advance;
    double x_offset;
    double y_offset;
};

struct _text_block {
    struct _text_layout *layout;
    cairo_font_face_t *face;
    int text;
    int num_texts;
    mume_vector_t *glyphs;
    struct _text_block *next;
};

struct _text_run {
    struct _text_block *block;
    int text;
    int num_texts;
    int glyph;
    int num_glyphs;
    int splitter;
    struct _text_run *next;
};

struct _text_line {
    struct _text_run *begin;
    struct _text_run *end;
    struct _text_line *next;
};

struct _text_layout {
    const char _[MUME_SIZEOF_OBJECT];
    struct _text_block *blocks;
    mume_vector_t *texts;
    mume_vector_t *rects;
    struct _text_run *runs;
    struct _text_line *lines;
    int modified;
    int font_size;
    int line_width;
    unsigned int format;
};

static hb_font_t* _ft_face_create_hb_font(FT_Face ft_face)
{
    hb_font_t *hb_font;
    hb_face_t *hb_face;
    unsigned int hb_upem;
    hb_face = hb_ft_face_create(ft_face, NULL);
    hb_font = hb_font_create(hb_face);
    hb_upem = hb_face_get_upem(hb_face);
    hb_font_set_scale(hb_font, hb_upem, hb_upem);
    hb_ft_font_set_funcs(hb_font);
    hb_face_destroy(hb_face);
    return hb_font;
}

static hb_font_t* _cairo_font_face_get_hb_font(cairo_font_face_t *face)
{
    static cairo_user_data_key_t _ukey_hb_font;

    hb_font_t *hb_font;
    FT_Face ft_face;

    hb_font = cairo_font_face_get_user_data(face, &_ukey_hb_font);
    if (hb_font)
        return hb_font;

    ft_face = cairo_font_face_get_ft_face(face);
    hb_font = _ft_face_create_hb_font(ft_face);

    cairo_font_face_set_user_data(
        face, &_ukey_hb_font, hb_font,
        (cairo_destroy_func_t)hb_font_destroy);

    return hb_font;
}

static cairo_scaled_font_t* _create_cairo_scaled_font(
    cairo_font_face_t *cairo_face, double font_size)
{
    cairo_scaled_font_t *scaled_font;
    cairo_matrix_t ctm, font_matrix;
    cairo_font_options_t *font_options;
    cairo_matrix_init_identity(&ctm);
    cairo_matrix_init_scale(&font_matrix, font_size, font_size);
    font_options = cairo_font_options_create();
    cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_NONE);
    cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_OFF);
    scaled_font = cairo_scaled_font_create(
        cairo_face, &font_matrix, &ctm, font_options);
    cairo_font_options_destroy(font_options);
    return scaled_font;
}

static const char* _text_block_get_text(
    const struct _text_block *block, int offset)
{
    const char* text = mume_vector_front(block->layout->texts);
    return text + block->text + offset;
}

static mume_rect_t* _text_block_get_rect(
    const struct _text_block *block, int offset)
{
    mume_rect_t *rect = mume_vector_front(block->layout->rects);
    return rect + block->text + offset;
}

static int _text_block_append_glyphs(
    const struct _text_block *block, int count)
{
    int r = mume_vector_size(block->glyphs);
    mume_vector_append(block->glyphs, count);
    return r;
}

static struct _text_glyph* _text_block_get_glyph(
    const struct _text_block *block, int offset)
{
    struct _text_glyph *glyph = mume_vector_front(block->glyphs);
    return glyph + offset;
}

static cairo_font_extents_t _text_blocks_max_font_extents(
    const struct _text_block *block, double font_size)
{
    cairo_font_extents_t font_extents;
    cairo_font_extents_t max_extents = { 0, 0, 0, 0, 0 };

    while (block) {
        font_extents = cairo_font_face_font_extents(
            block->face, font_size);

#define CHOOSE_BIGGER_MEMBER(_X) \
        max_extents._X = MAX(max_extents._X, font_extents._X)

        CHOOSE_BIGGER_MEMBER(ascent);
        CHOOSE_BIGGER_MEMBER(descent);
        CHOOSE_BIGGER_MEMBER(height);
        CHOOSE_BIGGER_MEMBER(max_x_advance);
        CHOOSE_BIGGER_MEMBER(max_y_advance);
#undef CHOOSE_BIGGER_MEMBER

        block = block->next;
    }

    return max_extents;
}

static void _destroy_text_blocks(struct _text_block *block)
{
    struct _text_block *next;
    while (block) {
        next = block->next;
        cairo_font_face_destroy(block->face);
        mume_vector_delete(block->glyphs);
        free(block);
        block = next;
    }
}

static void _create_text_run(
    struct _text_run **r0, struct _text_run **ri,
    struct _text_block *block, hb_buffer_t *hb_buf,
    hb_font_t *hb_font, double scale, int text,
    int num_texts, int splitter)
{
    struct _text_run *run;
    struct _text_glyph *glyph;
    hb_glyph_info_t *hb_glyph;
    hb_glyph_position_t *hb_pos;
    int i;

    /* Shape. */
    hb_buffer_reset(hb_buf);
    hb_buffer_set_direction(hb_buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(hb_buf, hb_script_from_string(NULL, -1));
    hb_buffer_set_language(hb_buf, hb_language_from_string(NULL, -1));
    hb_buffer_add_utf8(
        hb_buf, _text_block_get_text(block, text),
        num_texts, 0, num_texts);
    hb_shape(hb_font, hb_buf, NULL, 0);

    if (*ri) {
        (*ri)->next = malloc_abort(sizeof(struct _text_run));
        (*ri) = (*ri)->next;
    }
    else {
        (*ri) = malloc_abort(sizeof(struct _text_run));
        (*r0) = (*ri);
    }

    run = (*ri);
    run->block = block;
    run->text = text;
    run->num_texts = num_texts;
    run->num_glyphs = hb_buffer_get_length(hb_buf);
    run->glyph = _text_block_append_glyphs(block, run->num_glyphs);
    run->splitter = splitter;
    run->next = NULL;

    /* Convert. */
    hb_glyph = hb_buffer_get_glyph_infos(hb_buf, NULL);
    hb_pos = hb_buffer_get_glyph_positions(hb_buf, NULL);

    glyph = _text_block_get_glyph(block, run->glyph);
    for (i = 0; i < run->num_glyphs; ++i) {
        glyph[i].codepoint = hb_glyph[i].codepoint;
        glyph[i].cluster = hb_glyph[i].cluster;
        glyph[i].x_advance = hb_pos[i].x_advance * scale;
        glyph[i].y_advance = hb_pos[i].y_advance * scale;
        glyph[i].x_offset = hb_pos[i].x_offset * scale;
        glyph[i].y_offset = hb_pos[i].y_offset * scale;
    }
}

static struct _text_run* _create_text_runs(
    struct _text_block *block, double font_size)
{
    FT_Face ft_face;
    hb_buffer_t *hb_buf;
    hb_font_t *hb_font;
    double scale;
    const char *text;
    int i, c;
    struct _text_run *r0 = NULL;
    struct _text_run *ri = NULL;

    hb_buf = hb_buffer_create();

    for (; block; block = block->next) {
        ft_face = cairo_font_face_get_ft_face(block->face);

        if (NULL == ft_face) {
            mume_warning(("Text layout don't support non FT_Face\n"));
            continue;
        }

        scale = font_size / ft_face->units_per_EM;
        hb_font = _cairo_font_face_get_hb_font(block->face);

        text = _text_block_get_text(block, 0);
        for (i = c = 0; i < block->num_texts; ++i) {
            if ('\n' == text[i] || '\t' == text[i]) {
                _create_text_run(&r0, &ri, block, hb_buf, hb_font,
                                 scale, c, i - c, text[i]);

                c = i + 1;
            }
        }

        if (i > c) {
            _create_text_run(&r0, &ri, block, hb_buf, hb_font,
                             scale, c, i - c, 0);
        }
    }

    hb_buffer_destroy(hb_buf);

    return r0;
}

static const char* _text_run_get_text(
    const struct _text_run *run)
{
    return _text_block_get_text(run->block, run->text);
}

static mume_rect_t* _text_run_get_rect(const struct _text_run *run)
{
    return _text_block_get_rect(run->block, run->text);
}

static struct _text_glyph* _text_run_get_glyph(
    const struct _text_run *run)
{
    return _text_block_get_glyph(run->block, run->glyph);
}

static int _text_run_glyph_from_text(
    const struct _text_run *run, int pos)
{
    const struct _text_glyph *glyph;
    int i;

    assert(pos >= 0 && pos < run->num_texts);

    if (run->num_glyphs <= 0)
        return 0;

    if (pos >= run->num_glyphs)
        pos = run->num_glyphs - 1;

    glyph = _text_run_get_glyph(run);
    if (glyph[pos].cluster == pos)
        return pos;

    i = pos;
    if (glyph[pos].cluster < pos) {
        for (i = pos; i < run->num_glyphs - 1; ++i) {
            if (glyph[i + 1].cluster > pos)
                break;
        }
    }
    else {
        for (i = pos; i > 0; --i) {
            if (glyph[i - 1].cluster < pos)
                break;
        }
    }

    return i;
}

static void _text_run_break(struct _text_run *run, int i)
{
    struct _text_run *nr;
    struct _text_glyph *g;
    int j, glyph;

    assert(i > 0 && i < run->num_texts);

    glyph = _text_run_glyph_from_text(run, i);
    nr = malloc_abort(sizeof(struct _text_run));
    nr->block = run->block;
    nr->text = run->text + i;
    nr->num_texts = run->num_texts - i;
    nr->glyph = run->glyph + glyph;
    nr->num_glyphs = run->num_glyphs - glyph;
    nr->splitter = 0;
    nr->next = run->next;

    run->num_texts = i;
    run->num_glyphs = glyph;
    run->next = nr;

    /* Modify the "cluster" member of each glyph,
     * so it is relative to the new texts. */
    g = _text_run_get_glyph(nr);
    for (j = 0; j < nr->num_glyphs; ++j)
        g[j].cluster -= i;
}

static void _destroy_text_runs(struct _text_run *run)
{
    struct _text_run *next;
    while (run) {
        next = run->next;
        free(run);
        run = next;
    }
}

static void _create_text_line(
    struct _text_line **l0, struct _text_line **li,
    struct _text_run *begin, struct _text_run *end)
{
    if (*li) {
        (*li)->next = malloc_abort(sizeof(struct _text_line));
        (*li) = (*li)->next;
    }
    else {
        (*li) = malloc_abort(sizeof(struct _text_line));
        (*l0) = (*li);
    }

    (*li)->begin = begin;
    (*li)->end = end;
    (*li)->next = NULL;
}

static struct _text_line* _create_text_lines(struct _text_run *run)
{
    struct _text_line *l0 = NULL;
    struct _text_line *li = NULL;
    struct _text_run *rx = run;

    while (run) {
        if ('\n' == run->splitter) {
            _create_text_line(&l0, &li, rx, run->next);
            rx = run->next;
        }

        run = run->next;
    }

    if (rx)
        _create_text_line(&l0, &li, rx, run);

    return l0;
}

static double _text_line_width(const struct _text_line *line)
{
    const struct _text_run *run;
    const struct _text_glyph *glyph;
    int i;
    double width = 0;

    for (run = line->begin; run != line->end; run = run->next) {
        glyph = _text_run_get_glyph(run);

        for (i = 0; i < run->num_glyphs; ++i)
            width += glyph[i].x_offset + glyph[i].x_advance;
    }

    return width;
}

static double _text_lines_max_width(const struct _text_line *line)
{
    double r = 0, w;
    while (line) {
        w = _text_line_width(line);
        r = MAX(r, w);
        line = line->next;
    }

    return r;
}

static int _text_lines_count(const struct _text_line *line)
{
    int count = 0;
    while (line) {
        ++count;
        line = line->next;
    }

    return count;
}

static void _text_line_break(
    struct _text_line *line, struct _text_run *run, int i)
{
    struct _text_line *nl;

    assert(run != line->begin || i > 0);

    nl = malloc_abort(sizeof(struct _text_line));
    nl->end = line->end;
    nl->next = line->next;

    line->next = nl;

    if (i > 0) {
        _text_run_break(run, i);
        line->end = run->next;
        nl->begin = run->next;
    }
    else {
        line->end = run;
        nl->begin = run;
    }
}

static void _text_lines_break_into_width(
    struct _text_line *line, int line_width, int word_break)
{
    int i, j, b;
    double w0, w1;
    struct _text_run *run;
    const struct _text_glyph *glyph;
    const char *text;

    for (; line; line = line->next) {
        w0 = 0;

        for (run = line->begin; run != line->end; run = run->next) {
            text = _text_run_get_text(run);
            glyph = _text_run_get_glyph(run);

            for (i = j = b = 0; i < run->num_texts; ++i) {
                if (word_break && isspace(text[i]))
                    b = i;

                w1 = 0;
                for (; j < run->num_glyphs; ++j) {
                    if (glyph[j].cluster <= i)
                        w1 += glyph[j].x_offset + glyph[j].x_advance;
                    else
                        break;
                }

                w0 += w1;
                if (w0 <= line_width)
                    continue;

                /* Exceed line width, split the line. */
                if (word_break) {
                    if (run != line->begin || b > 0) {
                        _text_line_break(line, run, b);
                        break;
                    }
                }
                else {
                    if (run != line->begin || i > 0) {
                        _text_line_break(line, run, i);
                        break;
                    }
                }
            }

            if (w0 > line_width)
                break;
        }
    }
}

static void _destroy_text_lines(struct _text_line *line)
{
    struct _text_line *next;
    while (line) {
        next = line->next;
        free(line);
        line = next;
    }
}

static void _text_layout_calc_rects(
    struct _text_layout *self, double line_height)
{
    struct _text_line *line;
    struct _text_run *run;
    struct _text_glyph *glyph;
    mume_rect_t *rect;
    double x, y, w;
    int i, j;

    y = 0;
    for (line = self->lines; line; line = line->next) {
        x = w = 0;

        for (run = line->begin; run != line->end; run = run->next) {
            glyph = _text_run_get_glyph(run);
            rect = _text_run_get_rect(run);

            for (i = j = w = 0; i < run->num_texts; ++i) {
                w = 0;
                for (; j < run->num_glyphs; ++j) {
                    if (glyph[j].cluster <= i)
                        w += glyph[j].x_offset + glyph[j].x_advance;
                    else
                        break;
                }

                rect[i].x = round(x);
                rect[i].y = round(y);
                rect[i].width = round(w);
                rect[i].height = line_height;

                x += w;
            }
        }

        y += line_height;
    }
}

static void* _text_layout_ctor(
    struct _text_layout *self, int mode, va_list *app)
{
    if (!_mume_ctor(_text_layout_super_class(), self, mode, app))
        return NULL;

    self->blocks = NULL;
    self->texts = mume_vector_new(sizeof(char), NULL, NULL);
    self->rects = mume_vector_new(sizeof(mume_rect_t), NULL, NULL);
    self->runs = NULL;
    self->lines = NULL;
    self->modified = 0;
    self->font_size = 0;
    self->line_width = 0;
    self->format = 0;

    return self;
}

static void* _text_layout_dtor(struct _text_layout *self)
{
    _destroy_text_blocks(self->blocks);
    mume_vector_delete(self->texts);
    mume_vector_delete(self->rects);
    _destroy_text_lines(self->lines);
    _destroy_text_runs(self->runs);

    return _mume_dtor(_text_layout_super_class(), self);
}

const void* mume_text_layout_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_text_layout_meta_class(),
        "text layout",
        _text_layout_super_class(),
        sizeof(struct _text_layout),
        MUME_PROP_END,
        _mume_ctor, _text_layout_ctor,
        _mume_dtor, _text_layout_dtor,
        MUME_FUNC_END);
}

void mume_text_layout_reset(void *_self)
{
    struct _text_layout *self = _self;

    _destroy_text_blocks(self->blocks);
    self->blocks = NULL;

    mume_vector_clear(self->texts);
    mume_vector_clear(self->rects);

    _destroy_text_lines(self->lines);
    self->lines = NULL;

    _destroy_text_runs(self->runs);
    self->runs = NULL;

    self->modified = 0;
    self->font_size = 0;
    self->line_width = 0;
    self->format = 0;
}

void mume_text_layout_add_text(
    void *_self, cairo_font_face_t *face, const char *text, int length)
{
    struct _text_layout *self = _self;
    struct _text_block **nb = &self->blocks;

    assert(face && text);

    while (*nb)
        nb = &(*nb)->next;

    if (-1 == length)
        length = strlen(text);

    if (length > 0) {
        (*nb) = malloc_abort(sizeof(struct _text_block));
        (*nb)->layout = self;
        (*nb)->face = face;
        (*nb)->text = mume_vector_size(self->texts);
        (*nb)->num_texts = length;
        (*nb)->glyphs = mume_vector_new(
            sizeof(struct _text_glyph), NULL, NULL);
        (*nb)->next = NULL;

        cairo_font_face_reference(face);

        memcpy(mume_vector_append(self->texts, length),
               text, length);

        memset(mume_vector_append(self->rects, length),
               0, sizeof(mume_rect_t) * length);

        self->modified = 1;
    }
}

void mume_text_layout_perform(
    void *_self, int font_size, cairo_t *cr,
    mume_rect_t *rect, unsigned int format)
{
    struct _text_layout *self = _self;
    cairo_font_extents_t font_extents;
    int line_count;
    int need_update = self->modified ||
                      (font_size != self->font_size) ||
                      ((format & MUME_TLF_EXPANDTABS) !=
                       (self->format & MUME_TLF_EXPANDTABS)) ||
                      ((format & MUME_TLF_WORDBREAK) !=
                       (self->format & MUME_TLF_WORDBREAK)) ||
                      ((format & MUME_TLF_SINGLELINE) !=
                       (self->format & MUME_TLF_SINGLELINE));

    assert((rect || (format & MUME_TLF_SINGLELINE)) && font_size > 0);

    if (!need_update && !(format & MUME_TLF_SINGLELINE))
        need_update = (rect->width != self->line_width);

    if (need_update) {
        _destroy_text_lines(self->lines);
        _destroy_text_runs(self->runs);

        self->runs = _create_text_runs(self->blocks, font_size);
        if (format & MUME_TLF_EXPANDTABS) {
            /* self->lines = _create_tabbed_text_lines(r0); */
            self->lines = NULL;
        }
        else {
            self->lines = _create_text_lines(self->runs);
        }

        if (!(format & MUME_TLF_SINGLELINE)) {
            _text_lines_break_into_width(
                self->lines, rect->width,
                format & MUME_TLF_WORDBREAK);

            self->line_width = rect->width;
        }

        self->modified = 0;
        self->font_size = font_size;
        self->format = format;
    }

    line_count = _text_lines_count(self->lines);
    font_extents = _text_blocks_max_font_extents(
        self->blocks, font_size);

    /* Calculate each character's coordinates. */
    _text_layout_calc_rects(self, font_extents.height);

    if (format & MUME_TLF_CALCRECT) {
        mume_rect_t rr;

        rr.width = _text_lines_max_width(self->lines);
        rr.height = line_count * font_extents.height;

        rr.x = rect->x;
        if (format & MUME_TLF_CENTER)
            rr.x += (rect->width - rr.width) / 2;
        else if (format & MUME_TLF_RIGHT)
            rr.x += rect->width - rr.width;

        rr.y = rect->y;
        if (format & MUME_TLF_VCENTER)
            rr.y += (rect->height - rr.height) / 2;
        else if (format & MUME_TLF_BOTTOM)
            rr.y += rect->height - rr.height;

        *rect = rr;
    }

    if (format & MUME_TLF_DRAWTEXT) {
        struct _text_line *line;
        struct _text_run *run;
        struct _text_glyph *glyph;
        cairo_font_face_t *last_face = NULL;
        cairo_scaled_font_t *scaled_font;
        cairo_glyph_t *glyphs;
        int i, num_glyphs = 16;
        double x, y, dx, dy;

        if (format & MUME_TLF_VCENTER) {
            dy = (rect->height - line_count * font_extents.height) / 2;
        }
        else if (format & MUME_TLF_BOTTOM) {
            dy = rect->height - line_count * font_extents.height;
        }
        else {
            dy = 0;
        }

        cairo_save(cr);

        if (!(format & MUME_TLF_NOCLIP)) {
            cairo_rectangle(
                cr, rect->x, rect->y, rect->width, rect->height);
            cairo_clip(cr);
        }

        cairo_translate(cr, rect->x, rect->y);

        /* Move the "base line". */
        cairo_translate(
            cr, 0, font_extents.height - font_extents.descent);

        glyphs = cairo_glyph_allocate(num_glyphs);
        for (line = self->lines; line; line = line->next) {
            x = y = 0;

            if (format & MUME_TLF_CENTER) {
                dx = (rect->width - _text_line_width(line)) / 2;
            }
            else if (format & MUME_TLF_RIGHT) {
                dx = rect->width - _text_line_width(line);
            }
            else {
                dx = 0;
            }

            for (run = line->begin; run != line->end; run = run->next) {
                if (run->num_glyphs > num_glyphs) {
                    num_glyphs = run->num_glyphs;
                    cairo_glyph_free(glyphs);
                    glyphs = cairo_glyph_allocate(num_glyphs);
                }

                glyph = _text_run_get_glyph(run);
                for (i = 0; i < run->num_glyphs; ++i) {
                    glyphs[i].index = glyph[i].codepoint;
                    glyphs[i].x = x + glyph[i].x_offset + dx;
                    glyphs[i].y = y - glyph[i].y_offset + dy;
                    x += glyph[i].x_advance;
                    y -= glyph[i].y_advance;
                }

                if (last_face != run->block->face) {
                    scaled_font = _create_cairo_scaled_font(
                        run->block->face, font_size);
                    cairo_set_scaled_font(cr, scaled_font);
                    cairo_scaled_font_destroy(scaled_font);
                    last_face = run->block->face;
                }

                cairo_show_glyphs(cr, glyphs, run->num_glyphs);
            }

            dy += font_extents.height;
        }

        cairo_glyph_free(glyphs);
        cairo_restore(cr);
    }
}

const char* mume_text_layout_get_texts(const void *_self)
{
    const struct _text_layout *self = _self;
    return mume_vector_front(self->texts);
}

const mume_rect_t* mume_text_layout_get_rects(const void *_self)
{
    const struct _text_layout *self = _self;
    return mume_vector_front(self->rects);
}

int mume_text_layout_get_length(const void *_self)
{
    const struct _text_layout *self = _self;
    return mume_vector_size(self->texts);
}
