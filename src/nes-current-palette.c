/* nes-current-palette.c
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

#include "nes-current-palette.h"
#include "nes-item-palette.h"
#include "nes-list-palettes.h"
#include "retro-canvas.h"

struct _NesCurrentPalette
{
  GtkFrame  parent_instance;

  GtkWidget *palette;
  GtkWidget *check_hex;
  GtkWidget *box_main;
  guint32 *colours;
  guint32 *index_colour;
};


#define DEFAULT_COLOUR_CANVAS_WIDTH             128
#define DEFAULT_COLOUR_CANVAS_HEIGHT             32
#define DEFAULT_COLOUR_CANVAS_SCALE               1
#define DEFAULT_COLOUR_PALETTE_TYPE               0

static NesCurrentPalette *global_self;

G_DEFINE_FINAL_TYPE (NesCurrentPalette, nes_current_palette, GTK_TYPE_FRAME)

static void
nes_current_palette_class_init (NesCurrentPaletteClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

void
nes_current_palette_redraw ()
{
  gtk_widget_queue_draw (GTK_WIDGET (global_self->palette));
}

static void
show_hex (GtkCheckButton *btn,
         gpointer        user_data)
{
  NesCurrentPalette *self = NES_CURRENT_PALETTE (user_data);

	int show = gtk_check_button_get_active (GTK_CHECK_BUTTON (self->check_hex));

  retro_canvas_set_show_hex (RETRO_CANVAS (self->palette), show);

	GtkWidget **item = nes_list_palette_get_items ();

	for (guint32 i = 0; i < 4; i++) {
		nes_item_palette_show_hex (NES_ITEM_PALETTE (item[i]), show);
	}

  gtk_widget_queue_draw (GTK_WIDGET (self->palette));
}

static void
nes_current_palette_init (NesCurrentPalette *self)
{
  global_self = self;

  self->colours = NULL;

  self->box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);

  self->check_hex = gtk_check_button_new_with_label ("HEX");

  self->palette = g_object_new (RETRO_TYPE_CANVAS,
                               NULL);

  retro_canvas_shut_on_event_click (RETRO_CANVAS (self->palette));

  retro_canvas_set_show_hex (RETRO_CANVAS (self->palette), FALSE);

  gtk_widget_set_size_request (self->palette, DEFAULT_COLOUR_CANVAS_WIDTH, DEFAULT_COLOUR_CANVAS_HEIGHT);
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
  g_object_set (self->palette, "settings", &cs_palette, NULL);

  gtk_frame_set_child (GTK_FRAME (self), self->box_main);

  gtk_box_append (GTK_BOX (self->box_main), self->palette);
  gtk_box_append (GTK_BOX (self->box_main), self->check_hex);


  gtk_widget_set_hexpand_set (GTK_WIDGET (self->palette), TRUE);
  gtk_widget_set_hexpand_set (GTK_WIDGET (self->check_hex), TRUE);

  gtk_widget_set_halign (GTK_WIDGET (self->check_hex), GTK_ALIGN_END);

  g_signal_connect (self->check_hex, "toggled",
                    G_CALLBACK (show_hex),
                    self);
}

void
nes_current_palette (NesCurrentPalette *self,
                     guint32                            *colours,
                     guint32                            *index_colour)
{
  self->colours = colours;
  self->index_colour =index_colour;

  retro_canvas_set_colours (RETRO_CANVAS (self->palette), 4);
  retro_canvas_set_index_colours (RETRO_CANVAS (self->palette), index_colour);

  RetroCanvas *dr_canvas = retro_canvas_get_drawing_canvas ();

  retro_canvas_set_colours (RETRO_CANVAS (dr_canvas), 4);
  retro_canvas_set_index_colours (RETRO_CANVAS (dr_canvas), index_colour);

  RetroCanvas *tileset = retro_canvas_get_tileset ();

  retro_canvas_set_colours (RETRO_CANVAS (tileset), 4);
  retro_canvas_set_index_colours (RETRO_CANVAS (tileset), index_colour);

  gtk_widget_queue_draw (self->palette);
  gtk_widget_queue_draw (GTK_WIDGET (dr_canvas));
  gtk_widget_queue_draw (GTK_WIDGET (tileset));
}

NesCurrentPalette *
nes_current_get_widget (void)
{
  return global_self;
}
