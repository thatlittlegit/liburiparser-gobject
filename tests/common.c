/* common.c
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
#include <glib.h>
#include <json-glib/json-glib.h>

static Test* tests = NULL;

Test* get_tests()
{
    if (tests != NULL) {
        return tests;
    }

    GError* err = NULL;
    JsonParser* parser = json_parser_new();
    json_parser_load_from_file(parser, JSONFILE, &err);
    if (err != NULL) {
        g_error("failed to parse JSON: %s", err->message);
    }

    JsonNode* root = json_parser_get_root(parser);
    JsonArray* array = json_node_get_array(root);

    gint arraylen = json_array_get_length(array);
    tests = g_malloc0(sizeof(Test) * (arraylen + 1));

    for (guint i = 0; i < arraylen; i++) {
        if (json_array_get_null_element(array, i)) {
            continue;
        }
        JsonObject* object = json_array_get_object_element(array, i);
        Test test = g_new0(_Test, 1);
        tests[i] = test;

        test->uri = g_strdup(json_object_get_string_member(object, "uri"));
        test->scheme = g_strdup(json_object_get_string_member(object, "scheme"));
        test->host = g_strdup(json_object_get_string_member(object, "hostname"));

        if (json_object_has_member(object, "hostdata")) {
            JsonObject* hostobj = json_object_get_object_member(object, "hostdata");
            test->hostdata_proto = json_object_get_int_member(hostobj, "protocol");

            JsonArray* hostdata = json_object_get_array_member(hostobj, "data");
            gint hostlen = json_array_get_length(hostdata);
            for (int i = 0; i < 16 && i < hostlen; i++) {
                test->hostdata_data[i] = (guint8)json_array_get_int_element(hostdata, i);
            }
        } else {
            test->hostdata_proto = 0;
        }

        test->path = NULL;
        JsonArray* path = json_object_get_array_member(object, "path");
        gint pathlen = json_array_get_length(path);
        for (int i = pathlen - 1; i >= 0; i--) {
            test->path = g_list_append(test->path, g_strdup(json_array_get_string_element(path, i)));
        }
        test->path = g_list_reverse(test->path);
    }
    tests[arraylen - 1] = NULL;

    g_object_unref(parser);
    return tests;
}

gchar* join_glist(GList* list, gchar separator)
{
    gint len = g_list_length(list);
    GString* output = g_string_new(NULL);

    for (int i = 0; i < len; i++) {
        g_string_append_c(output, separator);
        g_string_append(output, g_list_nth_data(list, i));
    }

    return g_string_free(output, FALSE);
}
