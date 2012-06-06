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
#include "mume-resmgr-private.h"
#include "mume-button.h"
#include "mume-cursor.h"
#include "mume-debug.h"
#include "mume-dibfcn.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-list.h"
#include "mume-memory.h"
#include "mume-menubar.h"
#include "mume-objbase.h"
#include "mume-oset.h"
#include "mume-scrollbar.h"
#include "mume-splitter.h"
#include "mume-stream.h"
#include "mume-string.h"
#include "mume-tabctrl.h"
#include "mume-treeview.h"
#include "mume-types.h"
#include "mume-userdata.h"
#include "mume-virtfs.h"
#include MUME_ASSERT_H
#include MUME_STDINT_H
#include MUME_STRING_H

static const cairo_user_data_key_t _ukey_ft_face;
static const mume_user_data_key_t _ukey_resinit;

typedef struct _restype_s {
    const char *ns;
    mume_objtype_t *tp;
} _restype_t;

typedef struct _resrepo_s {
    mume_virtfs_t *vfs;
    mume_objbase_t *ob;
} _resrepo_t;

typedef struct _resinit_param_s {
    mume_resmgr_t *mgr;
    const char *ns;
} _resinit_param_t;

typedef struct _font_face_s {
    const char *name;
    cairo_font_face_t *font_face;
} _font_face_t;

static void _surface_destruct(void *obj, void *p)
{
    cairo_surface_destroy(*(cairo_surface_t**)obj);
}

static void _font_face_destruct(void *obj, void *p)
{
    cairo_font_face_destroy(((_font_face_t*)obj)->font_face);
}

static void _restype_destruct(void *obj, void *p)
{
    _restype_t *type = obj;
    mume_objtype_destroy(type->tp);
}

static void _resrepo_destruct(void *obj, void *p)
{
    _resrepo_t *repo = obj;
    if (repo->ob)
        mume_objbase_destroy(repo->ob);
    if (repo->vfs)
        mume_virtfs_destroy(repo->vfs);
}

static void _type_color_construct(void *obj, void *p)
{
    mume_color_t *c = obj;
    c->r = c->g = c->b = c->a = 0;
}

static int _type_color_setstr(
    void *type, void *obj, const char *str)
{
#define _WHITE_SPACE_CHARS " \t\r\n\f\v"
    const char *s = str;
    s += strspn(s, _WHITE_SPACE_CHARS);
#undef _WHITE_SPACE_CHARS
    if ('#' == *s) {
        /* #RRGGBB format */
        unsigned int c;
        sscanf(s, "#%x", &c);
        mume_color_setuint((mume_color_t*)obj, c);
        return 1;
    }
    else if (0 == strncmp(s, "rgb", 3)) {
        /* rgb(R,G,B) format */
        const char *t;
        if ((s = strchr(s, '(')) && (t = strchr(s, ')'))) {
            int r, g, b;
            r = g = b = 0;
            r = strtol(s + 1, (char**)&s, 10);
            if (s != t)
                g = strtol(s + 1, (char**)&s, 10);
            if (s != t)
                b = strtol(s + 1, NULL, 10);
            mume_color_setrgb_int(
                (mume_color_t*)obj, r, g, b);
            return 1;
        }
    }
    mume_warning(("invalid color value: %s\n", str));
    return 0;
}

static int _type_color_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    return snprintf(buf, len, "#%06x", mume_color_getuint(obj));
}

static void _type_point_construct(void *obj, void *p)
{
    mume_point_t *pt = obj;
    pt->x = pt->y = 0;
}

static int _type_point_setstr(
    void *type, void *obj, const char *str)
{
    const char *s = str;
    mume_point_t *pt = (mume_point_t*)obj;
    pt->x = strtol(s, (char**)&s, 10);
    if (*s)
        pt->y = strtol(s + 1, (char**)&s, 10);
    return 1;
}

static int _type_point_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    mume_point_t *pt = (mume_point_t*)obj;
    return snprintf(buf, len, "%d,%d", pt->x, pt->y);
}

