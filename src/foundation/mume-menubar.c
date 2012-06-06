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
#include "mume-menubar.h"
#include "mume-backend.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-menuitem.h"
#include "mume-objbase.h"
#include "mume-refobj.h"
#include "mume-resmgr-private.h"
#include "mume-text-layout.h"
#include "mume-types.h"
#include "mume-vector.h"
#include MUME_ASSERT_H

#define _menubar_super_class mume_window_class

#define _menubar_is_popup(_self) \
    mume_test_flag((_self)->flags, _MENUBAR_FLAG_POPUP)

#define _menubar_is_popuped(_self) \
    mume_test_flag((_self)->flags, _MENUBAR_FLAG_POPUPED)

#define _menubar_is_subitem_popuped(_self) \
    ((_self)->popupbar && _menubar_is_popuped( \
        (struct _menubar*)((_self)->popupbar)))

enum _menubar_flags_e {
    _MENUBAR_FLAG_POPUP,
    _MENUBAR_FLAG_POPUPED,
    _MENUBAR_FLAG_ACTIVE,
    _MENUBAR_FLAG_INVALID,
    _MENUBAR_FLAG_AUTODEL
};

struct _menuslot {
    void *item;
    mume_rect_t rect;
    int modified;
};

struct _menubar {
    const char _[MUME_SIZEOF_WINDOW];
    mume_vector_t *slots;
    int selected;
    int popuped;
    void *host;
    void *popupbar;
    unsigned int flags;
};

struct _menubar_theme {
    mume_rect_t popup_margin;
    mume_rect_t baritem_margin;
    mume_rect_t popitem_margin;
    mume_rect_t popmark_size;
    mume_resobj_brush_t barbg;
    mume_resobj_brush_t popupbg;
    mume_resobj_brush_t popmark_normal;
    mume_resobj_brush_t popmark_selected;
    mume_widget_bkgnd_t itembg;
    mume_widget_texts_t texts;
};

MUME_STATIC_ASSERT(sizeof(struct _menubar) == MUME_SIZEOF_MENUBAR);

static void _menuslot_dtor(void *obj, void *p)
{
    struct _menuslot *slot = obj;

    if (slot->item)
        mume_refobj_release(slot->item);
}

static struct _menubar_theme* _menubar_get_theme(
    const struct _menubar *self)
{
    struct _menubar_theme *theme = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "menubar", "theme"),
        mume_typeof_menubar_theme());

    if (NULL == theme)
        mume_abort(("Get menubar theme failed\n"));

    return theme;
}

static int _menubar_item_from_point(
    struct _menubar *self, int x, int y)
{
    int i;
    struct _menuslot *slot;

    mume_vector_foreach(self->slots, i, slot) {
        if (mume_rect_inside(slot->rect, x, y))
            return i;
    }

    return -1;
}

static void _menubar_invalidate_item(struct _menubar *self, int index)
{
    if (index != -1) {
        struct _menuslot *slot = mume_vector_at(self->slots, index);
        mume_invalidate_rect(self, &slot->rect);
    }
}

static void _menubar_measure_item(
    const struct _menubar *self, const void *item,
    struct _menubar_theme *theme, int *width, int *height)
{
    const char *text = mume_menuitem_get_text(item);
    mume_rect_t rect = mume_rect_empty;
    unsigned int format = MUME_TLF_CALCRECT |
                          MUME_TLF_SINGLELINE |
                          MUME_TLF_VCENTER;

    mume_charfmt_draw_text(
        NULL, &theme->texts.normal, format, text, -1, &rect);

    *width = rect.width;
    *height = rect.height;

    if (_menubar_is_popup(self)) {
        *width += theme->popitem_margin.x +
                  theme->popitem_margin.width;

        *height += theme->popitem_margin.y +
                   theme->popitem_margin.height;
    }
    else {
        *width += theme->baritem_margin.x +
                  theme->baritem_margin.width;

        *height += theme->baritem_margin.y +
                   theme->baritem_margin.height;
    }
}

