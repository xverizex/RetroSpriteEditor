/* nes-screen-background.c
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

#include "nes-screen-background.h"
#include "retro-canvas.h"
#include "global-functions.h"

struct _NesScreenBackground
{
  GtkWindow  parent_instance;

	RetroCanvas *background;
	RetroCanvas *screen;
	GtkWidget   *box_main;
};

G_DEFINE_FINAL_TYPE (NesScreenBackground, nes_screen_background, GTK_TYPE_WINDOW)

static void
nes_screen_background_class_init (NesScreenBackgroundClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
nes_screen_background_init (NesScreenBackground *self)
{
	gtk_window_set_hide_on_close (GTK_WINDOW (self), TRUE);

	self->background = g_object_new (RETRO_TYPE_CANVAS,
			NULL);
	self->screen = g_object_new (RETRO_TYPE_CANVAS,
			NULL);

	guint32 *idx_colour = global_nes_palette_get_memory_index (0);
	g_print ("idx_colour: %p\n", idx_colour);
	retro_canvas_set_index_colours (RETRO_CANVAS (self->background), idx_colour);
	retro_canvas_set_index_colours (RETRO_CANVAS (self->screen), idx_colour);

	GtkWidget *scroll_background = gtk_scrolled_window_new ();
	GtkWidget *scroll_screen = gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (scroll_background, TRUE);
	gtk_widget_set_vexpand (scroll_background, TRUE);
	gtk_widget_set_hexpand (scroll_screen, TRUE);
	gtk_widget_set_vexpand (scroll_screen, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll_background),
			GTK_WIDGET (self->background));
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll_screen),
			GTK_WIDGET (self->screen));

	gtk_widget_set_size_request (GTK_WIDGET (self->background), 128, 128);
	gtk_widget_set_size_request (GTK_WIDGET (self->screen), 128, 128);

	CanvasSettings back_cs;
	back_cs.type_canvas = TYPE_CANVAS_TILESET;
	back_cs.canvas_width = 128;
	back_cs.canvas_height = 128;
	back_cs.palette_type = global_type_palette_get_cur_platform ();
	back_cs.width_rect = 8;
	back_cs.height_rect = 8;
	back_cs.scale = 3;
	back_cs.count_x = 1;
	back_cs.count_y = 1;
	back_cs.left_top = FALSE;
	g_object_set (self->background, "settings", &back_cs, NULL);

	CanvasSettings screen_cs;
	screen_cs.type_canvas = TYPE_CANVAS_TILESET;
	screen_cs.canvas_width = 128; // 256
	screen_cs.canvas_height = 128; // 240
	screen_cs.palette_type = global_type_palette_get_cur_platform ();
	screen_cs.width_rect = 8;
	screen_cs.height_rect = 8;
	screen_cs.scale = 3;
	screen_cs.count_x = 1;
	screen_cs.count_y = 1;
	screen_cs.left_top = FALSE;
	g_object_set (self->screen, "settings", &screen_cs, NULL);

	self->box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (self->box_main), scroll_background);
	gtk_box_append (GTK_BOX (self->box_main), scroll_screen);

	gtk_window_set_child (GTK_WINDOW (self), self->box_main);


	gtk_window_set_default_size (GTK_WINDOW (self), 1280, 720);
}
