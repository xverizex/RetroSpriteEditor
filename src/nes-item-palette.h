/* nes-item-palette.h
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

#define NES_TYPE_ITEM_PALETTE (nes_item_palette_get_type())

G_DECLARE_FINAL_TYPE (NesItemPalette, nes_item_palette, NES, ITEM_PALETTE, GtkBox)

G_END_DECLS

void 
nes_item_set_id (NesItemPalette *self, guint32 item_id);

void 
nes_item_palette_redraw (NesItemPalette *self);

guint32 *
nes_item_palette_get_index_colours (NesItemPalette *self);

void 
nes_item_palette_set_index_colours (NesItemPalette *self, guint32 *index);

void 
nes_item_palette_set_colours (NesItemPalette *self,
                               guint32                         *colour,
                               guint32                          count);

GtkWidget *
nes_item_palette_get_radio (NesItemPalette *self);

guint32 *
nes_item_palette_get_colour (NesItemPalette *self);

guint32 *
nes_item_palette_get_colour_index (NesItemPalette *self);

void 
nes_item_palette_get_color_from_index (NesItemPalette *self);

void 
nes_item_palette_show_hex (NesItemPalette *self, int show);
