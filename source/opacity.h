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

#ifndef __OPACITY_H__
#define __OPACITY_H__

G_BEGIN_DECLS

#define ALPHA_MIN 0u
#define ALPHA_MAX 255u

#define DEFAULT_OVERRIDE_STYLE FALSE
#define DEFAULT_X_PROXIMITY 0u
#define DEFAULT_Y_PROXIMITY 32u
#define DEFAULT_NEAR_ALPHA 255u
#define DEFAULT_FAR_ALPHA 0u
#define DEFAULT_TRANSITION_TIME 5000u
#define DEFAULT_UPDATE_FREQUENCY 0.1

#define DEFAULT_CURRENT_ALPHA DEFAULT_NEAR_ALPHA
#define DEFAULT_IS_NEAR TRUE

#define TRANSITION_TIME_MIN 0u
#define TRANSITION_TIME_MAX 10000u

#define PROXIMITY_MIN 0u
#define PROXIMITY_MAX UINT_MAX

typedef struct _OpacityPlugin OpacityPlugin;
typedef struct _OpacityPluginClass OpacityPluginClass;

#define XFCE_TYPE_OPACITY_PLUGIN            (opacity_plugin_get_type())
#define XFCE_OPACITY_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_TYPE_OPACITY_PLUGIN, OpacityPlugin))
#define XFCE_OPACITY_PLUGIN_CLASS(class)    (G_TYPE_CHECK_CLASS_CAST((class), XFCE_TYPE_OPACITY_PLUGIN, OpacityPluginClass))
#define IS_XFCE_OPACITY_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_TYPE_OPACITY_PLUGIN))
#define IS_XFCE_OPACITY_PLUGIN_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE((class), XFCE_TYPE_OPACITY_PLUGIN))
#define XFCE_OPACITY_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_TYPE_OPACITY_PLUGIN), OpacityPluginClass)

GType opacity_plugin_get_type (void) G_GNUC_CONST;
void opacity_plugin_register_type(XfcePanelTypeModule *typeModule);

G_END_DECLS

#endif // __OPACITY_H__
