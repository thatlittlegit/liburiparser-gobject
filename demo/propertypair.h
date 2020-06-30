/* propertypair.h
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

#ifndef PROPERTYPAIR_H
#define PROPERTYPAIR_H
#include <glib-2.0/glib-object.h>
#include <gtk-3.0/gtk/gtk.h>
G_BEGIN_DECLS

#define WIZ_TYPE_PROPERTY_PAIR wiz_property_pair_get_type()
G_DECLARE_FINAL_TYPE(WizPropertyPair, wiz_property_pair, WIZ, PROPERTY_PAIR, GtkBox)

WizPropertyPair* wiz_property_pair_new(const gchar* id, const gchar* label);

const gchar* wiz_property_pair_get_id(WizPropertyPair* pair);
void wiz_property_pair_set_id(WizPropertyPair* pair, const gchar* id);

const gchar* wiz_property_pair_get_value(WizPropertyPair* pair);
void wiz_property_pair_set_value(WizPropertyPair* pair, const gchar* value);

G_END_DECLS
#endif