static void _type_rect_construct(void *obj, void *p)
{
    mume_rect_t *r = obj;
    r->x = r->y = r->width = r->height = 0;
}

static int _type_rect_setstr(
    void *type, void *obj, const char *str)
{
    const char *s = str;
    mume_rect_t *r = (mume_rect_t*)obj;
    r->x = strtol(s, (char**)&s, 10);
    if (*s)
        r->y = strtol(s + 1, (char**)&s, 10);
    if (*s)
        r->width = strtol(s + 1, (char**)&s, 10);
    if (*s)
        r->height = strtol(s + 1, (char**)&s, 10);
    return 1;
}

static int _type_rect_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    mume_rect_t *r = (mume_rect_t*)obj;
    return snprintf(buf, len, "%d,%d,%d,%d",
                    r->x, r->y, r->width, r->height);
}

static void _resobj_brush_init(
    mume_resmgr_t *mgr, const char *ns, void *obj)
{
    mume_resobj_brush_t *br = obj;
    assert(NULL == br->p);
    if (br->image) {
        br->p = mume_resmgr_get_image(mgr, ns, br->image);
        if (NULL == br->p) {
            mume_warning(("Get brush image failed: %s:%s\n",
                          ns, br->image));
        }
    }
}

static unsigned long _read_font_stream(
    FT_Stream stream, unsigned long offset,
    unsigned char *buffer, unsigned long count)
{
    if (stream->pos != offset) {
        mume_stream_seek(
            stream->descriptor.pointer, offset);
    }
    return (unsigned long)mume_stream_read(
        stream->descriptor.pointer, buffer, count);
}

static void _close_font_stream(FT_Stream stream)
{
    mume_stream_close(stream->descriptor.pointer);
    stream->descriptor.pointer = NULL;
    stream->size = 0;
    stream->base = 0;
    free(stream);
}

static FT_Face _resmgr_load_ft_face(mume_resmgr_t *mgr, const char *file)
{
    FT_Error err = 0;
    FT_Face face;
    FT_Stream strm;
    FT_Open_Args args;
    mume_stream_t *stm;
    stm = mume_resmgr_open_file(mgr, file);
    if (NULL == stm) {
        mume_warning(("Open font failed: %s\n", file));
        return NULL;
    }

    strm = malloc_abort(sizeof(*strm));
    strm->descriptor.pointer = stm;
    strm->pathname.pointer = NULL;
    strm->base = NULL;
    strm->size = (unsigned long)mume_stream_length(stm);
    strm->pos = 0;
    strm->cursor = 0;
    strm->read = _read_font_stream;
    strm->close = _close_font_stream;
    args.flags = FT_OPEN_STREAM;
    args.stream = strm;
    err = FT_Open_Face(mgr->ft_lib, &args, 0, &face);
    if (err) {
        _close_font_stream(strm);
        mume_warning(("FT_Open_Face(\"%s\"): %d\n", file, err));
        return NULL;
    }
    return face;
}

static cairo_font_face_t* _resmgr_get_font_face(
    mume_resmgr_t *mgr, const char *name)
{
    mume_oset_node_t *nd;
    FT_Face ft_face;
    cairo_font_face_t *font_face;
    cairo_status_t status;

    nd = mume_oset_find(mgr->font_faces, &name);
    if (nd)
        return ((_font_face_t*)mume_oset_data(nd))->font_face;

    ft_face = _resmgr_load_ft_face(mgr, name);
    if (NULL == ft_face)
        return NULL;

    font_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
    if (NULL == font_face) {
        FT_Done_Face(ft_face);
        return NULL;
    }

    status = cairo_font_face_set_user_data(
        font_face, &_ukey_ft_face, ft_face, NULL);

    if (status != CAIRO_STATUS_SUCCESS) {
        cairo_font_face_destroy(font_face);
        return NULL;
    }

    nd = mume_oset_new_name_node(name, sizeof(_font_face_t));
    ((_font_face_t*)mume_oset_data(nd))->font_face = font_face;
    mume_oset_insert(mgr->font_faces, nd);

    return font_face;
}

