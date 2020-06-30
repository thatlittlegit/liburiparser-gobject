/* propertypair.c
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
#include "propertypair.h"

enum {
    PROP_ID = 1,
    PROP_TITLE,
    PROP_VALUE,
    _N_PROPERTIES_
};
static GParamSpec* properties[_N_PROPERTIES_] = { NULL };

enum {
    SIGNAL_CHANGED,
    _N_SIGNALS_
};
static guint signals[_N_SIGNALS_] = { 0 };

struct _WizPropertyPair {
    GtkBox parent_instance;

    // <private>
    gboolean suppress_events;
    gchar* id;
    GtkWidget* label;
    GtkWidget* entry;
};

G_DEFINE_TYPE(WizPropertyPair, wiz_property_pair, GTK_TYPE_BOX);

static void wiz_property_pair_emit_changed(GtkEntry* buffer, WizPropertyPair* self)
{
    if (!self->suppress_events) {
        g_signal_emit(self, signals[SIGNAL_CHANGED], 0);
    }
}

static void wiz_property_pair_init(WizPropertyPair* self)
{
    gtk_box_set_spacing(&self->parent_instance, 16);
    self->label = gtk_label_new("");
    self->entry = gtk_entry_new();

    g_signal_connect(self->entry, "changed", G_CALLBACK(wiz_property_pair_emit_changed), self);

    gtk_widget_set_size_request(self->label, 100, 0);
    gtk_label_set_xalign(GTK_LABEL(self->label), 1);
    gtk_widget_set_sensitive(self->label, FALSE);
    gtk_entry_set_has_frame(GTK_ENTRY(self->entry), FALSE);

    if (gtk_get_locale_direction() == GTK_TEXT_DIR_RTL) {
        gtk_box_pack_start(&self->parent_instance, self->entry, TRUE, TRUE, 2);
        gtk_box_pack_end(&self->parent_instance, self->label, FALSE, FALSE, 2);
    } else {
        gtk_box_pack_start(&self->parent_instance, self->label, FALSE, FALSE, 2);
        gtk_box_pack_end(&self->parent_instance, self->entry, TRUE, TRUE, 2);
    }
}

static void wiz_property_pair_set_property(GObject* object, guint property_id, const GValue* _value, GParamSpec* pspec)
{
    WizPropertyPair* private = WIZ_PROPERTY_PAIR(object);
    const gchar* value = g_value_get_string(_value);

    if (_value == NULL || value == NULL) {
        value = "";
    }

    switch (property_id) {
    case PROP_ID:
        g_free(private->id);
    private
        ->id = g_strdup(value);
        break;
    case PROP_TITLE:
        gtk_label_set_text(GTK_LABEL(private->label), value);
        break;
    case PROP_VALUE:
    private
        ->suppress_events = TRUE;
        gtk_entry_set_text(GTK_ENTRY(private->entry), value);
    private
        ->suppress_events = FALSE;
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void wiz_property_pair_get_property(GObject* object, guint property_id, GValue* _value, GParamSpec* pspec)
{
    WizPropertyPair* private = WIZ_PROPERTY_PAIR(object);

    switch (property_id) {
    case PROP_ID:
        g_value_set_string(_value, private->id);
        break;
    case PROP_TITLE:
        g_value_set_string(_value, gtk_label_get_text(GTK_LABEL(private->label)));
        break;
    case PROP_VALUE:
        g_value_set_string(_value, gtk_entry_get_text(GTK_ENTRY(private->entry)));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void wiz_property_pair_class_init(WizPropertyPairClass* klass)
{
    G_OBJECT_CLASS(klass)->set_property = wiz_property_pair_set_property;
    G_OBJECT_CLASS(klass)->get_property = wiz_property_pair_get_property;

    properties[PROP_ID] = g_param_spec_string("id", "Identifier", "An arbitrary, optional string identifier for this object.", NULL, G_PARAM_READWRITE);
    properties[PROP_TITLE] = g_param_spec_string("title", "Title", "The current title of the property pair.", NULL, G_PARAM_READWRITE);
    properties[PROP_VALUE] = g_param_spec_string("value", "Pair Value", "The current value of the property pair.", NULL, G_PARAM_READWRITE);
    g_object_class_install_properties(G_OBJECT_CLASS(klass), _N_PROPERTIES_, properties);

    signals[SIGNAL_CHANGED] = g_signal_newv(
        "changed",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        NULL,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0,
        NULL);
}

WizPropertyPair* wiz_property_pair_new(const gchar* id, const gchar* label)
{
    return g_object_new(WIZ_TYPE_PROPERTY_PAIR, "title", label, "id", id, NULL);
}

const gchar* wiz_property_pair_get_id(WizPropertyPair* self)
{
    GValue value = G_VALUE_INIT;
    g_object_get_property(G_OBJECT(self), "id", &value);
    return g_value_get_string(&value);
}

void wiz_property_pair_set_id(WizPropertyPair* self, const gchar* _value)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_value_set_string(&value, _value);
    g_object_set_property(G_OBJECT(self), "id", &value);
    g_value_unset(&value);
}

const gchar* wiz_property_pair_get_value(WizPropertyPair* self)
{
    GValue value = G_VALUE_INIT;
    g_object_get_property(G_OBJECT(self), "value", &value);
    return g_value_get_string(&value);
}

void wiz_property_pair_set_value(WizPropertyPair* self, const gchar* _value)
{
    GValue value = G_VALUE_INIT;
    g_value_init(&value, G_TYPE_STRING);
    g_value_set_string(&value, _value);
    g_object_set_property(G_OBJECT(self), "value", &value);
    g_value_unset(&value);
}
