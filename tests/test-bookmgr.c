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
#include "mume-reader.h"
#include "test-util.h"

static void _book_enum_proc(void *closure, void *book)
{
    void **books = closure;

    while (books[0])
        ++books;

    *books = book;
}

static void _setup_bookshelf(void *shelf, int shelves, int books)
{
    int i;
    char buf[256];
    void *book;

    test_assert(mume_is_of(shelf, mume_bookshelf_class()));

    for (i = 0; i < shelves; ++i) {
        snprintf(buf, sizeof(buf), "Shelf %d", i);
        mume_bookshelf_add_shelf(shelf, mume_bookshelf_new(buf));
    }

    for (i = 0; i < books; ++i) {
        snprintf(buf, sizeof(buf), "Book %d", i);

        book = mume_bookmgr_get_book(mume_bookmgr(), buf);
        if (NULL == book)
            book = mume_bookmgr_add_book(mume_bookmgr(), buf, NULL);

        mume_bookshelf_add_book(shelf, book);
    }
}

static void _test_bookmgr1(void)
{
    void *mgr = mume_bookmgr();
    void *books[2] = { NULL, NULL };
    void *book1, *book2;

    test_assert(mume_bookmgr_count_books(mgr) == 0);

    book1 = mume_bookmgr_add_book(
        mgr, NULL, TESTS_DATA_DIR "/test.txt");

    test_assert(book1);
    test_assert(0 == strcmp(
        mume_book_get_id(book1),
        "50BE9D8F4888AF7BB9F9EFABD41FBF1E29412429"));

    test_assert(0 == strcmp(
        mume_book_get_path(book1),
        TESTS_DATA_DIR "/test.txt"));

    test_assert(NULL == mume_bookmgr_add_book(
        mgr, NULL, TESTS_DATA_DIR "/test.txt"));

    test_assert(mume_bookmgr_count_books(mgr) == 1);

    book2 = mume_bookmgr_add_book(
        mgr, "hello book", TESTS_DATA_DIR "/nonexist");
    test_assert(book2);
    test_assert(0 == strcmp(
        mume_book_get_id(book2),
        "hello book"));

    test_assert(NULL == mume_bookmgr_add_book(
        mgr, NULL, TESTS_DATA_DIR "/nonexist"));

    book2 = mume_bookmgr_add_book(
        mgr, NULL, TESTS_DATA_DIR "/test.pdf");

    test_assert(mume_bookmgr_count_books(mgr) == 3);
    mume_bookmgr_del_book(mgr, "hello book");
    mume_bookmgr_del_book(mgr, "non-exist");
    test_assert(mume_bookmgr_count_books(mgr) == 2);

    test_assert(0 == strcmp(
        mume_book_get_id(book2),
        "B0A073D356AB1973CB4DCFDD85243241C9461977"));

    test_assert(0 == strcmp(
        mume_book_get_path(book2),
        TESTS_DATA_DIR "/test.pdf"));

    test_assert(mume_bookmgr_enum_books(
        mgr, _book_enum_proc, books) == 2);

    test_assert(mume_is_of(book1, mume_book_class()));
    test_assert(mume_is_of(book2, mume_book_class()));

    test_assert(mume_is_of(books[0], mume_book_class()));
    test_assert(mume_is_of(books[1], mume_book_class()));

    test_assert(books[0] == book1 || books[0] == book2);
    test_assert(books[1] == book1 || books[1] == book2);
}

static void _test_bookmgr2(void)
{
    const char *file = TESTS_DATA_DIR "/test-bookmgr.xml";
    void *mgr = mume_bookmgr_new();
    const struct _item {
        const char *id;
        const char *path;
        const char *name;
    } items[] = {
        { "id1\"", NULL, NULL },
        { "id2</>", "path2/book2", "book2" },
        { "id3&'", "path3/book3", "book3" },
    };
    int i;

    for (i = 0; i < COUNT_OF(items); ++i)
        mume_bookmgr_add_book(mgr, items[i].id, items[i].path);

    test_assert(mume_bookmgr_save_to_file(mgr, file));
    mume_delete(mgr);

    mgr = mume_bookmgr_new();
    test_assert(mume_bookmgr_load_from_file(mgr, file));

    for (i = 0; i < COUNT_OF(items); ++i) {
        void *book = mume_bookmgr_get_book(mgr, items[i].id);

        test_assert(book);
        test_assert(0 == strcmp(
            mume_book_get_id(book), items[i].id));

        if (items[i].path) {
            test_assert(0 == strcmp(
                mume_book_get_path(book), items[i].path));

            test_assert(0 == strcmp(
                mume_book_get_name(book), items[i].name));
        }
    }

    mume_delete(mgr);
}

