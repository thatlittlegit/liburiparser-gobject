/* wizard.c
 *
 * Copyright 2020-2021 thatlittlegit <personal@thatlittlegit.tk>
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
#include "config.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <liburiparser-gobject.h>

#include "propertypair.h"

static void gtk_entry_set_error(GtkEntry*, gboolean);
static void configure_list_rows(GtkListBox* list, UpgUri* uri);

static void new_window(GtkApplication*, gpointer);
static void update_uri_field(UpgUri* uri, GParamSpec* spec, GtkEntry* entry);
static void update_uri_data(GtkEntry* entry, UpgUri* uri);

static void gtk_entry_set_error(GtkEntry* entry, gboolean val)
{
    GtkStyleContext* ctx = gtk_widget_get_style_context(GTK_WIDGET(entry));

    if (val) {
        gtk_style_context_add_class(ctx, "error");
    } else {
        gtk_style_context_remove_class(ctx, "error");
    }
}

static void configure_list_rows(GtkListBox* list, UpgUri* uri)
{
    GtkWidget* scheme = wiz_property_pair_new("scheme", _("Scheme"));
    g_object_bind_property(uri, "scheme", scheme, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, scheme, -1);

    GtkWidget* userinfo = wiz_property_pair_new("userinfo", _("Identification"));
    g_object_bind_property(uri, "userinfo", userinfo, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, userinfo, -1);

    GtkWidget* hostname = wiz_property_pair_new("host", _("Hostname"));
    g_object_bind_property(uri, "host", hostname, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, hostname, -1);

    GtkWidget* port = wiz_property_pair_new("port", _("Port"));
    //    g_object_bind_property_full (uri, "host", scheme, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, port, -1);

    GtkWidget* path = wiz_property_pair_new("path-str", _("Path"));
    g_object_bind_property(uri, "path-str", path, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, path, -1);

    GtkWidget* fragment = wiz_property_pair_new("fragment", _("Fragment"));
    g_object_bind_property(uri, "fragment", fragment, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, fragment, -1);

    GtkWidget* query = wiz_property_pair_new("query", _("Query"));
    g_object_bind_property(uri, "query-str", query, "value", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
    gtk_list_box_insert(list, query, -1);
}

static void new_window(GtkApplication* app, gpointer data)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/tk/thatlittlegit/liburiparser-gobject-demo/wizard.ui");

    UpgUri* uri = upg_uri_new(NULL, NULL);

    GtkWidget* uriField = GTK_WIDGET(gtk_builder_get_object(builder, "uri-field"));
    g_signal_connect(uri, "notify", G_CALLBACK(update_uri_field), uriField);
    g_signal_connect(uriField, "changed", G_CALLBACK(update_uri_data), uri);

    GtkListBox* list = GTK_LIST_BOX(gtk_builder_get_object(builder, "fields"));
    configure_list_rows(list, uri);

    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    gtk_application_add_window(app, GTK_WINDOW(window));
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_object_unref), builder);
    gtk_widget_show_all(window);
    gtk_window_set_title(GTK_WINDOW(window), _("URI Wizard"));
    gtk_window_present(GTK_WINDOW(window));
}

static void update_uri_field(UpgUri* uri, GParamSpec* spec, GtkEntry* entry)
{
    g_return_if_fail(UPG_IS_URI(uri));
    (void)spec;
    g_return_if_fail(GTK_IS_ENTRY(entry));

    char* text = upg_uri_to_string(uri);
    gtk_entry_set_text(entry, text ? text : "");
}

static void update_uri_data(GtkEntry* entry, UpgUri* uri)
{
    g_return_if_fail(GTK_IS_ENTRY(entry));
    g_return_if_fail(UPG_IS_URI(uri));

    const char* text = gtk_entry_get_text(entry);
    gtk_entry_set_error(entry, !upg_uri_configure_from_string(uri, text, NULL));
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, DATADIR "/locale");
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    g_log_set_always_fatal(G_LOG_LEVEL_WARNING);
    GtkApplication* app = gtk_application_new("tk.thatlittlegit.liburiparser-gobject-demo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(new_window), NULL);
    gint status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
