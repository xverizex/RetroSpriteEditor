/* main-canvas.c
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

#include "retro-canvas.h"
#include "custom-math.h"
#include "nes-params.h"
#include "nes-palette.h"
#include "nes-list-palettes.h"
#include "nes-current-palette.h"
#include "general-tools.h"
#include "global-functions.h"
#include "nes-screen-background.h"
#include "nes-frame-megatile.h"

#define NES_SCREEN_SIZE         32 * 28 
#define NES_MEGATILE_COUNT       8 *  7

struct _RetroCanvas
{
  GtkDrawingArea  parent_instance;

	guint32 is_copy;
	guint32 item_id;
	guint32 draw_blank;
  guint32 width;
  guint32 height;
  guint32 orig_width;
  guint32 orig_height;
  guint32 palette_type;
  guint32 width_rect;
  guint32 height_rect;
  gint32 scale;
  guint32 type_canvas;
  guint32 count_x;
  guint32 count_y;
  guint32 *colours;
  guint32 *index_color;
  guint32 count_colours;
  gboolean left_top;
  gint32 px;
  gint32 py;
  gint32 w;
  gint32 h;
  gint32 xw;
  gint32 yh;
  guint32 selected_index_color;
	guint8 *tile_background_pos;
	TileRef *tile_ref;
	gint32 first_tile_x;
	gint32 first_tile_y;
	guint8 *megatile;

  gboolean is_2_btn_pressed;
  gboolean is_0_btn_pressed;
  gint32 mx;
  gint32 my;
  gint32 ox;
  gint32 oy;
  guint32 last_pointx;
  guint32 last_pointy;
  guint32 last_blockx;
  guint32 last_blocky;
  guint32 colour_block_size;
  gboolean show_hex_index;


  guint32 tool;
  gboolean drawing_tool;
	guint32 index_id;

	gint32 tile_start_x;
	gint32 tile_start_y;
	gint32 tile_end_x;
	gint32 tile_end_y;

  GtkEventController *event_zoom;
  GtkEventController *event_motion;
  GtkGesture         *gesture_click_move_canvas;
  GtkGesture         *gesture_click_tool_press;
  GtkGesture         *gesture_click_select_index_color;
  GtkGesture         *gesture_click_select_one_color;
	RetroCanvas  *child_one_color;
};


G_DEFINE_FINAL_TYPE (RetroCanvas, retro_canvas, GTK_TYPE_DRAWING_AREA)

enum {
  PROP_SETTINGS = 1,
  N_PROPERTIES
};

static RetroCanvas *global_drawing_canvas;
static RetroCanvas *global_drawing_canvas_tileset;
static RetroCanvas *global_drawing_canvas_screen;

guint8 *
retro_canvas_screen_nes_get_megatile ()
{
	return global_drawing_canvas_screen->megatile;
}

void *
retro_canvas_screen_nes_get_tile_ref ()
{
	return global_drawing_canvas_screen->tile_ref;
}

void
retro_canvas_nes_set_screen (RetroCanvas *self)
{
	global_drawing_canvas_screen = self;
}

GtkWidget *
retro_canvas_nes_get_screen ()
{
	return GTK_WIDGET (global_drawing_canvas_screen);
}

void
retro_canvas_redraw_drawing_and_tileset (void)
{
	gtk_widget_queue_draw (GTK_WIDGET (global_drawing_canvas));
	gtk_widget_queue_draw (GTK_WIDGET (global_drawing_canvas_tileset));
}

static void
colour_rgb_get_double_color (guint32 color,
                             gdouble *r,
                             gdouble *g,
                             gdouble *b)
{
  guint8 red =   (color >>  0) & 0xff;
  guint8 green = (color >>  8) & 0xff;
  guint8 blue =  (color >> 16) & 0xff;

  *r = red / 255.0;
  *g = green / 255.0;
  *b = blue / 255.0;
}

static void
draw_grid_nes_screen_megatile_palette (cairo_t           *cr,
           int                      width,
           int                      height,
           RetroCanvas *self)
{


  double line_width = 1.0;

  cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
  cairo_set_line_width (cr, line_width);

  int x = self->px + 1;
  int y = self->py + 1;

  if (self->left_top) {
    x = y = 0;
  }

  guint32 count_rect_w = self->orig_width / (self->width_rect * 4);
  guint32 count_rect_h = self->orig_height / (self->height_rect * 4);

  guint32 rect_w_result_size = c_pow (self->width_rect * 4, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect * 4, self->scale);


  int xx = 0;
  int yy = 0;


  for (int cyy = 0; cyy < count_rect_h; cyy++) {
    for (int cxx = 0; cxx < count_rect_w; cxx++) {
      cairo_rectangle (cr, x + xx, y + yy,
                       rect_w_result_size + 4,
                       rect_h_result_size + 4);
      xx += rect_w_result_size;
      xx += 4;
    }
    yy += rect_h_result_size;
    yy += 4;
    xx = 0;
  }

  cairo_stroke (cr);
}

static void
draw_grid_nes_screen_one_palette (cairo_t           *cr,
           int                      width,
           int                      height,
           RetroCanvas *self)
{


  double line_width = 1.0;

  cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
  cairo_set_line_width (cr, line_width);

  int x = self->px + 1;
  int y = self->py + 1;

  if (self->left_top) {
    x = y = 0;
  }

  guint32 count_rect_w = self->orig_width / (self->width_rect * 2);
  guint32 count_rect_h = self->orig_height / (self->height_rect * 2);

  guint32 rect_w_result_size = c_pow (self->width_rect * 2, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect * 2, self->scale);


  int xx = 0;
  int yy = 0;


  for (int cyy = 0; cyy < count_rect_h; cyy++) {
    for (int cxx = 0; cxx < count_rect_w; cxx++) {
      cairo_rectangle (cr, x + xx, y + yy,
                       rect_w_result_size + 2,
                       rect_h_result_size + 2);
      xx += rect_w_result_size;
      xx += 2;
    }
    yy += rect_h_result_size;
    yy += 2;
    xx = 0;
  }

  cairo_stroke (cr);
}
static void
draw_grid (cairo_t                 *cr,
           int                      width,
           int                      height,
           RetroCanvas *self)
{


  double line_width = 1.0;

  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_set_line_width (cr, line_width);

  int x = self->px + 1;
  int y = self->py + 1;

  if (self->left_top) {
    x = y = 0;
  }

  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;

  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);


  int xx = 0;
  int yy = 0;


  for (int cyy = 0; cyy < count_rect_h; cyy++) {
    for (int cxx = 0; cxx < count_rect_w; cxx++) {
      cairo_rectangle (cr, x + xx, y + yy,
                       rect_w_result_size,
                       rect_h_result_size);
      xx += rect_w_result_size;
      xx++;
    }
    yy += rect_h_result_size;
    yy++;
    xx = 0;
  }

  cairo_stroke (cr);
}

static void
selection_bottom_right_left_up (RetroCanvas *self, guint32 cxx, guint32 cyy)
{
	int fx = self->first_tile_x;
	int fy = self->first_tile_y;
	int ex = cxx;
	int ey = cyy;
	guint8 cur_draw = 0xff;
	guint32 found = 0;
	self->tile_start_x = -1;
	self->tile_start_y = -1;
	self->tile_end_x = -1;
	self->tile_end_y = -1;

	self->tile_start_x = cxx;
	self->tile_start_y = ey;
	self->tile_end_x = fx;
	self->tile_end_y = cyy;

	for (int dy = 0; dy < 16; dy++) {
		for (int dx = 0; dx < 16; dx++) {
			found = 0;
			if ((dy <= fy) && (dy >= ey)) {
				if ((dx <= fx) && (dx >= ex)) {
					self->tile_background_pos[dy * 16 + dx] = cur_draw;
					found = 1;
					continue;
				}
			}
			if (found == 0) {
				self->tile_background_pos[dy * 16 + dx] = 0;
			}
		}
	}
}

static void
selection_bottom_left_right_up (RetroCanvas *self, guint32 cxx, guint32 cyy)
{
	self->tile_start_x = -1;
	self->tile_start_y = -1;
	self->tile_end_x = -1;
	self->tile_end_y = -1;
	int fx = self->first_tile_x;
	int fy = self->first_tile_y;
	int ex = cxx;
	int ey = cyy;
	guint8 cur_draw = 0xff;
	guint32 found = 0;
	self->tile_start_x = fx;
	self->tile_start_y = ey;
	self->tile_end_x = ex;
	self->tile_end_y = fy;

	for (int dy = 0; dy < 16; dy++) {
		for (int dx = 0; dx < 16; dx++) {
			found = 0;
			if ((dy <= fy) && (dy >= ey)) {
				if ((dx >= fx) && (dx <= ex)) {
					self->tile_background_pos[dy * 16 + dx] = cur_draw;
					found = 1;
					continue;
				}
			}
			if (found == 0) {
				self->tile_background_pos[dy * 16 + dx] = 0;
			}
		}
	}
}

static void
selection_top_right_left_down (RetroCanvas *self, guint32 cxx, guint32 cyy)
{
	self->tile_start_x = -1;
	self->tile_start_y = -1;
	self->tile_end_x = -1;
	self->tile_end_y = -1;
	int fx = self->first_tile_x;
	int fy = self->first_tile_y;
	int ex = cxx;
	int ey = cyy;
	guint8 cur_draw = 0xff;
	guint32 found = 0;
	self->tile_start_x = ex;
	self->tile_start_y = fy;
	self->tile_end_x = fx;
	self->tile_end_y = ey;

	for (int dy = 0; dy < 16; dy++) {
		for (int dx = 0; dx < 16; dx++) {
			found = 0;
			if ((dy >= fy) && (dy <= ey)) {
				if ((dx <= fx) && (dx >= ex)) {
					self->tile_background_pos[dy * 16 + dx] = cur_draw;
					found = 1;
					continue;
				}
			}
			if (found == 0) {
				self->tile_background_pos[dy * 16 + dx] = 0;
			}
		}
	}
}

static void
selection_top_left_right_down (RetroCanvas *self, guint32 cxx, guint32 cyy)
{
	self->tile_start_x = -1;
	self->tile_start_y = -1;
	self->tile_end_x = -1;
	self->tile_end_y = -1;
	int fx = self->first_tile_x;
	int fy = self->first_tile_y;
	int ex = cxx;
	int ey = cyy;
	guint8 cur_draw = 0xff;
	guint32 found = 0;
	self->tile_start_x = fx;
	self->tile_start_y = fy;
	self->tile_end_x = ex;
	self->tile_end_y = ey;

	for (int dy = 0; dy < 16; dy++) {
		for (int dx = 0; dx < 16; dx++) {
			found = 0;
			if ((dy >= fy) && (dy <= ey)) {
				if ((dx >= fx) && (dx <= ex)) {
					self->tile_background_pos[dy * 16 + dx] = cur_draw;
					found = 1;
					continue;
				}
			}
			if (found == 0) {
				self->tile_background_pos[dy * 16 + dx] = 0;
			}
		}
	}
}

static void
draw_tool_copy_tile_dst (RetroCanvas *self, cairo_t *cr, int width, int height)
{
  int posx = self->mx - self->px;
  int posy = self->my - self->py;

  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;

  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);

  guint32 y = self->py;
  guint32 x = self->px;
  int found = 0;
  int xx = 1;
  int yy = 1;
  int cyy = 0;
  int cxx = 0;
  int pointx = 0;
  int pointy = 0;
  for (cyy = 0; cyy < count_rect_h; cyy++) {
    for (cxx = 0; cxx < count_rect_w; cxx++) {
      int ex = xx + rect_w_result_size;
      int ey = yy + rect_h_result_size;

      if ((posx >= xx) && (posx <= ex)) {
        if ((posy >= yy) && (posy <= ey)) {
          found = 1;
          break;
        }
      }
      xx += rect_w_result_size;
      xx++;
    }
    if (found)
      break;
    yy += rect_h_result_size;
    yy++;
    xx = 1;
  }

  if (found == 0)
    return;

  double line_width = 1.0;

  if (self->left_top) {
    x = y = 0;
  }

	xx = 1;
	yy = 1;

  if (self->is_0_btn_pressed) {
		//self->tile_background_pos[cyy * 16 + cxx] = cur_draw;
	}

	NesPalette *nes = nes_palette_get ();

	int vx = self->tile_start_x;
	int vy = self->tile_start_y;
	int vex = self->tile_end_x - vx;
	int vey = self->tile_end_y - vy;
	vx = vy = 0;

	int cx = 0;
	int cy = 0;
	
	int cpowx = c_pow (1, self->scale);
	int cpowy = c_pow (1, self->scale);

	int offsetx = 1;
	int offsety = 1;
	for (int n = 0; n < cyy; n++) {
		offsety++;
	}
	for (int n = 0; n < cxx; n++) {
		offsetx++;
	}
	int blkx = 0;
	int blky = 0;

  for (cy = 0; cy < 16; cy++) {
		int mx = 0;
		int my = 0;
    for (cx = 0; cx < 16; cx++) {
			int nx = 0;
			int ny = 0;
			if (self->tile_background_pos[cy * 16 + cx] > 0) {
				for (ny = 0; ny < 8; ) {
					for ( nx = 0; nx < 8; ) {
						int oy = cy * 8 + ny;
						int ox = cx * 8 + nx;
						NesTilePoint *p = nes_palette_get_color (nes, ox, oy);

						if (p->index > 0) {
							guint32 *colours = global_type_palette_get_cur_ptr_palette (0);
							guint32 *indexed_colour = global_nes_palette_get_memory_index (0);

  						double r, g, b;
  						colour_rgb_get_double_color (colours[indexed_colour[p->index - 1]], &r, &g, &b);
  						cairo_set_source_rgb (cr, r, g, b);

							if (self->is_0_btn_pressed) {
								//g_print ("%d + %d + %d + %d\n", cyy * count_rect_w, vy * count_rect_w, cxx, vx);
								int index_pos = cyy * count_rect_w + vy * count_rect_w + cxx + vx;
								//g_print ("index_pos: %d; %d %d\n", index_pos, cx, cy);
								self->tile_ref[index_pos].tilex = cx;
								self->tile_ref[index_pos].tiley = cy;
							}
							/*
							 * TODO: fix this
							 */
  						cairo_rectangle (cr, 
									self->px + cxx * 8 * c_pow (1, self->scale) + vx * 8 * c_pow (1, self->scale) + mx + offsetx + blkx, 
									self->py + cyy * 8 * c_pow (1, self->scale) + vy * 8 * c_pow (1, self->scale) + my + offsety + blky,
											cpowx,
											cpowy);

							cairo_fill (cr);
						}

						mx += cpowx;
						nx++;
					}
					my += cpowy;
					ny++;
					mx = 0;
				}
				vx++;
				my = 0;
				blkx += 1;
			}
		}
		blkx = 0;
		blky += 1;
		vx = 0;
		cx = 0;
		vy++;
		my = 0;
	}
}

