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
#include "mume-objbase.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-oset.h"
#include "mume-stream.h"
#include "mume-types.h"
#include "mume-userdata.h"
#include "mume-vector.h"
#include "mume-virtfs.h"
#include MUME_ASSERT_H
#include MUME_CTYPE_H
#include MUME_EXPAT_H
#include MUME_STRING_H

typedef struct mume_objlink_s {
    const char *name;
    const char *target;
} mume_objlink_t;

static int _strstr_compare(const void *a, const void *b)
{
    return _mume_type_string_compare(
        *(void***)a, *(void***)b);
}

static void _objns_construct(
    mume_objns_t *ns, mume_objns_t *p)
{
    /* don't set ns->name, it is intended to be set outside */
    ns->pnt = p;
    ns->subs = NULL;
    ns->types = NULL;
    ns->objs = NULL;
    ns->lnks = NULL;
}

static void _objns_destruct(void *obj, void *p)
{
    mume_objns_t *ns = obj;
    if (ns->lnks)
        mume_oset_delete(ns->lnks);
    if (ns->objs)
        mume_oset_delete(ns->objs);
    if (ns->types)
        mume_oset_delete(ns->types);
    if (ns->subs)
        mume_oset_delete(ns->subs);
}

/* resolve a name that may contain namespace and link,
 * return the name with no prefix namespace and its
 * "real" namespace. (e.g. myspace:sub:myname > myname) */
static const char* _resolve_name(
    mume_objns_t **ns, const char *nm, int add)
{
    mume_objns_t *n = *ns;
    const char *c, *s = nm;
    mume_objns_t *an;
    const char *as;
_resolve:
    an = n;
    as = s;
    if (':' == *s) {
        /* absolute path, begin at root */
        ++s;
        while (n->pnt)
            n = n->pnt;
    }
    /* handle namespace delimiter ':' */
    while ((c = strchr(s, ':'))) {
        char buf[256];
        size_t len = c - s;
        if (len >= sizeof(buf)) {
            len = sizeof(buf) - 1;
            mume_warning(("name too long: %s\n", s));
        }
        strncpy(buf, s, len);
        buf[len] = '\0';
        n = mume_objns_getsub(n, buf, add);
        if (n) {
            s = c + 1;
        }
        else if (an->pnt && ':' != *as) {
            /* search parent namespace if path
               is not in absolute form. */
            an = an->pnt;
            n = an;
            s = as;
        }
        else
            break;
    }
    /* handle link */
    if (n && n->lnks) {
        /* FIXME: check for circular link */
        mume_oset_node_t *nd;
        nd = mume_oset_find(n->lnks, &s);
        if (nd) {
            s = ((mume_objlink_t*)(
                mume_oset_data(nd)))->target;
            goto _resolve;
        }
    }
    *ns = n;
    return s;
}

static mume_objtype_t* _objns_gettype(
    mume_objns_t *ns, const char *nm)
{
    mume_oset_node_t *nd;
    const char **ss = &nm;
    nm = _resolve_name(&ns, nm, 0);
    while (ns) {
        if (ns->types) {
            nd = mume_oset_find(ns->types, &ss);
            if (nd)
                return *(mume_objtype_t**)mume_oset_data(nd);
        }
        ns = ns->pnt;
    }
    return NULL;
}

static int _name_exists(mume_objns_t *ns, const char *nm)
{
    if (ns->subs && mume_oset_find(ns->subs, &nm))
        return 1;
    if (ns->objs && mume_oset_find(ns->objs, &nm))
        return 1;
    if (ns->lnks && mume_oset_find(ns->lnks, &nm))
        return 1;
    return 0;
}

static inline int _isalnumus(int c)
{
    return isalnum(c) || '_' == c;
}

static int _name_invalid(const char *nm)
{
    const char *s = nm;
    if (isalpha(*s) || *s++ == '_')
        while (_isalnumus(*s++));
    if (s[-1] || (nm + 1 == s)) {
        mume_warning(("invalid name: %s\n", nm));
        return 1;
    }
    return 0;
}

