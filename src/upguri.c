/* liburiparser-gobject.c
 *
 * Copyright 2020 thatlittlegit
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "liburiparser-gobject.h"
#include <uriparser/Uri.h>

struct _UpgUri;
static void upg_uri_class_init(UpgUriClass*);
static void upg_uri_init(UpgUri*);
static void upg_uri_dispose(GObject*);
static void upg_uri_finalize(GObject*);
static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec);
static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec);
static gchar* str_from_uritextrange(UriTextRangeA range);

enum {
    PROP_URI = 1,
    PROP_SCHEME,
    _N_PROPERTIES_
};

static GParamSpec* params[_N_PROPERTIES_] = { NULL };

/**
 * SECTION:upguri
 * @short_description: The URI Class
 * @include: liburiparser-gobject.h
 * @title: UpgUri
 *
 * #UpgUri is the main class that you have liburiparser-gobject for. It acts as
 * a wrapper over
 * [UriUriA](https://uriparser.github.io/doc/api/latest/structUriUriStructA.html)
 * that introduces proper memory management and good integration with GLib and
 * GObject. There is no way to get the internal UriUriA instance.
 *
 * UpgUri does not currently support many things, but it will probably never
 * support wide-char strings (UriUriW, etc.) You can use #g_utf16_to_utf8 to try
 * and convert a valid UTF-16 string to something UpgUri can handle.
 */
struct _UpgUri {
    GObject parent_instance;

    // private
    gboolean initialized;
    UriUriA internal_uri;
};

G_DEFINE_TYPE(UpgUri, upg_uri, G_TYPE_OBJECT);

static void upg_uri_class_init(UpgUriClass* klass)
{
    GObjectClass* glass = G_OBJECT_CLASS(klass);

    glass->set_property = upg_uri_set_property;
    glass->get_property = upg_uri_get_property;
    params[PROP_URI] = g_param_spec_string("uri",
        "URI",
        "The URI that the other properties grant information about.",
        NULL,
        G_PARAM_READWRITE);
    params[PROP_SCHEME] = g_param_spec_string("scheme",
        "Scheme",
        "The scheme of the set %UpgUri:uri.",
        NULL,
        G_PARAM_READWRITE);
    g_object_class_install_properties(glass, _N_PROPERTIES_, params);

    glass->dispose = upg_uri_dispose;
    glass->finalize = upg_uri_finalize;
}

static void upg_uri_init(UpgUri* self)
{
    self->initialized = FALSE;
}

static void upg_uri_dispose(GObject* self)
{
    G_OBJECT_CLASS(upg_uri_parent_class)->dispose(self);
}

static void upg_uri_finalize(GObject* self)
{
    G_OBJECT_CLASS(upg_uri_parent_class)->finalize(self);
}

static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec)
{
    UpgUri* self = G_TYPE_CHECK_INSTANCE_CAST(obj, UPG_TYPE_URI, UpgUri);

    switch (id) {
    case PROP_URI:
        upg_uri_set_uri(self, g_value_get_string(value));
        break;
    case PROP_SCHEME:
        upg_uri_set_scheme(self, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec)
{
    UpgUri* self = G_TYPE_CHECK_INSTANCE_CAST(obj, UPG_TYPE_URI, UpgUri);

    switch (id) {
    case PROP_URI:
        g_value_set_string(value, upg_uri_get_uri(self));
        break;
    case PROP_SCHEME:
        g_value_set_string(value, upg_uri_get_scheme(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static char* str_from_uritextrange(UriTextRangeA range)
{
    g_assert_nonnull(range.first);
    g_assert_nonnull(range.afterLast);

    ssize_t ptr_len = range.afterLast - range.first;
    g_assert(g_utf8_validate(range.first, ptr_len, NULL));
    return g_strndup(range.first, ptr_len);
}

/**
 * upg_uri_new:
 * @uri: The input URI to be parsed.
 * @error: A #GError.
 *
 * Creates a new #UpgUri by parsing @uri. Note that @uri must be a valid URI,
 * otherwise it will fail.
 *
 * Returns: (transfer full) (nullable): a new #UpgUri if the parsing was
 *          successful, or #NULL.
 */
UpgUri* upg_uri_new(gchar* uri, GError** error)
{
    UpgUri* ret = g_object_new(UPG_TYPE_URI, NULL);
    upg_uri_set_uri(ret, uri);

    if (ret->initialized) {
        return ret;
    }

    g_object_unref(ret);
    return NULL;
}

/**
 * upg_uri_set_uri:
 * @self: The URI object to reset.
 * @nuri: The new textual URI to be parsed.
 *
 * Sets the current URI for the given #UpgUri.
 *
 * Returns: Whether or not the operation succeeded.
 */
gboolean upg_uri_set_uri(UpgUri* self, const gchar* nuri)
{
    if (self->initialized) {
        uriFreeUriMembersA(&self->internal_uri);
    }

    if (nuri == NULL) {
        g_info("NULL passed to upg_uri_set_uri, ignoring (this might not work in future!)");
        return TRUE;
    }

    int ret = 0;
    const gchar* errorPos = NULL;
    if ((ret = uriParseSingleUriA(&self->internal_uri, nuri, &errorPos)) != URI_SUCCESS) {
        // FIXME use a GError instead of logging
        g_warning("Failed to parse URI '%s' (code %d, ep=\"%s\")", nuri, ret, errorPos);

        self->initialized = FALSE;
        return FALSE;
    } else {
        return self->initialized = TRUE;
    }
}

/**
 * upg_uri_get_uri:
 * @self: The URI to get the textual URI of.
 *
 * Converts the in-memory URI object into a string, in a process called
 * 'recomposition'.
 *
 * Returns: (transfer full): The textual representation of the URI.
 */
gchar* upg_uri_get_uri(UpgUri* self)
{
    g_assert(self->initialized);

    int len, ret;
    if ((ret = uriToStringCharsRequiredA(&self->internal_uri, &len)) != URI_SUCCESS) {
        // FIXME use a GError instead of logging
        g_warning("Failed to determine length of URI (code %d)", ret);
        return NULL;
    }
    len++;

    gchar* out = g_malloc0_n(len, sizeof(char));
    int written;
    if ((ret = uriToStringA(out, &self->internal_uri, len, &written)) != URI_SUCCESS) {
        // FIXME use a GError instead of logging
        g_warning("Failed to convert URI to string (code %d)", ret);
        return NULL;
    }

    g_assert(g_utf8_validate_len(out, written - 1, NULL));
    return out;
}

/**
 * upg_uri_set_scheme:
 * @self: The URI to set the scheme of.
 * @nscheme: (transfer none): The new scheme.
 *
 * > This API is currently unimplemented, do not use.
 *
 * Sets the scheme of the URI.
 *
 * Returns: Whether or not the setting was successful.
 */
gboolean upg_uri_set_scheme(UpgUri* uri, const gchar* nscheme)
{
    g_assert(uri->initialized);
    g_assert_not_reached();
    (void)uri;
    (void)nscheme;
}

/**
 * upg_uri_get_scheme:
 * @self: The URI to get the scheme of.

 * Gets the current URI scheme for the given #UpgUri object. If the #UpgUri
 * hasn't been initialized, returns #NULL.
 *
 * Returns: The scheme of the URI.
 */
gchar* upg_uri_get_scheme(UpgUri* uri)
{
    g_assert(uri->initialized);
    return str_from_uritextrange(uri->internal_uri.scheme);
}