static void _resobj_fontface_init(
    mume_resmgr_t *mgr, const char *ns, void *obj)
{
    mume_resobj_fontface_t *ff = obj;

    assert(NULL == ff->p);
    if (ff->file) {
        ff->p = _resmgr_get_font_face(mgr, ff->file);
        if (NULL == ff->p) {
            mume_warning(("Get font face failed: %s:%s\n",
                          ns, ff->file));
        }
    }
}

static void _resobj_charfmt_init(
    mume_resmgr_t *mgr, const char *ns, void *obj)
{
    mume_resobj_charfmt_t *cf = obj;
    assert(NULL == cf->p);
    if (cf->face) {
        cf->p = mume_resmgr_get_font_face(mgr, ns, cf->face);
        if (NULL == cf->p) {
            mume_warning(("Get charfmt font face failed: %s:%s\n",
                          ns, cf->face));
        }
    }
}

static void _resobj_cursor_init(
    mume_resmgr_t *mgr, const char *ns, void *obj)
{
    mume_resobj_cursor_t *cur = obj;
    assert(NULL == cur->p);
    cur->p = mume_cursor(cur->id);
}

static void _resinit_traverse(
    mume_type_t *type, void *data, void *param)
{
    mume_resinit_func_t *init;
    _resinit_param_t *rp = (_resinit_param_t*)param;
    init = (mume_resinit_func_t*)(uintptr_t)(
        mume_type_get_user_data(type, &_ukey_resinit));

    if (init)
        init(rp->mgr, rp->ns, data);
}

static void _parse_obj_dot_sub(
    const char *nds, char *buf, size_t len,
    const char **obj, const char **sub)
{
    const char *dot = strchr(nds, '.');
    if (dot) {
        if (dot - nds < len)
            len = dot - nds;
        else
            len = len - 1;
        strncpy(buf, nds, len);
        buf[len] = '\0';
        if (obj)
            *obj = buf;
        if (sub) {
            *sub = dot + 1;
            if (NULL == *sub)
                *sub = "";
        }
    }
    else {
        if (obj)
            *obj = nds;
        if (sub)
            *sub = "";
    }
}

static cairo_status_t _read_image_stream(
    void *closure, unsigned char *data, unsigned int length)
{
    if (mume_stream_read(closure, data, length))
        return CAIRO_STATUS_SUCCESS;
    return CAIRO_STATUS_READ_ERROR;
}

static mume_resobj_image_t* _get_resobj_image(
    mume_resmgr_t *mgr, const char *ns,
    const char *objnm, const char *subnm)
{
    mume_objdesc_t *obj;
    mume_oset_node_t *nd;
    mume_resobj_images_t *ris;
    mume_resobj_image_t *ri;

    obj = mume_resmgr_get_object(mgr, ns, objnm);
    if (NULL == obj || mume_objdesc_type(obj) !=
        _mume_typeof_resobj_images())
    {
        return NULL;
    }

    ris = (mume_resobj_images_t*)mume_objdesc_data(obj);
    if (NULL == ris->p && ris->file) {
        mume_stream_t *stm;
        stm = mume_resmgr_open_file(mgr, ris->file);
        if (stm) {
            const char *ext = strrchr(ris->file, '.');
            if (0 == strcasecmp(ext, ".bmp")) {
                mume_dib_t *dib = mume_dib_create(stm);
                if (dib) {
                    mume_color_t *kclr = NULL;
                    if (mume_color_getuint(&ris->kclr))
                        kclr = &ris->kclr;
                    ris->p = mume_dib_create_surface(dib, kclr);
                    mume_dib_destroy(dib);
                }
            }
            else if (0 == strcasecmp(ext, ".png")) {
                ris->p = cairo_image_surface_create_from_png_stream(
                    _read_image_stream, stm);
            }
            else {
                /* TODO: add support for other image formats */
            }
            mume_stream_close(stm);
        }

        if (NULL == ris->p) {
            mume_warning(("Load image failed: %s\n", ris->file));
        }
    }

    nd = mume_oset_find(&ris->sects, &subnm);
    if (NULL == nd) {
        mume_warning(("Section not exists: %s.%s\n", objnm, subnm));
        return NULL;
    }

    ri = (mume_resobj_image_t*)mume_oset_data(nd);
    ri->p0 = ris->p;

    return ri;
}

