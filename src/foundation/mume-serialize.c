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
#include "mume-serialize.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-octnr.h"
#include "mume-oset.h"
#include "mume-property.h"
#include "mume-stream.h"
#include "mume-string.h"
#include "mume-types.h"
#include "mume-variant.h"
#include "mume-vector.h"
#include MUME_ASSERT_H
#include MUME_EXPAT_H

#define _CURRENT_VERSION "1.0"
#define _CURRENT_COMPATIBILITY "1.0"

#define _XML_ERROR_COMPATIBILITY 1

#ifdef XML_LARGE_SIZE
# if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#  define XML_FMT_INT_MOD "I64"
# else
#  define XML_FMT_INT_MOD "ll"
# endif
#else
# define XML_FMT_INT_MOD "l"
#endif

#define _serialize_super_class mume_object_class

enum _serialize_flags_e {
    _SER_FLAG_CLASSMD,
    _SER_FLAG_NAMESTATIC,
    _SER_FLAG_OBJSTATIC
};

struct _serobj {
    const char *name;
    void *object;
    unsigned int flags;
};

struct _serialize {
    const char _[MUME_SIZEOF_OBJECT];
    const void **clazzs;
    mume_oset_t *objs;
    size_t clazzc;
    size_t clazzm;
    unsigned int flags;
};

struct _parse_context {
    struct _serialize *ser;
    mume_vector_t *stack;
    const void **clazzs;
    const void **props;
    void **vars;
    int propc;
    int propm;
    int depth;
    int error;
};

struct _xml_context {
    struct _parse_context *p;
    const void *clazz;
    int prop_offset;
    int prop_count;
    int has_char;
};

MUME_STATIC_ASSERT(sizeof(struct _serialize) == MUME_SIZEOF_SERIALIZE);

static void _xml_context_ctor(
    struct _xml_context *c, struct _parse_context *p,
    const void *clazz, int has_char)
{
    c->p = p;
    c->clazz = clazz;
    c->prop_offset = p->propc;
    c->prop_count = 0;
    c->has_char = has_char;
}

static void _xml_context_dtor(struct _xml_context *c)
{
    c->p->propc -= c->prop_count;
}

static void _xml_context_push_property(
    struct _xml_context *c, const void *clazz, const void *prop)
{
    struct _parse_context *p = c->p;

    if (p->propc >= p->propm) {
        int i, alloc = p->propm;

        while (p->propc >= alloc)
            alloc += (alloc >> 1) + 32;

        p->clazzs = realloc_abort(
            p->clazzs, alloc * sizeof(void*));

        p->props = realloc_abort(
            p->props, alloc * sizeof(void*));

        p->vars = realloc_abort(
            p->vars, alloc * sizeof(void*));

        for (i = p->propm; i < alloc; ++i)
            p->vars[i] = NULL;

        p->propm = alloc;
    }

    p->clazzs[p->propc] = clazz;
    p->props[p->propc] = prop;

    if (NULL == p->vars[p->propc]) {
        p->vars[p->propc] = mume_variant_new(
            mume_property_get_type(prop));
    }
    else {
        mume_variant_reset(
            p->vars[p->propc], mume_property_get_type(prop));
    }

    ++p->propc;
    ++c->prop_count;
}

static void _parse_context_ctor(
    struct _parse_context *c, struct _serialize *ser)
{
    c->ser = ser;
    c->depth = 0;
    c->error = 0;
    c->propc = 0;
    c->propm = 0;
    c->stack = mume_vector_new(
        sizeof(struct _xml_context), NULL, NULL);
    c->clazzs = NULL;
    c->props = NULL;
    c->vars = NULL;
}

static void _parse_context_dtor(struct _parse_context *c)
{
    int i;

    assert(0 == c->propc);

    for (i = 0; i < c->propm; ++i)
        mume_delete(c->vars[i]);

    free(c->vars);
    free(c->props);
    free(c->clazzs);

    mume_vector_delete(c->stack);
}

static void* _parse_context_last_variant(struct _parse_context *c)
{
    if (c->propc > 0)
        return c->vars[c->propc - 1];

    return NULL;
}

static void _serobj_destruct(void *obj, void *p)
{
    struct _serobj *o = obj;

    if (!mume_test_flag(o->flags, _SER_FLAG_NAMESTATIC))
        free((char*)o->name);

    if (!mume_test_flag(o->flags, _SER_FLAG_OBJSTATIC))
        mume_delete(o->object);
}

static const void* _serialize_get_class(
    struct _serialize *self, const char *name)
{
    char buf[MUME_SIZEOF_CLASS];
    const void *key = mume_class_key(buf, name);
    const void **clazz;

