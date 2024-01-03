/* nes-new-project.c
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

#include "nes-new-project.h"
#include "main-window.h"
#include "project.h"
#include "global-functions.h"
#include "retro-canvas.h"
#include "nes-palette.h"

struct _NesNewProject
{
  GtkWindow  parent_instance;

	GtkWidget		*vert_box;
	GtkWidget		*frame_location;
	GtkWidget		*entry_location;
	GtkWidget		*button_location;
	GtkWidget		*frame_tileset;
	GtkWidget		*dropdown_tileset;
	GtkWidget		*frame_project_name;
	GtkWidget		*entry_project_name;
	GtkWidget		*button_create_project;
	GtkWidget		*box_select_mode;
	GtkWidget		*frame_sprite_mode;
	GtkWidget		*frame_background_mode;
	GtkWidget		*popover_sprite_mode;
	GtkWidget		*popover_background_mode;
	GtkWidget		*list_box_sprite_mode;
	GtkWidget		*list_box_background_mode;
	GtkWidget		*label_sprite_mode;
	GtkWidget		*label_background_mode;
	GtkWidget		*menu_sprite_mode;
	GtkWidget		*menu_background_mode;
	guint32			index_sprite_mode;
	guint32			index_background_mode;
};

G_DEFINE_FINAL_TYPE (NesNewProject, nes_new_project, GTK_TYPE_WINDOW)

static const gchar *modes_str[] = {
	"8x8",
	"8x16"
};

static guint32 size_mode_str = sizeof (modes_str) / sizeof (void *);

static void
nes_new_project_class_init (NesNewProjectClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
async_selected_folder (GObject *source_object,
		GAsyncResult *res,
		gpointer user_data)
{
	GtkFileDialog *dia = GTK_FILE_DIALOG (source_object);
	GFile *folder_project = gtk_file_dialog_select_folder_finish (
			dia,
			res,
			NULL);
	if (!folder_project)
		return;

	NesNewProject *self = NES_NEW_PROJECT (user_data);

	GtkEntryBuffer *buf = gtk_entry_get_buffer (GTK_ENTRY (self->entry_location));
	char *folder = g_file_get_path (folder_project);
	gtk_entry_buffer_set_text (GTK_ENTRY_BUFFER (buf), folder, -1);
}

static void
click_location (GtkButton *btn, gpointer user_data)
{
	NesNewProject *self = NES_NEW_PROJECT (user_data);

	GtkFileDialog *dia = gtk_file_dialog_new ();
	gtk_file_dialog_select_folder (dia,
			GTK_WINDOW (self),
			NULL,
			async_selected_folder,
			self);

}

static void
click_create_project (GtkButton *btn, gpointer user_data)
{
	NesNewProject *self = NES_NEW_PROJECT (user_data);

	GtkEntryBuffer *buf_location = gtk_entry_get_buffer (GTK_ENTRY (self->entry_location));
	GtkEntryBuffer *buf_project_name = gtk_entry_get_buffer (GTK_ENTRY (self->entry_project_name));

	const char *location = gtk_entry_buffer_get_text (buf_location);
	const char *project_name = gtk_entry_buffer_get_text (buf_project_name);

	global_nes_set_height_by (NES_SPRITE, self->index_sprite_mode);
	global_nes_set_height_by (NES_BACKGROUND, self->index_background_mode);
	nes_palette_restructure ();

	project_set_folder_and_name (location, project_name);

	MainWindow *win = main_window_get ();
	main_window_set_cur_platform (MAIN_WINDOW (win), PLATFORM_PALETTE_NES);
	main_window_connect_widgets (win);
	global_restructure_canvas ();
	gtk_window_close (GTK_WINDOW (self));
}

static void
row_sprite_mode (GtkListBox *box,
		GtkListBoxRow		*row,
		gpointer 				user_data)
{
	NesNewProject *self = NES_NEW_PROJECT (user_data);
	self->index_sprite_mode = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW (row));
	gtk_label_set_text (GTK_LABEL (self->label_sprite_mode), modes_str[self->index_sprite_mode]);
	gtk_popover_popdown (GTK_POPOVER (self->popover_sprite_mode));
}

static void
row_background_mode (GtkListBox *box,
		GtkListBoxRow		*row,
		gpointer 				user_data)
{
	NesNewProject *self = NES_NEW_PROJECT (user_data);
	self->index_background_mode = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW (row));
	gtk_label_set_text (GTK_LABEL (self->label_background_mode), modes_str[self->index_sprite_mode]);
	gtk_popover_popdown (GTK_POPOVER (self->popover_background_mode));
}

static void
nes_new_project_init (NesNewProject *self)
{
	self->index_sprite_mode = 0;
	self->index_background_mode = 0;

	gtk_window_set_hide_on_close (GTK_WINDOW (self), TRUE);
	gtk_window_set_title (GTK_WINDOW (self), "New NES project");

	self->vert_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
	self->frame_location = g_object_new (GTK_TYPE_FRAME,
			"label", "Project Location", NULL);
	self->entry_location = gtk_entry_new ();
	self->button_location = gtk_button_new_with_label ("SET LOCATION");

	g_signal_connect (self->button_location, "clicked", G_CALLBACK (click_location), self);

	GtkWidget *box_location = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (box_location), self->entry_location);
	gtk_box_append (GTK_BOX (box_location), self->button_location);

	gtk_frame_set_child (GTK_FRAME (self->frame_location), box_location);

	gtk_box_append (GTK_BOX (self->vert_box), self->frame_location);

	gtk_window_set_child (GTK_WINDOW (self), self->vert_box);

	self->frame_tileset = g_object_new (GTK_TYPE_FRAME,
			"label", "Tileset map", NULL);

	self->frame_project_name = g_object_new (GTK_TYPE_FRAME,
			"label", "Project Name", NULL);
	self->entry_project_name = gtk_entry_new ();

	GtkWidget *box_project_name = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_append (GTK_BOX (box_project_name), self->entry_project_name);

	gtk_frame_set_child (GTK_FRAME (self->frame_project_name), box_project_name);

	gtk_box_append (GTK_BOX (self->vert_box), self->frame_project_name);

	self->button_create_project = gtk_button_new_with_label ("CREATE PROJECT");
	
	g_signal_connect (self->button_create_project, "clicked", G_CALLBACK (click_create_project), self);


	self->box_select_mode = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	self->frame_sprite_mode = g_object_new (GTK_TYPE_FRAME, "label", "Sprite mode", NULL);
	self->frame_background_mode = g_object_new (GTK_TYPE_FRAME, "label", "Background mode", NULL);
	self->popover_sprite_mode = gtk_popover_new ();
	self->popover_background_mode = gtk_popover_new ();
	self->list_box_sprite_mode = gtk_list_box_new ();
	self->list_box_background_mode = gtk_list_box_new ();
	self->label_sprite_mode = gtk_label_new (modes_str[0]);
	self->label_background_mode = gtk_label_new (modes_str[0]);

	GtkWidget *box_sprite_mode = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	GtkWidget *box_background_mode = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
	self->menu_sprite_mode = gtk_menu_button_new ();
	self->menu_background_mode = gtk_menu_button_new ();
	gtk_widget_set_halign (self->menu_background_mode, GTK_ALIGN_END);
	gtk_widget_set_hexpand (self->menu_background_mode, TRUE);
	gtk_widget_set_halign (self->menu_sprite_mode, GTK_ALIGN_END);
	gtk_widget_set_hexpand (self->menu_sprite_mode, TRUE);

	gtk_menu_button_set_popover (GTK_MENU_BUTTON (self->menu_sprite_mode), self->popover_sprite_mode);
	gtk_menu_button_set_popover (GTK_MENU_BUTTON (self->menu_background_mode), self->popover_background_mode);

	gtk_box_append (GTK_BOX (box_sprite_mode), self->label_sprite_mode);
	gtk_box_append (GTK_BOX (box_sprite_mode), self->menu_sprite_mode);

	gtk_box_append (GTK_BOX (box_background_mode), self->label_background_mode);
	gtk_box_append (GTK_BOX (box_background_mode), self->menu_background_mode);

	gtk_frame_set_child (GTK_FRAME (self->frame_sprite_mode), box_sprite_mode);
	gtk_frame_set_child (GTK_FRAME (self->frame_background_mode), box_background_mode);

	gtk_box_append (GTK_BOX (self->box_select_mode), self->frame_sprite_mode);
	gtk_box_append (GTK_BOX (self->box_select_mode), self->frame_background_mode);

	gtk_widget_hide (GTK_WIDGET (self->frame_background_mode));

	gtk_box_append (GTK_BOX (self->vert_box), self->box_select_mode);
	GtkWidget *label = NULL;
	for (guint32 i = 0; i < size_mode_str; i++) {
		label = gtk_label_new (modes_str[i]);
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_sprite_mode), label);
		label = gtk_label_new (modes_str[i]);
		gtk_list_box_append (GTK_LIST_BOX (self->list_box_background_mode), label);
	}

	gtk_popover_set_child (GTK_POPOVER (self->popover_sprite_mode), self->list_box_sprite_mode);
	gtk_popover_set_child (GTK_POPOVER (self->popover_background_mode), self->list_box_background_mode);

	gtk_box_append (GTK_BOX (self->vert_box), self->button_create_project);

	g_signal_connect (self->list_box_sprite_mode, "row-activated", G_CALLBACK (row_sprite_mode), self);
	g_signal_connect (self->list_box_background_mode, "row-activated", G_CALLBACK (row_background_mode), self);
}