static void
draw_tool_copy_tile (RetroCanvas *self, cairo_t *cr, int width, int height)
{
  int posx = self->mx - self->px;
  int posy = self->my - self->py;

  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;

  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);

  guint32 y = self->py;
  guint32 x = self->px;
  int found = 0;
  int xx = 1;
  int yy = 1;
  int cyy = 0;
  int cxx = 0;
  int pointx = 0;
  int pointy = 0;
  for (cyy = 0; cyy < count_rect_h; cyy++) {
    for (cxx = 0; cxx < count_rect_w; cxx++) {
      int ex = xx + rect_w_result_size;
      int ey = yy + rect_h_result_size;

      if ((posx >= xx) && (posx <= ex)) {
        if ((posy >= yy) && (posy <= ey)) {
          found = 1;
          break;
        }
      }
      xx += rect_w_result_size;
      xx++;
    }
    if (found)
      break;
    yy += rect_h_result_size;
    yy++;
    xx = 1;
  }

  if (found == 0)
    return;

  double line_width = 1.0;

  cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
  cairo_set_line_width (cr, line_width);

  if (self->left_top) {
    x = y = 0;
  }

	xx = 1;
	yy = 1;

	for (int cy = 0; cy < cyy; cy++) {
  	yy += rect_h_result_size;
		yy += 1;
	}

	for (int cx = 0; cx < cxx; cx++) {
  	xx += rect_w_result_size;
	  xx += 1;
	}

  cairo_rectangle (cr, x + xx, y + yy,
                   rect_w_result_size + 1,
                   rect_h_result_size + 1);

  cairo_stroke (cr);

  if (self->is_0_btn_pressed) {
		guint8 cur_draw = 0xff;

		if (self->first_tile_x == -1 && self->first_tile_y == -1) {
			self->first_tile_x = cxx;
			self->first_tile_y = cyy;
		} else {
			int fx = self->first_tile_x;
			int fy = self->first_tile_y;
			int ex = cxx;
			int ey = cyy;
			if (ey >= fy && ex >= fx) {
				selection_top_left_right_down (self, cxx, cyy);
			}
			if (ey >= fy && ex < fx) {
				selection_top_right_left_down (self, cxx, cyy);
			}
			if (ey < fy && ex >= fx) {
				selection_bottom_left_right_up (self, cxx, cyy);
			}
			if (ey < fy && ex < fx) {
				selection_bottom_right_left_up (self, cxx, cyy);
			}
		}
		self->tile_background_pos[cyy * 16 + cxx] = cur_draw;
	}

	xx = 1;
	yy = 1;
  for (int cy = 0; cy < count_rect_h; cy++) {
    for (int cx = 0; cx < count_rect_w; cx++) {
			if (self->tile_background_pos[cy * 16 + cx] > 0) {
  			cairo_rectangle (cr, x + xx, y + yy,
     		              rect_w_result_size + 1,
       		            rect_h_result_size + 1);
			}
  		xx += rect_w_result_size;
	  	xx += 1;
		}
  	yy += rect_h_result_size;
		yy += 1;
		xx = 0;
	}

  cairo_stroke (cr);
}