mume_resmgr_t* mume_resmgr_ctor(mume_resmgr_t *self)
{
    self->types = mume_list_new(_restype_destruct, NULL);
    self->repos = mume_list_new(_resrepo_destruct, NULL);
    self->font_faces = mume_oset_new(
        _mume_type_string_compare, _font_face_destruct, NULL);
    self->user_data = NULL;
    /* Register buildin types. */
    mume_resmgr_regtype(
        self, NULL, "int", mume_typeof_int());
    mume_resmgr_regtype(
        self, NULL, "point", _mume_typeof_point());
    mume_resmgr_regtype(
        self, NULL, "rect", _mume_typeof_rect());
    mume_resmgr_regtype(
        self, NULL, "color", _mume_typeof_color());
    mume_resmgr_regtype(
        self, NULL, "images", _mume_typeof_resobj_images());
    mume_resmgr_regtype(
        self, NULL, "brush", _mume_typeof_resobj_brush());
    mume_resmgr_regtype(
        self, NULL, "fontface", _mume_typeof_resobj_fontface());
    mume_resmgr_regtype(
        self, NULL, "charfmt", _mume_typeof_resobj_charfmt());
    mume_resmgr_regtype(self, "button", "theme",
                        mume_typeof_button_theme());
    mume_resmgr_regtype(self, "scrollbar", "theme",
                        mume_typeof_scrollbar_theme());
    mume_resmgr_regtype(self, "splitter", "theme",
                        mume_typeof_splitter_theme());
    mume_resmgr_regtype(self, "treeview", "theme",
                        mume_typeof_treeview_theme());
    mume_resmgr_regtype(self, "tabctrl", "theme",
                        mume_typeof_tabctrl_theme());
    mume_resmgr_regtype(self, "menubar", "theme",
                        mume_typeof_menubar_theme());

    /* Initialize freetype. */
    if (FT_Init_FreeType(&self->ft_lib))
        mume_abort(("FT_Init_FreeType\n"));

    return self;
}

mume_resmgr_t* mume_resmgr_dtor(mume_resmgr_t *self)
{
    FT_Error err;
    mume_user_data_delete(self->user_data);
    mume_oset_delete(self->font_faces);
    mume_list_delete(self->types);
    mume_list_delete(self->repos);
    if ((err = FT_Done_FreeType(self->ft_lib))) {
        mume_warning(("Done freetype error: %d\n", err));
    }

    return self;
}

void mume_resmgr_set_resinit(
    mume_type_t *type, mume_resinit_fcn_t *init)
{
    mume_type_set_user_data(
        type, &_ukey_resinit, (void*)(uintptr_t)init, NULL);
}

void mume_resmgr_regtype(
    mume_resmgr_t *mgr, const char *ns,
    const char *nm, mume_type_t *tp)
{
    _restype_t *rt;
    mume_list_node_t *nd;
    size_t size = sizeof(*rt);
    if (ns)
        size += strlen(ns) + 1;
    nd = mume_list_push_back(mgr->types, size);
    rt = (_restype_t*)mume_list_data(nd);
    if (ns) {
        rt->ns = (char*)rt + sizeof(*rt);
        strcpy((char*)rt->ns, ns);
    }
    else {
        rt->ns = NULL;
    }
    rt->tp = mume_objtype_create(nm, tp);
}

void mume_resmgr_set_user_data(
    mume_resmgr_t *self, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy)
{
    if (NULL == self->user_data)
        self->user_data = mume_user_data_new();

    mume_user_data_set(self->user_data, key, data, destroy);
}

void* mume_resmgr_get_user_data(
    const mume_resmgr_t *self, const mume_user_data_key_t *key)
{
    if (self->user_data)
        return mume_user_data_get(self->user_data, key);

    return NULL;
}

