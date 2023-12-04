/* retrospriteeditor-nes-item-palette.h
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

#define RETROSPRITEEDITOR_TYPE_NES_ITEM_PALETTE (retrospriteeditor_nes_item_palette_get_type())

G_DECLARE_FINAL_TYPE (RetrospriteeditorNesItemPalette, retrospriteeditor_nes_item_palette, RETROSPRITEEDITOR, NES_ITEM_PALETTE, GtkBox)

G_END_DECLS

void item_nes_palette_redraw (RetrospriteeditorNesItemPalette *self);
guint32 *item_nes_palette_get_index_colours (RetrospriteeditorNesItemPalette *self);

void item_nes_palette_set_index_colours (RetrospriteeditorNesItemPalette *self,
                                   guint32                         *index);

void item_nes_palette_set_colours (RetrospriteeditorNesItemPalette *self,
                               guint32                         *colour,
                               guint32                          count);

GtkWidget *item_nes_palette_get_radio (RetrospriteeditorNesItemPalette *self);
guint32 *item_nes_palette_get_colour (RetrospriteeditorNesItemPalette *self);
guint32 *item_nes_palette_get_colour_index (RetrospriteeditorNesItemPalette *self);
void item_nes_palette_get_color_from_index (RetrospriteeditorNesItemPalette *self);