static void _menubar_draw_item(
    const struct _menubar *self, int index, const void *item,
    cairo_t *cr, struct _menubar_theme *theme, mume_rect_t rect)
{
    mume_resobj_brush_t *br;
    mume_resobj_charfmt_t *cf;
    mume_rect_t r;

    const char *text = mume_menuitem_get_text(item);

    unsigned int format = MUME_TLF_DRAWTEXT |
                          MUME_TLF_SINGLELINE |
                          MUME_TLF_VCENTER;

    /* Background. */
    if (self->selected == index) {
        if (_menubar_is_popup(self) ||
            _menubar_is_subitem_popuped(self))
        {
            br = &theme->itembg.pressed;
            cf = &theme->texts.pressed;
        }
        else {
            br = &theme->itembg.hot;
            cf = &theme->texts.hot;
        }
    }
    else {
        br = &theme->itembg.normal;
        cf = &theme->texts.normal;
    }

    mume_draw_resobj_brush(
        cr, br, rect.x, rect.y, rect.width, rect.height);

    /* Text. */
    if (_menubar_is_popup(self)) {
        r = mume_rect_inflate(
            rect, -theme->popitem_margin.x,
            -theme->popitem_margin.y,
            -theme->popitem_margin.width,
            -theme->popitem_margin.height);
    }
    else {
        r = mume_rect_inflate(
            rect, -theme->baritem_margin.x,
            -theme->baritem_margin.y,
            -theme->baritem_margin.width,
            -theme->baritem_margin.height);
    }

    mume_charfmt_draw_text(cr, cf, format, text, -1, &r);

    /* Popup mark. */
    if (_menubar_is_popup(self) &&
        mume_menuitem_count_subitems(item))
    {
        if (self->selected == index) {
            br = &theme->popmark_selected;
        }
        else {
            br = &theme->popmark_normal;
        }

        mume_draw_resobj_brush(
            cr, br, rect.x + rect.width - theme->popmark_size.x,
            rect.y + (rect.height - theme->popmark_size.y) / 2,
            theme->popmark_size.x, theme->popmark_size.y);
    }
}

static void _menubar_layout_items(const struct _menubar *self)
{
    int i, x, y;
    int max_width = 0;
    struct _menuslot *slot;
    struct _menubar_theme *theme;

    theme = _menubar_get_theme(self);
    x = theme->popup_margin.x;
    y = theme->popup_margin.y;

    mume_vector_foreach(self->slots, i, slot) {
        if (slot->modified) {
            if (slot->item) {
                _menubar_measure_item(
                    self, slot->item, theme,
                    &slot->rect.width, &slot->rect.height);
            }
            else {
                slot->rect.width = 0;
                slot->rect.height = 0;
            }

            slot->modified = 0;
        }

        slot->rect.x = x;
        slot->rect.y = y;

        if (_menubar_is_popup(self)) {
            y += slot->rect.height;
        }
        else {
            x += slot->rect.width;
        }

        max_width = MAX(max_width, slot->rect.width);
    }

    if (_menubar_is_popup(self)) {
        mume_vector_foreach(self->slots, i, slot) {
            slot->rect.width = max_width;
        }
    }
}

static void _menubar_update_structure(struct _menubar *self)
{
    if (mume_test_flag(self->flags, _MENUBAR_FLAG_INVALID)) {
        _menubar_layout_items(self);
        mume_remove_flag(self->flags, _MENUBAR_FLAG_INVALID);
    }
}

static void _menubar_get_preferred_size(
    struct _menubar *self, int *width, int *height)
{
    int count = mume_menubar_count_items(self);

    _menubar_update_structure(self);

    if (count > 0) {
        struct _menuslot *slot;
        struct _menubar_theme *theme;

        theme = _menubar_get_theme(self);

        slot = mume_vector_at(self->slots, count - 1);

        if (width) {
            *width = slot->rect.x + slot->rect.width +
                     theme->popup_margin.width;
        }

        if (height) {
            *height = slot->rect.y + slot->rect.height +
                      theme->popup_margin.height;
        }
    }
    else {
        if (width)
            *width = 16;

        if (height)
            *height = 16;
    }
}

