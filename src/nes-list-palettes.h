/* nes-list-palettes.h
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

#define NES_TYPE_LIST_PALETTE (nes_list_palette_get_type())

G_DECLARE_FINAL_TYPE (NesListPalette, nes_list_palette, NES, LIST_PALETTE, GtkFrame)

G_END_DECLS

typedef struct _NesBanks {
	guint32 bank[2][4][4];
} NesBanks;

NesBanks *
nes_list_palette_get_bank (void);

void 
nes_list_palette_set_bank (guint32 index);

void 
nes_list_palette_redraw (void);

GtkWidget **
nes_list_palette_get_items (void);
