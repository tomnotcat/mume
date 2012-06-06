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
#ifndef MUME_FOUNDATION_RESMGR_H
#define MUME_FOUNDATION_RESMGR_H

/* Resource manager provide the interface to manage the gui
 * resource in a central place, those are mainly for use in
 * window widget implementation.
 *
 * Conventions:
 *  1: When search a resource, the latest loaded path is searched first.
 */

#include "mume-common.h"

MUME_BEGIN_DECLS

struct mume_resmgr_s {
    mume_list_t *types;
    mume_list_t *repos;
    mume_oset_t *font_faces;
    mume_user_data_t *user_data;
    FT_Library ft_lib;
};

typedef void mume_resinit_fcn_t(
    mume_resmgr_t *mgr, const char *ns, void *obj);

mume_public mume_resmgr_t* mume_resmgr_ctor(mume_resmgr_t *self);

mume_public mume_resmgr_t* mume_resmgr_dtor(mume_resmgr_t *self);

#define mume_resmgr_new() \
    mume_resmgr_ctor((mume_resmgr_t*)( \
        malloc_abort(sizeof(mume_resmgr_t))))

#define mume_resmgr_delete(_self) \
    free(mume_resmgr_dtor(_self))

/* Setup the resource initialization function to the specified type,
 * this init function will be called the first time an object is
 * fetched that has this type.
 */
mume_public void mume_resmgr_set_resinit(
    mume_type_t *type, mume_resinit_fcn_t *init);

/* Register a resource type.
 * This should be done before load resource.
 *
 * The function will add reference to <tp>.
 */
mume_public void mume_resmgr_regtype(
    mume_resmgr_t *mgr, const char *ns,
    const char *nm, mume_type_t *tp);

/* Set/Get user defined data to the resource manager. */
mume_public void mume_resmgr_set_user_data(
    mume_resmgr_t *self, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy);

mume_public void* mume_resmgr_get_user_data(
    const mume_resmgr_t *self, const mume_user_data_key_t *key);

/* Load resources from local file system.
 *
 * Parameters:
 *  path : The root path of resources
 *  file : The object description file.
 */
mume_public int mume_resmgr_load(
    mume_resmgr_t *mgr, const char *path, const char *file);

/* Find a resource object.
 *
 * parameter:
 *  ns : the resource namespace
 *  nm : the resource name
 */
mume_public mume_objdesc_t* mume_resmgr_get_object(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

/* Find and open a resource file for reading.
 *
 * User should close the returned stream after use.
 */
mume_public mume_stream_t* mume_resmgr_open_file(
    mume_resmgr_t *mgr, const char *file);

/* help functions for get buildin resource objects. */
mume_public int mume_resmgr_get_integer(
    mume_resmgr_t *mgr, const char *ns, const char *nm, int def);

mume_public mume_color_t* mume_resmgr_get_color(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public mume_resobj_image_t* mume_resmgr_get_image(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public cairo_surface_t* mume_resmgr_get_icon(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public mume_resobj_brush_t* mume_resmgr_get_brush(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public mume_resobj_fontface_t* mume_resmgr_get_font_face(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public mume_resobj_charfmt_t* mume_resmgr_get_charfmt(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public mume_resobj_cursor_t* mume_resmgr_get_cursor(
    mume_resmgr_t *mgr, const char *ns, const char *nm);

mume_public FT_Face cairo_font_face_get_ft_face(
    cairo_font_face_t *font_face);

mume_public cairo_font_extents_t cairo_font_face_font_extents(
    cairo_font_face_t *font_face, double font_size);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_RESMGR_H */
