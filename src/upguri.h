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

#if !defined(__LIBURIPARSER_GOBJECT_INSIDE__) && !defined(LIBURIPARSER_GOBJECT_COMPILATION)
#error "Only <liburiparser-gobject.h> can be included directly."
#endif

G_BEGIN_DECLS

#define UPG_TYPE_URI upg_uri_get_type()
G_DECLARE_FINAL_TYPE(UpgUri, upg_uri, UPG, FILE, GObject)

UpgUri* upg_uri_new(const gchar* uri, GError** error);
gboolean upg_uri_configure_from_string(UpgUri* self, const gchar* nuri, GError** error);
gchar* upg_uri_to_string(UpgUri* self);
gboolean upg_uri_set_scheme(UpgUri* self, const gchar* nscheme);
gchar* upg_uri_get_scheme(UpgUri* self);
gchar* upg_uri_get_host(UpgUri* self);
guint8* upg_uri_get_host_data(UpgUri* self, guint8* protocol);
gboolean upg_uri_set_host(UpgUri* self, const gchar* host);
GList* upg_uri_get_path(UpgUri* self);
gchar* upg_uri_get_path_str(UpgUri* self);
gboolean upg_uri_set_path(UpgUri* self, GList* list);
GHashTable* upg_uri_get_query(UpgUri* self);
gchar* upg_uri_get_query_str(UpgUri* self);
gboolean upg_uri_set_query(UpgUri* self, GHashTable* table);
gboolean upg_uri_set_query_str(UpgUri* self, const gchar* query);
gchar* upg_uri_get_fragment(UpgUri* self);
GHashTable* upg_uri_get_fragment_params(UpgUri* self);
gboolean upg_uri_set_fragment(UpgUri* self, const gchar* fragment);
gboolean upg_uri_set_fragment_params(UpgUri* self, GHashTable* table);
guint16 upg_uri_get_port(UpgUri* self);
gboolean upg_uri_set_port(UpgUri* self, guint16 port);
gchar* upg_uri_get_userinfo(UpgUri* self);
gchar* upg_uri_get_username(UpgUri* self);
gboolean upg_uri_set_userinfo(UpgUri* self, const gchar* userinfo);
G_END_DECLS

#endif
