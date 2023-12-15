#include "project.h"
#include "nes-list-palettes.h"
#include "nes-item-palette.h"
#include "nes-palette.h"
#include "retro-canvas.h"
#include "global-functions.h"
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <stdio.h>
#include <string.h>

#define  COUNT_PALETTES               2
#define  COUNT_COLOURS                4
typedef struct _NesConfig {
	int nes_p0 [COUNT_PALETTES] [COUNT_COLOURS];
	int nes_p1 [COUNT_PALETTES] [COUNT_COLOURS];
	int nes_p2 [COUNT_PALETTES] [COUNT_COLOURS];
	int nes_p3 [COUNT_PALETTES] [COUNT_COLOURS];
	int nes_np0;
	int nes_np1;
	int nes_np2;
	int nes_np3;
	int nes_pal;
} NesConfig;

struct Project {
	char *folder_path;
	char *name;
	char *name_screen;
	char *fullpath_to_project;
	char *file_to_export;
	NesConfig nes;
};

struct Project *prj;
static void open_nes_ref (void);
static void open_nes_screen_palettes (void);

void project_free (void)
{
	if (prj->name_screen) {
		g_free (prj->name_screen);
		prj->name_screen = NULL;
	}

	if (prj->folder_path) {
		g_free (prj->folder_path);
		prj->folder_path = NULL;
	}

	if (prj->name) {
		g_free (prj->name);
		prj->name = NULL;
	}

	if (prj->fullpath_to_project) {
		g_free (prj->fullpath_to_project);
		prj->fullpath_to_project = NULL;
	}

	if (prj->file_to_export) {
		g_free (prj->file_to_export);
		prj->file_to_export = NULL;
	}
}

static void
project_alloc (void)
{
	prj = g_malloc0 (sizeof (struct Project));
}

static void
project_free_and_alloc (void)
{
	project_free ();
	project_alloc ();
}

static void
write_header (xmlTextWriterPtr *writ)
{
	if (prj->name_screen == NULL) {
		prj->name_screen = g_strdup ("screen");
	}
	xmlTextWriterPtr writer = *writ;
	gchar screen_palettes[512];
	gchar screen_ref[512];
	snprintf (screen_palettes, 512, "%s/%s.palettes", prj->folder_path, prj->name_screen);
	snprintf (screen_ref, 512, "%s/%s.ref", prj->folder_path, prj->name_screen);

	writer = xmlNewTextWriterFilename (prj->fullpath_to_project, 0);
	xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL);
	xmlTextWriterStartElement (writer, "RetroSpriteEditor");
	xmlTextWriterStartElement (writer, "ProjectSettings");
			xmlTextWriterWriteElement (writer, "platform_id", "0");
			xmlTextWriterWriteElement (writer, "project_name", prj->name);
			xmlTextWriterWriteElement (writer, "project_folder", prj->folder_path);
			xmlTextWriterWriteElement (writer, "screen", prj->name_screen);
			xmlTextWriterWriteElement (writer, "fullpath", prj->fullpath_to_project);
	xmlTextWriterEndElement (writer);
	xmlTextWriterStartElement (writer, "ExportSettings");
			xmlTextWriterWriteElement (writer, "outfile", prj->file_to_export);
	xmlTextWriterEndElement (writer);

	*writ = writer;
}

static void write_nes_palettes (xmlTextWriterPtr writer);

void 
project_set_open_folder_and_name (const char *folder, const char *name)
{
	if (prj) {
		project_free_and_alloc ();
	} else {
		project_alloc ();
	}

	prj->folder_path = g_strdup (folder);
	memset (&prj->nes, 0, sizeof (NesConfig));
}

void 
project_set_folder_and_name (const char *folder, const char *name)
{
	if (prj) {
		project_free_and_alloc ();
	} else {
		project_alloc ();
	}

	prj->folder_path = g_strdup (folder);
	prj->name = g_strdup (name);
	prj->fullpath_to_project = g_strdup_printf ("%s/%s.rse", folder, name);
	prj->file_to_export = g_strdup_printf ("%s/%s.chr", folder, name);
	prj->name_screen = g_strdup_printf ("screen");
	memset (&prj->nes, 0, sizeof (NesConfig));

	xmlTextWriterPtr writer;
	write_header (&writer);
	write_nes_palettes (writer);
	xmlTextWriterEndElement (writer);
	xmlTextWriterEndDocument (writer);
	xmlFreeTextWriter (writer);
}

