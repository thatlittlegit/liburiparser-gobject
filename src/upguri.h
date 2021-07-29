/* upguri.h
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
#ifndef UPGURI_H
#define UPGURI_H

#include <glib-2.0/glib.h>
#include <glib-object.h>

#if !defined(__LIBURIPARSER_GOBJECT_INSIDE__) && !defined(LIBURIPARSER_GOBJECT_COMPILATION)
#error "Only <liburiparser-gobject.h> can be included directly."
#endif

G_BEGIN_DECLS

#define UPG_TYPE_URI upg_uri_get_type()
G_DECLARE_DERIVABLE_TYPE(UpgUri, upg_uri, UPG, URI, GObject)

struct _UpgUriClass {
    GObjectClass* parent_class;
    gpointer padding[16];
};

typedef enum {
    UPG_HIERARCHY_LAX = 0,
    UPG_HIERARCHY_STRICT = 1,
    UPG_HIERARCHY_NOTSELF = 2,
} UpgHierarchyFlags;

GType upg_hierarchy_flags_get_type(void);
#define UPG_TYPE_HIERARCHY_FLAGS upg_hierarchy_flags_get_type()

UpgUri* upg_uri_new(const gchar* uri, GError** error);
gboolean upg_uri_configure_from_string(UpgUri* self, const gchar* nuri, GError** error);
gchar* upg_uri_to_string(UpgUri* self);
void upg_uri_set_scheme(UpgUri* self, const gchar* nscheme);
gchar* upg_uri_get_scheme(UpgUri* self);
gchar* upg_uri_get_host(UpgUri* self);
guint8* upg_uri_get_host_data(UpgUri* self, guint8* protocol);
void upg_uri_set_host(UpgUri* self, const gchar* host);
GList* upg_uri_get_path(UpgUri* self);
gchar* upg_uri_get_path_str(UpgUri* self);
void upg_uri_set_path(UpgUri* self, GList* list);
void upg_uri_set_path_str(UpgUri* self, const char* path);
GHashTable* upg_uri_get_query(UpgUri* self);
gchar* upg_uri_get_query_str(UpgUri* self);
void upg_uri_set_query(UpgUri* self, GHashTable* table);
void upg_uri_set_query_str(UpgUri* self, const gchar* query);
gchar* upg_uri_get_fragment(UpgUri* self);
GHashTable* upg_uri_get_fragment_params(UpgUri* self);
void upg_uri_set_fragment(UpgUri* self, const gchar* fragment);
void upg_uri_set_fragment_params(UpgUri* self, GHashTable* table);
guint16 upg_uri_get_port(UpgUri* self);
void upg_uri_set_port(UpgUri* self, guint16 port);
gchar* upg_uri_get_userinfo(UpgUri* self);
gchar* upg_uri_get_username(UpgUri* self);
void upg_uri_set_userinfo(UpgUri* self, const gchar* userinfo);
UpgUri* upg_uri_apply_reference(UpgUri* self, const gchar* reference, GError** error);
gchar* upg_uri_subtract_to_reference(UpgUri* self, UpgUri* subtrahend, GError** error);
gboolean upg_uri_is_below(UpgUri* self, UpgUri* other, UpgHierarchyFlags flags);

guint upg_uri_hash(gconstpointer self);
gboolean upg_uri_equal(gconstpointer a, gconstpointer b);
gboolean upg_uri_nearly_equal(gconstpointer a, gconstpointer b);

UpgUri* upg_uri_copy(UpgUri* self);
gpointer upg_uri_ref(gpointer self);
void upg_uri_unref(gpointer self);
G_END_DECLS

#endif