    if (mume_test_flag(self->flags, _SER_FLAG_CLASSMD)) {
        qsort(self->clazzs, self->clazzc,
          sizeof(void*), mume_object_compare);
        mume_remove_flag(self->flags, _SER_FLAG_CLASSMD);
    }

    clazz = bsearch(&key, self->clazzs, self->clazzc,
                    sizeof(void*), mume_object_compare);

    return clazz ? *clazz : NULL;
}

static void _serialize_set_object(
    struct _serialize *self, const char *name,
    void *object, unsigned int flags)
{
    mume_oset_node_t *node;
    struct _serobj *data;

    node = mume_oset_find(self->objs, &name);
    if (NULL == object) {
        if (node)
            mume_oset_erase(self->objs, node);

        return;
    }

    if (node) {
        data = mume_oset_data(node);
        _serobj_destruct(data, NULL);
    }
    else {
        node = mume_oset_newnode(sizeof(*data));
        data = mume_oset_data(node);
        data->name = name;
        mume_oset_insert(self->objs, node);
    }

    if (mume_test_flag(flags, _SER_FLAG_NAMESTATIC))
        data->name = name;
    else
        data->name = strdup_abort(name);

    data->object = object;
    data->flags = flags;
}

static void _xml_write_indents(mume_stream_t *stm, int count)
{
    while (count-- > 0)
        mume_stream_printf(stm, "  ");
}

static void _xml_write_object(
    mume_stream_t *stm, int indent, const char *name, void *object)
{
    const void *clazz;
    const void *it;
    const void **props;
    char *a, *b;
    void *var;
    int i, propc;

    /* TODO: convert invalid xml character like < > " */
    clazz = mume_class_of(object);

    _xml_write_indents(stm, indent);

    if (name) {
        a = mume_escape_xml_entities(name);
        b = mume_escape_xml_entities(mume_class_name(clazz));

        mume_stream_printf(stm, "<%s class=\"%s\">\n", a, b);

        if (a != name)
            free(a);

        if (b != mume_class_name(clazz))
            free(b);
    }
    else {
        a = mume_escape_xml_entities(mume_class_name(clazz));
        mume_stream_printf(stm, "<%s>\n", a);

        if (a != mume_class_name(clazz))
            free(a);
    }

    /* Write object properties. */
    it = clazz;
    var = mume_variant_new(MUME_TYPE_INT);

    do {
        props = mume_class_properties(it, &propc);
        for (i = 0; i < propc; ++i) {
            mume_variant_reset(var, mume_property_get_type(props[i]));

            if (_mume_get_property(it, object, props[i], var)) {
                if (mume_variant_convert(var, MUME_TYPE_STRING)) {
                    if (mume_variant_get_string(var)) {
                        _xml_write_indents(stm, indent + 1);

                        a = mume_escape_xml_entities(
                            mume_property_get_name(props[i]));

                        b = mume_escape_xml_entities(
                            mume_variant_get_string(var));

                        mume_stream_printf(stm, "<%s>%s</%s>\n", a, b, a);

                        if (a != mume_property_get_name(props[i]))
                            free(a);

                        if (b != mume_variant_get_string(var))
                            free(b);
                    }
                }
                else if (mume_variant_get_type(var) == MUME_TYPE_OBJECT) {
                    if (mume_variant_get_object(var)) {
                        _xml_write_object(
                            stm, indent + 1,
                            mume_property_get_name(props[i]),
                            (void*)mume_variant_get_object(var));
                    }
                }
                else {
                    mume_warning(("Unknown property type: %s\n",
                                  mume_property_get_type(props[i])));
                }
            }
        }

        it = mume_super_class(it);
    } while (it != mume_object_class());

    mume_delete(var);

    if (mume_is_of(object, mume_octnr_class())) {
        void *end = mume_octnr_end(object);
        void *oit;

        for (oit = mume_octnr_begin(object); oit != end;
             oit = mume_octnr_next(object, oit))
        {
            assert(mume_octnr_value(object, oit));

            _xml_write_object(stm, indent + 1, NULL,
                              mume_octnr_value(object, oit));
        }
    }

    _xml_write_indents(stm, indent);

    if (name) {
        a = mume_escape_xml_entities(name);
        mume_stream_printf(stm, "</%s>\n", a);

        if (a != name)
            free(a);
    }
    else {
        a = mume_escape_xml_entities(mume_class_name(clazz));
        mume_stream_printf(stm, "</%s>\n", a);

        if (a != mume_class_name(clazz))
            free(a);
    }
}

static const char* _xml_find_attr(const char **attr, const char *key)
{
    int i;

    for (i = 0; attr[i]; i += 2) {
        if (0 == strcmp(attr[i], key))
            return attr[i + 1];
    }

    return NULL;
}

