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

#include <arpa/inet.h>         // for inet_ntoa
#include <gdk/gdk.h>           // for GdkEventButton, GdkRGBA, gdk_cursor_new
#include <glib-object.h>       // for g_signal_connect
#include <glib.h>              // for gpointer, TRUE, gboolean, g_idle_add
#include <gtk/gtk.h>           // for gtk_grid_attach, GtkWidget, gtk_button...
#include <netinet/in.h>        // for sockaddr_in, in_addr
#include <stdio.h>             // for NULL, fclose, fopen, sprintf, fgets
#include <string.h>            // for strnlen, strncpy
#include <unistd.h>            // for _exit
#include "configure.h"         // for configure_gpio
#include "discovered.h"        // for DISCOVERED, _DISCOVERED::(anonymous)
#include "ext.h"               // for ext_discovery
#include "gobject/gclosure.h"  // for G_CALLBACK
#include "gpio.h"              // for gpio_set_defaults, gpio_restore_state
#include "log.h"               // for log_trace, log_info
#include "main.h"              // for status_text, controller, top_window
#include "old_discovery.h"     // for old_discovery
#include "pango/pango-font.h"  // for pango_font_description_from_string
#include "protocols.h"         // for configure_protocols, protocols_restore...
#include "radio.h"             // for start_radio, radio

static GtkWidget *discovery_dialog;
static DISCOVERED *d;

GtkWidget *tcpaddr;
#define IPADDR_LEN 20
static char ipaddr_tcp_buf[IPADDR_LEN] = "10.10.10.10";
char *ipaddr_tcp = &ipaddr_tcp_buf[0];

#ifdef CLIENT_SERVER
GtkWidget *host_addr_entry;
static char host_addr_buffer[128]="g0orx.ddns.net";
char *host_addr = &host_addr_buffer[0];
GtkWidget *host_port_spinner;
gint host_port=45000;
#endif

static gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
  _exit(0);
}

static gboolean start_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  radio=(DISCOVERED *)data;
  gtk_widget_destroy(discovery_dialog);
  start_radio();
  return TRUE;
}

#ifdef MIDI
//
// This is a file open dialog. If we choose a readable file here, it is just copied
// to file "midi.props" in the local directory
//
static gboolean midi_cb(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    GtkWidget *opfile,*message;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    int fdin, fdout;
    size_t len,bytes_read,bytes_written;

    opfile = gtk_file_chooser_dialog_new ("Import MIDI description",
                                      GTK_WINDOW(top_window),
                                      action,
                                      "Cancel",
                                      GTK_RESPONSE_CANCEL,
                                      "Open",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);

    res = gtk_dialog_run (GTK_DIALOG (opfile));
    if (res == GTK_RESPONSE_ACCEPT) {
      char *filename, *cp;
      struct stat statbuf;
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (opfile);
      char *contents = NULL;
      filename = gtk_file_chooser_get_filename (chooser);
      fdin =open(filename, O_RDONLY);
      bytes_read = bytes_written = 0;
      if (fdin >= 0) {
        fstat(fdin, &statbuf);
        len=statbuf.st_size;
        //
        // Now first read the whole contents of the file, and then write it out.
        // This is for new-bees trying to import the midi.props in the working dir
        //
        contents=g_new(char, len);
        bytes_read = bytes_written = 0;
        if (contents) {
          bytes_read=read(fdin, contents, len);
        }
        close(fdin);
      }
      fdout=0;
      if (contents && bytes_read == len) {
	// should this file exist as a link or symlink, or should it
	// be read-only, remove it first
	unlink("midi.props");
        fdout=open("midi.props", O_WRONLY | O_CREAT, 0644);
        if (fdout >= 0) {
          bytes_written=write(fdout, contents, len);
          close(fdout);
          g_free(contents);
        }
      }
      if (fdin < 0 || bytes_read < len) {
        message = gtk_message_dialog_new (GTK_WINDOW(top_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"Cannot read input file!\n");
        gtk_dialog_run (GTK_DIALOG (message));
        gtk_widget_destroy(message);
      } else if (fdout < 0 || bytes_written < len) {
        message = gtk_message_dialog_new (GTK_WINDOW(top_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"Cannot write MIDI settings!\n");
        gtk_dialog_run (GTK_DIALOG (message));
        gtk_widget_destroy(message);
      } else {
	// only show basename in the message
	cp = filename + strlen(filename);
        while (cp >= filename) {
	  if (*cp == '/') {
	    cp++;
	    break;
	  }
	  cp--;
	}
        message = gtk_message_dialog_new (GTK_WINDOW(top_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"MIDI import: %ld Bytes read from file %s\n",len,cp);
        gtk_dialog_run (GTK_DIALOG (message));
        gtk_widget_destroy(message);
      }
      g_free(filename);
    }
    gtk_widget_destroy (opfile);
    return TRUE;
}
#endif

static gboolean protocols_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  configure_protocols(discovery_dialog);
  return TRUE;
}

#ifdef GPIO
static gboolean gpio_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  configure_gpio(discovery_dialog);
  return TRUE;
}

static void gpio_changed_cb(GtkWidget *widget, gpointer data) {
  controller=gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
  gpio_set_defaults(controller);
  gpio_save_state();
}
#endif

static gboolean discover_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  gtk_widget_destroy(discovery_dialog);
  g_idle_add(ext_discovery,NULL);
  return TRUE;
}

