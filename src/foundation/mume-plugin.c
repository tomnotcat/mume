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
#include "mume-plugin.h"
#include "mume-debug.h"
#include "mume-dlfcn.h"
#include "mume-memory.h"
#include "mume-oset.h"
#include "mume-string.h"
#include "mume-types.h"
#include MUME_ASSERT_H

#define _pcontext_super_class mume_object_class
#define _pcontext_super_meta_class mume_meta_class
#define _plugin_super_class mume_object_class
#define _plugin_super_meta_class mume_meta_class

struct _pcontext {
    const char _[MUME_SIZEOF_OBJECT];
    mume_oset_t *plugins;
    void *reserved;
};

struct _pcontext_class {
    const char _[MUME_SIZEOF_CLASS];
    void *reserved;
};

struct _plugin {
    const char _[MUME_SIZEOF_OBJECT];
    mume_oset_t *symbols;
    void *reserved;
};

struct _plugin_class {
    const char _[MUME_SIZEOF_CLASS];
    int (*start)(void *self);
    void (*stop)(void *self);
    void *reserved;
};

struct _plugin_record {
    mume_plugin_info_t info;
    mume_dlhdl_t *runtime_lib;
    struct _plugin *plugin;
    int state;
};

struct _symbol_record {
    const char *name;
    void *ptr;
};

MUME_STATIC_ASSERT(sizeof(struct _pcontext) == MUME_SIZEOF_PCONTEXT);
MUME_STATIC_ASSERT(sizeof(struct _pcontext_class) ==
                   MUME_SIZEOF_PCONTEXT_CLASS);

MUME_STATIC_ASSERT(sizeof(struct _plugin) == MUME_SIZEOF_PLUGIN);
MUME_STATIC_ASSERT(sizeof(struct _plugin_class) ==
                   MUME_SIZEOF_PLUGIN_CLASS);

static int _plugin_record_start(struct _plugin_record *pr, void *pctx)
{
    switch (pr->state) {
    case MUME_PLUGIN_INSTALLED:
        if (pr->info.runtime_lib) {
            char *buf;
            size_t len = 0;
            if (pr->info.plugin_path)
                len = strlen(pr->info.plugin_path);

            buf = malloc_abort(len + strlen(pr->info.runtime_lib) + 2);
            if (pr->info.plugin_path) {
                strcpy(buf, pr->info.plugin_path);
                buf[len] = '/';
                len += 1;
            }

            strcpy(buf + len, pr->info.runtime_lib);
            pr->runtime_lib = mume_dlopen(buf);
            if (NULL == pr->runtime_lib) {
                mume_error(("mume_dlopen(\"%s\"): %s\n",
                            buf, mume_dlerror()));
                free(buf);
                return 0;
            }

            free(buf);
            if (pr->info.runtime_symbol) {
                typedef const void* class_fcn_t(void);

                class_fcn_t *clazz = (class_fcn_t*)(intptr_t)(
                    mume_dlsym(pr->runtime_lib, pr->info.runtime_symbol));

                if (clazz) {
                    pr->plugin = mume_new(clazz(), pctx);
                }
                else {
                    mume_error(("mume_dlsym(\"%s\"): %s\n",
                                pr->info.runtime_symbol, mume_dlerror()));
                }
            }
        }
        break;
    case MUME_PLUGIN_STARTING:
        mume_warning(("Plugin is in starting: %s\n", pr->info.identifier));
        return 0;
    case MUME_PLUGIN_STOPPING:
        mume_warning(("Plugin is in stopping: %s\n", pr->info.identifier));
        return 0;
    case MUME_PLUGIN_ACTIVE:
        mume_warning(("Plugin is active: %s\n", pr->info.identifier));
        return 0;
    }

    pr->state = MUME_PLUGIN_RESOLVED;
    if (pr->plugin) {
        pr->state = MUME_PLUGIN_STARTING;
        if (!_mume_plugin_start(NULL, pr->plugin)) {
            mume_error(("Plugin start failed: %s\n", pr->info.identifier));
            pr->state = MUME_PLUGIN_RESOLVED;
            return 0;
        }
    }

    pr->state = MUME_PLUGIN_ACTIVE;
    return 1;
}

