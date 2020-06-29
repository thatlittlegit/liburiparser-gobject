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

typedef struct {
    GtkWidget* scheme;
    GtkWidget* userinfo;
    GtkWidget* hostname;
    GtkWidget* port;
    GtkWidget* path;
    GtkWidget* query;
    GtkWidget* fragment;
} FieldSet;

static void gtk_entry_set_error(GtkEntry*, gboolean);

static GtkWidget* create_property_pair(gchar* key);
static void set_property_pair_value(GtkWidget*, gchar*);

static void new_window(GtkApplication*, gpointer);
static void update_fields(GtkWidget*, FieldSet*);

static void gtk_entry_set_error(GtkEntry* entry, gboolean val)
{
    GtkStyleContext* ctx = gtk_widget_get_style_context(GTK_WIDGET(entry));

    if (val) {
        gtk_style_context_add_class(ctx, "error");
    } else {
        gtk_style_context_remove_class(ctx, "error");
    }
}

static GtkWidget* create_property_pair(gchar* key)
{
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    g_object_set_data(G_OBJECT(box), "name", key);
    GtkWidget* left = gtk_label_new(key);
    GtkWidget* right = gtk_label_new("");

    gtk_widget_set_size_request(left, 100, 0);
    gtk_label_set_xalign(GTK_LABEL(left), 1);
    gtk_label_set_xalign(GTK_LABEL(right), 0);
    gtk_widget_set_sensitive(left, FALSE);

    if (gtk_get_locale_direction() == GTK_TEXT_DIR_RTL) {
        gtk_box_pack_start(GTK_BOX(box), right, TRUE, TRUE, 2);
        gtk_box_pack_end(GTK_BOX(box), left, FALSE, FALSE, 2);
    } else {
        gtk_box_pack_start(GTK_BOX(box), left, FALSE, FALSE, 2);
        gtk_box_pack_end(GTK_BOX(box), right, TRUE, TRUE, 2);
    }

    return box;
}

static void set_property_pair_value(GtkWidget* pair, gchar* value)
{
    GList* kids = gtk_container_get_children(GTK_CONTAINER(pair));
    gtk_label_set_text(GTK_LABEL(g_list_nth_data(kids, 1)), value);
    g_list_free(kids);
}

static void update_fields(GtkWidget* entry, FieldSet* fields)
{
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

    set_property_pair_value(fields->scheme, scheme);
    set_property_pair_value(fields->userinfo, userinfo);
    set_property_pair_value(fields->hostname, host);
    set_property_pair_value(fields->port, portstr);
    set_property_pair_value(fields->path, path);
    set_property_pair_value(fields->query, query);
    set_property_pair_value(fields->fragment, fragment);

    g_free(scheme);
    g_free(userinfo);
    g_free(host);
    g_free(path);
    g_free(query);
    g_free(fragment);
    g_object_unref(uri);
}

static void new_window(GtkApplication* app, gpointer data)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/tk/thatlittlegit/liburiparser-gobject-demo/wizard.ui");

    GtkListBox* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "fields"));
    FieldSet* set = g_new0(FieldSet, 1);
    gtk_list_box_insert(list, set->scheme = create_property_pair("Scheme"), -1);
    gtk_list_box_insert(list, set->userinfo = create_property_pair("Identification"), -1);
    gtk_list_box_insert(list, set->hostname = create_property_pair("Hostname"), -1);
    gtk_list_box_insert(list, set->port = create_property_pair("Port"), -1);
    gtk_list_box_insert(list, set->path = create_property_pair("Path"), -1);
    gtk_list_box_insert(list, set->query = create_property_pair("Query"), -1);
    gtk_list_box_insert(list, set->fragment = create_property_pair("Fragment"), -1);

    GtkWidget* uri = GTK_WIDGET(gtk_builder_get_object(builder, "uri-field"));
    g_signal_connect(uri, "changed", G_CALLBACK(update_fields), set);

    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_application_add_window(app, GTK_WINDOW(window));
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), set);
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_object_unref), builder);
    gtk_widget_show_all(window);
    gtk_window_set_title(GTK_WINDOW(window), "URI Wizard");
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new("tk.thatlittlegit.liburiparser-gobject-demo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(new_window), NULL);
    gint status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
