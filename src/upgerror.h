/* upgerror.h
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
#ifndef UPGERROR_H
#define UPGERROR_H

#include <glib.h>

#if !defined(__LIBURIPARSER_GOBJECT_INSIDE__) && !defined(LIBURIPARSER_GOBJECT_COMPILATION)
#error "Only <liburiparser-gobject.h> can be included directly."
#endif

/**
 * SECTION: upgerror
 * @short_description: Error handling with UPG.
 *
 * #UpgError et al are the primary ways of reporting errors that aren't your
 * fault. This is not too interesting: the most interesting thing is probably
 * #UpgError.
 */

/**
 * UpgError:
 * @UPG_ERR_PARSE: An error occurred during parsing.
 * @UPG_ERR_NORMALIZE: An error occurred during normalization.
 *
 * The types of errors that can occur in UPG.
 */
typedef enum {
    UPG_ERR_PARSE,
    UPG_ERR_NORMALIZE,
} UpgError;

/**
 * UPG_ERROR:
 * An alias for upg_error_quark().
 */
#define UPG_ERROR upg_error_quark()
GQuark upg_error_quark();

#ifdef LIBURIPARSER_GOBJECT_COMPILATION
#define upg_strurierror(ret) __upg_str_from_urierror__(ret)
gchar* __upg_str_from_urierror__(gint ret);
#endif

#endif
