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

UpgUri* upg_uri_new(gchar* uri, GError** error);

G_END_DECLS

#endif
