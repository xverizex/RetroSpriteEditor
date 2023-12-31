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
  GtkFrame  parent_instance;
	gint        count;

	guint32  		cur_tool_back;
	guint32     cur_tool_screen;
	RetroCanvas *background;
	RetroCanvas *screen;
	GtkWidget   **screens;
	GtkWidget   **scrolls;
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
	GtkWidget   *notebook_screen;
	GtkWidget   *box_notebook;
	GtkWidget   *btn_add_screen;
	GtkWidget   *btn_remove_screen;
	GtkWidget   *box_btn_screen;
	GtkWidget   *fulldump_btn;
	guint8			last_page;
	guint8			*is_dump;
};

G_DEFINE_FINAL_TYPE (NesScreenBackground, nes_screen_background, GTK_TYPE_FRAME)

static void
nes_screen_background_class_init (NesScreenBackgroundClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
set_tool_for_all_screen_by_own (NesScreenBackground *self)
{
	retro_canvas_set_tool (RETRO_CANVAS (self->background), self->cur_tool_back);
	for (int i = 0; i < self->count; i++) {
		retro_canvas_set_tool (RETRO_CANVAS (self->screens[i]), self->cur_tool_screen);
	}
}

static void
set_tool_for_all_screen (NesScreenBackground *self, guint32 type_tool, guint32 screen_tool)
{
	self->cur_tool_back = type_tool;
	self->cur_tool_screen = screen_tool;

	retro_canvas_set_tool (RETRO_CANVAS (self->background), type_tool);

	for (int i = 0; i < self->count; i++) {
		retro_canvas_set_tool (RETRO_CANVAS (self->screens[i]), screen_tool);
	}
}

static void
toggled_tool_clear (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_clear));

	if (is) {
		set_tool_for_all_screen (self, 0, INDX_TOOL_CLEAR_TILE);
	}
}

static void
toggled_tool_megatile (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_copy));

	if (is) {
		set_tool_for_all_screen (self, 0, INDX_TOOL_NES_MEGATILE);
	}
}


static void
toggled_tool_copy (GtkToggleButton *source, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	gboolean is = gtk_toggle_button_get_active (source);

	guint32 type_tool = tool_button_get_type_index (TOOL_BUTTON (self->tool_copy));

	if (is) {
		set_tool_for_all_screen (self, type_tool, INDX_TOOL_COPY_TILE_DST);
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
create_new_screen (NesScreenBackground *self)
{
	GtkWidget *screen = g_object_new (RETRO_TYPE_CANVAS,
			NULL);

	GtkWidget *scroll_screen = gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (scroll_screen, TRUE);
	gtk_widget_set_vexpand (scroll_screen, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll_screen),
			GTK_WIDGET (screen));

	gtk_widget_set_size_request (GTK_WIDGET (scroll_screen), 256, 240);

	guint32 *idx_colour = global_nes_palette_get_memory_index (0);
	retro_canvas_set_index_colours (RETRO_CANVAS (screen), idx_colour);

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
	g_object_set (screen, "settings", &screen_cs, NULL);

	retro_canvas_shut_on_events_nes_screen (RETRO_CANVAS (screen));

	gchar *title = g_strdup_printf ("screen_%d", self->count);
	GtkWidget *label = gtk_label_new (title);
	g_free (title);

	gtk_notebook_append_page (GTK_NOTEBOOK (self->notebook_screen),
			scroll_screen,
			label);

	GtkWidget **temp = NULL;
	int index = self->count++;
	temp = realloc (self->scrolls, sizeof (GtkWidget *) * self->count);
	if (temp) {
		self->scrolls = temp;
		self->scrolls[index] = scroll_screen;
	}

	temp = realloc (self->screens, sizeof (GtkWidget *) * self->count);
	if (temp) {
		self->screens = temp;
		self->screens[index] = screen;
	}
	guint8 *temp_i8 = NULL;
	temp_i8 = realloc (self->is_dump, sizeof (guint8) * self->count);
	if (temp_i8) {
		self->is_dump = temp_i8;
		self->is_dump[index] = 0;
	}

	set_tool_for_all_screen_by_own (self);
	global_set_screen_count (self->count);
	global_set_screens (self->screens);
	global_set_fulldump_button (self->is_dump);
}

static void
remove_current_screen (NesScreenBackground *self)
{
	int page = global_nes_get_cur_screen ();

	gtk_notebook_remove_page (GTK_NOTEBOOK (self->notebook_screen), page);

	if (page < self->count) {
		for (int i = page; i < (self->count - 1); i++) {
			self->scrolls[i + 0] = self->scrolls[i + 1];
			self->screens[i + 0] = self->screens[i + 1];
			self->is_dump[i + 0] = self->is_dump[i + 1];
		}
	}

	self->count--;
	if (self->count < 0) {
		self->count = 0;
	}

	global_set_screen_count (self->count);
	global_set_screens (self->screens);

	for (int i = 0; i < self->count; i++) {
		char label[512];
		snprintf (label, 512, "screen_%d", i);
		gtk_notebook_set_tab_label_text (GTK_NOTEBOOK (self->notebook_screen),
				self->scrolls[i],
				label);
	}
}

static void
remove_screen (GtkButton *btn, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);
	remove_current_screen (self);
}

