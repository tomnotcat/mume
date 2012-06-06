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
#include "mume-index-view.h"
#include "mume-docdoc.h"

#define _index_view_super_class mume_treeview_class
#define _index_view_super_meta_class mume_treeview_meta_class

struct _index_view {
    const char _[MUME_SIZEOF_TREEVIEW];
    void *doc;
};

struct _index_view_class {
    const char _[MUME_SIZEOF_TREEVIEW_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _index_view) ==
                   MUME_SIZEOF_INDEX_VIEW);

MUME_STATIC_ASSERT(sizeof(struct _index_view_class) ==
                   MUME_SIZEOF_INDEX_VIEW_CLASS);

static void _index_view_reset(struct _index_view *self)
{
    self->doc = NULL;
}

static void _index_view_clear(struct _index_view *self)
{
    if (self->doc)
        mume_refobj_release(self->doc);

    _index_view_reset(self);
}

static void _index_view_build_tree(
    struct _index_view *self, void *parent, mume_tocitem_t *toc)
{
    mume_tocitem_t *it;
    void *last = NULL;

    for (it = toc; it; it = it->next) {
        last = mume_treeview_insert(
            self, parent, last, it->title, 0);

        if (it->child)
            _index_view_build_tree(self, last, it->child);
    }
}

static void* _index_view_ctor(
    struct _index_view *self, int mode, va_list *app)
{
    if (!_mume_ctor(_index_view_super_class(), self, mode, app))
        return NULL;

    _index_view_reset(self);
    return self;
}

static void* _index_view_dtor(struct _index_view *self)
{
    _index_view_clear(self);
    return _mume_dtor(_index_view_super_class(), self);
}

const void* mume_index_view_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_index_view_meta_class(),
        "index view",
        _index_view_super_class(),
        sizeof(struct _index_view),
        MUME_PROP_END,
        _mume_ctor, _index_view_ctor,
        _mume_dtor, _index_view_dtor,
        MUME_FUNC_END);
}

void* mume_index_view_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(mume_index_view_class(),
                    parent, x, y, w, h);
}

void mume_index_view_set_doc(void *_self, void *doc)
{
    struct _index_view *self = _self;
    mume_tocitem_t *toc;

    assert(mume_is_of(_self, mume_index_view_class()));
    assert(!doc || mume_is_of(doc, mume_docdoc_class()));

    _index_view_clear(self);

    self->doc = doc;
    if (NULL == self->doc)
        return;

    mume_refobj_addref(self->doc);

    toc = mume_docdoc_get_toc_tree(self->doc);
    if (toc) {
        _index_view_build_tree(self, mume_treeview_root(self), toc);
        mume_tocitem_destroy(toc);
    }
}

void* mume_index_view_get_doc(const void *_self)
{
    const struct _index_view *self = _self;
    assert(mume_is_of(_self, mume_index_view_class()));
    return self->doc;
}
