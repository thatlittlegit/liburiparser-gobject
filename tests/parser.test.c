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

static gboolean compare_lists(GList* list, gchar* slash_separated_expected);

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
    gchar* path;
};

struct TestUri tests[] = {
    { "https://google.com/search/howsearchworks", "https", "google.com", "/search/howsearchworks" },
    { "gopher://gopher.floodgap.com/0/gopher/proxy", "gopher", "gopher.floodgap.com", "/0/gopher/proxy" },
    { "gemini://gemini.circumlunar.space/docs/specification.gmi", "gemini", "gemini.circumlunar.space", "/docs/specification.gmi" },
    { "data:text/plain;charset=utf-8,hello", "data", NULL, NULL },
    { "file:///etc/passwd", "file", NULL, "/etc/passwd" },
    { "http://http.rip", "http", "http.rip", NULL },
    { "irc://irc.freenode.net", "irc", "irc.freenode.net", NULL },
    { "geo:39.108889,-76.771389", "geo", NULL, NULL },
    //{ "ipp:hell", "ipp" }, XXX
    { "http://127.0.0.1", "http", "127.0.0.1", NULL },
    { "http://[0000:0000:0000:0000:0000:0000:0000:0000]", "http", "0000:0000:0000:0000:0000:0000:0000:0000", NULL }
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
        g_assert_true(compare_lists(ipath, "/changed"));
        g_assert_true(compare_lists(rpath, "/changed"));

        g_list_free_full(ipath, g_free);
        g_list_free(npath);
        g_list_free_full(rpath, g_free);
        g_value_unset(&Rpath);
        g_value_unset(&vpath);

        g_object_unref(uri);
    }
}

static gboolean compare_lists(GList* list, gchar* wanted_str)
{
    gchar* current = wanted_str + 1;
    for (int i = 0; i < g_list_length(list); i++) {
        gchar* next = strchr(current, '/');
        gchar* nc = g_strndup(current, next ? next - current : strlen(current));
        g_assert_cmpstr(g_list_nth_data(list, i), ==, nc);
        g_free(nc);
        current = strchr(current, '/') + 1;
    }

    return TRUE;
}

void path_segments_are_right()
{
    for (int i = 0; i < TEST_COUNT; i++) {
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);
        g_assert_null(err);
        g_assert_nonnull(uri);

        if (tests[i].path == NULL) {
            continue; // FIXME data uris, etc. have odd behavior
            g_assert_null(upg_uri_get_path(uri));
            gchar* pathstr = upg_uri_get_path_str(uri);
            g_assert_cmpstr(pathstr, ==, "");
            g_free(pathstr);
        }

        GList* pathl = upg_uri_get_path(uri);
        g_assert_true(compare_lists(pathl, tests[i].path));
        g_list_free_full(pathl, g_free);

        gchar* paths = upg_uri_get_path_str(uri);
        g_assert_cmpstr(paths, ==, tests[i].path);

        GList* list = NULL;
        list = g_list_append(list, "path");
        list = g_list_append(list, "set");
        list = g_list_append(list, "successfully");
        upg_uri_set_path(uri, list);
        GList* set = upg_uri_get_path(uri);
        g_list_free(list);
        g_assert_true(compare_lists(set, "/path/set/successfully"));
        g_list_free_full(set, g_free);
        gchar* sets = upg_uri_get_path_str(uri);
        g_assert_cmpstr(sets, ==, "/path/set/successfully");
        g_free(sets);
        gchar* sett = upg_uri_get_uri(uri);
        g_assert_nonnull(g_strrstr(sett, "/path/set/successfully"));
        g_free(sett);

        GList* npath = NULL;
        npath = g_list_append(npath, "path");
        npath = g_list_append(npath, "changed");
        upg_uri_set_path(uri, npath);
        GList* spath = upg_uri_get_path(uri);
        g_assert_true(compare_lists(spath, "/path/changed"));
        gchar* tpath = upg_uri_get_path_str(uri);
        g_assert_cmpstr(tpath, ==, "/path/changed");
        g_list_free(npath);
        g_list_free_full(spath, g_free);
        g_free(tpath);

        g_object_unref(uri);
    }
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/urigobj/version-check-accurate", version_check_accurate);
    g_test_add_func("/urigobj/new-returns-instance", new_returns_instance);
    g_test_add_func("/urigobj/new-returns-null-on-error", new_returns_null_on_error);
    g_test_add_func("/urigobj/schemes-are-correct", schemes_are_correct);
    g_test_add_func("/urigobj/can-reset-schemes", can_reset_schemes);
    g_test_add_func("/urigobj/to-string-is-reparsable", to_string_is_reparsable);
    g_test_add_func("/urigobj/host-is-correct", host_is_correct);
    g_test_add_func("/urigobj/host-is-resettable", host_is_resettable);
    g_test_add_func("/urigobj/properties-work", properties_work);
    g_test_add_func("/urigobj/path-segments-are-right", path_segments_are_right);

    return g_test_run();
}
