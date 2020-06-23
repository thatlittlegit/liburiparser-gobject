/* fragments.test.c
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

static void get_fragment()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        gchar* retd = upg_uri_get_fragment(uri);
        g_assert_cmpstr(retd, ==, tests[i]->fragment);
        g_free(retd);

        g_object_unref(uri);
    }
}

static void change_fragment()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        upg_uri_set_fragment(uri, "hello");
        gchar* new_fragment = upg_uri_get_fragment(uri);
        g_assert_cmpstr(new_fragment, ==, "hello");
        g_free(new_fragment);

        g_object_unref(uri);
    }
}

static void get_fragment_table()
{
    FOR_EACH_CASE(tests)
    {
        if (tests[i]->fragment_params == NULL) {
            continue;
        }

        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GHashTable* params = upg_uri_get_fragment_params(uri);
        assert_hash_tables_same(params, tests[i]->fragment_params);
        g_hash_table_unref(params);

        g_object_unref(uri);
    }
}

static void set_fragment_table()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GHashTable* new_fragment = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(new_fragment, "param1", "value1");
        g_hash_table_insert(new_fragment, "param2", "value2");
        g_hash_table_insert(new_fragment, "param3", "value3");
        upg_uri_set_fragment_params(uri, new_fragment);

        GHashTable* retd = upg_uri_get_fragment_params(uri);
        assert_hash_tables_same(retd, new_fragment);

        g_hash_table_unref(new_fragment);
        g_hash_table_unref(retd);
        g_object_unref(uri);
    }
}

static void get_fragment_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "fragment", &reciever);
        const gchar* value = g_value_get_string(&reciever);
        g_assert_cmpstr(value, ==, tests[i]->fragment);

        g_value_unset(&reciever);
        g_object_unref(uri);
    }
}

static void change_fragment_property_recv_normal()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "fragment");
        g_object_set_property(G_OBJECT(uri), "fragment", &value);

        gchar* retd = upg_uri_get_fragment(uri);
        g_assert_cmpstr(retd, ==, "fragment");

        g_free(retd);
        g_value_unset(&value);
        g_object_unref(uri);
    }
}

static void change_fragment_property_recv_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "fragment");
        g_object_set_property(G_OBJECT(uri), "fragment", &value);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "fragment", &reciever);
        const gchar* retd = g_value_get_string(&reciever);
        g_assert_cmpstr(retd, ==, "fragment");

        g_value_unset(&reciever);
        g_value_unset(&value);
        g_object_unref(uri);
    }
}

static void change_fragment_params_property_recv_normal()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GHashTable* new_table = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(new_table, "param1", "value1");
        g_hash_table_insert(new_table, "param2", "value2");

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_POINTER);
        g_value_set_pointer(&value, new_table);
        g_object_set_property(G_OBJECT(uri), "fragment-params", &value);

        GHashTable* retd = upg_uri_get_fragment_params(uri);
        assert_hash_tables_same(retd, new_table);

        g_hash_table_unref(new_table);
        g_value_unset(&value);
        g_hash_table_unref(retd);
        g_object_unref(uri);
    }
}

static void change_fragment_params_property_recv_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GHashTable* new_table = g_hash_table_new(g_str_hash, g_str_equal);
        g_hash_table_insert(new_table, "param1", "value1");
        g_hash_table_insert(new_table, "param2", "value2");

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_POINTER);
        g_value_set_pointer(&value, new_table);
        g_object_set_property(G_OBJECT(uri), "fragment-params", &value);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "fragment-params", &reciever);
        GHashTable* retd = g_value_get_pointer(&reciever);
        assert_hash_tables_same(retd, new_table);

        g_hash_table_unref(new_table);
        g_value_unset(&value);
        g_value_unset(&reciever);
        g_hash_table_unref(retd);
        g_object_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_get_fragment", get_fragment);
    g_test_add_func("/upg_uri_set_fragment", change_fragment);
    g_test_add_func("/upg_uri_get_fragment_params", get_fragment_table);
    g_test_add_func("/upg_uri_set_fragment_params", set_fragment_table);
    g_test_add_func("/gobject-properties/upg_uri_get_fragment", get_fragment_property);
    g_test_add_func("/gobject-properties/upg_uri_set_fragment (recieved without properties)", change_fragment_property_recv_normal);
    g_test_add_func("/gobject-properties/upg_uri_set_fragment (recieved with properties)", change_fragment_property_recv_property);
    g_test_add_func("/gobject-properties/upg_uri_set_fragment_params (recieved without properties)", change_fragment_params_property_recv_normal);
    g_test_add_func("/gobject-properties/upg_uri_set_fragment_params (recieved with properties)", change_fragment_params_property_recv_property);
}