static void _test_bookshelf(void)
{
    const char *file = TESTS_DATA_DIR "/test-bookshelf.xml";
    int i, j, sc, bc;
    void *shelf;
    void *sub;
    void *sub2;
    void *book;
    char buf[256];

    sc = 10;
    bc = 50;
    shelf = mume_bookshelf_new("root");
    _setup_bookshelf(shelf, sc, bc);

    test_assert(mume_bookshelf_parent_shelf(shelf) == NULL);
    test_assert(mume_bookshelf_count_shelves(shelf) == sc);
    test_assert(mume_bookshelf_count_books(shelf) == bc);

    for (i = 0; i < sc; ++i) {
        sub = mume_bookshelf_get_shelf(shelf, i);
        test_assert(mume_is_of(sub, mume_bookshelf_class()));
        test_assert(mume_bookshelf_parent_shelf(sub) == shelf);
        test_assert(mume_bookshelf_count_shelves(sub) == 0);
        test_assert(mume_bookshelf_count_books(sub) == 0);
        _setup_bookshelf(sub, sc, bc);
    }

    mume_bookshelf_remove_shelves(shelf, sc / 2, sc / 2);
    mume_bookshelf_remove_books(shelf, bc / 2, bc / 2);

    test_assert(mume_bookshelf_count_shelves(shelf) == sc / 2);
    test_assert(mume_bookshelf_count_books(shelf) == bc / 2);

    test_assert(mume_bookshelf_save_to_file(shelf, file));
    mume_delete(shelf);

    shelf = mume_bookshelf_new("root");
    test_assert(mume_bookshelf_load_from_file(shelf, file));

    test_assert(mume_bookshelf_count_shelves(shelf) == sc / 2);
    test_assert(mume_bookshelf_count_books(shelf) == bc / 2);

    for (i = 0; i < sc / 2; ++i) {
        sub = mume_bookshelf_get_shelf(shelf, i);
        snprintf(buf, sizeof(buf), "Shelf %d", i);
        test_assert(0 == strcmp(buf, mume_bookshelf_get_name(sub)));
        test_assert(mume_is_of(sub, mume_bookshelf_class()));
        test_assert(mume_bookshelf_parent_shelf(sub) == shelf);
        test_assert(mume_bookshelf_count_shelves(sub) == sc);
        test_assert(mume_bookshelf_count_books(sub) == bc);

        for (j = 0; j < sc; ++j) {
            sub2 = mume_bookshelf_get_shelf(sub, j);
            snprintf(buf, sizeof(buf), "Shelf %d", j);
            test_assert(0 == strcmp(buf, mume_bookshelf_get_name(sub2)));
            test_assert(mume_bookshelf_parent_shelf(sub2) == sub);
            test_assert(mume_bookshelf_count_shelves(sub2) == 0);
            test_assert(mume_bookshelf_count_books(sub2) == 0);
        }

        for (j = 0; j < bc; ++j) {
            book = mume_bookshelf_get_book(sub, i);
            snprintf(buf, sizeof(buf), "Book %d", i);
            test_assert(0 == strcmp(buf, mume_book_get_id(book)));
            test_assert(mume_book_get_path(book) == NULL);
            test_assert(mume_book_get_name(book) == NULL);
        }
    }

    for (i = 0; i < bc / 2; ++i) {
        book = mume_bookshelf_get_book(shelf, i);
        snprintf(buf, sizeof(buf), "Book %d", i);
        test_assert(0 == strcmp(buf, mume_book_get_id(book)));
        test_assert(mume_book_get_path(book) == NULL);
        test_assert(mume_book_get_name(book) == NULL);
    }

    mume_delete(shelf);
}

void all_tests(void)
{
    _test_bookmgr1();
    _test_bookmgr2();
    _test_bookshelf();
}
