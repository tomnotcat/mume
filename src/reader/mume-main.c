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

static void _init_environments(void)
{
    char config_dir[256];
    char config_file[256];
    size_t dir_len;
    const char *home = getenv("HOME");

    setenv("MUME_BACKEND_NAME", "libmux11.so", 0);

    if (home) {
        dir_len = strcpy_c(config_dir, COUNT_OF(config_dir), home);
        dir_len += strcpy_c(
            config_dir + dir_len,
            COUNT_OF(config_dir) - dir_len, "/.mume/");
        mkdir(config_dir, S_IRWXU);
    }
    else {
        dir_len = 0;
    }

    strcpy_c(config_file, dir_len, config_dir);
    strcpy_s(config_file + dir_len,
             COUNT_OF(config_file) - dir_len, "profiles.xml");
    setenv("MUME_PROFILE_NAME", config_file, 0);

    setenv("MUME_THEME_NAME", MUME_DATA_DIR "default.theme", 0);

    strcpy_c(config_file, dir_len, config_dir);
    strcpy_s(config_file + dir_len,
             COUNT_OF(config_file) - dir_len, "books.xml");
    setenv("MUME_BOOKS_FILE", config_file, 0);

    strcpy_c(config_file, dir_len, config_dir);
    strcpy_s(config_file + dir_len,
             COUNT_OF(config_file) - dir_len, "myshelf.xml");
    setenv("MUME_MYSHELF_FILE", config_file, 0);

    strcpy_c(config_file, dir_len, config_dir);
    strcpy_s(config_file + dir_len,
             COUNT_OF(config_file) - dir_len, "recent.xml");
    setenv("MUME_RECENT_FILE", config_file, 0);

    strcpy_c(config_file, dir_len, config_dir);
    strcpy_s(config_file + dir_len,
             COUNT_OF(config_file) - dir_len, "history");
    setenv("MUME_HISTORY_DIR", config_file, 0);
}

static void _load_profile(const char *file)
{
    void *profile = mume_profile();
    void *window = mume_mainform();
    mume_rect_t rect = mume_rect_empty;
    int screen_cx = 0;
    int screen_cy = 0;

    mume_profile_load_from_file(profile, file);
    rect = mume_profile_get_rect(
        profile, "main_window", "geometry", rect);
    mume_screen_size(&screen_cx, &screen_cy);

#define MIN_WINDOW_SIZE 50
    if (rect.width < MIN_WINDOW_SIZE ||
        rect.height < MIN_WINDOW_SIZE ||
        rect.x + rect.width < MIN_WINDOW_SIZE ||
        rect.y + rect.height < MIN_WINDOW_SIZE ||
        screen_cx - rect.x < MIN_WINDOW_SIZE ||
        screen_cy - rect.y < MIN_WINDOW_SIZE)
    {
        rect = mume_rect_empty;
    }
#undef MIN_WINDOW_SIZE

    if (mume_rect_is_empty(rect)) {
        mume_window_center(window, mume_root_window());
    }
    else {
        mume_window_set_geometry(
            window, rect.x, rect.y, rect.width, rect.height);
    }

    mume_window_map(window);
}

static void _load_books(void)
{
    const char *file;
    void *bookmgr = mume_bookmgr();

    file = getenv("MUME_BOOKS_FILE");
    if (!mume_bookmgr_load_from_file(bookmgr, file))
        mume_warning(("Load books failed: %s\n", file));
}

static void _save_books(void)
{
    const char *file;
    void *bookmgr = mume_bookmgr();

    file = getenv("MUME_BOOKS_FILE");
    if (!mume_bookmgr_save_to_file(bookmgr, file))
        mume_warning(("Save books failed: %s\n", file));
}

static void _open_initial_doc(const char *file)
{
    if (file) {
        if (!mume_docmgr_load_file(mume_docmgr(), file)) {
            mume_warning(("Load document error: %s\n", file));
        }
    }
    else {
        mume_mainform_new_tab(mume_mainform());
    }
}

static void _save_profile(const char *file)
{
    void *profile = mume_profile();
    void *window = mume_mainform();
    mume_rect_t r;
    int i;

    mume_window_get_geometry(
        window, &r.x, &r.y, &r.width, &r.height);
    mume_profile_set_rect(
        profile, "main_window", "geometry", r);

    i = mume_window_is_mapped(window);
    mume_profile_set_int(
        profile, "main_window", "visible", i);

    mume_profile_save_to_file(profile, file);
}

static void _run_event_loop(void)
{
    mume_event_t event;

    while (mume_wait_event(&event)) {
        if (event.type != MUME_EVENT_QUIT)
            mume_disp_event(&event);
        else
            break;
    }
}

int main(int argc, char* argv[])
{
    const char *backend_name = NULL;
    const char *profile_name = NULL;
    const char *theme_name = NULL;
    mume_dlhdl_t *dlhdl = NULL;
    int c, width = 0, height = 0;
    unsigned int flags = 0;
    void *backend = NULL;
    void *frontend = mume_frontend_new();

    struct option longopts[] = {
        { "loglvl", required_argument, NULL, 'g' },
        { "backend", required_argument, NULL, 'b' },
        { "width", required_argument, NULL, 'w' },
        { "height", required_argument, NULL, 'h' },
        { "profile", required_argument, NULL, 'p' },
        { "theme", required_argument, NULL, 't' },
        { 0, 0, 0, 0 },
    };

    setlocale(LC_ALL,"");

    /* Set default log level to WARNING. */
    mume_logger_set_level(NULL, MUME_LOG_WARNING);

    while((c = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
        switch (c) {
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

        case 'p':
            profile_name = optarg;
            break;

        case 't':
            theme_name = optarg;
            break;
        }
    }

    _init_environments();
    mume_init_libcrypto();

    if (NULL == backend_name) {
        backend_name = getenv("MUME_BACKEND_NAME");
    }

    if (NULL == profile_name) {
        profile_name = getenv("MUME_PROFILE_NAME");
    }

    if (NULL == theme_name) {
        theme_name = getenv("MUME_THEME_NAME");
    }

    dlhdl = mume_dlopen(backend_name);
    if (NULL == dlhdl) {
        mume_abort(("Load backend failed %s: %s\n",
                    backend_name, mume_dlerror()));
    }

    mume_add_flag(flags, MUME_BACKEND_FLAG_CENTERED);
    mume_add_flag(flags, MUME_BACKEND_FLAG_RESIZABLE);
    backend = mume_backend_new_from_dll(
        dlhdl, width, height, flags, NULL);

    if (NULL == backend)
        mume_abort(("Create backend failed\n"));

    mume_gui_initialize(frontend, backend);
    mume_reader_init();

    if (!mume_resmgr_load(mume_resmgr(), theme_name, "reader.xml"))
        mume_warning(("Load theme error: %s\n", theme_name));

    _load_profile(profile_name);
    _load_books();
    _open_initial_doc(argv[optind]);
    _run_event_loop();
    _save_books();
    _save_profile(profile_name);

    mume_reader_uninit();
    mume_gui_uninitialize();
    mume_delete(backend);
    mume_delete(frontend);

    /* This will cause valgrind report leak?
    mume_dlclose(dlhdl);
    */

    return 0;
}
