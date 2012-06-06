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
#include "mume-filetc.h"
#include "mume-encoding.h"
#include "mume-list.h"
#include "mume-memory.h"
#include "mume-stream.h"
#include "mume-string.h"
#include MUME_STRING_H

#define _FILETC_MAX_EXT 32
#define _filetc_super_class mume_object_class

struct _ext_record {
    int type;
    char ext[_FILETC_MAX_EXT];
};

struct _magic_record {
    int type;
    mume_magic_t magic;
};

struct _filetc {
    const char _[MUME_SIZEOF_OBJECT];
    mume_list_t *exts;
    mume_list_t *magics;
};

static void* _filetc_ctor(
    struct _filetc *self, int mode, va_list *app)
{
    mume_magic_t magic;

    if (!_mume_ctor(_filetc_super_class(), self, mode, app))
        return NULL;

    self->exts = mume_list_new(NULL, NULL);
    self->magics = mume_list_new(NULL, NULL);

    /* Add buildin file type info. */
    mume_filetc_add_ext(self, MUME_FILETYPE_PDF, "pdf");
    mume_filetc_add_ext(self, MUME_FILETYPE_TXT, "txt");
    magic.offset = 0;
    magic.type = MUME_MAGICTYPE_STRING;
    strcpy_s(magic.value.s, MUME_MAX_MAGIC_STRING, "%PDF-");
    mume_filetc_add_magic(self, MUME_FILETYPE_PDF, magic);

    return self;
}

static void* _filetc_dtor(void *_self)
{
    struct _filetc *self = _self;
    mume_list_delete(self->magics);
    mume_list_delete(self->exts);
    return _mume_dtor(_filetc_super_class(), _self);
}

const void* mume_filetc_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "filetc",
        _filetc_super_class(),
        sizeof(struct _filetc),
        MUME_PROP_END,
        _mume_ctor, _filetc_ctor,
        _mume_dtor, _filetc_dtor,
        MUME_FUNC_END);
}

int mume_filetc_check_ext(const void *_self, const char *name)
{
    const struct _filetc *self = _self;
    const char *ext = strrchr(name, '.');
    if (ext) {
        const struct _ext_record *er;
        mume_list_node_t *ln;
        ++ext;
        ln = mume_list_front(self->exts);
        while (ln) {
            er = (const struct _ext_record*)mume_list_data(ln);
            if (0 == strcasecmp(ext, er->ext))
                return er->type;
            ln = mume_list_next(ln);
        }
    }
    return MUME_FILETYPE_UNKNOWN;
}

int mume_filetc_check_magic(const void *_self, mume_stream_t *stm)
{
    const struct _filetc *self = _self;
    const struct _magic_record *mr;
    mume_list_node_t *ln;
#define HOWMANY (256 * 1024)
#define SLOP 1
    char *buf = malloc_abort(HOWMANY + SLOP);
    size_t orig = mume_stream_tell(stm);
    size_t nbytes = mume_stream_read(stm, buf, HOWMANY);
    int type = MUME_FILETYPE_UNKNOWN;
    mume_stream_seek(stm, orig);
    /* NUL terminate */
    memset(buf + nbytes, 0, SLOP);
#undef SLOP
#undef HOWMANY
    if (0 == nbytes) {
        type = MUME_FILETYPE_EMPTY;
        goto done;
    }

    /* Check soft magics. */
    ln = mume_list_front(self->magics);
    while (ln) {
        mr = (const struct _magic_record*)mume_list_data(ln);
        if (mr->magic.offset < nbytes) {
            const char *data = buf + mr->magic.offset;
            switch (mr->magic.type) {
            case MUME_MAGICTYPE_STRING:
                if (0 == strncmp(mr->magic.value.s, data,
                                 strlen(mr->magic.value.s)))
                {
                    type = mr->type;
                }
                break;
            }

            if (type != MUME_FILETYPE_UNKNOWN)
                break;
        }
        ln = mume_list_next(ln);
    }

    /* Check text file. */
    if (mume_encoding_check(
            (const unsigned char*)buf, nbytes, NULL, NULL))
    {
        type = MUME_FILETYPE_TXT;
    }

done:
    free(buf);
    return type;
}

void mume_filetc_add_ext(void *_self, int type, const char *ext)
{
    struct _filetc *self = _self;
    struct _ext_record *er;
    mume_list_node_t *ln;
    ln = mume_list_push_front(self->exts, sizeof(*er));
    er = (struct _ext_record*)mume_list_data(ln);
    er->type = type;
    strcpy_s(er->ext, _FILETC_MAX_EXT, ext);
}

void mume_filetc_add_magic(void *_self, int type, mume_magic_t magic)
{
    struct _filetc *self = _self;
    struct _magic_record *mr;
    mume_list_node_t *ln;
    ln = mume_list_push_front(self->magics, sizeof(*mr));
    mr = (struct _magic_record*)mume_list_data(ln);
    mr->type = type;
    mr->magic = magic;
}