static void _adjust_popup_rect(
    mume_rect_t *rect, const mume_rect_t *exclude)
{
    int cx, cy, pos, vert;

    mume_screen_size(&cx, &cy);

    if (exclude) {
        vert = (rect->y + rect->height <= exclude->y ||
                rect->y >= exclude->y + exclude->height);
    }
    else {
        vert = 0;
    }

    pos = rect->x;

    if (rect->x < 0) {
        if (exclude) {
            if (vert)
                pos = 0;
            else
                pos = exclude->x + exclude->width;
        }
        else {
            pos = 0;
        }
    }
    else if (rect->x + rect->width > cx) {
        if (exclude) {
            if (vert)
                pos = cx - rect->width;
            else
                pos = exclude->x - rect->width;
        }
        else {
            pos = rect->x - rect->width;
        }
    }

    if (pos >= 0 && pos + rect->width <= cx)
        rect->x = pos;

    pos = rect->y;

    if (rect->y < 0) {
        if (exclude) {
            if (vert)
                pos = exclude->y + exclude->height;
            else
                pos = 0;
        }
        else {
            pos = 0;
        }
    }
    else if (rect->y + rect->height > cy) {
        if (exclude) {
            if (vert)
                pos = exclude->y - rect->height;
            else
                pos = cy - rect->height;
        }
        else {
            pos = rect->y - rect->height;
        }
    }

    if (pos >= 0 && pos + rect->height <= cy)
        rect->y = pos;

    if (exclude) {
        if (rect->y < 0 ||
            rect->y + rect->height > cy)
        {
            if (rect->y < 0)
                rect->y = 0;
            else
                rect->y = cy - rect->height;

            if (exclude->x + exclude->width + rect->width <= cx)
                rect->x = exclude->x + exclude->width;
            else
                rect->x = exclude->x - rect->width;
        }
    }
}

static void _menubar_popup_item(struct _menubar *self, int index)
{
    void *item, *sub;
    int i, count, x, y;
    mume_rect_t exclude;
    struct _menuslot *slot;

    if (self->popuped == index)
        return;

    if (_menubar_is_subitem_popuped(self))
        mume_menubar_popdown(self->popupbar);

    self->popuped = index;

    if (-1 == index)
        return;

    item = mume_menubar_get_item(self, index);
    count = mume_menuitem_count_subitems(item);

    if (0 == count)
        return;

    if (NULL == self->popupbar)
        self->popupbar = mume_menubar_new_popup(self);

    mume_menubar_clear_items(self->popupbar);
    for (i = 0; i < count; ++i) {
        sub = mume_menuitem_get_subitem(item, i);
        mume_menubar_add_item(self->popupbar, sub);
    }

    slot = mume_vector_at(self->slots, index);
    if (_menubar_is_popup(self)) {
        x = slot->rect.x + slot->rect.width;
        y = slot->rect.y;
    }
    else {
        x = slot->rect.x;
        y = slot->rect.y + slot->rect.height;
    }

    mume_translate_coords(self, NULL, &x, &y);
    exclude = slot->rect;
    mume_translate_coords(self, NULL, &exclude.x, &exclude.y);
    mume_menubar_popup(self->popupbar, x, y, &exclude, 0);
}

static void _menubar_select_item(struct _menubar *self, int index)
{
    if (self->selected != index) {
        _menubar_invalidate_item(self, self->selected);
        self->selected = index;
        _menubar_invalidate_item(self, self->selected);
    }
}

static void _menubar_invalidate_structure(struct _menubar *self)
{
    if (!mume_test_flag(self->flags, _MENUBAR_FLAG_INVALID)) {
        mume_invalidate_region(self, NULL);

        _menubar_popup_item(self, -1);
        _menubar_select_item(self, -1);

        mume_add_flag(self->flags, _MENUBAR_FLAG_INVALID);
    }
}

static void _menubar_close_popup(struct _menubar *self, void *item)
{
    mume_event_t event = mume_make_notify_event(
        self, self, MUME_MENUBAR_CLOSE, item);

    mume_send_event(&event);
}

static void* _menubar_ctor(
    struct _menubar *self, int mode, va_list *app)
{
    self->slots = mume_vector_new(
        sizeof(struct _menuslot), _menuslot_dtor, NULL);
    self->flags = 0;
    self->selected = -1;
    self->popuped = -1;
    self->host = NULL;
    self->popupbar = NULL;

    if (!_mume_ctor(_menubar_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->host = va_arg(*app, void*);
        self->flags = va_arg(*app, unsigned int);

        assert(mume_is_of(self->host, mume_window_class()));
    }

    if (mume_test_flag(self->flags, _MENUBAR_FLAG_POPUP)) {
        void *bwin = mume_backend_create_backwin(
            mume_backend(), MUME_BACKWIN_MENU,
            mume_root_backwin(), 0, 0, 1, 1);

        if (bwin) {
            mume_window_set_backwin(self, bwin);
            mume_refobj_release(bwin);
        }
    }

    return self;
}

static void* _menubar_dtor(struct _menubar *self)
{
    mume_delete(self->popupbar);

    if (self->host) {
        mume_event_t event = mume_make_notify_event(
            self->host, self, MUME_MENUBAR_DELETE, NULL);

        mume_send_event(&event);
    }

    mume_vector_delete(self->slots);
    return _mume_dtor(_menubar_super_class(), self);
}

static void _menubar_handle_button_down(
    struct _menubar *self, int x, int y, int state, int button)
{
    int index = _menubar_item_from_point(self, x, y);

