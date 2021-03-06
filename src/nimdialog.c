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
#include "nimfontchooser.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define NIM_CHILD_WIDGET  "nim-dialog-child-widget"
#define OBJECT_NAME   "nim-dialog-object-name"
#define OBJECT_GROUP  "nim-dialog-object-group"

#define DEFAULT_PREVIEW_WIDTH   200
#define DEFAULT_PREVIEW_HEIGHT  150
#define TIMEOUT_TO_CHANGE       500

#define DEFAULT_ROTATE_MODE           NIM_ROTATE_90
#define DEFAULT_ROTATE_ANGLE          90.0
#define DEFAULT_ROTATE_COLOR          "#000000ff"

#define DEFAULT_RESIZE_MODE           NIM_RESIZE_BOTH
#define DEFAULT_RESIZE_WIDTH          1024
#define DEFAULT_RESIZE_HEIGHT         800
#define DEFAULT_RESIZE_THUMB          TRUE
#define DEFAULT_RESIZE_FILTER         NIM_RESIZE_FILTER_LANCZOS
#define DEFAULT_RESIZE_FACTOR         1.0

#define DEFAULT_CONVERT_TYPE          NIM_CONVERT_TYPE_PNG
#define DEFAULT_CONVERT_QUALITY       90.0
#define DEFAULT_CONVERT_ASPEED        100
#define DEFAULT_CONVERT_COMBINE       TRUE

#define DEFAULT_ROUND_STICK           TRUE
#define DEFAULT_ROUND_TL              10
#define DEFAULT_ROUND_TR              10
#define DEFAULT_ROUND_BL              10
#define DEFAULT_ROUND_BR              10

#define DEFAULT_EFFECT_TYPE           NIM_EFFECT_NORMALIZE
#define DEFAULT_EFFECT_OFFX           8
#define DEFAULT_EFFECT_OFFY           8
#define DEFAULT_EFFECT_RADIUS         10.0
#define DEFAULT_EFFECT_SIGMA          6.0
#define DEFAULT_EFFECT_ANGLE          0.0
#define DEFAULT_EFFECT_BG             FALSE

#define DEFAULT_MARKER_TEXT_TYPE      TRUE
#define DEFAULT_MARKER_TEXT           "My image:)"
#define DEFAULT_MARKER_FONT           "Sans 72"
#define DEFAULT_MARKER_FONT_SIZE      72
#define DEFAULT_MARKER_FG             "#ffffffff"
#define DEFAULT_MARKER_OPACITY        80.0
#define DEFAULT_MARKER_METHOD         NIM_WATER_METHOD_ALIGN
#define DEFAULT_MARKER_PITCH          0.0
#define DEFAULT_MARKER_X              10.0
#define DEFAULT_MARKER_Y              10.0
#define DEFAULT_MARKER_ABS_X          10
#define DEFAULT_MARKER_ABS_Y          10
#define DEFAULT_MARKER_ADAPTIVE       FALSE
#define DEFAULT_MARKER_TEXT_EFFECT    NIM_WATER_TEXT_EFFECT_FLAT
#define DEFAULT_MARKER_FILE           FALSE

