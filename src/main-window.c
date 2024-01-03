/* main-window.c
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

#include "main-window.h"
#include "retro-canvas.h"
#include "frame-platform.h"
#include "tool-pencil.h"
#include "tool-mov.h"
#include "tool-swap.h"
#include "nes-new-project.h"
#include "general-tools.h"
#include "project.h"
#include "global-functions.h"
#include "nes-palette.h"
#include "nes-list-palettes.h"
#include "nes-screen-background.h"
#include <libxml/parser.h>

#define DEFAULT_CANVAS_WIDTH             128
#define DEFAULT_CANVAS_HEIGHT            128
#define DEFAULT_CANVAS_SCALE               3
#define DEFAULT_PALETTE_TYPE               0
#define DEFAULT_WINDOW_WIDTH            1280
#define DEFAULT_WINDOW_HEIGHT            720
#define DEFAULT_SCROLL_AREA_WIDTH        512
#define DEFAULT_SCROLL_AREA_HEIGHT       512

struct _MainWindow
{
  AdwApplicationWindow  parent_instance;

  GtkWidget           *header_bar;
  GtkWidget           *general_layout_nes;
  GtkWidget           *screen_layout_nes;
  GtkWidget           *menu_button;
  GtkWidget           *vert_layout;
  GtkWidget           *palette;
  GtkWidget           *canvas;
  GtkWidget           *scroll_window_canvas;
  GtkWidget           *frame_tools;
  GtkWidget           *box_tools;
  GtkIconTheme        *icon_theme;
  GtkWidget           *tool_button_pencil;
  GtkWidget           *tool_button_mov;
  GtkWidget           *tool_button_swap;
	GtkWidget  					*new_project_nes_window;
	GtkWidget						*hbox_nes_title_buttons;
	GtkWidget						*btn_nes_pixel_drawing;
	GtkWidget						*btn_nes_screen;
	GtkWidget 					*window_screen;
	int  								first;
	gint32							last_platform;
	guint32							cur_platform;
	guint32							cur_palette;
	guint32							last_bank;
};

G_DEFINE_FINAL_TYPE (MainWindow, main_window, ADW_TYPE_APPLICATION_WINDOW)

static void
main_window_class_init (MainWindowClass *klass)
{
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
tool_toggled (GtkToggleButton *btn_base,
         gpointer         user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  ToolButton *btn = TOOL_BUTTON (btn_base);

  retro_canvas_set_tool (RETRO_CANVAS (self->canvas),
                   gtk_toggle_button_get_active (btn_base)? tool_button_get_type_index (btn): 0);

}

#if 0
static void
action_menu (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

}
#endif

static void
async_selected_import_chr (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	GtkFileDialog *dia = GTK_FILE_DIALOG (source_object);
	GFile *chr = gtk_file_dialog_open_finish (
			dia,
			res,
			NULL);

	if (!chr)
		return;

	char *c_chr = g_strdup (g_file_get_path (chr));

	nes_palette_clean_map ();
	project_import_nes_chr (c_chr);
}

static void
action_import_nes_chr (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	if (global_type_palette_get_cur_platform () == NO_PLATFORM)
		return;

	GtkFileDialog *dia = gtk_file_dialog_new ();
	gtk_file_dialog_open (dia,
			GTK_WINDOW (self),
			NULL,
			async_selected_import_chr,
			self);
}

void
main_window_set_cur_platform (MainWindow *self, guint32 platform)
{
	self->cur_platform = platform;
}

void
main_window_connect_widgets (MainWindow *self)
{
	if (!self->first) {
		self->first = 1;
		switch (self->cur_platform) {
			case PLATFORM_PALETTE_NES:
				global_type_palette_set_cur (PLATFORM_PALETTE_NES, NES_TYPE_PALETTE_2C02);
				self->last_platform = PLATFORM_PALETTE_NES;
				gtk_widget_set_visible (self->general_layout_nes, TRUE);
				gtk_widget_set_visible (self->hbox_nes_title_buttons, TRUE);
				gtk_widget_set_visible (self->screen_layout_nes, FALSE);
				break;
			default:
				break;
		}
		return;
	}

	switch (self->last_platform) {
		case PLATFORM_PALETTE_NES:
			gtk_widget_set_visible (self->general_layout_nes, FALSE);
			gtk_widget_set_visible (self->screen_layout_nes, FALSE);
			gtk_widget_set_visible (self->hbox_nes_title_buttons, FALSE);
			global_type_palette_set_cur (NO_PLATFORM, 0);
			break;
		default:
			break;
	}
	switch (self->cur_platform) {
		case PLATFORM_PALETTE_NES:
			global_type_palette_set_cur (PLATFORM_PALETTE_NES, NES_TYPE_PALETTE_2C02);
			self->last_platform = PLATFORM_PALETTE_NES;
			gtk_widget_set_visible (self->general_layout_nes, TRUE);
			gtk_widget_set_visible (self->screen_layout_nes, FALSE);
			gtk_widget_set_visible (self->hbox_nes_title_buttons, TRUE);
			break;
		default:
			break;
	}
}

static void
async_selected_project (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	GtkFileDialog *dia = GTK_FILE_DIALOG (source_object);
	GFile *project = gtk_file_dialog_open_finish (
			dia,
			res,
			NULL);

	if (!project)
		return;

	char *c_project = g_strdup (g_file_get_path (project));

	self->cur_platform = PLATFORM_PALETTE_NES;
	self->cur_palette = NES_TYPE_PALETTE_2C02;
	global_type_palette_set_cur (PLATFORM_PALETTE_NES, NES_TYPE_PALETTE_2C02);

	main_window_connect_widgets (self);
	nes_palette_clean_map ();
	project_open_nes (c_project);
	global_restructure_canvas ();
	nes_palette_restructure ();
}

static void
action_open_project_nes (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);
	GtkFileDialog *dia = gtk_file_dialog_new ();
	gtk_file_dialog_open (dia,
			GTK_WINDOW (self),
			NULL,
			async_selected_project,
			self);

}

static void
export_to_file (MainWindow *self)
{
	if (global_type_palette_get_cur_platform () == NO_PLATFORM)
		return;

	char *filepath = project_get_filepath_to_export ();
	if (!filepath)
		return;

	GFile *file = g_file_new_for_path (filepath);
	GFileOutputStream *out = g_file_replace (file, 
			NULL,
			FALSE,
			G_FILE_CREATE_REPLACE_DESTINATION,
			NULL,
			NULL);

	DataForOutput dt;
	global_get_data_for_output (&dt);

	guint8 *d = dt.data;
	g_output_stream_write (G_OUTPUT_STREAM (out), dt.data, dt.size, NULL, NULL);
	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);

	g_free (dt.data);
}

static void
action_save_project (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	if (global_type_palette_get_cur_platform () == NO_PLATFORM)
		return;

	export_to_file (self);
	project_save_nes ();
}

static void
nes_pixel_drawing_toggled (GtkCheckButton *btn, gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	global_set_cur_bank (NES_SPRITE);
	nes_list_palette_set_bank (NES_SPRITE);
	nes_palette_set_cur (NES_SPRITE);

	gtk_widget_set_visible (self->general_layout_nes, TRUE);
	gtk_widget_set_visible (self->screen_layout_nes, FALSE);
}

static void
nes_screen_toggled (GtkCheckButton *btn, gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);

	global_set_cur_bank (NES_BACKGROUND);
	nes_list_palette_set_bank (NES_BACKGROUND);
	nes_palette_set_cur (NES_BACKGROUND);

	gtk_widget_set_visible (self->general_layout_nes, FALSE);
	gtk_widget_set_visible (self->screen_layout_nes, TRUE);
}

void
main_window_create_nes_widgets (MainWindow *self)
{

	global_type_palette_set_cur (PLATFORM_PALETTE_NES, NES_TYPE_PALETTE_2C02);
	for (int i = 0; i < 4; i++) {
		global_nes_palette_alloc_memory_index (i, sizeof (guint32) * 4);
	}

	global_nes_set_height_alloc (2);
	global_nes_palette_alloc_maps ();
	global_nes_palette_init_map (0);
	global_nes_palette_init_map (1);

  self->canvas = g_object_new(RETRO_TYPE_CANVAS, NULL);
  retro_canvas_set_drawing_canvas (RETRO_CANVAS (self->canvas));


  self->scroll_window_canvas = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->scroll_window_canvas),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child (
    GTK_SCROLLED_WINDOW (self->scroll_window_canvas),
    self->canvas);

  self->palette = g_object_new (FRAME_TYPE_PLATFORM, NULL);

  self->frame_tools = gtk_frame_new ("Tools");


  gtk_box_append (GTK_BOX (self->general_layout_nes), self->frame_tools);
  gtk_box_append (GTK_BOX (self->general_layout_nes), self->scroll_window_canvas);
  gtk_box_append (GTK_BOX (self->general_layout_nes), self->palette);

  self->box_tools = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);

  gtk_frame_set_child (GTK_FRAME (self->frame_tools), self->box_tools);

  self->icon_theme = gtk_icon_theme_get_for_display (gdk_display_get_default ());
  gtk_icon_theme_add_resource_path (self->icon_theme,
                                     "/io/github/xverizex/RetroSpriteEditor/icons");

  gtk_widget_set_hexpand (self->scroll_window_canvas, TRUE);
  gtk_widget_set_vexpand (self->scroll_window_canvas, TRUE);

  g_object_set (self->palette, "platform", PLATFORM_NES, NULL);
  gtk_widget_set_halign (self->palette, GTK_ALIGN_END);
  gtk_widget_set_hexpand (self->palette, FALSE);
  gtk_widget_set_vexpand (self->palette, FALSE);
  gtk_widget_set_hexpand (self->canvas, TRUE);
  gtk_widget_set_vexpand (self->canvas, TRUE);
  gtk_widget_set_vexpand (self->frame_tools, TRUE);

	guint32 height_tile = global_get_height_by (0);
  CanvasSettings cs;
  cs.type_canvas    = TYPE_CANVAS_TILESET;
  cs.canvas_width   = DEFAULT_CANVAS_WIDTH;
  cs.canvas_height  = DEFAULT_CANVAS_HEIGHT;
  cs.palette_type   = global_type_palette_get_cur_platform ();
  cs.scale          = DEFAULT_CANVAS_SCALE;
  cs.width_rect    = 8;
  cs.height_rect   = height_tile;
  cs.left_top      = FALSE;
	cs.pos_height    = 0;
  g_object_set (self->canvas, "settings", &cs, NULL);

  retro_canvas_shut_on_events (RETRO_CANVAS (self->canvas));

#ifdef __MINGW32__
  self->tool_button_pencil = g_object_new (TOOL_TYPE_PENCIL,
                                        "has-frame", FALSE,
                                        NULL);

	GtkWidget *image = gtk_image_new_from_file ("image-tools/pencil.png");
	gtk_button_set_child (GTK_BUTTON (self->tool_button_pencil), image);
	gtk_widget_set_valign (image, GTK_ALIGN_CENTER);


	self->tool_button_mov = g_object_new (TOOL_TYPE_MOV,
			"has-frame", FALSE,
			NULL);

	image = gtk_image_new_from_file ("image-tools/mov.png");
	gtk_button_set_child (GTK_BUTTON (self->tool_button_mov), image);
	gtk_widget_set_valign (image, GTK_ALIGN_CENTER);

	self->tool_button_swap = g_object_new (TOOL_TYPE_SWAP,
			"has-frame", FALSE,
			NULL);

	image = gtk_image_new_from_file ("image-tools/swap.png");
	gtk_button_set_child (GTK_BUTTON (self->tool_button_swap), image);
	gtk_widget_set_valign (image, GTK_ALIGN_CENTER);

#else
  self->tool_button_pencil = g_object_new (TOOL_TYPE_PENCIL,
			"icon-name", "pencil",
			"has-frame", FALSE,
			NULL);

	self->tool_button_mov = g_object_new (TOOL_TYPE_MOV,
			"icon-name", "mov",
			"has-frame", FALSE,
			NULL);

	self->tool_button_swap = g_object_new (TOOL_TYPE_SWAP,
			"icon-name", "swap",
			"has-frame", FALSE,
			NULL);
#endif


  gtk_box_append (GTK_BOX (self->box_tools), self->tool_button_pencil);
  gtk_box_append (GTK_BOX (self->box_tools), self->tool_button_mov);
  gtk_box_append (GTK_BOX (self->box_tools), self->tool_button_swap);

  g_signal_connect (self->tool_button_pencil, "toggled", G_CALLBACK (tool_toggled),
                    self);

  g_signal_connect (self->tool_button_mov, "toggled", G_CALLBACK (tool_toggled), self);

  g_signal_connect (self->tool_button_swap, "toggled", G_CALLBACK (tool_toggled), self);

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_button_pencil),
			GTK_TOGGLE_BUTTON (self->tool_button_mov));

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_button_swap), GTK_TOGGLE_BUTTON (self->tool_button_mov));

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->tool_button_pencil), GTK_TOGGLE_BUTTON (self->tool_button_swap));

	gtk_box_append (GTK_BOX (self->vert_layout), self->general_layout_nes);
	gtk_widget_set_visible (GTK_WIDGET (self->general_layout_nes), FALSE);

	g_object_set (self->tool_button_pencil, "has-tooltip", TRUE, NULL);
	g_object_set (self->tool_button_mov, "has-tooltip", TRUE, NULL);
	g_object_set (self->tool_button_swap, "has-tooltip", TRUE, NULL);

	gtk_widget_set_tooltip_markup (GTK_WIDGET (self->tool_button_pencil), 
			"<span weight=\"bold\">A pencil tool </span><span>- Draw pixels with it tool.</span>");

	gtk_widget_set_tooltip_markup (GTK_WIDGET (self->tool_button_mov), 
			"<span weight=\"bold\">A moving tool </span><span>- Moving tiles in the canvas.</span>");

	gtk_widget_set_tooltip_markup (GTK_WIDGET (self->tool_button_swap), 
			"<span weight=\"bold\">A swap tool </span><span>- Exchange tiles between each other.</span>");

	global_type_palette_set_cur (NO_PLATFORM, 0);

	self->hbox_nes_title_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	self->btn_nes_pixel_drawing = g_object_new (GTK_TYPE_TOGGLE_BUTTON, "label", "DRAWING", NULL);
	self->btn_nes_screen = g_object_new (GTK_TYPE_TOGGLE_BUTTON, "label", "SCREEN", NULL);

	gtk_box_append (GTK_BOX (self->hbox_nes_title_buttons), self->btn_nes_pixel_drawing);
	gtk_box_append (GTK_BOX (self->hbox_nes_title_buttons), self->btn_nes_screen);

	adw_header_bar_set_title_widget (ADW_HEADER_BAR (self->header_bar),
			self->hbox_nes_title_buttons);

	gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (self->btn_nes_pixel_drawing),
			GTK_TOGGLE_BUTTON (self->btn_nes_screen));

	g_signal_connect (self->btn_nes_pixel_drawing, "toggled", G_CALLBACK (nes_pixel_drawing_toggled), self);
	g_signal_connect (self->btn_nes_screen, "toggled", G_CALLBACK (nes_screen_toggled), self);

	self->window_screen = g_object_new (NES_TYPE_SCREEN_BACKGROUND, NULL);
	gtk_box_append (GTK_BOX (self->screen_layout_nes), self->window_screen);

	gtk_box_append (GTK_BOX (self->vert_layout), self->screen_layout_nes);
	gtk_widget_set_visible (GTK_WIDGET (self->screen_layout_nes), FALSE);
}

static void
action_new_project_nes (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);
	gtk_window_present (GTK_WINDOW (self->new_project_nes_window));
}

static void
action_export (GSimpleAction *simple,
		GVariant *parameter,
		gpointer user_data)
{
	MainWindow *self = MAIN_WINDOW (user_data);
	export_to_file (self);
}

static MainWindow *global;

MainWindow *
main_window_get (void)
{
	return global;
}

static void
main_window_init (MainWindow *self)
{
	global = self;
	self->first = 0;
	self->last_platform = -1;
	self->last_bank = NES_SPRITE;

	xmlInitParser ();

	self->menu_button = g_object_new (GTK_TYPE_MENU_BUTTON,
			"icon-name", "open-menu-symbolic",
			NULL);

	const GActionEntry entries[] = {
		{"export", 		action_export},
		{"new_project_nes", action_new_project_nes},
		{"save_project", action_save_project},
		{"open_project_nes", action_open_project_nes},
		{"import_nes_chr", action_import_nes_chr}
	};

	g_action_map_add_action_entries (G_ACTION_MAP (self), entries, G_N_ELEMENTS (entries), self);

	GMenu *menu_open = g_menu_new ();
	g_menu_append (menu_open, "Open Project NES", "win.open_project_nes");
	GMenu *menu_project = g_menu_new ();
	g_menu_append (menu_project, "New NES Project", "win.new_project_nes");
	GMenu *menu_import = g_menu_new ();
	g_menu_append (menu_import, "Import CHR", "win.import_nes_chr");

	GMenu *menu_root = g_menu_new ();
	g_menu_append_submenu (menu_root, "New Project", G_MENU_MODEL (menu_project));
	g_menu_append_submenu (menu_root, "Open", G_MENU_MODEL (menu_open));
	g_menu_append_submenu (menu_root, "Import", G_MENU_MODEL (menu_import));
	g_menu_append (menu_root, "Save", "win.save_project");
	g_menu_append (menu_root, "Export", "win.export");

	self->new_project_nes_window = g_object_new (NES_TYPE_NEW_PROJECT,
			NULL);

	gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->menu_button), G_MENU_MODEL (menu_root));

  self->header_bar = adw_header_bar_new ();
  adw_header_bar_set_decoration_layout (ADW_HEADER_BAR (self->header_bar), "icon:minimize,maximize,close");
  adw_header_bar_set_show_end_title_buttons (ADW_HEADER_BAR (self->header_bar), TRUE);
	adw_header_bar_pack_end (ADW_HEADER_BAR (self->header_bar), self->menu_button);

  self->vert_layout = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  self->general_layout_nes = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	self->screen_layout_nes = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), self->vert_layout);
  gtk_window_set_default_size (GTK_WINDOW (self), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

  gtk_box_append (GTK_BOX (self->vert_layout), self->header_bar);

	gtk_window_set_title (GTK_WINDOW (self), "Retro Sprite Editor");

	main_window_create_nes_widgets (self);
}