char *
project_get_filepath_to_export ()
{
	if (prj->fullpath_to_project == NULL)
		return NULL;

	return prj->file_to_export;
}

static const xmlChar *pname;

enum {
	TYPE_INT,
	TYPE_STRING,
	N_TYPE
};

static void
check_and_write (const xmlChar *name, char *sname, const xmlChar *value, void **v, int type)
{
	char *v0 = NULL;
	int  v1 = 0;
	if (strncmp (name, sname, strlen (sname) + 1))
		return;

	switch (type) {
		case TYPE_STRING:
			v0 = g_strdup (value);
			*v = v0;
			break;
		case TYPE_INT:
			v1 = atoi (value);
			*v = &v1;
			break;
	}
}

static void
handle_nes_name_value (const xmlChar *name, const xmlChar *value)
{
	check_and_write (name, "project_name", value, (void **) &prj->name, TYPE_STRING);
	check_and_write (name, "project_folder", value, (void **) &prj->folder_path, TYPE_STRING);
	check_and_write (name, "fullpath", value, (void **) &prj->fullpath_to_project, TYPE_STRING);
	check_and_write (name, "outfile", value, (void **) &prj->file_to_export, TYPE_STRING);
	check_and_write (name, "screen", value, (void **) &prj->name_screen, TYPE_STRING);
	if (!strncmp (name, "p00", 4)) {
		unsigned int val = atoi (value);
		prj->nes.nes_p0[prj->nes.nes_pal][prj->nes.nes_np0++] = val;
	}
	if (!strncmp (name, "p01", 4)) {
		unsigned int val = atoi (value);
		prj->nes.nes_p1[prj->nes.nes_pal][prj->nes.nes_np1++] = val;
	}
	if (!strncmp (name, "p02", 4)) {
		unsigned int val = atoi (value);
		prj->nes.nes_p2[prj->nes.nes_pal][prj->nes.nes_np2++] = val;
	}
	if (!strncmp (name, "p03", 4)) {
		unsigned int val = atoi (value);
		prj->nes.nes_p3[prj->nes.nes_pal][prj->nes.nes_np3++] = val;
	}
	if (prj->nes.nes_np3 == 4) {
		prj->nes.nes_pal++;
		prj->nes.nes_np0 = 0;
		prj->nes.nes_np1 = 0;
		prj->nes.nes_np2 = 0;
		prj->nes.nes_np3 = 0;
	}
}

static void
process_nes_node (xmlTextReaderPtr reader)
{
	const xmlChar *name, *value;
	name = xmlTextReaderConstName (reader);
	value = xmlTextReaderConstValue (reader);

	if (value == NULL) {
		pname = name;
	} else if (!strncmp (name, "#text", 6)) {
		//g_print ("%s == %s\n", pname, value);
		handle_nes_name_value (pname, value);
	}
}

