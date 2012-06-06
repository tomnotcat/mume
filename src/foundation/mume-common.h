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
#ifndef MUME_FOUNDATION_COMMON_H
#define MUME_FOUNDATION_COMMON_H

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "mume-global.h"

MUME_BEGIN_DECLS

#ifdef MUME_EXPORTS
# define mume_public MUME_API_EXPORT
#else
# define mume_public MUME_API_IMPORT
#endif

#ifndef MAX
# define MAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#endif

#ifndef MIN
# define MIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#endif

#ifndef SWAP
# define SWAP(_a, _b) \
    do { \
        (_a) ^= (_b); \
        (_b) ^= (_a); \
        (_a) ^= (_b); \
    } while (0)
#endif

#ifndef COUNT_OF
# define COUNT_OF(_table) (sizeof(_table) / sizeof(_table[0]))
#endif

#ifndef EXTRA_OF
# define EXTRA_OF(_type, _obj) \
    ((void*)((char*)(_obj) + sizeof(_type)))
#endif

#ifndef CONTAINER_OF
# define CONTAINER_OF(_ptr, _type, _member) \
    ((_type*)((char*)(_ptr) - offsetof(_type, _member)))
#endif

#define mume_test_flag(_val, _flag) ((_val) & (1 << (_flag)))
#define mume_add_flag(_val, _flag) ((_val) |= (1 << (_flag)))
#define mume_remove_flag(_val, _flag) ((_val) &= ~(1 << (_flag)))

typedef struct mume_list_s mume_list_t;
typedef struct mume_vector_s mume_vector_t;
typedef struct mume_oset_s mume_oset_t;
typedef struct mume_logger_s mume_logger_t;
typedef struct mume_stream_s mume_stream_t;
typedef struct mume_virtfs_s mume_virtfs_t;
typedef struct mume_timeval_s mume_timeval_t;
typedef struct mume_type_s mume_type_t;
typedef struct mume_prop_s mume_prop_t;
typedef struct mume_objbase_s mume_objbase_t;
typedef struct mume_objns_s mume_objns_t;
typedef struct mume_objtype_s mume_objtype_t;
typedef struct mume_objdesc_s mume_objdesc_t;
typedef struct mume_user_data_s mume_user_data_t;
typedef struct mume_user_data_key_s mume_user_data_key_t;
typedef void mume_destroy_func_t(void *p);
typedef void mume_mutex_t;
typedef void mume_sem_t;
typedef void mume_thread_t;

typedef void mume_confcn_t(void *obj, void *p);
typedef void mume_desfcn_t(void *obj, void *p);
typedef void mume_cpyfcn_t(void *t, const void *f, void *p);
typedef int mume_cmpfcn_t(const void *a, const void *b);

typedef struct mume_matrix_s mume_matrix_t;
typedef struct mume_point_s mume_point_t;
typedef cairo_rectangle_int_t mume_rect_t;
typedef struct mume_color_s mume_color_t;
typedef union mume_event_u mume_event_t;
typedef int mume_evtproc_t(mume_event_t *evt);
typedef struct mume_resmgr_s mume_resmgr_t;
typedef struct mume_timer_s mume_timer_t;
typedef struct mume_timerq_s mume_timerq_t;
typedef struct mume_resobj_image_s mume_resobj_image_t;
typedef struct mume_resobj_brush_s mume_resobj_brush_t;
typedef struct mume_resobj_fontface_s mume_resobj_fontface_t;
typedef struct mume_resobj_charfmt_s mume_resobj_charfmt_t;
typedef struct mume_resobj_cursor_s mume_resobj_cursor_t;
typedef void mume_dlhdl_t;

struct mume_color_s {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

static inline void mume_color_setrgb(
    mume_color_t *c, double r, double g, double b)
{
    c->r = (unsigned char)(255.0 * r);
    c->g = (unsigned char)(255.0 * g);
    c->b = (unsigned char)(255.0 * b);
}

static inline mume_color_t mume_color_make(
    double r, double g, double b)
{
    mume_color_t c;
    c.r = (unsigned char)(255.0 * r);
    c.g = (unsigned char)(255.0 * g);
    c.b = (unsigned char)(255.0 * b);
    return c;
}

#define mume_color_rval(_clr) ((double)(_clr)->r / 255.0)
#define mume_color_gval(_clr) ((double)(_clr)->g / 255.0)
#define mume_color_bval(_clr) ((double)(_clr)->b / 255.0)

static inline void mume_color_setrgb_int(
    mume_color_t *c, int r, int g, int b)
{
    c->r = (unsigned char)r;
    c->g = (unsigned char)g;
    c->b = (unsigned char)b;
}

static inline mume_color_t mume_color_make_int(int r, int g, int b)
{
    mume_color_t c;
    c.r = (unsigned char)r;
    c.g = (unsigned char)g;
    c.b = (unsigned char)b;
    return c;
}

static inline void mume_color_setuint(
    mume_color_t *c, unsigned int v)
{
    /* 0xRRGGBB */
    c->r = (unsigned char)((v & 0x00FF0000) >> 16);
    c->g = (unsigned char)((v & 0x0000FF00) >> 8);
    c->b = (unsigned char)(v & 0x000000FF);
}

static inline unsigned int mume_color_getuint(
    const mume_color_t *c)
{
    /* 0xRRGGBB */
    return (unsigned int)(c->r << 16) |
            (unsigned short)(c->g << 8) | (c->b);
}

MUME_END_DECLS

#endif /* MUME_FOUNDATION_COMMON_H */
