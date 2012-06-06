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
#ifndef MUME_RESMGR_PRIVATE_H
#define MUME_RESMGR_PRIVATE_H

#include "mume-resmgr.h"
#include "mume-oset.h"

MUME_BEGIN_DECLS

typedef int mume_resinit_func_t(
    mume_resmgr_t *mgr, const char *ns, void *obj);

struct mume_resobj_image_s {
    char *name;
    mume_rect_t rect;
    mume_rect_t margin;
    cairo_surface_t *p0;
    cairo_surface_t *p1;
};

typedef struct mume_resobj_images_s {
    char *file;
    mume_color_t kclr;
    mume_oset_t sects;
    cairo_surface_t *p;
} mume_resobj_images_t;

struct mume_resobj_brush_s {
    char *image;
    mume_color_t color;
    mume_resobj_image_t *p;
};

struct mume_resobj_fontface_s {
    char *name;
    char *file;
    cairo_font_face_t *p;
};

struct mume_resobj_charfmt_s {
    char *face;
    int size;
    int underline;
    int strikeout;
    mume_color_t color;
    mume_resobj_fontface_t *p;
};

struct mume_resobj_cursor_s {
    int id;
    void *p;
};

typedef struct mume_widget_bkgnd_s {
    mume_resobj_brush_t normal;
    mume_resobj_brush_t hot;
    mume_resobj_brush_t pressed;
    mume_resobj_brush_t disabled;
} mume_widget_bkgnd_t;

typedef struct mume_widget_texts_s {
    mume_resobj_charfmt_t normal;
    mume_resobj_charfmt_t hot;
    mume_resobj_charfmt_t pressed;
    mume_resobj_charfmt_t disabled;
} mume_widget_texts_t;

mume_public mume_type_t* _mume_typeof_point(void);
mume_public mume_type_t* _mume_typeof_rect(void);
mume_public mume_type_t* _mume_typeof_color(void);
mume_public mume_type_t* _mume_typeof_resobj_images(void);
mume_public mume_type_t* _mume_typeof_resobj_brush(void);
mume_public mume_type_t* _mume_typeof_resobj_fontface(void);
mume_public mume_type_t* _mume_typeof_resobj_charfmt(void);
mume_public mume_type_t* _mume_typeof_resobj_cursor(void);
mume_public mume_type_t* _mume_typeof_widget_bkgnd(void);
mume_public mume_type_t* _mume_typeof_widget_texts(void);

static inline int mume_resobj_charfmt_is_valid(
    const mume_resobj_charfmt_t *cf)
{
    return cf && cf->p && cf->p->p && cf->size > 0;
}

MUME_END_DECLS

#endif /* MUME_RESMGR_PRIVATE_H */
