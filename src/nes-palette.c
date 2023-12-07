/* retrospriteeditor-nes-palette.c
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

#include "nes-palette.h"
#include "retro-canvas.h"
#include "nes-list-palettes.h"
#include "global-functions.h"
#include "nes-current-palette.h"

struct _NesPalette
{
  GtkBox  parent_instance;

  GtkWidget *tileset;
  GtkWidget *bank0;
  GtkWidget *bank1;
  GtkWidget *box_banks;
  GtkWidget *colours;
  GtkWidget *frame_colours;
  GtkWidget *frame_tileset;
  GtkWidget *frame_banks;
  GtkWidget *frame_list_items_palette;
  GtkWidget *frame_current_palette;

  guint32 cur_palette;
  guint32 count_sprite_x;
  guint32 count_sprite_y;
  guint32 count_back_x;
  guint32 count_back_y;
  guint32 cur_width_rect;
  guint32 cur_heigh_rect;

  NesTilePoint *map[N_NES];
};

G_DEFINE_FINAL_TYPE (NesPalette, nes_palette, GTK_TYPE_BOX)

static NesPalette *global_nes;

#define DEFAULT_CANVAS_WIDTH             128
#define DEFAULT_CANVAS_HEIGHT            128
#define DEFAULT_CANVAS_SCALE               2
#define DEFAULT_PALETTE_TYPE               0

#define DEFAULT_COLOUR_CANVAS_WIDTH             256
#define DEFAULT_COLOUR_CANVAS_HEIGHT             64
#define DEFAULT_COLOUR_CANVAS_SCALE               1
#define DEFAULT_COLOUR_PALETTE_TYPE               0

void
nes_palette_redraw (void)
{
	gtk_widget_queue_draw (GTK_WIDGET (global_nes->tileset));
}

static void
nes_palette_class_init (NesPaletteClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
bank_memory_activate0 (GtkCheckButton *btn,
                       gpointer        data)
{
  NesPalette *self = NES_PALETTE (data);
  CanvasSettings sets;
  sets.type_canvas   = TYPE_CANVAS_TILESET;
  sets.canvas_width  = DEFAULT_CANVAS_WIDTH;
  sets.canvas_height = DEFAULT_CANVAS_HEIGHT;
  sets.scale         = DEFAULT_CANVAS_SCALE;
  sets.palette_type  = DEFAULT_PALETTE_TYPE;
  sets.width_rect   = 8;
  sets.height_rect  = 8;
  sets.left_top      = TRUE;

  self->cur_width_rect = sets.width_rect;
  self->cur_heigh_rect = sets.height_rect;

  g_object_set (self->tileset, "settings", &sets, NULL);


  self->cur_palette = NES_SPRITE;

  guint32 width = sets.canvas_width * sets.scale +
        (sets.canvas_width    / sets.width_rect);
  guint32 height = sets.canvas_height * sets.scale +
        (sets.canvas_height / sets.height_rect);

  gtk_widget_set_size_request (self->tileset, width, height);

  RetroCanvas *cnvs = retro_canvas_get_drawing_canvas ();

  if (GTK_IS_WIDGET (cnvs)) {
    sets.type_canvas   = TYPE_CANVAS_TILESET;
    sets.canvas_width  = -1;
    sets.canvas_height = -1;
    sets.scale         = -1;
    sets.palette_type  = -1;
    sets.width_rect    = 8;
    sets.height_rect   = 8;
    sets.left_top      = FALSE;
    g_object_set (cnvs, "settings", &sets, NULL);

  }

	global_set_cur_bank (NES_SPRITE);
	nes_list_palette_set_bank (NES_SPRITE);
}

static void
bank_memory_activate1 (GtkCheckButton *btn,
                       gpointer        data)
{
  NesPalette *self = NES_PALETTE (data);
  CanvasSettings sets;
  sets.type_canvas   = TYPE_CANVAS_TILESET;
  sets.canvas_width  = DEFAULT_CANVAS_WIDTH;
  sets.canvas_height = DEFAULT_CANVAS_HEIGHT;
  sets.scale         = DEFAULT_CANVAS_SCALE;
  sets.palette_type  = DEFAULT_PALETTE_TYPE;
  sets.width_rect   =  8;
  sets.height_rect  =  8;
  sets.left_top      = TRUE;

  self->cur_palette = NES_BACKGROUND;

  self->cur_width_rect = sets.width_rect;
  self->cur_heigh_rect = sets.height_rect;

  g_object_set (self->tileset, "settings", &sets, NULL);

  guint32 width = sets.canvas_width * sets.scale +
        (sets.canvas_width    / sets.width_rect);
  guint32 height = sets.canvas_height * sets.scale +
        (sets.canvas_height / sets.height_rect);

  gtk_widget_set_size_request (self->tileset, width, height);

  RetroCanvas *cnvs = retro_canvas_get_drawing_canvas ();

  if (GTK_IS_WIDGET (cnvs)) {
    sets.type_canvas   = TYPE_CANVAS_TILESET;
    sets.canvas_width  = -1;
    sets.canvas_height = -1;
    sets.scale         = -1;
    sets.palette_type  = -1;
    sets.width_rect    =  8;
    sets.height_rect   =  8;
    sets.left_top      = FALSE;
    g_object_set (cnvs, "settings", &sets, NULL);
  }

	global_set_cur_bank (NES_BACKGROUND);
	nes_list_palette_set_bank (NES_BACKGROUND);
}

static void
nes_background_init (NesTilePoint *p)
{
	guint32 i = 0;
	guint32 blkx = 0;
	guint32 blky = 0;
	guint32 ax = 0;
	guint32 ay = 0;
	for (guint32 y = 0; y < 128; y++) {
		for (guint32 x = 0; x < 128; x++) {
			p[i].x = x;
			p[i].y = y;
			p[i].blockx = blkx;
			p[i].blocky = blky;
			p[i].index  = 0;
			ax++;
			if (ax >= 16) {
				ax = 0;
				blkx++;
			}
		}
		blkx = 0;
		ay++;
		if (ay >= 16) {
			ay = 0;
			blky++;
		}
	}
}

static void
nes_sprite_init (NesTilePoint *p)
{
	guint32 i = 0;
	guint32 blkx = 0;
	guint32 blky = 0;
	guint32 ax = 0;
	guint32 ay = 0;
	for (guint32 y = 0; y < 128; y++) {
		for (guint32 x = 0; x < 128; x++) {
			p[i].x = x;
			p[i].y = y;
			p[i].blockx = blkx;
			p[i].blocky = blky;
			p[i].index  = 0;
			ax++;
			if (ax >= 8) {
				ax = 0;
				blkx++;
			}
		}
		blkx = 0;
		ay++;
		if (ay >= 8) {
			ay = 0;
			blky++;
		}
	}
}

#define                  NES_CANVAS_SIZE        128

NesTilePoint *
nes_palette_get_block (guint32 w, guint32 h, guint32 blkx, guint32 blky, guint32 index_map)
{
	NesTilePoint *p = g_malloc0 (sizeof (NesTilePoint) * w * h);

	NesTilePoint *z = global_nes->map[index_map];

	guint32 indx = 0;
	gint ndx = 0;
	guint32 offset = blky * NES_CANVAS_SIZE * w;
	for (guint32 y = 0; y < h; y++) {
		guint32 i = offset + y * NES_CANVAS_SIZE + blkx * w;
		for (guint32 x = 0; x < w; x++) {
			p[indx].x = x;
			p[indx].y = y;
			ndx = z[i + x].index;
			if (ndx > 0)
				ndx--;
			p[indx].index = ndx;
			indx++;
		}
	}

	return p;
}

static void
nes_init_map (NesTilePoint *p, guint32 type)
{
	switch (type)
	{
		case NES_SPRITE:
			nes_sprite_init (p);
			break;
		case NES_BACKGROUND:
			nes_background_init (p);
			break;
		default:
			break;
	}
}

static void
nes_palette_init (NesPalette *self)
{
  global_nes = self;
  self->cur_palette = NES_SPRITE;

  self->tileset = g_object_new (RETRO_TYPE_CANVAS,
                                NULL);
  retro_canvas_set_tileset (RETRO_CANVAS (self->tileset));

  self->colours = g_object_new (RETRO_TYPE_CANVAS,
                                NULL);


  self->frame_current_palette = g_object_new (NES_TYPE_CURRENT_PALETTE,
                                            "label", "Select Colour",
                                            NULL);

  self->frame_list_items_palette = g_object_new (NES_TYPE_LIST_PALETTE,
                                                 "label", "List Palette", NULL);

  self->frame_colours = gtk_frame_new ("Palette");
  gtk_frame_set_child (GTK_FRAME (self->frame_colours), self->colours);

  self->frame_tileset = gtk_frame_new ("Tileset");
  gtk_frame_set_child (GTK_FRAME (self->frame_tileset), self->tileset);

  self->frame_banks = gtk_frame_new ("Banks");
	global_set_cur_bank (NES_SPRITE);

  CanvasSettings cs_front;
  cs_front.type_canvas   = TYPE_CANVAS_TILESET;
  cs_front.canvas_width   = DEFAULT_CANVAS_WIDTH;
  cs_front.canvas_height  = DEFAULT_CANVAS_HEIGHT;
  cs_front.palette_type   = DEFAULT_PALETTE_TYPE;
  cs_front.scale          = DEFAULT_CANVAS_SCALE;
  cs_front.width_rect    = 8;
  cs_front.height_rect   = 8;
  cs_front.left_top       = TRUE;
  g_object_set (self->tileset, "settings", &cs_front, NULL);


  self->cur_width_rect = cs_front.width_rect;
  self->cur_heigh_rect = cs_front.height_rect;

  CanvasSettings cs_back;
  cs_back.type_canvas   = TYPE_CANVAS_TILESET;
  cs_back.canvas_width   = DEFAULT_CANVAS_WIDTH;
  cs_back.canvas_height  = DEFAULT_CANVAS_HEIGHT;
  cs_back.palette_type   = DEFAULT_PALETTE_TYPE;
  cs_back.scale          = DEFAULT_CANVAS_SCALE;
  cs_back.width_rect    = 8;
  cs_back.height_rect   = 8;
  cs_back.left_top       = TRUE;

  self->count_sprite_x = cs_front.canvas_width / cs_front.width_rect;
  self->count_sprite_y = cs_front.canvas_height / cs_front.height_rect;
  self->count_back_x = cs_back.canvas_width / cs_back.width_rect;
  self->count_back_y = cs_back.canvas_height / cs_back.height_rect;

  CanvasSettings cs_palette;
  cs_palette.type_canvas   = TYPE_CANVAS_COLOUR_GRID;
  cs_palette.canvas_width   = DEFAULT_COLOUR_CANVAS_WIDTH;
  cs_palette.canvas_height  = DEFAULT_COLOUR_CANVAS_HEIGHT;
  cs_palette.palette_type   = DEFAULT_COLOUR_PALETTE_TYPE;
  cs_palette.scale          = DEFAULT_COLOUR_CANVAS_SCALE;
  cs_palette.width_rect    = 1;
  cs_palette.height_rect   = 1;
  cs_palette.count_x        = 16;
  cs_palette.count_y        = 4;
  cs_palette.left_top       = TRUE;
  g_object_set (self->colours, "settings", &cs_palette, NULL);

  guint32 *pcolours = global_type_palette_get_cur_ptr_palette (0);
  retro_canvas_set_colours (RETRO_CANVAS (self->colours), pcolours, 64);
  guint32 *index_color = g_malloc0 (sizeof (guint32) * 64);
  for (int col = 0; col < 64; col++) {
    index_color[col] = col;
  }
  retro_canvas_set_index_colours (RETRO_CANVAS (self->colours), index_color);


  gtk_widget_set_size_request (self->tileset, 256, 256);
  gtk_box_append (GTK_BOX (self), self->frame_tileset);

  gtk_widget_set_size_request (self->colours,
                               cs_palette.canvas_width * cs_palette.scale,
                               cs_palette.canvas_height * cs_palette.scale);


  self->box_banks = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  self->bank0 = gtk_check_button_new_with_label ("sprites");
  self->bank1 = gtk_check_button_new_with_label ("backgrounds");

  gtk_frame_set_child (GTK_FRAME (self->frame_banks), self->box_banks);

  gtk_check_button_set_group (GTK_CHECK_BUTTON (self->bank1),
                              GTK_CHECK_BUTTON (self->bank0));

  gtk_box_append (GTK_BOX(self->box_banks), self->bank0);
  gtk_box_append (GTK_BOX(self->box_banks), self->bank1);

  gtk_box_append (GTK_BOX (self), self->frame_banks);

  gtk_box_append (GTK_BOX (self), self->frame_colours);

  g_signal_connect (self->bank0, "toggled",
                    G_CALLBACK (bank_memory_activate0),
                    self);
  g_signal_connect (self->bank1, "toggled",
                    G_CALLBACK (bank_memory_activate1),
                    self);

  gtk_check_button_set_active (GTK_CHECK_BUTTON (self->bank0), TRUE);

  global_type_palette_set_cur (PLATFORM_PALETTE_NES, NES_TYPE_PALETTE_2C02);

  retro_canvas_set_type_palette (
    RETRO_CANVAS (self->colours),
    NES_TYPE_PALETTE_2C02,
    64);

  guint32 t = 128 * 128;
  self->map[NES_SPRITE] = g_malloc0 (sizeof (NesTilePoint) * t);
  self->map[NES_BACKGROUND] = g_malloc0 (sizeof (NesTilePoint) * t);

  gtk_box_append (GTK_BOX (self), self->frame_list_items_palette);

	nes_init_map (self->map[NES_SPRITE], NES_SPRITE);
	nes_init_map (self->map[NES_BACKGROUND], NES_BACKGROUND);


  gtk_box_append (GTK_BOX (self), self->frame_current_palette);
}

NesPalette *
nes_palette_get (void)
{
  return global_nes;
}

NesTilePoint *
nes_palette_get_color (NesPalette *self,
                       guint32 x, guint32 y)
{
  NesTilePoint *point = &self->map[self->cur_palette][y * 128 + x];

  return point;
}

NesTilePoint *
nes_palette_get_map (guint32 index)
{
	return global_nes->map[index];
}

void 
nes_palette_set_color_with_map (NesPalette *self,
                    NesParamPoint               *params,
                    guint32                      index,
										guint32 										map)
{
  guint32 x = params->blockx * self->cur_width_rect + params->x;
  guint32 y = params->blocky * self->cur_heigh_rect + params->y;

  NesTilePoint *point = &self->map[map][y * 128 + x];

  point->blockx = params->blockx;
  point->blocky = params->blocky;
  point->x = params->x;
  point->y = params->y;
  point->index = index;

  gtk_widget_queue_draw (GTK_WIDGET (self->tileset));
}                                    

void 
nes_palette_set_color (NesPalette *self,
                    NesParamPoint               *params,
                    guint32                      index)
{
  guint32 cur_palette = self->cur_palette;
  guint32 x = params->blockx * self->cur_width_rect + params->x;
  guint32 y = params->blocky * self->cur_heigh_rect + params->y;

  NesTilePoint *point = &self->map[cur_palette][y * 128 + x];

  point->blockx = params->blockx;
  point->blocky = params->blocky;
  point->x = params->x;
  point->y = params->y;
  point->index = index;

  gtk_widget_queue_draw (GTK_WIDGET (self->tileset));
}                                    