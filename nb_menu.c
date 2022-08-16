/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
* 2022 - Ramakrishnan Muthukrishnan VU2JXN <ram@rkrishnan.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <gtk/gtk.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "new_menu.h"

#include <wdsp.h>

static GtkWidget *parent_window = NULL;
static GtkWidget *menu_b = NULL;
static GtkWidget *dialog = NULL;

double nb_lag_time = 0.0001;
double nb_lead_time = 0.0001;
double nb_transition_time = 0.0001;
double nb_threshold_value = 20.0;

void nb_changed() {
    SetEXTANBHangtime(0, nb_lag_time);
    SetEXTANBAdvtime(0, nb_lead_time);
    SetEXTANBTau(0, nb_transition_time);
    SetEXTANBThreshold(0, nb_threshold_value);
}

static void cleanup() {
  if(dialog!=NULL) {
    gtk_widget_destroy(dialog);
    dialog=NULL;
    sub_menu=NULL;
  }
}

static gboolean close_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
    cleanup();
    return TRUE;
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    cleanup();
    return FALSE;
}

static void nb_lag_time_value_changed_cb(GtkWidget *widget, gpointer data) {
    nb_lag_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    nb_lag_time *= 0.001; // convert ms to sec
    nb_changed();
}

static void nb_lead_time_value_changed_cb(GtkWidget *widget, gpointer data) {
    nb_lead_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    nb_lead_time *= 0.001;
    nb_changed();
}

static void nb_transition_time_value_changed_cb(GtkWidget *widget, gpointer data) {
    nb_transition_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    nb_transition_time *= 0.001; // ms to s
    nb_changed();
}

static void nb_threshold_value_changed_cb(GtkWidget *widget, gpointer data) {
    nb_threshold_value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
    nb_changed();
}

void nb_menu(GtkWidget *parent) {
    parent_window = parent;

    dialog = gtk_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent_window));
    gtk_window_set_title(GTK_WINDOW(dialog),"piHPSDR - Noise Blanker");
    g_signal_connect (dialog, "delete_event", G_CALLBACK (delete_event), NULL);

    GdkRGBA color;
    color.red = 1.0;
    color.green = 1.0;
    color.blue = 1.0;
    color.alpha = 1.0;
    gtk_widget_override_background_color(dialog,GTK_STATE_FLAG_NORMAL,&color);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing (GTK_GRID(grid),10);
    //gtk_grid_set_row_spacing (GTK_GRID(grid),10);
    //gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);
    //gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);

    GtkWidget *close_b = gtk_button_new_with_label("Close");

    g_signal_connect (close_b, "pressed", G_CALLBACK(close_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),close_b,0,0,1,1);

    GtkWidget *nb_lag_label=gtk_label_new("NB Lag/Hang (ms)");
    gtk_widget_show(nb_lag_label);
    gtk_grid_attach(GTK_GRID(grid),nb_lag_label,0,1,1,1);

    GtkWidget *nb_lead_label=gtk_label_new("NB Lead/Adv (ms)");
    gtk_widget_show(nb_lead_label);
    gtk_grid_attach(GTK_GRID(grid),nb_lead_label,0,2,1,1);

    GtkWidget *nb_transition_label=gtk_label_new("NB Transition/Tau (ms)");
    gtk_widget_show(nb_transition_label);
    gtk_grid_attach(GTK_GRID(grid),nb_transition_label,0,3,1,1);

    GtkWidget *nb_threshold_label=gtk_label_new("NB Threshold");
    gtk_widget_show(nb_threshold_label);
    gtk_grid_attach(GTK_GRID(grid),nb_threshold_label,0,4,1,1);

    GtkWidget *nb2_mode_label=gtk_label_new("NB2 Mode");
    gtk_widget_show(nb2_mode_label);
    gtk_grid_attach(GTK_GRID(grid),nb2_mode_label,0,5,1,1);

    // lag time spin button
    GtkWidget *nb_lag_time_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(nb_lag_time_b),(double)nb_lag_time);
    gtk_widget_show(nb_lag_time_b);
    gtk_grid_attach(GTK_GRID(grid),nb_lag_time_b,1,1,1,1);
    g_signal_connect(nb_lag_time_b,"value_changed",G_CALLBACK(nb_lag_time_value_changed_cb),NULL);

    GtkWidget *nb_lead_time_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(nb_lead_time_b),(double)nb_lead_time);
    gtk_widget_show(nb_lead_time_b);
    gtk_grid_attach(GTK_GRID(grid),nb_lead_time_b,1,2,1,1);
    g_signal_connect(nb_lead_time_b,"value_changed",G_CALLBACK(nb_lead_time_value_changed_cb),NULL);

    GtkWidget *nb_transition_time_b=gtk_spin_button_new_with_range(0.0, 0.1, 0.0001);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(nb_transition_time_b),(double)nb_transition_time);
    gtk_widget_show(nb_transition_time_b);
    gtk_grid_attach(GTK_GRID(grid),nb_transition_time_b,1,3,1,1);
    g_signal_connect(nb_transition_time_b,"value_changed",G_CALLBACK(nb_transition_time_value_changed_cb),NULL);

    GtkWidget *nb_threshold_value_b=gtk_spin_button_new_with_range(15.0, 500.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(nb_threshold_value_b),(double)nb_threshold_value);
    gtk_widget_show(nb_threshold_value_b);
    gtk_grid_attach(GTK_GRID(grid),nb_threshold_value_b,1,4,1,1);
    g_signal_connect(nb_threshold_value_b,"value_changed",G_CALLBACK(nb_threshold_value_changed_cb),NULL);

    gtk_container_add(GTK_CONTAINER(content),grid);
    sub_menu=dialog;
    gtk_widget_show_all(dialog);
}
