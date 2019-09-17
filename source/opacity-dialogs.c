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

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "opacity.h"
#include "opacity-dialogs.h"

#define BORDER 16
#define GRID_PADDING 8

GtkWidget* make_integer_slider(gint min, gint max);
void bind_slider_value_to(GtkScale *slider, GObject *object, const gchar *property);

GtkWidget* make_integer_spinner(gint min, gint max);
void bind_spinner_value_to(GtkSpinButton *slider, GObject *object, const gchar *property);
void bind_toggle_button_value_to(GtkToggleButton *toggle, GObject *object, const gchar *property);

GtkWidget *make_style_settings(OpacityPlugin *plugin);
GtkWidget *make_proximity_settings(OpacityPlugin *plugin);
GtkWidget *make_alpha_settings(OpacityPlugin *plugin);

static void opacity_plugin_configure_response(GtkWidget *dialog, gint response, XfcePanelPlugin *panel_plugin) {
    // A close button is fine.
    
    /* remove the dialog data from the plugin */
    g_object_set_data(G_OBJECT(panel_plugin), "dialog", NULL);
    
    /* unlock the panel menu */
    xfce_panel_plugin_unblock_menu(panel_plugin);

    /* save the plugin */
    //DynamicOpacitySave(dynamicOpacity->plugin, dynamicOpacity);

    /* destroy the properties dialog */
    gtk_widget_destroy(dialog);
}

GtkWidget* make_integer_slider(gint min, gint max) {
    GtkWidget *slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, (gdouble) min, (gdouble) max, 1.0);
    gtk_scale_set_draw_value(GTK_SCALE(slider), TRUE);
    gtk_scale_set_digits(GTK_SCALE(slider), 0);
    return slider; // Please don't leak.
}

void bind_slider_value_to(GtkScale *slider, GObject *object, const gchar *property) {    
    guint value = 0; // Make polymorphic? This can't be a double if the binding is a uint.
    
    g_object_get(object, property, &value, NULL);
    gtk_range_set_value(GTK_RANGE(slider), value); // Seemed to be nessessary.
    
    GObject *adjustment; 
    g_object_get(G_OBJECT(slider), "adjustment", &adjustment, NULL); // Should this be a ref?
    g_object_bind_property(
        G_OBJECT(adjustment), "value", 
        G_OBJECT(object), property,
        G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE
    );
    g_object_unref(adjustment);
}

GtkWidget* make_integer_spinner(gint min, gint max) {
    GtkWidget *spinner = gtk_spin_button_new_with_range((gdouble) min, (gdouble) max, 1.0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spinner), 0);
    return spinner; // Please don't leak.
}

// Return GBinding*?
void bind_spinner_value_to(GtkSpinButton *spinner, GObject *object, const gchar *property) {
    guint value = 0; // Make polymorphic? This can't be a double if the binding is a uint.
    
    g_object_get(object, property, &value, NULL);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinner), value); // Seemed to be nessessary.
    
    GObject *adjustment;
    g_object_get(G_OBJECT(spinner), "adjustment", &adjustment, NULL);
    g_object_bind_property(
        G_OBJECT(adjustment), "value", 
        G_OBJECT(object), property,
        G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE
    );
    g_object_unref(adjustment);
}

void bind_toggle_button_value_to(GtkToggleButton *toggle, GObject *object, const gchar *property) {
    gboolean value;    
    g_object_get(object, property, &value, NULL);
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), value); // Seemed to be nessessary.

    g_object_bind_property(
        G_OBJECT(toggle), "active", 
        G_OBJECT(object), property,
        G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE
    );
}

GtkWidget *make_style_settings(OpacityPlugin *plugin) {   
    GtkWidget *grid = gtk_grid_new();    
    gtk_grid_set_column_spacing(GTK_GRID(grid), GRID_PADDING * 2);
    gtk_grid_set_row_spacing(GTK_GRID(grid), GRID_PADDING);
    
    GtkWidget *override_style_check_button = gtk_check_button_new_with_label("Allow forcing panel's background style");   
    bind_toggle_button_value_to(GTK_TOGGLE_BUTTON(override_style_check_button), G_OBJECT(plugin), "override-style");
    
    gtk_grid_attach(GTK_GRID(grid), override_style_check_button, 0, 0, 1, 1);
    gtk_widget_set_halign(override_style_check_button, GTK_ALIGN_START);
    
    return grid;
}

