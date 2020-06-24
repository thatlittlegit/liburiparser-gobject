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

        if (json_object_has_member(object, "userinfo")) {
            test->userinfo = g_strdup(json_object_get_string_member(object, "userinfo"));
        } else {
            test->userinfo = NULL;
        }

        if (json_object_has_member(object, "username")) {
            test->username = g_strdup(json_object_get_string_member(object, "username"));
        } else {
            test->username = NULL;
        }

        test->host = g_strdup(json_object_get_string_member(object, "hostname"));
        test->port = json_object_get_int_member(object, "port");

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

        test->query_order = NULL;
        if (json_object_has_member(object, "query")) {
            JsonObject* query = json_object_get_object_member(object, "query");

            GList* members = json_object_get_members(query);
            gint memberslen = g_list_length(members);
            test->query = g_hash_table_new(g_str_hash, g_str_equal);
            for (gint i = 0; i < memberslen; i++) {
                gchar* current_member = g_strdup(g_list_nth_data(members, i));
                test->query_order = g_list_prepend(test->query_order, current_member);
                g_hash_table_insert(test->query,
                    current_member,
                    g_strdup(json_object_get_string_member(query, current_member)));
            }
            test->query_order = g_list_reverse(test->query_order);

            g_list_free(members);
        } else {
            test->query = NULL;
        }

        if (json_object_has_member(object, "fragment")) {
            test->fragment = g_strdup(json_object_get_string_member(object, "fragment"));
        } else {
            test->fragment = NULL;
        }

        if (json_object_has_member(object, "fragment-params")) {
            JsonObject* fragment = json_object_get_object_member(object, "fragment-params");

            GList* members = json_object_get_members(fragment);
            test->fragment_params = g_hash_table_new(g_str_hash, g_str_equal);
            GList* current = members;
            do {
                g_hash_table_insert(test->fragment_params, g_strdup(current->data),
                    g_strdup(json_object_get_string_member(fragment, current->data)));
            } while ((current = current->next));

            g_list_free(members);
        } else {
            test->fragment_params = NULL;
        }
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

GList* split_to_glist(gchar* string, gchar separator)
{
    string++;
    GList* new_list = NULL;

    gchar* current = string;
    while (current != NULL) {
        gchar* next = strchr(current, separator);
        gchar* nc;
        if (next) {
            next++;
            nc = g_strndup(current, next - current - 1);
        } else {
            nc = g_strndup(current, strlen(current));
        }

        new_list = g_list_prepend(new_list, nc);
        current = next;
    }

    new_list = g_list_reverse(new_list);
    return new_list;
}

gboolean compare_lists(GList* a, GList* b)
{
    gint alen = g_list_length(a);
    gint blen = g_list_length(b);

    for (int i = 0; i < alen && i < blen; i++) {
        if (!g_str_equal(g_list_nth_data(a, i), g_list_nth_data(b, i))) {
            return FALSE;
        }
    }

    return TRUE;
}

gboolean compare_list_and_str(GList* a, gchar* b, gchar separator)
{
    GList* rb = split_to_glist(b, separator);
    gboolean ret = compare_lists(a, rb);
    g_list_free_full(rb, g_free);
    return ret;
}

gchar* hash_table_to_str(GList* query_order, GHashTable* table)
{
    if (query_order == NULL || table == NULL || g_hash_table_size(table) == 0) {
        return g_strdup("");
    }

    GString* output = g_string_new(NULL);
    g_string_append_c(output, '?');

    gboolean first = TRUE;
    GList* current = query_order;
    do {
        if (!first) {
            g_string_append_c(output, '&');
        } else {
            first = FALSE;
        }

        g_string_append(output, current->data);
        gchar* next = g_hash_table_lookup(table, current->data);
        if (next != NULL) {
            g_string_append_c(output, '=');
            g_string_append(output, next);
        }
    } while ((current = current->next));

    return g_string_free(output, FALSE);
}

void assert_hash_tables_same(GHashTable* a, GHashTable* b)
{
    g_assert_cmpuint(g_hash_table_size(a), ==, g_hash_table_size(b));

    GHashTableIter iter;
    gpointer key = NULL, value = NULL;
    g_hash_table_iter_init(&iter, a);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        g_assert_cmpstr(value, ==, g_hash_table_lookup(b, key));
    }
}
