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
#include "mume-profile.h"

#define _profile_super_class mume_object_class

struct _profile {
    const char _[MUME_SIZEOF_OBJECT];
    mume_objbase_t *ob;
};

MUME_STATIC_ASSERT(sizeof(struct _profile) == MUME_SIZEOF_PROFILE);

static mume_objdesc_t* _profile_get_object(
    struct _profile *self, const char *ns,
    const char *nm, const char *type)
{
    mume_objns_t *on;
    mume_objdesc_t *od;

    if (ns)
        on = mume_objbase_getns(self->ob, ns, (type != NULL));
    else
        on = mume_objbase_root(self->ob);

    if (on)
        od = mume_objns_getobj(on, nm);
    else
        od = NULL;

    if (NULL == od && type)
        od = mume_objns_addobj(on, type, nm);

    return od;
}

static void* _profile_get_casted_object(
    struct _profile *self, const char *section,
    const char *name, const char *type, mume_type_t *p)
{
    void *obj = mume_objdesc_cast(
        _profile_get_object(self, section, name, type), p);

    if (NULL == obj) {
        if (type) {
            mume_warning((
                "Set object failed: %s:%s\n", section, name));
        }
        else {
            mume_warning((
                "Get object failed: %s:%s\n", section, name));
        }
    }

    return obj;
}

static void* _profile_ctor(
    struct _profile *self, int mode, va_list *app)
{
    mume_objns_t *rn;

    if (!_mume_ctor(_profile_super_class(), self, mode, app))
        return NULL;

    self->ob = mume_objbase_create();
    rn = mume_objbase_root(self->ob);
    mume_objns_regtype(rn, "int", mume_typeof_int());
    mume_objns_regtype(rn, "float", mume_typeof_float());
    mume_objns_regtype(rn, "rect", _mume_typeof_rect());

    return self;
}

static void* _profile_dtor(struct _profile *self)
{
    mume_objbase_destroy(self->ob);
    return _mume_dtor(_profile_super_class(), self);
}

const void* mume_profile_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_profile_meta_class(),
        "profile",
        _profile_super_class(),
        sizeof(struct _profile),
        MUME_PROP_END,
        _mume_ctor, _profile_ctor,
        _mume_dtor, _profile_dtor,
        MUME_FUNC_END);
}

int mume_profile_load(void *_self, mume_stream_t *stm)
{
    struct _profile *self = _self;
    assert(mume_is_of(_self, mume_profile_class()));
    return mume_objbase_load_xml(self->ob, NULL, stm);
}

int mume_profile_save(void *_self, mume_stream_t *stm)
{
    struct _profile *self = _self;
    assert(mume_is_of(_self, mume_profile_class()));
    return mume_objbase_save_xml(self->ob, stm);
}

void mume_profile_set_int(
    void *_self, const char *section, const char *name, int value)
{
    struct _profile *self = _self;
    int *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, "int", mume_typeof_int());

    if (obj)
        *obj = value;
}

int mume_profile_get_int(
    void *_self, const char *section, const char *name, int defval)
{
    struct _profile *self = _self;
    int *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, NULL, mume_typeof_int());

    if (obj)
        return *obj;

    return defval;
}

void mume_profile_set_float(
    void *_self, const char *section, const char *name, float value)
{
    struct _profile *self = _self;
    float *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, "float", mume_typeof_float());

    if (obj)
        *obj = value;
}

float mume_profile_get_float(
    void *_self, const char *section, const char *name, float defval)
{
    struct _profile *self = _self;
    float *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, NULL, mume_typeof_float());

    if (obj)
        return *obj;

    return defval;
}

void mume_profile_set_rect(
    void *_self, const char *section,
    const char *name, mume_rect_t value)
{
    struct _profile *self = _self;
    mume_rect_t *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, "rect", _mume_typeof_rect());

    if (obj)
        *obj = value;
}

mume_rect_t mume_profile_get_rect(
    void *_self, const char *section,
    const char *name, mume_rect_t defval)
{
    struct _profile *self = _self;
    mume_rect_t *obj;

    assert(mume_is_of(_self, mume_profile_class()));

    obj = _profile_get_casted_object(
        self, section, name, NULL, _mume_typeof_rect());

    if (obj)
        return *obj;

    return defval;
}