static void
calculate_last_block_and_point (RetroCanvas *self, int *ax, int *ay)
{
  int posx = self->mx - self->px;
  int posy = self->my - self->py;


  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;

  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);

  guint32 y = 0;
  guint32 x = 0;
  int found = 0;
  int xx = 1;
  int yy = 1;
  int cyy = 0;
  int cxx = 0;
  int pointx = 0;
  int pointy = 0;
  for (cyy = 0; cyy < count_rect_h; cyy++) {
    for (cxx = 0; cxx < count_rect_w; cxx++) {
      int ex = xx + rect_w_result_size;
      int ey = yy + rect_h_result_size;

      if ((posx >= xx) && (posx <= ex)) {
        if ((posy >= yy) && (posy <= ey)) {
          found = 1;
          break;
        }
      }
      xx += rect_w_result_size;
      xx++;
    }
    if (found)
      break;
    yy += rect_h_result_size;
    yy++;
    xx = 1;
  }

  if (found == 0)
    return;

  found = 0;

  for (y = yy; y < (yy + rect_h_result_size);) {
    for (x = xx; x < (xx + rect_w_result_size);) {
			*ax = x;
			*ay = y;
      int ex = x + c_pow (1, self->scale);
      int ey = y + c_pow (1, self->scale);
      if ((posx >= x) && (posx <= ex)) {
        if ((posy >= y) && (posy <= ey)) {
          found = 1;
          break;
        }
      }

      x += c_pow (1, self->scale);
      pointx++;
    }
    if (found)
      break;
    y += c_pow (1, self->scale);
    pointy++;
    pointx = 0;
  }

  if (found == 0)
    return;

  self->last_pointx = pointx;
  self->last_pointy = pointy;
  self->last_blockx = cxx;
  self->last_blocky = cyy;
}