static void _objtype_destruct(void *obj, void *p)
{
    mume_objtype_destroy(*(mume_objtype_t**)obj);
}

static void _objdesc_destruct(void *obj, void *p)
{
    mume_objdesc_t *desc = obj;
    mume_type_objdes(
        mume_objdesc_type(desc),
        mume_objdesc_data(desc));

    mume_user_data_delete(desc->udatas);
    mume_objtype_destroy(desc->type);
}

mume_objbase_t* mume_objbase_construct(
    mume_objbase_t *base)
{
    base->root.name = "";
    _objns_construct(&base->root, NULL);
    return base;
}

mume_objbase_t* mume_objbase_destruct(
    mume_objbase_t *base)
{
    _objns_destruct(&base->root, NULL);
    return base;
}

void mume_objdesc_set_user_data(
    mume_objdesc_t *od, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy)
{
    if (NULL == od->udatas)
        od->udatas = mume_user_data_new();

    mume_user_data_set(od->udatas, key, data, destroy);
}

void* mume_objdesc_get_user_data(
    mume_objdesc_t *od, const mume_user_data_key_t *key)
{
    if (od->udatas)
        return mume_user_data_get(od->udatas, key);

    return NULL;
}

mume_objns_t* mume_objbase_getns(
    mume_objbase_t *base, const char *name, int add)
{
    mume_objns_t *ns = mume_objbase_root(base);
    if (name) {
        name = _resolve_name(&ns, name, add);
        if (ns && *name)
            ns = mume_objns_getsub(ns, name, add);
    }
    return ns;
}

mume_objns_t* mume_objns_getsub(
    mume_objns_t *pnt, const char *name, int add)
{
    mume_oset_node_t *nd;
    assert(pnt && name);
    if (pnt->subs) {
        nd = mume_oset_find(pnt->subs, &name);
        if (nd)
            return (mume_objns_t*)mume_oset_data(nd);
    }

    if (add) {
        if (_name_exists(pnt, name) || _name_invalid(name)) {
            return NULL;
        }
        else if (NULL == pnt->subs) {
            pnt->subs = mume_oset_new(
                _mume_type_string_compare, _objns_destruct, NULL);
        }

        nd = mume_oset_new_name_node(name, sizeof(mume_objns_t));
        mume_oset_insert(pnt->subs, nd);
        _objns_construct(
            (mume_objns_t*)mume_oset_data(nd), pnt);

        return (mume_objns_t*)mume_oset_data(nd);
    }
    return NULL;
}

mume_objtype_t* mume_objtype_create(
    const char *name, mume_type_t *type)
{
    mume_objtype_t *t;
    assert(name && type);
    if (_name_invalid(name))
        return NULL;
    t = malloc_abort(sizeof(mume_objtype_t) + strlen(name) + 1);
    t->name = (char*)t + sizeof(mume_objtype_t);
    strcpy((char*)t->name, name);
    t->type = mume_type_reference(type);
    t->refcount = 1;
    return t;
}

void mume_objtype_destroy(mume_objtype_t *type)
{
    if (0 == --type->refcount) {
        mume_type_destroy(type->type);
        free(type);
    }
}

mume_objtype_t* mume_objns_addtype(
    mume_objns_t *ns, mume_objtype_t *type)
{
    mume_oset_node_t *nd;
    assert(ns && type);
    if (NULL == ns->types) {
        ns->types = mume_oset_new(
            _strstr_compare, _objtype_destruct, NULL);
    }

    nd = mume_oset_newnode(sizeof(mume_objtype_t*));
    *(mume_objtype_t**)mume_oset_data(nd) = type;
    if (mume_oset_insert(ns->types, nd)) {
        mume_objtype_reference(type);
        return type;
    }
    mume_oset_delnode(nd);
    return NULL;
}

