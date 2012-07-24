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
#include <string.h>
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define RADIO_DATA_KEY "main-dialog-radio-data"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimMainDialogPrivate
{
  GtkWidget *progress;
  GtkWidget *progress_label;

  GtkWidget *prefs;         /* Preferences */
  GtkWidget *combo;        /* functions */
  GtkWidget *ret_to_here; /* Return to here */
  GtkWidget *select_files; /* Select the modified files */
  GtkWidget *save_overwrite; /* Overwrite original files */
  GtkWidget *save_with_marker;  /* Add marker to original name */
  GtkWidget *save_to_folder;    /* Save to other selected folder */
  GtkWidget *save_marker;       /* GtkEntry */
  GtkWidget *save_folder;       /* GtkFileChooserButton */

  gint active_function;
  gint active_save_mode;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimMainDialog, nim_main_dialog, GTK_TYPE_DIALOG)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void radio_save_toggled (GtkWidget *widget, NimMainDialog *this);
static void function_changed_cb (GtkComboBox *combo, NimMainDialog *this);
static void check_button_toggled (GtkToggleButton *button, NimMainDialog *this);
static void response_cb (NimMainDialog *this, gint response_id);
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
typedef struct {
  gint value;
  GtkWidget *widget;
} RadioData;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void radio_data_free (gpointer uptr)
{
  g_slice_free (RadioData, uptr);
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
  GtkWidget *alignment;
  GtkSizeGroup *group;
  GtkWidget *frame;
  GtkWidget *framealign;
  GtkWidget *radiobutton;
  GtkWidget *label;
  GtkWidget *mainbox;
  gchar *text, *foldername;
  gboolean active;
  RadioData *rd;

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
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (this));

  gtk_label_set_markup (GTK_LABEL (label), "<b>Functions</b>");
  gtk_alignment_set_padding (GTK_ALIGNMENT (framealign), 5, 5, 30, 5);
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (frame), framealign);
  gtk_container_add (GTK_CONTAINER (framealign), funcbox);
  gtk_box_pack_start (GTK_BOX (funcbox), priv->combo, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (funcbox), priv->prefs, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (mainbox), frame, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (content_area), mainbox, FALSE, FALSE, 0);

  /* Frame of options to return */
  frame = gtk_frame_new (NULL);
  label = gtk_label_new (NULL);
  framealign = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  retbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  priv->ret_to_here = gtk_check_button_new_with_label ("Return to here");
  priv->select_files = gtk_check_button_new_with_label ("Select the modified files");

  gtk_label_set_markup (GTK_LABEL (label), "<b>Options to return</b>");
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_alignment_set_padding (GTK_ALIGNMENT (framealign), 5, 5, 30, 5);
  gtk_container_add (GTK_CONTAINER (frame), framealign);
  gtk_container_add (GTK_CONTAINER (framealign), retbox);
  gtk_box_pack_start (GTK_BOX (retbox), priv->ret_to_here, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (retbox), priv->select_files, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (mainbox), frame, FALSE, FALSE, 0);
  
  /* Frame "options to save" */
  frame = gtk_frame_new (NULL);
  label = gtk_label_new (NULL);
  framealign = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  savebox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

  gtk_label_set_markup (GTK_LABEL (label), "<b>Options to save</b>");
  gtk_frame_set_label_widget (GTK_FRAME (frame), label);
  gtk_alignment_set_padding (GTK_ALIGNMENT (framealign), 5, 5, 30, 5);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (frame), framealign);
  gtk_container_add (GTK_CONTAINER (framealign), savebox);
  gtk_box_pack_start (GTK_BOX (mainbox), frame, FALSE, FALSE, 0);

  priv->save_marker = gtk_entry_new ();
  priv->save_folder = gtk_file_chooser_button_new (NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  priv->save_overwrite = gtk_radio_button_new_with_label (NULL, "Overwrite original files");
  priv->save_with_marker = gtk_radio_button_new_with_label_from_widget (
                                                GTK_RADIO_BUTTON (priv->save_overwrite),
                                                "Add marker to originsl name");
  priv->save_to_folder = gtk_radio_button_new_with_label_from_widget (
                                                GTK_RADIO_BUTTON (priv->save_overwrite),
                                                "Save to other selected folder");
                                                
  gtk_box_pack_start (GTK_BOX (savebox), priv->save_overwrite, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (savebox), priv->save_with_marker, FALSE, FALSE, 0);

  alignment = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 30, 5);
  gtk_container_add (GTK_CONTAINER (alignment), priv->save_marker);
  gtk_box_pack_start (GTK_BOX (savebox), alignment, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (priv->save_marker, FALSE);
  
  alignment = gtk_alignment_new (0.0f, 0.0f, 0.0f, 0.0f);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 30, 5);
  gtk_container_add (GTK_CONTAINER (alignment), priv->save_folder);
  gtk_box_pack_start (GTK_BOX (savebox), priv->save_to_folder, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (savebox), alignment, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (priv->save_folder, FALSE);

  gtk_size_group_add_widget (group, priv->save_marker);
  gtk_size_group_add_widget (group, priv->save_folder);
  gtk_size_group_add_widget (group, priv->save_overwrite);
  gtk_size_group_add_widget (group, priv->combo);

  rd = g_slice_new0 (RadioData);
  rd->value = 0;
  rd->widget = NULL;
  g_object_set_data_full (G_OBJECT (priv->save_overwrite), RADIO_DATA_KEY, rd, radio_data_free);

  rd = g_slice_new0 (RadioData);
  rd->value = 1;
  rd->widget = priv->save_marker;
  g_object_set_data_full (G_OBJECT (priv->save_with_marker), RADIO_DATA_KEY, rd, radio_data_free);

  rd = g_slice_new0 (RadioData);
  rd->value = 2;
  rd->widget = priv->save_folder;
  g_object_set_data_full (G_OBJECT (priv->save_to_folder), RADIO_DATA_KEY, rd, radio_data_free);

  g_signal_connect (priv->save_overwrite, "toggled", G_CALLBACK (radio_save_toggled), this);
  g_signal_connect (priv->save_with_marker, "toggled", G_CALLBACK (radio_save_toggled), this);
  g_signal_connect (priv->save_to_folder, "toggled", G_CALLBACK (radio_save_toggled), this);
  g_signal_connect (priv->ret_to_here, "toggled", G_CALLBACK (check_button_toggled), this);

  text = nim_config_get_string (NIM_CFG_GRP_COMMON, NIM_CFG_ADD_SUFFIX, "modif");
  foldername = nim_config_get_string (NIM_CFG_GRP_COMMON, NIM_CFG_LAST_FOLDER, "~");
  priv->active_save_mode = nim_config_get_int (NIM_CFG_GRP_COMMON, NIM_CFG_ACTIVE_SAVE, 1);
  priv->active_function = nim_config_get_int (NIM_CFG_GRP_COMMON, NIM_CFG_MODE, NIM_FUNC_RESIZE);

  gtk_entry_set_text (GTK_ENTRY (priv->save_marker), text);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (priv->save_folder), foldername);

  switch (priv->active_save_mode) {
    case 0:
      radiobutton = priv->save_overwrite; break;
    case 2:
      radiobutton = priv->save_to_folder; break;
    case 1:
    default:
      radiobutton = priv->save_with_marker; break;
  }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), TRUE);

  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Resize");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Rotate");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Convert");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Draw watermarks");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Create thumbnails");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Rounding the corners");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Add shadow");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (priv->combo), "Group rename");
  g_signal_connect (priv->combo, "changed", G_CALLBACK (function_changed_cb), this);
  gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo), priv->active_function);

  active = nim_config_get_bool (NIM_CFG_GRP_COMMON, NIM_CFG_RET_TO_HERE, TRUE);
  gtk_widget_set_sensitive (priv->select_files, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->ret_to_here), active);

  active = nim_config_get_bool (NIM_CFG_GRP_COMMON, NIM_CFG_SELECT_FILES, FALSE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->select_files), active);
  
  gtk_dialog_add_button (GTK_DIALOG (this), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  gtk_dialog_add_button (GTK_DIALOG (this), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);
  g_signal_connect (this, "response", G_CALLBACK (response_cb), NULL);
  gtk_widget_show_all (mainbox);
  

  g_free (text);
  g_free (foldername);

  return GTK_WIDGET (obj);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void radio_save_toggled (GtkWidget *widget, NimMainDialog *this)
{
  RadioData *rd;
  GtkToggleButton *button;
  gboolean active;
  
  button = GTK_TOGGLE_BUTTON (widget);
  rd = g_object_get_data (G_OBJECT (widget), RADIO_DATA_KEY);
  active = gtk_toggle_button_get_active (button);

  if (rd->widget)
    gtk_widget_set_sensitive (rd->widget, active);

  if (active)
    this->priv->active_save_mode = rd->value;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void function_changed_cb (GtkComboBox *combo, NimMainDialog *this)
{
  this->priv->active_function = gtk_combo_box_get_active (combo);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void response_cb (NimMainDialog *this, gint response_id)
{
  if (response_id == GTK_RESPONSE_APPLY) {
    gchar *dirname;
    const gchar *marker;
    gboolean active;
    NimMainDialogPrivate *priv;

    priv = this->priv;
    marker = gtk_entry_get_text (GTK_ENTRY (priv->save_marker));
    dirname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (priv->save_folder));

    marker = marker && strlen (marker) > 0 ? marker : "modif";
    nim_config_set_string (NIM_CFG_GRP_COMMON, NIM_CFG_LAST_FOLDER, dirname);
    nim_config_set_int (NIM_CFG_GRP_COMMON, NIM_CFG_MODE, priv->active_function);
    nim_config_set_int (NIM_CFG_GRP_COMMON, NIM_CFG_ACTIVE_SAVE, priv->active_save_mode);
    nim_config_set_string (NIM_CFG_GRP_COMMON, NIM_CFG_ADD_SUFFIX, marker);

    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->ret_to_here));
    nim_config_set_bool (NIM_CFG_GRP_COMMON, NIM_CFG_RET_TO_HERE, active);

    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->select_files));
    nim_config_set_bool (NIM_CFG_GRP_COMMON, NIM_CFG_SELECT_FILES, active);
    
    nim_config_sync ();

    g_free (dirname);
  }
   
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void check_button_toggled (GtkToggleButton *button, NimMainDialog *this)
{
  gboolean active;
  
  active = gtk_toggle_button_get_active (button);
  gtk_widget_set_sensitive (this->priv->select_files, active);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
