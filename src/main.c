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

int main (int argc, char **argv)
{

//  if (g_thread_supported ())
//    g_thread_init (NULL);
    
  gdk_threads_init ();
  gdk_threads_enter ();
  
  gtk_init (&argc, &argv);
  NimDialog *dialog;
  gint dialog_type = 0;

  if (argc > 1) 
    dialog_type = (gint) g_ascii_strtoll (argv [1], NULL, 10);
    
  dialog = nim_dialog_new (NULL, dialog_type);
  nim_dialog_run (dialog);

  gdk_threads_leave ();
  return 0;
}
