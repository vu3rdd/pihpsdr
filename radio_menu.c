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

#include "radio_menu.h"
#include <gdk/gdk.h>           // for GdkRGBA, GdkEvent, GdkEventButton
#include <glib-object.h>       // for g_signal_connect
#include <glib.h>              // for gpointer, g_idle_add, FALSE, gboolean
#include <glib/gtypes.h>       // for GPOINTER_TO_INT
#include <gtk/gtk.h>           // for gtk_grid_attach, gtk_toggle_button_set...
#include <stdio.h>             // for NULL
#include "band.h"              // for band_get_band, BAND, band_get_current_...
#include "discovered.h"        // for NEW_PROTOCOL, ORIGINAL_PROTOCOL, DISCO...
#include "ext.h"               // for ext_vfo_update, ext_split_toggle
#include "gobject/gclosure.h"  // for G_CALLBACK
#include "main.h"              // for display_width, display_height
#include "new_menu.h"          // for sub_menu
#include "new_protocol.h"      // for filter_board_changed, schedule_high_pr...
#include "radio.h"             // for filter_board, protocol, active_receiver
#include "receiver.h"          // for RECEIVER
#include "sliders.h"           // for att_type_changed
#include "transmitter.h"       // for reconfigure_transmitter, TRANSMITTER

static GtkWidget *parent_window=NULL;
static GtkWidget *dialog=NULL;

static GtkWidget *receivers_1;
static GtkWidget *receivers_2;
static GtkWidget *duplex_b;
static GtkWidget *mute_rx_b;

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


static void calibration_value_changed_cb(GtkWidget *widget, gpointer data) {
  calibration=(long long)gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void rx_gain_calibration_value_changed_cb(GtkWidget *widget, gpointer data) {
  rx_gain_calibration=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void vfo_divisor_value_changed_cb(GtkWidget *widget, gpointer data) {
  vfo_encoder_divisor=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

/*
static void toolbar_dialog_buttons_cb(GtkWidget *widget, gpointer data) {
  toolbar_dialog_buttons=toolbar_dialog_buttons==1?0:1;
  update_toolbar_labels();
}
*/

static void ptt_cb(GtkWidget *widget, gpointer data) {
  mic_ptt_enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void ptt_ring_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    mic_ptt_tip_bias_ring=0;
  }
}

static void ptt_tip_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    mic_ptt_tip_bias_ring=1;
  }
}

static void bias_cb(GtkWidget *widget, gpointer data) {
  mic_bias_enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void iqswap_cb(GtkWidget *widget, gpointer data) {
  iqswap=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void split_cb(GtkWidget *widget, gpointer data) {
  int new=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  if (new != split) g_idle_add(ext_split_toggle, NULL);
}

//
// call-able from outside, e.g. toolbar or MIDI, through g_idle_add
//
void setDuplex() {
  if(duplex) {
    gtk_container_remove(GTK_CONTAINER(fixed),transmitter->panel);
    reconfigure_transmitter(transmitter,display_width/4,display_height/2);
    create_dialog(transmitter);
  } else {
    GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(transmitter->dialog));
    gtk_container_remove(GTK_CONTAINER(content),transmitter->panel);
    gtk_widget_destroy(transmitter->dialog);
    reconfigure_transmitter(transmitter,display_width,rx_height);
  }
  g_idle_add(ext_vfo_update, NULL);
}

static void duplex_cb(GtkWidget *widget, gpointer data) {
  if (isTransmitting()) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (duplex_b), duplex);
    return;
  }
  duplex=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  setDuplex();
}

