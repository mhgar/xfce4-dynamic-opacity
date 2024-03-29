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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <xfconf/xfconf.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE // K
#include <libwnck/libwnck.h>

#include <common/panel-private.h>
#include <common/panel-xfconf.h>
#include <common/panel-utils.h>


#include <limits.h>

#include "opacity.h"
#include "opacity-dialogs.h"

#define UPDATE_INTERVAL 20



// Use WNCK stuff globally. If we have a list of everything we can filter out what we need per plugin instance.
static void opacity_wnck_init_once(void);

static void opacity_plugin_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void opacity_plugin_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

static void opacity_plugin_construct(XfcePanelPlugin *panel_plugin);
static void opacity_plugin_free_data(XfcePanelPlugin *panel_plugin);

static gboolean update(gpointer data);

void opacity_plugin_transition_to_alpha(const guint interval_ms, OpacityPlugin *plugin);

static void gdk_window_get_geometry_quick(GdkWindow *window, gint *x, gint *y, gint *width, gint *height);
static GdkWindow *get_panel_window(XfcePanelPlugin *panel_plugin);
static gint get_panel_id(XfcePanelPlugin *panel_plugin);

static float clamped_lerp(float a, float b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    return (a * (1.0f - t)) + (b * t);
}

//#define ABS(x) ((x < 0) ? (-x) : (x))
//#define CLAMP(x, min, max) ((x < min) ? (min) : ((x > max) ? (max) : (x)))
#define CLAMP01(x) ((x < 0) ? (0) : ((x > 1) ? (1) :(x)))

//static int min(int a, int b) { return a < b ? a : b; }
//static int man(int a, int b) { return a > b ? a : b; }


/*
    I need confirmation that this is how it works, because I can't find the correct documentation,
    but it seems to be rgb values. If you are like me and don't know how to express rgb values
    from 0.0 to 1.0, select your desired color on your panel, and then go to your xfconf settings 
    file (mine is located at $HOME/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml) and
    search for <property name="background-rgba" type="array"> and there you will have all the
    values for your color.
    
*/
static gdouble g_red = 0.0;     // Value goes from 0.0 to 1.0
static gdouble g_green = 0.0;   // Value goes from 0.0 to 1.0
static gdouble g_blue = 0.0;    // Value goes from 0.0 to 1.0


typedef struct {
    gboolean has_init;
    WnckScreen *screen;
} WnckGlobals;

enum {
    PROP_0,
    PROP_OVERRIDE_STYLE,
    PROP_X_PROXIMITY,
    PROP_Y_PROXIMITY,
    PROP_NEAR_ALPHA,
    PROP_FAR_ALPHA,
    PROP_TRANSITION_TIME,
    PROP_CURRENT_ALPHA,
    PROP_IS_NEAR,
    PROP_BG_COLOR,
    PROP_TIMEOUT_TAG
};

struct _OpacityPluginClass {
    XfcePanelPluginClass __parent__;
};

struct _OpacityPlugin {
    XfcePanelPlugin __parent__;

    // Configuration
    gboolean    override_style;
    guint       x_proximity;
    guint       y_proximity;
    guint       near_alpha;
    guint       far_alpha;
    guint       transition_time;
    
    // Live data
    guint     current_alpha; // Overrides color's alpha
    gboolean  is_near;
    gint timeout_tag;
};

XFCE_PANEL_DEFINE_PLUGIN(OpacityPlugin, opacity_plugin)

WnckGlobals wnck_globals;

static void gdk_window_get_geometry_quick(GdkWindow *window, gint *x, gint *y, gint *width, gint *height) {
    gdk_window_get_position(window, x, y);
    *width = gdk_window_get_width(window);
    *height = gdk_window_get_height(window);
}

static GdkWindow *get_panel_window(XfcePanelPlugin *panel_plugin) {
    return gtk_widget_get_window(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(panel_plugin))));
}

static gint get_panel_id(XfcePanelPlugin *panel_plugin) {
    GtkWidget *panel = gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(panel_plugin)));
    gint value;
    g_object_get(G_OBJECT(panel), "id", &value, NULL);

    return value;
}

