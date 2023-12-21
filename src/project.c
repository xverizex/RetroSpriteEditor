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

void 
project_free (void)
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

	char str_count[33];
	int count_screen = global_get_screen_count ();
	snprintf (str_count ,32,"%d", count_screen);

	writer = xmlNewTextWriterFilename (prj->fullpath_to_project, 0);
	xmlTextWriterStartDocument (writer, NULL, "UTF-8", NULL);
	xmlTextWriterStartElement (writer, "RetroSpriteEditor");
	xmlTextWriterStartElement (writer, "ProjectSettings");
			xmlTextWriterWriteElement (writer, "platform_id", "0");
			xmlTextWriterWriteElement (writer, "project_name", prj->name);
			xmlTextWriterWriteElement (writer, "project_folder", prj->folder_path);
			xmlTextWriterWriteElement (writer, "screen", prj->name_screen);
			xmlTextWriterWriteElement (writer, "fullpath", prj->fullpath_to_project);
			xmlTextWriterWriteElement (writer, "count_screen", str_count);
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
	int  *v1 = 0;
	if (strncmp (name, sname, strlen (sname) + 1))
		return;

	switch (type) {
		case TYPE_STRING:
			v0 = g_strdup (value);
			*v = v0;
			break;
		case TYPE_INT:
			v1 = g_malloc0 (sizeof (gint32));
			*v1 = atoi (value);
			*v = v1;
			break;
	}
}

static void
handle_nes_name_value (const xmlChar *name, const xmlChar *value)
{
	int *count_screen = NULL;

	check_and_write (name, "project_name", value, (void **) &prj->name, TYPE_STRING);
	check_and_write (name, "project_folder", value, (void **) &prj->folder_path, TYPE_STRING);
	check_and_write (name, "fullpath", value, (void **) &prj->fullpath_to_project, TYPE_STRING);
	check_and_write (name, "outfile", value, (void **) &prj->file_to_export, TYPE_STRING);
	check_and_write (name, "screen", value, (void **) &prj->name_screen, TYPE_STRING);
	check_and_write (name, "count_screen", value, (void **) &count_screen, TYPE_INT);

	if (count_screen) {
		global_set_screen_count (*count_screen);
		g_free (count_screen);
	}

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
	for (int cur_bank = 0; cur_bank < 2; cur_bank++) {
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
    				nes_palette_set_color_with_map (nes, &n, index + 1, cur_bank);
						nes_palette_set_draw_tile (nes, cur_bank, blkx, blky, 1);
					} else {
    				nes_palette_set_color_with_map (nes, &n, 0, cur_bank);
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
	global_nes_create_screens ();
	open_nes_ref ();
	open_nes_screen_palettes ();
}

void
project_import_nes_chr (char *chr)
{
	if (prj->file_to_export) {
		g_free (prj->file_to_export);
	}
	prj->file_to_export = g_strdup_printf ("%s", chr);
	read_tilemap_and_set_nes ();
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
	int max_refs = global_get_screen_count ();

	for (int indx = 0; indx < max_refs; indx++) {
		gchar screen_ref[512];
		snprintf (screen_ref, 512, "%s/%s_%d.ref", prj->folder_path, prj->name_screen, indx);
		GFile *file_ref = g_file_new_for_path (screen_ref);

		if (!g_file_query_exists (file_ref, NULL)) {
			return;
		}
		GFileInputStream *in = NULL;
		in = g_file_read (file_ref,
			NULL,
			NULL
			);

		GtkWidget *screen = global_get_screen (indx);
		TileRef *tile = retro_canvas_nes_get_tile_ref (RETRO_CANVAS (screen));
		int screen_size = 32 * 30;

		for (int i = 0; i < screen_size; i++) {
			g_input_stream_read (G_INPUT_STREAM (in), &tile[i], sizeof (TileRef), NULL, NULL);
		}

		g_input_stream_close (G_INPUT_STREAM (in), NULL, NULL);
	}
}

static void
open_nes_screen_palettes ()
{
	int max_palettes = global_get_screen_count ();

	for (int indx = 0; indx < max_palettes; indx++) {
		gchar screen_palettes[512];
		snprintf (screen_palettes, 512, "%s/%s_%d.palettes", prj->folder_path, prj->name_screen, indx);

		GFile *file_ref = g_file_new_for_path (screen_palettes);

		if (!g_file_query_exists (file_ref, NULL)) {
			return;
		}
		GFileInputStream *in = NULL;
		in = g_file_read (file_ref,
			NULL,
			NULL
			);

		GtkWidget *screen = global_get_screen (indx);
		guint8 *tile = retro_canvas_nes_get_megatile (RETRO_CANVAS (screen));
		int screen_size = 8 * 8;
		for (int i = 0; i < screen_size; i++) {
			g_input_stream_read (G_INPUT_STREAM (in), &tile[i], sizeof (guint8), NULL, NULL);
		}

		g_input_stream_close (G_INPUT_STREAM (in), NULL, NULL);
	}
}
static void
save_nes_ref (guint32 indx, gchar *screen_ref)
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

	GtkWidget *screen = global_get_screen (indx);

	TileRef *tile = retro_canvas_nes_get_tile_ref (RETRO_CANVAS (screen));
	int screen_size = 32 * 30;

	for (int i = 0; i < screen_size; i++) {
		g_output_stream_write (G_OUTPUT_STREAM (out), &tile[i], sizeof (TileRef), NULL, NULL);
	}

	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);
}

