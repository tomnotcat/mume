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
#ifndef MUME_READER_GSTATE_H
#define MUME_READER_GSTATE_H

#include "mume-common.h"

#define GCRYPT_NO_MPI_MACROS
#define GCRYPT_NO_DEPRECATED
#include <gcrypt.h>

MUME_BEGIN_DECLS

/* Initialize mume reader. must be called before any other
 * mume reader operations.
 */
murdr_public int mume_reader_init(void);

/* Uninitialize mume reader. after this operation, user should
 * not call any other mume reader operations.
 */
murdr_public void mume_reader_uninit(void);

murdr_public void* mume_profile(void);

murdr_public void* mume_bookmgr(void);

murdr_public void* mume_filetc(void);

murdr_public void* mume_docmgr(void);

murdr_public void* mume_winmgr(void);

murdr_public void* mume_mainform(void);

murdr_public char* mume_create_digest(mume_stream_t *stm);

static inline void mume_init_libcrypto(void)
{
    /* It is important that these initialization steps are not
     * done by a library but by the actual application.
     * http://www.gnupg.org/documentation/manuals/gcrypt/
     * Initializing-the-library.html#Initializing-the-library */

    /* Version check should be the very first call because it
       makes sure that important subsystems are intialized. */
    if (!gcry_check_version(GCRYPT_VERSION))
        mume_abort(("libgcrypt version mismatch\n"));

    /* Disable secure memory.  */
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);

    /* Tell Libgcrypt that initialization has completed. */
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
}

MUME_END_DECLS

#endif /* MUME_READER_GSTATE_H */
