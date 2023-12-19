/* nes-setup-palette.c
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

#include "nes-setup-palette.h"
#include "retro-canvas.h"
#include "global-functions.h"

struct _NesSetupPalette
{
  GtkWindow  parent_instance;
	GtkWidget  *grid;
	GtkWidget  *pal[16];
	GtkWidget  *col[16];
};

G_DEFINE_FINAL_TYPE (NesSetupPalette, nes_setup_palette, GTK_TYPE_WINDOW)

static void
nes_setup_palette_class_init (NesSetupPaletteClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

RetroCanvas *
nes_setup_palette_get_palette (NesSetupPalette *self, guint32 indx)
{
	return RETRO_CANVAS (self->col[indx]);
}

static void
nes_setup_palette_init (NesSetupPalette *self)
{
	for (guint32 i = 0; i < 16; i++) {
		self->pal[i] = g_object_new (RETRO_TYPE_CANVAS,
				NULL);

		CanvasSettings cs;
		cs.type_canvas   = TYPE_CANVAS_COLOUR_GRID;
		cs.canvas_width  = 16 * 16;
		cs.canvas_height = 16 * 4;
		cs.palette_type  = global_type_palette_get_cur_palette ();
		cs.width_rect    =  16;
		cs.height_rect   =  16;
		cs.scale         =   1;
		cs.count_x       =  16;
		cs.count_y       =   4;
		cs.left_top      = TRUE;

		g_object_set (self->pal[i], "settings", &cs, NULL);
  	retro_canvas_set_type_palette (
    	RETRO_CANVAS (self->pal[i]),
    	global_type_palette_get_cur_palette (),
    	64);

		retro_canvas_set_colours (RETRO_CANVAS (self->pal[i]), 64);
		guint32 *index_color = g_malloc0 (sizeof (guint32) * 64);
		for (int col = 0; col < 64; col++) {
			index_color[col] = col;
		}
		retro_canvas_set_index_colours (RETRO_CANVAS (self->pal[i]), index_color);

		self->col[i] = g_object_new (RETRO_TYPE_CANVAS,
				NULL);

		cs.type_canvas   = TYPE_CANVAS_COLOUR_PALETTE;
		cs.canvas_width  = 128 * 1;
		cs.canvas_height = 16;
		cs.palette_type  = global_type_palette_get_cur_palette ();
		cs.width_rect    = 128;
		cs.height_rect   =  16;
		cs.scale         =   1;
		cs.count_x       =   1;
		cs.count_y       =   1;
		cs.left_top      = TRUE;

		g_object_set (self->col[i], "settings", &cs, NULL);

		gtk_widget_set_size_request (GTK_WIDGET (self->pal[i]), 16 * 16, 16 * 4);
		gtk_widget_set_size_request (GTK_WIDGET (self->col[i]), 128 * 1, 16 * 1);
		
		retro_canvas_set_xy_click (RETRO_CANVAS (self->pal[i]));
		retro_canvas_set_child_color (RETRO_CANVAS (self->pal[i]),
				RETRO_CANVAS (self->col[i]));
	}

	self->grid = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (self->grid),
			32);
	gtk_grid_set_row_spacing (GTK_GRID (self->grid),
			32);

	guint32 index_widget = 0;
	for (guint32 y = 0; y < 4; y++) {
		for (guint32 x = 0; x < 4; x++) {
			gtk_grid_attach (GTK_GRID (self->grid),
					self->pal[index_widget],
					x,
					y * 2 + 0,
					1,
					1);
			gtk_grid_attach (GTK_GRID (self->grid),
					self->col[index_widget],
					x,
					y * 2 + 1,
					1,
					1);
			index_widget++;

		}
	}

	gtk_window_set_child (GTK_WINDOW (self), self->grid);

	for (guint32 i = 0; i < 4; i++) {
		gtk_widget_queue_draw (GTK_WIDGET (self->pal[i]));
		gtk_widget_queue_draw (GTK_WIDGET (self->col[i]));
	}

	gtk_window_set_title (GTK_WINDOW (self), "Setup Palette");
}

void 
nes_setup_palette_draw (NesSetupPalette *self)
{
	for (guint32 i = 0; i < 4; i++) {
		gtk_widget_queue_draw (GTK_WIDGET (self->pal[i]));
		gtk_widget_queue_draw (GTK_WIDGET (self->col[i]));
	}
}
