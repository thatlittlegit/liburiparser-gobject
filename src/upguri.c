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
static UriTextRangeA uritextrange_from_str(gchar* str);

enum {
    PROP_SCHEME = 1,
    PROP_HOST,
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
    params[PROP_SCHEME] = g_param_spec_string("scheme",
        "Scheme",
        "The scheme of the UpgUri object.",
        NULL,
        G_PARAM_READWRITE);
    params[PROP_HOST] = g_param_spec_string("host",
        "Hostname",
        "The hostname of this UpgUri object."
        "If hostdata is set, then this will be set to the stringified version "
        "of that. If this is set, hostdata is NULL.",
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

    UpgUri* uri = G_TYPE_CHECK_INSTANCE_CAST(self, UPG_TYPE_URI, UpgUri);
    uriFreeUriMembersA(&uri->internal_uri);
    uri->initialized = FALSE;
}

static void upg_uri_finalize(GObject* self)
{
    G_OBJECT_CLASS(upg_uri_parent_class)->finalize(self);
}

static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec)
{
    UpgUri* self = G_TYPE_CHECK_INSTANCE_CAST(obj, UPG_TYPE_URI, UpgUri);

    switch (id) {
    case PROP_SCHEME:
        upg_uri_set_scheme(self, g_value_get_string(value));
        break;
    case PROP_HOST:
        // FIXME const props
        upg_uri_set_host(self, (gchar*)g_value_get_string(value));
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec)
{
    UpgUri* self = G_TYPE_CHECK_INSTANCE_CAST(obj, UPG_TYPE_URI, UpgUri);

    switch (id) {
    case PROP_SCHEME:
        g_value_set_string(value, upg_uri_get_scheme(self));
        break;
    case PROP_HOST:
        g_value_set_string(value, upg_uri_get_host(self));
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

static UriTextRangeA uritextrange_from_str(gchar* str)
{
    g_assert_nonnull(str);
    int len = strlen(str);

    return (UriTextRangeA) { str, str + len };
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
 * Sets the scheme of the URI.
 *
 * Returns: Whether or not the setting was successful.
 */
gboolean upg_uri_set_scheme(UpgUri* uri, const gchar* nscheme)
{
    g_assert(uri->initialized);

    if (nscheme == NULL) {
        uri->internal_uri.scheme = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

    uri->internal_uri.scheme = uritextrange_from_str(g_strdup(nscheme));
    return TRUE;
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
    if (uri->initialized) {
        return str_from_uritextrange(uri->internal_uri.scheme);
    }

    return NULL;
}

/**
 * upg_uri_get_host:
 * @self: The URI to get the hostname of.
 *
 * Gets the current hostname of the given #UpgUri. If @self doesn't have a
 * hostname, but rather an IPvX address, then we return it as a string. See
 * upg_uri_get_host_data() for getting that in a more numerical form.
 *
 * If there is no hostname, or the hostname is empty (""), then returns #NULL.
 *
 * Returns: (transfer full) (nullable): A stringified form of the hostname.
 */
gchar* upg_uri_get_host(UpgUri* uri)
{
    g_assert(uri->initialized);

    if (uri->internal_uri.hostText.first == NULL
        || uri->internal_uri.hostText.afterLast - uri->internal_uri.hostText.first == 0) {
        return NULL;
    }

    return str_from_uritextrange(uri->internal_uri.hostText);
}

/**
 * upg_uri_get_host_data:
 * @self: The URI to get the hostname of.
 * @protocol: (out): The protocol used by the name. Set to zero if no valid
 *                   protocol.
 *
 * Gets the current host information as an array, with a value indicating the
 * protocol put into @uri.
 *
 * |[<!-- language="C" -->
 * guint8 protocol;
 * guint8* data = upg_uri_get_host_data (uri, &protocol);
 *
 * if (data == NULL)
 *   // hostname or IPvFuture
 * else if (protocol == 4)
 *   // IPv4
 * else if (protocol == 6)
 *   // IPv6
 * else
 *   g_assert_not_reached ();
 * ]|
 *
 * Returns: (transfer none): The host data as an array. Do not modify it! It
 * points directly to an internal data structure's data. (It is also not
 * guaranteed to work like this forever; it won't be a breaking change if this
 * doesn't anymore.)
 */
const guint8* upg_uri_get_host_data(UpgUri* uri, guint8* protocol)
{
    g_assert(uri->initialized);

    UriHostDataA* data = &uri->internal_uri.hostData;
    if (data->ip4 != NULL) {
        *protocol = 4;
        return data->ip4->data;
    }

    if (data->ip6 != NULL) {
        *protocol = 6;
        return data->ip6->data;
    }

    return NULL;
}

/**
 * upg_uri_set_host:
 * @self: The URI to set the host of.
 * @host: (transfer none): The host to set the URI to.
 *
 * Sets the URI of @self to @nhost. The structured data is always reset after
 * this, but in future it might be set properly if @nhost is a valid IPvX
 * address.
 *
 * Returns: Whether or not the setting succeeded.
 */
gboolean upg_uri_set_host(UpgUri* uri, gchar* host)
{
    g_assert(uri->initialized);

    // FIXME we should probably parse the incoming host to check if it's IPvX
    uri->internal_uri.hostData = (UriHostDataA) { NULL, NULL, { NULL, NULL } };
    uri->internal_uri.hostText = uritextrange_from_str(host);

    return TRUE;
}