static void
read_tilemap_and_set_nes (void)
{
	GFile *file = g_file_new_for_path (prj->file_to_export);
	GFileInputStream *input = g_file_read (file,
			NULL,
			NULL);

	unsigned char *data = g_malloc0 (8192);
	g_input_stream_read (G_INPUT_STREAM (input),
			data,
			8192,
			NULL,
			NULL);

	g_input_stream_close (G_INPUT_STREAM (input), NULL, NULL);

	int blkx = 0;
	int blky = 0;
	int yy = 0;
	int xx = 0;
	unsigned char *t = data;
	for (int i = 0; i < 2; i++) {
		blkx = 0;
		blky = 0;
		for (int tile = 0; tile < 256; tile++) {
			int index = 0;

			for (int y = 0; y < 8; y++) {
				for (int x = 7; x >= 0; x--) {
					int found = 0;
					unsigned char bit = 1 << x;
					if ((t[y] & bit) && (t[y + 8] & bit)) {
						index = 3;
						found = 1;
					} else {
						if ((t[y] & bit)) {
							index = 1;
							found = 1;
						}
						if ((t[y + 8] & bit)) {
							index = 2;
							found = 1;
						}
					}

					yy = y;
					xx = 7 - x;

					NesParamPoint n;
		    	n.blockx = blkx;
   				n.blocky = blky;
   				n.x = xx;
   				n.y = yy;
   				NesPalette *nes = nes_palette_get ();

					if (found > 0) {
    				nes_palette_set_color_with_map (nes, &n, index + 1, i);
					} else {
    				nes_palette_set_color_with_map (nes, &n, 0, i);
					}
				}
			}
			t += 16;
			blkx++;
			if (blkx >= 16) {
				blkx = 0;
				blky++;
			}
		}
	}

	g_free (data);
}

static void
parse_and_set_project_header (char *filepath)
{
	char *folder = g_strdup (filepath);
	char *name = strrchr (folder, '/');
	if (name) {
		*name = 0;
		name++;
	} else {
		name = strrchr (folder, '\\');
		if (name) {
			*name = 0;
			name++;
		}
	}

	char *n = strstr (name, ".rse");
	if (n)
		*n = 0;
	project_set_open_folder_and_name (folder, name);
}

void
project_open_nes (char *filepath)
{
	parse_and_set_project_header (filepath);

	xmlTextReaderPtr reader;
	reader = xmlNewTextReaderFilename (filepath);
	int ret = xmlTextReaderRead (reader);
	while (ret == 1) {
		process_nes_node (reader);
		ret = xmlTextReaderRead (reader);
	}

	xmlFreeTextReader (reader);
	NesBanks *bank = nes_list_palette_get_bank ();
	for (guint32 pl = 0; pl < 2; pl++) {
		for (guint32 i = 0; i < 4; i++) {
			bank->bank[pl][i][0] = prj->nes.nes_p0[pl][i];
			bank->bank[pl][i][1] = prj->nes.nes_p1[pl][i];
			bank->bank[pl][i][2] = prj->nes.nes_p2[pl][i];
			bank->bank[pl][i][3] = prj->nes.nes_p3[pl][i];
		}
	}

	GtkWidget **items = nes_list_palette_get_items ();
	for (guint32 i = 0; i < 4; i++) {
		guint32 *color_index = nes_item_palette_get_colour_index (NES_ITEM_PALETTE (items[i]));
		guint32 cur_bank = global_get_cur_bank ();
		*(color_index + 0) = prj->nes.nes_p0[cur_bank][i];
		*(color_index + 1) = prj->nes.nes_p1[cur_bank][i];
		*(color_index + 2) = prj->nes.nes_p2[cur_bank][i];
		*(color_index + 3) = prj->nes.nes_p3[cur_bank][i];
		nes_item_palette_get_color_from_index (NES_ITEM_PALETTE (items[i]));
	}

	read_tilemap_and_set_nes ();
	open_nes_ref ();
	open_nes_screen_palettes ();
}

static void
write_nes_palettes (xmlTextWriterPtr writer)
{
	//GtkWidget **items = nes_list_palette_get_items ();

	xmlTextWriterStartElement (writer, "Palettes");
	const char *palettes[] = {
		"Sprites",
		"Background"
	};
	unsigned int size_pl = sizeof (palettes) / sizeof (void *);

	for (guint32 pl = 0; pl < size_pl; pl++) {
		xmlTextWriterStartElement (writer, palettes[pl]);
		for (guint32 i = 0; i < 4; i++) {
			NesBanks *bank = nes_list_palette_get_bank ();
			char id[15];
			snprintf (id, 15, "Palette%02d", i);
			xmlTextWriterStartElement (writer, id);
			for (guint32 m = 0; m < 4; m++) {
				char index_num[15];
				char num[15];
				snprintf (index_num, 15, "%d", bank->bank[pl][i][m]);
				snprintf (num, 15, "p%02d", m);
				xmlTextWriterWriteElement (writer, num, index_num);
			}
			xmlTextWriterEndElement (writer);
		}
		xmlTextWriterEndElement (writer);
	}
	xmlTextWriterEndElement (writer);
}

