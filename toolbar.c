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

#include <gtk/gtk.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "actions.h"
#include "gpio.h"
#include "toolbar.h"
#include "mode.h"
#include "filter.h"
#include "bandstack.h"
#include "band.h"
#include "discovered.h"
#include "new_protocol.h"
#include "old_protocol.h"
#include "vfo.h"
#include "alex.h"
#include "agc.h"
#include "channel.h"
#include "wdsp.h"
#include "radio.h"
#include "receiver.h"
#include "transmitter.h"
#include "property.h"
#include "new_menu.h"
#include "button_text.h"
#include "ext.h"	
#ifdef CLIENT_SERVER
#include "client_server.h"
#endif
#include "log.h"

int function=0;

static int width;
static int height;

static GtkWidget *parent_window;
static GtkWidget *toolbar;

static GtkWidget *last_dialog;

static GtkWidget *sim_mox;
static GtkWidget *sim_s1;
static GtkWidget *sim_s2;
static GtkWidget *sim_s3;
static GtkWidget *sim_s4;
static GtkWidget *sim_s5;
static GtkWidget *sim_s6;
static GtkWidget *sim_function;

static GdkRGBA white;
static GdkRGBA gray;

SWITCH *toolbar_switches=switches_controller1[0];

void update_toolbar_labels() {
  gtk_button_set_label(GTK_BUTTON(sim_mox),ActionTable[toolbar_switches[0].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s1),ActionTable[toolbar_switches[1].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s2),ActionTable[toolbar_switches[2].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s3),ActionTable[toolbar_switches[3].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s4),ActionTable[toolbar_switches[4].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s5),ActionTable[toolbar_switches[5].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_s6),ActionTable[toolbar_switches[6].switch_function].button_str);
  gtk_button_set_label(GTK_BUTTON(sim_function),ActionTable[toolbar_switches[7].switch_function].button_str);
}

void band_cb(GtkWidget *widget, gpointer data) {
  start_band();
}

void bandstack_cb(GtkWidget *widget, gpointer data) {
  start_bandstack();
}

void mode_cb(GtkWidget *widget, gpointer data) {
  start_mode();
}

void filter_cb(GtkWidget *widget, gpointer data) {
  start_filter();
}

void agc_cb(GtkWidget *widget, gpointer data) {
  start_agc();
}

void noise_cb(GtkWidget *widget, gpointer data) {
  start_noise();
}

void ctun_cb (GtkWidget *widget, gpointer data) {
  int id=active_receiver->id;
#ifdef CLIENT_SERVER
  if(radio_is_remote) {
    send_ctun(client_socket,id,vfo[id].ctun==1?0:1);
  } else {
#endif
    vfo[id].ctun=vfo[id].ctun==1?0:1;
    if(!vfo[id].ctun) {
      vfo[id].offset=0;
    }
    vfo[id].ctun_frequency=vfo[id].frequency;
    set_offset(active_receiver,vfo[id].offset);
    g_idle_add(ext_vfo_update,NULL);
#ifdef CLIENT_SERVER
  }
#endif
}

void lock_cb(GtkWidget *widget, gpointer data) {
#ifdef CLIENT_SERVER
  if(radio_is_remote) {
    send_lock(client_socket,locked==1?0:1);
  } else {
#endif
    locked=locked==1?0:1;
    g_idle_add(ext_vfo_update,NULL);
#ifdef CLIENT_SERVER
  }
#endif
}

void mox_cb(GtkWidget *widget, gpointer data) {

  if(getTune()==1) {
    setTune(0);
  }
  if(getMox()==1) {
    setMox(0);
  } else if(canTransmit() || tx_out_of_band) {
    setMox(1);
  } else {
    transmitter_set_out_of_band(transmitter);
  }
  g_idle_add(ext_vfo_update,NULL);
}

void mox_update(int state) {
  if(getTune()==1) {
    setTune(0);
  }
  if(state) {
    if(canTransmit() || tx_out_of_band) {
      setMox(state);
    } else {
      transmitter_set_out_of_band(transmitter);
    }
  } else {
    setMox(state);
  }
  g_idle_add(ext_vfo_update,NULL);
}