static void
draw_tool_pencil (RetroCanvas *self, cairo_t *cr, int width, int height)
{
	int x, y;
	x = y = 0;
	calculate_last_block_and_point (self, &x, &y);

  int psize = 1;

  if (self->is_0_btn_pressed) {
    NesParamPoint n;
    n.blockx = self->last_blockx;
    n.blocky = self->last_blocky;
    n.x = self->last_pointx;
    n.y = self->last_pointy;
    NesPalette *nes = nes_palette_get ();
    nes_palette_set_color (nes, &n, self->selected_index_color + 1);
  }

	guint32 *colour = global_type_palette_get_cur_ptr_palette (0);
  double r, g, b;
  colour_rgb_get_double_color (colour[self->index_color[self->selected_index_color]], &r, &g, &b);
  cairo_set_source_rgb (cr, r, g, b);

	psize = c_pow (1, self->scale);
	cairo_rectangle (cr, self->px + x, self->py + y, psize, psize);
	cairo_fill (cr);
}

static void
draw_tool (cairo_t                 *cr,
                int                      width,
                int                      height,
                RetroCanvas *self)
{
  if (!self->drawing_tool)
    return;

  int posx = self->mx - self->px;
  int posy = self->my - self->py;

	switch (self->tool) {
		case INDX_TOOL_PENCIL:
			draw_tool_pencil (self, cr, width, height);
			break;
		case INDX_TOOL_COPY_TILE_SRC:
			draw_tool_copy_tile (self, cr, width, height);
			break;
		case INDX_TOOL_COPY_TILE_DST:
			draw_tool_copy_tile_dst (self, cr, width, height);
			break;
	}

}

void
retro_canvas_set_index (RetroCanvas *self,
		guint32 index)
{
	self->selected_index_color = index;
}
static void
draw_rectangle (cairo_t                 *cr,
                int                      width,
                int                      height,
                RetroCanvas *self)
{


  int x = self->px;
  int y = self->py;

  if (self->left_top) {
    x = y = 0;
  }

  cairo_set_source_rgb (cr, 0x4c / 255.0, 0x4c / 255.0, 0x4c / 255.0);
  cairo_paint (cr);

  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;
  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);

  /*
   * Calculate full size of rectangle with white lines as grid.
   */
  guint32 w = count_rect_w * rect_w_result_size + count_rect_w + 1;
  guint32 h = count_rect_h * rect_h_result_size + count_rect_h + 1;

  self->w = w;
  self->h = h;
  self->xw = self->px + w;
  self->yh = self->py + h;

  cairo_rectangle (cr, x, y, w, h);

  double r, g, b;
	guint32 *colours = global_type_palette_get_cur_ptr_palette (0);

  colour_rgb_get_double_color (colours[self->index_color[0]], &r, &g, &b);
  cairo_set_source_rgb (cr, r, g, b);
  cairo_fill (cr);
}

void
retro_canvas_set_index_colours (RetroCanvas *self, guint32 *index_color)
{
  self->index_color = index_color;
}

void
retro_canvas_set_colours (RetroCanvas *self, guint32 count)
{
  self->count_colours = count;
}

guint32 *
retro_canvas_palette_get_ptr_index_colours (RetroCanvas *self,
		guint32 x,
		guint32 y
		)
{
	return NULL;
}

void
retro_canvas_set_type_palette (RetroCanvas *self,
                                                guint32                  type,
                                                guint32                  count)
{
  self->colours = global_type_palette_get_cur_ptr_palette (0);
  self->count_colours = count;
}