static void mute_rx_cb(GtkWidget *widget, gpointer data) {
  mute_rx_while_transmitting=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void sat_cb(GtkWidget *widget, gpointer data) {
  sat_mode=gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
  g_idle_add(ext_vfo_update, NULL);
}

void load_filters(void) {
  if(filter_board==N2ADR) {
    // set OC filters
    BAND *band;
    band=band_get_band(band160);
    band->OCrx=band->OCtx=1;
    band=band_get_band(band80);
    band->OCrx=band->OCtx=66;
    band=band_get_band(band60);
    band->OCrx=band->OCtx=68;
    band=band_get_band(band40);
    band->OCrx=band->OCtx=68;
    band=band_get_band(band30);
    band->OCrx=band->OCtx=72;
    band=band_get_band(band20);
    band->OCrx=band->OCtx=72;
    band=band_get_band(band17);
    band->OCrx=band->OCtx=80;
    band=band_get_band(band15);
    band->OCrx=band->OCtx=80;
    band=band_get_band(band12);
    band->OCrx=band->OCtx=96;
    band=band_get_band(band10);
    band->OCrx=band->OCtx=96;
    if(protocol==NEW_PROTOCOL) {
      schedule_high_priority();
    }
    return;
  }

  if(protocol==NEW_PROTOCOL) {
    filter_board_changed();
  }

  if(filter_board==ALEX || filter_board==APOLLO) {
    BAND *band=band_get_current_band();
    // mode and filters have nothing to do with the filter board
    //BANDSTACK_ENTRY* entry=bandstack_entry_get_current();
    //setFrequency(entry->frequency);
    //setMode(entry->mode);
    //set_mode(active_receiver,entry->mode);
    //FILTER* band_filters=filters[entry->mode];
    //FILTER* band_filter=&band_filters[entry->filter];
    //set_filter(active_receiver,band_filter->low,band_filter->high);
    if(active_receiver->id==0) {
      set_alex_rx_antenna(band->alexRxAntenna);
      set_alex_tx_antenna(band->alexTxAntenna);
      set_alex_attenuation(band->alexAttenuation);
    }
  }
  att_type_changed();
}

static void none_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    filter_board = NONE;
    load_filters();
  }
}

static void alex_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    filter_board = ALEX;
    load_filters();
  }
}

static void apollo_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    filter_board = APOLLO;
    load_filters();
  }
}

static void charly25_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    filter_board = CHARLY25;
    load_filters();
  }
}

static void n2adr_cb(GtkWidget *widget, gpointer data) {
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    filter_board = N2ADR;
    load_filters();
  }
}


static void sample_rate_cb(GtkToggleButton *widget, gpointer data) {
  if(gtk_toggle_button_get_active(widget)) {
#ifdef CLIENT_SERVER
    if(radio_is_remote) {
      send_sample_rate(client_socket,-1,GPOINTER_TO_INT(data));
    } else {
#endif
      radio_change_sample_rate(GPOINTER_TO_INT(data));
    }
#ifdef CLIENT_SERVER
  }
#endif
}

static void receivers_cb(GtkToggleButton *widget, gpointer data) {
  //
  // reconfigure_radio requires that the RX panels are active
  // (segfault otherwise), therefore ignore this while TXing
  //
  if (isTransmitting()) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (receivers_1), receivers==1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (receivers_2), receivers==2);
    return;
  }
  if(gtk_toggle_button_get_active(widget)) {
#ifdef CLIENT_SERVER
    if(radio_is_remote) {
      send_receivers(client_socket,GPOINTER_TO_INT(data));
    } else {
#endif
      radio_change_receivers(GPOINTER_TO_INT(data));
#ifdef CLIENT_SERVER
    }
#endif
  }
}

static void region_cb(GtkWidget *widget, gpointer data) {
  radio_change_region(gtk_combo_box_get_active (GTK_COMBO_BOX(widget)));
}

static void rit_cb(GtkWidget *widget,gpointer data) {
  rit_increment=GPOINTER_TO_INT(data);
}

static void ck10mhz_cb(GtkWidget *widget, gpointer data) {
  atlas_clock_source_10mhz = GPOINTER_TO_INT(data);
}

static void ck128mhz_cb(GtkWidget *widget, gpointer data) {
  atlas_clock_source_128mhz=atlas_clock_source_128mhz==1?0:1;
}

static void micsource_cb(GtkWidget *widget, gpointer data) {
  atlas_mic_source=atlas_mic_source==1?0:1;
}

static void penelopetx_cb(GtkWidget *widget, gpointer data) {
  atlas_penelope=atlas_penelope==1?0:1;
}

static void janus_cb(GtkWidget *widget, gpointer data) {
  atlas_janus=atlas_janus==1?0:1;
}

