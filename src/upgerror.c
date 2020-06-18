/* upgerror.c
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
#include "liburiparser-gobject.h"
#include <uriparser/UriBase.h>

G_DEFINE_QUARK(upgerror, upg_error);

/*
 * __upg_str_from_urierror__:
 * @ret: The return value to explain.
 *
 * > This is an internal function! Do not use!
 *
 * Explains a liburiparser return value.
 *
 * Returns: (transfer none): The explained value.
 */
gchar* __upg_str_from_urierror__(gint ret)
{
    switch (ret) {
    case URI_SUCCESS:
        return "nothing is wrong and the person who wrote this software is an idiot";
    case URI_ERROR_SYNTAX:
        return "the text parsed was invalid";
    case URI_ERROR_NULL:
        g_assert_not_reached();
        return "a parameter that should've been passed wasn't: programmer error!";
    case URI_ERROR_MALLOC:
        return "out of memory";
    case URI_ERROR_OUTPUT_TOO_LARGE:
        g_assert_not_reached();
        return "output too large for buffer: programmer error!";
    case URI_ERROR_NOT_IMPLEMENTED:
        return "function not implemented";
    case URI_ERROR_RANGE_INVALID:
        g_assert_not_reached();
        return "a range passed to a function was invalid: programmer error!";
    case URI_ERROR_ADDBASE_REL_BASE:
    case URI_ERROR_REMOVEBASE_REL_BASE:
    case URI_ERROR_REMOVEBASE_REL_SOURCE:
        return "the base URI given is not absolute";
    default:
        return "unknown or irrelevant error";
    }
}
