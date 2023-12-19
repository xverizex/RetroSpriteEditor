#include "global-functions.h"
#include "nes-palette.h"
#include "retro-canvas.h"
#include "nes-list-palettes.h"
#include "nes-item-palette.h"
#include "nes-palette.h"
#include "nes-setup-palette.h"
#include "nes-screen-background.h"

static guint32 cur_platform;
static guint32 cur_palette;

guint32 colours_2c02[64] = {
  0x626262, // 00
  0xae2e00, // 01
  0xc32706, // 02
  0xae2447, // 03
  0x731d6b, // 04
  0x241679, // 05
  0x04186d, // 06
  0x052a4f, // 07
  0x09422b, // 08
  0x0e541f, // 09
  0x105921, // 0A
  0x28501c, // 0B
  0x743e07, // 0C
  0x000000, // 0D
  0x000000, // 0E
  0x000000, // 0F
  0xababab, // 10
  0xf96200, // 11
  0xf9473a, // 12
  0xf93e7c, // 13
  0xd13aaf, // 14
  0x6833c6, // 15
  0x0e3cbd, // 16
  0x125797, // 17
  0x1a7865, // 18
  0x219344, // 19
  0x249e41, // 1a
  0x49943b, // 1b
  0xb17d25, // 1c
  0x000000, // 1d
  0x000000, // 1e
  0x000000, // 1f
  0xffffff, // 20
  0xfbaf5e, // 21
  0xfa8c8a, // 22
  0xfa75c7, // 23
  0xfa6ef0, // 24
  0xcc6ff1, // 25
  0x5b7ff4, // 26
  0x28a0f2, // 27
  0x30c4c0, // 28
  0x38e192, // 29
  0x3fef76, // 2a
  0x86e967, // 2b
  0xf4d357, // 2c
  0x4e4e4e, // 2d
  0x000000, // 2e
  0x000000, // 2f
  0xffffff, // 30
  0xfde1bb, // 31
  0xfdd3cd, // 32
  0xfcc7e3, // 33
  0xfcc2f7, // 34
  0xf2c2f7, // 35
  0xc4c8f9, // 36
  0x9ed5fb, // 37
  0x89e4ea, // 38
  0x8af0d5, // 39
  0xa1f6c2, // 3a
  0xc6f6b7, // 3b
  0xf3eeb3, // 3c
  0xb8b8b8, // 3d
  0x000000, // 3e
  0x000000  // 3f
};

guint32 
global_type_palette_get_cur_platform (void)
{
  return cur_platform;
}

guint32 
global_type_palette_get_cur_palette (void)
{
  return cur_palette;
}

void 
global_type_palette_set_cur (guint32 index, guint32 typen)
{
  cur_platform = index;
  cur_palette = typen;
}

static guint32 *
get_ptr_colour_nes (guint32 index)
{
  switch (cur_palette)
    {
    case NES_TYPE_PALETTE_2C02:
      return &colours_2c02[index];
		default:
			break;
    }

  return NULL;
}

static guint32
get_max_index_nes (void)
{
	return 4;
}

guint32 
global_get_max_index (void)
{
  switch (cur_platform)
    {
    case PLATFORM_PALETTE_NES:
      return get_max_index_nes ();
		default:
			break;
    }

  return 0;
}

static guint32 *indexed_colors_palette[4];

void 
global_nes_palette_alloc_memory_index (guint32 index, gsize size)
{
	indexed_colors_palette[index] = g_malloc0 (size);
}

guint32 *
global_nes_palette_get_memory_index (guint32 index)
{
	return indexed_colors_palette[index];
}

static guint8 *tile_background;

void
global_nes_alloc_tile_background ()
{
	if (tile_background == NULL)
		tile_background = g_malloc0 (16 * 16);
}

guint8 *
global_nes_get_tile_background ()
{
	return tile_background;
}

typedef struct _FormData {
	DataForOutput *st;
	int cnvs_width;
	int cnvs_height;
	int blkx;
	int blky;
	int map_index;
} FormData;

#define TILE_SIZE             16
#define NES_NAMETABLE_SIZE   256 
#define COUNT_NAMETABLES       2

static void
form_data (FormData *fd)
{
	DataForOutput *st = fd->st;

	gsize offset = fd->map_index * TILE_SIZE * NES_NAMETABLE_SIZE;

	guint8 *s = &st->data[offset];

	guint32 heighty = fd->cnvs_height / fd->blky;
	guint32 widthx  = fd->cnvs_width  / fd->blkx;

	for (guint32 yy = 0; yy < heighty; yy++) {
		for (guint32 xx = 0; xx < widthx; xx++) {
			NesTilePoint *n = nes_palette_get_block (fd->blkx, fd->blky, xx, yy, fd->map_index);

			guint8 tile[16] = {0, };
			guint indx = 0;
			guint ii = 0;
			for (gint32 y = 0; y < 8; y++) {
				guint8 pal = 0;
				for (gint32 x = 7; x >= 0; x--) {
					if ((n[ii].index == 1) || (n[ii].index == 3)) {
						pal |= (1 << x);
					}
					ii++;
				}
				tile[indx++] = pal;
			}
			ii = 0;
			for (gint32 y = 0; y < 8; y++) {
				guint8 pal = 0;
				for (gint32 x = 7; x >= 0; x--) {
					if ((n[ii].index == 2) || (n[ii].index == 3)) {
						pal |= (1 << x);
					}
					ii++;
				}
				tile[indx++] = pal;
			}

			for (guint i = 0; i < 16; i++) {
					*s = tile[i];
					s++;
			}
		}
	}
}