void radio_menu(GtkWidget *parent) {
  parent_window=parent;

  dialog=gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parent_window));
  //gtk_window_set_decorated(GTK_WINDOW(dialog),FALSE);
  gtk_window_set_title(GTK_WINDOW(dialog),"piHPSDR - Radio");
  g_signal_connect (dialog, "delete_event", G_CALLBACK (delete_event), NULL);

  GdkRGBA color;
  color.red = 1.0;
  color.green = 1.0;
  color.blue = 1.0;
  color.alpha = 1.0;
  gtk_widget_override_background_color(dialog,GTK_STATE_FLAG_NORMAL,&color);

  GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_set_column_spacing (GTK_GRID(grid),5);
  gtk_grid_set_row_spacing (GTK_GRID(grid),5);
  gtk_grid_set_column_homogeneous (GTK_GRID(grid), FALSE);
  gtk_grid_set_row_homogeneous (GTK_GRID(grid), FALSE);

  int col=0;
  int row=0;
  int temp_row=0;

  GtkWidget *close_b=gtk_button_new_with_label("Close");
  g_signal_connect (close_b, "button_press_event", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid),close_b,col,row,1,1);

  col++;

  GtkWidget *region_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(region_label), "<b>Region:</b>");
  gtk_grid_attach(GTK_GRID(grid),region_label,col,row,1,1);
  
  col++;

  GtkWidget *region_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(region_combo),NULL,"Other");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(region_combo),NULL,"UK");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(region_combo),NULL,"WRC15");
  gtk_combo_box_set_active(GTK_COMBO_BOX(region_combo),region);
  gtk_grid_attach(GTK_GRID(grid),region_combo,col,row,1,1);
  g_signal_connect(region_combo,"changed",G_CALLBACK(region_cb),NULL);

  col++;


  row++;
  col=0;

  GtkWidget *receivers_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(receivers_label), "<b>Receivers:</b>");
  gtk_label_set_justify(GTK_LABEL(receivers_label),GTK_JUSTIFY_LEFT);
  gtk_grid_attach(GTK_GRID(grid),receivers_label,col,row,1,1);

  row++;
  
  receivers_1=gtk_radio_button_new_with_label(NULL,"1");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (receivers_1), receivers==1);
  gtk_grid_attach(GTK_GRID(grid),receivers_1,col,row,1,1);
  g_signal_connect(receivers_1,"toggled",G_CALLBACK(receivers_cb),(gpointer *)1);

  row++;

  if(radio->supported_receivers>1) {
    receivers_2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(receivers_1),"2");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (receivers_2), receivers==2);
    gtk_grid_attach(GTK_GRID(grid),receivers_2,col,row,1,1);
    g_signal_connect(receivers_2,"toggled",G_CALLBACK(receivers_cb),(gpointer *)2);
    row++;
  }

  col++;
  if(row>temp_row) temp_row=row;

  row=1;

  switch(protocol) {
    case ORIGINAL_PROTOCOL:
      {
      GtkWidget *sample_rate_label=gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(sample_rate_label), "<b>Sample Rate:</b>");
      gtk_grid_attach(GTK_GRID(grid),sample_rate_label,col,row,1,1);
      row++;

      GtkWidget *sample_rate_48=gtk_radio_button_new_with_label(NULL,"48000");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_48), active_receiver->sample_rate==48000);
      gtk_grid_attach(GTK_GRID(grid),sample_rate_48,col,row,1,1);
      g_signal_connect(sample_rate_48,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)48000);
      row++;

      GtkWidget *sample_rate_96=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(sample_rate_48),"96000");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_96), active_receiver->sample_rate==96000);
      gtk_grid_attach(GTK_GRID(grid),sample_rate_96,col,row,1,1);
      g_signal_connect(sample_rate_96,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)96000);
      row++;

      GtkWidget *sample_rate_192=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(sample_rate_96),"192000");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_192), active_receiver->sample_rate==192000);
      gtk_grid_attach(GTK_GRID(grid),sample_rate_192,col,row,1,1);
      g_signal_connect(sample_rate_192,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)192000);
      row++;

      GtkWidget *sample_rate_384=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(sample_rate_192),"384000");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_384), active_receiver->sample_rate==384000);
      gtk_grid_attach(GTK_GRID(grid),sample_rate_384,col,row,1,1);
      g_signal_connect(sample_rate_384,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)384000);
      row++;

      if(protocol==NEW_PROTOCOL) {
        GtkWidget *sample_rate_768=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(sample_rate_384),"768000");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_768), active_receiver->sample_rate==768000);
        gtk_grid_attach(GTK_GRID(grid),sample_rate_768,col,row,1,1);
        g_signal_connect(sample_rate_768,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)768000);
        row++;
  
        GtkWidget *sample_rate_1536=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(sample_rate_768),"1536000");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sample_rate_1536), active_receiver->sample_rate==1536000);
        gtk_grid_attach(GTK_GRID(grid),sample_rate_1536,col,row,1,1);
        g_signal_connect(sample_rate_1536,"toggled",G_CALLBACK(sample_rate_cb),(gpointer *)1536000);
        row++;
  
