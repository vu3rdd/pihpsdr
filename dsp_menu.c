/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
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

#include "dsp_menu.h"
#include <gdk/gdk.h>           // for GdkRGBA, GdkEvent, GdkEventButton
#include <glib-object.h>       // for g_signal_connect, G_OBJECT
#include <glib.h>              // for gpointer, gboolean, FALSE, TRUE
#include <glib/gtypes.h>       // for GPOINTER_TO_UINT
#include <gtk/gtk.h>           // for gtk_grid_attach, gtk_widget_show, GtkW...
#include <stdio.h>             // for NULL
#include "agc.h"               // for AGC_LONG, AGC_SLOW
#include "gobject/gclosure.h"  // for G_CALLBACK
#include "new_menu.h"          // for sub_menu
#include "radio.h"             // for active_receiver
#include "receiver.h"          // for RECEIVER
#include "wdsp.h"              // for SetRXAAGCHangThreshold, SetRXAEMNRPosi...

static GtkWidget *parent_window=NULL;

static GtkWidget *dialog=NULL;

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

static void agc_hang_threshold_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->agc_hang_threshold=(int)gtk_range_get_value(GTK_RANGE(widget));
  if(active_receiver->agc==AGC_LONG || active_receiver->agc==AGC_SLOW) {
    SetRXAAGCHangThreshold(active_receiver->id, (int)active_receiver->agc_hang_threshold);
  }
}

static void nr4_reduction_amount_scale_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_reduction_amount=(int)gtk_range_get_value(GTK_RANGE(widget));

  SetRXASBNRreductionAmount(active_receiver->id, (int)active_receiver->nr4_reduction_amount);
}

static void nr4_smoothing_factor_scale_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_smoothing_factor=(int)gtk_range_get_value(GTK_RANGE(widget));

  SetRXASBNRsmoothingFactor(active_receiver->id, (int)active_receiver->nr4_smoothing_factor);
}

static void nr4_whitening_factor_scale_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_whitening_factor = (int)gtk_range_get_value(GTK_RANGE(widget));

  SetRXASBNRwhiteningFactor(active_receiver->id, (int)active_receiver->nr4_whitening_factor);
}

static void nr4_noise_rescale_scale_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_noise_rescale=(int)gtk_range_get_value(GTK_RANGE(widget));

  SetRXASBNRnoiseRescale(active_receiver->id, (int)active_receiver->nr4_noise_rescale);
}

static void nr4_post_filter_threshold_scale_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->nr4_post_filter_threshold=(int)gtk_range_get_value(GTK_RANGE(widget));

  SetRXASBNRpostFilterThreshold(active_receiver->id, (int)active_receiver->nr4_post_filter_threshold);
}

static void pre_post_agc_cb(GtkToggleButton *widget, gpointer data) {
  if(gtk_toggle_button_get_active(widget)) {
    active_receiver->nr_agc=GPOINTER_TO_UINT(data);
    SetRXAEMNRPosition(active_receiver->id, active_receiver->nr_agc);
  }
}

static void nr2_gain_cb(GtkToggleButton *widget, gpointer data) {
  if(gtk_toggle_button_get_active(widget)) {
    active_receiver->nr2_gain_method=GPOINTER_TO_UINT(data);
    SetRXAEMNRgainMethod(active_receiver->id, active_receiver->nr2_gain_method);
  }
}

static void nr2_npe_method_cb(GtkToggleButton *widget, gpointer data) {
  if(gtk_toggle_button_get_active(widget)) {
    active_receiver->nr2_npe_method=GPOINTER_TO_UINT(data);
    SetRXAEMNRnpeMethod(active_receiver->id, active_receiver->nr2_npe_method);
  }
}

static void ae_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    active_receiver->nr2_ae=1;
  } else {
    active_receiver->nr2_ae=0;
  }
  SetRXAEMNRaeRun(active_receiver->id, active_receiver->nr2_ae);
}

