/* nes-item-palette.c
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

#include "config.h"

#include "nes-item-palette.h"
#include "retro-canvas.h"
#include "global-functions.h"

struct _NesItemPalette
{
  GtkBox  parent_instance;
  GtkWidget *radio_btn;
  GtkWidget *canvas;

  guint32 *colour;
  guint32 *index_color;
};

#define DEFAULT_COLOUR_CANVAS_WIDTH             128
#define DEFAULT_COLOUR_CANVAS_HEIGHT             32
#define DEFAULT_COLOUR_CANVAS_SCALE               1
#define DEFAULT_COLOUR_PALETTE_TYPE               0

G_DEFINE_FINAL_TYPE (NesItemPalette, nes_item_palette, GTK_TYPE_BOX)

static void
nes_item_palette_class_init (NesItemPaletteClass *klass)
{
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
nes_item_palette_init (NesItemPalette *self)
{
  self->index_color = NULL;

  self->radio_btn = gtk_check_button_new ();
  self->canvas = g_object_new (RETRO_TYPE_CANVAS,
                               NULL);

  gtk_widget_set_size_request (self->canvas, DEFAULT_COLOUR_CANVAS_WIDTH, DEFAULT_COLOUR_CANVAS_HEIGHT);
  CanvasSettings cs_palette;
  cs_palette.type_canvas    = TYPE_CANVAS_COLOUR_PALETTE;
  cs_palette.canvas_width   = DEFAULT_COLOUR_CANVAS_WIDTH;
  cs_palette.canvas_height  = DEFAULT_COLOUR_CANVAS_HEIGHT;
  cs_palette.palette_type   = DEFAULT_COLOUR_PALETTE_TYPE;
  cs_palette.scale          = DEFAULT_COLOUR_CANVAS_SCALE;
  cs_palette.width_rect     = 32;
  cs_palette.height_rect    = 32;
  cs_palette.count_x        = 4;
  cs_palette.count_y        = 1;
  cs_palette.left_top       = TRUE;
  g_object_set (self->canvas, "settings", &cs_palette, NULL);

  gtk_box_append (GTK_BOX (self), self->radio_btn);
  gtk_box_append (GTK_BOX (self), self->canvas);
}

guint32 *
nes_item_palette_get_index_colours (NesItemPalette *self)
{
  return self->index_color;
}

void 
nes_item_palette_set_index_colours (NesItemPalette *self,
                                   guint32                         *index)
{
  self->index_color = index;
  retro_canvas_set_index_colours (RETRO_CANVAS (self->canvas),
											index);
}

void 
nes_item_set_id (NesItemPalette *self,
		guint32 item_id)
{
	retro_canvas_set_item_id (RETRO_CANVAS (self->canvas), item_id);
}

void 
nes_item_palette_show_hex (NesItemPalette *self, int show)
{
	retro_canvas_set_show_hex (RETRO_CANVAS (self->canvas), show);

	gtk_widget_queue_draw (GTK_WIDGET (self->canvas));
}

void 
nes_item_palette_set_colours (NesItemPalette *self, guint32 count)
{
  retro_canvas_set_colours (RETRO_CANVAS (self->canvas), count);
}

void 
nes_item_palette_redraw (NesItemPalette *self)
{
	gtk_widget_queue_draw (GTK_WIDGET (self->canvas));
}

GtkWidget *
nes_item_palette_get_radio (NesItemPalette *self)
{
  return self->radio_btn;
}

guint32 *
nes_item_palette_get_colour_index (NesItemPalette *self)
{
  return self->index_color;
}

void 
nes_item_palette_get_color_from_index (NesItemPalette *self)
{
	//guint32 *colours = global_type_palette_get_cur_ptr_palette (0);
}