int mume_resmgr_load(
    mume_resmgr_t *mgr, const char *path, const char *file)
{
    _resrepo_t *rr;
    _restype_t *rt;
    mume_stream_t *stm;
    mume_list_node_t *nd, *it;
    nd = mume_list_push_back(mgr->repos, sizeof(_resrepo_t));
    rr = (_resrepo_t*)mume_list_data(nd);
    rr->ob = NULL;
    rr->vfs = mume_virtfs_create(path);
    if (NULL == rr->vfs) {
        mume_warning(("create vfs failed: %s\n", path));
        mume_list_erase(mgr->repos, nd);
        return 0;
    }

    rr->ob = mume_objbase_create();
    it = mume_list_front(mgr->types);
    while (it) {
        rt = (_restype_t*)mume_list_data(it);
        mume_objns_addtype(
            mume_objbase_getns(rr->ob, rt->ns, 1), rt->tp);
        it = mume_list_next(it);
    }

    if (file && (stm = mume_virtfs_open_read(rr->vfs, file))) {
        if (!mume_objbase_load_xml(rr->ob, rr->vfs, stm)) {
            mume_warning(("Load object failed: %s\n", file));
        }
        mume_stream_close(stm);
    }

    return 1;
}

mume_objdesc_t* mume_resmgr_get_object(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    mume_list_node_t *nd;
    mume_objbase_t *ob;
    mume_objdesc_t *od = NULL;
    nd = mume_list_back(mgr->repos);
    while (nd) {
        ob = ((_resrepo_t*)mume_list_data(nd))->ob;
        if (ns) {
            mume_objns_t *n = mume_objbase_getns(ob, ns, 0);
            if (n)
                od = mume_objns_getobj(n, nm);
        }
        else {
            od = mume_objns_getobj(
                mume_objbase_root(ob), nm);
        }

        if (od) {
            if (!mume_objdesc_get_user_data(od, &_ukey_resinit)) {
                _resinit_param_t rp;
                rp.mgr = mgr, rp.ns = ns;

                mume_type_traverse(
                    mume_objdesc_type(od),
                    mume_objdesc_data(od),
                    &rp, _resinit_traverse);

                mume_objdesc_set_user_data(
                    od, &_ukey_resinit, od, NULL);
            }

            return od;
        }

        nd = mume_list_prev(nd);
    }

    return NULL;
}

mume_stream_t* mume_resmgr_open_file(
    mume_resmgr_t *mgr, const char *file)
{
    mume_stream_t *stm;
    mume_list_node_t *nd;
    nd = mume_list_back(mgr->repos);
    while (nd) {
        stm = mume_virtfs_open_read(
            ((_resrepo_t*)mume_list_data(nd))->vfs, file);
        if (stm)
            return stm;
        nd = mume_list_prev(nd);
    }
    return NULL;
}

int mume_resmgr_get_integer(
    mume_resmgr_t *mgr, const char *ns, const char *nm, int def)
{
    int *val = mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        mume_typeof_int());
    if (val)
        return *val;
    return def;
}

mume_color_t* mume_resmgr_get_color(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        _mume_typeof_color());
}

mume_resobj_image_t* mume_resmgr_get_image(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    /* format: image_set.image (e.g. button.normal) */
    char buf[256];
    const char *sub;
    _parse_obj_dot_sub(nm, buf, sizeof(buf), &nm, &sub);
    return _get_resobj_image(mgr, ns, nm, sub);
}

