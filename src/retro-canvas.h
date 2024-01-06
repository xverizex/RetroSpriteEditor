/* retro-canvas.h
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

#include <adwaita.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RETRO_TYPE_CANVAS (retro_canvas_get_type())

G_DECLARE_FINAL_TYPE (RetroCanvas, retro_canvas, RETRO, CANVAS, GtkDrawingArea)

G_END_DECLS

typedef struct _CanvasSettings {
  gint32 type_canvas;
  gint32 canvas_width;
  gint32 canvas_height;
  gint32 palette_type;
  gint32 width_rect;
  gint32 height_rect;
  gint32 scale;
  gint32 count_x;
  gint32 count_y;
  gboolean left_top;
	guint32 pos_height;
} CanvasSettings;

enum {
  TYPE_CANVAS_TILESET,
  TYPE_CANVAS_COLOUR_GRID,
  TYPE_CANVAS_COLOUR_PALETTE,
	TYPE_CANVAS_SCREEN_BACKGROUND,
  N_TYPE_CANVAS
};



enum {
  NES_SPRITE,
  NES_BACKGROUND,
  N_NES
};


void retro_canvas_set_type_palette (RetroCanvas *self,
                                                guint32                  type,
                                                guint32                  count);

void retro_canvas_shut_on_event_click (RetroCanvas *self);
void retro_canvas_shut_on_events (RetroCanvas *self);

void retro_canvas_restructure (RetroCanvas *self);
RetroCanvas *retro_canvas_get_drawing_canvas (void);
RetroCanvas *retro_canvas_get_tileset (void);
void retro_drawing_canvas_redraw (void);

void retro_canvas_set_drawing_canvas (RetroCanvas *);
void retro_canvas_set_tileset (RetroCanvas *);


void retro_canvas_set_tool (RetroCanvas *, guint32 tool);

guint32 *
retro_canvas_palette_get_ptr_index_colours (RetroCanvas *self,
		guint32 x, guint32 y);

void
retro_canvas_set_xy_click (RetroCanvas *self);

void
retro_canvas_set_child_color (RetroCanvas *self, RetroCanvas *child);

void
retro_canvas_set_index_colours (RetroCanvas *self, guint32 *index_color);
void
retro_canvas_set_colours (RetroCanvas *self, guint32 count);

void
retro_canvas_set_item_id (RetroCanvas *self, guint32 id);

guint32
retro_canvas_get_item_id (RetroCanvas *self);

void
retro_canvas_set_show_hex (RetroCanvas *self, gboolean val);
void
retro_canvas_set_index_colours_by_index (RetroCanvas * self, guint32 index_id, guint32 index_color);

void
retro_canvas_set_colours_by_index (RetroCanvas * self, guint32 index_id, guint32 index_color);

void
retro_canvas_set_index_id (RetroCanvas *self, guint32 index);

void
retro_canvas_set_index (RetroCanvas *self, guint32 index);

void
retro_canvas_redraw_drawing_and_tileset (void);

void
retro_canvas_shut_on_events_nes_screen (RetroCanvas *self);

void
retro_canvas_set_blank (RetroCanvas *self, int is_blank);

void
retro_canvas_set_copy (RetroCanvas *self, guint32 is_copy);

guint8 *
retro_canvas_get_megatile (RetroCanvas *self);

guint8 *
retro_canvas_screen_nes_get_megatile (void);

guint8 *
retro_canvas_nes_get_megatile_by_block (RetroCanvas *self);

void
retro_canvas_nes_set_screen (RetroCanvas *self);

GtkWidget *
retro_canvas_nes_get_screen (void);

void *
retro_canvas_screen_nes_get_tile_ref (void);

void *
retro_canvas_nes_get_tile_ref (RetroCanvas *self);

guint8 *
retro_canvas_nes_get_megatile (RetroCanvas *self);
