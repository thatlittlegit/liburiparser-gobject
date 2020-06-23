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
#include "upgerror.h"
#include <uriparser/Uri.h>

struct _UpgUri;
static void upg_uri_class_init(UpgUriClass*);
static void upg_uri_init(UpgUri*);
static void upg_uri_dispose(GObject*);
static void upg_uri_finalize(GObject*);
static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec);
static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec);
static gchar* str_from_uritextrange(UriTextRangeA range);
static UriTextRangeA uritextrange_from_str(const gchar* str);
static void upg_free_upsl_(UriPathSegmentA** segment, UriPathSegmentA** tail);

#define upg_free_utr(p) g_free((gchar*)p.first)
#define upg_free_upsl(u) upg_free_upsl_(&u.pathHead, &u.pathTail)

enum {
    PROP_SCHEME = 1,
    PROP_HOST,
    PROP_PATH,
    PROP_PATHSTR,
    PROP_QUERY,
    PROP_QUERYSTR,
    PROP_FRAGMENT,
    PROP_FRAGMENTPARAMS,
    _N_PROPERTIES_
};

enum {
    MASK_SCHEME = 1 << 1,
    MASK_HOST = 1 << 2,
    MASK_PATH = 1 << 3,
    MASK_QUERY = 1 << 4,
    MASK_FRAGMENT = 1 << 5,
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
    gint32 modified;
    UriPathSegmentA* original_segment;
    UriTextRangeA original_host;
    UriHostDataA original_hostdata;
    UriTextRangeA original_query;
    UriTextRangeA original_fragment;
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
    /**
     * UpgUri:path: (type GList(gchar*))
     *
     * The path segments of this object.
     */
    params[PROP_PATH] = g_param_spec_pointer("path",
        "Path",
        "The path segments of this object.",
        G_PARAM_READWRITE);
    params[PROP_PATHSTR] = g_param_spec_string("path-str",
        "Path string",
        "The path segments of this object, represented as a string. "
        "May be settable in future.",
        NULL,
        G_PARAM_READABLE);
    /**
     * UpgUri:query: (type GHashTable)
     *
     * The query parameters of this URI.
     */
    params[PROP_QUERY] = g_param_spec_pointer("query",
        "Query",
        "The query parameters of this URI.",
        G_PARAM_READWRITE);
    params[PROP_QUERYSTR] = g_param_spec_string("query-str",
        "Query string",
        "The query parameters of the URI, as a string",
        NULL,
        G_PARAM_READWRITE);
    params[PROP_FRAGMENT] = g_param_spec_string("fragment",
        "Fragment",
        "The fragment of the URI.",
        NULL,
        G_PARAM_READWRITE);
    /**
     * UpgUri:fragment_params: (type GHashTable)
     *
     * The fragment parameters of the URI.
     */
    params[PROP_FRAGMENTPARAMS] = g_param_spec_pointer("fragment-params",
        "Fragment parameters",
        "The fragment parameters of the URI.",
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

    if (!uri->initialized) {
        return;
    }

    if (uri->modified & MASK_SCHEME) {
        upg_free_utr(uri->internal_uri.scheme);
    }

    if (uri->modified & MASK_HOST) {
        upg_free_utr(uri->internal_uri.hostText);
    }

    if (uri->modified & MASK_PATH) {
        upg_free_upsl(uri->internal_uri);
    }

    if (uri->modified & MASK_QUERY) {
        upg_free_utr(uri->internal_uri.query);
    }

    if (uri->modified & MASK_FRAGMENT) {
        upg_free_utr(uri->internal_uri.fragment);
    }

    uri->modified = 0;
    uri->internal_uri.pathHead = uri->original_segment;
    uri->internal_uri.hostText = uri->original_host;
    uri->internal_uri.hostData = uri->original_hostdata;
    uri->internal_uri.query = uri->original_query;
    uri->internal_uri.fragment = uri->original_fragment;
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
        upg_uri_set_host(self, g_value_get_string(value));
        break;
    case PROP_PATH:
        upg_uri_set_path(self, g_value_get_pointer(value));
        break;
    case PROP_QUERY:
        upg_uri_set_query(self, g_value_get_pointer(value));
        break;
    case PROP_QUERYSTR:
        upg_uri_set_query_str(self, g_value_get_string(value));
        break;
    case PROP_FRAGMENT:
        upg_uri_set_fragment(self, g_value_get_string(value));
        break;
    case PROP_FRAGMENTPARAMS:
        upg_uri_set_fragment_params(self, g_value_get_pointer(value));
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
    case PROP_SCHEME:
        g_value_take_string(value, upg_uri_get_scheme(self));
        break;
    case PROP_HOST:
        g_value_set_string(value, upg_uri_get_host(self));
        break;
    case PROP_PATH:
        g_value_set_pointer(value, upg_uri_get_path(self));
        break;
    case PROP_PATHSTR:
        g_value_set_string(value, upg_uri_get_path_str(self));
        break;
    case PROP_QUERY:
        g_value_set_pointer(value, upg_uri_get_query(self));
        break;
    case PROP_QUERYSTR:
        g_value_take_string(value, upg_uri_get_query_str(self));
        break;
    case PROP_FRAGMENT:
        g_value_take_string(value, upg_uri_get_fragment(self));
        break;
    case PROP_FRAGMENTPARAMS:
        g_value_set_pointer(value, upg_uri_get_fragment_params(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static char* str_from_uritextrange(UriTextRangeA range)
{
    g_assert(range.first != NULL);
    g_assert(range.afterLast != NULL);

    ssize_t ptr_len = range.afterLast - range.first;
    g_assert(g_utf8_validate(range.first, ptr_len, NULL));
    return g_strndup(range.first, ptr_len);
}

static UriTextRangeA uritextrange_from_str(const gchar* str)
{
    g_assert(str != NULL);
    int len = strlen(str);
    gchar* dupd = g_strdup(str);

    return (UriTextRangeA) { dupd, dupd + len };
}

static void upg_free_upsl_(UriPathSegmentA** segment, UriPathSegmentA** tail)
{
    UriPathSegmentA* current = *segment;
    do {
        upg_free_utr(current->text);
        current = current->next;
    } while (current != NULL);
    g_free(*segment);
    *segment = NULL;
    *tail = NULL;
}

static GHashTable* parse_query_string(gchar* str)
{
    GHashTable* out = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    gchar** chunks = g_strsplit(str, "&", 0);
    for (gint i = 0; chunks[i] != NULL; i++) {
        gchar** chunk = g_strsplit(chunks[i], "=", 2);
        g_hash_table_insert(out, g_strdup(chunk[0]), g_strdup(chunk[1]));
        g_strfreev(chunk);
    }

    g_strfreev(chunks);
    return out;
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
UpgUri* upg_uri_new(const gchar* uri, GError** error)
{
    UpgUri* ret = g_object_new(UPG_TYPE_URI, NULL);
    GError* err = NULL;
    if (!upg_uri_set_uri(ret, uri, &err)) {
        g_object_unref(ret);
        g_propagate_error(error, err);
        return NULL;
    }
    return ret;
}

/**
 * upg_uri_set_uri:
 * @self: The URI object to reset.
 * @nuri: The new textual URI to be parsed.
 * @error: A #GError.
 *
 * Sets the current URI for the given #UpgUri. If the parsing failed, places a
 * #GError with more information into @error and returns #FALSE.
 *
 * If @nuri is #NULL, then disposes of the object; i.e. the URI is cleared.
 *
 * Returns: Whether or not the operation succeeded.
 */
gboolean upg_uri_set_uri(UpgUri* self, const gchar* nuri, GError** error)
{
    g_assert(error == NULL || *error == NULL);

    if (self->initialized || nuri == NULL) {
        upg_uri_dispose(G_OBJECT(self));

        if (nuri == NULL) {
            return TRUE;
        }
    }

    int ret = 0;
    if ((ret = uriParseSingleUriA(&self->internal_uri, nuri, NULL)) != URI_SUCCESS) {
        g_set_error(error, upg_error_quark(), UPG_ERR_PARSE,
            "Failed to parse URI: %s", upg_strurierror(ret));
        return self->initialized = FALSE;
    } else {
        self->original_hostdata = self->internal_uri.hostData;
        self->original_host = self->internal_uri.hostText;
        self->original_segment = self->internal_uri.pathHead;
        self->original_query = self->internal_uri.query;
        self->original_fragment = self->internal_uri.fragment;
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
    if (nscheme == NULL || uri->modified & MASK_SCHEME) {
        upg_free_utr(uri->internal_uri.scheme);

        if (nscheme == NULL) {
            uri->internal_uri.scheme = (UriTextRangeA) { NULL, NULL };
            return TRUE;
        }
    }

    uri->modified |= MASK_SCHEME;
    uri->internal_uri.scheme = uritextrange_from_str(nscheme);
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
        return g_strdup("");
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
gboolean upg_uri_set_host(UpgUri* uri, const gchar* host)
{
    g_assert(uri->initialized);
    uri->modified |= MASK_HOST;

    // FIXME we should probably parse the incoming host to check if it's IPvX
    uri->internal_uri.hostData = (UriHostDataA) { NULL, NULL, { NULL, NULL } };
    uri->internal_uri.hostText = uritextrange_from_str(host);

    return TRUE;
}

/**
 * upg_uri_get_path:
 * @self: The URI to get path information from.
 *
 * Gets the path information for @uri and returns it as a #GList. If @uri hasn't
 * been initialized, returns #NULL.
 *
 * Note that the list elements are owned by you. You might want to use
 * g_list_free_full() to free the list, instead of just g_list_free or similar.
 *
 * Returns: (nullable) (transfer full) (element-type gchar*): The path segments,
 *                                                            or #NULL.
 */
GList* upg_uri_get_path(UpgUri* uri)
{
    if (!uri->initialized || uri->internal_uri.pathHead == NULL) {
        return NULL;
    }

    GList* list = NULL;
    const UriPathSegmentA start = { { "", "" }, uri->internal_uri.pathHead };
    const UriPathSegmentA* current = &start;

    if (current == NULL) {
        return NULL;
    }

    do {
        current = current->next;
        list = g_list_prepend(list, str_from_uritextrange(current->text));
    } while (current != NULL && current != uri->internal_uri.pathTail);
    list = g_list_reverse(list);

    return list;
}

/**
 * upg_uri_get_path_str:
 * @self: The URI to get the stringified path for.
 *
 * Takes the path data from upg_uri_get_path() and puts it into a string, using
 * slashes as separators.
 *
 * Returns: (nullable) (transfer full): The stringified path segments, or #NULL
 *                                      if @uri hasn't been initialized.
 */
gchar* upg_uri_get_path_str(UpgUri* uri)
{
    if (!uri->initialized) {
        return NULL;
    }

    GString* ret = g_string_new(NULL);
    GList* ocurrent = upg_uri_get_path(uri);
    GList* current = ocurrent;

    if (current == NULL) {
        g_list_free_full(ocurrent, g_free);
        g_string_free(ret, TRUE);
        return g_strdup("");
    }

    while (current != NULL) {
        g_string_append_c(ret, '/');
        g_string_append(ret, (const gchar*)current->data);
        current = current->next;
    }

    g_list_free_full(ocurrent, g_free);
    return g_string_free(ret, FALSE);
}

/**
 * upg_uri_set_path:
 * @self: The #UpgUri to set the path of.
 * @list: (transfer none) (type GList(gchar*)): A list of path segments.
 *
 * Sets the path segments of @uri to @list.
 *
 * Returns: Whether or not the setting succeeded.
 */
gboolean upg_uri_set_path(UpgUri* self, GList* list)
{
    if (self->modified & MASK_PATH) {
        upg_free_upsl(self->internal_uri);
    }

    gint len = g_list_length(list);
    if (len == 0) {
        upg_free_upsl(self->internal_uri);
        self->internal_uri.pathHead = NULL;
        self->internal_uri.pathTail = NULL;
        self->modified |= MASK_PATH;
        return TRUE;
    }

    UriPathSegmentA* segments = g_new0(UriPathSegmentA, len);
    GList* current = list;
    for (gint i = 0; current != NULL; i++) {
        segments[i] = (UriPathSegmentA) { uritextrange_from_str(current->data), &segments[i + 1] };
        current = current->next;
    }
    segments[len - 1].next = NULL;
    self->internal_uri.pathHead = segments;
    self->internal_uri.pathTail = &segments[len - 1];
    self->modified |= MASK_PATH;

    return TRUE;
}

/**
 * upg_uri_get_query:
 * @self: The URI object to get the query of.
 *
 * Gets the query parameters as a #GHashTable. If a parameter doesn't have a
 * value, it is stored as #NULL in the return value. If the query isn't set,
 * returns #NULL.
 *
 * Returns: (transfer full) (nullable): The query parameters.
 */
GHashTable* upg_uri_get_query(UpgUri* self)
{
    if (self->internal_uri.query.first == NULL
        || self->internal_uri.query.afterLast == NULL) {
        return NULL;
    }

    gchar* query = str_from_uritextrange(self->internal_uri.query);
    GHashTable* ret = parse_query_string(query);
    g_free(query);
    return ret;
}

/**
 * upg_uri_get_query_str:
 * @self: The URI object to get the query string of.
 *
 * Gets the query parameters as a string, including the first '?'. If the query
 * isn't set, returns #NULL.
 *
 * Returns: (transfer full) (nullable): The query parameters as a string.
 */
gchar* upg_uri_get_query_str(UpgUri* self)
{
    if (self->internal_uri.query.first == NULL) {
        return NULL;
    }

    gchar* str_current = str_from_uritextrange(self->internal_uri.query);
    GString* ret = g_string_new(str_current);
    g_string_prepend(ret, "?");
    g_free(str_current);
    return g_string_free(ret, FALSE);
}

/**
 * upg_uri_set_query:
 * @self: The URI object to set the query string of.
 * @table: The new query table.
 *
 * Sets the current query parameters to @query.
 *
 * Returns: Whether or not the operation was successful.
 */
gboolean upg_uri_set_query(UpgUri* self, GHashTable* query)
{
    if (query == NULL) {
        return upg_uri_set_query_str(self, NULL);
    }

    GString* built = g_string_new(NULL);

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, query);
    gpointer key = NULL, value = NULL;
    gboolean first = TRUE;
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if (first) {
            first = FALSE;
        } else {
            g_string_append_c(built, '&');
        }

        g_string_append(built, key);

        if (value != NULL) {
            g_string_append_c(built, '=');
            g_string_append(built, value);
        }
    }

    gchar* final = g_string_free(built, FALSE);
    upg_uri_set_query_str(self, final);
    g_free(final);

    return TRUE;
}

/**
 * upg_uri_set_query_str:
 * @self: The URI to set the query string of.
 * @query: (transfer none): The new query string.
 *
 * Sets the query string of @self to @query.
 */
gboolean upg_uri_set_query_str(UpgUri* self, const gchar* nq)
{
    if (self->modified & MASK_QUERY) {
        upg_free_utr(self->internal_uri.query);
    }

    self->modified |= MASK_QUERY;

    if (nq == NULL) {
        self->internal_uri.query = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

    gint len = strlen(nq);
    if ((nq[0] == '?' && len <= 1) || (len == 0)) {
        self->internal_uri.query = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

    if (nq[0] == '?') {
        nq++;
    }

    self->internal_uri.query = uritextrange_from_str(nq);
    return TRUE;
}

/**
 * upg_uri_get_fragment:
 * @self: The URI to get the fragment of.
 *
 * Gets the fragment of @self, if there is one.
 *
 * Returns: (transfer full) (nullable): The fragment of @self.
 */
gchar* upg_uri_get_fragment(UpgUri* uri)
{
    if (uri->internal_uri.fragment.first == NULL) {
        return NULL;
    }

    return str_from_uritextrange(uri->internal_uri.fragment);
}

/**
 * upg_uri_get_fragment_params:
 * @self: The URI to get the fragment parameters of.
 *
 * Returns the fragment parameters of @self, if they exist. They follow a syntax
 * like query parameters, but are in the fragment and not the query.
 *
 * Returns: (transfer full) (nullable): The fragment parameters of @self.
 */
GHashTable* upg_uri_get_fragment_params(UpgUri* uri)
{
    if (uri->internal_uri.fragment.first == NULL) {
        return NULL;
    }

    gchar* fragment = str_from_uritextrange(uri->internal_uri.fragment);
    GHashTable* ret = parse_query_string(fragment);
    g_free(fragment);
    return ret;
}

/**
 * upg_uri_set_fragment:
 * @self: The URI to set the fragment of.
 * @fragment: (transfer none): The new fragment of @self.
 *
 * Sets the fragment of @self to @fragment.
 *
 * Returns: Whether or not the setting succeeded.
 */
gboolean upg_uri_set_fragment(UpgUri* uri, const gchar* fragment)
{
    if (uri->modified & MASK_FRAGMENT) {
        upg_free_utr(uri->internal_uri.fragment);
    }
    uri->modified |= MASK_FRAGMENT;

    if (fragment == NULL) {
        uri->internal_uri.fragment = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

    uri->internal_uri.fragment = uritextrange_from_str(fragment);
    return TRUE;
}

/**
 * upg_uri_set_fragment_params:
 * @self: The URI to set the fragment parameters of.
 * @table: (nullable) (transfer none): The new parameter table.
 *
 * Sets the fragment parameters of @self to @params.
 *
 * Returns: Whether or not the setting succeeded.
 */
gboolean upg_uri_set_fragment_params(UpgUri* uri, GHashTable* params)
{
    if (params == NULL) {
        return upg_uri_set_fragment(uri, NULL);
    }

    GString* made = g_string_new(NULL);

    GHashTableIter iter;
    gpointer key = NULL, value = NULL;
    g_hash_table_iter_init(&iter, params);

    gboolean first = TRUE;
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        if (first) {
            first = FALSE;
        } else {
            g_string_append_c(made, '&');
        }

        g_string_append(made, key);
        g_string_append_c(made, '=');
        g_string_append(made, value);
    }

    gchar* made_str = g_string_free(made, FALSE);
    gboolean ret = upg_uri_set_fragment(uri, made_str);
    g_free(made_str);
    return ret;
}
