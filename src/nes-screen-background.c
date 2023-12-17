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
#include "nes-frame-megatile.h"
#include "retro-canvas.h"
#include "global-functions.h"
#include "tool-copy-tile.h"
#include "tool-clear-tile.h"
#include "project.h"

struct _NesScreenBackground
{
  GtkWindow  parent_instance;

	RetroCanvas *background;
	RetroCanvas *screen;
	GtkWidget   *box_main;
	GtkWidget   *frame_megatile[4];
	GtkWidget   *grid_megatiles;
	GtkWidget   *frame_grid_megatile;
	GtkWidget   *box_info;
	GtkWidget   *frame_tools;
	GtkWidget   *grid_tools;
	GtkWidget   *tool_copy;
	GtkWidget   *tool_megatile;
	GtkWidget   *tool_clear;
	GtkWidget   *btn_export;
};

G_DEFINE_FINAL_TYPE (NesScreenBackground, nes_screen_background, GTK_TYPE_WINDOW)

static void
nes_screen_background_class_init (NesScreenBackgroundClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
toggled_tool_clear (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_clear));

	if (is) {
		retro_canvas_set_tool (RETRO_CANVAS (self->background), 0);
		retro_canvas_set_tool (RETRO_CANVAS (self->screen), INDX_TOOL_CLEAR_TILE);
	}
}

static void
toggled_tool_megatile (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_copy));

	if (is) {
		retro_canvas_set_tool (RETRO_CANVAS (self->background), 0);
		retro_canvas_set_tool (RETRO_CANVAS (self->screen), INDX_TOOL_NES_MEGATILE);
	}
}

static void
toggled_tool_copy (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_copy));

	if (is) {
		retro_canvas_set_tool (RETRO_CANVAS (self->background), type_tool);
		retro_canvas_set_tool (RETRO_CANVAS (self->screen), INDX_TOOL_COPY_TILE_DST);
	}
}

static NesScreenBackground *global;

GtkWidget *
nes_screen_background_get_frame_megatile (guint32 indx)
{
	return global->frame_megatile[indx];
}

static void
export_click (GtkButton *btn, gpointer user_data)
{
	project_nes_export_screen ();
}

static void
nes_screen_background_init (NesScreenBackground *self)
{
	global = self;
	gtk_window_set_hide_on_close (GTK_WINDOW (self), TRUE);

	self->background = g_object_new (RETRO_TYPE_CANVAS,
			NULL);
	self->screen = g_object_new (RETRO_TYPE_CANVAS,
			NULL);

	self->box_info = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	self->grid_megatiles = gtk_grid_new ();

	guint32 *idx_colour = global_nes_palette_get_memory_index (0);
	retro_canvas_set_index_colours (RETRO_CANVAS (self->background), idx_colour);
	retro_canvas_set_index_colours (RETRO_CANVAS (self->screen), idx_colour);

	retro_canvas_nes_set_screen (RETRO_CANVAS (self->screen));

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

	gtk_widget_set_size_request (GTK_WIDGET (scroll_background), 128, 128);
	gtk_widget_set_size_request (GTK_WIDGET (scroll_screen), 256, 240);

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
	screen_cs.type_canvas = TYPE_CANVAS_SCREEN_BACKGROUND;
	screen_cs.canvas_width = 256;
	screen_cs.canvas_height = 240;
	screen_cs.palette_type = global_type_palette_get_cur_platform ();
	screen_cs.width_rect = 8;
	screen_cs.height_rect = 8;
	screen_cs.scale = 2;
	screen_cs.count_x = 1;
	screen_cs.count_y = 1;
	screen_cs.left_top = FALSE;
	g_object_set (self->screen, "settings", &screen_cs, NULL);

	self->box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (self->box_main), scroll_background);
	gtk_box_append (GTK_BOX (self->box_main), scroll_screen);

	char *labels_frame[] = {
		"Left Top",
		"Right Top",
		"Left Bottom",
		"Right Bottom"
	};

	int x = 0;
	int y = 0;
	for (int i = 0; i < 4; i++) {
		self->frame_megatile[i] = g_object_new (NES_TYPE_FRAME_MEGATILE, "label", labels_frame[i], NULL);
		gtk_grid_attach (GTK_GRID (self->grid_megatiles),
			self->frame_megatile[i],
			x,
			y,
			1,
			1);
		x++;
		if (x == 2) {
			x = 0;
			y++;
		}
	}

	self->frame_grid_megatile = g_object_new (GTK_TYPE_FRAME, "label", "Megatile", NULL);
	gtk_frame_set_child (GTK_FRAME (self->frame_grid_megatile), self->grid_megatiles);

	gtk_box_append (GTK_BOX (self->box_main), self->box_info);
	gtk_box_append (GTK_BOX (self->box_info), self->frame_grid_megatile);

	gtk_window_set_child (GTK_WINDOW (self), self->box_main);

	retro_canvas_shut_on_events_nes_screen (RETRO_CANVAS (self->background));
	retro_canvas_shut_on_events_nes_screen (RETRO_CANVAS (self->screen));

	retro_canvas_set_copy (RETRO_CANVAS (self->background), 1);


	gtk_window_set_default_size (GTK_WINDOW (self), 1280, 720);

	self->frame_tools = g_object_new (GTK_TYPE_FRAME, "label", "Tools", NULL);
	self->grid_tools = gtk_grid_new ();

	self->tool_copy = g_object_new (TOOL_TYPE_COPY_TILE, "label", "copy", NULL);
	self->tool_megatile = g_object_new (TOOL_TYPE_COPY_TILE, "label", "megatile", NULL);
	self->tool_clear = g_object_new (TOOL_TYPE_CLEAR_TILE, "label", "clear", NULL);

	gtk_grid_attach (GTK_GRID (self->grid_tools),
			self->tool_copy,
			0, 0,
			1, 1);

	gtk_grid_attach (GTK_GRID (self->grid_tools),
			self->tool_megatile,
			1, 0,
			1, 1);
	
	gtk_grid_attach (GTK_GRID (self->grid_tools),
			self->tool_clear,
			2, 0,
			1, 1);

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_copy),
			GTK_TOGGLE_BUTTON (self->tool_megatile));

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_megatile),
			GTK_TOGGLE_BUTTON (self->tool_clear));

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_copy),
			GTK_TOGGLE_BUTTON (self->tool_clear));

	gtk_frame_set_child (GTK_FRAME (self->frame_tools), self->grid_tools);
	gtk_box_append (GTK_BOX (self->box_info), self->frame_tools);

	g_signal_connect (self->tool_copy, "toggled", G_CALLBACK (toggled_tool_copy), self);
	g_signal_connect (self->tool_megatile, "toggled", G_CALLBACK (toggled_tool_megatile), self);
	g_signal_connect (self->tool_clear, "toggled", G_CALLBACK (toggled_tool_clear), self);

	self->btn_export = gtk_button_new_with_label ("EXPORT");
	gtk_box_append (GTK_BOX (self->box_info), self->btn_export);

	g_signal_connect (self->btn_export, "clicked", G_CALLBACK (export_click), self);
}
