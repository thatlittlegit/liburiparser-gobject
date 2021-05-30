/* liburiparser-gobject.c
 *
 * Copyright 2020-2021 thatlittlegit
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

static void upg_uri_class_init(UpgUriClass*);
static void upg_uri_init(UpgUri*);
static void upg_uri_dispose(GObject*);
static void upg_uri_finalize(GObject*);
static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec);
static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec);
static gchar* str_from_uritextrange(UriTextRangeA range);
static UriTextRangeA uritextrange_from_str(const gchar* str);
static void upg_free_upsl_(UriPathSegmentA** segment, UriPathSegmentA** tail);
static gboolean upg_uri_set_internal_uri(UpgUri* self, void* internal);
static gchar* upg_uriuri_to_string(UriUriA* self, GError** error);

#define upg_free_utr(p) g_free((gchar*)p.first)
#define upg_free_upsl(u) upg_free_upsl_(&u.pathHead, &u.pathTail)
#define priv(x) ((UpgUriPrivate*)upg_uri_get_instance_private(x))
// Don't use for anything expensive or where a has side effects!
#define either_or(a, b) ((a) ? (a) : (b))

enum {
    PROP_SCHEME = 1,
    PROP_HOST,
    PROP_PATH,
    PROP_PATHSTR,
    PROP_QUERY,
    PROP_QUERYSTR,
    PROP_FRAGMENT,
    PROP_FRAGMENTPARAMS,
    PROP_PORT,
    PROP_USERINFO,
    PROP_USERNAME,
    _N_PROPERTIES_
};

enum {
    MASK_SCHEME = 1 << 1,
    MASK_HOST = 1 << 2,
    MASK_PATH = 1 << 3,
    MASK_QUERY = 1 << 4,
    MASK_FRAGMENT = 1 << 5,
    MASK_PORT = 1 << 6,
    MASK_USERINFO = 1 << 7,
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
typedef struct {
    GObject parent_instance;

    // private
    gboolean initialized;
    gboolean dirty;
    UriUriA internal_uri;
    gint32 modified;
    UriPathSegmentA* original_segment;
    UriTextRangeA original_host;
    UriHostDataA original_hostdata;
    UriTextRangeA original_query;
    UriTextRangeA original_fragment;
    UriTextRangeA original_port;
    UriTextRangeA original_userinfo;
    UriTextRangeA original_scheme;
    gchar* cached;
} UpgUriPrivate;
G_DEFINE_TYPE_WITH_PRIVATE(UpgUri, upg_uri, G_TYPE_OBJECT);

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
    /**
     * UpgUri:port: (type guint16)
     *
     * The port of the URI.
     */
    params[PROP_PORT] = g_param_spec_uint("port",
        "Port",
        "The port of the URI.",
        0,
        65535,
        0,
        G_PARAM_READWRITE);
    params[PROP_USERINFO] = g_param_spec_string("userinfo",
        "User Information",
        "The user information of the URI.",
        NULL,
        G_PARAM_READWRITE);
    params[PROP_USERNAME] = g_param_spec_string("username",
        "User Name",
        "The username portion of the user information.",
        NULL,
        G_PARAM_READABLE);
    g_object_class_install_properties(glass, _N_PROPERTIES_, params);

    glass->dispose = upg_uri_dispose;
    glass->finalize = upg_uri_finalize;
}

static void upg_uri_init(UpgUri* self)
{
    priv(self)->dirty = TRUE;
}

static void upg_uri_dispose(GObject* self)
{
    G_OBJECT_CLASS(upg_uri_parent_class)->dispose(self);

    UpgUriPrivate* uri = priv(UPG_URI(self));

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

    if (uri->modified & MASK_PORT) {
        upg_free_utr(uri->internal_uri.portText);
    }

    if (uri->modified & MASK_USERINFO) {
        upg_free_utr(uri->internal_uri.userInfo);
    }

    uri->modified = 0;
    uri->internal_uri.scheme = uri->original_scheme;
    uri->internal_uri.pathHead = uri->original_segment;
    uri->internal_uri.userInfo = uri->original_userinfo;
    uri->internal_uri.hostText = uri->original_host;
    uri->internal_uri.hostData = uri->original_hostdata;
    uri->internal_uri.query = uri->original_query;
    uri->internal_uri.fragment = uri->original_fragment;
    uri->internal_uri.portText = uri->original_port;
    uriFreeUriMembersA(&uri->internal_uri);
    uri->initialized = FALSE;
    g_free(uri->cached);
    uri->dirty = TRUE;
}

