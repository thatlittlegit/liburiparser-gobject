/* parent.test.c
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

static void is_below(void)
{
    FOR_EACH_CASE(tests)
    {
        UpgUri* uri = upg_uri_new(tests[i]->uri, NULL);
        UpgUri* copy;

        g_assert_true(upg_uri_is_below(uri, uri, UPG_HIERARCHY_STRICT));
        g_assert_true(upg_uri_is_below(uri, uri, UPG_HIERARCHY_LAX));
        g_assert_false(upg_uri_is_below(uri, uri, UPG_HIERARCHY_NOTSELF));

        copy = upg_uri_copy(uri);
        upg_uri_set_scheme(copy, "x-other-proto");
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_true(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        upg_uri_unref(copy);

        copy = upg_uri_copy(uri);
        upg_uri_set_userinfo(copy, "user:pass");
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_true(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        upg_uri_unref(copy);

        copy = upg_uri_copy(uri);
        upg_uri_set_host(copy, "other-host.net");
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        upg_uri_unref(copy);

        copy = upg_uri_copy(uri);
        upg_uri_set_port(copy, 25565);
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        upg_uri_unref(copy);

        copy = upg_uri_copy(uri);
        upg_uri_set_path_str(copy, "/testingtesting098");
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        upg_uri_unref(copy);

        copy = upg_uri_copy(uri);
        GList* components = upg_uri_get_path(copy);
        components = g_list_append(components, g_strdup("beneath"));
        upg_uri_set_path(copy, components);
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_STRICT));
        g_assert_false(upg_uri_is_below(copy, uri, UPG_HIERARCHY_LAX));
        g_assert_true(upg_uri_is_below(uri, copy, UPG_HIERARCHY_STRICT));
        g_assert_true(upg_uri_is_below(uri, copy, UPG_HIERARCHY_LAX));
        g_list_free_full(components, g_free);
        upg_uri_unref(copy);

        upg_uri_unref(uri);
    }
}

declare_tests
{
    g_test_add_func("/upg_uri_is_below", is_below);
}