void dsp_menu(GtkWidget *parent) {
  parent_window=parent;

  dialog=gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent_window));
  //gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
  gtk_window_set_title(GTK_WINDOW(dialog),"piHPSDR - DSP");
  g_signal_connect (dialog, "delete_event", G_CALLBACK (delete_event), NULL);

  GdkRGBA color;
  color.red = 1.0;
  color.green = 1.0;
  color.blue = 1.0;
  color.alpha = 1.0;
  gtk_widget_override_background_color(dialog,GTK_STATE_FLAG_NORMAL,&color);

  GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_set_column_spacing (GTK_GRID(grid),10);
  //gtk_grid_set_row_spacing (GTK_GRID(grid),10);
  //gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);
  //gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);

  GtkWidget *close_b=gtk_button_new_with_label("Close");
  g_signal_connect (close_b, "pressed", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid),close_b,0,0,1,1);

  GtkWidget *agc_hang_threshold_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(agc_hang_threshold_label), "<b>AGC Hang Threshold</b>");
  gtk_widget_show(agc_hang_threshold_label);
  gtk_grid_attach(GTK_GRID(grid),agc_hang_threshold_label,0,1,1,1);

  GtkWidget *agc_hang_threshold_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(agc_hang_threshold_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(agc_hang_threshold_scale),active_receiver->agc_hang_threshold);
  gtk_widget_show(agc_hang_threshold_scale);
  gtk_grid_attach(GTK_GRID(grid),agc_hang_threshold_scale,1,1,2,1);
  g_signal_connect(G_OBJECT(agc_hang_threshold_scale),"value_changed",G_CALLBACK(agc_hang_threshold_value_changed_cb),NULL);

  GtkWidget *pre_post_agc_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(pre_post_agc_label), "<b>NR/NR2/ANF</b>");
  //gtk_widget_override_font(pre_post_agc_label, pango_font_description_from_string("Arial 18"));
  gtk_widget_show(pre_post_agc_label);
  gtk_grid_attach(GTK_GRID(grid),pre_post_agc_label,0,2,1,1);

  GtkWidget *pre_agc_b=gtk_radio_button_new_with_label(NULL,"Pre AGC");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pre_agc_b),active_receiver->nr_agc==0);
  gtk_widget_show(pre_agc_b);
  gtk_grid_attach(GTK_GRID(grid),pre_agc_b,1,2,1,1);
  g_signal_connect(pre_agc_b,"toggled",G_CALLBACK(pre_post_agc_cb),(gpointer *)0);

  GtkWidget *post_agc_b=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(pre_agc_b),"Post AGC");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (post_agc_b), active_receiver->nr_agc==1);
  gtk_widget_show(post_agc_b);
  gtk_grid_attach(GTK_GRID(grid),post_agc_b,2,2,1,1);
  g_signal_connect(post_agc_b,"toggled",G_CALLBACK(pre_post_agc_cb),(gpointer *)1);

  GtkWidget *nr2_gain_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(nr2_gain_label), "<b>NR Gain Method</b>");
  //gtk_widget_override_font(nr2_gain_label, pango_font_description_from_string("Arial 18"));
  gtk_widget_show(nr2_gain_label);
  gtk_grid_attach(GTK_GRID(grid),nr2_gain_label,0,3,1,1);

  GtkWidget *linear_b=gtk_radio_button_new_with_label(NULL,"Linear");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (linear_b),active_receiver->nr2_gain_method==0);
  gtk_widget_show(linear_b);
  gtk_grid_attach(GTK_GRID(grid),linear_b,1,3,1,1);
  g_signal_connect(linear_b,"toggled",G_CALLBACK(nr2_gain_cb),(gpointer *)0);

  GtkWidget *log_b=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(linear_b),"Log");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (log_b), active_receiver->nr2_gain_method==1);
  gtk_widget_show(log_b);
  gtk_grid_attach(GTK_GRID(grid),log_b,2,3,1,1);
  g_signal_connect(log_b,"toggled",G_CALLBACK(nr2_gain_cb),(gpointer *)1);

  GtkWidget *gamma_b=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(log_b),"Gamma");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gamma_b), active_receiver->nr2_gain_method==2);
  gtk_widget_show(gamma_b);
  gtk_grid_attach(GTK_GRID(grid),gamma_b,3,3,1,1);
  g_signal_connect(gamma_b,"toggled",G_CALLBACK(nr2_gain_cb),(gpointer *)2);

  GtkWidget *nr2_npe_method_label=gtk_label_new("NR2 NPE Method");
  gtk_label_set_markup(GTK_LABEL(nr2_npe_method_label), "<b>NR2 NPE Method</b>");
  //gtk_widget_override_font(nr2_npe_method_label, pango_font_description_from_string("Arial 18"));
  gtk_widget_show(nr2_npe_method_label);
  gtk_grid_attach(GTK_GRID(grid),nr2_npe_method_label,0,4,1,1);

  GtkWidget *osms_b=gtk_radio_button_new_with_label(NULL,"OSMS");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (osms_b),active_receiver->nr2_npe_method==0);
  gtk_widget_show(osms_b);
  gtk_grid_attach(GTK_GRID(grid),osms_b,1,4,1,1);
  g_signal_connect(osms_b,"toggled",G_CALLBACK(nr2_npe_method_cb),(gpointer *)0);

  GtkWidget *mmse_b=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(osms_b),"MMSE");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mmse_b), active_receiver->nr2_npe_method==1);
  gtk_widget_show(mmse_b);
  gtk_grid_attach(GTK_GRID(grid),mmse_b,2,4,1,1);
  g_signal_connect(mmse_b,"toggled",G_CALLBACK(nr2_npe_method_cb),(gpointer *)1);

  GtkWidget *ae_b=gtk_check_button_new_with_label("NR2 AE Filter");
  //gtk_widget_override_font(ae_b, pango_font_description_from_string("Arial 18"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ae_b), active_receiver->nr2_ae);
  gtk_widget_show(ae_b);
  gtk_grid_attach(GTK_GRID(grid),ae_b,0,5,1,1);
  g_signal_connect(ae_b,"toggled",G_CALLBACK(ae_cb),NULL);

  GtkWidget *nr4_reduction_amount_label=gtk_label_new("NR4 Reduction Amount");
  gtk_label_set_markup(GTK_LABEL(nr4_reduction_amount_label), "<b>NR4 Reduction Amount</b>");
  gtk_widget_show(nr4_reduction_amount_label);
  gtk_grid_attach(GTK_GRID(grid),nr4_reduction_amount_label,0,6,1,1);

  GtkWidget *nr4_reduction_amount_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 20.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(nr4_reduction_amount_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(nr4_reduction_amount_scale),active_receiver->nr4_reduction_amount);
  gtk_widget_show(nr4_reduction_amount_scale);
  gtk_grid_attach(GTK_GRID(grid),nr4_reduction_amount_scale,1,6,2,1);
  g_signal_connect(G_OBJECT(nr4_reduction_amount_scale),"value_changed",G_CALLBACK(nr4_reduction_amount_scale_changed_cb),NULL);

  GtkWidget *nr4_smoothing_factor_label=gtk_label_new("NR4 Smoothing factor");
  gtk_label_set_markup(GTK_LABEL(nr4_smoothing_factor_label), "<b>NR4 Smoothing factor</b>");
  gtk_widget_show(nr4_smoothing_factor_label);
  gtk_grid_attach(GTK_GRID(grid),nr4_smoothing_factor_label,0,7,1,1);

  GtkWidget *nr4_smoothing_factor_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(nr4_smoothing_factor_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(nr4_smoothing_factor_scale),active_receiver->nr4_smoothing_factor);
  gtk_widget_show(nr4_smoothing_factor_scale);
  gtk_grid_attach(GTK_GRID(grid),nr4_smoothing_factor_scale,1,7,2,1);
  g_signal_connect(G_OBJECT(nr4_smoothing_factor_scale),"value_changed",G_CALLBACK(nr4_smoothing_factor_scale_changed_cb),NULL);

  GtkWidget *nr4_whitening_factor_label=gtk_label_new("NR4 Whitening factor");
  gtk_label_set_markup(GTK_LABEL(nr4_whitening_factor_label), "<b>NR4 Whitening factor</b>");
  gtk_widget_show(nr4_whitening_factor_label);
  gtk_grid_attach(GTK_GRID(grid),nr4_whitening_factor_label,0,8,1,1);

  GtkWidget *nr4_whitening_factor_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(nr4_whitening_factor_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(nr4_whitening_factor_scale),active_receiver->nr4_whitening_factor);
  gtk_widget_show(nr4_whitening_factor_scale);
  gtk_grid_attach(GTK_GRID(grid),nr4_whitening_factor_scale,1,8,2,1);
  g_signal_connect(G_OBJECT(nr4_whitening_factor_scale),"value_changed",G_CALLBACK(nr4_whitening_factor_scale_changed_cb),NULL);

  GtkWidget *nr4_noise_rescale_label=gtk_label_new("NR4 noise Rescale");
  gtk_label_set_markup(GTK_LABEL(nr4_noise_rescale_label), "<b>NR4 noise rescale</b>");
  gtk_widget_show(nr4_noise_rescale_label);
  gtk_grid_attach(GTK_GRID(grid),nr4_noise_rescale_label,0,9,1,1);

  GtkWidget *nr4_noise_rescale_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 12.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(nr4_noise_rescale_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(nr4_noise_rescale_scale),active_receiver->nr4_noise_rescale);
  gtk_widget_show(nr4_noise_rescale_scale);
  gtk_grid_attach(GTK_GRID(grid),nr4_noise_rescale_scale,1,9,2,1);
  g_signal_connect(G_OBJECT(nr4_noise_rescale_scale),"value_changed",G_CALLBACK(nr4_noise_rescale_scale_changed_cb),NULL);

  GtkWidget *nr4_post_filter_threshold_label=gtk_label_new("NR4 post filter threshold");
  gtk_label_set_markup(GTK_LABEL(nr4_post_filter_threshold_label), "<b>NR4 post filter threshold</b>");
  gtk_widget_show(nr4_post_filter_threshold_label);
  gtk_grid_attach(GTK_GRID(grid),nr4_post_filter_threshold_label,0,10,1,1);

  GtkWidget *nr4_post_filter_threshold_scale=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -10.0, 10.0, 1.0);
  gtk_range_set_increments (GTK_RANGE(nr4_post_filter_threshold_scale),1.0,1.0);
  gtk_range_set_value (GTK_RANGE(nr4_post_filter_threshold_scale),active_receiver->nr4_post_filter_threshold);
  gtk_widget_show(nr4_post_filter_threshold_scale);
  gtk_grid_attach(GTK_GRID(grid),nr4_post_filter_threshold_scale,1,10,2,1);
  g_signal_connect(G_OBJECT(nr4_post_filter_threshold_scale),"value_changed",G_CALLBACK(nr4_post_filter_threshold_scale_changed_cb),NULL);

  gtk_container_add(GTK_CONTAINER(content),grid);

  sub_menu=dialog;

  gtk_widget_show_all(dialog);

}

