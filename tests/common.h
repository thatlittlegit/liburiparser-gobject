/* common.h
 *
 * Copyright 2020 thatlittlegit <personal@thatlittlegit.tk>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <glib.h>
#include <liburiparser-gobject.h>
#include <locale.h>
#include <string.h>

typedef struct {
    const gchar* uri;
    const gchar* nonnormalized;
    const gchar* relative; /* with reference of
                            * '../1/../2/aaaa/bbbb/cccc/../file' applied
                            * (or '../2/aaaa/bbbb/file')
                            */
    const gchar* scheme;
    const gchar* userinfo;
    const gchar* username;
    const gchar* host;
    guint hostdata_proto;
    guint8 hostdata_data[16];
    guint16 port;
    GList* path;
    GHashTable* query;
    /* the query iteration can potentially be out of order; there's no
     * requirement (that I know of) to keep them in order. therefore, we keep
     * track here.
     */
    GList* query_order;
    const gchar* fragment;
    GHashTable* fragment_params;
} _Test, *Test;

/*
 * get_tests:
 *
 * > This is an internal API, meant to be used by tests only.
 *
 * Returns: (array zero-terminated=1): The tests, as an array.
 */
Test* get_tests();

gchar* join_glist(GList* list, gchar separator);
GList* split_to_glist(gchar* str, gchar separator);
gboolean compare_lists(GList* a, GList* b);
gboolean compare_list_and_str(GList* a, gchar* b, gchar separator);
gchar* hash_table_to_str(GList* order, GHashTable* table);
void assert_hash_tables_same(GHashTable* a, GHashTable* b);

#define FOR_EACH_CASE(x)       \
    Test* tests = get_tests(); \
    gint i = -1;               \
    while (tests[++i])

#define declare_tests                    \
    static void __register_tests();      \
    int main(int argc, char** argv)      \
    {                                    \
        setlocale(LC_ALL, "");           \
        g_test_init(&argc, &argv, NULL); \
        __register_tests();              \
        return g_test_run();             \
    }                                    \
    static void __register_tests()
