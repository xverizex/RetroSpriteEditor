#pragma once
#include <gtk/gtk.h>
#include <libxml/parser.h>

void project_set_folder_and_name (const char *folder, const char *name);
char * project_get_filepath_to_export (void);
void project_save_nes (void);
void project_open_nes (char *filepath);
void project_nes_export_screen (void);
void project_import_nes_chr (char *chr);
