/*
 * Без имени.c
 * 
 * Copyright 2012 Voldemar Khramtsov <harestomper@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

/*
gcc -Wall `pkg-config --cflags --libs gobject-2.0,gtk+-3.0,glib-2.0` -o dialog-test ./main.c ./nimdialog.c
*/

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include "nimdialog.h"
#include "nimimaging.h"

int main (int argc, char **argv)
{

//  if (g_thread_supported ())
//    g_thread_init (NULL);
    
  gdk_threads_init ();
  gdk_threads_enter ();
  MagickWandGenesis ();
  
  gtk_init (&argc, &argv);
  NimDialog *dialog;
  GKeyFile *config;
  MagickWand *wand;
  gchar *fontname;
  gchar *text;
  gint fontsize;
  gchar *foreground;
  gint dialog_type = 0;
  gchar *data;

  if (argc > 1) 
    dialog_type = (gint) g_ascii_strtoll (argv [1], NULL, 10);

  dialog = nim_dialog_new (NULL, dialog_type);

  if (nim_dialog_run (dialog) == GTK_RESPONSE_APPLY) {
    config = g_key_file_new ();
    data = nim_dialog_get_data (dialog, NULL);
    g_key_file_load_from_data (config, data, -1, 0, NULL);
    fontname = g_key_file_get_string (config, MARKER_GROUP, "water_font", NULL);
    fontsize = g_key_file_get_integer (config, MARKER_GROUP, "water_font:size", NULL);
    text = g_key_file_get_string (config, MARKER_GROUP, "water_entry", NULL);
    foreground = g_key_file_get_string (config, MARKER_GROUP, "water_font_color", NULL);
    wand = nim_imaging_draw_text_simple (text, fontname, fontsize, foreground, NULL, FALSE, 0);
    nim_imaging_effect_from_wand (&wand, NIM_EFFECT_SHADOW, 0, 0, 0.0, 6.0, 0.0, FALSE);
    MagickWriteImage (wand, "test/result-text.png");
    DestroyMagickWand (wand);
    g_print ("%s\n", data);
  }

  gdk_threads_leave ();
  MagickWandTerminus ();
  return 0;
}