static void upg_uri_finalize(GObject* self)
{
    G_OBJECT_CLASS(upg_uri_parent_class)->finalize(self);
}

static void upg_uri_set_property(GObject* obj, guint id, const GValue* value, GParamSpec* spec)
{
    UpgUri* self = UPG_URI(obj);

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
    case PROP_PORT:
        upg_uri_set_port(self, g_value_get_uint(value));
        break;
    case PROP_USERINFO:
        upg_uri_set_userinfo(self, g_value_get_string(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static void upg_uri_get_property(GObject* obj, guint id, GValue* value, GParamSpec* spec)
{
    UpgUri* self = UPG_URI(obj);

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
    case PROP_PORT:
        g_value_set_uint(value, upg_uri_get_port(self));
        break;
    case PROP_USERINFO:
        g_value_take_string(value, upg_uri_get_userinfo(self));
        break;
    case PROP_USERNAME:
        g_value_take_string(value, upg_uri_get_username(self));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static char* str_from_uritextrange(UriTextRangeA range)
{
    if (range.first == NULL || range.afterLast == NULL) {
        return NULL;
    }

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
    while (current != NULL) {
        upg_free_utr(current->text);
        current = current->next;
    }
    g_free(*segment);
    *segment = NULL;
    *tail = NULL;
}

static GHashTable* parse_query_string(gchar* str)
{
    if (str == NULL) {
        return NULL;
    }

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
 * @uri: (nullable): The input URI to be parsed, or #NULL.
 * @error: A #GError.
 *
 * > The URI is normalized while it is parsed. You cannot use
 * > liburiparser-gobject if you need non-normalized URI support; the way that
 * > normalization works is difficult to achieve with the memory model used.
 *
 * Creates a new #UpgUri by parsing @uri. Note that @uri must be a valid URI,
 * otherwise it will fail. It can also be #NULL, in which case an empty URI will
 * be returned.
 *
 * Returns: (transfer full) (nullable): a new #UpgUri if the parsing was
 *          successful, or #NULL.
 */
UpgUri* upg_uri_new(const gchar* uri, GError** error)
{
    UpgUri* ret = g_object_new(UPG_TYPE_URI, NULL);
    if (!upg_uri_configure_from_string(ret, uri, error)) {
        g_object_unref(ret);
        return NULL;
    }
    return ret;
}

/**
 * upg_uri_configure_from_string:
 * @self: The URI object to reset.
 * @nuri: The new textual URI to be parsed.
 * @error: A #GError.
 *
 * > The URI is normalized while it is parsed. You cannot use
 * > liburiparser-gobject if you need non-normalized URI support; the way that
 * > normalization works is difficult to achieve with the memory model used.
 *
 * Sets the current URI for the given #UpgUri. If the parsing failed, places a
 * #GError with more information into @error and returns #FALSE.
 *
 * If @nuri is #NULL, then disposes of the object; i.e. the URI is cleared.
 *
 * Returns: Whether or not the operation succeeded.
 */
gboolean upg_uri_configure_from_string(UpgUri* _self, const gchar* nuri, GError** error)
{
    g_assert(error == NULL || *error == NULL);

    if (nuri == NULL) {
        return TRUE;
    }

    UriUriA parsed;
    int ret = 0;
    if ((ret = uriParseSingleUriA(&parsed, nuri, NULL)) != URI_SUCCESS) {
        g_set_error(error, upg_error_quark(), UPG_ERR_PARSE,
            "Failed to parse URI: %s", upg_strurierror(ret));
        return FALSE;
    }

    if ((ret = uriNormalizeSyntaxA(&parsed)) != URI_SUCCESS) {
        g_set_error(error, upg_error_quark(), UPG_ERR_NORMALIZE,
            "Failed to normalize URI: %s", upg_strurierror(ret));
        return FALSE;
    }

    return upg_uri_set_internal_uri(_self, &parsed);
}

/* [this API may soon be public]
 * upg_uri_set_internal_uri:
 * @self: The URI to configure.
 * @uri: (transfer none) (not nullable): The UriUriA object to use.
 *
 * > Avoid this API. It may break in future if the underlying parser is changed,
 * > or something else happens. This function should still be there, if ABI
 * > compatibility is a concern, but make sure to check the return value and
 * > have a backup plan.
 *
 * Sets the internal URI object of @self.
 *
 * Returns: Whether or not the operation succeeded.
 */
static gboolean upg_uri_set_internal_uri(UpgUri* _self, void* uri)
{
    UpgUriPrivate* self = priv(_self);

    if (self->initialized) {
        upg_uri_dispose(G_OBJECT(self));
    }

    memcpy(&self->internal_uri, uri, sizeof(UriUriA));

    self->original_scheme = self->internal_uri.scheme;
    self->original_userinfo = self->internal_uri.userInfo;
    self->original_hostdata = self->internal_uri.hostData;
    self->original_host = self->internal_uri.hostText;
    self->original_segment = self->internal_uri.pathHead;
    self->original_query = self->internal_uri.query;
    self->original_fragment = self->internal_uri.fragment;
    self->original_port = self->internal_uri.portText;

    return self->initialized = TRUE;
}

/**
 * upg_uri_to_string_ign:
 * @self: The URI to convert to a string.
 * @fatal: The response that should be given to errors.
 *
 * Calls upg_uri_to_string() and responds to the error. This is useful if
 * exception ignoring isn't just passing #NULL to the function, like in Vala
 * where try/catches have to be used.
 *
 * Returns: (transfer full) (allow-none): The textual representation of the URI.
 */
gchar* upg_uri_to_string_ign(UpgUri* self, UpgUriFatalRanking ranking)
{
    GError* err = NULL;
    gchar* res = upg_uri_to_string(self, &err);
    if (err == NULL) {
        return res;
    }

    switch (ranking) {
    case UPG_URI_FATAL:
        g_error("%s", err->message);
        g_error_free(err);
        return NULL;
    case UPG_URI_NONFATAL_NEVERNULL:
        g_warning("%s", err->message);
        g_error_free(err);
        return g_strdup("");
    case UPG_URI_NONFATAL_NULLABLE:
        g_warning("%s", err->message);
        g_error_free(err);
        return NULL;
    default:
        g_critical("Unknown UpgUriFatalRanking value %u", ranking);
        G_BREAKPOINT();
        return NULL;
    }
}

/**
 * upg_uri_to_string:
 * @self: The URI to convert to a string.
 * @error: A #GError.
 *
 * Converts the in-memory URI object into a string.
 *
 * Returns: (transfer full): The textual representation of the URI.
 */
gchar* upg_uri_to_string(UpgUri* _self, GError** err)
{
    UpgUriPrivate* self = priv(_self);
    g_assert(self->initialized);

    if (!self->dirty) {
        return g_strdup(self->cached);
    }

    gchar* string;
    if ((string = upg_uriuri_to_string(&self->internal_uri, err)) == NULL) {
        return NULL;
    }

    self->dirty = FALSE;
    g_free(self->cached);
    self->cached = g_strdup(string);
    return string;
}

/*
 * upg_uriuri_to_string:
 * @self: The URI to convert.
 *
 * Converts @self to a string.
 *
 * Returns: (transfer full): @self as a string.
 */
gchar* upg_uriuri_to_string(UriUriA* self, GError** error)
{
    int len, ret;
    if ((ret = uriToStringCharsRequiredA(self, &len)) != URI_SUCCESS) {
        g_set_error(error, UPG_ERROR, UPG_ERR_TOSTRING, "Failed to determine length of URI: %s", upg_strurierror(ret));
        return NULL;
    }
    len++;

    gchar* out = g_malloc0_n(len, sizeof(char));
    int written;
    if ((ret = uriToStringA(out, self, len, &written)) != URI_SUCCESS) {
        g_set_error(error, UPG_ERROR, UPG_ERR_TOSTRING, "Failed to write out URI: %s", upg_strurierror(ret));
        g_free(out);
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
gboolean upg_uri_set_scheme(UpgUri* _self, const gchar* nscheme)
{
    UpgUriPrivate* uri = priv(_self);
    uri->dirty = TRUE;

    if (uri->modified & MASK_SCHEME) {
        upg_free_utr(uri->internal_uri.scheme);
    }
    uri->modified |= MASK_SCHEME;

    if (nscheme == NULL) {
        uri->internal_uri.scheme = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

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
    return str_from_uritextrange(priv(uri)->internal_uri.scheme);
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
    return str_from_uritextrange(priv(uri)->internal_uri.hostText);
}

/**
 * upg_uri_get_host_data:
 * @self: The URI to get the hostname of.
 * @protocol: (out): The protocol used by the name. Set to zero if no valid
 *                   protocol.
 *
 * Gets the current host information as an array, with a value indicating the
 * protocol put into @uri. If @uri hasn't been initialized, returns #NULL.
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
 * Returns: (transfer full) (nullable): The host data as an array.
 */
guint8* upg_uri_get_host_data(UpgUri* _self, guint8* protocol)
{
    UpgUriPrivate* uri = priv(_self);
    g_assert(uri->initialized);

    UriHostDataA* data = &uri->internal_uri.hostData;
    if (data->ip4 != NULL) {
        *protocol = 4;
        void* ret = g_malloc0(4);
        memcpy(ret, data->ip4->data, 4);
        return ret;
    }

    if (data->ip6 != NULL) {
        *protocol = 6;
        void* ret = g_malloc0(16);
        memcpy(ret, data->ip6->data, 16);
        return ret;
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
gboolean upg_uri_set_host(UpgUri* _self, const gchar* host)
{
    UpgUriPrivate* uri = priv(_self);
    g_assert(uri->initialized);
    uri->modified |= MASK_HOST;
    uri->dirty = TRUE;

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
GList* upg_uri_get_path(UpgUri* _self)
{
    UpgUriPrivate* uri = priv(_self);
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
gchar* upg_uri_get_path_str(UpgUri* _self)
{
    UpgUriPrivate* uri = priv(_self);
    if (!uri->initialized) {
        return NULL;
    }

    GString* ret = g_string_new(NULL);
    GList* ocurrent = upg_uri_get_path(_self);
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
gboolean upg_uri_set_path(UpgUri* _self, GList* list)
{
    UpgUriPrivate* self = priv(_self);
    if (self->modified & MASK_PATH) {
        upg_free_upsl(self->internal_uri);
    }

    gint len = g_list_length(list);
    if (len == 0) {
        self->internal_uri.pathHead = NULL;
        self->internal_uri.pathTail = NULL;
        self->modified |= MASK_PATH;
        self->dirty = TRUE;
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
    self->dirty = TRUE;

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
GHashTable* upg_uri_get_query(UpgUri* _self)
{
    UpgUriPrivate* self = priv(_self);
    gchar* query = str_from_uritextrange(self->internal_uri.query);
    GHashTable* ret = parse_query_string(query);
    g_free(query);
    return ret;
}

/**
 * upg_uri_get_query_str:
 * @self: The URI object to get the query string of.
 *
 * Gets the query parameters as a string, excluding the first '?'. If the query
 * isn't set, returns #NULL.
 *
 * Returns: (transfer full) (nullable): The query parameters as a string.
 */
gchar* upg_uri_get_query_str(UpgUri* self)
{
    return str_from_uritextrange(priv(self)->internal_uri.query);
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
gboolean upg_uri_set_query(UpgUri* _self, GHashTable* query)
{
    if (query == NULL) {
        return upg_uri_set_query_str(_self, NULL);
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
    upg_uri_set_query_str(_self, final);
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
gboolean upg_uri_set_query_str(UpgUri* _self, const gchar* nq)
{
    UpgUriPrivate* self = priv(_self);
    if (self->modified & MASK_QUERY) {
        upg_free_utr(self->internal_uri.query);
    }

    self->modified |= MASK_QUERY;
    self->dirty = TRUE;

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
    return str_from_uritextrange(priv(uri)->internal_uri.fragment);
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
    gchar* fragment = str_from_uritextrange(priv(uri)->internal_uri.fragment);
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
gboolean upg_uri_set_fragment(UpgUri* _self, const gchar* fragment)
{
    UpgUriPrivate* uri = priv(_self);
    if (uri->modified & MASK_FRAGMENT) {
        upg_free_utr(uri->internal_uri.fragment);
    }
    uri->dirty = TRUE;
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

/**
 * upg_uri_get_port:
 * @self: The URI to get the port of.
 *
 * Gives the current port number of the URI. If no port number is provided,
 * returns 0.
 *
 * Note that because of it returning zero, there is no way to distinguish
 * `https://example.edu` and `https://example.edu:0`. It is simply believed that
 * zero is such a bizarre port, and that is is reserved by IANA, that it should
 * therefore not be found in any sane URI. Re-parsing the URI manually may be
 * the only option here, if you must determine the difference.
 *
 * Returns: The port of the URI, as a #guint16, or 0 if there isn't one.
 */
guint16 upg_uri_get_port(UpgUri* self)
{
    gchar* port_str = str_from_uritextrange(priv(self)->internal_uri.portText);
    guint ret = strtoull(either_or(port_str, "0"), NULL, 10);
    g_free(port_str);
    return (guint16)ret;
}

/**
 * upg_uri_set_port:
 * @self: The URI to change.
 * @port: The new port value.
 *
 * Sets the port of @self to @port. There is no way to change the port to 0;
 * setting the port to zero will remove the port entirely from the URI.
 *
 * Returns: Whether or not the operation succeeded.
 */
gboolean upg_uri_set_port(UpgUri* _self, guint16 port)
{
    UpgUriPrivate* self = priv(_self);
    if (self->modified & MASK_PORT) {
        upg_free_utr(self->internal_uri.portText);
    }
    self->modified |= MASK_PORT;
    self->dirty = TRUE;

    if (port == 0) {
        self->internal_uri.portText = (UriTextRangeA) { NULL, NULL };
        return TRUE;
    }

    gchar buf[6];
    g_ascii_dtostr(buf, 6, port);
    self->internal_uri.portText = uritextrange_from_str(buf);
    return TRUE;
}

/**
 * upg_uri_get_userinfo:
 * @self: The URI to get the user information of.
 *
 * Gets the user information of the URI. Returns #NULL if there isn't any.
 *
 * Returns: (transfer full) (nullable): The user information of the URI.
 */
gchar* upg_uri_get_userinfo(UpgUri* uri)
{
    return str_from_uritextrange(priv(uri)->internal_uri.userInfo);
}

/**
 * upg_uri_get_username:
 * @self: The URI to get the user information of.
 *
 * Gets the username of the URI; that is, the text before the colon in the user
 * information. Returns #NULL if there isn't user information: if there is user
 * information, then at least an empty string will be returned.
 *
 * Returns: (transfer full) (nullable): The username of @self.
 */
gchar* upg_uri_get_username(UpgUri* uri)
{
    gchar* userinfo = str_from_uritextrange(priv(uri)->internal_uri.userInfo);
    gchar** chunks = g_strsplit(either_or(userinfo, ""), ":", 2);
    gchar* ret = g_strdup(chunks[0]);
    g_strfreev(chunks);
    g_free(userinfo);
    return ret;
}

/**
 * upg_uri_set_userinfo:
 * @self: The URI to set the user information of.
 * @userinfo: The new user information.
 *
 * Sets the user information of @self to @userinfo. If it is set to #NULL, then
 * it is removed.
 *
 * Returns: Whether or not the operation was successful.
 */
gboolean upg_uri_set_userinfo(UpgUri* _self, const gchar* userinfo)
{
    UpgUriPrivate* uri = priv(_self);
    if (uri->modified & MASK_USERINFO) {
        upg_free_utr(uri->internal_uri.userInfo);
    }
    uri->modified |= MASK_USERINFO;
    uri->dirty = TRUE;

    if (userinfo == NULL) {
        uri->internal_uri.userInfo = (UriTextRangeA) { NULL, NULL };
    }

    uri->internal_uri.userInfo = uritextrange_from_str(userinfo);
    return TRUE;
}

/**
 * upg_uri_apply_reference:
 * @self: The URI to use as a base.
 * @reference: The reference to apply to @self.
 * @error: A #GError.
 *
 * Applies @reference to @self and returns the new URI.
 *
 * Returns: (transfer full) (nullable): The applied URI, or #NULL if @error is
 *          set.
 */
UpgUri* upg_uri_apply_reference(UpgUri* self, const gchar* reference_str, GError** error)
{
    UpgUri* final = NULL;

    UriUriA reference;
    gint ret;
    if ((ret = uriParseSingleUriA(&reference, reference_str, NULL)) != URI_SUCCESS) {
        g_set_error(error, UPG_ERROR, UPG_ERR_PARSE, "Failed to parse reference: %s", upg_strurierror(ret));
        return final;
    }

    UriUriA* base = &priv(self)->internal_uri;
    UriUriA applied;
    if ((ret = uriAddBaseUriA(&applied, &reference, base)) != URI_SUCCESS) {
        g_set_error(error, UPG_ERROR, UPG_ERR_REFERENCE, "Failed to apply reference: %s", upg_strurierror(ret));
        goto cleanup;
    }

    if ((ret = uriNormalizeSyntaxA(&applied)) != URI_SUCCESS) {
        g_set_error(error, UPG_ERROR, UPG_ERR_NORMALIZE, "Failed to normalize applied URI: %s", upg_strurierror(ret));
        goto cleanup;
    }

    final = upg_uri_new(NULL, error);
    if (final == NULL) {
        goto cleanup;
    }

    upg_uri_set_internal_uri(final, &applied);

cleanup:
    uriFreeUriMembersA(&reference);

    return final;
}

/**
 * upg_uri_subtract_to_reference:
 * @self: The minutend.
 * @subtrahend: The subtrahend (to be subtracted from @self).
 * @error: A #GError.
 *
 * Subtracts @subtrahend from @self to give a reference that can be used on
 * @self to get @subtrahend. The resulting value should be relative, but if the
 * scheme and hostname differ then it could very well be absolute.
 *
 * If @error is set, returns #NULL.
 *
 * Returns: (transfer full) (nullable): The subtracted reference.
 */
gchar* upg_uri_subtract_to_reference(UpgUri* self, UpgUri* subtrahend, GError** err)
{
    UriUriA* base = &priv(self)->internal_uri;
    UriUriA* source = &priv(subtrahend)->internal_uri;

    UriUriA dest;
    gint ret;

    if ((ret = uriRemoveBaseUriA(&dest, source, base, FALSE)) != URI_SUCCESS) {
        g_set_error(err, UPG_ERROR, UPG_ERR_REFERENCE, "Failed to create reference: %s", upg_strurierror(ret));
        return NULL;
    }

    gchar* final;
    if ((final = upg_uriuri_to_string(&dest, err)) == NULL) {
        uriFreeUriMembersA(&dest);
        return NULL;
    }

    uriFreeUriMembersA(&dest);
    return final;
}

/**
 * upg_uri_unref:
 * @self: The #UpgUri to unref.
 *
 * Wrapper for Vala over g_object_unref(). Don't worry if you're using C.
 */
void upg_uri_unref(gpointer self)
{
    g_object_unref(self);
}

/**
 * upg_uri_ref:
 * @self: The #UpgUri to ref.
 *
 * Wrapper for Vala over g_object_ref(). Don't worry if you're using C.
 *
 * Returns: @self
 */
gpointer upg_uri_ref(gpointer self)
{
    return g_object_ref(UPG_URI(self));
}
