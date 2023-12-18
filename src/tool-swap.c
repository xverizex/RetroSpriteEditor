/* tool-swap.c
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

#include "tool-swap.h"

struct _ToolSwap
{
  ToolButton  parent_instance;
};

G_DEFINE_FINAL_TYPE (ToolSwap, tool_swap, TOOL_TYPE_BUTTON)

static void
tool_swap_class_init (ToolSwapClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
tool_swap_init (ToolSwap *self)
{
  ToolButton *parent = TOOL_BUTTON (self);

  tool_button_set_index (parent, INDX_TOOL_SWAP);
}