static void
draw_screen_background (cairo_t                 *cr,
                    int                      width,
                    int                      height,
                    RetroCanvas *self)
{
  int xx = self->px + 1;
  int yy = self->py + 1;

  int posx = self->mx - self->px;
  int posy = self->my - self->py;

  guint32 count_rect_w = self->orig_width / self->width_rect;
  guint32 count_rect_h = self->orig_height / self->height_rect;

  guint32 rect_w_result_size = c_pow (self->width_rect, self->scale);
  guint32 rect_h_result_size = c_pow (self->height_rect, self->scale);

  guint32 y = self->py;
  guint32 x = self->px;
  int found = 0;
  xx = 1;
  yy = 1;
  int cyy = 0;
  int cxx = 0;
  int pointx = 0;
  int pointy = 0;
  double line_width = 1.0;

  if (self->left_top) {
    x = y = 0;
  }

	xx = 1;
	yy = 1;

	NesPalette *nes = nes_palette_get ();

	int cx = 0;
	int cy = 0;
	
	int cpowx = c_pow (1, self->scale);
	int cpowy = c_pow (1, self->scale);

	int offsetx = 1;
	int offsety = 1;

	int blkx = 0;
	int blky = 0;

	int max = NES_SCREEN_SIZE;

  for (cyy = 0; cyy < 24; cyy++) {
		int mx = 0;
		int my = 0;
    for (cxx = 0; cxx < 32; cxx++) {
			int nx = 0;
			int ny = 0;
			int index_ref = cyy * 32 + cxx;

			int blockx = cxx / 4;
			int blocky = cyy / 4;
			int ccx = cxx - blockx * 4;
			int ccy = cyy - blocky * 4;
			ccx = ccx / 2;
			ccy = ccy / 2;
			guint8 pal = ccy * 2 + ccx;

			guint8 *meg = &self->megatile[blocky * 8 + blockx];

			guint8 check_pal = 0xc0;
			for (int i = 0; i < 4; i++) {
				if (pal == i)
					break;
				check_pal >>= 2;
			}

			guint8 cp = *meg & check_pal;
			guint8 pal_offset[4] = {
					6,
					4,
					2,
					0
				};

			cp >>= pal_offset[pal];

			if (self->tile_ref[index_ref].tilex >= 0 &&
					self->tile_ref[index_ref].tiley >= 0) {
				for (ny = 0; ny < 8; ) {
					for ( nx = 0; nx < 8; ) {
						cx = self->tile_ref[index_ref].tilex;
						cy = self->tile_ref[index_ref].tiley;
						int oy = cy * 8 + ny;
						int ox = cx * 8 + nx;
						NesTilePoint *p = nes_palette_get_color (nes, ox, oy);

						if (p->index > 0) {
							guint32 *colours = global_type_palette_get_cur_ptr_palette (0);

							guint32 *indexed_colour = global_nes_palette_get_memory_index (cp);

  						double r, g, b;
  						colour_rgb_get_double_color (colours[indexed_colour[p->index - 1]], &r, &g, &b);
  						cairo_set_source_rgb (cr, r, g, b);

  						cairo_rectangle (cr, 
									self->px + cxx * 8 * c_pow (1, self->scale) + mx + offsetx + blkx, 
									self->py + cyy * 8 * c_pow (1, self->scale) + my + offsety + blky,
											cpowx,
											cpowy);

							cairo_fill (cr);
						}

						mx += cpowx;
						nx++;
					}
					my += cpowy;
					ny++;
					mx = 0;
				}
				my = 0;
				//blkx += 1;
			}
			offsetx++;
		}
		offsetx = 1;
		blkx = 0;
		blky += 1;
		cx = 0;
		my = 0;
	}
}
static void
draw_pixels (cairo_t                 *cr,
                    int                      width,
                    int                      height,
                    RetroCanvas *self)
{
  int xx = self->px + 1;
  int yy = self->py + 1;

  int nx = 0;
  int ny = 0;
  NesPalette *nes = nes_palette_get ();
  for (guint32 y = 0; y < self->orig_height; y++) {
    for (guint32 x = 0; x < self->orig_width; x++) {

			guint32 *colours = global_type_palette_get_cur_ptr_palette (0);
      double r, g, b;
      NesTilePoint *p = nes_palette_get_color (nes, x, y);
      if (p->index > 0) {
        colour_rgb_get_double_color (colours[self->index_color[p->index - 1]], &r, &g, &b);
        cairo_set_source_rgb (cr, r, g, b);
        int psize = c_pow (1, self->scale);
        cairo_rectangle (cr, xx, yy, psize, psize);
        cairo_fill (cr);
      }
      nx++;
      xx += c_pow (1, self->scale);
      if (nx == 8) {
        xx++;
        nx = 0;
      }
    }
    xx = self->px + 1;
    nx = 0;
    ny++;
    yy += c_pow (1, self->scale);
    if (ny == 8) {
      yy++;
      ny = 0;
    }
  }
}

static void
draw_colour_blocks (cairo_t                 *cr,
                    int                      width,
                    int                      height,
                    RetroCanvas *self)
{
  guint32 indx = 0;

  cairo_set_font_size (cr, 16.0);

  guint32 offsetx = 0;
  guint32 offsety = 0;
  for (guint32 y = 0; y < self->count_y; y++) {
    for (guint32 x = 0; x < self->count_x; x++) {

			guint32 *colours = global_type_palette_get_cur_ptr_palette (0);

      double r, g, b;
      if (colours && self->index_color) {
        colour_rgb_get_double_color (colours[self->index_color[indx]], &r, &g, &b);
        cairo_set_source_rgb (cr, r, g, b);

        cairo_rectangle (cr,
                      x * self->colour_block_size * self->scale,
                      y * self->colour_block_size * self->scale,
                      self->colour_block_size * self->scale,
                      self->colour_block_size * self->scale);

        cairo_fill (cr);

        if (self->show_hex_index) {
	  			offsetx = x * self->colour_block_size * self->scale;
	  			offsety = y * self->colour_block_size * self->scale;
          int px = offsetx + self->colour_block_size / 2 - 10;
          int py = y + self->colour_block_size - 4;
          cairo_move_to (cr, px, py);
          char hex_index[32] = "\0";
					if (self->index_color) {
	  				if (colours[self->index_color[indx]] == 0x0) {
						  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
					  } else {
						  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	  				}
					}

          snprintf (hex_index, 32, "%X", self->index_color[indx]);
          cairo_show_text (cr, hex_index);
        }

				cairo_move_to (cr, x, y);
      }

      indx++;
    }
  }
}

