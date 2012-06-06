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
#include "mume-reader.h"
#include "test-util.h"

int main(int argc, char* argv[])
{
    const char *locale = "";
    const char *backend_name = NULL;
    mume_dlhdl_t *dlhdl = NULL;
    int c, width = 0, height = 0, reader = 0;
    unsigned int flags = 0;
    void *backend = NULL;
    void *frontend = mume_frontend_new();

    struct option longopts[] = {
        {"locale", required_argument, NULL, 'l'},
        {"loglvl", required_argument, NULL, 'g'},
        {"backend", required_argument, NULL, 'b'},
        {"width", required_argument, NULL, 'w'},
        {"height", required_argument, NULL, 'h'},
        {"reader", required_argument, NULL, 'r'},
        {0, 0, 0, 0},
    };

    test_name = argv[0];
    while((c = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (c) {
        case 'l':
            locale = optarg;
            break;
        case 'g':
            mume_logger_set_level(NULL, atoi(optarg));
            break;
        case 'b':
            backend_name = optarg;
            break;
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
            break;
        case 'r':
            reader = atoi(optarg);
            break;
        }
    }

    if (NULL == backend_name) {
        mume_debug(("You must choose a backend\n"));
        return 1;
    }

    if (strcmp(backend_name, "console")) {
        dlhdl = mume_dlopen(backend_name);
        if (NULL == dlhdl) {
            mume_abort(("load backend %s: %s\n",
                        backend_name, mume_dlerror()));
        }

        mume_add_flag(flags, MUME_BACKEND_FLAG_CENTERED);
        mume_add_flag(flags, MUME_BACKEND_FLAG_RESIZABLE);
        backend = mume_backend_new_from_dll(
            dlhdl, width, height, flags, NULL);

        if (NULL == backend)
            mume_abort(("create backend failed\n"));

        mume_gui_initialize(frontend, backend);
    }

    if (reader) {
        mume_init_libcrypto();
        mume_reader_init();
    }

    mume_debug(("backend is: %s\n", backend_name));
    /* const char *locale =setlocale(LC_ALL,""); */
    locale = setlocale(LC_CTYPE, locale);
    if (NULL == locale) {
        mume_warning(("set locale failed\n"));
        return 1;
    }

    mume_debug(("locale is: %s\n", locale));
    test_decl_run(all_tests);
    if (reader) {
        mume_reader_uninit();
    }

    if (backend) {
        mume_gui_uninitialize();
        mume_delete(backend);
    }

    mume_delete(frontend);
    /* This will cause valgrind report leak?
    mume_dlclose(dlhdl);
    */

    return 0;
}
