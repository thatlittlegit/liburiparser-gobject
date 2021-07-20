/* comparison.test.c
 *
 * Copyright 2021 thatlittlegit <personal@thatlittlegit.tk>
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

static void hash(void)
{
    UpgUri* dummy = upg_uri_new("//test/hello", NULL);
    guint hash = upg_uri_hash(dummy);
    upg_uri_unref(dummy);

    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);
        g_assert_cmpuint(upg_uri_hash(uri), !=, hash);
        upg_uri_unref(uri);
    }
}

static void equal(void)
{
    UpgUri* dummy = upg_uri_new("//test/hello", NULL);

    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* second = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* copy = upg_uri_copy(uri);

        g_assert_false(uri == second);
        g_assert_false(copy == uri);

        g_assert_false(upg_uri_equal(uri, dummy));
        g_assert_true(upg_uri_equal(uri, uri));
        g_assert_true(upg_uri_equal(uri, second));
        g_assert_true(upg_uri_equal(uri, copy));

        // TODO it wouldn't be unreasonable to change every property

        upg_uri_set_fragment(uri, "x--test");
        g_assert_true(upg_uri_equal(uri, uri));
        g_assert_false(upg_uri_equal(uri, second));
        g_assert_false(upg_uri_equal(uri, copy));
        g_assert_false(upg_uri_equal(uri, dummy));

        upg_uri_unref(uri);
        upg_uri_unref(second);
        upg_uri_unref(copy);
    }

    upg_uri_unref(dummy);
}

static void nearly_equal(void)
{
    UpgUri* dummy = upg_uri_new("//test/hello", NULL);

    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* second = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* copy = upg_uri_copy(uri);

        g_assert_false(uri == second);
        g_assert_false(copy == uri);

        g_assert_false(upg_uri_nearly_equal(uri, dummy));
        g_assert_true(upg_uri_nearly_equal(uri, uri));
        g_assert_true(upg_uri_nearly_equal(uri, second));
        g_assert_true(upg_uri_nearly_equal(uri, copy));

        // TODO it wouldn't be unreasonable to change every property

        upg_uri_set_fragment(uri, "x--test");
        g_assert_true(upg_uri_nearly_equal(uri, uri));
        g_assert_true(upg_uri_nearly_equal(uri, second));
        g_assert_true(upg_uri_nearly_equal(uri, copy));
        g_assert_false(upg_uri_nearly_equal(uri, dummy));

        upg_uri_unref(uri);
        upg_uri_unref(second);
        upg_uri_unref(copy);
    }

    upg_uri_unref(dummy);
}

declare_tests
{
    g_test_add_func("/upg_uri_hash", hash);
    g_test_add_func("/upg_uri_equal", equal);
    g_test_add_func("/upg_uri_nearly_equal", nearly_equal);
}
