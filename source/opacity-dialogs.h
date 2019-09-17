/*
 * Copyright (C) 2019 Maxwell Hunter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __DYNAMIC_OPACITY_DIALOGS_H__
#define __DYNAMIC_OPACITY_DIALOGS_H__

G_BEGIN_DECLS

typedef struct _OpacityPluginDialog OpacityPluginDialog;

void opacity_plugin_configure(XfcePanelPlugin *panel_plugin);
void opacity_plugin_about(XfcePanelPlugin *panel_plugin);

G_END_DECLS

#endif