static void _plugin_record_stop(struct _plugin_record *pr)
{
    switch (pr->state) {
    case MUME_PLUGIN_INSTALLED:
        mume_warning(("Plugin is in installed: %s\n", pr->info.identifier));
        return;
    case MUME_PLUGIN_RESOLVED:
        mume_warning(("Plugin is in resolved: %s\n", pr->info.identifier));
        return;
    case MUME_PLUGIN_STARTING:
        mume_warning(("Plugin is in starting: %s\n", pr->info.identifier));
        return;
    case MUME_PLUGIN_STOPPING:
        mume_warning(("Plugin is in stopping: %s\n", pr->info.identifier));
        return;
    }

    if (pr->plugin) {
        pr->state = MUME_PLUGIN_STOPPING;
        _mume_plugin_stop(NULL, pr->plugin);
    }

    pr->state = MUME_PLUGIN_RESOLVED;
}

static void _plugin_record_destruct(void *obj, void *p)
{
    struct _plugin_record *r = obj;
    switch (r->state) {
    case MUME_PLUGIN_STARTING:
    case MUME_PLUGIN_STOPPING:
        mume_error(("Invalid plugin state in destruct\n"));
        break;
    case MUME_PLUGIN_ACTIVE:
        _plugin_record_stop(r);
        break;
    }

    if (r->plugin)
        mume_delete(r->plugin);

    if (r->runtime_lib)
        mume_dlclose(r->runtime_lib);
}

static int _plugin_start(void *self)
{
    return 1;
}

static void _plugin_stop(void *self)
{
}

static void* _pcontext_ctor(
    struct _pcontext *self, int mode, va_list *app)
{
    if (!_mume_ctor(_pcontext_super_class(), self, mode, app))
        return NULL;

    self->plugins = mume_oset_new(
        _mume_type_string_compare, _plugin_record_destruct, NULL);

    return self;
}

static void* _pcontext_dtor(void *_self)
{
    struct _pcontext *self = _self;
    mume_oset_delete(self->plugins);
    return _mume_dtor(_pcontext_super_class(), _self);
}

static void* _pcontext_class_ctor(
    struct _pcontext_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_pcontext_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);
        (void)method;
    }

    return self;
}

static void* _plugin_ctor(
    struct _plugin *self, int mode, va_list *app)
{
    if (!_mume_ctor(_plugin_super_class(), self, mode, app))
        return NULL;

    self->symbols = mume_oset_new(
        _mume_type_string_compare, NULL, NULL);

    return self;
}

static void* _plugin_dtor(void *_self)
{
    struct _plugin *self = _self;
    mume_oset_delete(self->symbols);
    return _mume_dtor(_plugin_super_class(), _self);
}

static void* _plugin_class_ctor(
    struct _plugin_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_plugin_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_plugin_start)
            *(voidf**)&self->start = method;
        else if (selector == (voidf*)_mume_plugin_stop)
            *(voidf**)&self->stop = method;
    }

    return self;
}

const void* mume_pcontext_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_pcontext_meta_class(),
        "pcontext",
        _pcontext_super_class(),
        sizeof(struct _pcontext),
        MUME_PROP_END,
        _mume_ctor, _pcontext_ctor,
        _mume_dtor, _pcontext_dtor,
        MUME_FUNC_END);
}

const void* mume_pcontext_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "pcontext class",
        _pcontext_super_meta_class(),
        sizeof(struct _pcontext_class),
        MUME_PROP_END,
        _mume_ctor, _pcontext_class_ctor,
        MUME_FUNC_END);
}

const void* mume_plugin_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_plugin_meta_class(),
        "plugin",
        _plugin_super_class(),
        sizeof(struct _plugin),
        MUME_PROP_END,
        _mume_ctor, _plugin_ctor,
        _mume_dtor, _plugin_dtor,
        _mume_plugin_start, _plugin_start,
        _mume_plugin_stop, _plugin_stop,
        MUME_FUNC_END);
}

const void* mume_plugin_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "plugin class",
        _plugin_super_meta_class(),
        sizeof(struct _plugin_class),
        MUME_PROP_END,
        _mume_ctor, _plugin_class_ctor,
        MUME_FUNC_END);
}