static gboolean update(gpointer data) {
    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(data);
    gtk_widget_hide(GTK_WIDGET(plugin)); // Hack to prevent the plugin from wanting to take up space.

    GdkWindow *panel_window = get_panel_window(XFCE_PANEL_PLUGIN(plugin));
    guint last_alpha = plugin->current_alpha;
    plugin->is_near = FALSE;
    
    GdkRectangle panel_geom;
    gdk_window_get_geometry_quick(panel_window, &(panel_geom.x), &(panel_geom.y), &(panel_geom.width), &(panel_geom.height));
    
    //DBG("%d %d %d %d", panel_geom.x, panel_geom.y, panel_geom.width, panel_geom.height);

    // Extend bounds.          
    panel_geom.x -= plugin->x_proximity;
    panel_geom.width += plugin->x_proximity * 2;
    panel_geom.y -= plugin->y_proximity;
    panel_geom.height += plugin->y_proximity * 2;    
    
    for (GList *windows = wnck_screen_get_windows(wnck_globals.screen); windows != NULL; windows = windows->next) {        
        WnckWindow *window = WNCK_WINDOW(windows->data);
        
        if (wnck_window_get_state(window) & WNCK_WINDOW_STATE_SKIP_PAGER) continue;
        if (wnck_window_is_minimized(window)) continue;
        if (!wnck_window_is_on_workspace(window, wnck_screen_get_active_workspace(wnck_globals.screen))) continue;       

        GdkRectangle window_geom;
        wnck_window_get_geometry(window, &(window_geom.x), &(window_geom.y), &(window_geom.width), &(window_geom.height));
        
        if (!gdk_rectangle_intersect(&panel_geom, &window_geom, NULL)) continue;
        
        plugin->is_near = TRUE;
    }
    
    opacity_plugin_transition_to_alpha(UPDATE_INTERVAL, plugin);
    
    if (last_alpha != plugin->current_alpha) {       
        XfconfChannel *channel = xfconf_channel_get("xfce4-panel");
        gdouble alpha = ((gdouble) plugin->current_alpha) / 255.0;

        gint panel_id = get_panel_id(XFCE_PANEL_PLUGIN(plugin));

        if (panel_id != -1) {
            gchar* property = g_strdup_printf("/panels/panel-%d/background-rgba", panel_id);

            xfconf_channel_set_array(channel, property, G_TYPE_DOUBLE, &g_red, G_TYPE_DOUBLE, &g_green, G_TYPE_DOUBLE, &g_blue, G_TYPE_DOUBLE, &alpha, G_TYPE_INVALID);
            gdk_window_invalidate_rect(panel_window, NULL, FALSE);
            
            g_free(property);
        } else {
            DBG("Could not find the panel!");
        }
    }
    
    return TRUE;
}

static void opacity_wnck_init_once(void) {
    if (wnck_globals.has_init) return;
    
    wnck_globals.has_init = TRUE;
    wnck_globals.screen = wnck_screen_get_default();
    wnck_screen_force_update(wnck_globals.screen);
}


