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
#ifndef CATCH_GUI_BACKENDS_WIN32_BACKEND_H
#define CATCH_GUI_BACKENDS_WIN32_BACKEND_H

#include "common.h"
#include "mume-backend.h"

CATCH_BEGIN_DECLS

struct win32_backend_s {
    mume_backend_t base;
    mume_logger_t *logger;
    mume_frontend_t *front;
    mume_target_t *root;
};

mume_public mume_backend_t* win32_create_backend(
    mume_frontend_t *front, mume_logger_t *logger);

CATCH_END_DECLS

#endif  /* CATCH_GUI_BACKENDS_WIN32_BACKEND_H */
