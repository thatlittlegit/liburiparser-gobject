/* liburiparser-gobject-version.c
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

/**
 * upg_check_version:
 * @major: required major version
 * @minor: required minor version
 * @patch: required patch version
 *
 * Runtime version checking. Use UPG_CHECK_VERSION() instead if you can, but if
 * you're using GIR/VAPI this is probably your best shot.
 *
 * Checks if the version you ask for is compatible with the version built.
 *
 * Returns: Whether or not the versions are compatible.
 */
gboolean upg_check_version(gint major, gint minor, gint patch)
{
    return UPG_CHECK_VERSION(major, minor, patch);
}