static void
form_data_nes (DataForOutput *st)
{
	st->size = NES_NAMETABLE_SIZE * COUNT_NAMETABLES * TILE_SIZE;
	guint8 *data = g_malloc0 (st->size);
	st->data = data;

	FormData fd;
	fd.st = st;
	fd.cnvs_width = 128;
	fd.cnvs_height = 128;
	fd.blkx = 8;
	fd.blky = 8;
	fd.map_index = NES_BACKGROUND;
	form_data (&fd);

	fd.st = st;
	fd.cnvs_width = 128;
	fd.cnvs_height = 128;
	fd.blkx = 8;
	fd.blky = 8;
	fd.map_index = NES_SPRITE;
	form_data (&fd);
}

void 
global_get_data_for_output (DataForOutput *st)
{
	switch (cur_platform)
	{
		case PLATFORM_PALETTE_NES:
			form_data_nes (st);
			break;
		default:
			break;
	}
}

static guint32 cur_bank;

void
global_set_cur_bank (guint32 index)
{
	cur_bank = index;
}

guint32
global_get_cur_bank (void)
{
	return cur_bank;
}

static void
inner_nes_set_colour_index_for_palette (int item_id, int index_id, int index_colour)
{
	NesBanks *bank = nes_list_palette_get_bank ();
	bank->bank[cur_bank][item_id][index_id] = index_colour;
}

static void
inner_colour_save_to_banks (void)
{
	NesBanks *bank = nes_list_palette_get_bank ();

	GtkWidget **items = nes_list_palette_get_items ();
  for (guint i = 0; i < 4; i++) {
    guint32 *indx = nes_item_palette_get_index_colours (NES_ITEM_PALETTE (items[i]));
    bank->bank[cur_bank][i][0] = *(indx + 0);
    bank->bank[cur_bank][i][1] = *(indx + 1);
    bank->bank[cur_bank][i][2] = *(indx + 2);
    bank->bank[cur_bank][i][3] = *(indx + 3);
  }
}

void
global_colour_save_to_banks (void)
{
	switch (cur_platform)
	{
		case PLATFORM_PALETTE_NES:
			inner_colour_save_to_banks ();
			break;
		default:
			break;
	}
}

void
global_set_colour_index_for_palette (int item_id, int index_id, int index_colour)
{
	switch (cur_platform)
	{
		case PLATFORM_PALETTE_NES:
			inner_nes_set_colour_index_for_palette (item_id, index_id, index_colour);
			break;
		default:
			break;
	}
}

guint32 *
global_type_palette_get_cur_ptr_palette (guint32 index)
{
  switch (cur_platform)
    {
    case PLATFORM_PALETTE_NES:
      return get_ptr_colour_nes (index);
		default:
			break;
    }

  return NULL;
}

static void
global_nes_screen_init (NesTilePoint *p)
{
	guint32 i = 0;
	guint32 blkx = 0;
	guint32 blky = 0;
	guint32 ax = 0;
	guint32 ay = 0;
	for (guint32 y = 0; y < 240; y++) {
		for (guint32 x = 0; x < 256; x++) {
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

static void
global_nes_background_init (NesTilePoint *p)
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

static void
global_nes_sprite_init (NesTilePoint *p)
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

static NesTilePoint *nes_map[2];
static NesTilePoint *nes_screen_map[1];

void
global_nes_palette_alloc_maps (void)
{
	nes_map[0] = g_malloc0 (sizeof (NesTilePoint) * 128 * 128);
	nes_map[1] = g_malloc0 (sizeof (NesTilePoint) * 128 * 128);
	nes_screen_map[0] = g_malloc0 (sizeof (NesTilePoint) * 256 * 224);
	global_nes_screen_init (nes_screen_map[0]);
}

void *
global_nes_get_map (guint32 indx)
{
	return (NesTilePoint *) nes_map[indx];
}

void *
global_nes_screen_get_map (guint32 indx)
{
	return (NesTilePoint *) nes_screen_map[indx];
}

void
global_nes_palette_init_map (guint32 type)
{
	switch (type)
	{
		case NES_SPRITE:
			global_nes_sprite_init (nes_map[type]);
			break;
		case NES_BACKGROUND:
			global_nes_background_init (nes_map[type]);
			break;
		default:
			break;
	}
}

static gint32 screen_count;
void
global_set_screen_count (gint32 count)
{
	if (count < 0)
		count = 0;
	screen_count = count;
}

int
global_get_screen_count ()
{
	return screen_count;
}

static GtkWidget **screens;

void
global_set_screens (GtkWidget **scr)
{
	screens = scr;
}

GtkWidget *
global_get_screen (guint32 indx)
{
	return screens[indx];
}

void
global_nes_create_screens ()
{
	nes_screen_background_create_tabs_screen ();
}

static int cur_page;

void
global_nes_set_cur_screen (int indx)
{
	cur_page = indx;
}

int
global_nes_get_cur_screen ()
{
	return cur_page;
}

guint8
global_convert_setup_megatile_to_nes (guint8 megatile)
{
	guint8 ret = 0;
	ret |= (megatile << 6) & 0xc0; 
	ret |= (megatile << 2) & 0x30;
	ret |= (megatile >> 2) & 0x0c;
	ret |= (megatile >> 6) & 0x03;

	return ret;
}