GtkWidget *make_alpha_settings(OpacityPlugin *plugin) {
    GtkWidget *grid = gtk_grid_new();
    
    gtk_grid_set_column_spacing(GTK_GRID(grid), GRID_PADDING * 2);
    gtk_grid_set_row_spacing(GTK_GRID(grid), GRID_PADDING);
    
    // Near alpha slider
    GtkWidget *near_alpha_label = gtk_label_new("Opacity when a window is near:");
    gtk_grid_attach(GTK_GRID(grid), near_alpha_label, 0, 0, 1, 1);
    gtk_widget_set_halign(near_alpha_label, GTK_ALIGN_START);
    
    GtkWidget *near_alpha_slider = make_integer_slider(ALPHA_MIN, ALPHA_MAX);
    bind_slider_value_to(GTK_SCALE(near_alpha_slider), G_OBJECT(plugin), "near-alpha");
    gtk_grid_attach_next_to(GTK_GRID(grid), near_alpha_slider, near_alpha_label, GTK_POS_RIGHT, 1, 1);
    gtk_widget_set_hexpand(near_alpha_slider, TRUE); // Needed for the other one?
    
    // Far alpha slider
    GtkWidget *far_alpha_label = gtk_label_new("Normal opacity:");
    gtk_grid_attach_next_to(GTK_GRID(grid), far_alpha_label, near_alpha_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_halign(far_alpha_label, GTK_ALIGN_START);
    
    GtkWidget *far_alpha_slider = make_integer_slider(ALPHA_MIN, ALPHA_MAX);
    bind_slider_value_to(GTK_SCALE(far_alpha_slider), G_OBJECT(plugin), "far-alpha");
    gtk_grid_attach_next_to(GTK_GRID(grid), far_alpha_slider, far_alpha_label, GTK_POS_RIGHT, 1, 1);
    gtk_widget_set_hexpand(far_alpha_slider, TRUE);
    
    // Transition time
    GtkWidget *transition_time_label = gtk_label_new("Transition time (ms):");
    gtk_grid_attach_next_to(GTK_GRID(grid), transition_time_label, far_alpha_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_halign(transition_time_label, GTK_ALIGN_START);
    
    GtkWidget *transition_time_spinner = make_integer_spinner(TRANSITION_TIME_MIN, TRANSITION_TIME_MAX);
    bind_spinner_value_to(GTK_SPIN_BUTTON(transition_time_spinner), G_OBJECT(plugin), "transition-time");
    gtk_grid_attach_next_to(GTK_GRID(grid), transition_time_spinner, transition_time_label, GTK_POS_RIGHT, 1, 1);
    //gtk_widget_set_hexpand(transition_time_spinner, TRUE);
    gtk_widget_set_halign(transition_time_spinner, GTK_ALIGN_START);
    
    return grid;
}

GtkWidget *make_proximity_settings(OpacityPlugin *plugin) {
    GtkWidget *grid = gtk_grid_new();
    
    gtk_grid_set_column_spacing(GTK_GRID(grid), GRID_PADDING * 2);
    gtk_grid_set_row_spacing(GTK_GRID(grid), GRID_PADDING);
    
    // X proximity
    GtkWidget *x_proximity_label = gtk_label_new("Left and right (px):");
    gtk_grid_attach(GTK_GRID(grid), x_proximity_label, 0, 0, 1, 1);
    gtk_widget_set_halign(x_proximity_label, GTK_ALIGN_START);    
    GtkWidget *x_proximity_spinner = make_integer_spinner(TRANSITION_TIME_MIN, TRANSITION_TIME_MAX);
    bind_spinner_value_to(GTK_SPIN_BUTTON(x_proximity_spinner), G_OBJECT(plugin), "x-proximity");
    gtk_grid_attach_next_to(GTK_GRID(grid), x_proximity_spinner, x_proximity_label, GTK_POS_RIGHT, 1, 1);
    gtk_widget_set_halign(x_proximity_spinner, GTK_ALIGN_START);
    
    // Y proximity
    GtkWidget *y_proximity_label = gtk_label_new("Up and down (px):");
    gtk_grid_attach_next_to(GTK_GRID(grid), y_proximity_label, x_proximity_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_halign(y_proximity_label, GTK_ALIGN_START);    
    GtkWidget *y_proximity_spinner = make_integer_spinner(TRANSITION_TIME_MIN, TRANSITION_TIME_MAX);
    bind_spinner_value_to(GTK_SPIN_BUTTON(y_proximity_spinner), G_OBJECT(plugin), "y-proximity");
    gtk_grid_attach_next_to(GTK_GRID(grid), y_proximity_spinner, y_proximity_label, GTK_POS_RIGHT, 1, 1);
    gtk_widget_set_halign(y_proximity_label, GTK_ALIGN_START);
    
    return grid;
}

void opacity_plugin_configure(XfcePanelPlugin *panel_plugin) {
    OpacityPlugin *plugin = XFCE_OPACITY_PLUGIN(panel_plugin);

    xfce_panel_plugin_block_menu(panel_plugin); // ?
    
    GtkWidget *dialog = xfce_titled_dialog_new_with_buttons(
        _("Dynamic Opacity"),
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(panel_plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "gtk-close", GTK_RESPONSE_OK,
        NULL
    );
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 500);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);    
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "xfce4-settings");
    
    g_object_set_data(G_OBJECT(panel_plugin), "dialog", dialog);
    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(opacity_plugin_configure_response), plugin);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), BORDER);
    
    // Actual settings widgets.
    GtkWidget *container = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *main_grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(container), main_grid, TRUE, TRUE, BORDER);
    gtk_grid_set_column_spacing(GTK_GRID(main_grid), 0);
    gtk_grid_set_row_spacing(GTK_GRID(main_grid), GRID_PADDING);
    
    // Styling.
    GtkWidget *style_settings_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(style_settings_label), "<b>Panel Style</b>");
    gtk_grid_attach(GTK_GRID(main_grid), style_settings_label, 0, 0, 1, 1); // First item
    gtk_widget_set_halign(style_settings_label, GTK_ALIGN_START);    
    GtkWidget *style_settings = make_style_settings(plugin);
    gtk_grid_attach_next_to(GTK_GRID(main_grid), style_settings, style_settings_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_margin_start(style_settings, BORDER);
    
    // Make the proximity settings section.
    GtkWidget *proximity_settings_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(proximity_settings_label), "<b>Window Proximity</b>");
    gtk_grid_attach_next_to(GTK_GRID(main_grid), proximity_settings_label, style_settings, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_halign(proximity_settings_label, GTK_ALIGN_START);    
    GtkWidget *proximity_settings = make_proximity_settings(plugin);
    gtk_grid_attach_next_to(GTK_GRID(main_grid), proximity_settings, proximity_settings_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_margin_start(proximity_settings, BORDER);
    
    // Make the alpha settings section.
    GtkWidget *alpha_settings_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(alpha_settings_label), "<b>Opacity</b>");
    gtk_grid_attach_next_to(GTK_GRID(main_grid), alpha_settings_label, proximity_settings, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_halign(alpha_settings_label, GTK_ALIGN_START);    
    GtkWidget *alpha_settings = make_alpha_settings(plugin);
    gtk_grid_attach_next_to(GTK_GRID(main_grid), alpha_settings, alpha_settings_label, GTK_POS_BOTTOM, 1, 1);
    gtk_widget_set_margin_start(alpha_settings, BORDER);
    
    gtk_widget_show_all(dialog);
}

void opacity_plugin_about(XfcePanelPlugin *plugin) {
    const gchar *auth[] = {
        "Maxwell Hunter <maxwellhunter@protonmail.com>",
        NULL
    };
    
    // I don't have an icon.
    //GdkPixbuf *icon = xfce_panel_pixbuf_from_source("xfce4-sample-plugin", NULL, 32);
    
    gtk_show_about_dialog (
        NULL,
        "logo",         NULL,
        "license",      "GNU General Public License version 2 (GPLv2)",
        "version",      "0.0.1",
        "program-name", "Dynamic Opacity",
        "comments",     _("Changes the opacity of the panel when windows are near."),
        "website",      "",
        "copyright",    _("Copyright \xc2\xa9 2019 Maxwell Hunter\n"),
        "authors",      auth,
        NULL
    );
    /*
    if (icon) {
        g_object_unref(G_OBJECT(icon));
    }
    */
}

