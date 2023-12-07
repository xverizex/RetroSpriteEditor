/* nes-setup-palette.h
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
#include "retro-canvas.h"

G_BEGIN_DECLS

#define NES_TYPE_SETUP_PALETTE (nes_setup_palette_get_type())

G_DECLARE_FINAL_TYPE (NesSetupPalette, nes_setup_palette, NES, SETUP_PALETTE, GtkWindow)

G_END_DECLS


RetroCanvas *
nes_setup_palette_get_palette (NesSetupPalette *self, guint32 indx);

void 
nes_setup_palette_draw (NesSetupPalette *self);
