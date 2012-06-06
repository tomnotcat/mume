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
#include "mume-docmgr.h"
#include "mume-docdoc.h"
#include "mume-gstate.h"

#define _docmgr_super_class mume_object_class

struct _docloader {
    /* File type. */
    int type;
    /* Document class list. */
    mume_list_t *list;
};

struct _docmgr {
    const char _[MUME_SIZEOF_OBJECT];
    /* Document loaders. */
    mume_oset_t *ldrs;
    /* Loaded documents. */
    mume_list_t *docs;
};

MUME_STATIC_ASSERT(sizeof(struct _docmgr) == MUME_SIZEOF_DOCMGR);

static int _docloader_compare(const void *a, const void *b)
{
    const struct _docloader *lr1 = a;
    const struct _docloader *lr2 = b;
    return lr1->type - lr2->type;
}

static void _docloader_destruct(void *obj, void *p)
{
    struct _docloader *lr = obj;
    mume_list_delete(lr->list);
}

static void* _docmgr_ctor(
    struct _docmgr *self, int mode, va_list *app)
{
    if (!_mume_ctor(_docmgr_super_class(), self, mode, app))
        return NULL;

    self->ldrs = mume_oset_new(
        _docloader_compare, _docloader_destruct, NULL);
    self->docs = mume_list_new(mume_object_destruct, NULL);

    return self;
}

static void* _docmgr_dtor(struct _docmgr *self)
{
    mume_list_delete(self->docs);
    mume_oset_delete(self->ldrs);

    return _mume_dtor(_docmgr_super_class(), self);
}

const void* mume_docmgr_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_docmgr_meta_class(),
        "docmgr",
        _docmgr_super_class(),
        sizeof(struct _docmgr),
        MUME_PROP_END,
        _mume_ctor, _docmgr_ctor,
        _mume_dtor, _docmgr_dtor,
        MUME_FUNC_END);
}

void mume_docmgr_register(void *_self, int type, const void *clazz)
{
    struct _docmgr *self = _self;
    struct _docloader *lr;
    mume_list_node_t *ln;
    mume_oset_node_t *sn;

    assert(mume_is_of(_self, mume_docmgr_class()));

    sn = mume_oset_find(self->ldrs, &type);
    if (sn) {
        lr = (struct _docloader*)mume_oset_data(sn);
    }
    else {
        sn = mume_oset_newnode(sizeof(struct _docloader));
        lr = (struct _docloader*)mume_oset_data(sn);
        lr->type = type;
        lr->list = mume_list_new(NULL, NULL);
        mume_oset_insert(self->ldrs, sn);
    }

    ln = mume_list_push_front(lr->list, sizeof(void*));
    *(const void**)mume_list_data(ln) = clazz;
}

void* mume_docmgr_load(void *_self, int type, mume_stream_t *stm)
{
    struct _docmgr *self = _self;
    struct _docloader *lr;
    void **ldr;
    mume_oset_node_t *sn;
    mume_list_node_t *ln;

    assert(mume_is_of(_self, mume_docmgr_class()));

    if (MUME_FILETYPE_UNKNOWN == type) {
        /* Check file type. */
        type = mume_filetc_check_magic(mume_filetc(), stm);
    }

    sn = mume_oset_find(self->ldrs, &type);
    if (NULL == sn) {
        mume_warning(("Unsupported file type: %d\n", type));
        return NULL;
    }

    lr = (struct _docloader*)mume_oset_data(sn);
    mume_list_foreach(lr->list, ln, ldr) {
        void *doc = mume_new(*ldr);
        if (_mume_docdoc_load(NULL, doc, stm)) {
            ln = mume_list_push_back(self->docs, sizeof(void*));
            *(void**)mume_list_data(ln) = doc;
            return doc;
        }
        mume_delete(doc);
    }

    return NULL;
}

void* mume_docmgr_load_file(void *self, const char *file)
{
    void *doc;
    int type = mume_filetc_check_ext(mume_filetc(), file);
    mume_stream_t *stm = mume_file_stream_open(file, MUME_OM_READ);

    if (NULL == stm)
        return NULL;

    doc = mume_docmgr_load(self, type, stm);
    mume_stream_close(stm);

    return doc;
}