static void opacity_plugin_class_init(OpacityPluginClass *class) {
    XfcePanelPluginClass *plugin_class = XFCE_PANEL_PLUGIN_CLASS(class);
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    
    opacity_wnck_init_once();
    
    gobject_class->get_property = opacity_plugin_get_property;
    gobject_class->set_property = opacity_plugin_set_property;    
    
    plugin_class->construct = opacity_plugin_construct;
    plugin_class->free_data = opacity_plugin_free_data;
    plugin_class->configure_plugin = opacity_plugin_configure;
    plugin_class->about = opacity_plugin_about;
    
    // Configuration
    g_object_class_install_property(
        gobject_class, 
        PROP_OVERRIDE_STYLE,
        g_param_spec_boolean(
            "override-style",
            NULL, NULL,
            DEFAULT_OVERRIDE_STYLE,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_X_PROXIMITY,
        g_param_spec_uint(
            "x-proximity",
            NULL, NULL,
            PROXIMITY_MIN,
            PROXIMITY_MAX,
            DEFAULT_X_PROXIMITY,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_Y_PROXIMITY,
        g_param_spec_uint(
            "y-proximity",
            NULL, NULL,
            PROXIMITY_MIN,
            PROXIMITY_MAX,
            DEFAULT_Y_PROXIMITY,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_NEAR_ALPHA,
        g_param_spec_uint(
            "near-alpha",
            NULL, NULL,
            ALPHA_MIN,
            ALPHA_MAX,
            DEFAULT_NEAR_ALPHA,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_FAR_ALPHA,
        g_param_spec_uint(
            "far-alpha",
            NULL, NULL,
            ALPHA_MIN,
            ALPHA_MAX,
            DEFAULT_FAR_ALPHA,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_TRANSITION_TIME,
        g_param_spec_uint(
            "transition-time",
            NULL, NULL,
            TRANSITION_TIME_MIN,
            TRANSITION_TIME_MAX,
            DEFAULT_TRANSITION_TIME,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    // Hidden data
    g_object_class_install_property(
        gobject_class, 
        PROP_CURRENT_ALPHA,
        g_param_spec_uint(
            "current-alpha",
            NULL, NULL,
            ALPHA_MIN,
            ALPHA_MAX,
            DEFAULT_CURRENT_ALPHA,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
    
    g_object_class_install_property(
        gobject_class, 
        PROP_IS_NEAR,
        g_param_spec_boolean(
            "is-near",
            NULL, NULL,
            DEFAULT_IS_NEAR,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );

    g_object_class_install_property(
        gobject_class, 
        PROP_TIMEOUT_TAG,
        g_param_spec_int(
            "timeout-tag",
            NULL, NULL,
            INT_MIN, INT_MAX, -1,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
        )
    );
}

static void opacity_plugin_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(object);
    
    // It says set here but it's setting the input value, not something on the object.
    switch (prop_id) {
    case PROP_OVERRIDE_STYLE:
        g_value_set_boolean(value, plugin->override_style);
        break;
    case PROP_X_PROXIMITY:
        g_value_set_uint(value, plugin->x_proximity);
        break;
    case PROP_Y_PROXIMITY:
        g_value_set_uint(value, plugin->y_proximity);
        break;
    case PROP_NEAR_ALPHA:
        g_value_set_uint(value, plugin->near_alpha);
        break;
    case PROP_FAR_ALPHA:
        g_value_set_uint(value, plugin->far_alpha);
        break;
    case PROP_TRANSITION_TIME:
        g_value_set_uint(value, plugin->transition_time);
        break;
    case PROP_CURRENT_ALPHA:
        g_value_set_uint(value, plugin->current_alpha);
        break;
    case PROP_IS_NEAR:
        g_value_set_boolean(value, plugin->is_near);
        break;
    case PROP_TIMEOUT_TAG:
        g_value_set_int(value, plugin->timeout_tag);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void opacity_plugin_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(object);
    
    // Again, it says get sometimes but we're actually setting stuff on the object.
    switch (prop_id) {
    case PROP_OVERRIDE_STYLE:
        // May want to add diff setting.
        plugin->override_style = g_value_get_boolean(value);
        if (plugin->override_style) {            
            gint panel_id = get_panel_id(XFCE_PANEL_PLUGIN(plugin));

            if (panel_id != -1) {
                DBG("Found panel %d", panel_id);

                XfconfChannel *channel = xfconf_channel_get("xfce4-panel");
                gchar* property = g_strdup_printf("/panels/panel-%d/background-style", panel_id);
                xfconf_channel_set_uint(channel, property, 1);
                
                GdkWindow *panel_window = get_panel_window(XFCE_PANEL_PLUGIN(plugin));
                gdk_window_invalidate_rect(panel_window, NULL, FALSE);

                g_free(property);
            } else {
                DBG("Could not find the panel!");
            }
        }
        break;
    case PROP_X_PROXIMITY:
        plugin->x_proximity = g_value_get_uint(value);
        break;
    case PROP_Y_PROXIMITY:
        plugin->y_proximity = g_value_get_uint(value);
        break;
    case PROP_NEAR_ALPHA:
        plugin->near_alpha = g_value_get_uint(value);
        break;
    case PROP_FAR_ALPHA:
        plugin->far_alpha = g_value_get_uint(value);
        break;
    case PROP_TRANSITION_TIME:
        plugin->transition_time = g_value_get_uint(value);
        break;
    case PROP_CURRENT_ALPHA:
        plugin->current_alpha = g_value_get_uint(value);
        break;
    case PROP_IS_NEAR:
        // We might want to prep transitioning here.
        plugin->is_near = g_value_get_boolean(value);
        break;
    case PROP_TIMEOUT_TAG:
        plugin->timeout_tag = g_value_get_int(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void opacity_plugin_init(OpacityPlugin *plugin) {
    DBG("INIT");
    plugin->override_style = DEFAULT_OVERRIDE_STYLE;
    plugin->x_proximity = DEFAULT_X_PROXIMITY;
    plugin->y_proximity = DEFAULT_Y_PROXIMITY;
    plugin->near_alpha = DEFAULT_NEAR_ALPHA;
    plugin->far_alpha = DEFAULT_FAR_ALPHA;
    plugin->transition_time = DEFAULT_TRANSITION_TIME;
    
    // Start opaque, it'll transition to light if it needs to.
    plugin->current_alpha = DEFAULT_CURRENT_ALPHA;
    plugin->is_near = DEFAULT_IS_NEAR;
    plugin->timeout_tag = -1;
}

void opacity_plugin_transition_to_alpha(const guint interval_ms, OpacityPlugin *plugin) {
    gfloat dt = (gfloat) interval_ms / 1000.0f;
    dt *= ((gfloat) plugin->transition_time / 1000.0f);

    plugin->current_alpha = plugin->is_near ?
        clamped_lerp(plugin->current_alpha, plugin->near_alpha, dt) :
        clamped_lerp(plugin->current_alpha, plugin->far_alpha, dt);

    /*
    gfloat alpha_range, alpha_per_ms, alpha_delta, alpha_percentage_step, alpha_current_percentage;

    if (plugin->current_alpha == (plugin->is_near ? plugin->near_alpha : plugin->far_alpha)) return; // Already done.    

    // Use float to increase precision with low update intervals
    alpha_range = ABS((gfloat) plugin->near_alpha - (gfloat) plugin->far_alpha);    
    alpha_per_ms = alpha_range / (plugin->transition_time != 0 ? (gfloat) plugin->transition_time : 1.0f);
    alpha_delta = alpha_per_ms * (gfloat) interval_ms;
    alpha_percentage_step = CLAMP01(alpha_delta / alpha_range);
    alpha_current_percentage = CLAMP01((plugin->current_alpha - MIN(plugin->near_alpha, plugin->far_alpha) / alpha_range));
    
    // Can be made simpler, but keep in mind that we can't assume near_alpha is always bigger than far_alpha!!
    plugin->current_alpha = (guint) plugin->is_near ? 
        clamped_lerp(plugin->far_alpha, plugin->near_alpha, alpha_current_percentage + alpha_percentage_step) :
        clamped_lerp(plugin->near_alpha, plugin->far_alpha, alpha_current_percentage + alpha_percentage_step);
    */
}

static void opacity_plugin_construct(XfcePanelPlugin *panel_plugin) {
    GError* error;
    xfconf_init(&error);
    
    const PanelProperty properties[] = {
        { "override-style", G_TYPE_BOOLEAN },
        { "x-proximity", G_TYPE_UINT },
        { "y-proximity", G_TYPE_UINT },
        { "near-alpha", G_TYPE_UINT },
        { "far-alpha", G_TYPE_UINT },
        { "transition-time", G_TYPE_UINT},
        
        // Unsure about exposing these, but might make referencing easier.
        { "current-alpha", G_TYPE_UINT },
        { "is-near", G_TYPE_BOOLEAN },
        { "timeout-tag", G_TYPE_INT },
        NULL,
    };  
    
    xfce_panel_plugin_menu_show_configure(panel_plugin);
    xfce_panel_plugin_menu_show_about(panel_plugin);
    
    panel_properties_bind(
        NULL, 
        G_OBJECT(panel_plugin), 
        xfce_panel_plugin_get_property_base(panel_plugin), 
        properties, 
        TRUE // Could be FALSE
    );

    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(panel_plugin);

    gint timeout_tag = g_timeout_add(UPDATE_INTERVAL, update, plugin); // I previously wrote UNSAFE!!! here and have no idea why.
    plugin->timeout_tag = timeout_tag;

    gtk_widget_hide(GTK_WIDGET(panel_plugin)); // Not working sometimes? Investigate.

    gint panel_id = get_panel_id(XFCE_PANEL_PLUGIN(plugin));
    DBG("Constructed on %d", panel_id);
}

static void opacity_plugin_free_data(XfcePanelPlugin *panel_plugin) {
    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(panel_plugin);
    g_source_remove(plugin->timeout_tag);

    panel_properties_unbind(G_OBJECT(panel_plugin));
    xfconf_shutdown();
}