cairo_surface_t* mume_resmgr_get_icon(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    mume_resobj_image_t *image;

    image = mume_resmgr_get_image(mgr, ns, nm);
    if (NULL == image)
        return NULL;

    if (image->p0 && NULL == image->p1) {
        unsigned char *data;
        cairo_format_t format;
        int width, height, stride;
        mume_rect_t rect = image->rect;

        data = cairo_image_surface_get_data(image->p0);
        format = cairo_image_surface_get_format(image->p0);
        width = cairo_image_surface_get_width(image->p0);
        height = cairo_image_surface_get_height(image->p0);
        stride = cairo_image_surface_get_stride(image->p0);

        if (rect.x >= 0 && rect.x + rect.width < width &&
            rect.y >= 0 && rect.y + rect.height < height)
        {
            int bytes;

            switch (format) {
            case CAIRO_FORMAT_ARGB32:
            case CAIRO_FORMAT_RGB24:
                bytes = 4;
                break;

            case CAIRO_FORMAT_A8:
                bytes = 1;
                break;

            case CAIRO_FORMAT_RGB16_565:
                bytes = 2;
                break;

            default:
                mume_warning(("Unsupported image format: %d\n",
                              format));
                return NULL;
            }

            data += rect.y * stride + rect.x * bytes;
            image->p1 = cairo_image_surface_create_for_data(
                data, format, rect.width, rect.height, stride);
        }
        else {
            mume_warning((
                "Invalid image rect: %s:%s (%d, %d, %d, %d)\n",
                ns, nm, rect.x, rect.y, rect.width, rect.height));
        }
    }

    return image->p1;
}

mume_resobj_brush_t* mume_resmgr_get_brush(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        _mume_typeof_resobj_brush());
}

mume_resobj_fontface_t* mume_resmgr_get_font_face(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        _mume_typeof_resobj_fontface());
}

mume_resobj_charfmt_t* mume_resmgr_get_charfmt(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        _mume_typeof_resobj_charfmt());
}

mume_resobj_cursor_t* mume_resmgr_get_cursor(
    mume_resmgr_t *mgr, const char *ns, const char *nm)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mgr, ns, nm),
        _mume_typeof_resobj_cursor());
}

FT_Face cairo_font_face_get_ft_face(cairo_font_face_t *font_face)
{
    return cairo_font_face_get_user_data(font_face, &_ukey_ft_face);
}

cairo_font_extents_t cairo_font_face_font_extents(
    cairo_font_face_t *font_face, double font_size)
{
    FT_Face ft_face;
    cairo_font_extents_t fs_metrics;
    double scale;

    ft_face = cairo_font_face_get_ft_face(font_face);
    assert(ft_face && ft_face->units_per_EM);

    scale = ft_face->units_per_EM / font_size;
    fs_metrics.ascent = ft_face->ascender / scale;
    fs_metrics.descent = - ft_face->descender / scale;
    fs_metrics.height = ft_face->height / scale;
    fs_metrics.max_x_advance = ft_face->max_advance_width / scale;
    fs_metrics.max_y_advance = 0;

    return fs_metrics;
}

mume_type_t* _mume_typeof_point(void)
{
    static mume_type_simple_t _type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(mume_point_t),
            1,
            _type_point_construct,
        },
        _type_point_setstr,
        _type_point_getstr,
    };
    assert(_type.base.refcount > 0);
    return &_type.base;
}

mume_type_t* _mume_typeof_rect(void)
{
    static mume_type_simple_t _type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(mume_rect_t),
            1,
            _type_rect_construct,
        },
        _type_rect_setstr,
        _type_rect_getstr,
    };
    assert(_type.base.refcount > 0);
    return &_type.base;
}

mume_type_t* _mume_typeof_color(void)
{
    static mume_type_simple_t _type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(mume_color_t),
            1,
            _type_color_construct,
        },
        _type_color_setstr,
        _type_color_getstr,
    };
    assert(_type.base.refcount > 0);
    return &_type.base;
}

mume_type_t* _mume_typeof_resobj_images(void)
{
    static mume_type_t *tp;
    if (NULL == tp) {
        mume_type_t *t1, *t2, *t3;
        t3 = mume_type_pointer_create(_surface_destruct);
        /* image */
        MUME_COMPOUND_CREATE(
            t1, mume_resobj_image_t, NULL, NULL, NULL,
            _mume_type_string_compare);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), name);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), rect);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), margin);
        MUME_DIRECT_PROPERTY(mume_typeof_pointer(), p0);
        MUME_DIRECT_PROPERTY(t3, p1);
        MUME_COMPOUND_FINISH();
        /* image set */
        t2 = mume_type_oset_create(t1);
        MUME_COMPOUND_CREATE(
            tp, mume_resobj_images_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), file);
        MUME_DIRECT_PROPERTY(_mume_typeof_color(), kclr);
        MUME_DIRECT_PROPERTY(t2, sects);
        MUME_DIRECT_PROPERTY(t3, p);
        MUME_COMPOUND_FINISH();
        /* tp has reference now */
        mume_type_destroy(t1);
        mume_type_destroy(t2);
        mume_type_destroy(t3);
    }
    return tp;
}