#define DEFAULT_COMMON_TYPE           NIM_SAVE_MARK
#define DEFAULT_COMMON_SUFFIX         "modified"
#define DEFAULT_COMMON_FOLDER         (g_get_user_special_dir (G_USER_DIRECTORY_PICTURES))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimDialogPrivate
{
  GtkBuilder *builder;
  gint        dialog_type;
  GKeyFile   *config;
  MagickWand *preview_wand;
  guint       source;
  void (*source_func) (NimDialog *this);
  gpointer    user_data;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_finalize (GObject *object);
static void fake_callback_function (NimDialog *this);
static GObject* nim_dialog_set_default (NimDialog *dialog,
                                        const gchar *group,
                                        const gchar *wname,
                                        GCallback callback);
static void nim_dialog_write_value (NimDialog *this, GObject *object);
static GObject* nim_dialog_write_value_by_name (NimDialog *this, const gchar *object_name);
static void nim_dialog_widget_set_visible (GtkWidget *widget, gboolean setting);

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
static void nim_dialog_rotate_angle_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  gint value;
  GObject *object;
  NimDialogPrivate *priv;

  priv = this->priv;
  value = gtk_combo_box_get_active (combo);
  object = gtk_builder_get_object (priv->builder, "rotate_custom_box");

  if (object)
    gtk_widget_set_sensitive (GTK_WIDGET (object), value == NIM_ROTATE_CUSTOM);

  nim_dialog_write_value (this, G_OBJECT (combo));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_widget_set_visible (GtkWidget *widget, gboolean setting)
{
    g_return_if_fail (GTK_IS_WIDGET (widget));

    gtk_widget_set_no_show_all (widget, !setting);
    gtk_widget_set_visible (widget, setting);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*
  Simple common callback
*/
static void nim_dialog_simple_callback (GObject *widget, NimDialog *this)
{
    nim_dialog_write_value (this, widget);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_resize_mode_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *width_box;
  GObject *height_box;
  gint active;
  
  priv = this->priv;
  active = gtk_combo_box_get_active (combo);

  width_box = gtk_builder_get_object (priv->builder, "resize_width_box");
  height_box = gtk_builder_get_object (priv->builder, "resize_height_box");
  gtk_widget_set_sensitive (GTK_WIDGET (width_box), active == NIM_RESIZE_HEIGHT ? FALSE : TRUE);
  gtk_widget_set_sensitive (GTK_WIDGET (height_box), active == NIM_RESIZE_WIDTH ? FALSE : TRUE);
  nim_dialog_write_value (this, G_OBJECT (combo));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_resize_thumb_toggled (GtkToggleButton *button, NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *object;
  gboolean active;

  priv = this->priv;
  active = gtk_toggle_button_get_active (button);
  object = gtk_builder_get_object (priv->builder, "resize_filter_box");
  gtk_widget_set_sensitive (GTK_WIDGET (object), !active);
  nim_dialog_write_value (this, G_OBJECT (button));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_resize_filter_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  gint active;
  GObject *factor_box;

  priv = this->priv;
  factor_box = gtk_builder_get_object (priv->builder, "resize_factor_box");
  active = gtk_combo_box_get_active (combo);

  gtk_widget_set_sensitive (GTK_WIDGET (factor_box), active != NIM_RESIZE_FILTER_NONE);
  nim_dialog_write_value (this, G_OBJECT (combo));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_conv_type_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  gint active;
  GObject *quality_box;
  GObject *animation_box;
  
  priv = this->priv;
  active = gtk_combo_box_get_active (combo);
  active = active < 0 ? 0 : active;

  quality_box = gtk_builder_get_object (priv->builder, "conv_quality_box");
  animation_box = gtk_builder_get_object (priv->builder, "conv_animation_box");

  nim_dialog_widget_set_visible (GTK_WIDGET (quality_box),
                                  (active == NIM_CONVERT_TYPE_JPEG
                                  || active == NIM_CONVERT_TYPE_PNG
                                  || active == NIM_CONVERT_TYPE_TIFF));
  nim_dialog_widget_set_visible (GTK_WIDGET (animation_box),
                                  (active == NIM_CONVERT_TYPE_GIF));
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
static void  nim_dialog_round_spin_changed (NimDialog *this)
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
  GtkSpinButton *spin;

  priv = this->priv;

  if (GTK_IS_SPIN_BUTTON (priv->user_data)) {
    spin = GTK_SPIN_BUTTON (priv->user_data);
    priv->user_data = NULL;
  } else {
    return;
  }
    
  filename = nim_imaging_find_file (NIM_FIND_IMAGE);
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
    nim_imaging_resize_from_wand (&wand,
                                   DEFAULT_PREVIEW_WIDTH,
                                   DEFAULT_PREVIEW_HEIGHT,
                                   NIM_RESIZE_BOTH,
                                   TRUE,
                                   Lanczos2Filter,
                                   1.0);
    if ((pixbuf = nim_imaging_convert_wand_to_pixbuf (wand)) != NULL)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
      g_object_unref (G_OBJECT (pixbuf));
    }

    wand = DestroyMagickWand (wand);
  }

  nim_dialog_write_value (this, G_OBJECT (spin));
  g_free (filename);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_widget_changed_timeout (GtkWidget *widget, NimDialog *this)
{
  this->priv->user_data = (gpointer) widget;
  fake_callback_function (this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_effect_widget_changed_worker (NimDialog *this)
{
  gchar *filename;
  NimDialogPrivate *priv;

  GObject *background;
  GObject *offx_spin;
  GObject *offy_spin;
  GObject *radius_spin;
  GObject *sigma_spin;
  GObject *effect_type;
  GObject *angle_spin;
  GObject *image;
  GdkPixbuf *pixbuf;
  MagickWand *wand;
  gboolean enable_bg;
  gdouble offx, offy, radius, sigma, efftype, angle;

  priv = this->priv;
  priv->user_data = NULL;

  if (!IsMagickWand (priv->preview_wand)) {
    gboolean result;
    filename = nim_imaging_find_file (NIM_FIND_IMAGE);
    
    if (filename == NULL)
      return;

    priv->preview_wand = NewMagickWand ();
    result = MagickReadImage (priv->preview_wand, filename) == MagickTrue;
    g_free (filename);

    if (!result) {
      priv->preview_wand = DestroyMagickWand (priv->preview_wand);
      return;
    }
  }
    
  image = gtk_builder_get_object (priv->builder, "effect_preview_image");
  background = gtk_builder_get_object (priv->builder, "effect_enable_bg_button");
  offx_spin = gtk_builder_get_object (priv->builder, "effect_offsetx_spin");
  offy_spin = gtk_builder_get_object (priv->builder, "effect_offsety_spin");
  radius_spin = gtk_builder_get_object (priv->builder, "effect_radius_spin");
  sigma_spin = gtk_builder_get_object (priv->builder, "effect_sigma_spin");
  effect_type = gtk_builder_get_object (priv->builder, "effect_type_combo");
  angle_spin = gtk_builder_get_object (priv->builder, "effect_angle_spin");

  offx = gtk_spin_button_get_value (GTK_SPIN_BUTTON (offx_spin));
  offy = gtk_spin_button_get_value (GTK_SPIN_BUTTON (offy_spin));
  radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (radius_spin));
  sigma = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sigma_spin));
  efftype = gtk_combo_box_get_active (GTK_COMBO_BOX (effect_type));
  enable_bg = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (background));
  angle = gtk_spin_button_get_value (GTK_SPIN_BUTTON (angle_spin));

  if (IsMagickWand (priv->preview_wand)) {
    wand = CloneMagickWand (priv->preview_wand);
    nim_imaging_effect_from_wand (&wand, efftype, offx, offy, radius, sigma, angle, enable_bg);
    nim_imaging_resize_from_wand (&wand,
                                   DEFAULT_PREVIEW_WIDTH,
                                   DEFAULT_PREVIEW_HEIGHT,
                                   NIM_RESIZE_BOTH,
                                   TRUE,
                                   LanczosFilter,
                                   1.0);
  } else {
    return;
  }

  pixbuf = nim_imaging_convert_wand_to_pixbuf (wand);

  if (pixbuf) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
    g_object_unref (G_OBJECT (pixbuf));
  }

  if (IsMagickWand (wand))
    wand = DestroyMagickWand (wand);

  return;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_effect_type_combo_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  gint active;
  GObject *offxbox;
  GObject *offybox;
  GObject *radiusbox;
  GObject *sigmabox;
  GObject *anglebox;
  GObject *bgbox;
  gboolean offxactive;
  gboolean offyactive;
  gboolean radiusactive;
  gboolean sigmaactive;
  gboolean angleactive;
  gboolean bgactive;
  
  priv = this->priv;

  active = gtk_combo_box_get_active (combo);
  offxbox = gtk_builder_get_object (priv->builder, "effect_offsetx_box");
  offybox = gtk_builder_get_object (priv->builder, "effect_offsety_box");
  radiusbox = gtk_builder_get_object (priv->builder, "effect_radius_box");
  sigmabox = gtk_builder_get_object (priv->builder, "effect_sigma_box");
  anglebox = gtk_builder_get_object (priv->builder, "effect_angle_box");
  bgbox = gtk_builder_get_object (priv->builder, "effect_enable_bg_button");

  offxactive = active == NIM_EFFECT_SHADOW
            || active == NIM_EFFECT_ROLL;
            
  offyactive = active == NIM_EFFECT_SHADOW
            || active == NIM_EFFECT_ROLL;

  bgactive = active == NIM_EFFECT_SHADOW;
  radiusactive = active == NIM_EFFECT_BLUR
             || active == NIM_EFFECT_SHARPEN
             || active == NIM_EFFECT_MOTION
             || active == NIM_EFFECT_OIL
             || active == NIM_EFFECT_SKETCH
             || active == NIM_EFFECT_SPREAD
             || active == NIM_EFFECT_CHARCOAL
             || active == NIM_EFFECT_EDGE
             || active == NIM_EFFECT_EMBROSS
             || active == NIM_EFFECT_GAUSSIAN;
             
  sigmaactive = active == NIM_EFFECT_SHADOW
             || active == NIM_EFFECT_BLUR
             || active == NIM_EFFECT_SHARPEN
             || active == NIM_EFFECT_MOTION
             || active == NIM_EFFECT_SKETCH
             || active == NIM_EFFECT_CHARCOAL
             || active == NIM_EFFECT_EMBROSS
             || active == NIM_EFFECT_GAUSSIAN;

  angleactive = active == NIM_EFFECT_SKETCH
             || active == NIM_EFFECT_MOTION
             || active == NIM_EFFECT_RADIAL_BLUR;

  nim_dialog_widget_set_visible (GTK_WIDGET (offxbox), offxactive);
  nim_dialog_widget_set_visible (GTK_WIDGET (offybox), offyactive);
  nim_dialog_widget_set_visible (GTK_WIDGET (radiusbox), radiusactive);
  nim_dialog_widget_set_visible (GTK_WIDGET (sigmabox), sigmaactive);
  nim_dialog_widget_set_visible (GTK_WIDGET (anglebox), angleactive);
  nim_dialog_widget_set_visible (GTK_WIDGET (bgbox), bgactive);
  fake_callback_function (this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_common_radio_toggled (GtkToggleButton *button, NimDialog *this)
{
  gboolean active;
  GtkWidget *child_box;

  active = gtk_toggle_button_get_active (button);
  child_box = (GtkWidget *) g_object_get_data (G_OBJECT (button), NIM_CHILD_WIDGET);
  
  if (GTK_IS_WIDGET (child_box)) 
    gtk_widget_set_sensitive (child_box, active);

  nim_dialog_write_value (this, G_OBJECT (button));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_choose_folder_activated (GtkWidget *widget, NimDialog *this)
{
  NimDialogPrivate *priv;
  GtkWidget *fcdialog;
  GObject *dialog;
  GObject *entry;
  gint response;
  const gchar *filename;
  
  priv = this->priv;
  dialog = gtk_builder_get_object (priv->builder, "dialog1");
  entry = gtk_builder_get_object (priv->builder, "save_choose_entry");
  filename = gtk_entry_get_text (GTK_ENTRY (entry));
  fcdialog = gtk_file_chooser_dialog_new (NULL,
                                          GTK_WINDOW (dialog),
                                          GTK_FILE_CHOOSER_ACTION_OPEN
                                          | GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
                                          GTK_STOCK_CANCEL,
                                          GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_OK,
                                          NULL);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fcdialog), filename);
  response = gtk_dialog_run (GTK_DIALOG (fcdialog));
  gtk_widget_hide (fcdialog);

  if (response == GTK_RESPONSE_OK) {
    gchar *foldername;
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
static void nim_dialog_water_file_button_toggled (GtkWidget *widget, NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *entry;
  GtkWidget *fcdialog;
  GObject *dialog;
  GtkFileFilter *filter;
  GtkWidget *preview;
  gint response;
  gboolean active;

  priv = this->priv;

  if (GTK_IS_TOGGLE_BUTTON (widget) && !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    return;
    
  dialog = gtk_builder_get_object (priv->builder, "dialog1");
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
  entry = gtk_builder_get_object (priv->builder, "water_file");

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
      nim_dialog_write_value (this, G_OBJECT (entry));
      g_free (filename);
    }
  }

  gtk_widget_destroy (fcdialog);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_water_method_changed (GtkComboBox *combo, NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *alignbox;
  GObject *commonbox;
  GObject *absbox;
  gint active;

  priv = this->priv;
  alignbox = gtk_builder_get_object (priv->builder, "water_align_box");
  absbox = gtk_builder_get_object (priv->builder, "water_abs_box");
  commonbox = gtk_builder_get_object (priv->builder, "water_align_common_box");
  active = gtk_combo_box_get_active (combo);
  
  gtk_widget_set_sensitive (GTK_WIDGET (commonbox),
                                  active == NIM_WATER_METHOD_ALIGN
                               || active == NIM_WATER_METHOD_ABS);

  nim_dialog_widget_set_visible (GTK_WIDGET (alignbox), active < NIM_WATER_METHOD_ABS);
  nim_dialog_widget_set_visible (GTK_WIDGET (absbox), active == NIM_WATER_METHOD_ABS);
  nim_dialog_write_value (this, G_OBJECT (combo));
  g_print ("%s: %d\n", G_STRLOC, active);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void fontchooser_font_set_cb (NimFontChooser *chooser, const gchar *font_name, NimDialog *this)
{
  if (font_name)
    g_key_file_set_string (this->priv->config, MARKER_GROUP, MARKER_FONT_NAME_KEY, font_name);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void fontchooser_color_set_cb (NimFontChooser *chooser, const gchar *foreground, NimDialog *this)
{
  if (foreground)
    g_key_file_set_string (this->priv->config, MARKER_GROUP, MARKER_FONT_COLOR_KEY, foreground);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void fontchooser_size_changed (NimFontChooser *chooser, gint fontsize, NimDialog *this)
{
    g_key_file_set_integer (this->priv->config, MARKER_GROUP, MARKER_FONT_SIZE_KEY, fontsize);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GObject* nim_dialog_set_default (NimDialog *this,
                                        const gchar *group,
                                        const gchar *wname,
                                        GCallback callback)
{
  NimDialogPrivate *priv;
  GObject *object;
  gint vint;
  gdouble vdoub;
  gboolean vbool;
  gchar *vchar;
  const gchar *signal_name = NULL;
  const gchar *norm_group = group ? group : COMMON_GROUP;

  if (wname == NULL) return NULL;
    
  priv = this->priv;
  object = gtk_builder_get_object (priv->builder, wname);

  if (!G_IS_OBJECT (object)) return NULL;

  g_object_set_data_full (object, OBJECT_NAME, g_strdup (wname), g_free);
  g_object_set_data_full (object, OBJECT_GROUP, g_strdup (norm_group), g_free);

  if (callback) {
    if (GTK_IS_COMBO_BOX (object)) {
      signal_name = "changed";
    } else if (GTK_IS_SPIN_BUTTON (object)) {
      signal_name = "value-changed";
    } else if (GTK_IS_ADJUSTMENT (object)) {
      signal_name = "value-changed";
    } else if (GTK_IS_TOGGLE_BUTTON (object)) {
      signal_name = "toggled";
    } else if (GTK_IS_ENTRY (object)) {
      signal_name = "changed";
    } else if (GTK_IS_COLOR_BUTTON (object)) {
      signal_name = "color-set";
    } else if (GTK_IS_FONT_BUTTON (object)) {
      signal_name = "font-set";
    } else if (GTK_IS_BUTTON (object)) {
      signal_name = "clicked";
    }

    if (signal_name)
      g_signal_connect (object, signal_name, callback, this);
  }

  if (GTK_IS_EDITABLE (object))
    gtk_editable_set_editable (GTK_EDITABLE (object), TRUE);
    
  if (g_key_file_has_key (priv->config, group, wname, NULL)) {
    if (GTK_IS_COMBO_BOX (object)) {
      vint = g_key_file_get_integer (priv->config, group, wname, NULL);
      gtk_combo_box_set_active (GTK_COMBO_BOX (object), vint);

    } else if (GTK_IS_SPIN_BUTTON (object)) {
      vdoub = g_key_file_get_double (priv->config, group, wname, NULL);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (object), vdoub);

    } else if (GTK_IS_ADJUSTMENT (object)) {
      vdoub = g_key_file_get_double (priv->config, group, wname, NULL);
      gtk_adjustment_set_value (GTK_ADJUSTMENT (object), vdoub);

    } else if (GTK_IS_TOGGLE_BUTTON (object)) {
      vbool = g_key_file_get_boolean (priv->config, group, wname, NULL);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (object), vbool);

    } else if (GTK_IS_ENTRY (object)) {
      vchar = g_key_file_get_string (priv->config, group, wname, NULL);
      gtk_entry_set_text (GTK_ENTRY (object), vchar ? vchar : "");
      if (vchar) g_free (vchar);

    } else if (GTK_IS_COLOR_BUTTON (object)) {
      GdkRGBA color;
      vchar = g_key_file_get_string (priv->config, group, wname, NULL);

      if (vchar && color_to_rgba (&color, vchar))
        gtk_color_button_set_rgba (GTK_COLOR_BUTTON (object), &color);

      if (vchar) g_free (vchar);

    } else if (GTK_IS_FONT_BUTTON (object)) {
      vchar = g_key_file_get_string (priv->config, group, wname, NULL);

      if (vchar) {
        gtk_font_button_set_font_name (GTK_FONT_BUTTON (object), vchar);
        g_free (vchar);
      }
    } 
  }

  nim_dialog_write_value (this, object);
  
  return object;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_write_value (NimDialog *this, GObject *object)
{
  NimDialogPrivate *priv;
  const gchar *wname, *group;
  gint vint;
  gdouble vdoub;
  gboolean vbool;
  const gchar *vchar;

  if (!G_IS_OBJECT (object)) return;

  priv = this->priv;
  wname = (const gchar*) g_object_get_data (object, OBJECT_NAME);
  group = (const gchar*) g_object_get_data (object, OBJECT_GROUP);

  if (wname && group) {
    if (GTK_IS_COMBO_BOX (object)) {
      vint = gtk_combo_box_get_active (GTK_COMBO_BOX (object));
      g_key_file_set_integer (priv->config, group, wname, vint);

    } else if (GTK_IS_SPIN_BUTTON (object)) {
      vdoub = gtk_spin_button_get_value (GTK_SPIN_BUTTON (object));
      g_key_file_set_double (priv->config, group, wname, vdoub);

    } else if (GTK_IS_ADJUSTMENT (object)) {
      vdoub = gtk_adjustment_get_value (GTK_ADJUSTMENT (object));
      g_key_file_set_double (priv->config, group, wname, vdoub);

    } else if (GTK_IS_TOGGLE_BUTTON (object)) {
      vbool = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (object));
      g_key_file_set_boolean (priv->config, group, wname, vbool);

    } else if (GTK_IS_ENTRY (object)) {
      vchar = gtk_entry_get_text (GTK_ENTRY (object));
      g_key_file_set_string (priv->config, group, wname, vchar);

    } else if (GTK_IS_COLOR_BUTTON (object)) {
      GdkRGBA color;
      gchar *value;

      gtk_color_button_get_rgba (GTK_COLOR_BUTTON (object), &color);
      value = rgba_to_color (&color);

      if (value) {
        g_key_file_set_string (priv->config, group, wname, value);
        g_free (value);
      }

    } else if (GTK_IS_FONT_BUTTON (object)) {
      const gchar *font;
      gint fontsize;
      gchar *fontsize_key;

      fontsize_key = g_strdup_printf ("%s:size", wname);
      font = gtk_font_button_get_font_name (GTK_FONT_BUTTON (object));
      fontsize = gtk_font_chooser_get_font_size (GTK_FONT_CHOOSER (object));
      g_key_file_set_string (priv->config, group, wname, font);
      g_key_file_set_integer (priv->config, group, fontsize_key, fontsize);
      g_free (fontsize_key);
    }
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GObject* nim_dialog_write_value_by_name (NimDialog *this, const gchar *object_name)
{
  GObject *object;

  if (object_name) {
    object = gtk_builder_get_object (this->priv->builder, object_name);
    nim_dialog_write_value (this, object);
    return object;
  }

  return NULL; 
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_config_init (NimDialog *this)
{
  NimDialogPrivate *priv;
  gchar *filename;

  priv = this->priv;
  filename = nim_imaging_find_file (NIM_FIND_CONFIG);
  priv->config = g_key_file_new ();

printf ("%s: %s\n", G_STRLOC, filename);
  if (filename) {
    if (!g_key_file_load_from_file (priv->config, filename, G_KEY_FILE_NONE, NULL)) {
printf ("%s: Set defaults values\n", G_STRLOC);
      g_key_file_set_integer (priv->config, ROTATE_GROUP, "rotate_angle_combo", DEFAULT_ROTATE_MODE);
      g_key_file_set_string (priv->config, ROTATE_GROUP, "rotate_color_button", DEFAULT_ROTATE_COLOR);
      g_key_file_set_double (priv->config, ROTATE_GROUP, "rotate_angle_spin", DEFAULT_ROTATE_ANGLE);

      g_key_file_set_string (priv->config, COMMON_GROUP, "save_choose_entry", DEFAULT_COMMON_FOLDER);
      g_key_file_set_string (priv->config, COMMON_GROUP, "save_suffix_entry", DEFAULT_COMMON_SUFFIX);
      g_key_file_set_boolean (priv->config, COMMON_GROUP, "save_overwrite_radio", DEFAULT_COMMON_TYPE == NIM_SAVE_OVER);
      g_key_file_set_boolean (priv->config, COMMON_GROUP, "save_suffix_radio", DEFAULT_COMMON_TYPE == NIM_SAVE_MARK);
      g_key_file_set_boolean (priv->config, COMMON_GROUP, "save_choose_radio", DEFAULT_COMMON_TYPE == NIM_SAVE_OTHER);

      g_key_file_set_double (priv->config, MARKER_GROUP, "water_alignx", DEFAULT_MARKER_X);
      g_key_file_set_double (priv->config, MARKER_GROUP, "water_aligny", DEFAULT_MARKER_Y);
      g_key_file_set_double (priv->config, MARKER_GROUP, "water_absx", DEFAULT_MARKER_ABS_X);
      g_key_file_set_double (priv->config, MARKER_GROUP, "water_absy", DEFAULT_MARKER_ABS_Y);
      g_key_file_set_boolean (priv->config, MARKER_GROUP, "water_adaptive_size", DEFAULT_MARKER_ADAPTIVE);
      g_key_file_set_integer (priv->config, MARKER_GROUP, "water_text_effect", DEFAULT_MARKER_TEXT_EFFECT);
      g_key_file_set_boolean (priv->config, MARKER_GROUP, "water_file", DEFAULT_MARKER_FILE);
      g_key_file_set_double (priv->config, MARKER_GROUP, "water_pitch", DEFAULT_MARKER_PITCH);
      g_key_file_set_integer (priv->config, MARKER_GROUP, "water_method", DEFAULT_MARKER_METHOD);
      g_key_file_set_double (priv->config, MARKER_GROUP, "water_opacity", DEFAULT_MARKER_OPACITY);
      g_key_file_set_string (priv->config, MARKER_GROUP, "water_font_color", DEFAULT_MARKER_FG);
      g_key_file_set_string (priv->config, MARKER_GROUP, "water_font", DEFAULT_MARKER_FONT);
      g_key_file_set_integer (priv->config, MARKER_GROUP, "water_font:size", DEFAULT_MARKER_FONT_SIZE);
      g_key_file_set_string (priv->config, MARKER_GROUP, "water_entry", DEFAULT_MARKER_TEXT);

      g_key_file_set_boolean (priv->config, EFFECTS_GROUP, "effect_enable_bg_button", DEFAULT_EFFECT_BG);
      g_key_file_set_double (priv->config, EFFECTS_GROUP, "effect_angle_spin", DEFAULT_EFFECT_ANGLE);
      g_key_file_set_double (priv->config, EFFECTS_GROUP, "effect_sigma_spin", DEFAULT_EFFECT_SIGMA);
      g_key_file_set_double (priv->config, EFFECTS_GROUP, "effect_radius_spin", DEFAULT_EFFECT_RADIUS);
      g_key_file_set_double (priv->config, EFFECTS_GROUP, "effect_offsety_spin", DEFAULT_EFFECT_OFFY);
      g_key_file_set_double (priv->config, EFFECTS_GROUP, "effect_offsetx_spin", DEFAULT_EFFECT_OFFX);
      g_key_file_set_integer (priv->config, EFFECTS_GROUP, "effect_type_combo", DEFAULT_EFFECT_TYPE);

      g_key_file_set_integer (priv->config, ROUND_GROUP, "round_tl_spin", DEFAULT_ROUND_TL);
      g_key_file_set_integer (priv->config, ROUND_GROUP, "round_bl_spin", DEFAULT_ROUND_BL);
      g_key_file_set_integer (priv->config, ROUND_GROUP, "round_tr_spin", DEFAULT_ROUND_TR);
      g_key_file_set_integer (priv->config, ROUND_GROUP, "round_br_spin", DEFAULT_ROUND_BR);
      g_key_file_set_boolean (priv->config, ROUND_GROUP, "round_stick_button", DEFAULT_ROUND_STICK);

      g_key_file_set_integer (priv->config, CONVERT_GROUP, "conv_type_combo", DEFAULT_CONVERT_TYPE);
      g_key_file_set_double (priv->config, CONVERT_GROUP, "conv_quality_spin", DEFAULT_CONVERT_QUALITY);
      g_key_file_set_integer (priv->config, CONVERT_GROUP, "conv_speed_spin", DEFAULT_CONVERT_ASPEED);
      g_key_file_set_boolean (priv->config, CONVERT_GROUP, "conv_combine_files", DEFAULT_CONVERT_COMBINE);

      g_key_file_set_integer (priv->config, RESIZE_GROUP, "resize_mode_combo",  DEFAULT_RESIZE_MODE);
      g_key_file_set_integer (priv->config, RESIZE_GROUP, "resize_width_spin", DEFAULT_RESIZE_WIDTH);
      g_key_file_set_integer (priv->config, RESIZE_GROUP, "resize_height_spin", DEFAULT_RESIZE_HEIGHT);
      g_key_file_set_boolean (priv->config, RESIZE_GROUP, "resize_thumb_button", DEFAULT_RESIZE_THUMB);
      g_key_file_set_integer (priv->config, RESIZE_GROUP, "resize_filter_combo", DEFAULT_RESIZE_FILTER);
      g_key_file_set_double (priv->config, RESIZE_GROUP, "resize_factor_spin", DEFAULT_RESIZE_FACTOR);
    } else {
printf ("%s: Read config success\n", G_STRLOC);
    }
    g_free (filename);
  }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_common_init (NimDialog *this)
{
  GObject *button;
  GObject *child;
  NimDialogPrivate *priv;
  
  priv = this->priv;
  button = gtk_builder_get_object (priv->builder, "save_suffix_radio");
  child = gtk_builder_get_object (priv->builder, "save_option_box1");
  g_object_set_data (button, NIM_CHILD_WIDGET, child);

  button = gtk_builder_get_object (priv->builder, "save_choose_radio");
  child = gtk_builder_get_object (priv->builder, "save_option_box2");
  g_object_set_data (button, NIM_CHILD_WIDGET, child);

  nim_dialog_set_default (this, COMMON_GROUP, "save_overwrite_radio",
                            G_CALLBACK (nim_dialog_common_radio_toggled));
  nim_dialog_set_default (this, COMMON_GROUP, "save_suffix_radio",
                            G_CALLBACK (nim_dialog_common_radio_toggled));
  nim_dialog_set_default (this, COMMON_GROUP, "save_choose_radio",
                            G_CALLBACK (nim_dialog_common_radio_toggled));
  nim_dialog_set_default (this, COMMON_GROUP, "save_choose_button",
                            G_CALLBACK (nim_dialog_choose_folder_activated));
  nim_dialog_set_default (this, COMMON_GROUP, "save_suffix_entry",
                            G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, COMMON_GROUP, "save_choose_entry",
                            G_CALLBACK (nim_dialog_simple_callback));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_rotate_init (NimDialog *this)
{
  GObject *combo;
  nim_dialog_set_default (this, ROTATE_GROUP, "rotate_angle_combo",
                            G_CALLBACK (nim_dialog_rotate_angle_combo_changed));
  nim_dialog_set_default (this, ROTATE_GROUP, "rotate_angle_spin",
                            G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, ROTATE_GROUP, "rotate_color_button",
                            G_CALLBACK (nim_dialog_simple_callback));
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_resize_init (NimDialog *this)
{
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_mode_combo",
                            G_CALLBACK (nim_dialog_resize_mode_combo_changed));
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_width_spin",
                            G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_height_spin",
                            G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_thumb_button",
                            G_CALLBACK (nim_dialog_resize_thumb_toggled));
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_filter_combo",
                            G_CALLBACK (nim_dialog_resize_filter_combo_changed));
  nim_dialog_set_default (this, RESIZE_GROUP, "resize_factor_spin",
                            G_CALLBACK (nim_dialog_simple_callback));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_convert_init (NimDialog *this)
{
  nim_dialog_set_default (this, CONVERT_GROUP, "conv_type_combo",
                              G_CALLBACK (nim_dialog_conv_type_combo_changed));
  nim_dialog_set_default (this, CONVERT_GROUP, "quality_adj",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, CONVERT_GROUP, "anim_speed_adj",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, CONVERT_GROUP, "conv_combine_files",
                              G_CALLBACK (nim_dialog_simple_callback));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void  nim_dialog_rounder_init (NimDialog *this)
{
  GObject *object;

  this->priv->source_func = nim_dialog_round_spin_changed;
  nim_dialog_set_default (this, ROUND_GROUP, "round_tl_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, ROUND_GROUP, "round_tr_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, ROUND_GROUP, "round_bl_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  object = nim_dialog_set_default (this, ROUND_GROUP, "round_br_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, ROUND_GROUP, "round_stick_button",
                              G_CALLBACK (nim_dialog_simple_callback));
  this->priv->user_data = (gpointer) object;
  nim_dialog_round_spin_changed (this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_effects_init (NimDialog *this)
{
  this->priv->source_func = nim_dialog_effect_widget_changed_worker;

  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_type_combo",
                              G_CALLBACK (nim_dialog_effect_type_combo_changed));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_offsetx_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_offsety_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_radius_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_sigma_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_angle_spin",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_set_default (this, EFFECTS_GROUP, "effect_enable_bg_button",
                              G_CALLBACK (nim_dialog_widget_changed_timeout));
  nim_dialog_effect_widget_changed_worker (this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_dialog_marker_init (NimDialog *this)
{
  NimDialogPrivate *priv;
  GObject *button;
  GObject *child;
  GObject *mainbox;
  GtkWidget *fontchooser;
  gchar *fontname;
  gchar *color;
  gint fontsize;

  priv = this->priv;

  nim_dialog_set_default (this, MARKER_GROUP, "water_file",
                              G_CALLBACK (nim_dialog_water_file_button_toggled));
  child = nim_dialog_set_default (this, MARKER_GROUP, "water_entry",
                              G_CALLBACK (nim_dialog_simple_callback));
  gtk_editable_set_editable (GTK_EDITABLE (child), TRUE);
  
  nim_dialog_set_default (this, MARKER_GROUP, "water_opacity",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_pitch",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_alignx",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_aligny",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_absx",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_absy",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_text_effect",
                              G_CALLBACK (nim_dialog_simple_callback));
  nim_dialog_set_default (this, MARKER_GROUP, "water_method",
                              G_CALLBACK (nim_dialog_water_method_changed));
  mainbox = gtk_builder_get_object (priv->builder, "water_font_box");
  fontchooser = nim_font_chooser_new ();
  gtk_container_add (GTK_CONTAINER (mainbox), fontchooser);
  g_signal_connect (fontchooser, "font-set", G_CALLBACK (fontchooser_font_set_cb), this);
  g_signal_connect (fontchooser, "color-set", G_CALLBACK (fontchooser_color_set_cb), this);
  g_signal_connect (fontchooser, "size_changed", G_CALLBACK (fontchooser_size_changed), this);

  fontname = g_key_file_get_string (priv->config, MARKER_GROUP, MARKER_FONT_NAME_KEY, NULL);
  color = g_key_file_get_string (priv->config, MARKER_GROUP, MARKER_FONT_COLOR_KEY, NULL);
  fontsize = g_key_file_get_integer (priv->config, MARKER_GROUP, MARKER_FONT_SIZE_KEY, NULL);

  fontname = (fontname != NULL) ? fontname : g_strdup (DEFAULT_MARKER_FONT);
  nim_font_chooser_set_font_name (NIM_FONT_CHOOSER (fontchooser), fontname);
  g_free (fontname);

  color = (color != NULL) ? color : g_strdup (DEFAULT_MARKER_FG);
  nim_font_chooser_set_foreground (NIM_FONT_CHOOSER (fontchooser), color);
  g_free (color);

  fontsize = fontsize == 0 ? DEFAULT_MARKER_FONT_SIZE : fontsize;
  fontsize = CLAMP (fontsize, NIM_MIN_FONT_SIZE, NIM_MAX_FONT_SIZE);
  nim_font_chooser_set_font_size (NIM_FONT_CHOOSER (fontchooser), fontsize);
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
  GtkWidget *mainbox;
  GError *error = NULL;
  gchar *uifile;

  object = g_object_new (NIM_TYPE_DIALOG, NULL);
  this = NIM_DIALOG (object);
  priv = this->priv;
  priv->preview_wand = NULL;
  priv->source = 0L;
  priv->source_func = NULL;
  priv->user_data = NULL;

  priv->dialog_type = dialog_type;
  priv->builder = gtk_builder_new ();
  uifile = nim_imaging_find_file (NIM_FIND_UI);
  gtk_builder_add_from_file (priv->builder, uifile, &error);
  g_free (uifile);

  if (error) {
    g_print ("ERROR: %i: %s\n", error->code, error->message);
    g_error_free (error);
    return this;
  }
  
  gchar *widget_name;

  widget_name = g_strdup_printf ("property%i", dialog_type);
  widget = (GtkWidget *) gtk_builder_get_object (priv->builder, widget_name);
  dialog = (GtkWidget *) gtk_builder_get_object (priv->builder, "dialog1");
  mainbox = (GtkWidget *) gtk_builder_get_object (priv->builder, "main_box");
  
  g_free (widget_name);

  if (widget && mainbox) {
    gtk_box_pack_start (GTK_BOX (mainbox), widget, TRUE, TRUE, 0);
    gtk_widget_show_all (mainbox);
  }

  if (dialog && GTK_IS_WINDOW (parent_window)) {
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent_window);
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  }

  nim_dialog_config_init (this);
  nim_dialog_common_init (this);

  switch (dialog_type) {
    case NIM_FUNCTION_RESIZE:
      nim_dialog_resize_init (this); break;
    case NIM_FUNCTION_ROTATE:
      nim_dialog_rotate_init (this); break;
    case NIM_FUNCTION_CONVERT:
      nim_dialog_convert_init (this); break;
    case NIM_FUNCTION_MARK:
      nim_dialog_marker_init (this); break;
    case NIM_FUNCTION_CORNERS:
      nim_dialog_rounder_init (this); break;
    case NIM_FUNCTION_EFFECTS:
      nim_dialog_effects_init (this); break;
    default: break;
  } 

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
    this->priv->source = g_timeout_add (TIMEOUT_TO_CHANGE, (GSourceFunc) fake_timeout_function, this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *nim_dialog_get_data (NimDialog *this, gsize *length)
{
  gchar *result;
  
  result = g_key_file_to_data (this->priv->config, length, NULL);

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