static gboolean exit_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  gtk_widget_destroy(discovery_dialog);
  _exit(0);
  return TRUE;
}

static gboolean tcp_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
    strncpy(ipaddr_tcp, gtk_entry_get_text(GTK_ENTRY(tcpaddr)), IPADDR_LEN);
    ipaddr_tcp[IPADDR_LEN-1]=0;
    // remove possible trailing newline chars in ipaddr_tcp
	    int len=strnlen(ipaddr_tcp,IPADDR_LEN);
	    while (--len >= 0) {
	      if (ipaddr_tcp[len] != '\n') break;
	      ipaddr_tcp[len]=0;
	    }
	    //fprintf(stderr,"New TCP addr = %s.\n", ipaddr_tcp);
	    // save this value to config file
	    FILE *fp = fopen("ip.addr", "w");
	    if (fp) {
		fprintf(fp,"%s\n",ipaddr_tcp);
		fclose(fp);
	    }
	    gtk_widget_destroy(discovery_dialog);
	    g_idle_add(ext_discovery,NULL);
	    return TRUE;
	}

#ifdef CLIENT_SERVER
static gboolean connect_cb (GtkWidget *widget, GdkEventButton *event, gpointer data) {
  // connect to remote host running piHPSDR
  strncpy(host_addr, gtk_entry_get_text(GTK_ENTRY(host_addr_entry)), 30);
  host_port=gtk_spin_button_get_value(GTK_SPIN_BUTTON(host_port_spinner));
  log_trace("connect_cb: %s:%d",host_addr,host_port);
  setProperty("host",host_addr);
  char temp[16];
  sprintf(temp,"%d",host_port);
  setProperty("port",temp);
  saveProperties("remote.props");
  if(radio_connect_remote(host_addr,host_port)==0) {
    gtk_widget_destroy(discovery_dialog);
  } else {
    // dialog box to display connection error
    GtkWidget *dialog=gtk_dialog_new_with_buttons("Remote Connect",GTK_WINDOW(discovery_dialog),GTK_DIALOG_DESTROY_WITH_PARENT,"OK",GTK_RESPONSE_NONE,NULL);
    GtkWidget *content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    char message[128];
    sprintf(message,"Connection failed to %s:%d",host_addr,host_port);
    GtkWidget *label=gtk_label_new(message);
    g_signal_connect_swapped(dialog,"response",G_CALLBACK(gtk_widget_destroy),dialog);
    gtk_container_add(GTK_CONTAINER(content_area),label);
    gtk_widget_show_all(dialog);
  }
  return TRUE;
}
#endif

