/* references.test.c
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

static void apply_reference()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* base = upg_uri_new(tests[i]->uri, NULL);

        UpgUri* applied = upg_uri_apply_reference(base, "../1/../2/aaaa/bbbb/cccc/../file", NULL);
        gchar* str = upg_uri_to_string(applied);
        g_assert_cmpstr(str, ==, tests[i]->relative);

        g_free(str);
        g_object_unref(base);
        g_object_unref(applied);
    }
}

static void apply_reference_throws_when_needed()
{
    GError* err = NULL;

    UpgUri* base1 = upg_uri_new("https://example.edu", NULL);
    upg_uri_apply_reference(base1, "ä", &err);
    g_assert_error(err, UPG_ERROR, UPG_ERR_PARSE);
    g_error_free(err);
    err = NULL;
    g_object_unref(base1);

    UpgUri* base2 = upg_uri_new("example", NULL);
    upg_uri_apply_reference(base2, "..", &err);
    g_assert_error(err, UPG_ERROR, UPG_ERR_REFERENCE);
    g_error_free(err);
    err = NULL;
    g_object_unref(base2);

    // XXX cause a UPG_ERR_NORMALIZE error
}

static void subtract_reference()
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* initial = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* modified = upg_uri_new(tests[i]->relative, NULL);

        gchar* result = upg_uri_subtract_to_reference(initial, modified, NULL);
        g_assert_nonnull(strstr(result, "2/aaaa/bbbb/file"));
        g_free(result);

        g_object_unref(modified);
        g_object_unref(initial);
    }
}

static void subtract_reference_throws_when_needed()
{
    GError* err = NULL;

    UpgUri* base1 = upg_uri_new("example", NULL);
    UpgUri* source1 = upg_uri_new("example", NULL);
    upg_uri_subtract_to_reference(base1, source1, &err);
    g_assert_error(err, UPG_ERROR, UPG_ERR_REFERENCE);
    g_error_free(err);
    g_object_unref(base1);
    g_object_unref(source1);
}

declare_tests
{
    g_test_add_func("/upg_uri_apply_reference", apply_reference);
    g_test_add_func("/upg_uri_apply_reference: throws when needed", apply_reference_throws_when_needed);
    g_test_add_func("/upg_uri_subtract_reference", subtract_reference);
    g_test_add_func("/upg_uri_subtract_reference: throws when needed", subtract_reference_throws_when_needed);
}