mume_objdesc_t* mume_objns_addobj(
    mume_objns_t *ns, const char *type, const char *name)
{
    mume_objdesc_t *obj;
    mume_oset_node_t *nd;
    mume_objtype_t *t;
    assert(ns && type && name);
    t = _objns_gettype(ns, type);
    if (NULL == t || _name_exists(ns, name)
        || _name_invalid(name))
    {
        return NULL;
    }
    else if (NULL == ns->objs) {
        ns->objs = mume_oset_new(
            _mume_type_string_compare, _objdesc_destruct, NULL);
    }

    nd = mume_oset_new_name_node(
        name, sizeof(mume_objdesc_t) +
        mume_type_size(t->type));
    /* _name_exists already checked name conflict,
       insert should always success. */
    mume_oset_insert(ns->objs, nd);
    obj = (mume_objdesc_t*)mume_oset_data(nd);
    obj->type = mume_objtype_reference(t);
    obj->udatas = NULL;
    mume_type_objcon(
        t->type, mume_objdesc_data(obj));
    return obj;
}

mume_objdesc_t* mume_objns_getobj(
    mume_objns_t *ns, const char *name)
{
    mume_oset_node_t *nd;
    assert(ns && name);
    name = _resolve_name(&ns, name, 0);
    while (ns) {
        if (ns->objs) {
            nd = mume_oset_find(ns->objs, &name);
            if (nd)
                return (mume_objdesc_t*)mume_oset_data(nd);
        }
        ns = ns->pnt;
    }
    return NULL;
}

int mume_objns_link(
    mume_objns_t *ns, const char *name, const char *tgt)
{
    mume_objlink_t *lnk;
    mume_oset_node_t *nd;
    assert(ns && name && tgt);
    if (_name_exists(ns, name) || _name_invalid(name)) {
        return 0;
    }
    else if (NULL == ns->lnks) {
        ns->lnks = mume_oset_new(
            _mume_type_string_compare, NULL, NULL);
    }

    nd = mume_oset_new_name_node(
        name, sizeof(mume_objlink_t) + strlen(tgt) + 1);
    /* _name_exists already checked name conflict,
       insert should always success. */
    mume_oset_insert(ns->lnks, nd);
    lnk = (mume_objlink_t*)mume_oset_data(nd);
    lnk->target = (char*)lnk + sizeof(mume_objlink_t);
    strcpy((char*)lnk->target, tgt);
    return 1;
}

#ifdef XML_LARGE_SIZE
# if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#  define XML_FMT_INT_MOD "I64"
# else
#  define XML_FMT_INT_MOD "ll"
# endif
#else
# define XML_FMT_INT_MOD "l"
#endif

typedef struct _xml_parse_context_s {
    mume_objns_t *ns;
    mume_type_t *t;
    void *d;
} _xml_parse_context_t;

typedef struct _xml_parse_data_s {
    size_t depth;
    mume_virtfs_t *vfs;
    mume_objbase_t *base;
    mume_objns_t *initns;
    mume_vector_t stack;
} _xml_parse_data_t;

static int _objbase_load_xml(
    mume_objbase_t *base, mume_objns_t *ns,
    mume_virtfs_t *vfs, mume_stream_t *stm);

static void _xml_parse_include(
    _xml_parse_data_t *d, const char **attr)
{
    int i;
    _xml_parse_context_t *c;
    if (NULL == d->vfs) {
        mume_warning(("can't include file, no VFS provided\n"));
        return;
    }

    c = (_xml_parse_context_t*)mume_vector_back(&d->stack);
    assert(c->ns);
    for (i = 0; attr[i]; i += 2) {
        if (0 == strcmp(attr[i], "file")) {
            mume_stream_t *stm;
            stm = mume_virtfs_open_read(d->vfs, attr[i + 1]);
            if (stm) {
                /* FIXME: check for multiple inclusion */
                _objbase_load_xml(
                    d->base, c->ns, d->vfs, stm);
                mume_stream_close(stm);
            }
            else {
                mume_warning(("can't open include file: %s\n", attr[i + 1]));
            }
            break;
        }
        else {
            mume_warning(("unknown attribute: %s\n", attr[i]));
        }
    }
}

