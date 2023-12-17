/* nes-frame-megatile.c
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

#include "nes-frame-megatile.h"
#include "nes-list-palettes.h"
#include "nes-item-palette.h"
#include "retro-canvas.h"
#include "global-functions.h"

struct _NesFrameMegatile
{
  GtkFrame  parent_instance;

	guint32 id;
	GtkWidget *box_main;
	GtkWidget *canvas;
	RetroCanvas *cnvs[4];
	GtkWidget *menu;
};

G_DEFINE_FINAL_TYPE (NesFrameMegatile, nes_frame_megatile, GTK_TYPE_FRAME)

static void
nes_frame_megatile_class_init (NesFrameMegatileClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
canvas_activated_row (GtkListBox *box,
		GtkListBoxRow *row,
		gpointer user_data)
{
	NesFrameMegatile *self = NES_FRAME_MEGATILE	(user_data);
	int index = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW (row));
	guint32 *index_color = global_nes_palette_get_memory_index (index);

	retro_canvas_set_index_colours (RETRO_CANVAS (self->canvas), index_color);
	gtk_widget_queue_draw (GTK_WIDGET (self->canvas));

	int indx = global_nes_get_cur_screen ();
	GtkWidget *screen = global_get_screen (indx);
	guint8 *megatile = retro_canvas_nes_get_megatile_by_block (RETRO_CANVAS (screen));

	guint8 m[] = {
		0x03,
		0x0c,
		0x30,
		0xc0,
	};

	guint8 and[] = {
		0xfc,
		0xf3,
		0xcf,
		0x3f
	};

	int offset = 6;
	for (int i = 0; i < 4; i++) {
		if (i == self->id) {
			guint8 pal = index << offset;
			switch (self->id) {
				case 0:
					*megatile &= 0x3f;
					break;
				case 1:
					*megatile &= 0xcf;
					break;
				case 2:
					*megatile &= 0xf3;
					break;
				case 3:
					*megatile &= 0xfc;
					break;
			}
			*megatile |= pal;
			break;
		}
		offset -= 2;
	}

	gtk_widget_queue_draw (GTK_WIDGET (screen));
}

GtkWidget *
nes_frame_megatile_get_canvas (NesFrameMegatile *self)
{
	return self->canvas;
}

static void
nes_frame_megatile_init (NesFrameMegatile *self)
{
	static int id = 0;
	self->id = id++;
	CanvasSettings cs_palette;
  cs_palette.type_canvas    = TYPE_CANVAS_COLOUR_PALETTE;
  cs_palette.canvas_width   = 32 * 4;
  cs_palette.canvas_height  = 32;
  cs_palette.palette_type   = 0;
  cs_palette.scale          = 1;
  cs_palette.width_rect     = 32;
  cs_palette.height_rect    = 32;
  cs_palette.count_x        = 4;
  cs_palette.count_y        = 1;
  cs_palette.left_top       = TRUE;

	self->box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);

	gtk_frame_set_child (GTK_FRAME (self), self->box_main);

	self->canvas = g_object_new (RETRO_TYPE_CANVAS, NULL);
	gtk_widget_set_size_request (self->canvas, 32 * 4, 32);
	retro_canvas_set_blank (RETRO_CANVAS (self->canvas), 0);
	guint32 *index_color = global_nes_palette_get_memory_index (0);
  g_object_set (self->canvas, "settings", &cs_palette, NULL);

	retro_canvas_set_index_colours (RETRO_CANVAS (self->canvas), index_color);
	retro_canvas_set_colours (RETRO_CANVAS (self->canvas), 4);

	GtkWidget *listbox = gtk_list_box_new ();
	self->menu = gtk_menu_button_new ();
	for (int i = 0; i < 4; i++) {
		RetroCanvas *cnvs = g_object_new (RETRO_TYPE_CANVAS, NULL);
		gtk_widget_set_size_request (GTK_WIDGET (cnvs), 32 * 4, 32);
		gtk_list_box_append (GTK_LIST_BOX (listbox), GTK_WIDGET (cnvs));

		index_color = global_nes_palette_get_memory_index (i);
		retro_canvas_set_index_colours (RETRO_CANVAS (cnvs), index_color);
		retro_canvas_set_colours (RETRO_CANVAS (cnvs), 4);
		retro_canvas_set_item_id (RETRO_CANVAS (cnvs), i);
  	g_object_set (cnvs, "settings", &cs_palette, NULL);

		self->cnvs[i] = cnvs;
	}

	g_signal_connect (listbox, "row-activated", G_CALLBACK (canvas_activated_row), self);

	GtkWidget *popover = gtk_popover_new ();
	gtk_popover_set_child (GTK_POPOVER (popover), listbox);
	gtk_menu_button_set_popover (GTK_MENU_BUTTON (self->menu), popover);

	gtk_box_append (GTK_BOX (self->box_main), self->canvas);
	gtk_box_append (GTK_BOX (self->box_main), self->menu);
}
