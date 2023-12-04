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