void discovery() {
//fprintf(stderr,"discovery\n");

  protocols_restore_state();

  selected_device=0;
  devices=0;

  // Try to locate IP addr
  FILE *fp=fopen("ip.addr","r");
  if (fp) {
    fgets(ipaddr_tcp, IPADDR_LEN,fp);
    fclose(fp);
    ipaddr_tcp[IPADDR_LEN-1]=0;
    // remove possible trailing newline char in ipaddr_tcp
    int len=strnlen(ipaddr_tcp,IPADDR_LEN);
    while (--len >= 0) {
      if (ipaddr_tcp[len] != '\n') break;
      ipaddr_tcp[len]=0;
    }
  }
#ifdef USBOZY
//
// first: look on USB for an Ozy
//
  log_trace("looking for USB based OZY devices");

  if (ozy_discover() != 0)
  {
    discovered[devices].protocol = ORIGINAL_PROTOCOL;
    discovered[devices].device = DEVICE_OZY;
    discovered[devices].software_version = 10;              // we can't know yet so this isn't a real response
    discovered[devices].status = STATE_AVAILABLE;
    strcpy(discovered[devices].name,"Ozy on USB");

    strcpy(discovered[devices].info.network.interface_name,"USB");
    devices++;
  }
#endif

  if(enable_protocol_1) {
    status_text("Protocol 1 ... Discovering Devices");
    old_discovery();
  }

  /* if(enable_protocol_2) { */
  /*   status_text("Protocol 2 ... Discovering Devices"); */
  /*   new_discovery(); */
  /* } */

  status_text("Discovery");
  
    log_info("discovery: found %d devices", devices);
    gdk_window_set_cursor(gtk_widget_get_window(top_window),gdk_cursor_new(GDK_ARROW));

    discovery_dialog = gtk_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(discovery_dialog),GTK_WINDOW(top_window));
    gtk_window_set_title(GTK_WINDOW(discovery_dialog),"piHPSDR - Discovery");
    //gtk_window_set_decorated(GTK_WINDOW(discovery_dialog),FALSE);

    //gtk_widget_override_font(discovery_dialog, pango_font_description_from_string("FreeMono 16"));
    g_signal_connect(discovery_dialog, "delete_event", G_CALLBACK(delete_event_cb), NULL);

    GdkRGBA color;
    color.red = 1.0;
    color.green = 1.0;
    color.blue = 1.0;
    color.alpha = 1.0;
    gtk_widget_override_background_color(discovery_dialog,GTK_STATE_FLAG_NORMAL,&color);

    GtkWidget *content;

    content=gtk_dialog_get_content_area(GTK_DIALOG(discovery_dialog));

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);
    //gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);
    gtk_grid_set_row_spacing (GTK_GRID(grid),10);

    int row=0;
    if(devices==0) {
      GtkWidget *label=gtk_label_new("No local devices found!");
      gtk_grid_attach(GTK_GRID(grid),label,0,row,3,1);
      row++;
    } else {
      char version[16];
      char text[256];
      for(row=0;row<devices;row++) {
        d=&discovered[row];
	log_trace("%p Protocol=%d name=%s",d,d->protocol,d->name);
        sprintf(version,"v%d.%d",
                          d->software_version/10,
                          d->software_version%10);
        switch(d->protocol) {
          case ORIGINAL_PROTOCOL:
          case NEW_PROTOCOL:
#ifdef USBOZY
            if(d->device==DEVICE_OZY) {
              sprintf(text,"%s (%s) on USB /dev/ozy", d->name, d->protocol==ORIGINAL_PROTOCOL?"Protocol 1":"Protocol 2");
            } else {
#endif
              sprintf(text,"%s (%s %s) %s (%02X:%02X:%02X:%02X:%02X:%02X) on %s: ",
                            d->name,
                            d->protocol==ORIGINAL_PROTOCOL?"Protocol 1":"Protocol 2",
                            version,
                            inet_ntoa(d->info.network.address.sin_addr),
                            d->info.network.mac_address[0],
                            d->info.network.mac_address[1],
                            d->info.network.mac_address[2],
                            d->info.network.mac_address[3],
                            d->info.network.mac_address[4],
                            d->info.network.mac_address[5],
                            d->info.network.interface_name);
#ifdef USBOZY
            }
#endif
            break;
        }

        GtkWidget *label=gtk_label_new(text);
        gtk_widget_override_font(label, pango_font_description_from_string("Sans 11"));
        gtk_widget_set_halign (label, GTK_ALIGN_START);
        gtk_widget_show(label);
        gtk_grid_attach(GTK_GRID(grid),label,0,row,3,1);

        GtkWidget *start_button=gtk_button_new_with_label("Start");
        gtk_widget_override_font(start_button, pango_font_description_from_string("Sans 16"));
        gtk_widget_show(start_button);
        gtk_grid_attach(GTK_GRID(grid),start_button,3,row,1,1);
        g_signal_connect(start_button,"button-press-event",G_CALLBACK(start_cb),(gpointer)d);

        // if not available then cannot start it
        if(d->status!=STATE_AVAILABLE) {
          gtk_button_set_label(GTK_BUTTON(start_button),"In Use");
          gtk_widget_set_sensitive(start_button, FALSE);
        }

          // if not on the same subnet then cannot start it
          if((d->info.network.interface_address.sin_addr.s_addr&d->info.network.interface_netmask.sin_addr.s_addr) != (d->info.network.address.sin_addr.s_addr&d->info.network.interface_netmask.sin_addr.s_addr)) {
            gtk_button_set_label(GTK_BUTTON(start_button),"Subnet!");
            gtk_widget_set_sensitive(start_button, FALSE);
          }
      }
    }