#ifdef GPIO
        gtk_widget_set_sensitive(sample_rate_768,FALSE);
        gtk_widget_set_sensitive(sample_rate_1536,FALSE);
#endif
      }
      col++;
      }
      break;
  

  }
  row++;
  if(row>temp_row) temp_row=row;


  row=1;

  if(protocol==ORIGINAL_PROTOCOL || protocol==NEW_PROTOCOL) {

    if((protocol==NEW_PROTOCOL && device==NEW_DEVICE_ORION) ||
       (protocol==NEW_PROTOCOL && device==NEW_DEVICE_ORION2) ||
       (protocol==ORIGINAL_PROTOCOL && device==DEVICE_ORION) ||
       (protocol==ORIGINAL_PROTOCOL && device==DEVICE_ORION2)) {

      GtkWidget *ptt_ring_b=gtk_radio_button_new_with_label(NULL,"PTT On Ring, Mic and Bias on Tip");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ptt_ring_b), mic_ptt_tip_bias_ring==0);
      gtk_grid_attach(GTK_GRID(grid),ptt_ring_b,col,row,1,1);
      g_signal_connect(ptt_ring_b,"toggled",G_CALLBACK(ptt_ring_cb),NULL);
      row++;

      GtkWidget *ptt_tip_b=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(ptt_ring_b),"PTT On Tip, Mic and Bias on Ring");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ptt_tip_b), mic_ptt_tip_bias_ring==1);
      gtk_grid_attach(GTK_GRID(grid),ptt_tip_b,col,row,1,1);
      g_signal_connect(ptt_tip_b,"toggled",G_CALLBACK(ptt_tip_cb),NULL);
      row++;

      GtkWidget *ptt_b=gtk_check_button_new_with_label("PTT Enabled");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ptt_b), mic_ptt_enabled);
      gtk_grid_attach(GTK_GRID(grid),ptt_b,col,row,1,1);
      g_signal_connect(ptt_b,"toggled",G_CALLBACK(ptt_cb),NULL);
      row++;

      GtkWidget *bias_b=gtk_check_button_new_with_label("BIAS Enabled");
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bias_b), mic_bias_enabled);
      gtk_grid_attach(GTK_GRID(grid),bias_b,col,row,1,1);
      g_signal_connect(bias_b,"toggled",G_CALLBACK(bias_cb),NULL);
      row++;

      if(row>temp_row) temp_row=row;
      col++;
    }

    row=1;

    GtkWidget *filter_board_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(filter_board_label), "<b>Filter Board:</b>");
    gtk_grid_attach(GTK_GRID(grid),filter_board_label,col,row,1,1);
    row++;

    GtkWidget *none_b = gtk_radio_button_new_with_label(NULL, "NONE");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(none_b), filter_board == NONE);
    gtk_grid_attach(GTK_GRID(grid), none_b, col, row, 1, 1);
    row++;

    GtkWidget *alex_b = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(none_b), "ALEX");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(alex_b), filter_board == ALEX);
    gtk_grid_attach(GTK_GRID(grid), alex_b, col, row, 1, 1);
    row++;

    GtkWidget *apollo_b = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(none_b), "APOLLO");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(apollo_b), filter_board == APOLLO);
    gtk_grid_attach(GTK_GRID(grid), apollo_b, col, row, 1, 1);
    row++;

    GtkWidget *charly25_b = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(none_b), "CHARLY25");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(charly25_b), filter_board==CHARLY25);
    gtk_grid_attach(GTK_GRID(grid), charly25_b, col, row, 1, 1);
    row++;

    GtkWidget *n2adr_b = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(none_b), "N2ADR");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(n2adr_b), filter_board==N2ADR);
    gtk_grid_attach(GTK_GRID(grid), n2adr_b, col, row, 1, 1);
    row++;

    g_signal_connect(none_b, "toggled", G_CALLBACK(none_cb), NULL);
    g_signal_connect(alex_b, "toggled", G_CALLBACK(alex_cb), NULL);
    g_signal_connect(apollo_b, "toggled", G_CALLBACK(apollo_cb), NULL);
    g_signal_connect(charly25_b, "toggled", G_CALLBACK(charly25_cb), NULL);
    g_signal_connect(n2adr_b, "toggled", G_CALLBACK(n2adr_cb), NULL);

    if(row>temp_row) temp_row=row;
    col++;
  }

  row=1;

  GtkWidget *rit_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(rit_label), "<b>RIT/XIT step (Hz):</b>");
  gtk_grid_attach(GTK_GRID(grid),rit_label,col,row,1,1);
  row++;

  GtkWidget *rit_1=gtk_radio_button_new_with_label(NULL,"1");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rit_1), rit_increment==1);
  gtk_grid_attach(GTK_GRID(grid),rit_1,col,row,1,1);
  g_signal_connect(rit_1,"pressed",G_CALLBACK(rit_cb),(gpointer *)1);
  row++;

  GtkWidget *rit_10=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rit_1),"10");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rit_10), rit_increment==10);
  gtk_grid_attach(GTK_GRID(grid),rit_10,col,row,1,1);
  g_signal_connect(rit_10,"pressed",G_CALLBACK(rit_cb),(gpointer *)10);
  row++;

  GtkWidget *rit_100=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(rit_10),"100");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rit_100), rit_increment==100);
  gtk_grid_attach(GTK_GRID(grid),rit_100,col,row,1,1);
  g_signal_connect(rit_100,"pressed",G_CALLBACK(rit_cb),(gpointer *)100);
  row++;

  if(row>temp_row) temp_row=row;
  col++;
  row=1;

