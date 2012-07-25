/*
 * nimresizedialog.c
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

#include "nimresizedialog.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimResizeDialogPrivate
{
  gint mode;
  gint width;
  gint height;
  GList *filelist;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
  RESIZE_WIDTH,
  RESIZE_HEIGHT,
  RESIZE_BOTH,
  RESIZE_NO_ASPECT
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimResizeDialog, nim_resize_dialog, GTK_TYPE_DIALOG)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void width_value_changed (GtkSpinButton *spin, NimResizeDialog *this);
static void height_value_changed (GtkSpinButton *spin, NimResizeDialog *this);
static void combo_value_changed (GtkComboBox *combo, NimResizeDialog *this);
static void response_cb (NimResizeDialog *this, gint response_id);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_resize_dialog_class_init (NimResizeDialogClass *klass)
{
  
  g_type_class_add_private ((gpointer)klass, sizeof (NimResizeDialogPrivate));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_resize_dialog_init (NimResizeDialog *this)
{
  GtkWidget *table;
  GtkWidget *align;
  GtkWidget *frame;
  GtkWidget *combo;
  GtkWidget *spin_width;
  GtkWidget *spin_height;
  GtkAdjustment *adj_width;
  GtkAdjustment *adj_height;
  GtkWidget *align_width;
  GtkWidget *align_height;
  GtkWidget *label_mode;
  GtkWidget *label_width;
  GtkWidget *label_height;
  GtkWidget *label_title;
  GtkWidget *content_area;
  GtkWidget *mainbox;
  NimResizeDialogPrivate *priv;
  
  this->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, NIM_TYPE_RESIZE_DIALOG, NimResizeDialogPrivate);
  priv = this->priv;

  combo = gtk_combo_box_text_new ();
  frame = gtk_frame_new (NULL);
  align = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  align_width = gtk_alignment_new (1.0f, 0.5f, 0.0f, 0.0f);
  align_height = gtk_alignment_new (1.0f, 0.5f, 0.0f, 0.0f);
  adj_width = gtk_adjustment_new ((gdouble) priv->width, 0, 10000, 1, 10, 0);
  adj_height = gtk_adjustment_new ((gdouble) priv->height, 0, 10000, 1, 10, 0);
  spin_width = gtk_spin_button_new (adj_width, 0, 0);
  spin_height = gtk_spin_button_new (adj_height, 0, 0);
  label_mode = gtk_label_new ("Resize mode:");
  label_width = gtk_label_new ("Width:");
  label_height = gtk_label_new ("Height:");
  label_title = gtk_label_new (NULL);
  mainbox = gtk_grid_new ();
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (this));

  priv->width = nim_config_get_int (NIM_CFG_GRP_RESIZE, NIM_CFG_WIDTH, 0);
  priv->height = nim_config_get_int (NIM_CFG_GRP_RESIZE, NIM_CFG_HEIGHT, 0);
  priv->mode = nim_config_get_int (NIM_CFG_GRP_RESIZE, NIM_CFG_MODE, RESIZE_BOTH);

  gtk_label_set_markup (GTK_LABEL (label_title), "<b>Resize</b>");
  gtk_misc_set_alignment (GTK_MISC (label_mode), 0.0f, 0.5f);
  gtk_misc_set_alignment (GTK_MISC (label_width), 0.0f, 0.5f);
  gtk_misc_set_alignment (GTK_MISC (label_height), 0.0f, 0.5f);
  gtk_alignment_set_padding (GTK_ALIGNMENT (align), 5, 5, 30, 5);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

  gtk_container_add (GTK_CONTAINER (frame), align);
  gtk_container_add (GTK_CONTAINER (align), mainbox);
  gtk_box_pack_start (GTK_BOX (content_area), frame, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (align_width), spin_width);
  gtk_container_add (GTK_CONTAINER (align_height), spin_height);

  gtk_grid_attach (GTK_GRID (mainbox), label_mode, 0, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (mainbox), label_width, 0, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (mainbox), label_height, 0, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (mainbox), combo, 1, 0, 1, 1);
  gtk_grid_attach (GTK_GRID (mainbox), align_width, 1, 1, 1, 1);
  gtk_grid_attach (GTK_GRID (mainbox), align_height, 1, 2, 1, 1);

  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), "Width");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), "Height");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), "Width and height");
  gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (combo), "Specified size only");
  gtk_combo_set_active (GTK_COMBO_BOX (combo), priv->mode);

  g_signal_connect (spin_width, "value-changed", G_CALLBACK (width_value_changed), this);
  g_signal_connect (spin_height, "value-changed", G_CALLBACK (height_value_changed), this);
  g_signal_connect (combo, "changed", G_CALLBACK (combo_value_changed), this);
  g_signal_connect (this, "response", G_CALLBACK (response_cb), NULL);

  gtk_dialog_add_button (GTK_DIALOG (this), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (this), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GtkWidget *nim_resize_dialog_new (GtkWindow *parent_window, GList *filelist)
{
  GObject *obj;
  GtkWidget *widget;

  obj = g_object_new (NIM_TYPE_RESIZE_DIALOG, NULL);
  widget = GTK_WIDGET (obj);

  if (parent_window)
    gtk_window_set_transient_for (GTK_WINDOW (widget), parent_window);

  NIM_RESIZE_DIALOG (obj)->filelist = filelist;
  
  return widget;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void width_value_changed (GtkSpinButton *spin, NimResizeDialog *this)
{
  this->priv->width = gtk_spin_button_get_value_as_int (spin);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void height_value_changed (GtkSpinButton *spin, NimResizeDialog *this)
{
  this->priv->height = gtk_spin_button_get_value_as_int (spin);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void combo_value_changed (GtkComboBox *combo, NimResizeDialog *this)
{
  this->priv->mode = gtk_combo_box_get_active (combo);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void response_cb (NimResizeDialog *this, gint response_id)
{
  if (response_id == GTK_RESPONSE_APPLY)
  {
    nim_config_set_int (NIM_CFG_GRP_RESIZE, NIM_CFG_MODE, this->priv->mode);
    nim_config_set_int (NIM_CFG_GRP_RESIZE, NIM_CFG_WIDTH, this->priv->width);
    nim_config_set_int (NIM_CFG_GRP_RESIZE, NIM_CFG_HEIGHT, this->priv->height);
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
