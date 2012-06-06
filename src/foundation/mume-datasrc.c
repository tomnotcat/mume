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
#include "mume-datasrc.h"
#include "mume-debug.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include MUME_ASSERT_H

/* Data format class. */
#define _datafmt_super_class mume_object_class

struct _datafmt {
    const char _[MUME_SIZEOF_OBJECT];
    char *str;
};

struct _datafmt_class {
    const char _[MUME_SIZEOF_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _datafmt) == MUME_SIZEOF_DATAFMT);
MUME_STATIC_ASSERT(sizeof(struct _datafmt_class) ==
                   MUME_SIZEOF_DATAFMT_CLASS);

static void* _datafmt_ctor(
    struct _datafmt *self, int mode, va_list *app)
{
    if (!_mume_ctor(_datafmt_super_class(), self, mode, app))
        return NULL;

    self->str = strdup_abort(va_arg(*app, char*));
    return self;
}

static void* _datafmt_dtor(struct _datafmt *self)
{
    free(self->str);
    return _mume_dtor(_datafmt_super_class(), self);
}

const void* mume_datafmt_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_datafmt_meta_class(),
        "datafmt",
        _datafmt_super_class(),
        sizeof(struct _datafmt),
        MUME_PROP_END,
        _mume_ctor, _datafmt_ctor,
        _mume_dtor, _datafmt_dtor,
        MUME_FUNC_END);
}

const void* mume_datafmt_meta_class(void)
{
    return mume_meta_class();
}

const char* mume_datafmt_get_name(const void *_self)
{
    const struct _datafmt *self = _self;
    return self->str;
}

/* Data record class. */
#define _datarec_super_class mume_refobj_class
#define _datarec_super_meta_class mume_refobj_meta_class

struct _datarec {
    const char _[MUME_SIZEOF_REFOBJ];
    void *format;
    void *data;
    size_t length;
};

struct _datarec_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    const void* (*get_format)(void *self);
    const void* (*get_data)(void *self);
    size_t (*get_length)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _datarec) == MUME_SIZEOF_DATAREC);
MUME_STATIC_ASSERT(sizeof(struct _datarec_class) ==
                   MUME_SIZEOF_DATAREC_CLASS);

static const void* _datarec_get_format(struct _datarec *self)
{
    return self->format;
}

static const void* _datarec_get_data(struct _datarec *self)
{
    return self->data;
}

static size_t _datarec_get_length(struct _datarec *self)
{
    return self->length;
}

static void* _datarec_ctor(
    struct _datarec *self, int mode, va_list *app)
{
    if (!_mume_ctor(_datarec_super_class(), self, mode, app))
        return NULL;

    self->format = va_arg(*app, void*);
    self->data = va_arg(*app, void*);
    self->length = va_arg(*app, size_t);
    return self;
}

static void* _datarec_dtor(struct _datarec *self)
{
    free(self->data);
    return _mume_dtor(_datarec_super_class(), self);
}

static void* _datarec_class_ctor(
    struct _datarec_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_datarec_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_datarec_get_format)
            *(voidf**)&self->get_format = method;
        if (selector == (voidf*)_mume_datarec_get_data)
            *(voidf**)&self->get_data = method;
        else if (selector == (voidf*)_mume_datarec_get_length)
            *(voidf**)&self->get_length = method;
    }

    return self;
}

const void* mume_datarec_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_datarec_meta_class(),
        "datarec",
        _datarec_super_class(),
        sizeof(struct _datarec),
        MUME_PROP_END,
        _mume_ctor, _datarec_ctor,
        _mume_dtor, _datarec_dtor,
        _mume_datarec_get_format, _datarec_get_format,
        _mume_datarec_get_data, _datarec_get_data,
        _mume_datarec_get_length, _datarec_get_length,
        MUME_FUNC_END);
}

const void* mume_datarec_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "datarec class",
        _datarec_super_meta_class(),
        sizeof(struct _datarec_class),
        MUME_PROP_END,
        _mume_ctor, _datarec_class_ctor,
        MUME_FUNC_END);
}

void* mume_datarec_new(
    const void *format, void *data, size_t length)
{
    return mume_new(mume_datarec_class(), format, data, length);
}

void* mume_datarec_new_with_text(const char *text)
{
    return mume_datarec_new(
        mume_datafmt(MUME_DATAFMT_TEXT),
        strdup_abort(text), strlen(text) + 1);
}