#ifdef CLIENT_SERVER

    loadProperties("remote.props");
    char *value;
    value=getProperty("host");
    if(value!=NULL) strcpy(host_addr_buffer,value);
    value=getProperty("port");
    if(value!=NULL) host_port=atoi(value);

    GtkWidget *connect_b=gtk_button_new_with_label("Connect to Server");
    g_signal_connect (connect_b, "button-press-event", G_CALLBACK(connect_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),connect_b,0,row,1,1);

    host_addr_entry=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(host_addr_entry), 30);
    gtk_grid_attach(GTK_GRID(grid),host_addr_entry,1,row,1,1);
    gtk_entry_set_text(GTK_ENTRY(host_addr_entry), host_addr);

    GtkWidget *host_port_label =gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(host_port_label), "<b>Server Port</b>");
    gtk_widget_show(host_port_label);
    gtk_grid_attach(GTK_GRID(grid),host_port_label,2,row,1,1);

    host_port_spinner =gtk_spin_button_new_with_range(45000,55000,1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(host_port_spinner),(double)host_port);
    gtk_widget_show(host_port_spinner);
    gtk_grid_attach(GTK_GRID(grid),host_port_spinner,3,row,1,1);

    row++;
#endif

#ifdef GPIO
    controller=NO_CONTROLLER;
    gpio_set_defaults(controller);
    gpio_restore_state();

    GtkWidget *gpio=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gpio),NULL,"No Controller");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gpio),NULL,"Controller1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gpio),NULL,"Controller2 V1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gpio),NULL,"Controller2 V2");
    //gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(gpio),NULL,"Controller I2C");
    gtk_grid_attach(GTK_GRID(grid),gpio,0,row,1,1);

    gtk_combo_box_set_active(GTK_COMBO_BOX(gpio),controller);
    g_signal_connect(gpio,"changed",G_CALLBACK(gpio_changed_cb),NULL);
#endif

    GtkWidget *discover_b=gtk_button_new_with_label("Discover");
    g_signal_connect (discover_b, "button-press-event", G_CALLBACK(discover_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),discover_b,1,row,1,1);

    GtkWidget *protocols_b=gtk_button_new_with_label("Protocols");
    g_signal_connect (protocols_b, "button-press-event", G_CALLBACK(protocols_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),protocols_b,2,row,1,1);

/*
#ifdef MIDI
    GtkWidget *midi_b=gtk_button_new_with_label("ImportMIDI");
    g_signal_connect (midi_b, "button-press-event", G_CALLBACK(midi_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),midi_b,3,row,1,1);
#endif
*/
    row++;

#ifdef GPIO
    GtkWidget *gpio_b=gtk_button_new_with_label("Configure GPIO");
    g_signal_connect (gpio_b, "button-press-event", G_CALLBACK(gpio_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),gpio_b,0,row,1,1);
#endif

    GtkWidget *tcp_b=gtk_button_new_with_label("Use new TCP Addr:");
    g_signal_connect (tcp_b, "button-press-event", G_CALLBACK(tcp_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),tcp_b,1,row,1,1);

    tcpaddr=gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(tcpaddr), 20);
    gtk_grid_attach(GTK_GRID(grid),tcpaddr,2,row,1,1);
    gtk_entry_set_text(GTK_ENTRY(tcpaddr), ipaddr_tcp);

    GtkWidget *exit_b=gtk_button_new_with_label("Exit");
    g_signal_connect (exit_b, "button-press-event", G_CALLBACK(exit_cb), NULL);
    gtk_grid_attach(GTK_GRID(grid),exit_b,3,row,1,1);

    gtk_container_add (GTK_CONTAINER (content), grid);
    gtk_widget_show_all(discovery_dialog);

    // autostart if one device and autostart enabled
    log_trace("%s: devices=%d autostart=%d",__FUNCTION__,devices,autostart);

    // if(devices==1 && autostart) {
    if(autostart) {
        d=&discovered[0];
	if(d->status==STATE_AVAILABLE) {
          if(start_cb(NULL,NULL,(gpointer)d)) return;
	}
    }
}


