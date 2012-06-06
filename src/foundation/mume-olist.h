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
#ifndef MUME_FOUNDATION_OLIST_H
#define MUME_FOUNDATION_OLIST_H

#include "mume-list.h"
#include "mume-octnr.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_OLIST (MUME_SIZEOF_OCTNR + \
                           sizeof(mume_list_t))

#define MUME_SIZEOF_OLIST_CLASS (MUME_SIZEOF_OCTNR_CLASS)

mume_public const void* mume_olist_class(void);

#define mume_olist_meta_class mume_octnr_meta_class

mume_public void* mume_olist_new(void (*del)(void*));

mume_public void* mume_olist_front(const void *self);

mume_public void* mume_olist_back(const void *self);

mume_public void mume_olist_push_front(void *self, void *object);

mume_public void mume_olist_push_back(void *self, void *object);

mume_public void mume_olist_remove(void *self, void *object);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_OLIST_H */
