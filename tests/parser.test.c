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
    g_assert_true(LIBURIPARSER_GOBJECT_CHECK_VERSION(0, 0, 0));
}

void new_returns_instance()
{
    UpgUri* uri = upg_uri_new("https://test.test/path", NULL);
    g_assert_nonnull(uri);
    g_assert_true(G_TYPE_CHECK_INSTANCE_TYPE(uri, UPG_TYPE_URI));
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
};

struct TestUri tests[] = {
    { "https://google.com", "https" },
    { "gopher://gopher.floodgap.com", "gopher" },
    { "gemini://gemini.circumlunar.space", "gemini" },
    { "data:text/plain;charset=utf-8,hello", "data" },
    { "file:///etc/passwd", "file" },
    { "http://http.rip", "http" },
    { "irc://irc.freenode.net", "irc" },
    { "geo:39.108889,-76.771389", "geo" },
    //{ "ipp:hell", "ipp" }, XXX
};

void schemes_are_correct()
{
    for (int i = 0; i < 8; i++) { // TODO sizeof or whatever
        GError* err = NULL;
        UpgUri* uri = upg_uri_new(tests[i].uri, &err);

        g_assert_null(err);
        g_assert_nonnull(uri);
        g_assert_cmpstr(upg_uri_get_scheme(uri), ==, tests[i].scheme);

        g_object_unref(uri);
    }
}

void can_reset_schemes()
{
    for (int i = 0; i < 8; i++) {
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
    for (int i = 0; i < 8; i++) {
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

    return g_test_run();
}