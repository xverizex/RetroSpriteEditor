/* nes-list-palettes.c
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

#include "nes-list-palettes.h"
#include "nes-item-palette.h"
#include "nes-current-palette.h"
#include "nes-setup-palette.h"
#include "nes-palette.h"
#include "retro-canvas.h"
#include "global-functions.h"

struct _NesListPalette
{
  GtkFrame  parent_instance;
  GtkWidget *list_palette;
  GtkWidget *btn_palette_setup;
  GtkWidget *box_main;
	GtkWidget *setup_palette_window;
  GtkWidget *items[4];
	NesBanks   banks;
};

static NesListPalette *global;

G_DEFINE_FINAL_TYPE (NesListPalette, nes_list_palette, GTK_TYPE_FRAME)

static void
nes_list_palette_class_init (NesListPaletteClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

GtkWidget **
nes_list_palette_get_items ()
{
	return global->items;
}

static void
select_palette (GtkCheckButton *btn,
         gpointer        user_data)
{

  NesItemPalette *self = NES_ITEM_PALETTE (user_data);

  NesCurrentPalette *cur_palette = nes_current_get_widget ();

  guint32 *colour = global_type_palette_get_cur_ptr_palette (0);
  guint32 *index_color = nes_item_palette_get_index_colours (self);

  nes_current_palette (cur_palette, colour, index_color);
}

static void
palette_setup (
  GtkButton* btn,
  gpointer user_data
	)
{
	NesListPalette *self = NES_LIST_PALETTE (user_data);
	gtk_window_present (GTK_WINDOW (self->setup_palette_window));
	nes_setup_palette_draw (NES_SETUP_PALETTE (self->setup_palette_window));
}

NesBanks*
nes_list_palette_get_bank (void)
{
	return &global->banks;
}

void
nes_list_palette_set_bank (guint32 index)
{
	for (guint i = 0; i < 4; i++) {
		guint32 *indx = nes_item_palette_get_index_colours (NES_ITEM_PALETTE (global->items[i]));
		*(indx + 0) = global->banks.bank[index][i][0];
		*(indx + 1) = global->banks.bank[index][i][1];
		*(indx + 2) = global->banks.bank[index][i][2];
		*(indx + 3) = global->banks.bank[index][i][3];
	}
	nes_list_palette_redraw ();
	nes_current_palette_redraw ();
	nes_palette_redraw ();
	retro_drawing_canvas_redraw ();
}

void
nes_list_palette_redraw (void)
{
	for (guint i = 0; i < 4; i++) {
		nes_item_palette_redraw (NES_ITEM_PALETTE (global->items[i]));
	}
}

static void
nes_list_palette_init (NesListPalette *self)
{
	global = self;
  self->btn_palette_setup = gtk_button_new_with_label ("PALETTE SETUP");
	g_signal_connect (self->btn_palette_setup, "clicked", G_CALLBACK (palette_setup), self);

	self->setup_palette_window = g_object_new (NES_TYPE_SETUP_PALETTE,
			NULL);

	gtk_window_set_hide_on_close (GTK_WINDOW (self->setup_palette_window), TRUE);

  self->list_palette = gtk_grid_new ();

  GtkWidget *radiobuttons[4] = {0, };
  GtkWidget *items[4];

  guint32 index = 0;
  guint32 i = 0;
	guint32 nindx = 0;
	guint32 nmax = 4;
  for (guint32 y = 0; y < 2; y++) {
    for (guint32 x = 0; x < 2; x++) {
      GtkWidget *item = g_object_new (NES_TYPE_ITEM_PALETTE,
                                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                                      NULL);

			self->items[i] = item;
			nes_item_set_id (NES_ITEM_PALETTE (item), i);

      radiobuttons[i] = nes_item_palette_get_radio (NES_ITEM_PALETTE (item));
      items[i] = item;

      gtk_grid_attach (GTK_GRID (self->list_palette),
                       item,
                       x,
                       y,
                       1,
                       1);

      guint32 *pcolours = global_type_palette_get_cur_ptr_palette (0);
      guint32 *index_colours = g_malloc0 (sizeof (guint32) * 4);
      *(index_colours + 0) = index + 0;
      *(index_colours + 1) = index + 1;
      *(index_colours + 2) = index + 2;
      *(index_colours + 3) = index + 3;

			for (guint32 in = 0; in < 2; in++) {
				self->banks.bank[in][i][0] = index + 0;
				self->banks.bank[in][i][1] = index + 1;
				self->banks.bank[in][i][2] = index + 2;
				self->banks.bank[in][i][3] = index + 3;
			}

      i++;

      nes_item_palette_set_index_colours (NES_ITEM_PALETTE (item), index_colours);
      nes_item_palette_set_colours (NES_ITEM_PALETTE (item), pcolours, 4);
			for (; nindx < nmax; nindx++) {
				RetroCanvas *c = nes_setup_palette_get_palette (NES_SETUP_PALETTE (self->setup_palette_window), nindx);
				retro_canvas_set_index_colours (RETRO_CANVAS (c), (index_colours + nindx % 4));
				retro_canvas_set_colours (RETRO_CANVAS (c), pcolours, 1);
			}
			nmax += 4;
      index += 16;
    }
  }

  self->box_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_box_append (GTK_BOX (self->box_main) , self->btn_palette_setup);
  gtk_box_append (GTK_BOX (self->box_main), self->list_palette);

  gtk_frame_set_child (GTK_FRAME (self), self->box_main);

  gtk_check_button_set_group (GTK_CHECK_BUTTON (radiobuttons[1]), GTK_CHECK_BUTTON (radiobuttons[0]));
  gtk_check_button_set_group (GTK_CHECK_BUTTON (radiobuttons[2]), GTK_CHECK_BUTTON (radiobuttons[0]));
  gtk_check_button_set_group (GTK_CHECK_BUTTON (radiobuttons[3]), GTK_CHECK_BUTTON (radiobuttons[0]));

  for (i = 0; i < 4; i++) {
    g_signal_connect (radiobuttons[i], "toggled",
                      G_CALLBACK (select_palette),
                      items[i]);
  }

  gtk_check_button_set_active (GTK_CHECK_BUTTON (radiobuttons[0]), TRUE);
}