void
retro_canvas_set_show_hex (RetroCanvas *self,
                     gboolean                 val)
{
  self->show_hex_index = val;
}

static void
draw_canvas (GtkDrawingArea *area,
             cairo_t        *cr,
             int             width,
             int             height,
             gpointer        data)
{
  RetroCanvas *self = RETRO_CANVAS (area);

	if (self->draw_blank) {
		cairo_set_source_rgb (cr, 0., 0., 0.);
		cairo_paint (cr);
		return;
	}

  switch (self->type_canvas)
    {
    case TYPE_CANVAS_TILESET:
      draw_rectangle (cr, width, height, self);
      draw_grid (cr, width, height, self);
      draw_pixels (cr, width, height, self);
      draw_tool (cr, width, height, self);
      break;
    case TYPE_CANVAS_COLOUR_GRID:
    case TYPE_CANVAS_COLOUR_PALETTE:
      draw_colour_blocks (cr, width, height, self);
      break;
		case TYPE_CANVAS_SCREEN_BACKGROUND:
      draw_rectangle (cr, width, height, self);
      draw_grid (cr, width, height, self);
      draw_grid_nes_screen_one_palette (cr, width, height, self);
      draw_grid_nes_screen_megatile_palette (cr, width, height, self);
      draw_screen_background (cr, width, height, self);
      draw_tool (cr, width, height, self);
			break;
		default:
			break;
    }

}

static GParamSpec *obj_properties[N_PROPERTIES] = {NULL, };

static void
canvas_set_settings (RetroCanvas *self,
                     CanvasSettings          *stgs)
{
  self->type_canvas = stgs->type_canvas >= 0? stgs->type_canvas: self->type_canvas;
  self->width = self->orig_width = stgs->canvas_width >= 0? stgs->canvas_width: self->width;
  self->height = self->orig_height = stgs->canvas_height >= 0? stgs->canvas_height: self->height;
  self->palette_type = stgs->palette_type >= 0? stgs->palette_type: self->palette_type;
  self->width_rect = stgs->width_rect >= 0? stgs->width_rect: self->width_rect;
  self->height_rect = stgs->height_rect >= 0? stgs->height_rect: self->height_rect;
  self->scale = stgs->scale >= 0? stgs->scale: self->scale;
  self->count_x = stgs->count_x >= 0? stgs->count_x: self->count_x;
  self->count_y = stgs->count_y >= 0? stgs->count_y:self->count_y;
  self->left_top = stgs->left_top;
  self->px = 0;
  self->py = 0;


  switch (self->type_canvas)
    {
    case TYPE_CANVAS_COLOUR_GRID:
      self->colour_block_size = 16;
      break;
    case TYPE_CANVAS_COLOUR_PALETTE:
      self->colour_block_size = 32;
      break;
    default:
      break;
    }

}

void
retro_drawing_canvas_redraw (void)
{
	gtk_widget_queue_draw (GTK_WIDGET (global_drawing_canvas));
}

static void
canvas_set_property (GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *spec)
{
  RetroCanvas *cnvs = RETRO_CANVAS (object);

  switch (property_id)
    {
    case PROP_SETTINGS:
      canvas_set_settings (cnvs, g_value_get_pointer (value));
      guint32 width =  cnvs->width + cnvs->width_rect == 1? 0: cnvs->width_rect;
      guint32 height = cnvs->height + cnvs->height_rect == 1? 0: cnvs->height_rect;
      gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (cnvs), width);
      gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (cnvs), height);
      gtk_widget_queue_draw (GTK_WIDGET (cnvs));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, spec);
      break;
    }
}

static void
canvas_get_property (GObject    *object,
                     guint       property_id,
                     GValue     *value,
                     GParamSpec *spec)
{
  RetroCanvas *self = RETRO_CANVAS (object);

  switch (property_id)
    {
    case PROP_SETTINGS:
      CanvasSettings *settings = (CanvasSettings *)
        g_value_get_pointer (value);
      settings->canvas_width = self->width;
      settings->canvas_height = self->height;
      settings->palette_type = self->palette_type;
      settings->scale = self->scale;
      break;
		default:
			break;
    }
}

void
retro_canvas_set_blank (RetroCanvas *self, int is_blank)
{
	self->draw_blank = is_blank;
}

static void
retro_canvas_class_init (RetroCanvasClass *klass)
{
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = canvas_set_property;
  object_class->get_property = canvas_get_property;

  obj_properties[PROP_SETTINGS] =
      g_param_spec_pointer ("settings",
                            "Settings",
                            "Set settings for canvas",
                            G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPERTIES, obj_properties);
}


static gboolean
zoom_changed (
  GtkEventControllerScroll* evt,
  gdouble dx,
  gdouble dy,
  gpointer user_data
)
{

  RetroCanvas *self = RETRO_CANVAS (user_data);


  int posx = (self->mx - self->px);
  int posy = (self->my - self->py);

  if (posx < 0 || posy < 0)
    return TRUE;

  if (posx > (self->xw - self->px) ||
      posy > (self->yh - self->py))
    return TRUE;


  if (dy < 0.0) {
    self->scale++;
    if (self->scale > 5) {
      self->scale = 5;
      goto end;
    }

    int rect_cx = posx / c_pow (self->width_rect, (self->scale - 1));
    int rect_cy = posy / c_pow (self->height_rect, (self->scale - 1));

    posx += posx - rect_cx - 1;
    posy += posy - rect_cy - 1;

    self->px = self->mx - posx;
    self->py = self->my - posy;

  } else {
    self->scale--;
    if (self->scale < 1) {
      self->scale = 1;
      goto end;
    }

    int rect_cx = posx / c_pow (self->width_rect, (self->scale + 1));
    int rect_cy = posy / c_pow (self->height_rect, (self->scale + 1));

    posx = (posx + rect_cx + 1) / 2;
    posy = (posy + rect_cy + 1) / 2;

    self->px = self->mx - posx;
    self->py = self->my - posy;
  }

end:
  gtk_widget_queue_draw (GTK_WIDGET (self));

  return TRUE;
}

