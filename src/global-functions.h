#pragma once
#include <gtk/gtk.h>

enum {
	NO_PLATFORM,
  PLATFORM_PALETTE_NES,
  N_PLATFORM_PALETTE
};

enum {
	NES_MODE_8X8,
	NES_MODE_8X16,
	N_NES_MODE
};

enum {
  NES_TYPE_PALETTE_2C02,
  N_NES_TYPE_PALETTE
};

typedef struct _DataForOutput {
	guint8 *data;
	gsize size;
} DataForOutput;

typedef struct _TileRef {
	gint32 tilex;
	gint32 tiley;
} TileRef;

void global_get_data_for_output (DataForOutput *st);
guint32 global_get_max_index (void);
guint32 global_type_palette_get_cur_platform (void);
guint32 global_type_palette_get_cur_palette (void);
guint32 *global_type_palette_get_cur_ptr_palette (guint32 index);
void global_type_palette_set_cur (guint32 index, guint32 typen);
void global_set_colour_index_for_palette (int item_id, int index_id, int index_colour);
void global_set_cur_bank (guint32 index);
guint32 global_get_cur_bank (void);
void global_colour_save_to_banks (void);
void global_nes_palette_alloc_memory_index (guint32 index, gsize size);
guint32 *global_nes_palette_get_memory_index (guint32 index);
void global_nes_palette_init_map (guint32 index);
void global_nes_palette_alloc_maps (void);
void *global_nes_get_map (guint32 indx);
void *global_nes_screen_get_map (guint32 indx);
void global_nes_alloc_tile_background (void);
guint8 *global_nes_get_tile_background (void);
int global_get_screen_count (void);
void global_set_screen_count (gint32 count);
void global_set_screens (GtkWidget **screens);
GtkWidget *global_get_screen (guint32 index);
void global_nes_create_screens (void);
int global_nes_get_cur_screen (void);
void global_nes_set_cur_screen (int indx);
guint8 global_convert_setup_megatile_to_nes (guint8 megatile);
void global_set_fulldump_button (guint8 *b);
guint8 *global_get_fulldump_button (void);
void global_nes_set_height_alloc (guint32 size);
void global_nes_set_height_by (guint32 index, guint32 mode);
guint32 global_get_height_by (guint32 index);
guint32 global_nes_get_count_height (void);
void global_restructure_canvas (void);
