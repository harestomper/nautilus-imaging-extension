/*
 * nimmaindialog.c
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

#include "nimmaindialog.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimMainDialogPrivate
{
  GtkWidget *progress;
  GtkWidget *progress_label;
  GtkWidget *apply;
  GtkWidget *prefs;
  GtkWidget *combo;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimMainDialog, nim_main_dialog, GTK_TYPE_DIALOG)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_main_dialog_class_init (NimMainDialogClass *klass)
{
  
  g_type_class_add_private ((gpointer)klass, sizeof (NimMainDialogPrivate));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_main_dialog_init (NimMainDialog *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, NIM_TYPE_MAIN_DIALOG, NimMainDialogPrivate);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GtkWidget *nim_main_dialog_new (void)
{
  NimMainDialog *this;
  NimMainDialogPrivate *priv;
  GObject *obj;
  GtkWidget *content_area;
  GtkWidget *funcbox;
  GtkWidget *retbox;
  GtkWidget *savebox;
  GtkWidget *marker;
  GtkWidget *folder;
  GtkWidget *alignment;
  GtkSizeGroup *group;
  GtkWidget *frame;
  GtkWidget *framealign;
  GtkWidget *checkbutton;
  GtkWidget *label;
  GtkWidget *mainbox;
  GtkListStore *store;
  GtkCellRendererText *cell;
  GList *radiogroup = NULL;

  obj = g_object_new (NIM_TYPE_MAIN_DIALOG, NULL);
  this = NIM_MAIN_DIALOG (obj);
  priv = this->priv;
  group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  mainbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  /* Frame of selector */
  priv->combo = gtk_combo_box_text_new ();
  priv->prefs = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);
  frame = gtk_frame_new (NULL);
  framealign = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  label = gtk_label_new (NULL);
  funcbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);

  gtk_label_set_markup (GTK_LABEL (label), "<b>Functions</b>");
  gtk_alignment_get_padding (GTK_ALIGNMENT (framealin), 0, 0, 30, 5);
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_container_add (GTK_CONTAINER (frame), framealign);
  gtk_container_add (GTK_CONTAINER (framealign), funcbox);
  gtk_box_pack_start (GTK_BOX (funcbox), priv->combo, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (funcbox), priv->prefs, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (mainbox), funcbox, FALSE, FALSE, 0);
  gtk_size_group_add_widget (group, priv->combo);

  /* Frame of options to return */

  

  gtk_dialog_add_button (GTK_DIALOG (this), GTK_RESPONSE_CLOSE, GTK_STOCK_CLOSE);
  gtk_dialog_add_button (GTK_DIALOG (this), GTK_RESPONSE_APPLY, GTK_STOCK_APPLY);
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
