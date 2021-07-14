/* copy.test.c
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

static void copy_test(void)
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* original = upg_uri_new(tests[i]->uri, NULL);
        g_assert_nonnull(original);

        UpgUri* copy = upg_uri_copy(original);
        g_assert_nonnull(copy);

        g_assert_cmphex((guint64)copy, !=, (guint64)original);

        upg_uri_set_scheme(original, "x-test-scheme");
        char* scheme = upg_uri_get_scheme(copy);
        g_assert_cmpstr(scheme, ==, tests[i]->scheme);
        g_free(scheme);

        upg_uri_unref(copy);
        upg_uri_unref(original);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_copy", copy_test);
}