static void
save_nes_screen_palettes (guint32 indx, gchar *screen_palette)
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

	GtkWidget *screen = global_get_screen (indx);
	guint8 *tile = retro_canvas_nes_get_megatile (RETRO_CANVAS (screen));
	int screen_size = 8 * 8;
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

	int count_screen = global_get_screen_count ();
	for (int i = 0; i < count_screen; i++) {
		snprintf (screen_palettes, 512, "%s/%s_%d.palettes", prj->folder_path, prj->name_screen, i);
		snprintf (screen_ref, 512, "%s/%s_%d.ref", prj->folder_path, prj->name_screen, i);
	
		save_nes_ref (i, screen_ref);
		save_nes_screen_palettes (i, screen_palettes);
	}
	

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
export_little_code_to_screen_ca65 (guint32 screen_num)
{
	gchar path[512];
	snprintf (path, 512, "%s/screen_%d_ca65.s", prj->folder_path, screen_num);

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
	gchar *s = data;

	snprintf (s,
			1024,
			"PPUCTRL      = $2000\n"
			"PPUMASK      = $2001\n"
			"PPUSTATUS    = $2002\n"
			"OAMADDR      = $2003\n"
			"PPUSCROLL    = $2005\n"
			"PPUADDR      = $2006\n"
			"PPUDATA      = $2007\n"
			"OAMDMA       = $4014\n"
			"\n\n"
			".export load_screen_%d\n"
			"\n\n"
			".segment \"CODE\"\n"
			".proc load_screen_%d\n"
			"\n"
			"%n",
			screen_num,
			screen_num,
			&n);

	s += n;

	snprintf (s,
			4096,
			"; save to 0 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$00\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\n"
			"; save to 1 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$05\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"; save to 2 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$09\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"; save to 3 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$0d\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
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

		s += n;

		snprintf (s, 1024,
				  "\tLDA #$1f\n"
					"\tPHA\n"
					"load_screen_main:\n"
					"\tPLA\n"
					"\tTAX\n"
					"\tINX\n"
					"\tTXA\n"
					"\tPHA\n"
					"\tCPX #$20\n"
					"\tBEQ pre_pre_load_0\n"
					"\tCPX #$21\n"
					"\tBEQ pre_pre_load_1\n"
					"\tCPX #$22\n"
					"\tBEQ pre_pre_load_2\n"
					"\tCPX #$23\n"
					"\tBEQ pre_pre_load_3\n"
					"\tJMP end\n"
					"%n",
					&n);

		s += n;

		for (int i = 0; i < 4; i++) {
			snprintf (s, 1024,
					"pre_pre_load_%d:\n"
					"\tLDX #$00\n"
					"\tLDY #$00\n"
					"pre_load_%d:\n"
					"\tLDA PPUSTATUS\n"
					"\tLDA #$2%d\n"
					"\tSTA PPUADDR\n"
					"load_%d:\n"
					"\tTXA\n"
					"\tCMP size_screen_%d\n"
					"\tBEQ jmp_load_screen_main_%d\n"
					"\tLDA screen_%d_indx, Y\n"
					"\tSTA PPUADDR\n"
					"\tINY\n"
					"\tLDA screen_%d, X\n"
					"\tSTA PPUDATA\n"
					"\tINX\n"
					"\tBNE pre_load_%d\n"
					"\tJMP load_screen_main\n"
					"jmp_load_screen_main_%d:\n"
					"\tJMP load_screen_main\n"
					"%n",
				i, i, i, i, i, i, i, i, i, i, &n);
			s += n;
		}

		snprintf (s, 512,
				"end:\n"
				"\tPLA\n"
				"\tLDY #$00\n"
				"palette_cycle:\n"
				"\tTYA\n"
				"\tCMP palette_size\n"
				"\tBEQ end_palette\n"
				"\tLDA PPUSTATUS\n"
				"\tLDA #$23\n"
				"\tSTA PPUADDR\n"
				"\tLDA palette_indx, Y\n"
				"\tCLC\n"
				"\tADC #$c0\n"
				"\tSTA PPUADDR\n"
				"\tLDA palette, Y\n"
				"\tSTA PPUDATA\n"
				"\tINY\n"
				"\tBNE palette_cycle\n"
				"end_palette:\n"
				"\tRTS\n"
				".endproc\n"
				"\n\n"
				"%n",
				&n);

		s += n;

		GtkWidget *screen = global_get_screen (screen_num);

		TileRef *tile_ref = retro_canvas_nes_get_tile_ref (RETRO_CANVAS (screen));
		int screen_size = 32 * 30;


		/*
		 * save tiles
		 */
		snprintf (s, 255,
				".segment \"RODATA\"\n"
				"%n", &n);

		s += n;

		int scr_num = 0;
		int index = 0;
		guint8 sizes[4] = {0, };
		for (int i = 0; i < screen_size; i += 4) {
			if (i % 256 == 0) {
				snprintf (s, 256,
						"screen_%d:\n"
						"%n"
						,
						scr_num++, &n);
				s += n;
				index = 0;
			}


			int m = 0;
			int found = 0;
			for (int in = 0; in < 4; in++) {
				int tile = tile_ref[i + in].tiley * 16 + tile_ref[i + in].tilex;

				if (tile >= 0) {
					if (found == 0) {
						found = 1;
						snprintf (s, 512,
							".byte %n", &n);
						s += n;
					}
					snprintf (s, 512, "$%02x%n,%n", tile, &m, &n);
					sizes[scr_num - 1]++;
				}
				index++;

				if (in < 3) {
					if (tile >= 0)
						s += n;
				} else {
					if (tile >= 0)
						s += m;
				}
			}

			if (*(s - 1) == ',') {
				s--;
				*s = 0;
			}

			if (found) {
				snprintf (s, 512, "\n%n", &n);
				s += n;
			}
		}

		index = 0;
		scr_num = 0;
		for (int i = 0; i < screen_size; i += 4) {
			if (i % 256 == 0) {
				snprintf (s, 256,
						"screen_%d_indx:\n"
						"%n"
						,
						scr_num++, &n);
				s += n;
				index = 0;
			}


			int m = 0;
			int found = 0;
			for (int in = 0; in < 4; in++) {
				int tile = tile_ref[i + in].tiley * 16 + tile_ref[i + in].tilex;

				if (tile >= 0) {
					if (found == 0) {
						found = 1;
						snprintf (s, 512,
							".byte %n", &n);
						s += n;
					}
					snprintf (s, 512, "$%02x%n,%n", index, &m, &n);
				}
				index++;

				if (in < 3) {
					if (tile >= 0)
						s += n;
				} else {
					if (tile >= 0)
						s += m;
				}
			}

			if (*(s - 1) == ',') {
				s--;
				*s = 0;
			}

			if (found) {
				snprintf (s, 512, "\n%n", &n);
				s += n;
			}
		}
		/*
		 * save sizes
		 */
		for (int i = 0; i < 4; i++) {
			snprintf (s, 512,
					"size_screen_%d:\n"
					".byte $%02x\n"
					"%n",
					i,
					sizes[i], &n);

			s += n;
		}


		/*
		 * save palettes megatiles
		 */

		guint8 *tile = retro_canvas_nes_get_megatile (RETRO_CANVAS (screen));
		int megatile_count = 8 * 8;
		int indx = 0;

		int size_palette = 0;

		snprintf (s, 255,
				"palette:\n%n",
				&n);

		s += n;
		int found = 0;
		int m;
		guint8 palette_indx[64] = {0, };

		for (int i = 0; i < 64; i++) {
			guint8 tile0 = global_convert_setup_megatile_to_nes (tile[i]);
			if (tile0 > 0) {
				if (found == 0) {
					found = 1;
					snprintf (s, 255,
							".byte %n", &n);
					s += n;
				}
				snprintf (s, 255, "$%02x,%n", tile0, &n);
				size_palette++;
				palette_indx[i] = 1;
				s += n;
			}

			if (found && i > 0 && ((i % 4) == 0)) {
				found = 0;
				if (*(s - 1) == ',') {
					s--;
					*s = 0;
				}
				snprintf (s, 255,
						"\n%n",
						&n);

				s += n;
			}
		}

		snprintf (s, 255,
				"palette_indx:\n%n",
				&n);

		s += n;

		for (int i = 0; i < 64; i++) {
			if (palette_indx[i] == 1) {
				if (found == 0) {
					found = 1;
					snprintf (s, 255,
							".byte %n", &n);
					s += n;
				}
				snprintf (s, 255, "$%02x,%n", i, &n);
				s += n;
			}

			if (found && i > 0 && ((i % 4) == 0)) {
				found = 0;
				if (*(s - 1) == ',') {
					s--;
					*s = 0;
				}
				snprintf (s, 255,
						"\n%n",
						&n);

				s += n;
			}
		}

		snprintf (s, 512,
				"palette_size:\n"
				".byte $%02x\n"
				"%n",
				size_palette,
				&n);

		s += n;

	guint32 len_data = strlen (data);

	g_output_stream_write (G_OUTPUT_STREAM (out), data, len_data, NULL, NULL);
	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);

	g_free (data);
}