static void _xml_parse_namespace(
    _xml_parse_data_t *d, const char **attr)
{
    int i;
    mume_objns_t *ps, *ns;
    _xml_parse_context_t *c;
    const char *name = NULL;
    c = (_xml_parse_context_t*)mume_vector_back(&d->stack);
    assert(c->ns);
    ps = c->ns;
    for (i = 0; attr[i]; i += 2) {
        if (0 == strcmp(attr[i], "name")) {
            name = attr[i + 1];
        }
        else {
            mume_warning(("unknown attribute: %s\n", attr[i]));
        }
    }

    if (NULL == name) {
        mume_warning(("namespace name not found\n"));
        return;
    }

    ns = mume_objns_getsub(ps, name, 1);
    if (NULL == ns) {
        mume_warning(("add namespace failed: %s:%s\n", ps->name, name));
        return;
    }
    c = mume_vector_push_back(&d->stack);
    c->ns = ns;
    c->t = NULL;
    c->d = NULL;
}

static void _xml_parse_link(
    _xml_parse_data_t *d, const char **attr)
{
    int i;
    _xml_parse_context_t *c;
    const char *name = NULL;
    const char *tgt = NULL;
    c = (_xml_parse_context_t*)mume_vector_back(&d->stack);
    assert(c->ns);
    for (i = 0; attr[i]; i += 2) {
        if (0 == strcmp(attr[i], "name")) {
            name = attr[i + 1];
        }
        else if (0 == strcmp(attr[i], "target")) {
            tgt = attr[i + 1];
        }
        else {
            mume_warning(("unknown attribute: %s\n", attr[i]));
        }
    }

    if (NULL == name || NULL == tgt) {
        mume_warning(("invalid link: %s - %s\n",
                      name ? name : "NULL",
                      tgt ? tgt : "NULL"));
        return;
    }
    else if (!mume_objns_link(c->ns, name, tgt)) {
        mume_warning(("add link failed: %s: %s - %s\n",
                      c->ns->name, name, tgt));
        return;
    }
}

static void _xml_setval_error(
    const char *name, const char *prop, const char *value)
{
    mume_warning(("set property failed: %s.%s = %s\n",
                  name, prop, value));
}

static void _xml_parse_element(
    mume_type_t *type, void *obj,
    const char *name, const char **attr)
{
    int i;
    mume_prop_t *prop;
    switch (type->type) {
    case MUME_TYPE_SIMPLE:
        for (i = 0; attr[i]; i += 2) {
            if (strcmp(attr[i], "value")) {
                mume_warning(("unknown attribute: %s\n", attr[i]));
                continue;
            }
            if (!mume_type_setstr(type, obj, attr[i + 1]))
                _xml_setval_error(name, attr[i], attr[i + 1]);
            break;
        }
        break;
    case MUME_TYPE_COMPOUND:
        for (i = 0; attr[i]; i += 2) {
            prop = mume_type_prop(type, attr[i]);
            if (NULL == prop) {
                mume_warning(("unknown property: %s\n", attr[i]));
                continue;
            }
            switch (prop->prop_type) {
            case MUME_PROP_SIMPLE:
                if (!mume_prop_setstr(prop, obj, attr[i + 1]))
                    _xml_setval_error(name, attr[i], attr[i + 1]);
                break;
            case MUME_PROP_DIRECT:
                if (!mume_type_setstr(
                        mume_prop_type(prop), (char*)obj +
                        mume_prop_offset(prop), attr[i + 1]))
                {
                    _xml_setval_error(name, attr[i], attr[i + 1]);
                }
                break;
            }
        }
        break;
    }
}