static void
open_nes_ref ()
{
	gchar screen_ref[512];
	snprintf (screen_ref, 512, "%s/%s.ref", prj->folder_path, prj->name_screen);
	GFile *file_ref = g_file_new_for_path (screen_ref);

	if (!g_file_query_exists (file_ref, NULL)) {
		return;
	}
	GFileInputStream *in = NULL;
	in = g_file_read (file_ref,
			NULL,
			NULL
			);

	TileRef *tile = retro_canvas_screen_nes_get_tile_ref ();
	int screen_size = 32 * 28;
	for (int i = 0; i < screen_size; i++) {
		g_input_stream_read (G_INPUT_STREAM (in), &tile[i], sizeof (TileRef), NULL, NULL);
	}

	g_input_stream_close (G_INPUT_STREAM (in), NULL, NULL);
}

static void
open_nes_screen_palettes ()
{
	gchar screen_palettes[512];
	snprintf (screen_palettes, 512, "%s/%s.palettes", prj->folder_path, prj->name_screen);

	GFile *file_ref = g_file_new_for_path (screen_palettes);

	if (!g_file_query_exists (file_ref, NULL)) {
		return;
	}
	GFileInputStream *in = NULL;
	in = g_file_read (file_ref,
			NULL,
			NULL
			);

	guint8 *tile = retro_canvas_screen_nes_get_megatile ();
	int screen_size = 8 * 7;
	for (int i = 0; i < screen_size; i++) {
		g_input_stream_read (G_INPUT_STREAM (in), &tile[i], sizeof (guint8), NULL, NULL);
	}

	g_input_stream_close (G_INPUT_STREAM (in), NULL, NULL);
}
static void
save_nes_ref (gchar *screen_ref)
{
	GFile *file_ref = g_file_new_for_path (screen_ref);

	GFileOutputStream *out = NULL;
	if (g_file_query_exists (file_ref, NULL)) {
		out = g_file_replace (file_ref,
				NULL,
				FALSE,
				G_FILE_CREATE_REPLACE_DESTINATION,
				NULL,
				NULL);
	} else {
		out = g_file_create (file_ref,
			G_FILE_CREATE_NONE,
			NULL,
			NULL
			);
	}

	TileRef *tile = retro_canvas_screen_nes_get_tile_ref ();
	int screen_size = 32 * 28;
	for (int i = 0; i < screen_size; i++) {
		g_output_stream_write (G_OUTPUT_STREAM (out), &tile[i], sizeof (TileRef), NULL, NULL);
	}

	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);
}

static void
save_nes_screen_palettes (gchar *screen_palette)
{
	GFile *file_ref = g_file_new_for_path (screen_palette);

	GFileOutputStream *out = NULL;
	if (g_file_query_exists (file_ref, NULL)) {
		out = g_file_replace (file_ref,
				NULL,
				FALSE,
				G_FILE_CREATE_REPLACE_DESTINATION,
				NULL,
				NULL);
	} else {
		out = g_file_create (file_ref,
			G_FILE_CREATE_NONE,
			NULL,
			NULL
			);
	}

	guint8 *tile = retro_canvas_screen_nes_get_megatile ();
	int screen_size = 8 * 7;
	for (int i = 0; i < screen_size; i++) {
		g_output_stream_write (G_OUTPUT_STREAM (out), &tile[i], sizeof (guint8), NULL, NULL);
	}

	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);
}
static void
write_nes_screen (xmlTextWriterPtr writer)
{
	gchar screen_palettes[512];
	gchar screen_ref[512];
	snprintf (screen_palettes, 512, "%s/%s.palettes", prj->folder_path, prj->name_screen);
	snprintf (screen_ref, 512, "%s/%s.ref", prj->folder_path, prj->name_screen);
	
	save_nes_ref (screen_ref);
	save_nes_screen_palettes (screen_palettes);

}