static void
export_full_dump_to_screen_ca65 (guint32 screen_num)
{
	gchar path[512];
	snprintf (path, 512, "%s/screen_%d_ca65.s", prj->folder_path, screen_num);

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
	gchar *s = data;

	snprintf (s,
			1024,
			"PPUCTRL      = $2000\n"
			"PPUMASK      = $2001\n"
			"PPUSTATUS    = $2002\n"
			"OAMADDR      = $2003\n"
			"PPUSCROLL    = $2005\n"
			"PPUADDR      = $2006\n"
			"PPUDATA      = $2007\n"
			"OAMDMA       = $4014\n"
			"\n\n"
			".export load_screen_%d\n"
			"\n\n"
			".segment \"CODE\"\n"
			".proc load_screen_%d\n"
			"\n"
			"%n",
			screen_num,
			screen_num,
			&n);

	s += n;

	snprintf (s,
			4096,
			"; save to 0 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$00\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\n"
			"; save to 1 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$05\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"; save to 2 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$09\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"; save to 3 palette.\n"
			"\tLDA PPUSTATUS\n"
			"\tLDX #$3f\n"
			"\tSTX PPUADDR\n"
			"\tLDX #$0d\n"
			"\tSTX PPUADDR\n"
			"\t\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
			"\tLDX #$%x\n"
			"\tSTX PPUDATA\n"
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

		s += n;

		GtkWidget *screen = global_get_screen (screen_num);

		TileRef *tile_ref = retro_canvas_nes_get_tile_ref (RETRO_CANVAS (screen));
		int screen_size = 32 * 30;

		snprintf (s, 4096, 
				"\tLDA PPUSTATUS\n"
				"\tLDA #$20\n"
				"\tPHA\n"
				"\tSTA PPUADDR\n"
				"\tLDA #$00\n"
				"\tSTA PPUADDR\n"
				"\tJMP load_screen0\n"
				"cycle_load_screen:\n"
				"\tLDY #$00\n"
				"\tPLA\n"
				"\tTAX\n"
				"\tINX\n"
				"\tTXA\n"
				"\tPHA\n"
				"\tCPX #$21\n"
				"\tBEQ load_1\n"
				"\tCPX #$22\n"
				"\tBEQ load_2\n"
				"\tCPX #$23\n"
				"\tBEQ load_3\n"
				"\tCPX #$24\n"
				"\tBEQ load_4 ; load_palettes\n"
				"\tJMP end_load_screen\n"
				"load_screen0:\n"
				"\tLDY #$00\n"
				"%n"
				,
			&n);
		s += n;
		for (int i = 0; i < 3; i++) {
			snprintf (s, 512,
				"load_%d:\n"
				"\tLDX screen_%d, Y\n"
				"\tSTX PPUDATA\n"
				"\tINY\n"
				"\tBNE load_%d\n"
				"\tJMP cycle_load_screen\n"
				"%n",
				i,
				i,
				i,
			&n);

			s += n;
		}

		snprintf (s, 512,
			"load_%d:\n"
			"\tLDX screen_%d, Y\n"
			"\tSTX PPUDATA\n"
			"\tINY\n"
			"\tCPY #$c0\n"
			"\tBNE load_%d\n"
			"\tJMP cycle_load_screen\n"
			"%n",
			3,
			3,
			3,
		&n);

		s += n;

		snprintf (s, 512,
			"load_%d:\n"
			"\tLDX screen_%d, Y\n"
			"\tSTX PPUDATA\n"
			"\tINY\n"
			"\tCPY #$40\n"
			"\tBNE load_%d\n"
			"\tJMP cycle_load_screen\n"
			"%n",
			4,
			4,
			4,
		&n);

		s += n;

		snprintf (s, 512,
				"end_load_screen:\n"
				"\tPLA\n"
				"\tRTS\n"
				".endproc\n"
				"\n"
				"\n"
				"%n",
				&n);

		s += n;

		/*
		 * save tiles
		 */
		snprintf (s, 255,
				".segment \"RODATA\"\n"
				"%n", &n);

		s += n;

		int scr_num = 0;
		for (int i = 0; i < screen_size; i += 4) {
			if (i % 256 == 0) {
				snprintf (s, 256,
						"screen_%d:\n"
						"%n"
						,
						scr_num++, &n);
				s += n;
			}
			int tile0 = tile_ref[i + 0].tiley * 16 + tile_ref[i + 0].tilex;
			int tile1 = tile_ref[i + 1].tiley * 16 + tile_ref[i + 1].tilex;
			int tile2 = tile_ref[i + 2].tiley * 16 + tile_ref[i + 2].tilex;
			int tile3 = tile_ref[i + 3].tiley * 16 + tile_ref[i + 3].tilex;
			snprintf (s, 512,
					".byte $%02x, $%02x, $%02x, $%02x ; (%03d - %03d) \n%n",
					tile0 < 0? 0: tile0,
					tile1 < 0? 0: tile1,
					tile2 < 0? 0: tile2,
					tile3 < 0? 0: tile3,
					i,
					i + 3,
					&n
					);
			s += n;
		}
		/*
		 * save palettes megatiles
		 */

		guint8 *tile = retro_canvas_nes_get_megatile (RETRO_CANVAS (screen));
		int megatile_count = 8 * 8;
		int indx = 0;
		snprintf (s, 255,
				"screen_4:\n%n",
				&n);

		s += n;
		for (int i = 0; i < 16; i++) {
			guint8 tile0 = global_convert_setup_megatile_to_nes (tile[indx + 0]);
			guint8 tile1 = global_convert_setup_megatile_to_nes (tile[indx + 1]);
			guint8 tile2 = global_convert_setup_megatile_to_nes (tile[indx + 2]);
			guint8 tile3 = global_convert_setup_megatile_to_nes (tile[indx + 3]);
			snprintf (s, 255, ".byte $%02x, $%02x, $%02x, $%02x; (%03d - %03d)\n%n",
					tile0,
					tile1,
					tile2,
					tile3,
					indx,
					indx + 3,
					&n
					);
			indx += 4;
			s += n;
		}

		guint32 len_data = strlen (data);

	g_output_stream_write (G_OUTPUT_STREAM (out), data, len_data, NULL, NULL);
	g_output_stream_close (G_OUTPUT_STREAM (out), NULL, NULL);

	g_free (data);
}

static void
export_to_ca65 (void)
{
	int count_screen = global_get_screen_count ();
	for (int i = 0; i < count_screen; i++) {
		//export_full_dump_to_screen_ca65 (i);
		export_little_code_to_screen_ca65 (i);
	}
}

void
project_nes_export_screen ()
{
	export_to_ca65 ();
}
