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
#include <glib.h>
#include <liburiparser-gobject.h>
#include <locale.h>

void versions_correct()
{
    g_assert_true(UPG_CHECK_VERSION(0, 0, 0));
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
    g_test_incomplete("not yet implemented");
    return;
    GError* error;
    g_assert_null(upg_uri_new("ä", &error));
    g_assert_nonnull(error);
}

struct TestUri {
    gchar* uri;
    gchar* scheme;
    gchar* host;
};

struct TestUri tests[] = {
    { "https://google.com", "https", "google.com" },
    { "gopher://gopher.floodgap.com", "gopher", "gopher.floodgap.com" },
    { "gemini://gemini.circumlunar.space", "gemini", "gemini.circumlunar.space" },
    { "data:text/plain;charset=utf-8,hello", "data", NULL },
    { "file:///etc/passwd", "file", NULL },
    { "http://http.rip", "http", "http.rip" },
    { "irc://irc.freenode.net", "irc", "irc.freenode.net" },
    { "geo:39.108889,-76.771389", "geo", NULL },
    //{ "ipp:hell", "ipp" }, XXX
    { "http://127.0.0.1", "http", "127.0.0.1" },
    { "http://[0000:0000:0000:0000:0000:0000:0000:0000]", "http", "0000:0000:0000:0000:0000:0000:0000:0000" }
};
#define TEST_COUNT (sizeof(tests) / sizeof(struct TestUri))

guint8 local_data4[] = { 4, 127, 0, 0, 1 };
guint8 undef_data6[] = { 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
guint8* hostdatae[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    local_data4,
    undef_data6,
};

void schemes_are_correct()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);
        gchar* scheme = upg_uri_get_scheme(uri);
        g_assert_cmpstr(scheme, ==, tests[i].scheme);

        g_free(scheme);
        g_object_unref(uri);
    }
}

void can_reset_schemes()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* oscheme = upg_uri_get_scheme(uri);
        upg_uri_set_scheme(uri, "fake");
        gchar* nscheme = upg_uri_get_scheme(uri);
        g_assert_cmpstr(nscheme, !=, oscheme);
        g_assert_cmpstr(nscheme, ==, "fake");

        gchar* tstr = upg_uri_get_uri(uri);
        g_assert_nonnull(tstr);

        UpgUri* new_uri = upg_uri_new(tstr, &err);
        gchar* nstr = upg_uri_get_uri(new_uri);
        g_assert_cmphex((guint64)nstr, ==, (guint64)strstr(nstr, "fake"));

        g_free(oscheme);
        g_free(nscheme);
        g_free(tstr);
        g_free(nstr);
        g_object_unref(uri);
        g_object_unref(new_uri);
    }
}

void to_string_is_reparsable()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);
        gchar* oscheme = upg_uri_get_scheme(uri);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* uristr = upg_uri_get_uri(uri);
        g_assert_cmpstr(uristr, ==, tests[i].uri);

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
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);

        gchar* host = upg_uri_get_host(uri);
        g_assert_cmpstr(host, ==, tests[i].host);

        if (hostdatae[i] != NULL) {
            guint8 protocol;
            const guint8* hostdata = upg_uri_get_host_data(uri, &protocol);
            g_assert_cmpint(protocol, ==, *hostdatae[i]);
            gint protodlen = protocol == 4 ? 4 : 16;
            g_assert_cmpmem(hostdata, protodlen, hostdatae[i] + 1, protodlen);
        }

        g_free(host);
        g_object_unref(uri);
    }
}

void host_is_resettable()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);

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
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);
        g_assert_null(err);
        g_assert_nonnull(uri);

        GValue vscheme = G_VALUE_INIT;
        g_value_init(&vscheme, G_TYPE_STRING);
        g_value_set_static_string(&vscheme, "junk");
        g_object_set_property(G_OBJECT(uri), "scheme", &vscheme);
        gchar* nscheme = upg_uri_get_scheme(uri);
        g_assert_cmpstr(nscheme, ==, "junk");
        g_free(nscheme);
        g_value_unset(&vscheme);
    }
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/urigobj/versions-correct", versions_correct);
    g_test_add_func("/urigobj/new-returns-instance", new_returns_instance);
    g_test_add_func("/urigobj/new-returns-null-on-error", new_returns_null_on_error);
    g_test_add_func("/urigobj/schemes-are-correct", schemes_are_correct);
    g_test_add_func("/urigobj/can-reset-schemes", can_reset_schemes);
    g_test_add_func("/urigobj/to-string-is-reparsable", to_string_is_reparsable);
    g_test_add_func("/urigobj/host-is-correct", host_is_correct);
    g_test_add_func("/urigobj/host-is-resettable", host_is_resettable);
    g_test_add_func("/urigobj/properties-work", properties_work);

    return g_test_run();
}
