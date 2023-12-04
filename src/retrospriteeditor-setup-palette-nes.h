/* retrospriteeditor-setup-palette-nes.h
 *
 * Copyright 2023 vi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RETROSPRITEEDITOR_TYPE_SETUP_PALETTE_NES (retrospriteeditor_setup_palette_nes_get_type())

G_DECLARE_FINAL_TYPE (RetrospriteeditorSetupPaletteNes, retrospriteeditor_setup_palette_nes, RETROSPRITEEDITOR, SETUP_PALETTE_NES, GtkWindow)

G_END_DECLS

#include "retrospriteeditor-canvas.h"

RetrospriteeditorCanvas *
setup_palette_window_get_palette (RetrospriteeditorSetupPaletteNes *self, guint32 indx);
void setup_palette_nes_draw (RetrospriteeditorSetupPaletteNes *self);
