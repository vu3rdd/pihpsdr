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

// Define maximum window size.
// Standard values 800 and 480: suitable for RaspberryBi 7-inch screen

#define MAX_DISPLAY_WIDTH 1024 // edit
#define MAX_DISPLAY_HEIGHT 600 // edit

#include "main.h"
#include <gdk/gdk.h>           // for gdk_cursor_new, GdkEventKey, gdk_scree...
#include <gio/gio.h>           // for g_application_run, G_APPLICATION, G_AP...
#include <glib-object.h>       // for g_object_unref, g_signal_connect
#include <gtk/gtk.h>           // for GtkWidget, gtk_events_pending, gtk_gri...
#include <pthread.h>           // for pthread_create, pthread_t
#include <stdbool.h>           // for bool
#include <stdio.h>             // for perror
#include <string.h>            // for strcpy, strlen
#include <unistd.h>            // for _exit, usleep, getcwd
#include "audio.h"             // for audio_get_cards
#include "band.h"              // for canTransmit
#include "css.h"               // for load_css
#include "discovered.h"        // for NEW_PROTOCOL, ORIGINAL_PROTOCOL
#include "ext.h"               // for ext_discovery, ext_vfo_update
#include "gdk/gdkkeysyms.h"    // for GDK_KEY_d, GDK_KEY_space, GDK_KEY_u
#include "gobject/gclosure.h"  // for G_CALLBACK
#include "gpio.h"              // for gpio_close
#include "log.h"               // for log_debug, log_warn
#include "new_protocol.h"      // for setMox, getMox, getTune, new_protocol_...
#include "old_protocol.h"      // for old_protocol_stop
#include "radio.h"             // for radioSaveState, radio, step, protocol
#include "transmitter.h"       // for transmitter_set_out_of_band
#include "vfo.h"               // for vfo_move
#include "wdsp.h"              // for WDSPwisdom, wisdom_get_status

struct utsname unameData;

gint display_width;
gint display_height;
gint full_screen = 1;

static GdkCursor *cursor_arrow;
static GdkCursor *cursor_watch;

GtkWidget *top_window;
GtkWidget *grid;

static GtkWidget *status;

void status_text(char *text) {
  gtk_label_set_text(GTK_LABEL(status), text);
  usleep(100000);
  while (gtk_events_pending())
    gtk_main_iteration();
}

static pthread_t wisdom_thread_id;
static int wisdom_running = 0;

static void *wisdom_thread(void *arg) {
  WDSPwisdom((char *)arg);
  wisdom_running = 0;
  return NULL;
}

//
// handler for key press events.
// SpaceBar presses toggle MOX, everything else downstream
// code to switch mox copied from mox_cb() in toolbar.c,
// but added the correct return values.
//
bool keypress_cb(GtkWidget *widget, GdkEventKey *event, gpointer data) {

  if (radio != NULL) {
    if (event->keyval == GDK_KEY_space) {

      if (getTune() == 1) {
        setTune(0);
      }
      if (getMox() == 1) {
        setMox(0);
      } else if (canTransmit() || tx_out_of_band) {
        setMox(1);
      } else {
        transmitter_set_out_of_band(transmitter);
      }
      g_idle_add(ext_vfo_update, NULL);
      return TRUE;
    }
    if (event->keyval == GDK_KEY_d) {
      vfo_move(step, TRUE);
      return TRUE;
    }
    if (event->keyval == GDK_KEY_u) {
      vfo_move(-step, TRUE);
      return TRUE;
    }
  }

  return FALSE;
}

gboolean main_delete(GtkWidget *widget) {
  if (radio != NULL) {
#ifdef GPIO
    gpio_close();
#endif
#ifdef CLIENT_SERVER
    if (!radio_is_remote) {
#endif
      switch (protocol) {
      case ORIGINAL_PROTOCOL:
        old_protocol_stop();
        break;
      case NEW_PROTOCOL:
        new_protocol_stop();
        break;
      }
#ifdef CLIENT_SERVER
    }
#endif
    radioSaveState();
  }
  _exit(0);
}

