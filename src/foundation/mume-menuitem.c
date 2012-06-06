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
#include "mume-menuitem.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-ovector.h"
#include MUME_ASSERT_H

#define _menuitem_super_class mume_refobj_class

struct _menuitem {
    const char _[MUME_SIZEOF_REFOBJ];
    int command;
    char *text;
    void *subitems;
};

MUME_STATIC_ASSERT(sizeof(struct _menuitem) == MUME_SIZEOF_MENUITEM);

static void* _menuitem_ctor(
    struct _menuitem *self, int mode, va_list *app)
{
    self->command = 0;
    self->text = NULL;
    self->subitems = NULL;

    if (!_mume_ctor(_menuitem_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->command = va_arg(*app, int);
        self->text = strdup_abort(va_arg(*app, char*));
    }

    return self;
}

static void* _menuitem_dtor(struct _menuitem *self)
{
    free(self->text);
    mume_delete(self->subitems);
    return _mume_dtor(_menuitem_super_class(), self);
}

const void* mume_menuitem_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_menuitem_meta_class(),
        "menuitem",
        _menuitem_super_class(),
        sizeof(struct _menuitem),
        MUME_PROP_END,
        _mume_ctor, _menuitem_ctor,
        _mume_dtor, _menuitem_dtor,
        MUME_FUNC_END);
}

void* mume_menuitem_new(int command, const char *text)
{
    return mume_new(mume_menuitem_class(), command, text);
}

int mume_menuitem_get_command(const void *_self)
{
    const struct _menuitem *self = _self;
    assert(mume_is_of(_self, mume_menuitem_class()));
    return self->command;
}

const char* mume_menuitem_get_text(const void *_self)
{
    const struct _menuitem *self = _self;
    assert(mume_is_of(_self, mume_menuitem_class()));
    return self->text;
}

void mume_menuitem_insert_subitem(void *_self, int index, void *item)
{
    struct _menuitem *self = _self;

    assert(mume_is_of(_self, mume_menuitem_class()));

    if (NULL == self->subitems)
        self->subitems = mume_ovector_new(mume_refobj_release);

    mume_refobj_addref(item);
    mume_ovector_insert(self->subitems, index, item);
}

void mume_menuitem_add_subitem(void *self, void *item)
{
    mume_menuitem_insert_subitem(
        self, mume_menuitem_count_subitems(self), item);
}

void mume_menuitem_remove_subitems(void *_self, int index, int count)
{
    struct _menuitem *self = _self;
    assert(mume_is_of(_self, mume_menuitem_class()));
    mume_ovector_erase(self->subitems, index, count);
}

int mume_menuitem_count_subitems(const void *_self)
{
    const struct _menuitem *self = _self;

    assert(mume_is_of(_self, mume_menuitem_class()));

    if (self->subitems)
        return mume_octnr_size(self->subitems);

    return 0;
}

void* mume_menuitem_get_subitem(const void *_self, int index)
{
    const struct _menuitem *self = _self;

    assert(mume_is_of(_self, mume_menuitem_class()));

    if (self->subitems)
        return mume_ovector_at(self->subitems, index);

    return NULL;
}
