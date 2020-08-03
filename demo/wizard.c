/* wizard.c
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
#include <gtk/gtk.h>
#include <liburiparser-gobject.h>

#include "propertypair.h"

typedef struct {
    GtkWidget* scheme;
    GtkWidget* userinfo;
    GtkWidget* hostname;
    GtkWidget* port;
    GtkWidget* path;
    GtkWidget* query;
    GtkWidget* fragment;
} FieldSet;

typedef struct {
    GtkWidget* uri;
    gchar* property;
} ModificationTuple;

static void gtk_entry_set_error(GtkEntry*, gboolean);
static void configure_list_rows(GtkListBox* list, FieldSet* set, GtkWidget* uri);

static void new_window(GtkApplication*, gpointer);
static void update_fields(GtkWidget*, FieldSet*);
static void update_uri(WizPropertyPair*, GtkEntry*);

static void gtk_entry_set_error(GtkEntry* entry, gboolean val)
{
    GtkStyleContext* ctx = gtk_widget_get_style_context(GTK_WIDGET(entry));

    if (val) {
        gtk_style_context_add_class(ctx, "error");
    } else {
        gtk_style_context_remove_class(ctx, "error");
    }
}

static void update_fields(GtkWidget* entry, FieldSet* fields)
{
    if (GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(entry), "suppress"))) {
        return;
    }

    const gchar* new_uri = gtk_entry_get_text(GTK_ENTRY(entry));

    GError* err = NULL;
    UpgUri* uri = upg_uri_new(new_uri, &err);

    if (err) {
        gtk_entry_set_error(GTK_ENTRY(entry), TRUE);
        return;
    }
    gtk_entry_set_error(GTK_ENTRY(entry), FALSE);

    gchar* scheme = upg_uri_get_scheme(uri);
    gchar* userinfo = upg_uri_get_userinfo(uri);
    gchar* host = upg_uri_get_host(uri);
    guint16 port = upg_uri_get_port(uri);
    gchar portstr[5];
    if (port == 0) {
        portstr[0] = 0;
    } else {
        g_ascii_dtostr(portstr, 5, (int)port);
    }
    gchar* path = upg_uri_get_path_str(uri);
    gchar* query = upg_uri_get_query_str(uri);
    gchar* fragment = upg_uri_get_fragment(uri);

    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->scheme), scheme);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->userinfo), userinfo);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->hostname), host);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->port), portstr);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->path), path);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->query), query);
    wiz_property_pair_set_value(WIZ_PROPERTY_PAIR(fields->fragment), fragment);

    g_free(scheme);
    g_free(userinfo);
    g_free(host);
    g_free(path);
    g_free(query);
    g_free(fragment);
    g_object_unref(uri);
}

static void configure_list_rows(GtkListBox* list, FieldSet* set, GtkWidget* uri)
{
    gtk_list_box_insert(list, set->scheme = GTK_WIDGET(wiz_property_pair_new("scheme", "Scheme")), -1);
    gtk_list_box_insert(list, set->userinfo = GTK_WIDGET(wiz_property_pair_new("userinfo", "Identification")), -1);
    gtk_list_box_insert(list, set->hostname = GTK_WIDGET(wiz_property_pair_new("host", "Hostname")), -1);
    gtk_list_box_insert(list, set->port = GTK_WIDGET(wiz_property_pair_new("port", "Port")), -1);
    gtk_list_box_insert(list, set->path = GTK_WIDGET(wiz_property_pair_new("path-str", "Path")), -1);
    gtk_list_box_insert(list, set->query = GTK_WIDGET(wiz_property_pair_new("query-str", "Query")), -1);
    gtk_list_box_insert(list, set->fragment = GTK_WIDGET(wiz_property_pair_new("fragment", "Fragment")), -1);

    g_signal_connect(set->scheme, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->userinfo, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->hostname, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->port, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->path, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->query, "changed", G_CALLBACK(update_uri), uri);
    g_signal_connect(set->fragment, "changed", G_CALLBACK(update_uri), uri);
}

static void new_window(GtkApplication* app, gpointer data)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/tk/thatlittlegit/liburiparser-gobject-demo/wizard.ui");
    FieldSet* set = g_new0(FieldSet, 1);

    GtkWidget* uri = GTK_WIDGET(gtk_builder_get_object(builder, "uri-field"));
    g_signal_connect(uri, "changed", G_CALLBACK(update_fields), set);

    GtkListBox* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "fields"));
    configure_list_rows(list, set, uri);

    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_application_add_window(app, GTK_WINDOW(window));
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), set);
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_object_unref), builder);
    gtk_widget_show_all(window);
    gtk_window_set_title(GTK_WINDOW(window), "URI Wizard");
    gtk_window_present(GTK_WINDOW(window));
}

static void update_uri(WizPropertyPair* pair, GtkEntry* uri_widget)
{
    GError* err = NULL;
    UpgUri* uri = upg_uri_new(gtk_entry_get_text(uri_widget), &err);

    gtk_entry_set_error(uri_widget, err != NULL);
    if (err != NULL) {
        return;
    }

    const gchar* id = wiz_property_pair_get_id(pair);
    const gchar* str = wiz_property_pair_get_value(pair);
    GValue value = G_VALUE_INIT;
    if (g_str_equal(id, "port")) {
        g_value_init(&value, G_TYPE_UINT);
        g_value_set_uint(&value, CLAMP(g_ascii_strtoull(str, NULL, 10), 0, 65535));
    } else {

        g_value_init(&value, G_TYPE_STRING);
        g_value_set_string(&value, str);
    }
    g_object_set_property(G_OBJECT(uri), wiz_property_pair_get_id(pair), &value);
    g_value_unset(&value);

    g_object_set_data(G_OBJECT(uri_widget), "suppress", GUINT_TO_POINTER(TRUE));

    gchar* text_uri = upg_uri_to_string_ign(uri, TRUE);
    gtk_entry_set_text(uri_widget, text_uri);
    g_free(text_uri);

    g_object_set_data(G_OBJECT(uri_widget), "suppress", GUINT_TO_POINTER(FALSE));
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new("tk.thatlittlegit.liburiparser-gobject-demo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(new_window), NULL);
    gint status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
