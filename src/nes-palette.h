/* nes-palette.h
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
#include "nes-params.h"

G_BEGIN_DECLS

#define NES_TYPE_PALETTE (nes_palette_get_type())

G_DECLARE_FINAL_TYPE (NesPalette, nes_palette, NES, PALETTE, GtkBox)

G_END_DECLS



NesTilePoint *
nes_palette_get_map (guint32 index);

NesTilePoint *
nes_palette_get_block (guint32 w, guint32 h, guint32 x, guint32 y, guint32 index_map);

NesPalette *
nes_palette_get (void);

void 
nes_palette_redraw (void);

NesTilePoint *
nes_palette_screen_get_color (NesPalette *self,
                       guint32 x, guint32 y);

NesTilePoint *
nes_palette_get_color (NesPalette *self,
                       guint32 x, guint32 y);

void 
nes_palette_set_color (NesPalette *self,
                    NesParamPoint               *params,
                    guint32                      index);

void 
nes_palette_set_color_with_map (NesPalette *self,
                    NesParamPoint               *params,
                    guint32                      index,
										guint32                      map);

void
nes_palette_clean_map (void);
