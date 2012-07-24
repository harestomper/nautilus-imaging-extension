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
gcc -Wall `pkg-config --cflags --libs glib-2.0,gobject-2.0,gtk+-3.0` -o test-maindialog ../src/nimmaindialog.c ../src/nimconfig.c test-maindialog.c
*/

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <stdio.h>
#include "nimconfig.h"
#include "nimmaindialog.h"

int main(int argc, char **argv)
{
  g_type_init ();
  gtk_init (&argc, &argv);

  GtkWidget *dialog;

  dialog = nim_main_dialog_new();
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_hide (dialog);
  gtk_widget_destroy (dialog);
  
  return 0;
}