#ifdef GPIO
  GtkWidget *vfo_divisor_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(vfo_divisor_label), "<b>VFO Encoder Divisor:</b>");
  gtk_grid_attach(GTK_GRID(grid),vfo_divisor_label,col,row,1,1);
  row++;

  GtkWidget *vfo_divisor=gtk_spin_button_new_with_range(1.0,60.0,1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(vfo_divisor),(double)vfo_encoder_divisor);
  gtk_grid_attach(GTK_GRID(grid),vfo_divisor,col,row,1,1);
  g_signal_connect(vfo_divisor,"value_changed",G_CALLBACK(vfo_divisor_value_changed_cb),NULL);
  row++;
#endif
   
  GtkWidget *iqswap_b=gtk_check_button_new_with_label("Swap IQ");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (iqswap_b), iqswap);
  gtk_grid_attach(GTK_GRID(grid),iqswap_b,col,row,1,1);
  g_signal_connect(iqswap_b,"toggled",G_CALLBACK(iqswap_cb),NULL);
  row++;

  if(row>temp_row) temp_row=row;

  col++;

#ifdef USBOZY
  if (protocol==ORIGINAL_PROTOCOL && (device == DEVICE_OZY || device == DEVICE_METIS))
#else
  if (protocol==ORIGINAL_PROTOCOL && radio->device == DEVICE_METIS)