mume_type_t* _mume_typeof_resobj_brush(void)
{
    static mume_type_t *tp;
    if (NULL == tp) {
        MUME_COMPOUND_CREATE(
            tp, mume_resobj_brush_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), image);
        MUME_DIRECT_PROPERTY(_mume_typeof_color(), color);
        MUME_DIRECT_PROPERTY(mume_typeof_pointer(), p);
        MUME_COMPOUND_FINISH();
        mume_resmgr_set_resinit(tp, _resobj_brush_init);
    }
    return tp;
}

mume_type_t* _mume_typeof_resobj_fontface(void)
{
    static mume_type_t *tp;

    if (NULL == tp) {
        MUME_COMPOUND_CREATE(
            tp, mume_resobj_fontface_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), name);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), file);
        MUME_DIRECT_PROPERTY(mume_typeof_pointer(), p);
        MUME_COMPOUND_FINISH();

        mume_resmgr_set_resinit(tp, _resobj_fontface_init);
    }

    return tp;
}

mume_type_t* _mume_typeof_resobj_charfmt(void)
{
    static mume_type_t *tp;

    if (NULL == tp) {
        MUME_COMPOUND_CREATE(
            tp, mume_resobj_charfmt_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_string(), face);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), size);
        MUME_DIRECT_PROPERTY(mume_typeof_bool(), underline);
        MUME_DIRECT_PROPERTY(mume_typeof_bool(), strikeout);
        MUME_DIRECT_PROPERTY(_mume_typeof_color(), color);
        MUME_DIRECT_PROPERTY(mume_typeof_pointer(), p);
        MUME_COMPOUND_FINISH();
        mume_resmgr_set_resinit(tp, _resobj_charfmt_init);
    }

    return tp;
}

mume_type_t* _mume_typeof_resobj_cursor(void)
{
    static mume_type_t *tp;
    static const mume_enumitem_t items[] = {
        { "arrow", MUME_CURSOR_ARROW },
        { "hdblarrow", MUME_CURSOR_HDBLARROW },
        { "vdblarrow", MUME_CURSOR_VDBLARROW },
        { "hand", MUME_CURSOR_HAND },
        { "ibeam", MUME_CURSOR_IBEAM },
        { "wait", MUME_CURSOR_WAIT },
        { NULL, 0 },
    };

    if (NULL == tp) {
        mume_type_t *t0;
        t0 = mume_type_enum_create(items, MUME_CURSOR_NONE);

        MUME_COMPOUND_CREATE(
            tp, mume_resobj_cursor_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(t0, id);
        MUME_DIRECT_PROPERTY(mume_typeof_pointer(), p);
        MUME_COMPOUND_FINISH();

        mume_type_destroy(t0);
        mume_resmgr_set_resinit(tp, _resobj_cursor_init);
    }

    return tp;
}

mume_type_t* _mume_typeof_widget_bkgnd(void)
{
    static mume_type_t *tp;
    if (NULL == tp) {
        MUME_COMPOUND_CREATE(
            tp, mume_widget_bkgnd_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), normal);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), hot);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), pressed);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), disabled);
        MUME_COMPOUND_FINISH();
    }
    return tp;
}

mume_type_t* _mume_typeof_widget_texts(void)
{
    static mume_type_t *tp;

    if (NULL == tp) {
        MUME_COMPOUND_CREATE(
            tp, mume_widget_texts_t, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_charfmt(), normal);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_charfmt(), hot);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_charfmt(), pressed);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_charfmt(), disabled);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
