/* retro-app.c
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

#include "retro-app.h"
#include "main-window.h"

struct _RetroApp
{
	AdwApplication parent_instance;
};

G_DEFINE_TYPE (RetroApp, retro_app, ADW_TYPE_APPLICATION)

RetroApp *
retro_app_new (const char *application_id, GApplicationFlags  flags)
{
	g_return_val_if_fail (application_id != NULL, NULL);

	return g_object_new (RETRO_TYPE_APP,
	                     "application-id", application_id,
	                     "flags", flags,
	                     NULL);
}

static void
retro_app_activate (GApplication *app)
{
	GtkWindow *window;

	g_assert (RETRO_IS_APP (app));

	window = gtk_application_get_active_window (GTK_APPLICATION (app));

	if (window == NULL)
		window = g_object_new (MAIN_TYPE_WINDOW,
		                       "application", app,
		                       NULL);

	gtk_window_present (window);
}

static void
retro_app_class_init (RetroAppClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

	app_class->activate = retro_app_activate;
}

static void
retro_app_about_action (GSimpleAction *action,
                                            GVariant      *parameter,
                                            gpointer       user_data)
{
	static const char *developers[] = {"xverizex <horisontdawn@yandex.ru>", NULL};
	RetroApp *self = user_data;
	GtkWindow *window = NULL;
	GtkWidget *about;

	window = gtk_application_get_active_window (GTK_APPLICATION (self));

	g_assert (RETRO_IS_APP (self));

	about = adw_about_window_new ();
	gtk_window_set_transient_for (GTK_WINDOW (about), window);

	adw_about_window_set_license_type (ADW_ABOUT_WINDOW(about), GTK_LICENSE_GPL_3_0);
	adw_about_window_set_application_name (ADW_ABOUT_WINDOW(about), APPLICATION_NAME);
	adw_about_window_set_application_icon (ADW_ABOUT_WINDOW(about), "io.github.xverizex.RetroSpriteEditor");
	adw_about_window_set_version (ADW_ABOUT_WINDOW(about), PACKAGE_VERSION);
	adw_about_window_set_developers (ADW_ABOUT_WINDOW(about), developers);
	adw_about_window_set_developer_name (ADW_ABOUT_WINDOW(about), DEVELOPER_NAME);
	adw_about_window_set_copyright (ADW_ABOUT_WINDOW(about), "© 2023 xverizex");
	adw_about_window_set_comments (ADW_ABOUT_WINDOW(about), "Pixel Editor for Retro Consoles");
	adw_about_window_set_issue_url (ADW_ABOUT_WINDOW(about), "https://github.com/xverizex/RetroSpriteEditor/issues/new/choose");
	adw_about_window_add_link (ADW_ABOUT_WINDOW (about),
                                    "_Donate",
                              "https://www.donationalerts.com/r/xverizex");
	adw_about_window_add_link (ADW_ABOUT_WINDOW (about),
                                    "_Source Code",
                              "https://github.com/xverizex/RetroSpriteEditor");

	gtk_window_present (GTK_WINDOW (about));

        // Left this here in case.. uhh... uhhhhm....
	//adw_show_about_window (window,
	//                       "application-name", APPLICATION_NAME,
	//                       "application-icon", "io.github.xverizex.RetroSpriteEditor",
	//                       "developer-name", "xverizex",
	//                       "version", PACKAGE_VERSION,
	//                       "developers", developers,
        //                       "comments", "Pixel Editor for Retro Consoles",
	//                       "copyright", "© 2023 xverizex",
	//                       NULL);
}

static void
retro_app_quit_action (GSimpleAction *action,
                                           GVariant      *parameter,
                                           gpointer       user_data)
{
	RetroApp *self = user_data;

	g_assert (RETRO_IS_APP (self));

	g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
	{ "quit", retro_app_quit_action },
	{ "about", retro_app_about_action },
};

static void
retro_app_init (RetroApp *self)
{
	g_action_map_add_action_entries (G_ACTION_MAP (self),
	                                 app_actions,
	                                 G_N_ELEMENTS (app_actions),
	                                 self);
	gtk_application_set_accels_for_action (GTK_APPLICATION (self),
	                                       "app.quit",
	                                       (const char *[]) { "<primary>q", NULL });

	gtk_application_set_accels_for_action (GTK_APPLICATION (self),
	                                       "app.about",
	                                       (const char *[]) { "F1", NULL });
}
