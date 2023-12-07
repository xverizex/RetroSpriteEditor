/* tool-button.h
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

#pragma once

#include <gtk/gtk.h>
#include "general-tools.h"

G_BEGIN_DECLS

#define TOOL_TYPE_BUTTON (tool_button_get_type())

G_DECLARE_DERIVABLE_TYPE (ToolButton, tool_button, TOOL, BUTTON, GtkToggleButton)

struct _ToolButtonClass {
  GtkToggleButtonClass parent_instance;

  gpointer padding[12];
};
G_END_DECLS

void 
tool_button_set_index (ToolButton *self, guint32 index);

guint32 
tool_button_get_type_index (ToolButton *self);