static void _xml_parse_subobject(
    _xml_parse_data_t *d, const char *name, const char **attr)
{
    int i;
    mume_type_t *pt;
    void *pd;
    _xml_parse_context_t *c;
    mume_prop_t *prop;
    c = (_xml_parse_context_t*)mume_vector_back(&d->stack);
    assert(c->t && c->d);
    pt = c->t;
    pd = c->d;
    switch (pt->type) {
    case MUME_TYPE_COMPOUND:
        prop = mume_type_prop(pt, name);
        if (NULL == prop) {
            mume_warning(("unknown property: %s\n", name));
            break;
        }
        switch (prop->prop_type) {
        case MUME_PROP_SIMPLE:
            for (i = 0; attr[i]; i += 2) {
                if (strcmp(attr[i], "value")) {
                    mume_warning(("unknown attribute: %s\n", attr[i]));
                    continue;
                }
                if (!mume_prop_setstr(prop, pd, attr[i + 1]))
                    _xml_setval_error(name, attr[i], attr[i + 1]);
                break;
            }
            break;
        case MUME_PROP_DIRECT:
            c = mume_vector_push_back(&d->stack);
            c->t = mume_prop_type(prop);
            c->d = (char*)pd + mume_prop_offset(prop);
            _xml_parse_element(c->t, c->d, name, attr);
            break;
        }
        break;
    case MUME_TYPE_CONTAINER:
        c = mume_vector_push_back(&d->stack);
        c->t = mume_type_ctnr_vtype(pt);
        c->d = mume_type_newobj(c->t);
        _xml_parse_element(c->t, c->d, name, attr);
        /* element will be inserted after its value
           been parsed, because some container (like set)
           need the value to be set before insert. */
        break;
    }
}

static void _xml_parse_object(
    _xml_parse_data_t *d, const char **attr)
{
    int i;
    const char *type = NULL;
    const char *name = NULL;
    const char *value = NULL;
    mume_objns_t *ns;
    mume_objdesc_t *obj;
    _xml_parse_context_t *c;
    for (i = 0; attr[i]; i += 2) {
        if (0 == strcmp(attr[i], "type")) {
            type = attr[i + 1];
        }
        else if (0 == strcmp(attr[i], "name")) {
            name = attr[i + 1];
        }
        else if (0 == strcmp(attr[i], "value")) {
            value = attr[i + 1];
        }
        else {
            mume_warning(("unknown attribute: %s\n", attr[i]));
        }
    }

    if (NULL == name) {
        mume_warning(("object name not found\n"));
        return;
    }
    else if (NULL == type) {
        /* facility for "sigleton" object */
        type = name;
    }

    c = mume_vector_back(&d->stack);
    assert(c->ns);
    ns = c->ns;
    obj = mume_objns_addobj(ns, type, name);
    if (NULL == obj) {
        mume_warning(("add object failed: %s:%s:%s\n",
                      ns->name, type, name));
        return;
    }

    c = mume_vector_push_back(&d->stack);
    c->ns = NULL;
    c->t = mume_objdesc_type(obj);
    c->d = mume_objdesc_data(obj);
    if (value && !mume_type_setstr(c->t, c->d, value)) {
        mume_warning(("set value failed: %s:%s:%s = %s\n",
                      ns->name, type, name, value));
    }
}

