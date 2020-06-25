/* parser.test.c
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
#include "common.h"

void version_check_accurate()
{
    g_assert_false(UPG_CHECK_VERSION(UPG_MAJOR_VERSION - 1, 0, 0));
    g_assert_true(UPG_CHECK_VERSION(UPG_MAJOR_VERSION, UPG_MINOR_VERSION - 1, UPG_PATCH_VERSION + 1));
    g_assert_false(UPG_CHECK_VERSION(UPG_MAJOR_VERSION, UPG_MINOR_VERSION + 1, UPG_PATCH_VERSION + 1));
    g_assert_false(UPG_CHECK_VERSION(UPG_MAJOR_VERSION, UPG_MINOR_VERSION, UPG_PATCH_VERSION + 1));
    g_assert_false(upg_check_version(UPG_MAJOR_VERSION - 1, 0, 0));
    g_assert_true(upg_check_version(UPG_MAJOR_VERSION, UPG_MINOR_VERSION - 1, UPG_PATCH_VERSION + 1));
    g_assert_false(upg_check_version(UPG_MAJOR_VERSION, UPG_MINOR_VERSION + 1, UPG_PATCH_VERSION + 1));
    g_assert_false(UPG_CHECK_VERSION(UPG_MAJOR_VERSION, UPG_MINOR_VERSION, UPG_PATCH_VERSION + 1));
}

void new_returns_instance()
{
    UpgUri* uri = upg_uri_new("https://test.test/path", NULL);
    g_assert_nonnull(uri);
    g_assert_true(G_TYPE_CHECK_INSTANCE_TYPE(uri, UPG_TYPE_URI));
    g_object_unref(uri);
}

void new_returns_null_on_error()
{
    GError* error = NULL;
    g_assert_null(upg_uri_new("ä", &error));
    g_assert_error(error, UPG_ERROR, UPG_ERR_PARSE);
    g_error_free(error);
}

void to_string_is_reparsable()
{
    FOR_EACH_CASE(tests)
    {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &err);
        gchar* oscheme = upg_uri_get_scheme(uri);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* uristr = upg_uri_to_string(uri);
        g_assert_cmpstr(uristr, ==, tests[i]->uri);

        UpgUri* reparsed = upg_uri_new(uristr, &err);
        g_assert_null(err);
        g_assert_nonnull(reparsed);

        gchar* rscheme = upg_uri_get_scheme(reparsed);
        g_assert_cmpstr(oscheme, ==, rscheme);

        g_free(rscheme);
        g_free(oscheme);
        g_free(uristr);
        g_object_unref(uri);
        g_object_unref(reparsed);
    }
}

void host_is_correct()
{
    FOR_EACH_CASE(tests)
    {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* host = upg_uri_get_host(uri);
        g_assert_cmpstr(host, ==, tests[i]->host);

        if (tests[i]->hostdata_proto != 0) {
            guint8 protocol;
            const guint8* hostdata = upg_uri_get_host_data(uri, &protocol);
            g_assert_cmpint(protocol, ==, tests[i]->hostdata_proto);
            gint protodlen = protocol == 4 ? 4 : 16;
            g_assert_cmpmem(hostdata, protodlen, tests[i]->hostdata_data, protodlen);
        }

        g_free(host);
        g_object_unref(uri);
    }
}

void host_is_resettable()
{
    FOR_EACH_CASE(tests)
    {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* ohost = upg_uri_get_host(uri);
        upg_uri_set_host(uri, "localhost");
        gchar* nhost = upg_uri_get_host(uri);
        g_assert_cmpstr(ohost, !=, nhost);
        g_assert_cmpstr(nhost, ==, "localhost");

        g_free(ohost);
        g_free(nhost);
        g_object_unref(uri);
    }
}

void properties_work()
{
    FOR_EACH_CASE(tests)
    {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &err);
        g_assert_null(err);
        g_assert_nonnull(uri);

        GValue vhost = G_VALUE_INIT;
        g_value_init(&vhost, G_TYPE_STRING);
        g_value_set_static_string(&vhost, "netmetube.com");
        g_object_set_property(G_OBJECT(uri), "host", &vhost);
        gchar* nhost = upg_uri_get_host(uri);
        g_assert_cmpstr(nhost, ==, "netmetube.com");
        g_free(nhost);
        g_value_unset(&vhost);

        GValue vpath = G_VALUE_INIT;
        g_value_init(&vpath, G_TYPE_POINTER);
        GList* npath = NULL;
        npath = g_list_append(npath, "changed");
        g_value_set_pointer(&vpath, npath);
        g_object_set_property(G_OBJECT(uri), "path", &vpath);
        GList* rpath = upg_uri_get_path(uri);
        GList* ipath = NULL;
        GValue Rpath = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "path", &Rpath);
        ipath = g_value_get_pointer(&Rpath);
        g_assert_true(compare_list_and_str(ipath, "/changed", '/'));
        g_assert_true(compare_list_and_str(rpath, "/changed", '/'));

        g_list_free_full(ipath, g_free);
        g_list_free(npath);
        g_list_free_full(rpath, g_free);
        g_value_unset(&Rpath);
        g_value_unset(&vpath);

        GValue vquery = G_VALUE_INIT;
        g_value_init(&vquery, G_TYPE_POINTER);
        GHashTable* nquery = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(nquery, "example", "value");
        g_value_set_pointer(&vquery, nquery);
        g_object_set_property(G_OBJECT(uri), "query", &vquery);
        GHashTable* rquery = upg_uri_get_query(uri);
        g_assert_cmpstr(g_hash_table_lookup(rquery, "example"), ==, "value");
        GValue Rquery = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "query", &Rquery);
        GHashTable* Rhuery = g_value_get_pointer(&Rquery);
        g_assert_cmpstr(g_hash_table_lookup(Rhuery, "example"), ==, "value");

        g_hash_table_unref(nquery);
        g_hash_table_unref(rquery);
        g_hash_table_unref(Rhuery);

        GValue vqstr = G_VALUE_INIT;
        g_value_init(&vqstr, G_TYPE_STRING);
        g_value_set_static_string(&vqstr, "?ababa=aba&caca=non");
        g_object_set_property(G_OBJECT(uri), "query_str", &vqstr);
        gchar* rqstr = upg_uri_get_query_str(uri);
        g_assert_cmpstr(rqstr, ==, "?ababa=aba&caca=non");
        GValue Vqstr = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "query_str", &Vqstr);
        gchar* Rqstr = (gchar*)g_value_get_string(&Vqstr);
        g_assert_cmpstr(Rqstr, ==, "?ababa=aba&caca=non");
        g_assert_cmpstr(Rqstr, ==, rqstr);
        GValue Vqhtr = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "query", &Vqhtr);
        GHashTable* Rqhtr = g_value_get_pointer(&Vqhtr);
        g_assert_cmpstr(g_hash_table_lookup(Rqhtr, "ababa"), ==, "aba");
        g_assert_cmpstr(g_hash_table_lookup(Rqhtr, "caca"), ==, "non");

        g_free(rqstr);
        g_value_unset(&vqstr);
        g_value_unset(&Vqstr);
        g_value_unset(&Vqhtr);
        g_hash_table_unref(Rqhtr);

        g_object_unref(uri);
    }
}

void path_segments_are_right()
{
    FOR_EACH_CASE(tests)
    {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &err);
        g_assert_null(err);
        g_assert_nonnull(uri);

        GList* pathl = upg_uri_get_path(uri);
        g_assert_true(compare_lists(pathl, tests[i]->path));
        g_list_free_full(pathl, g_free);

        gchar* paths = upg_uri_get_path_str(uri);
        gchar* expected = join_glist(tests[i]->path, '/');
        g_assert_cmpstr(paths, ==, expected);
        g_free(expected);
        g_free(paths);

        GList* list = NULL;
        list = g_list_append(list, "path");
        list = g_list_append(list, "set");
        list = g_list_append(list, "successfully");
        upg_uri_set_path(uri, list);
        GList* set = upg_uri_get_path(uri);
        g_list_free(list);
        g_assert_true(compare_list_and_str(set, "/path/set/successfully", '/'));
        g_list_free_full(set, g_free);
        gchar* sets = upg_uri_get_path_str(uri);
        g_assert_cmpstr(sets, ==, "/path/set/successfully");
        g_free(sets);
        gchar* sett = upg_uri_to_string(uri);
        g_assert_nonnull(g_strrstr(sett, "/path/set/successfully"));
        g_free(sett);

        g_object_unref(uri);
    }
}

void query_is_right()
{
    FOR_EACH_CASE(tests)
    {
        GError* error = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &error);
        g_assert_nonnull(uri);
        g_assert_null(error);

        if (tests[i]->query == NULL) {
            g_assert_null(upg_uri_get_query(uri));
            g_object_unref(uri);
            continue;
        }

        gchar* correct_str = hash_table_to_str(tests[i]->query_order, tests[i]->query);
        gchar* returned_str = upg_uri_get_query_str(uri);
        g_assert_cmpstr(returned_str, ==, correct_str);

        GHashTable* table = upg_uri_get_query(uri);
        GHashTableIter iter;
        g_hash_table_iter_init(&iter, table);
        gpointer key = NULL, value = NULL;
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            g_assert_cmpstr(value, ==, g_hash_table_lookup(tests[i]->query, key));
        }
        g_assert_nonnull(table);

        g_free(correct_str);
        g_free(returned_str);
        g_hash_table_unref(table);

        g_object_unref(uri);
    }
}

void query_is_resettable()
{
    FOR_EACH_CASE(tests)
    {
        GError* error = NULL;
        UpgUri* uri = upg_uri_new(tests[i]->uri, &error);
        g_assert_nonnull(uri);
        g_assert_null(error);

        GHashTable* new_table = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(new_table, "a", "b");
        g_hash_table_insert(new_table, "1234567890", "`1234567890-");
        GList* new_order = NULL;
        new_order = g_list_append(new_order, "a");
        new_order = g_list_append(new_order, "1234567890");

        g_assert_true(upg_uri_set_query(uri, new_table));
        gchar* new_str = upg_uri_get_query_str(uri);
        g_assert_nonnull(strstr(new_str, "a=b"));
        g_assert_nonnull(strstr(new_str, "1234567890=`1234567890-"));
        GHashTable* new_retd = upg_uri_get_query(uri);
        gchar* new_retd_str = hash_table_to_str(new_order, new_retd);
        g_assert_cmpstr(new_retd_str, ==, "?a=b&1234567890=`1234567890-");
        g_free(new_retd_str);
        g_assert_cmpstr(g_hash_table_lookup(new_retd, "a"), ==, "b");
        g_assert_cmpstr(g_hash_table_lookup(new_retd, "1234567890"), ==, "`1234567890-");

        g_assert_true(upg_uri_set_query_str(uri, "?nonono=no"));
        gchar* sec_str = upg_uri_get_query_str(uri);
        g_assert_cmpstr(sec_str, ==, "?nonono=no");
        GHashTable* sec_retd = upg_uri_get_query(uri);
        g_assert_cmpstr(g_hash_table_lookup(sec_retd, "nonono"), ==, "no");

        g_list_free(new_order);
        g_hash_table_unref(new_table);
        g_free(new_str);
        g_hash_table_unref(new_retd);
        g_free(sec_str);
        g_hash_table_unref(sec_retd);
        g_object_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/urigobj/version-check-accurate", version_check_accurate);
    g_test_add_func("/urigobj/new-returns-instance", new_returns_instance);
    g_test_add_func("/urigobj/new-returns-null-on-error", new_returns_null_on_error);
    g_test_add_func("/urigobj/to-string-is-reparsable", to_string_is_reparsable);
    g_test_add_func("/urigobj/host-is-correct", host_is_correct);
    g_test_add_func("/urigobj/host-is-resettable", host_is_resettable);
    g_test_add_func("/urigobj/properties-work", properties_work);
    g_test_add_func("/urigobj/path-segments-are-right", path_segments_are_right);
    g_test_add_func("/urigobj/query-is-right", query_is_right);
    g_test_add_func("/urigobj/query-is-resettable", query_is_resettable);
}
