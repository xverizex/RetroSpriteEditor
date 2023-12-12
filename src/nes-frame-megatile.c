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

struct _NesFrameMegatile
{
  GtkFrame  parent_instance;

	GtkWidget *box_main;
};

G_DEFINE_FINAL_TYPE (NesFrameMegatile, nes_frame_megatile, GTK_TYPE_FRAME)

static void
nes_frame_megatile_class_init (NesFrameMegatileClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
nes_frame_megatile_init (NesFrameMegatile *self)
{
	self->box_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);

	gtk_frame_set_child (GTK_FRAME (self), self->box_main);
}