    _menubar_select_item(self, index);

    if (_menubar_is_popup(self)) {
    }
    else {
        if (_menubar_is_subitem_popuped(self)) {
            _menubar_popup_item(self, -1);
        }
        else {
            _menubar_popup_item(self, index);
            _menubar_invalidate_item(self, index);
        }
    }
}

static void _menubar_handle_button_up(
    struct _menubar *self, int x, int y, int state, int button)
{
    int index = _menubar_item_from_point(self, x, y);

    if (index != -1) {
        void *item = mume_menubar_get_item(self, index);
        if (0 == mume_menuitem_count_subitems(item)) {
            _menubar_close_popup(self, item);
        }
    }
    else if (_menubar_is_popup(self)) {
        _menubar_close_popup(self, NULL);
    }
}

static void _menubar_handle_mouse_motion(
    struct _menubar *self, int x, int y, int state)
{
    int index = _menubar_item_from_point(self, x, y);

    if (_menubar_is_popup(self)) {
        if (index != -1) {
            _menubar_popup_item(self, index);
            _menubar_select_item(self, index);
        }
    }
    else {
        if (_menubar_is_subitem_popuped(self)) {
            if (index != -1) {
                if (index != self->popuped)
                    _menubar_popup_item(self, index);
                _menubar_select_item(self, index);
            }
        }
        else {
            _menubar_select_item(self, index);
        }
    }
}

static void _menubar_handle_mouse_leave(
    struct _menubar *self, int x, int y, int state, int mode, int detail)
{
    if (!_menubar_is_subitem_popuped(self))
        _menubar_select_item(self, -1);
}

static void _menubar_handle_expose(
    struct _menubar *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    mume_rect_t r;
    int i;
    struct _menuslot *slot;
    struct _menubar_theme *theme;

    if (count)
        return;

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (!cr)
        return;

    _menubar_update_structure(self);
    theme = _menubar_get_theme(self);

    /* Background. */
    r.x = r.y = 0;
    mume_window_get_geometry(
        self, NULL, NULL, &r.width, &r.height);

    if (_menubar_is_popup(self)) {
        mume_draw_resobj_brush(
            cr, &theme->popupbg, r.x, r.y, r.width, r.height);
    }
    else {
        mume_draw_resobj_brush(
            cr, &theme->barbg, r.x, r.y, r.width, r.height);
    }

    /* Menu items. */
    r = mume_current_invalid_rect();

    mume_vector_foreach(self->slots, i, slot) {
        if (!mume_rect_is_empty(
                mume_rect_intersect(slot->rect, r)))
        {
            _menubar_draw_item(
                self, i, slot->item, cr, theme, slot->rect);
        }
    }

    mume_window_end_paint(self, cr);
}

static void _menubar_handle_sizehint(
    struct _menubar *self, int *pref_w, int *pref_h,
    int *min_w, int *min_h, int *max_w, int *max_h)
{
    _menubar_get_preferred_size(self, pref_w, pref_h);
}

static void _menubar_handle_notify(
    struct _menubar *self, void *window, int code, void *data)
{
    switch (code) {
    case MUME_MENUBAR_CLOSE:
        if (mume_is_of(self->host, mume_menubar_class())) {
            _mume_window_handle_notify(
                NULL, self->host, window, code, data);
        }
        else {
            if (data) {
                int cmd = mume_menuitem_get_command(data);

                mume_post_event(
                    mume_make_command_event(self->host, self, cmd));
            }

            if (_menubar_is_popup(self)) {
                mume_menubar_popdown(self);
            }
            else if (_menubar_is_subitem_popuped(self)) {
                _menubar_popup_item(self, -1);
                _menubar_select_item(self, -1);
            }
        }
        break;

    case MUME_MENUBAR_DELETE:
        self->popupbar = NULL;
        break;
    }
}

static void _menubar_handle_close(struct _menubar *self)
{
    if (_menubar_is_popup(self))
        _menubar_close_popup(self, NULL);
}

const void* mume_menubar_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_menubar_meta_class(),
        "menubar",
        _menubar_super_class(),
        sizeof(struct _menubar),
        MUME_PROP_END,
        _mume_ctor, _menubar_ctor,
        _mume_dtor, _menubar_dtor,
        _mume_window_handle_button_down,
        _menubar_handle_button_down,
        _mume_window_handle_button_up,
        _menubar_handle_button_up,
        _mume_window_handle_mouse_motion,
        _menubar_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _menubar_handle_mouse_leave,
        _mume_window_handle_expose,
        _menubar_handle_expose,
        _mume_window_handle_sizehint,
        _menubar_handle_sizehint,
        _mume_window_handle_notify,
        _menubar_handle_notify,
        _mume_window_handle_close,
        _menubar_handle_close,
        MUME_FUNC_END);
}