static void _xml_start_element(
    void *data, const char *el, const char **attr)
{
    struct _parse_context *d = data;
    struct _xml_context *c;
    const char *val;
    const void *clazz;
    const void *prop;

    if (d->depth == mume_vector_size(d->stack)) {
        if (d->depth > 0) {
            if (d->depth > 1) {
                c = mume_vector_back(d->stack);
                if (mume_is_ancestor(c->clazz, mume_octnr_class())) {
                    /* Container element. */
                    static void *container_prop;
                    clazz = _serialize_get_class(d->ser, el);

                    if (NULL == container_prop) {
                        container_prop = mume_property_new(
                            MUME_TYPE_OBJECT, "", 0, 0);
                    }

                    if (clazz) {
                        _xml_context_push_property(c, clazz, container_prop);
                        c = mume_vector_push_back(d->stack);
                        _xml_context_ctor(c, d, clazz, 0);
                    }
                    else {
                        mume_warning(("Class not exist: %s\n", el));
                    }
                }
                else {
                    /* Object property. */
                    clazz = c->clazz;
                    prop = mume_seek_property(&clazz, el);
                    if (prop) {
                        _xml_context_push_property(c, clazz, prop);

                        if (mume_property_get_type(prop) != MUME_TYPE_OBJECT) {
                            c = mume_vector_push_back(d->stack);
                            _xml_context_ctor(c, d, NULL, 1);
                        }
                        else {
                            val = _xml_find_attr(attr, "class");
                            if (val) {
                                clazz = _serialize_get_class(d->ser, val);
                                if (clazz) {
                                    c = mume_vector_push_back(d->stack);
                                    _xml_context_ctor(c, d, clazz, 0);
                                }
                                else {
                                    mume_warning(("Class not exist: %s\n", val));
                                }
                            }
                            else {
                                mume_warning(("Element has no class\n"));
                            }
                        }
                    }
                    else {
                        mume_warning(("Property not exist: %s\n", el));
                    }
                }
            }
            else {
                val = _xml_find_attr(attr, "class");
                if (val) {
                    clazz = _serialize_get_class(d->ser, val);
                    if (clazz) {
                        c = mume_vector_push_back(d->stack);
                        _xml_context_ctor(c, d, clazz, 0);
                    }
                    else {
                        mume_warning(("Class not exist: %s\n", val));
                    }
                }
                else {
                    mume_warning(("Element has no class\n"));
                }
            }
        }
        else {
            /* Root node. */
            val = _xml_find_attr(attr, "compatibility");

            if (val && strcmp(val, _CURRENT_VERSION) <= 0) {
                c = mume_vector_push_back(d->stack);
                _xml_context_ctor(c, d, NULL, 0);
            }
            else {
                d->error = _XML_ERROR_COMPATIBILITY;

                mume_warning(("Incompatible file: %s > %s\n",
                              val, _CURRENT_VERSION));
            }
        }
    }
    else {
        mume_warning(("Invalid element start: %s\n", el));
    }

    ++d->depth;
}

static void _xml_end_element(void *data, const char *el)
{
    struct _parse_context *d = data;

    if (d->depth == mume_vector_size(d->stack)) {
        struct _xml_context *c = mume_vector_back(d->stack);
        void *object = NULL;

        if (c->clazz) {
            if (mume_is_ancestor(c->clazz, mume_octnr_class())) {
                int i;
                void **vars = d->vars + c->prop_offset;

                object = mume_new_with_props(
                    c->clazz, 0, NULL, NULL, NULL);

                for (i = 0; i < c->prop_count; ++i) {
                    assert(mume_variant_get_object(vars[i]));
                    mume_octnr_insert(object, mume_octnr_end(object),
                                      mume_clone(mume_variant_get_object(vars[i])));
                }
            }
            else {
                const void **clazzs = d->clazzs + c->prop_offset;
                const void **props = d->props + c->prop_offset;
                void **vars = d->vars + c->prop_offset;
                object = mume_new_with_props(
                    c->clazz, c->prop_count, clazzs, props, vars);
            }
        }

        _xml_context_dtor(c);
        mume_vector_pop_back(d->stack);

        if (object) {
            if (mume_vector_size(d->stack) == 1) {
                _serialize_set_object(d->ser, el, object, 0);
            }
            else {
                void *var = _parse_context_last_variant(d);
                assert(mume_variant_get_type(var) == MUME_TYPE_OBJECT);

                mume_variant_set_object(var, object);
                mume_delete(object);
            }
        }
    }
    else {
        mume_warning(("Invalid element end: %s\n", el));
    }

    --d->depth;
}

