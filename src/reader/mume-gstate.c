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
#include "mume-gstate.h"
#include "mume-bookmgr.h"
#include "mume-docmgr.h"
#include "mume-docview.h"
#include "mume-mainform.h"
#include "mume-profile.h"
#include MUME_ERRNO_H

struct _gstate {
    void *profile;
    void *bookmgr;
    void *filetc;
    void *docmgr;
    void *mainform;
};

static struct _gstate *_gstate;

#define tohex(n) ((n) < 10 ? ((n) + '0') : (((n) - 10) + 'A'))

static char* _do_bin2hex(
    const void *buffer, size_t length, char *stringbuf, int with_colon)
{
    const unsigned char *s;
    char *p;

    if (!stringbuf)
    {
        /* Not really correct for with_colon but we don't care about the
           one wasted byte. */
        size_t n = with_colon? 3:2;
        size_t nbytes = n * length + 1;
        if (length &&  (nbytes-1) / n != length)
        {
            errno = ENOMEM;
            return NULL;
        }
        stringbuf = malloc_abort(nbytes);
        if (!stringbuf)
            return NULL;
    }

    for (s = buffer, p = stringbuf; length; length--, s++)
    {
        if (with_colon && s != buffer)
            *p++ = ':';
        *p++ = tohex ((*s>>4)&15);
        *p++ = tohex (*s&15);
    }

    *p = 0;

    return stringbuf;
}

static char* _bin2hex(
    const void *buffer, size_t length, char *stringbuf)
{
    return _do_bin2hex(buffer, length, stringbuf, 0);
}

int mume_reader_init(void)
{
    if (!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
        mume_abort(("libgcrypt has not been initialized\n"));

    if (NULL == _gstate) {
        mume_resmgr_t *rmgr = mume_resmgr();

        _gstate = malloc_abort(sizeof(*_gstate));
        _gstate->profile = mume_profile_new();
        _gstate->bookmgr = mume_bookmgr_new();
        _gstate->filetc = mume_filetc_new();
        _gstate->docmgr = mume_docmgr_new();
        _gstate->mainform = NULL;

        mume_resmgr_regtype(rmgr, "docview", "theme",
                            mume_typeof_docview_theme());

        return 1;
    }

    return 0;
}

void mume_reader_uninit(void)
{
    if (_gstate) {
        mume_delete(_gstate->mainform);
        mume_delete(_gstate->docmgr);
        mume_delete(_gstate->filetc);
        mume_delete(_gstate->bookmgr);
        mume_delete(_gstate->profile);
        free(_gstate);
        _gstate = NULL;
    }
}

void* mume_profile(void)
{
    return _gstate->profile;
}

void* mume_bookmgr(void)
{
    return _gstate->bookmgr;
}

void* mume_filetc(void)
{
    return _gstate->filetc;
}

void* mume_docmgr(void)
{
    return _gstate->docmgr;
}

void* mume_mainform(void)
{
    if (NULL == _gstate->mainform) {
        _gstate->mainform = mume_mainform_new(
            mume_root_window(), 0, 0,
            MUME_MAINFORM_WIDTH, MUME_MAINFORM_HEIGHT);
    }

    return _gstate->mainform;
}

char* mume_create_digest(mume_stream_t *stm)
{
    gcry_error_t err;
    gcry_md_hd_t hd;
    char buf[10240];
    char *result;
    size_t len;
    int algo = GCRY_MD_SHA1;

    err = gcry_md_open(&hd, algo, 0);
    if (err) {
        mume_error(("gcry_md_open failure: %s/%s\n",
                    gcry_strsource(err), gcry_strerror(err)));
    }

    /* gcry_md_reset(hd); */

    while ((len = mume_stream_read(stm, buf, sizeof(buf))))
        gcry_md_write(hd, buf, len);

    gcry_md_final(hd);
    len = gcry_md_get_algo_dlen(algo);
    result = malloc_abort(len * 2 + 1);

    _bin2hex(gcry_md_read(hd, algo), len, result);

    gcry_md_close(hd);

    return result;
}