static void _xml_start_element(
    void *data, const char *el, const char **attr)
{
    _xml_parse_context_t *c;
    _xml_parse_data_t *d = data;
    if (d->depth == mume_vector_size(&d->stack)) {
        if (d->depth) {
            c = (_xml_parse_context_t*)mume_vector_back(&d->stack);
            if (c->t && c->d) {
                _xml_parse_subobject(d, el, attr);
            }
            else if (0 == strcmp(el, "object")) {
                _xml_parse_object(d, attr);
            }
            else if (0 == strcmp(el, "link")) {
                _xml_parse_link(d, attr);
            }
            else if (0 == strcmp(el, "namespace")) {
                _xml_parse_namespace(d, attr);
            }
            else if (0 == strcmp(el, "include")) {
                _xml_parse_include(d, attr);
            }
            else {
                mume_warning(("unknown tag: %s\n", el));
            }
        }
        else {
            /* root node */
            c = (_xml_parse_context_t*)(
                mume_vector_push_back(&d->stack));
            c->ns = d->initns;
            c->t = NULL;
            c->d = NULL;
        }
    }
    d->depth += 1;
}

static void _xml_end_element(void *data, const char *el)
{
    _xml_parse_data_t *d = data;
    if (d->depth == mume_vector_size(&d->stack)) {
        mume_type_t *pt;
        void *pd;
        _xml_parse_context_t *c;
        c = (_xml_parse_context_t*)(
            mume_vector_back(&d->stack));
        pt = c->t;
        pd = c->d;
        mume_vector_pop_back(&d->stack);
        if (mume_vector_size(&d->stack)) {
            c = (_xml_parse_context_t*)(
                mume_vector_back(&d->stack));
            /* insert the container element */
            if (c->t && mume_type_is_container(c->t)) {
                assert(pt == mume_type_ctnr_vtype(c->t));
                if (el) {
                    mume_type_ctnr_insert(c->t, c->d, pd);
                }
                mume_type_delobj(pt, pd);
            }
        }
    }
    d->depth -= 1;
}

static int _objbase_load_xml(
    mume_objbase_t *base, mume_objns_t *ns,
    mume_virtfs_t *vfs, mume_stream_t *stm)
{
    char buf[1024];
    size_t len;
    int done;
    int status;
    XML_Parser parser = XML_ParserCreate(NULL);
    _xml_parse_data_t data;
    data.depth = 0;
    data.vfs = vfs;
    data.base = base;
    data.initns = ns;
    mume_vector_ctor(
        &data.stack, sizeof(_xml_parse_context_t), NULL, NULL);
    XML_SetUserData(parser, &data);
    XML_SetElementHandler(
        parser, _xml_start_element, _xml_end_element);
    do {
        len = mume_stream_read(stm, buf, sizeof(buf));
        done = len < sizeof(buf);
        status = XML_Parse(parser, buf, len, done);
        if (XML_STATUS_ERROR == status) {
            mume_warning(("parse xml error: %s at line %"
                          XML_FMT_INT_MOD "u\n",
                          XML_ErrorString(XML_GetErrorCode(parser)),
                          XML_GetCurrentLineNumber(parser)));
            break;
        }
    } while (!done);

    /* when the xml format is invalid, free the stack  */
    while (data.depth)
        _xml_end_element(&data, NULL);

    mume_vector_dtor(&data.stack);
    XML_ParserFree(parser);
    return XML_STATUS_OK == status;
}

int mume_objbase_load_xml(
    mume_objbase_t *base, mume_virtfs_t *vfs, mume_stream_t *stm)
{
    return _objbase_load_xml(
        base, mume_objbase_root(base), vfs, stm);
}

static void _xml_write_indents(mume_stream_t *stm, int count)
{
    while (count-- > 0) {
        mume_stream_printf(stm, "  ");
    }
}