static void _xml_handle_char(void *data, const char *txt, int len)
{
    struct _parse_context *d = data;

    if (d->depth > 0 && d->depth == mume_vector_size(d->stack)) {
        struct _xml_context *c = mume_vector_back(d->stack);
        void *var;
        int type;

        if (!c->has_char)
            return;

        var = _parse_context_last_variant(d);
        type = mume_variant_get_type(var);

        if (type != MUME_TYPE_STRING)
            mume_variant_reset(var, MUME_TYPE_STRING);

        mume_variant_append_string(var, txt, len);

        if (type != MUME_TYPE_STRING) {
            mume_variant_convert(var, type);
            assert(mume_variant_get_type(var) == type);
        }
    }
}

static void* _serialize_ctor(
    struct _serialize *self, int mode, va_list *app)
{
    assert(MUME_CTOR_NORMAL == mode);

    if (!_mume_ctor(_serialize_super_class(), self, mode, app))
        return NULL;

    self->clazzs = NULL;
    self->objs = mume_oset_new(
        _mume_type_string_compare, _serobj_destruct, NULL);
    self->clazzc = 0;
    self->clazzm = 0;
    self->flags = 0;

    return self;
}

static void* _serialize_dtor(struct _serialize *self)
{
    mume_oset_delete(self->objs);
    free(self->clazzs);
    return _mume_dtor(_serialize_super_class(), self);
}

const void* mume_serialize_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_serialize_meta_class(),
        "serialize",
        _serialize_super_class(),
        sizeof(struct _serialize),
        MUME_PROP_END,
        _mume_ctor, _serialize_ctor,
        _mume_dtor, _serialize_dtor,
        MUME_FUNC_END);
}

void mume_serialize_register(void *_self, const void *clazz)
{
    struct _serialize *self = _self;

    assert(mume_is_of(_self, mume_serialize_class()));
    assert(mume_is_of(clazz, mume_meta_class()));

    self->clazzs = mume_ensure_buffer(
        self->clazzs, &self->clazzm,
        self->clazzc + 1, sizeof(void*));

    self->clazzs[self->clazzc] = clazz;
    self->clazzc += 1;

    mume_add_flag(self->flags, _SER_FLAG_CLASSMD);
}

int mume_serialize_out(void *_self, mume_stream_t *stm)
{
    struct _serialize *self = _self;
    struct _serobj *data;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_serialize_class()));

    mume_stream_printf(
        stm, "<mume version=\"%s\" compatibility=\"%s\">\n",
        _CURRENT_VERSION, _CURRENT_COMPATIBILITY);

    mume_oset_foreach(self->objs, node, data) {
        _xml_write_object(stm, 1, data->name, data->object);
    }

    mume_stream_printf(stm, "</mume>\n");
    return 1;
}

int mume_serialize_in(void *_self, mume_stream_t *stm)
{
    struct _serialize *self = _self;
    char buf[1024];
    size_t len;
    int done;
    int status;
    XML_Parser parser;
    struct _parse_context context;

    assert(mume_is_of(_self, mume_serialize_class()));

    _parse_context_ctor(&context, self);

    parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &context);
    XML_SetElementHandler(
        parser, _xml_start_element, _xml_end_element);
    XML_SetCharacterDataHandler(parser, _xml_handle_char);

    do {
        len = mume_stream_read(stm, buf, sizeof(buf));
        done = len < sizeof(buf);
        status = XML_Parse(parser, buf, len, done);

        if (XML_STATUS_ERROR == status) {
            mume_warning(("Parse xml error: %s at line %"
                          XML_FMT_INT_MOD "u\n",
                          XML_ErrorString(XML_GetErrorCode(parser)),
                          XML_GetCurrentLineNumber(parser)));
            break;
        }

        if (context.error)
            break;

    } while (!done);

    /* When the xml format is invalid, free the stack.  */
    while (context.depth)
        _xml_end_element(&context, NULL);

    _parse_context_dtor(&context);

    XML_ParserFree(parser);

    return (XML_STATUS_OK == status) && (0 == context.error);
}

void mume_serialize_set_object(
    void *self, const char *name, const void *object)
{
    assert(mume_is_of(self, mume_serialize_class()));
    assert(mume_is_of(object, mume_object_class()));

    _serialize_set_object(self, name, mume_clone(object), 0);
}

void mume_serialize_set_static_object(
    void *self, const char *name, const void *object)
{
    unsigned int flags = 0;

    assert(mume_is_of(self, mume_serialize_class()));
    assert(mume_is_of(object, mume_object_class()));

    mume_add_flag(flags, _SER_FLAG_NAMESTATIC);
    mume_add_flag(flags, _SER_FLAG_OBJSTATIC);

    _serialize_set_object(self, name, (void*)object, flags);
}

const void* mume_serialize_get_object(
    const void *_self, const char *name)
{
    const struct _serialize *self = _self;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_serialize_class()));

    node = mume_oset_find(self->objs, &name);
    if (node)
        return ((struct _serobj*)mume_oset_data(node))->object;

    return NULL;
}
