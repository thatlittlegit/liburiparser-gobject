/* schemes.test.c
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

static void get_schemes()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        gchar* retd = upg_uri_get_scheme(uri);
        g_assert_cmpstr(retd, ==, tests[i]->scheme);
        g_free(retd);

        g_object_unref(uri);
    }
}

static void change_schemes()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        upg_uri_set_scheme(uri, "testscheme");
        gchar* set = upg_uri_get_scheme(uri);
        g_assert_cmpstr(set, ==, "testscheme");
        g_free(set);

        g_object_unref(uri);
    }
}

static void get_schemes_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "scheme", &value);
        const gchar* retd = g_value_get_string(&value);
        g_assert_cmpstr(tests[i]->scheme, ==, retd);

        g_value_unset(&value);
        g_object_unref(uri);
    }
}

static void change_schemes_property_recv_normal()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "fakescheme");
        g_object_set_property(G_OBJECT(uri), "scheme", &value);
        g_value_unset(&value);

        gchar* retd = upg_uri_get_scheme(uri);
        g_assert_cmpstr(retd, ==, "fakescheme");
        g_free(retd);

        g_object_unref(uri);
    }
}

static void change_schemes_property_recv_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "fakescheme");
        g_object_set_property(G_OBJECT(uri), "scheme", &value);
        g_value_unset(&value);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "scheme", &reciever);
        const gchar* retd = g_value_get_string(&reciever);
        g_assert_cmpstr(retd, ==, "fakescheme");
        g_value_unset(&reciever);

        g_object_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_get_scheme", get_schemes);
    g_test_add_func("/upg_uri_set_scheme", change_schemes);
    g_test_add_func("/gobject-properties/upg_uri_get_scheme (recieved without properties)", get_schemes_property);
    g_test_add_func("/gobject-properties/upg_uri_set_scheme (recieved without properties)", change_schemes_property_recv_normal);
    g_test_add_func("/gobject-properties/upg_uri_set_scheme (recieved with properties)", change_schemes_property_recv_property);
}
