#pragma once
#include <gtk/gtk.h>

enum {
  PLATFORM_PALETTE_NES,
  N_PLATFORM_PALETTE
};

enum {
  NES_TYPE_PALETTE_2C02,
  N_NES_TYPE_PALETTE
};

typedef struct _DataForOutput {
	guint8 *data;
	gsize size;
} DataForOutput;

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