#endif
  {
    row=1;
    GtkWidget *atlas_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(atlas_label), "<b>Atlas bus:</b>");
    gtk_grid_attach(GTK_GRID(grid),atlas_label,col,row,1,1);
    row++;

    GtkWidget *ck10mhz_1=gtk_radio_button_new_with_label(NULL,"10MHz clock=Atlas");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck10mhz_1), atlas_clock_source_10mhz==0);
    gtk_grid_attach(GTK_GRID(grid),ck10mhz_1,col,row,1,1);
    g_signal_connect(ck10mhz_1,"toggled",G_CALLBACK(ck10mhz_cb),(gpointer *)0);
    row++;

    GtkWidget *ck10mhz_2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(ck10mhz_1),"10MHz clock=Penelope");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck10mhz_2), atlas_clock_source_10mhz==1);
    gtk_grid_attach(GTK_GRID(grid),ck10mhz_2,col,row,1,1);
    g_signal_connect(ck10mhz_2,"toggled",G_CALLBACK(ck10mhz_cb),(gpointer *)1);
    row++;

    GtkWidget *ck10mhz_3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(ck10mhz_2),"10MHz clock=Mercury");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck10mhz_3), atlas_clock_source_10mhz==2);
    gtk_grid_attach(GTK_GRID(grid),ck10mhz_3,col,row,1,1);
    g_signal_connect(ck10mhz_3,"toggled",G_CALLBACK(ck10mhz_cb),(gpointer *)2);
    row++;

    GtkWidget *ck128_b=gtk_check_button_new_with_label("122.88MHz ck=Mercury");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck128_b), atlas_clock_source_128mhz);
    gtk_grid_attach(GTK_GRID(grid),ck128_b,col,row,1,1);
    g_signal_connect(ck128_b,"toggled",G_CALLBACK(ck128mhz_cb),NULL);
    row++;

    GtkWidget *mic_src_b=gtk_check_button_new_with_label("Mic src=Penelope");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mic_src_b), atlas_mic_source);
    gtk_grid_attach(GTK_GRID(grid),mic_src_b,col,row,1,1);
    g_signal_connect(mic_src_b,"toggled",G_CALLBACK(micsource_cb),NULL);
    row++;

    GtkWidget *pene_tx_b=gtk_check_button_new_with_label("Penelope TX");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pene_tx_b), atlas_penelope);
    gtk_grid_attach(GTK_GRID(grid),pene_tx_b,col,row,1,1);
    g_signal_connect(pene_tx_b,"toggled",G_CALLBACK(penelopetx_cb),NULL);
    row++;

    GtkWidget *janus_b=gtk_check_button_new_with_label("Janus");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (janus_b), atlas_janus);
    gtk_grid_attach(GTK_GRID(grid),janus_b,col,row,1,1);
    g_signal_connect(janus_b,"toggled",G_CALLBACK(janus_cb),NULL);
    row++;

    if(row>temp_row) temp_row=row;
  }

  row=temp_row;
  col=0;

  GtkWidget *split_b=gtk_check_button_new_with_label("Split");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (split_b), split);
  gtk_grid_attach(GTK_GRID(grid),split_b,col,row,1,1);
  g_signal_connect(split_b,"toggled",G_CALLBACK(split_cb),NULL);

  col++;

  duplex_b=gtk_check_button_new_with_label("Duplex");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (duplex_b), duplex);
  gtk_grid_attach(GTK_GRID(grid),duplex_b,col,row,1,1);
  g_signal_connect(duplex_b,"toggled",G_CALLBACK(duplex_cb),NULL);

  col++;

  
  GtkWidget *sat_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(sat_combo),NULL,"SAT Off");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(sat_combo),NULL,"SAT");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(sat_combo),NULL,"RSAT");
  gtk_combo_box_set_active(GTK_COMBO_BOX(sat_combo),sat_mode);
  gtk_grid_attach(GTK_GRID(grid),sat_combo,col,row,1,1);
  g_signal_connect(sat_combo,"changed",G_CALLBACK(sat_cb),NULL);

  col++;

  mute_rx_b=gtk_check_button_new_with_label("Mute RX when TX");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mute_rx_b), mute_rx_while_transmitting);
  gtk_grid_attach(GTK_GRID(grid),mute_rx_b,col,row,1,1);
  g_signal_connect(mute_rx_b,"toggled",G_CALLBACK(mute_rx_cb),NULL);

  row++;
  col=0;
 
  GtkWidget *calibration_label=gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(calibration_label), "<b>Frequency\nCalibration(Hz):</b>");
  gtk_grid_attach(GTK_GRID(grid),calibration_label,col,row,1,1);
  col++;

  GtkWidget *calibration_b=gtk_spin_button_new_with_range(-10000.0,10000.0,1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(calibration_b),(double)calibration);
  gtk_grid_attach(GTK_GRID(grid),calibration_b,col,row,1,1);
  g_signal_connect(calibration_b,"value_changed",G_CALLBACK(calibration_value_changed_cb),NULL);

  if(have_rx_gain) {
    col++;
    GtkWidget *rx_gain_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(rx_gain_label), "<b>RX Gain Calibration:</b>");
    gtk_grid_attach(GTK_GRID(grid),rx_gain_label,col,row,1,1);
    col++;
    
    GtkWidget *rx_gain_calibration_b=gtk_spin_button_new_with_range(-50.0,50.0,1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(rx_gain_calibration_b),(double)rx_gain_calibration);
    gtk_grid_attach(GTK_GRID(grid),rx_gain_calibration_b,col,row,1,1);
    g_signal_connect(rx_gain_calibration_b,"value_changed",G_CALLBACK(rx_gain_calibration_value_changed_cb),NULL);

  }
  row++;

  if(row>temp_row) temp_row=row;



  gtk_container_add(GTK_CONTAINER(content),grid);

  sub_menu=dialog;

  gtk_widget_show_all(dialog);

}