static void
add_screen (GtkButton *btn, gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);
	create_new_screen (self);
}

void
nes_screen_background_create_tabs_screen ()
{
	int max = global_get_screen_count ();
	for (int i = 0; i < max; i++) {
		create_new_screen (global);
	}
}

static gboolean 
screen_select_page (GtkNotebook *note,
		gboolean object,
		gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);
	int page = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->notebook_screen));

	global_nes_set_cur_screen (page);

	return TRUE;
}

static void
screen_switch_page (GtkNotebook *note,
		GtkWidget *page,
		guint page_num,
		gpointer user_data)
{
	NesScreenBackground *self = NES_SCREEN_BACKGROUND (user_data);

	global_nes_set_cur_screen (page_num);

	if (self->is_dump) {
		self->is_dump[self->last_page] = gtk_check_button_get_active (GTK_CHECK_BUTTON (self->fulldump_btn));
		gtk_check_button_set_active (GTK_CHECK_BUTTON (self->fulldump_btn), self->is_dump[page_num]);
	}

	self->last_page = page_num;
}

static void
nes_screen_background_init (NesScreenBackground *self)
{
	global = self;

	self->last_page = 0;
	self->count = 0;
	self->screens = NULL;
	self->scrolls = NULL;
	self->is_dump = NULL;

	self->background = g_object_new (RETRO_TYPE_CANVAS,
			NULL);

	self->box_info = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	self->grid_megatiles = gtk_grid_new ();

	guint32 *idx_colour = global_nes_palette_get_memory_index (0);
	retro_canvas_set_index_colours (RETRO_CANVAS (self->background), idx_colour);

	retro_canvas_nes_set_screen (RETRO_CANVAS (self->screen));

	GtkWidget *scroll_background = gtk_scrolled_window_new ();
	gtk_widget_set_hexpand (scroll_background, TRUE);
	gtk_widget_set_vexpand (scroll_background, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroll_background),
			GTK_WIDGET (self->background));

	gtk_widget_set_size_request (GTK_WIDGET (scroll_background), 128, 128);

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


	self->box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (self->box_main), scroll_background);

	self->notebook_screen = gtk_notebook_new ();
	g_signal_connect (self->notebook_screen, "select-page", G_CALLBACK (screen_select_page), self);
	g_signal_connect (self->notebook_screen, "switch-page", G_CALLBACK (screen_switch_page), self);
	self->box_notebook = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	self->btn_add_screen = gtk_button_new_with_label ("ADD");
	self->btn_remove_screen = gtk_button_new_with_label ("REMOVE");
	self->box_btn_screen = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (self->box_btn_screen), self->btn_add_screen);
	gtk_box_append (GTK_BOX (self->box_btn_screen), self->btn_remove_screen);
	gtk_box_append (GTK_BOX (self->box_notebook), self->box_btn_screen);
	gtk_box_append (GTK_BOX (self->box_notebook), self->notebook_screen);


	gtk_widget_set_size_request (GTK_WIDGET (self->notebook_screen), 400, 400);
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (self->notebook_screen), TRUE);

	g_signal_connect (self->btn_add_screen, "clicked", G_CALLBACK (add_screen), self);
	g_signal_connect (self->btn_remove_screen, "clicked", G_CALLBACK (remove_screen), self);

	gtk_box_append (GTK_BOX (self->box_main), self->box_notebook);
	//gtk_box_append (GTK_BOX (self->box_main), scroll_screen);

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

	gtk_frame_set_child (GTK_FRAME (self), self->box_main);

	retro_canvas_shut_on_events_nes_screen (RETRO_CANVAS (self->background));

	retro_canvas_set_copy (RETRO_CANVAS (self->background), 1);

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

	self->fulldump_btn = g_object_new (GTK_TYPE_CHECK_BUTTON,
			"label", "Full dump screen",
			NULL);

	g_object_set (self->fulldump_btn, "has-tooltip", TRUE, NULL);
	gtk_widget_set_tooltip_text (GTK_WIDGET (self->fulldump_btn),
			"If you want the screen exported as a full screen bytes, "
			"then toggle button. Such screen will sequentially fill up "
			"the screen with null tile. If the button untangled, then "
			"it is saving the position and tile of each tile in the code."
			);

	gtk_box_append (GTK_BOX (self->box_info), self->fulldump_btn);

	g_signal_connect (self->btn_export, "clicked", G_CALLBACK (export_click), self);
}