int mume_pcontext_install_plugin(void *_self, const mume_plugin_info_t *pi)
{
    struct _pcontext *self = _self;
    struct _plugin_record *pr;
    mume_oset_node_t *node;
    size_t len;
    assert(pi && pi->identifier);
    node = mume_oset_find(self->plugins, &pi->identifier);
    if (node) {
        mume_warning(("Plugin exist: %s\n", pi->identifier));
        return 0;
    }

    len = mume_strslen(
        7, 1, pi->identifier, pi->name, pi->version,
        pi->provider_name, pi->plugin_path,
        pi->runtime_lib, pi->runtime_symbol);
    node = mume_oset_newnode(sizeof(*pr) + len);
    pr = (struct _plugin_record*)mume_oset_data(node);
    mume_strscpy((char*)pr + sizeof(*pr),
                 &pr->info.identifier, pi->identifier,
                 &pr->info.name, pi->name,
                 &pr->info.version, pi->version,
                 &pr->info.provider_name, pi->provider_name,
                 &pr->info.plugin_path, pi->plugin_path,
                 &pr->info.runtime_lib, pi->runtime_lib,
                 &pr->info.runtime_symbol, pi->runtime_symbol,
                 NULL);
    pr->runtime_lib = NULL;
    pr->plugin = NULL;
    pr->state = MUME_PLUGIN_INSTALLED;
    mume_oset_insert(self->plugins, node);
    return 1;
}

int mume_pcontext_start_plugin(void *_self, const char *id)
{
    struct _pcontext *self = _self;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node) {
        mume_warning(("Plugin not exist: %s\n", id));
        return 0;
    }

    return _plugin_record_start(mume_oset_data(node), self);
}

void mume_pcontext_stop_plugin(void *_self, const char *id)
{
    struct _pcontext *self = _self;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node) {
        mume_warning(("Plugin not exist: %s\n", id));
        return;
    }

    _plugin_record_stop(mume_oset_data(node));
}

void mume_pcontext_stop_plugins(void *_self)
{
    struct _pcontext *self = _self;
    struct _plugin_record *pr;
    mume_oset_node_t *nd;
    mume_oset_foreach(self->plugins, nd, pr) {
        _plugin_record_stop(pr);
    }
}

void mume_pcontext_uninstall_plugin(void *_self, const char *id)
{
    struct _pcontext *self = _self;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node) {
        mume_warning(("Plugin not exist: %s\n", id));
        return;
    }

    mume_oset_erase(self->plugins, node);
}

void mume_pcontext_uninstall_plugins(void *_self)
{
    struct _pcontext *self = _self;
    mume_oset_clear(self->plugins);
}

const mume_plugin_info_t* mume_pcontext_get_plugin_info(
    const void *_self, const char *id)
{
    const struct _pcontext *self = _self;
    const struct _plugin_record *pi;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node) {
        mume_warning(("Plugin not exist: %s\n", id));
        return NULL;
    }

    pi = mume_oset_data(node);
    return &pi->info;
}

int mume_pcontext_get_plugin_state(const void *_self, const char *id)
{
    const struct _pcontext *self = _self;
    const struct _plugin_record *pi;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node)
        return MUME_PLUGIN_UNINSTALLED;

    pi = mume_oset_data(node);
    return pi->state;
}

void* mume_pcontext_resolve_symbol(
    void *_self, const char *id, const char *name)
{
    struct _pcontext *self = _self;
    struct _plugin_record *pr;
    mume_oset_node_t *node = mume_oset_find(self->plugins, &id);
    if (NULL == node) {
        mume_warning(("Plugin not exist: %s\n", id));
        return NULL;
    }

    pr = (struct _plugin_record*)mume_oset_data(node);
    if (pr->state != MUME_PLUGIN_ACTIVE) {
        if (!_plugin_record_start(pr, self)) {
            mume_warning(("Plugin start failed: %s\n", id));
        }
    }

    node = mume_oset_find(pr->plugin->symbols, &name);
    if (node)
        return ((struct _symbol_record*)(mume_oset_data(node)))->ptr;

    if (pr->runtime_lib)
        return mume_dlsym(pr->runtime_lib, name);

    return NULL;
}

int mume_plugin_define_symbol(
    void *_self, const char *name, void *ptr)
{
    struct _plugin *self = _self;
    mume_oset_node_t *node;
    if (mume_oset_find(self->symbols, &name)) {
        mume_warning(("Symbol exists: %s\n", name));
        return 0;
    }

    node = mume_oset_new_name_node(
        name, sizeof(struct _symbol_record));
    ((struct _symbol_record*)mume_oset_data(node))->ptr = ptr;
    mume_oset_insert(self->symbols, node);
    return 1;
}

int _mume_plugin_start(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_plugin_meta_class(), mume_plugin_class(),
        struct _plugin_class, start, (_self));
}

void _mume_plugin_stop(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_plugin_meta_class(), mume_plugin_class(),
        struct _plugin_class, stop, (_self));
}