void* mume_menubar_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(
        mume_menubar_class(), parent, x, y, w, h, parent, 0);
}

void* mume_menubar_new_popup(void *host)
{
    unsigned int flags = 0;
    mume_add_flag(flags, _MENUBAR_FLAG_POPUP);
    return mume_new(
        mume_menubar_class(), mume_root_window(),
        0, 0, 0, 0, host, flags);
}

int mume_menubar_load(void *_self, const char *res)
{
    struct _menubar *self = _self;

    assert(mume_is_of(_self, mume_menubar_class()));

    (void)self;
    return 0;
}

void mume_menubar_insert_item(void *_self, int index, void *item)
{
    struct _menubar *self = _self;
    struct _menuslot *slot;

    assert(mume_is_of(_self, mume_menubar_class()));

    slot = mume_vector_insert(self->slots, index, 1);
    slot->item = item;
    slot->rect = mume_rect_empty;
    slot->modified = 1;

    if (slot->item)
        mume_refobj_addref(slot->item);

    _menubar_invalidate_structure(self);
}

void mume_menubar_add_item(void *self, void *item)
{
    mume_menubar_insert_item(
        self, mume_menubar_count_items(self), item);
}

void mume_menubar_remove_items(void *_self, int index, int count)
{
    struct _menubar *self = _self;

    assert(mume_is_of(_self, mume_menubar_class()));

    mume_vector_erase(self->slots, index, count);
    _menubar_invalidate_structure(self);
}

int mume_menubar_count_items(const void *_self)
{
    const struct _menubar *self = _self;
    assert(mume_is_of(_self, mume_menubar_class()));
    return mume_vector_size(self->slots);
}

void mume_menubar_clear_items(void *self)
{
    mume_menubar_remove_items(
        self, 0, mume_menubar_count_items(self));
}

void* mume_menubar_get_item(const void *_self, int index)
{
    const struct _menubar *self = _self;
    const struct _menuslot *slot;

    assert(mume_is_of(_self, mume_menubar_class()));

    slot = mume_vector_at(self->slots, index);
    return slot->item;
}

void mume_menubar_popup(
    void *_self, int x, int y,
    const mume_rect_t *exclude, int auto_delete)
{
    struct _menubar *self = _self;
    mume_rect_t r;

    assert(mume_is_of(_self, mume_menubar_class()));
    assert(mume_test_flag(self->flags, _MENUBAR_FLAG_POPUP));

    if (_menubar_is_popuped(self))
        return;

    r.x = x;
    r.y = y;
    _menubar_get_preferred_size(self, &r.width, &r.height);
    _adjust_popup_rect(&r, exclude);

    mume_window_set_geometry(
        self, r.x, r.y, r.width, r.height);

    mume_window_map(self);

    mume_open_popup(self);

    mume_add_flag(self->flags, _MENUBAR_FLAG_POPUPED);

    if (auto_delete)
        mume_add_flag(self->flags, _MENUBAR_FLAG_AUTODEL);
}

void mume_menubar_popdown(void *_self)
{
    struct _menubar *self = _self;

    assert(mume_is_of(_self, mume_menubar_class()));
    assert(mume_test_flag(self->flags, _MENUBAR_FLAG_POPUP));

    if (!_menubar_is_popuped(self))
        return;

    if (_menubar_is_subitem_popuped(self))
        mume_menubar_popdown(self->popupbar);

    mume_window_unmap(self);

    mume_close_popup(self);

    mume_remove_flag(self->flags, _MENUBAR_FLAG_POPUPED);

    if (mume_test_flag(self->flags, _MENUBAR_FLAG_AUTODEL))
        mume_delete(self);
}

mume_type_t* mume_typeof_menubar_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _menubar_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), popup_margin);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), baritem_margin);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), popitem_margin);
        MUME_DIRECT_PROPERTY(_mume_typeof_point(), popmark_size);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), barbg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), popupbg);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), popmark_normal);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), popmark_selected);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), itembg);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_texts(), texts);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