const void* _mume_datarec_get_format(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_datarec_meta_class(), mume_datarec_class(),
        struct _datarec_class, get_format, (_self));
}

const void* _mume_datarec_get_data(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_datarec_meta_class(), mume_datarec_class(),
        struct _datarec_class, get_data, (_self));
}

size_t _mume_datarec_get_length(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_datarec_meta_class(), mume_datarec_class(),
        struct _datarec_class, get_length, (_self));
}

/* Data source class. */
#define _datasrc_super_class mume_refobj_class
#define _datasrc_super_meta_class mume_refobj_meta_class

struct _datasrc {
    const char _[MUME_SIZEOF_REFOBJ];
    void **datas;
    const void **formats;
    int count;
};

struct _datasrc_class {
    const char _[MUME_SIZEOF_REFOBJ_CLASS];
    const void** (*get_formats)(void *self);
    void* (*get_data)(void *self, const void *format);
};

MUME_STATIC_ASSERT(sizeof(struct _datasrc) == MUME_SIZEOF_DATASRC);
MUME_STATIC_ASSERT(sizeof(struct _datasrc_class) ==
                   MUME_SIZEOF_DATASRC_CLASS);

static const void** _datasrc_get_formats(struct _datasrc *self)
{
    return self->formats;
}

static void* _datasrc_get_data(
    struct _datasrc *self, const void *format)
{
    int i;
    for (i = 0; i < self->count; ++i) {
        if (self->formats[i] == format) {
            mume_refobj_addref(self->datas[i]);
            return self->datas[i];
        }
    }

    return NULL;
}

static void* _datasrc_ctor(
    struct _datasrc *self, int mode, va_list *app)
{
    void **datas;
    int i;

    if (!_mume_ctor(_datasrc_super_class(), self, mode, app))
        return NULL;

    datas = va_arg(*app, void**);
    self->count = va_arg(*app, int);
    assert(self->count >= 0);

    self->datas = malloc_abort(
        2 * sizeof(void*) * (self->count + 1));
    self->datas[self->count] = NULL;

    self->formats = (const void**)self->datas + self->count + 1;
    self->formats[self->count] = NULL;

    for (i = 0; i < self->count; ++i) {
        mume_refobj_addref(datas[i]);
        self->datas[i] = datas[i];
        self->formats[i] = mume_datarec_get_format(datas[i]);
    }

    return self;
}

static void* _datasrc_dtor(struct _datasrc *self)
{
    int i;
    for (i = 0; i < self->count; ++i)
        mume_refobj_release(self->datas[i]);

    free(self->datas);

    return _mume_dtor(_datasrc_super_class(), self);
}

static void* _datasrc_class_ctor(
    struct _datasrc_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_datasrc_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_datasrc_get_formats)
            *(voidf**)&self->get_formats = method;
        else if (selector == (voidf*)_mume_datasrc_get_data)
            *(voidf**)&self->get_data = method;
    }

    return self;
}

const void* mume_datasrc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_datasrc_meta_class(),
        "datasrc",
        _datasrc_super_class(),
        sizeof(struct _datasrc),
        MUME_PROP_END,
        _mume_ctor, _datasrc_ctor,
        _mume_dtor, _datasrc_dtor,
        _mume_datasrc_get_formats, _datasrc_get_formats,
        _mume_datasrc_get_data, _datasrc_get_data,
        MUME_FUNC_END);
}

const void* mume_datasrc_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "datasrc class",
        _datasrc_super_meta_class(),
        sizeof(struct _datasrc_class),
        MUME_PROP_END,
        _mume_ctor, _datasrc_class_ctor,
        MUME_FUNC_END);
}

void* mume_datasrc_new(void **datas, int count)
{
    return mume_new(mume_datasrc_class(), datas, count);
}

const void** _mume_datasrc_get_formats(
    const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_datasrc_meta_class(), mume_datasrc_class(),
        struct _datasrc_class, get_formats, (_self));
}

int mume_datasrc_count_formats(const void **formats)
{
    int count = 0;

    if (formats) {
        while (formats[count])
            ++count;
    }

    return count;
}

void* _mume_datasrc_get_data(
    const void *_clazz, void *_self, const void *format)
{
    MUME_SELECTOR_RETURN(
        mume_datasrc_meta_class(), mume_datasrc_class(),
        struct _datasrc_class, get_data, (_self, format));
}

void mume_datasrc_snapshot(void *_self)
{
    assert(0);
}
