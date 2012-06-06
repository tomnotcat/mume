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
#ifndef MUME_FOUNDATION_PLUGIN_H
#define MUME_FOUNDATION_PLUGIN_H

#include "mume-object.h"

/* The plugin framework, inspired by "c pluff". */

MUME_BEGIN_DECLS

#define MUME_SIZEOF_PCONTEXT (MUME_SIZEOF_OBJECT +  \
                              sizeof(void*) * 2)

#define MUME_SIZEOF_PCONTEXT_CLASS (MUME_SIZEOF_CLASS + \
                                    sizeof(void*))

#define MUME_SIZEOF_PLUGIN (MUME_SIZEOF_OBJECT +    \
                            sizeof(void*) * 2)

#define MUME_SIZEOF_PLUGIN_CLASS (MUME_SIZEOF_CLASS +  \
                                  sizeof(voidf*) * 2 + \
                                  sizeof(void*))

enum mume_plugin_state_e {
    MUME_PLUGIN_UNINSTALLED,
    MUME_PLUGIN_INSTALLED,
    MUME_PLUGIN_RESOLVED,
    MUME_PLUGIN_STARTING,
    MUME_PLUGIN_STOPPING,
    MUME_PLUGIN_ACTIVE
};

typedef struct _mume_plugin_info {
    const char *identifier;
    const char *name;
    const char *version;
    const char *provider_name;
    const char *plugin_path;
    const char *runtime_lib;
    const char *runtime_symbol;
} mume_plugin_info_t;

mume_public const void* mume_pcontext_class(void);

mume_public const void* mume_pcontext_meta_class(void);

mume_public const void* mume_plugin_class(void);

mume_public const void* mume_plugin_meta_class(void);

#define mume_pcontext_new() mume_new(mume_pcontext_class())

mume_public int mume_pcontext_install_plugin(
    void *self, const mume_plugin_info_t *pi);

mume_public int mume_pcontext_start_plugin(void *self, const char *id);

mume_public void mume_pcontext_stop_plugin(void *self, const char *id);

mume_public void mume_pcontext_stop_plugins(void *self);

mume_public void mume_pcontext_uninstall_plugin(void *self, const char *id);

mume_public void mume_pcontext_uninstall_plugins(void *self);

mume_public const mume_plugin_info_t* mume_pcontext_get_plugin_info(
    const void *self, const char *id);

mume_public int mume_pcontext_get_plugin_state(
    const void *self, const char *id);

mume_public void* mume_pcontext_resolve_symbol(
    void *self, const char *id, const char *name);

mume_public int mume_plugin_define_symbol(
    void *self, const char *name, void *ptr);

/* Selector for start a plugin. */
mume_public int _mume_plugin_start(const void *clazz, void *self);

/* Selector for stop a plugin. */
mume_public void _mume_plugin_stop(const void *clazz, void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_PLUGIN_H */
