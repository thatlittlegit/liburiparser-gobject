/* common.h
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

typedef struct {
    const gchar* uri;
    const gchar* scheme;
    const gchar* host;
    guint hostdata_proto;
    guint8 hostdata_data[16];
    GList* path;
} _Test, *Test;

/*
 * get_tests:
 *
 * > This is an internal API, meant to be used by tests only.
 *
 * Returns: (array zero-terminated=1): The tests, as an array.
 */
Test* get_tests();

gchar* join_glist(GList* list, gchar separator);
GList* split_to_glist(gchar* str, gchar separator);

#define FOR_EACH_CASE(x)       \
    Test* tests = get_tests(); \
    gint i = -1;               \
    while (tests[++i])