static void _xml_write_subobject(
    mume_stream_t *stm, mume_type_t *type,
    int indent, const char *name, void *obj)
{
    switch (type->type) {
    case MUME_TYPE_SIMPLE:
        {
            char buf[256];
            assert(name);
            _xml_write_indents(stm, indent);
            mume_type_getstr(type, obj, buf, COUNT_OF(buf));
            mume_stream_printf(stm, "<%s value=\"%s\"/>\n", name, buf);
        }
        break;
    case MUME_TYPE_COMPOUND:
        {
            size_t i;
            char buf[256];
            mume_prop_t *prop;
            mume_type_compound_t *comp;
            comp = (mume_type_compound_t*)type;
            if (name) {
                _xml_write_indents(stm, indent);
                mume_stream_printf(stm, "<%s>\n", name);
                ++indent;
            }

            for (i = 0; i < comp->propc; ++i) {
                prop = comp->props[i];
                if (mume_prop_is_direct(prop)) {
                    _xml_write_subobject(
                        stm, mume_prop_type(prop),
                        indent, prop->name,
                        (char*)obj + mume_prop_offset(prop));
                }
                else if (mume_prop_is_simple(prop)) {
                    mume_prop_getstr(
                        prop, obj, buf, sizeof(buf));
                    mume_stream_printf(
                        stm, "<%s value=\"%s\"/>", prop->name, buf);
                }
            }

            if (name) {
                _xml_write_indents(stm, indent - 1);
                mume_stream_printf(stm, "</%s>\n", name);
            }
        }
        break;
    case MUME_TYPE_CONTAINER:
        {
            void *c, *e;
            mume_type_t *vt;
            c = mume_type_ctnr_begin(type, obj);
            e = mume_type_ctnr_end(type, obj);
            vt = mume_type_ctnr_vtype(type);
            if (name) {
                _xml_write_indents(stm, indent);
                mume_stream_printf(stm, "<%s>\n", name);
                ++indent;
            }

            while (c != e) {
                _xml_write_subobject(
                    stm, vt, indent, "l",
                    mume_type_ctnr_value(type, obj, c));
                c = mume_type_ctnr_next(type, obj, c);
            }

            if (name) {
                _xml_write_indents(stm, indent - 1);
                mume_stream_printf(stm, "</%s>\n", name);
            }
        }
        break;
    }
}

static void _xml_write_namespace(
    mume_stream_t *stm, int indent, mume_objns_t *ns)
{
    mume_oset_node_t *it;
    if (ns->subs) {
        mume_objns_t *sub;
        it = mume_oset_first(ns->subs);
        while (it) {
            sub = (mume_objns_t*)mume_oset_data(it);
            _xml_write_indents(stm, 1);
            mume_stream_printf(
                stm, "<namespace name=\"%s\">\n", sub->name);
            _xml_write_namespace(stm, indent + 1, sub);
            _xml_write_indents(stm, 1);
            mume_stream_printf(stm, "</namespace>\n");
            it = mume_oset_next(it);
        }
    }

    if (ns->objs) {
        mume_objdesc_t *obj;
        it = mume_oset_first(ns->objs);
        while (it) {
            obj = (mume_objdesc_t*)mume_oset_data(it);
            _xml_write_indents(stm, indent);
            if (mume_type_is_simple(mume_objdesc_type(obj))) {
                char buf[256];
                mume_type_getstr(
                    mume_objdesc_type(obj),
                    mume_objdesc_data(obj),
                    buf, sizeof(buf));
                mume_stream_printf(
                    stm, "<object type=\"%s\" name=\"%s\""
                    " value=\"%s\"/>\n",
                    mume_objdesc_type_name(obj),
                    mume_objdesc_name(obj), buf);
            }
            else {
                mume_stream_printf(
                    stm, "<object type=\"%s\" name=\"%s\">\n",
                    mume_objdesc_type_name(obj),
                    mume_objdesc_name(obj));
                _xml_write_subobject(
                    stm, mume_objdesc_type(obj), indent + 1,
                    NULL, mume_objdesc_data(obj));
                _xml_write_indents(stm, indent);
                mume_stream_printf(stm, "</object>\n");
            }
            it = mume_oset_next(it);
        }
    }
}

int mume_objbase_save_xml(
    mume_objbase_t *base, mume_stream_t *stm)
{
    mume_stream_printf(stm, "<root>\n");
    _xml_write_namespace(stm, 1, mume_objbase_root(base));
    mume_stream_printf(stm, "</root>\n");
    return 1;
}
