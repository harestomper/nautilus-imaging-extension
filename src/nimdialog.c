/*
 * nimdialog.c
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

#include <gtk/gtk.h>
#include <math.h>

#include "nimdialog.h"
#include "nimimaging.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define NIM_CHILD_WIDGET  "nim-dialog-child-widget"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimDialogPrivate
{
  GtkBuilder *builder;
  gint        dialog_type;
  MagickWand *preview_wand;
  guint       source;
  void (*source_func) (NimDialog *this);
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_finalize (GObject *object);
static void fake_callback_function (NimDialog *this);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimDialog, nim_dialog, G_TYPE_OBJECT)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_class_init (NimDialogClass *klass)
{
  GObjectClass *g_object_class;

  g_object_class = G_OBJECT_CLASS (klass);

  g_object_class->finalize = nim_dialog_finalize;

  g_type_class_add_private ((gpointer) klass, sizeof (NimDialogPrivate));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_finalize (GObject *object)
{
  NimDialog *self;

  g_return_if_fail (NIM_IS_DIALOG (object));

  self = NIM_DIALOG (object);

  if (IsMagickWand (self->priv->preview_wand))
    self->priv->preview_wand = DestroyMagickWand (self->priv->preview_wand);

  G_OBJECT_CLASS (nim_dialog_parent_class)->finalize (object);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_init (NimDialog *this)
{
  this->priv = G_TYPE_INSTANCE_GET_PRIVATE (this, NIM_TYPE_DIALOG, NimDialogPrivate);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_combo_init (NimDialog *this, const gchar *object_name, GCallback callback, gint active_value)
{
  NimDialogPrivate *priv;
  GObject *object;

  priv = this->priv;
  object = gtk_builder_get_object (priv->builder, object_name);

  if (object) {
    g_signal_connect (object, "changed", callback, this);
    gtk_combo_box_set_active (GTK_COMBO_BOX (object), active_value);
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_rotate_angle_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  gint value;
  GObject *object;
  NimDialogPrivate *priv;

  priv = this->priv;
  value = gtk_combo_box_get_active (combo);
  object = gtk_builder_get_object (priv->builder, "rotate_custom_box");

  if (object)
    gtk_widget_set_sensitive (GTK_WIDGET (object), value < 3 ? FALSE : TRUE);
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_resize_mode_combo_changed (GtkComboBox *combo, NimDialog *this)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_conv_type_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  gint active;
  gchar *boxname;
  GtkWidget *widget;

  priv = this->priv;
  active = gtk_combo_box_get_active (combo);
  active = active < 0 ? 0 : active;
  boxname = g_strdup_printf ("conv_pref%i", active);
  widget = (GtkWidget *) gtk_builder_get_object (priv->builder, boxname);
  g_free (boxname);

  if (GTK_IS_WIDGET (widget)) {
    gint value = 0;
    GtkWidget *temp_widget = NULL;

    while (TRUE) {
      boxname = g_strdup_printf ("conv_pref%i", value);
      temp_widget = (GtkWidget *) gtk_builder_get_object (priv->builder, boxname);
      g_free (boxname);

      if (GTK_IS_WIDGET (temp_widget)) {
        gtk_widget_set_no_show_all (temp_widget, TRUE);
        gtk_widget_set_visible (temp_widget, FALSE);
        value++;
      } else {
        break;
      }
    }

    gtk_widget_set_no_show_all (widget, FALSE);
    gtk_widget_set_visible (widget, TRUE);
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_water_method_combo_changed (GtkComboBox *combo, NimDialog *this)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_thumb_mode_combo_changed (GtkComboBox *combo, NimDialog *this)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_effect_type_combo_changed (GtkComboBox *combo, NimDialog *this)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void save_options_radio_toggled (GtkToggleButton *button, NimDialog *this)
{
  gboolean active;
  GtkWidget *child_box;

  active = gtk_toggle_button_get_active (button);
  child_box = (GtkWidget *) g_object_get_data (G_OBJECT (button), NIM_CHILD_WIDGET);
  
  if (GTK_IS_WIDGET (child_box)) 
    gtk_widget_set_sensitive (child_box, active);
    
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_choose_folder_activated (GtkWidget *widget, NimDialog *this)
{
  NimDialogPrivate *priv;
  GtkWidget *fcdialog;
  GtkWidget *dialog;
  GtkWidget *entry;
  gint response;
  
  priv = this->priv;
  dialog = (GtkWidget *) gtk_builder_get_object (priv->builder, "dialog1");
  fcdialog = gtk_file_chooser_dialog_new (NULL,
                                          GTK_WINDOW (dialog),
                                          GTK_FILE_CHOOSER_ACTION_OPEN
                                          | GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                          GTK_STOCK_CANCEL,
                                          GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK,
                                          NULL);
  response = gtk_dialog_run (GTK_DIALOG (fcdialog));
  gtk_widget_hide (fcdialog);

  if (response == GTK_RESPONSE_OK) {
    gchar *foldername;
    entry = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_choose_entry");
    foldername = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fcdialog));
    gtk_entry_set_text (GTK_ENTRY (entry), foldername);
    g_free (foldername);
  }
    
  gtk_widget_destroy (fcdialog);
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_water_file_update_preview (GtkFileChooser *chooser, NimDialog *this)
{
  GtkWidget *preview;
  gchar *filename;

  preview = gtk_file_chooser_get_preview_widget (chooser);
  filename = gtk_file_chooser_get_preview_filename (chooser);

  if (filename) {
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 150, 150, &error);

    if (!error) {
      gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
      gtk_widget_show_all (preview);
    } else {
      g_error_free (error);
    }

    if (pixbuf)
      g_object_unref (G_OBJECT (pixbuf));

    g_free (filename);
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_water_file_button_activated (GtkWidget *widget, NimDialog *this)
{
  NimDialogPrivate *priv;
  GtkWidget *entry;
  GtkWidget *fcdialog;
  GtkWidget *dialog;
  GtkFileFilter *filter;
  GtkWidget *preview;
  gint response;

  priv = this->priv;
  dialog = (GtkWidget *) gtk_builder_get_object (priv->builder, "dialog1");
  fcdialog = gtk_file_chooser_dialog_new (NULL,
                                          GTK_WINDOW (dialog),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL,
                                          GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK,
                                          NULL);
  preview = gtk_image_new ();
  filter = gtk_file_filter_new ();
  entry = (GtkWidget *) gtk_builder_get_object (priv->builder, "water_file_entry");

  gtk_file_filter_add_pixbuf_formats (filter);
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fcdialog), filter);
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (fcdialog), preview);
  g_signal_connect (fcdialog, "update-preview", G_CALLBACK (nim_dialog_water_file_update_preview), this);

  response = gtk_dialog_run (GTK_DIALOG (fcdialog));
  gtk_widget_hide (fcdialog);

  if (response == GTK_RESPONSE_OK) {
    gchar *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fcdialog));

    if (filename) {
      gtk_entry_set_text (GTK_ENTRY (entry), filename);
      g_free (filename);
    }
  }

  gtk_widget_destroy (fcdialog);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_radio_init (NimDialog *this)
{
  NimDialogPrivate *priv;
  GtkWidget *child_widget;
  GtkWidget *button;

  priv = this->priv;

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_suffix_radio");
  child_widget = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_option_box1");
  g_object_set_data (G_OBJECT (button), NIM_CHILD_WIDGET, child_widget);
  g_signal_connect (button, "toggled", G_CALLBACK (save_options_radio_toggled), this);

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_choose_radio");
  child_widget = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_option_box2");
  g_object_set_data (G_OBJECT (button), NIM_CHILD_WIDGET, child_widget);
  g_signal_connect (button, "toggled", G_CALLBACK (save_options_radio_toggled), this);

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "radio_draw_text");
  child_widget = (GtkWidget *) gtk_builder_get_object (priv->builder, "textbox");
  g_object_set_data (G_OBJECT (button), NIM_CHILD_WIDGET, child_widget);
  g_signal_connect (button, "toggled", G_CALLBACK (save_options_radio_toggled), this);

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "radio_draw_picture");
  child_widget = (GtkWidget *) gtk_builder_get_object (priv->builder, "filebox");
  g_object_set_data (G_OBJECT (button), NIM_CHILD_WIDGET, child_widget);
  g_signal_connect (button, "toggled", G_CALLBACK (save_options_radio_toggled), this);

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "save_choose_button");
  g_signal_connect (button, "clicked", G_CALLBACK (nim_dialog_choose_folder_activated), this);

  button = (GtkWidget *) gtk_builder_get_object (priv->builder, "water_file_button");
  g_signal_connect (button, "clicked", G_CALLBACK (nim_dialog_water_file_button_activated), this);
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gdouble round_spin_get_value (NimDialog *this, const gchar *wname, gboolean stick, gdouble value)
{
  GtkSpinButton *spin;
  gdouble result;

  spin = GTK_SPIN_BUTTON (gtk_builder_get_object (this->priv->builder, wname));
  if (stick)
  {
    result = value;
    gtk_spin_button_set_value (spin, value);
  } else {
    result = gtk_spin_button_get_value (spin);
  }

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void  round_spin_changed_cb (GtkSpinButton *spin, NimDialog *this)
{
  NimDialogPrivate *priv;
  gdouble corners [NIM_CORNER_LAST] = {0};
  GtkWidget *preview;
  GtkToggleButton *button;
  GdkPixbuf *pixbuf;
  MagickWand *wand;
  gchar *filename;
  gdouble stick_value;
  gboolean active;

  priv = this->priv;
  filename = nim_imaging_get_path_to_test_image (0);
  preview = GTK_WIDGET (gtk_builder_get_object (priv->builder, "round_preview_image"));

  button = GTK_TOGGLE_BUTTON (gtk_builder_get_object (priv->builder, "round_stick_button"));
  active = gtk_toggle_button_get_active (button);

  if (active)
    stick_value = gtk_spin_button_get_value (spin);
  
  corners [NIM_CORNER_TL] = round_spin_get_value (this, "round_tl_spin", active, stick_value);
  corners [NIM_CORNER_TR] = round_spin_get_value (this, "round_tr_spin", active, stick_value);
  corners [NIM_CORNER_BL] = round_spin_get_value (this, "round_bl_spin", active, stick_value);
  corners [NIM_CORNER_BR] = round_spin_get_value (this, "round_br_spin", active, stick_value);

  if ((wand = nim_imaging_round_corners (filename, corners)) != NULL)
  {
    if ((pixbuf = nim_imaging_convert_wand_to_pixbuf (wand)) != NULL)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
      g_object_unref (G_OBJECT (pixbuf));
    }

    wand = DestroyMagickWand (wand);
  }

  g_free (filename);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_round_spins_init (NimDialog *this)
{
  GtkSpinButton *spin;
  NimDialogPrivate *priv;

  priv = this->priv;
  spin = GTK_SPIN_BUTTON (gtk_builder_get_object (priv->builder, "round_tl_spin"));
  g_signal_connect (spin, "value-changed", G_CALLBACK (round_spin_changed_cb), this);

  spin = GTK_SPIN_BUTTON (gtk_builder_get_object (priv->builder, "round_tr_spin"));
  g_signal_connect (spin, "value-changed", G_CALLBACK (round_spin_changed_cb), this);

  spin = GTK_SPIN_BUTTON (gtk_builder_get_object (priv->builder, "round_bl_spin"));
  g_signal_connect (spin, "value-changed", G_CALLBACK (round_spin_changed_cb), this);

  spin = GTK_SPIN_BUTTON (gtk_builder_get_object (priv->builder, "round_br_spin"));
  g_signal_connect (spin, "value-changed", G_CALLBACK (round_spin_changed_cb), this);

  round_spin_changed_cb (spin, this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean _effect_widget_value_changed (NimDialog *this)
{
  gchar *filename;
  NimDialogPrivate *priv;

  GtkWidget *background;
  GtkWidget *offx_spin;
  GtkWidget *offy_spin;
  GtkWidget *radius_spin;
  GtkWidget *sigma_spin;
  GtkWidget *effect_type;
  GtkWidget *image;
  GdkPixbuf *pixbuf;
  MagickWand *wand;
  gboolean enable_bg;
  gdouble offx, offy, radius, sigma, efftype;

  priv = this->priv;

  if (!IsMagickWand (priv->preview_wand)) {
    gboolean result;
    filename = nim_imaging_get_path_to_test_image (NIM_FIND_IMAGE);
    
    if (filename == NULL)
      return FALSE;

    priv->preview_wand = NewMagickWand ();
    result = MagickReadImage (priv->preview_wand, filename) == MagickTrue;
    g_free (filename);

    if (!result) {
      priv->preview_wand = DestroyMagickWand (priv->preview_wand);
      return FALSE;
    }
  }
    
  image = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_preview_image");
  background = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_enable_bg_button");
  offx_spin = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_offsetx_spin");
  offy_spin = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_offsety_spin");
  radius_spin = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_radius_spin");
  sigma_spin = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_sigma_spin");
  effect_type = (GtkWidget *) gtk_builder_get_object (priv->builder, "effect_type_combo");

  offx = gtk_spin_button_get_value (GTK_SPIN_BUTTON (offx_spin));
  offy = gtk_spin_button_get_value (GTK_SPIN_BUTTON (offy_spin));
  radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (radius_spin));
  sigma = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sigma_spin));
  efftype = gtk_combo_box_get_active (GTK_COMBO_BOX (effect_type));
  enable_bg = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (background));
  sigma = efftype == NIM_EFFECT_SHADOW ? ceil (radius / 2.0) : sigma;

  if (IsMagickWand (priv->preview_wand)) {
    wand = CloneMagickWand (priv->preview_wand);
    nim_imaging_effect_from_wand (&wand, efftype, offx, offy, radius, sigma, enable_bg);
  } else {
    return FALSE;
  }

  pixbuf = nim_imaging_convert_wand_to_pixbuf (wand);

  if (pixbuf) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
    g_object_unref (G_OBJECT (pixbuf));
  }

  if (IsMagickWand (wand))
    wand = DestroyMagickWand (wand);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void effect_widget_value_changed (GtkWidget *widget, NimDialog *this)
{
//  g_idle_add ((GSourceFunc) _effect_widget_value_changed, this);
  fake_callback_function (this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_effects_init (NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *background;
  GObject *offx_spin;
  GObject *offy_spin;
  GObject *radius_spin;
  GObject *sigma_spin;
  GObject *effect_type;

  priv = this->priv;
  priv->source_func = _effect_widget_value_changed;

  background = gtk_builder_get_object (priv->builder, "effect_enable_bg_button");
  offx_spin = gtk_builder_get_object (priv->builder, "effect_offsetx_spin");
  offy_spin = gtk_builder_get_object (priv->builder, "effect_offsety_spin");
  radius_spin = gtk_builder_get_object (priv->builder, "effect_radius_spin");
  sigma_spin = gtk_builder_get_object (priv->builder, "effect_sigma_spin");
  effect_type = gtk_builder_get_object (priv->builder, "effect_type_combo");
  
  g_signal_connect (background, "toggled", G_CALLBACK (effect_widget_value_changed), this);
  g_signal_connect (offx_spin, "value-changed", G_CALLBACK (effect_widget_value_changed), this);
  g_signal_connect (offy_spin, "value-changed", G_CALLBACK (effect_widget_value_changed), this);
  g_signal_connect (radius_spin, "value-changed", G_CALLBACK (effect_widget_value_changed), this);
  g_signal_connect (sigma_spin, "value-changed", G_CALLBACK (effect_widget_value_changed), this);
  g_signal_connect (effect_type, "changed", G_CALLBACK (effect_widget_value_changed), this);

  effect_widget_value_changed (NULL, this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
NimDialog* nim_dialog_new (GtkWindow* parent_window, gint dialog_type)
{
  NimDialog *this;
  NimDialogPrivate *priv;
  GObject *object;
  GtkWidget *widget;
  GtkWidget *dialog;
  GError *error = NULL;
  gchar *uifile;

  object = g_object_new (NIM_TYPE_DIALOG, NULL);
  this = NIM_DIALOG (object);
  priv = this->priv;
  priv->preview_wand = NULL;

  priv->dialog_type = dialog_type;
  priv->builder = gtk_builder_new ();
  uifile = nim_imaging_get_path_to_test_image (1);
  gtk_builder_add_from_file (priv->builder, uifile, &error);
  g_free (uifile);
//  gtk_builder_add_from_file (priv->builder, "../ui/common.ui", &error);

  if (error) {
    g_print ("ERROR: %i: %s\n", error->code, error->message);
    g_error_free (error);
    return this;
  }
  
  gchar *widget_name;

  widget_name = g_strdup_printf ("property%i", dialog_type);
  widget = (GtkWidget *) gtk_builder_get_object (priv->builder, widget_name);
  dialog = (GtkWidget *) gtk_builder_get_object (priv->builder, "dialog1");
  g_free (widget_name);

  if (widget) {
    gtk_widget_set_no_show_all (widget, FALSE);
    gtk_widget_show_all (widget);
  }

  if (dialog && GTK_IS_WINDOW (parent_window)) {
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  }

  nim_dialog_combo_init (this, "rotate_angle_combo", G_CALLBACK (nim_dialog_rotate_angle_combo_changed), 0);
  nim_dialog_combo_init (this, "resize_mode_combo", G_CALLBACK (nim_dialog_resize_mode_combo_changed), 0);
  nim_dialog_combo_init (this, "conv_type_combo", G_CALLBACK (nim_dialog_conv_type_combo_changed), 0);
  nim_dialog_combo_init (this, "water_method_combo", G_CALLBACK (nim_dialog_water_method_combo_changed), 0);
  nim_dialog_combo_init (this, "thumb_mode_combo", G_CALLBACK (nim_dialog_thumb_mode_combo_changed), 0);
  nim_dialog_combo_init (this, "effect_type_combo", G_CALLBACK (nim_dialog_effect_type_combo_changed), 0);

  nim_dialog_radio_init (this);
  nim_dialog_round_spins_init (this);
  nim_dialog_effects_init (this);
  
  return this;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint nim_dialog_run (NimDialog *this)
{
  NimDialogPrivate *priv;
  GtkWidget *dialog;
  gint response = GTK_RESPONSE_CANCEL;

  g_return_val_if_fail (NIM_IS_DIALOG (this), response);

  priv = this->priv;
  dialog = (GtkWidget *) gtk_builder_get_object (priv->builder, "dialog1");

  if (dialog) {
    gtk_widget_show_all (dialog);
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);
  }

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean fake_timeout_function (NimDialog *this)
{
  if (this->priv->source_func)
    (this->priv->source_func) (this);

  this->priv->source = 0;
  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void fake_callback_function (NimDialog *this)
{
  if (this->priv->source) {
    g_source_remove (this->priv->source);
    this->priv->source = 0;
  }

  if (this->priv->source_func)
    this->priv->source = g_timeout_add (800, (GSourceFunc) fake_timeout_function, this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
