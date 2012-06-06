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
#include "mume-base.h"
#include "test-util.h"

/************************* myplugin.h *************************/
mume_public const void* myplugin_class(void);

mume_public int myplugin_count(const void *self);

/************************* point.c *************************/
struct _myplugin {
    const char _[MUME_SIZEOF_PLUGIN];
    void *pcontext;
    char *name;
    int count;
};

#define _myplugin_super_class mume_plugin_class

void *_myplugin_class;

static void* _myplugin_ctor(
    struct _myplugin *self, int mode, va_list *app)
{
    if (!_mume_ctor(_myplugin_super_class(), self, mode, app))
        return NULL;

    self->pcontext = va_arg(*app, void*);
    self->name = NULL;
    self->count = 0;
    return self;
}

static void* _myplugin_dtor(struct _myplugin *self)
{
    free(self->name);
    return _mume_dtor(_myplugin_super_class(), self);
}

static int _myplugin_start(void *_self)
{
    struct _myplugin *self = _self;
    ++self->count;
    mume_plugin_define_symbol(
        self, "defined_count", (void*)(intptr_t)myplugin_count);
    return _mume_plugin_start(_myplugin_super_class(), self);
}

static void _myplugin_stop(void *_self)
{
    struct _myplugin *self = _self;
    --self->count;
    _mume_plugin_stop(_myplugin_super_class(), self);
}

const void* myplugin_class(void)
{
    return _myplugin_class ? _myplugin_class :
            mume_setup_class(
                &_myplugin_class,
                mume_plugin_meta_class(),
                "myplugin",
                _myplugin_super_class(),
                sizeof(struct _myplugin),
                MUME_PROP_END,
                _mume_ctor, _myplugin_ctor,
                _mume_dtor, _myplugin_dtor,
                _mume_plugin_start, _myplugin_start,
                _mume_plugin_stop, _myplugin_stop,
                MUME_FUNC_END);
}

int myplugin_count(const void *_self)
{
    const struct _myplugin *self = _self;
    return self->count;
}

/************************* test *************************/
void all_tests(void)
{
    const char *myid = "myplugin";
    const mume_plugin_info_t *pi;
    mume_plugin_info_t info = {
        "myplugin", "demo plugin", "1.0", "Tom",
        NULL, NULL, "myplugin_class"
    };
    void *pcontext = mume_pcontext_new();
    int (*count1)(const void*);
    int (*count2)(const void*);
    info.runtime_lib = test_name;
    /* Plugin state. */
    test_assert(MUME_PLUGIN_UNINSTALLED ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    test_assert(mume_pcontext_install_plugin(pcontext, &info));
    test_assert(!mume_pcontext_install_plugin(pcontext, &info));
    pi = mume_pcontext_get_plugin_info(pcontext, myid);
    test_assert(pi && pi != &info);
    test_assert(pi->identifier != info.identifier &&
                pi->name != info.name &&
                pi->version != info.version &&
                pi->provider_name != info.provider_name &&
                pi->plugin_path == info.plugin_path &&
                pi->runtime_lib != info.runtime_lib &&
                pi->runtime_symbol != info.runtime_symbol);
    test_assert(0 == strcmp(pi->identifier, info.identifier));
    test_assert(0 == strcmp(pi->name, info.name));
    test_assert(0 == strcmp(pi->version, info.version));
    test_assert(0 == strcmp(pi->provider_name, info.provider_name));
    test_assert(0 == strcmp(pi->runtime_lib, info.runtime_lib));
    test_assert(0 == strcmp(pi->runtime_symbol, info.runtime_symbol));
    test_assert(MUME_PLUGIN_INSTALLED ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    test_assert(mume_pcontext_start_plugin(pcontext, myid));
    test_assert(MUME_PLUGIN_ACTIVE ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    mume_pcontext_stop_plugin(pcontext, myid);
    test_assert(MUME_PLUGIN_RESOLVED ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    mume_pcontext_uninstall_plugin(pcontext, myid);
    test_assert(MUME_PLUGIN_UNINSTALLED ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    /* Resolve symbols. */
    test_assert(mume_pcontext_resolve_symbol(
        pcontext, myid, "myplugin_count") == NULL);
    test_assert(mume_pcontext_resolve_symbol(
        pcontext, myid, "defined_count") == NULL);
    test_assert(mume_pcontext_install_plugin(pcontext, &info));
    count1 = (int(*)(const void*))(intptr_t)mume_pcontext_resolve_symbol(
        pcontext, myid, "myplugin_count");
    test_assert(count1);
    count2 = (int(*)(const void*))(intptr_t)mume_pcontext_resolve_symbol(
        pcontext, myid, "defined_count");
    test_assert(count2);
    test_assert(MUME_PLUGIN_ACTIVE ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    mume_pcontext_stop_plugins(pcontext);
    mume_pcontext_uninstall_plugins(pcontext);
    test_assert(mume_pcontext_resolve_symbol(
        pcontext, myid, "myplugin_count") == NULL);
    test_assert(mume_pcontext_resolve_symbol(
        pcontext, myid, "defined_count") == NULL);
    test_assert(MUME_PLUGIN_UNINSTALLED ==
                mume_pcontext_get_plugin_state(pcontext, myid));
    mume_delete(pcontext);
}
