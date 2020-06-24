/* port.test.c
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

static void get_port()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        guint16 port = upg_uri_get_port(uri);
        g_assert_cmpuint(port, ==, tests[i]->port);

        g_object_unref(uri);
    }
}

static void change_port()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        upg_uri_set_port(uri, 1000);
        guint16 port = upg_uri_get_port(uri);
        g_assert_cmpuint(port, ==, 1000);

        g_object_unref(uri);
    }
}

static void get_port_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "port", &reciever);
        guint16 retd = g_value_get_uint(&reciever);
        g_assert_cmpuint(retd, ==, tests[i]->port);
        g_value_unset(&reciever);

        g_object_unref(uri);
    }
}

static void change_port_property_recv_normal()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_UINT);
        g_value_set_uint(&value, 1000);
        g_object_set_property(G_OBJECT(uri), "port", &value);
        g_value_unset(&value);

        guint set = upg_uri_get_port(uri);
        g_assert_cmpuint(set, ==, 1000);

        g_object_unref(uri);
    }
}

static void change_port_property_recv_property()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);

        GValue value = G_VALUE_INIT;
        g_value_init(&value, G_TYPE_UINT);
        g_value_set_uint(&value, 1000);
        g_object_set_property(G_OBJECT(uri), "port", &value);
        g_value_unset(&value);

        GValue reciever = G_VALUE_INIT;
        g_object_get_property(G_OBJECT(uri), "port", &reciever);
        guint retd = g_value_get_uint(&reciever);
        g_assert_cmpuint(retd, ==, 1000);
        g_value_unset(&reciever);

        g_object_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_get_port", get_port);
    g_test_add_func("/upg_uri_set_port", change_port);
    g_test_add_func("/gobject-properties/upg_uri_get_port", get_port_property);
    g_test_add_func("/gobject-properties/upg_uri_set_port (recieved without properties)", change_port_property_recv_normal);
    g_test_add_func("/gobject-properties/upg_uri_set_port (recieved with properties)", change_port_property_recv_property);
}