static void
moving_canvas (RetroCanvas *self,
               gdouble                  x,
               gdouble                  y)
{
    self->px += self->mx - self->ox;
    self->py += self->my - self->oy;
    self->ox = self->mx;
    self->oy = self->my;

    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
event_motion (GtkEventControllerMotion *evt,
              gdouble                   x,
              gdouble                   y,
              gpointer                  user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);
  self->mx = x;
  self->my = y;

  if (self->is_2_btn_pressed) {
    moving_canvas (self, x, y);
  }

  if (x >= self->px && x <= self->xw) {
    if (y >= self->py && y <= self->yh) {
      if (self->tool > 0) {
        self->drawing_tool = TRUE;
        gtk_widget_queue_draw (GTK_WIDGET (self));
      } else {
        self->drawing_tool = FALSE;
      }
    } else {
      self->drawing_tool = FALSE;
    }

  } else {
    self->drawing_tool = FALSE;
  }

}

static void
mouse_mov_canvas_press (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);

  self->ox = x;
  self->oy = y;
  self->is_2_btn_pressed = TRUE;
}

static void
mouse_mov_canvas_release (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);

  self->is_2_btn_pressed = FALSE;
}

static void
mouse_hit_canvas_release (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
   RetroCanvas *self = RETRO_CANVAS (user_data);
  self->is_0_btn_pressed = FALSE;
}

static void
select_index_color (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);
  guint32 xx = 0;
  guint32 max_index_color = global_get_max_index ();

  for (guint32 index_color = 0; index_color < max_index_color; index_color++) {
	  if (x >= xx && x <= (xx + 32)) {
		  self->selected_index_color = index_color;
		  RetroCanvas *main_canvas = retro_canvas_get_drawing_canvas ();
		  retro_canvas_set_index (RETRO_CANVAS (main_canvas), index_color);
		  break;
	  }
	  xx += 32;
  }
}

void
retro_canvas_set_index_colours_by_index (RetroCanvas * self, guint32 index_id, guint32 index_color)
{
	self->index_color[index_id] = index_color;	
}

void
retro_canvas_set_colours_by_index (RetroCanvas * self, guint32 index_id, guint32 index_color)
{
  //guint32 *pcolours = global_type_palette_get_cur_ptr_palette (0);
}

void
retro_canvas_set_index_id (RetroCanvas *self, guint32 index)
{
	self->index_id = index;
}

void
retro_canvas_set_item_id (RetroCanvas *self,
										guint32 								id)
{
	self->item_id = id;
}

guint32
retro_canvas_get_item_id (RetroCanvas *self)
{
	return self->item_id;
}

static void
select_index_color_for_palette (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);
  guint32 xx = 0;
  guint32 yy = 0;
	guint32 w = self->width_rect * self->scale * 16;
	guint32 h = self->height_rect * self->scale * 4;
	guint32 index_color = 0;

	int found = 0;
	for (; yy < h; yy += self->height_rect * self->scale) {
		xx = 0;
		for (; xx < w; xx += self->width_rect * self->scale) {
	  	if ((x >= xx) && (x <= (xx + self->width_rect * self->scale))) {
				if ((y >= yy) && (y <= (yy + self->height_rect * self->scale))) {
					found = 1;
			  	self->selected_index_color = index_color;
					retro_canvas_set_index_colours_by_index (RETRO_CANVAS (self->child_one_color), self->index_id, index_color);
					global_colour_save_to_banks ();
					gtk_widget_queue_draw (GTK_WIDGET (self->child_one_color));
					nes_list_palette_redraw ();
					nes_current_palette_redraw ();
		  		break;
				}
	  	}
			index_color++;
		}
		if (found)
			break;
  }
}

void
retro_canvas_set_xy_click (RetroCanvas *self)
{
	gtk_widget_add_controller (GTK_WIDGET (self),
			GTK_EVENT_CONTROLLER (self->gesture_click_select_one_color));
}

void
retro_canvas_set_child_color (RetroCanvas *self,
		RetroCanvas *child)
{
	self->child_one_color = child;
}

static void
clear_tile_selection (RetroCanvas *self)
{
	guint32 t = 16 * 16; 
	for (guint32 it = 0; it < t; it++) {
		self->tile_background_pos[it] = 0;
	}
}

static void
calculate_megatile (RetroCanvas *self, int gx, int gy)
{
	self->mx = gx;
	self->my = gy;
	int x, y;
	x = y = 0;
	calculate_last_block_and_point (self, &x, &y);

	self->last_blockx /= 4 % 8;
	self->last_blocky /= 4 % 7;
}

static void
set_palette_megatile (RetroCanvas *self)
{
	guint32 mega_palettes[] = {
		0xc0,
		0x30,
		0x0c,
		0x03
	};

	int indx = self->last_blocky * 8 + self->last_blockx;
	int offset = 6;
	for (int i = 0; i < 4; i++) {
		GtkWidget *frame = nes_screen_background_get_frame_megatile (i);
		GtkWidget *canvas = nes_frame_megatile_get_canvas (NES_FRAME_MEGATILE (frame));

		guint8 palette = ((mega_palettes[i] & self->megatile[indx]) >> offset);

		guint32 *index_colour = global_nes_palette_get_memory_index (palette);
		retro_canvas_set_index_colours (RETRO_CANVAS (canvas), index_colour);
		gtk_widget_queue_draw (GTK_WIDGET (canvas));

		offset -= 2;
	}
}