void
project_save_nes (void)
{
	xmlTextWriterPtr writer;
	write_header (&writer);
	write_nes_palettes (writer);
	write_nes_screen (writer);
	xmlTextWriterEndElement (writer);
	xmlTextWriterEndDocument (writer);
	xmlFreeTextWriter (writer);
}

static void
export_to_ca65 (void)
{
	gchar path[512];
	snprintf (path, 512, "%s/screen_ca65.s", prj->folder_path);

	GFile *file = g_file_new_for_path (path);

	GFileOutputStream *out = NULL;
	if (g_file_query_exists (file, NULL)) {
		out = g_file_replace (file,
				NULL,
				FALSE,
				G_FILE_CREATE_REPLACE_DESTINATION,
				NULL,
				NULL);
	} else {
		out = g_file_create (file,
			G_FILE_CREATE_NONE,
			NULL,
			NULL
			);
	}

	guint32 *c0 = global_nes_palette_get_memory_index (0);
	guint32 *c1 = global_nes_palette_get_memory_index (1);
	guint32 *c2 = global_nes_palette_get_memory_index (2);
	guint32 *c3 = global_nes_palette_get_memory_index (3);

	int n = 0;
	gchar *data = g_malloc0 (16384);
	snprintf (data,
			4096,
			"; save to 0 palette.\n"
			"\tLDX #$3f\n"
			"\tSTX $2006\n"
			"\tLDX #$00\n"
			"\tSTX $2006\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"; save to 1 palette.\n"
			"\tLDX #$3f\n"
			"\tSTX $2006\n"
			"\tLDX #$05\n"
			"\tSTX $2006\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"; save to 2 palette.\n"
			"\tLDX #$3f\n"
			"\tSTX $2006\n"
			"\tLDX #$09\n"
			"\tSTX $2006\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"; save to 3 palette.\n"
			"\tLDX #$3f\n"
			"\tSTX $2006\n"
			"\tLDX #$0d\n"
			"\tSTX $2006\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\tLDX #$%x\n"
			"\tSTX $2007\n"
			"\t\n"
			"\t\n"
			"\t\n"
			"%n"
			,
		c0[0], c0[1], c0[2], c0[3],
		c1[1], c1[2], c1[3],
		c2[1], c2[2], c2[3],
		c3[1], c3[2], c3[3],
		&n);

		gchar *s = data;
		s += n;

		TileRef *tile_ref = retro_canvas_screen_nes_get_tile_ref ();
		int screen_size = 32 * 28;

		guint16 addr = 0x2000;
		for (int i = 0; i < screen_size; i++) {
			if (tile_ref[i].tilex >= 0 && tile_ref[i].tiley >= 0) {
				int tile = tile_ref[i].tiley * 32 + tile_ref[i].tilex;
				guint8 a0 = (addr & 0xff00) >> 8;
				guint8 a1 = (addr & 0x00ff) >> 0;
				snprintf (s, 256, 
						"\tLDA #$%x\n"
						"\tSTA $2006\n"
						"\tLDA #$%x\n"
						"\tSTA $2006\n"
						"\tLDX #$%x\n"
						"\tSTX $2007\n"
						"%n",
						a0, a1,
						tile,
						&n);
				s += n;
			}
			addr++;
		}

		snprintf (s, 255, "\n\n%n", &n);
		s += n;

		/*
		 * save palettes megatiles
		 */

		guint8 *tile = retro_canvas_screen_nes_get_megatile ();
		int megatile_count = 8 * 7;
		int indx = 0;
		for (int i = 0; i < 14; i++) {
			snprintf (s, 255, ".byte $%x, $%x, $%x, $%x\n%n",
					tile[indx + 0],
					tile[indx + 1],
					tile[indx + 2],
					tile[indx + 3],
					&n
					);
			indx += 4;
			s += n;
		}

		guint32 len_data = strlen (data);

	g_output_stream_write (G_OUTPUT_STREAM (out), data, len_data, NULL, NULL);
	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);
}

void
project_nes_export_screen ()
{
	export_to_ca65 ();
}
