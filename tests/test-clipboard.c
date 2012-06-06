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
#include "mume-gui.h"
#include "test-util.h"

void all_tests(void)
{
    void *clipboard;
    void *datarec[1];
    void *datasrc;
    const void **formats;
    const char *text = "test clipboard";
    int i, count = 0;

    /* Get the clipboard object. */
    clipboard = mume_clipboard();

    /* Create data records. */
    datarec[count++] = mume_datarec_new_with_text(text);

    /* Create data source. */
    datasrc = mume_datasrc_new(datarec, count);
    for (i = 0; i < count; ++i) {
        /* Data source has reference to data records now. */
        mume_refobj_release(datarec[i]);
    }

    /* Set clipboard data. */
    mume_clipboard_set_data(clipboard, datasrc);

    /* Clipboard has reference to data source. */
    mume_refobj_release(datasrc);

    /* Get clipboard data. */
    datasrc = mume_clipboard_get_data(clipboard);
    test_assert(datasrc);

    formats = mume_datasrc_get_formats(datasrc);
    test_assert(formats);

    count = mume_datasrc_count_formats(formats);
    test_assert(1 == count);
    test_assert(mume_datafmt(MUME_DATAFMT_TEXT) == formats[0]);

    datarec[0] = mume_datasrc_get_data(datasrc, formats[0]);
    test_assert(datarec[0]);
    test_assert(formats[0] == mume_datarec_get_format(datarec[0]));
    test_assert(!strcmp(mume_datarec_get_data(datarec[0]), text));
    test_assert((strlen(text) + 1) ==
                mume_datarec_get_length(datarec[0]));
    mume_refobj_release(datarec[0]);

    /* Release reference after get from clipboard. */
    mume_refobj_release(datasrc);
}
