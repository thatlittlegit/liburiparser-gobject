/* userinfo.test.c
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

static void get_userinfo()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        gchar* retd = upg_uri_get_userinfo(uri);
        g_assert_cmpstr(retd, ==, tests[i]->userinfo);
        g_free(retd);

        g_object_unref(uri);
    }
}

static void get_username()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        gchar* retd = upg_uri_get_username(uri);
        g_assert_cmpstr(retd, ==, tests[i]->username);
        g_free(retd);

        g_object_unref(uri);
    }
}

static void change_userinfo()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);
        upg_uri_set_userinfo(uri, "administrator:secure");

        gchar* new_userinfo = upg_uri_get_userinfo(uri);
        g_assert_cmpstr(new_userinfo, ==, "administrator:secure");
        g_free(new_userinfo);

        gchar* new_username = upg_uri_get_username(uri);
        g_assert_cmpstr(new_username, ==, "administrator");
        g_free(new_username);

        g_object_unref(uri);
    }
}

static void get_userinfo_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "userinfo", &reciever);
        const gchar* retd = g_value_get_string(&reciever);
        g_assert_cmpstr(retd, ==, tests[i]->userinfo);

        g_value_unset(&reciever);
        g_object_unref(uri);
    }
}

static void get_username_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "username", &reciever);
        const gchar* retd = g_value_get_string(&reciever);
        g_assert_cmpstr(retd, ==, tests[i]->username);

        g_value_unset(&reciever);
        g_object_unref(uri);
    }
}

static void change_userinfo_property_recv_normal()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "administrator:password");
        g_object_set_property(G_OBJECT(uri), "userinfo", &value);
        g_value_unset(&value);

        gchar* retdUi = upg_uri_get_userinfo(uri);
        g_assert_cmpstr(retdUi, ==, "administrator:password");
        g_free(retdUi);
        gchar* retdUn = upg_uri_get_username(uri);
        g_assert_cmpstr(retdUn, ==, "administrator");
        g_free(retdUn);

        g_object_unref(uri);
    }
}

static void change_userinfo_property_recv_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_STRING);
        g_value_set_static_string(&value, "administrator:password");
        g_object_set_property(G_OBJECT(uri), "userinfo", &value);
        g_value_unset(&value);

        GValue recieverUi = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "userinfo", &recieverUi);
        const gchar* retdUi = g_value_get_string(&recieverUi);
        g_assert_cmpstr(retdUi, ==, "administrator:password");
        g_value_unset(&recieverUi);

        GValue recieverUn = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "username", &recieverUn);
        const gchar* retdUn = g_value_get_string(&recieverUn);
        g_assert_cmpstr(retdUn, ==, "administrator");
        g_value_unset(&recieverUn);

        g_object_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_get_userinfo", get_userinfo);
    g_test_add_func("/upg_uri_get_username", get_username);
    g_test_add_func("/upg_uri_set_userinfo", change_userinfo);
    g_test_add_func("/gobject-properties/upg_uri_get_userinfo", get_userinfo_property);
    g_test_add_func("/gobject-properties/upg_uri_get_username", get_username_property);
    g_test_add_func("/gobject-properties/upg_uri_set_userinfo (recieved without properties)", change_userinfo_property_recv_normal);
    g_test_add_func("/gobject-properties/upg_uri_set_userinfo (recieved with properties)", change_userinfo_property_recv_property);
}