static int init(void *data) {
  char wisdom_directory[1024];

  audio_get_cards();

  // wait for get_cards to complete
  // g_mutex_lock(&audio_mutex);
  // g_mutex_unlock(&audio_mutex);

  cursor_arrow = gdk_cursor_new(GDK_ARROW);
  cursor_watch = gdk_cursor_new(GDK_WATCH);

  gdk_window_set_cursor(gtk_widget_get_window(top_window), cursor_watch);

  //
  // Let WDSP (via FFTW) check for wisdom file in current dir
  // If there is one, the "wisdom thread" takes no time
  // Depending on the WDSP version, the file is wdspWisdom or wdspWisdom00.
  // sem_trywait() is not elegant, replaced this with wisdom_running variable.
  //
  char *c = getcwd(wisdom_directory, sizeof(wisdom_directory));
  if (c == NULL) {
      perror("getcwd");
  }
  strcpy(&wisdom_directory[strlen(wisdom_directory)], "/");
  g_info("Securing wisdom file in directory: %s\n", wisdom_directory);
  status_text("Checking FFTW Wisdom file ...");
  wisdom_running = 1;
  pthread_create(&wisdom_thread_id, NULL, wisdom_thread, wisdom_directory);
  while (wisdom_running) {
    // wait for the wisdom thread to complete, meanwhile
    // handling any GTK events.
    usleep(100000); // 100ms
    while (gtk_events_pending()) {
      gtk_main_iteration();
    }
    status_text(wisdom_get_status());
  }

  g_idle_add(ext_discovery, NULL);
  return 0;
}

static void activate_pihpsdr(GtkApplication *app, gpointer data) {
  load_css();

  GdkScreen *screen = gdk_screen_get_default();
  if (screen == NULL) {
      g_info("no default screen!\n");
      _exit(0);
  }

  display_width = gdk_screen_get_width(screen);
  display_height = gdk_screen_get_height(screen);

  log_debug("width=%d height=%d", display_width, display_height);

  // Go to "window" mode if there is enough space on the screen.
  // Do not forget extra space needed for window top bars, screen bars etc.

  if (display_width > (MAX_DISPLAY_WIDTH + 10) &&
      display_height > (MAX_DISPLAY_HEIGHT + 30)) {
    display_width = MAX_DISPLAY_WIDTH;
    display_height = MAX_DISPLAY_HEIGHT;
    // full_screen=0;//edit (comment for full screen)
  } else {
    //
    // Some RaspPi variants report slightly too large screen sizes
    // on a 7-inch screen, e.g. 848*480 while the physical resolution is 800*480
    // Therefore, as a work-around, limit window size to 800*480
    //
    if (display_width > MAX_DISPLAY_WIDTH) {
      display_width = MAX_DISPLAY_WIDTH;
    }
    if (display_height > MAX_DISPLAY_HEIGHT) {
      display_height = MAX_DISPLAY_HEIGHT;
    }
    full_screen = 1;
  }

  log_debug("display_width=%d display_height=%d", display_width,
	    display_height);

  top_window = gtk_application_window_new(app);
  if (full_screen) {
    gtk_window_fullscreen(GTK_WINDOW(top_window));
  }
  gtk_widget_set_size_request(top_window, display_width, display_height);
  gtk_window_set_title(GTK_WINDOW(top_window), "Verdure SDR V1.2");
  gtk_window_set_position(GTK_WINDOW(top_window), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_resizable(GTK_WINDOW(top_window), TRUE); // edit
  GError *error;
  if (!gtk_window_set_icon_from_file(GTK_WINDOW(top_window), "hpsdr.png",
                                     &error)) {
      log_warn("failed to set icon for top_window");
      if (error != NULL) {
	  log_warn("%s", error->message);
      }
  }
  g_signal_connect(top_window, "delete-event", G_CALLBACK(main_delete), NULL);

  //
  // We want to use the space-bar as an alternative to go to TX
  //
  gtk_widget_add_events(top_window, GDK_KEY_PRESS_MASK);
  g_signal_connect(top_window, "key_press_event", G_CALLBACK(keypress_cb),
                   NULL);

  grid = gtk_grid_new();
  gtk_widget_set_size_request(grid, display_width, display_height);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), FALSE);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
  gtk_container_add(GTK_CONTAINER(top_window), grid);

  GtkWidget *image = gtk_image_new_from_file("hpsdr.png");
  gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 4);

  status = gtk_label_new("");
  gtk_label_set_justify(GTK_LABEL(status), GTK_JUSTIFY_LEFT);
  gtk_widget_show(status);
  gtk_grid_attach(GTK_GRID(grid), status, 1, 3, 1, 1);
  gtk_widget_hide(top_window);

  g_idle_add(init, NULL);
}

int main(int argc, char **argv) {
  GtkApplication *pihpsdr;
  int status;

  pihpsdr = gtk_application_new("org.rkrishnan.pihpsdr", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(pihpsdr, "activate", G_CALLBACK(activate_pihpsdr), NULL);
  status = g_application_run(G_APPLICATION(pihpsdr), argc, argv);
  g_object_unref(pihpsdr);
  return status;
}