guint8 *
retro_canvas_nes_get_megatile_by_block (RetroCanvas *self)
{
	int indx = self->last_blocky * 8 + self->last_blockx;

	return &self->megatile[indx];
}

guint8 *
retro_canvas_get_megatile (RetroCanvas *self)
{
	return self->megatile;
}

static void
mouse_hit_canvas_press (GtkGestureClick *evt,
         gint             n_press,
         gdouble          x,
         gdouble          y,
         gpointer         user_data)
{
  RetroCanvas *self = RETRO_CANVAS (user_data);

  self->is_0_btn_pressed = TRUE;

  if (self->tool > 0) {

    NesParamPoint n;
		NesPalette *nes = NULL;

		switch (self->tool) {
			case INDX_TOOL_PENCIL:
    		n.blockx = self->last_blockx;
    		n.blocky = self->last_blocky;
    		n.x = self->last_pointx;
    		n.y = self->last_pointy;
    		nes = nes_palette_get ();
    		nes_palette_set_color (nes, &n, self->selected_index_color + 1);
				break;

			case INDX_TOOL_COPY_TILE_SRC:
				clear_tile_selection (self);
				self->first_tile_x = -1;
				self->first_tile_y = -1;
				break;
			case INDX_TOOL_COPY_TILE_DST:
				break;
			case INDX_TOOL_NES_MEGATILE:
				calculate_megatile (self, x, y);
				set_palette_megatile (self);
				break;
		}

    gtk_widget_queue_draw (GTK_WIDGET (self));
  }

}


static void
retro_canvas_init (RetroCanvas *self)
{
	global_nes_alloc_tile_background ();
	self->tile_background_pos = global_nes_get_tile_background ();
	self->tile_start_x = -1;
	self->tile_start_y = -1;
	self->tile_end_x = -1;
	self->tile_end_y = -1;
	self->draw_blank = 0;
	self->child_one_color = NULL;
  self->colours = NULL;
  self->count_colours = 0;
  self->left_top = FALSE;
  self->mx = 0;
  self->my = 0;
  self->px = 0;
  self->py = 0;
  self->is_2_btn_pressed = FALSE;
  self->is_0_btn_pressed = FALSE;
  self->tool = 0;
  self->drawing_tool = FALSE;
  self->show_hex_index = FALSE;

	self->megatile = g_malloc0 (sizeof (guint8) * NES_MEGATILE_COUNT);

	self->tile_ref = g_malloc0 (NES_SCREEN_SIZE * sizeof (TileRef));
	for (guint32 i = 0; i < NES_SCREEN_SIZE; i++) {
		self->tile_ref[i].tilex = -1;
		self->tile_ref[i].tiley = -1;
	}

	guint32 t = 16 * 16;
	for (guint32 it = 0; it < t; it++) {
		self->tile_background_pos[it] = 0;
	}

  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (self), draw_canvas,
                                  NULL, NULL);

  self->event_zoom = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  g_signal_connect (self->event_zoom, "scroll",
                    G_CALLBACK (zoom_changed),
                    self);

  self->event_motion = gtk_event_controller_motion_new ();

  g_signal_connect (self->event_motion,
                   "motion",
                   G_CALLBACK (event_motion),
                   self);

  self->gesture_click_move_canvas = gtk_gesture_click_new ();
  self->gesture_click_tool_press  = gtk_gesture_click_new ();
  self->gesture_click_select_index_color = gtk_gesture_click_new ();
  self->gesture_click_select_one_color = gtk_gesture_click_new ();

  g_signal_connect (self->gesture_click_move_canvas, "pressed",
                    G_CALLBACK (mouse_mov_canvas_press),
                    self);

  g_signal_connect (self->gesture_click_move_canvas, "released",
                    G_CALLBACK (mouse_mov_canvas_release),
                    self);

  g_signal_connect (self->gesture_click_tool_press, "pressed",
                    G_CALLBACK (mouse_hit_canvas_press),
                    self);

  g_signal_connect (self->gesture_click_tool_press, "released",
                    G_CALLBACK (mouse_hit_canvas_release),
                    self);

  g_signal_connect (self->gesture_click_select_index_color, "pressed",
		  G_CALLBACK (select_index_color),
		  self);

  g_signal_connect (self->gesture_click_select_one_color, "pressed",
		  G_CALLBACK (select_index_color_for_palette),
		  self);

  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (self->gesture_click_select_one_color), 1);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (self->gesture_click_select_index_color), 1);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (self->gesture_click_tool_press), 1);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (self->gesture_click_move_canvas), 2);
}

void 
retro_canvas_shut_on_event_click (RetroCanvas *self)
{
	gtk_widget_add_controller (GTK_WIDGET (self),
			GTK_EVENT_CONTROLLER (self->gesture_click_select_index_color));
}

void 
retro_canvas_shut_on_events (RetroCanvas *self)
{
  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->event_motion));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->event_zoom));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->gesture_click_move_canvas));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->gesture_click_tool_press));
}

void 
retro_canvas_shut_on_events_nes_screen (RetroCanvas *self)
{
  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->event_motion));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->event_zoom));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->gesture_click_move_canvas));

  gtk_widget_add_controller (GTK_WIDGET (self),
                             GTK_EVENT_CONTROLLER (self->gesture_click_tool_press));
}


RetroCanvas *
retro_canvas_get_tileset (void)
{
  return global_drawing_canvas_tileset;
}

void 
retro_canvas_set_tileset (RetroCanvas *self)
{
  global_drawing_canvas_tileset = self;
}

RetroCanvas *
retro_canvas_get_drawing_canvas (void)
{
  return global_drawing_canvas;
}

void 
retro_canvas_set_drawing_canvas (RetroCanvas *self)
{
  global_drawing_canvas = self;
}

void 
retro_canvas_set_tool (RetroCanvas *self,
                      guint32                  tool)
{
  self->tool = tool;
}

void
retro_canvas_set_copy (RetroCanvas *self, guint32 is_copy)
{
	self->is_copy = is_copy;
}