void tune_cb(GtkWidget *widget, gpointer data) {
  if(getMox()==1) {
    setMox(0);
  }
  if(getTune()==1) {
    setTune(0);
  } else if(canTransmit() || tx_out_of_band) {
    setTune(1);
  } else {
    transmitter_set_out_of_band(transmitter);
  }
  g_idle_add(ext_vfo_update,NULL);
}

void tune_update(int state) {
  if(getMox()==1) {
    setMox(0);
  }
  if(state) {
    setTune(0);
    if(canTransmit() || tx_out_of_band) {
      setTune(1);
    } else {
      transmitter_set_out_of_band(transmitter);
    }
  } else {
    setTune(state);
  }
  g_idle_add(ext_vfo_update,NULL);
}

void switch_pressed_cb(GtkWidget *widget, gpointer data) {
  gint i=GPOINTER_TO_INT(data);
  PROCESS_ACTION *a=g_new(PROCESS_ACTION,1);
  a->action=toolbar_switches[i].switch_function;
  a->mode=PRESSED;
  g_idle_add(process_action,a);
}

void switch_released_cb(GtkWidget *widget, gpointer data) {
  gint i=GPOINTER_TO_INT(data);
  PROCESS_ACTION *a=g_new(PROCESS_ACTION,1);
  a->action=toolbar_switches[i].switch_function;
  a->mode=RELEASED;
  g_idle_add(process_action,a);
}

GtkWidget *toolbar_init(int my_width, int my_height, GtkWidget* parent) {
    width=my_width;
    height=my_height;
    parent_window=parent;

    int button_width=width/8;

    log_trace("toolbar_init: width=%d height=%d button_width=%d", width,height,button_width);

    white.red=1.0;
    white.green=1.0;
    white.blue=1.0;
    white.alpha=0.0;

    gray.red=0.25;
    gray.green=0.25;
    gray.blue=0.25;
    gray.alpha=0.0;

    toolbar=gtk_grid_new();
    gtk_widget_set_size_request (toolbar, width, height);
    gtk_grid_set_column_homogeneous(GTK_GRID(toolbar),TRUE);

    sim_mox=gtk_button_new_with_label(ActionTable[toolbar_switches[0].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_mox),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(0));
    gtk_grid_attach(GTK_GRID(toolbar),sim_mox,0,0,4,1);

    sim_s1=gtk_button_new_with_label(ActionTable[toolbar_switches[1].switch_function].button_str);
    gtk_widget_set_size_request (sim_s1, button_width, 0);
    g_signal_connect(G_OBJECT(sim_s1),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(sim_s1),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(1));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s1,4,0,4,1);

    sim_s2=gtk_button_new_with_label(ActionTable[toolbar_switches[2].switch_function].button_str);
    gtk_widget_set_size_request (sim_s2, button_width, 0);
    g_signal_connect(G_OBJECT(sim_s2),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(2));
    g_signal_connect(G_OBJECT(sim_s2),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(2));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s2,8,0,4,1);

    sim_s3=gtk_button_new_with_label(ActionTable[toolbar_switches[3].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_s3),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(3));
    g_signal_connect(G_OBJECT(sim_s3),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(3));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s3,12,0,4,1);

    sim_s4=gtk_button_new_with_label(ActionTable[toolbar_switches[4].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_s4),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(4));
    g_signal_connect(G_OBJECT(sim_s4),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(4));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s4,16,0,4,1);

    sim_s5=gtk_button_new_with_label(ActionTable[toolbar_switches[5].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_s5),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(5));
    g_signal_connect(G_OBJECT(sim_s5),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(5));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s5,20,0,4,1);

    sim_s6=gtk_button_new_with_label(ActionTable[toolbar_switches[6].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_s6),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(6));
    g_signal_connect(G_OBJECT(sim_s6),"released",G_CALLBACK(switch_released_cb),GINT_TO_POINTER(6));
    gtk_grid_attach(GTK_GRID(toolbar),sim_s6,24,0,4,1);

    sim_function=gtk_button_new_with_label(ActionTable[toolbar_switches[7].switch_function].button_str);
    g_signal_connect(G_OBJECT(sim_function),"pressed",G_CALLBACK(switch_pressed_cb),GINT_TO_POINTER(7));
    gtk_grid_attach(GTK_GRID(toolbar),sim_function,28,0,4,1);

    last_dialog=NULL;

    gtk_widget_show_all(toolbar);

  return toolbar;
}
