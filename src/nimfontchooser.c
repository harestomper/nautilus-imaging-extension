/*
 * nimfontchooser.c
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

#include "nimfontchooser.h"
#include <wand/magick_wand.h>
#include <math.h>
#include "nimimaging.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define PREVIEW_SIZE                  18
#define FILE_CHOOSER_PREVIEW_WIDTH    250
#define FILE_CHOOSER_PREVIEW_HEIGHT   150
#define SEPARATOR_ID                  "<separator>"
#define LAST_ROW                      "Choose file"
#define CHECK_DARK                    (1.0 / 3.0)
#define CHECK_LIGHT                   (2.0 / 3.0)
#define CHECK_SIZE                    4
#define VISIBLE_ROWS                  10
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
  COLUMN_PIXBUF,
  COLUMN_TEXT,
  COLUMN_DATA,
  COLUMN_LAST
};

enum {
  SIGNAL_FONT_SET,
  SIGNAL_COLOR_SET,
  SIGNAL_SIZE_CHANGED,
  SIGNAL_LAST
};

static uint   signals [SIGNAL_LAST];
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimFontChooserPrivate
{
  GtkWidget             *fontbutton;  // Toggle button to open popup window
  GtkWidget             *spin_size;    // Spin button for font size
  GtkWidget             *popup_window; // Popup window for menu of font selector
  GtkWidget             *treeview;     // Tree of menu
  GtkWidget             *scrolled;
  GtkWidget             *image_font;   // Image of font preview
  GtkWidget             *image_color;  // Image of foreground color
  GtkWidget             *cs_dialog;
  GtkTreeModel          *treemodel; // Model of the tree view
  GtkTreeRowReference   *active_path; // Current selected path
  GSList                *exists_other;   // LIst of custom added files
  gint                  n_elem;        // Num of the elements on tree view
  gint                  last_active_row;
  GdkRGBA               foreground;    // String format of the font color
  GdkRGBA               background;    // String format of the background
  GdkDevice             *grab_pointer;
  GdkDevice             *grab_keyboard;
  gboolean               list_complete;
  gchar                 *font_name;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimFontChooser, nim_font_chooser, GTK_TYPE_BOX)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_row_separator_func (GtkTreeModel *model,
                                                     GtkTreeIter *iter,
                                                     NimFontChooser *chooser);
static void nim_font_chooser_fontname_changed (GtkComboBox *combo, NimFontChooser *this);
static void nim_font_shooser_open_filechooser (GtkWidget *widget, NimFontChooser *chooser);
static void nim_font_chooser_font_button_toggled (GtkWidget *widget, NimFontChooser *chooser);
static void nim_font_chooser_open_colorchooser (GtkWidget *widget, NimFontChooser *chooser);
static void nim_font_chooser_size_changed (GtkWidget *widget, NimFontChooser *chooser);
//static gint nim_font_chooser_get_active (NimFontChooser *chooser);
static void nim_font_chooser_set_active (NimFontChooser *chooser, gint new_active);
static void nim_font_chooser_set_active_iter (NimFontChooser *chooser, GtkTreeIter *iter);
static gboolean nim_font_chooser_get_active_iter (NimFontChooser *chooser, GtkTreeIter *iter);
static gint nim_font_chooser_list_length (NimFontChooser *chooser);
static gboolean nim_font_chooser_find_similar (NimFontChooser *chooser, const gchar *pattern, GtkTreeIter *iter);

static gboolean nim_font_chooser_append (NimFontChooser  *chooser,
                                      const gchar       *new_font,
                                      GtkTreeIter           *iter,
                                      gboolean         set_active);

static gboolean nim_font_chooser_read_fonts (NimFontChooser *chooser);
static gboolean image_color_draw_cb (GtkWidget *widget, cairo_t *cr, NimFontChooser *chooser);
static void nim_font_chooser_set_active_internal (NimFontChooser *chooser, GtkTreePath *path);
static void nim_font_chooser_set_diaplayed_path (NimFontChooser *chooser, GtkTreePath *path);
static gboolean nim_font_chooser_popdown (NimFontChooser *chooser);
static gboolean nim_font_chooser_popup (NimFontChooser *chooser);
static gboolean popup_window_key_press_event_cb (GtkWidget *widget,
                                      GdkEventKey *event,
                                      NimFontChooser *chooser);

static gboolean popup_window_button_press_event_cb (GtkWidget   *widget,
                                      GdkEventButton            *event,
                                      NimFontChooser            *chooser);

static GtkTreePath *nim_font_chooser_get_diaplayed_path (NimFontChooser *chooser);

static void nim_font_chooser_popup_for_device (NimFontChooser *chooser, GdkDevice *device);

static gboolean popup_grab_on_window (NimFontChooser   *chooser,
                                      GdkWindow         *window,
                                      GdkDevice  *grab_keyboard,
                                      GdkDevice   *grab_pointer,
                                      guint32         grab_time);

static void nim_font_chooser_list_position (NimFontChooser *chooser,
                                      gint        *x,
                                      gint        *y,
                                      gint    *width,
                                      gint   *height);

static gboolean tree_view_button_release_event_cb (GtkTreeView *tree,
                                      GdkEventButton *event,
                                      NimFontChooser *chooser);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_class_init (NimFontChooserClass *klass)
{
  GObject *object;
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  
  g_type_class_add_private ((gpointer) klass, sizeof (NimFontChooserPrivate));

  signals [SIGNAL_FONT_SET] = g_signal_new ("font-set",
                              NIM_TYPE_FONT_CHOOSER,
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);

  signals [SIGNAL_COLOR_SET] = g_signal_new ("color-set",
                              NIM_TYPE_FONT_CHOOSER,
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);

  signals [SIGNAL_SIZE_CHANGED] = g_signal_new ("size_changed",
                              NIM_TYPE_FONT_CHOOSER,
                              G_SIGNAL_RUN_LAST,
                              0,
                              NULL,
                              NULL,
                              g_cclosure_marshal_VOID__INT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_INT);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_init (NimFontChooser *this)
{
  NimFontChooserPrivate *priv;
  GtkCellRenderer *celltext;
  GtkCellRenderer *cellpixbuf;
  GtkTreeViewColumn *column;
  GtkAdjustment *adjustment;
  GtkWidget *colorbutton;
  GtkWidget *filebutton;
  GtkWidget *fileimage;
  GdkPixbuf *pixbuf;
  GtkSizeGroup *wgroup;
  GtkSizeGroup *hgroup;
  
  this->priv = G_TYPE_INSTANCE_GET_PRIVATE (this, NIM_TYPE_FONT_CHOOSER, NimFontChooserPrivate);
  priv = this->priv;

  priv->n_elem = 0;
  priv->last_active_row = 0;
  priv->exists_other = NULL;
  priv->active_path = NULL;
  priv->cs_dialog = NULL;
  priv->grab_pointer = NULL;
  priv->grab_keyboard = NULL;
  priv->foreground.red = 1.;
  priv->foreground.green = 1.;
  priv->foreground.blue = 1.;
  priv->foreground.alpha = 1.;
  priv->background.red = 1.;
  priv->background.green = 1.;
  priv->background.blue = 1.;
  priv->background.alpha = 1.;
  priv->list_complete = FALSE;
  priv->font_name = NULL;
  
  priv->treemodel = (GtkTreeModel*) gtk_list_store_new (COLUMN_LAST, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
  priv->treeview = gtk_tree_view_new_with_model (priv->treemodel);
  column = gtk_tree_view_column_new ();
  celltext = gtk_cell_renderer_text_new ();
  cellpixbuf = gtk_cell_renderer_pixbuf_new ();
  priv->scrolled = gtk_scrolled_window_new (NULL, NULL);
  priv->popup_window = gtk_window_new (GTK_WINDOW_POPUP);
  hgroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  wgroup = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cellpixbuf, FALSE);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), celltext, TRUE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);
  gtk_container_add (GTK_CONTAINER (priv->scrolled), priv->treeview);
  gtk_container_add (GTK_CONTAINER (priv->popup_window), priv->scrolled);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_tree_view_set_hover_selection (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  priv->image_font = gtk_image_new ();
  priv->image_color = gtk_drawing_area_new ();
  priv->fontbutton = gtk_toggle_button_new ();
  adjustment = gtk_adjustment_new (26, 8, 1000, 1, 10, 0);
  priv->spin_size = gtk_spin_button_new (adjustment, 0, 0);
  colorbutton = gtk_button_new ();
  filebutton = gtk_button_new ();
  pixbuf = gtk_widget_render_icon_pixbuf (filebutton, GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
  fileimage = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (G_OBJECT (pixbuf));

  gtk_widget_set_app_paintable (priv->image_color, TRUE);
  gtk_container_add (GTK_CONTAINER (colorbutton), priv->image_color);
  gtk_container_add (GTK_CONTAINER (filebutton), fileimage);
  gtk_container_add (GTK_CONTAINER (priv->fontbutton), priv->image_font);
  gtk_box_pack_start (GTK_BOX (this), priv->fontbutton, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (this), colorbutton, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (this), filebutton, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (this), priv->spin_size, TRUE, TRUE, 0);

  gtk_size_group_add_widget (wgroup, priv->fontbutton);
  gtk_size_group_add_widget (wgroup, colorbutton);
  gtk_size_group_add_widget (wgroup, filebutton);
  gtk_size_group_add_widget (wgroup, priv->spin_size);
  gtk_size_group_add_widget (hgroup, priv->fontbutton);
  gtk_size_group_add_widget (hgroup, colorbutton);
  gtk_size_group_add_widget (hgroup, filebutton);
  
  gtk_orientable_set_orientation (GTK_ORIENTABLE (this), GTK_ORIENTATION_HORIZONTAL);
  gtk_box_set_spacing (GTK_BOX (this), 3);

  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column),
                                    cellpixbuf, "pixbuf", COLUMN_PIXBUF, NULL);

  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column),
                                          celltext, "text", COLUMN_TEXT, NULL);

  g_object_set (G_OBJECT (celltext), "ellipsize", PANGO_ELLIPSIZE_END,
                                                  "ellipsize-set", TRUE, NULL);
                                    
  gtk_widget_show_all (GTK_WIDGET (this));

  g_signal_connect (filebutton, "clicked",
                         G_CALLBACK (nim_font_shooser_open_filechooser), this);

  g_signal_connect (priv->fontbutton, "toggled",
                      G_CALLBACK (nim_font_chooser_font_button_toggled), this);

  g_signal_connect (priv->treeview, "button-release-event",
                          G_CALLBACK (tree_view_button_release_event_cb), this);
                          
  g_signal_connect (priv->popup_window, "key-press-event",
                           G_CALLBACK (popup_window_key_press_event_cb), this);

  g_signal_connect (priv->popup_window, "button-press-event",
                        G_CALLBACK (popup_window_button_press_event_cb), this);

  g_signal_connect (colorbutton, "clicked",
                        G_CALLBACK (nim_font_chooser_open_colorchooser), this);

  g_signal_connect (priv->spin_size, "value-changed",
                             G_CALLBACK (nim_font_chooser_size_changed), this);

  g_signal_connect (priv->image_color, "draw", G_CALLBACK (image_color_draw_cb), this);
                             
  gdk_threads_add_idle ((GSourceFunc) nim_font_chooser_read_fonts,  this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GtkWidget* nim_font_chooser_new (void)
{
  return g_object_new (NIM_TYPE_FONT_CHOOSER, NULL);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void file_chooser_update_preview_cb (GtkFileChooser *filechooser, NimFontChooser *this)
{
  GdkPixbuf *pixbuf = NULL;
  NimFontChooserPrivate *priv;
  GtkWidget *preview_widget;
  gchar *filename, *basename = NULL, *fontname = NULL;

  filename = gtk_file_chooser_get_preview_filename (filechooser);
  
  if (filename) {
    gchar *temp;
    basename = g_path_get_basename (filename);
    temp = g_strrstr (basename, ".");
    if (temp != NULL)
      fontname = g_strndup (basename, temp - basename);
  }
  
  if (filename && !g_file_test (filename, G_FILE_TEST_IS_DIR)
      && nim_imaging_make_font_preview (&pixbuf,
                  filename,
                  FILE_CHOOSER_PREVIEW_WIDTH,
                  FILE_CHOOSER_PREVIEW_WIDTH,
                  FILE_CHOOSER_PREVIEW_HEIGHT,
                  "#000000ff",
                  "#ffffffff",
                  fontname ? fontname : basename,
                  0.0))
  {
    preview_widget = gtk_file_chooser_get_preview_widget (filechooser);
    gtk_image_clear (GTK_IMAGE (preview_widget));
    gtk_image_set_from_pixbuf (GTK_IMAGE (preview_widget), pixbuf);
    gtk_file_chooser_set_preview_widget_active (filechooser, TRUE);
    g_object_unref (G_OBJECT (pixbuf));
  }

  if (filename)
    g_free (filename);

  if (basename)
    g_free (basename);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_shooser_open_filechooser (GtkWidget *widget, NimFontChooser *chooser)
{
  GtkWidget *dialog;
  GtkWidget *preview_widget;
  GtkFileFilter *filter;
  gint response;
  NimFontChooserPrivate *priv;

  priv = chooser->priv;
  dialog = gtk_file_chooser_dialog_new (
                                  "Choose font file",
                                  NULL,
                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                  GTK_STOCK_CANCEL,
                                  GTK_RESPONSE_CANCEL,
                                  GTK_STOCK_OK,
                                  GTK_RESPONSE_OK,
                                  NULL);
  preview_widget = gtk_image_new ();
  filter = gtk_file_filter_new ();
  gtk_file_filter_add_mime_type (filter, "application/x-font-ttf");
  gtk_file_filter_add_pattern (filter, "*.ttf");
  gtk_file_filter_set_name (filter, "Files of fonts");
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (dialog), preview_widget);
  gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (dialog), FALSE);
  g_signal_connect (dialog, "update-preview", G_CALLBACK (file_chooser_update_preview_cb), chooser);
  gtk_widget_show_all (preview_widget);
  gtk_widget_show_all (dialog);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK) {
    gchar *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    if (!filename || !nim_font_chooser_append (chooser, filename, NULL, TRUE))
        nim_font_chooser_set_active (chooser, priv->last_active_row);

    if (filename)
      g_free (filename);
  }
  
  gtk_widget_hide (dialog);
  gtk_widget_destroy (dialog);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gint nim_font_chooser_list_length (NimFontChooser *chooser)
{
  gint response = 0;
  GtkTreeIter iter;
  NimFontChooserPrivate *priv;

  priv = chooser->priv;

  if (gtk_tree_model_get_iter_first (priv->treemodel, &iter))
  {
    do {
        response++;
    } while (gtk_tree_model_iter_next (priv->treemodel, &iter));
  }
  
  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_find_similar (NimFontChooser *chooser, const gchar *pattern, GtkTreeIter *iter)
{
  NimFontChooserPrivate *priv;
  gboolean response = FALSE;
  GtkTreeIter temp_iter;

  priv = chooser->priv;

  if (gtk_tree_model_get_iter_first (priv->treemodel, &temp_iter))
  {
    do {
      gchar *value = NULL;
      gtk_tree_model_get (priv->treemodel, &temp_iter, COLUMN_DATA, &value, -1);

      if (g_strcmp0 (value, pattern) == 0)
      {
          response = TRUE;

          if (iter)
            *iter = temp_iter;
      }

      if (value)
        g_free (value);
        
    } while (!response && gtk_tree_model_iter_next (priv->treemodel, &temp_iter));
  }

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_append (NimFontChooser *chooser,
                                         const gchar *new_font,
                                         GtkTreeIter *iter,
                                         gboolean set_active)
{
  gboolean response = FALSE;
  GtkTreeIter new_iter;
  NimFontChooserPrivate *priv;
  gint length;
  gchar *text;

  priv = chooser->priv;
  text = g_path_get_basename (new_font);
  length = nim_font_chooser_list_length (chooser);

  if (nim_font_chooser_find_similar (chooser, text, &new_iter)) {
    if (set_active) {
      response = set_active;
    }
  } else {
    GdkPixbuf *pixbuf = NULL;
    GdkRGBA rgba;
    GtkStyleContext *context;
    gchar *strcolor;

    context = gtk_widget_get_style_context (priv->fontbutton);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &rgba);
    strcolor = rgba_to_color (&rgba);
    strcolor = strcolor != NULL ? strcolor : g_strdup ("#000000ff");
    nim_imaging_make_font_preview (&pixbuf, new_font,
                                                PREVIEW_SIZE,
                                                PREVIEW_SIZE + ceil (PREVIEW_SIZE / 2.0),
                                                PREVIEW_SIZE,
                                                strcolor,
                                                "#00000000",
                                                "Aa", 0.0);
    g_free (strcolor);

    if (pixbuf)
    {
      gtk_list_store_append (GTK_LIST_STORE (priv->treemodel), &new_iter);
      gtk_list_store_set (GTK_LIST_STORE (priv->treemodel), &new_iter,
                                          COLUMN_PIXBUF, pixbuf,
                                          COLUMN_TEXT, text,
                                          COLUMN_DATA, new_font,
                                          -1);
      g_object_unref (G_OBJECT (pixbuf));
      response = TRUE;
    }

  }

  if (text)
    g_free (text);

  if (response && iter)
    *iter = new_iter;

  if (length == 0)
    nim_font_chooser_set_active (chooser, -1);
  else if (response && set_active)
    nim_font_chooser_set_active_iter (chooser, &new_iter);
  else
    nim_font_chooser_set_active (chooser, priv->last_active_row);

  if (iter)
    *iter = new_iter;
    
  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_read_fonts (NimFontChooser *chooser)
{
  NimFontChooserPrivate *priv;
  gchar **fonts;
  size_t n, n_elem, n_ok;

  priv = chooser->priv;
  fonts = MagickQueryFonts ("*", &n_elem);

  for (n = 0; n < n_elem; n++)
    if (nim_font_chooser_append (chooser, fonts [n], NULL, FALSE))
      n_ok++;

  if (n_ok > 0) {
    if (priv->font_name)
      nim_font_chooser_append (chooser, priv->font_name, NULL, TRUE);
    else
      nim_font_chooser_set_active (chooser, 0);
  }

  priv->list_complete = TRUE;
  if (priv->font_name)
    g_free (priv->font_name);

  priv->font_name = NULL;
  
  if (n_elem > 0)
    g_free (fonts);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_font_button_toggled (GtkWidget *widget, NimFontChooser *chooser)
{
  NimFontChooserPrivate *priv;
  gboolean active;
  priv = chooser->priv;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->fontbutton));

  if (active) {
    nim_font_chooser_popup (chooser);
    gtk_widget_set_sensitive (widget, FALSE);
  } else if (gtk_widget_get_visible (priv->popup_window))
    nim_font_chooser_popdown (chooser);

}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void cs_dialog_response (GtkWidget *dialog, gint response, NimFontChooser *chooser)
{
  NimFontChooserPrivate *priv;

  priv = chooser->priv;
  gtk_widget_hide (dialog);
  
  if (response == GTK_RESPONSE_OK)
  {
    gchar *color;
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (dialog), &priv->foreground);
    gtk_widget_queue_draw (priv->image_color);
    color = rgba_to_color (&priv->foreground);
    g_signal_emit (chooser, signals [SIGNAL_COLOR_SET], 0, color, NULL);
    g_free (color);
  }

  gtk_widget_destroy (dialog);
  priv->cs_dialog = NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void cs_dialog_destroy (GtkWidget *widget, NimFontChooser *chooser)
{
  chooser->priv->cs_dialog = NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_open_colorchooser (GtkWidget *widget, NimFontChooser *chooser)
{
  GtkWidget *toplevel = NULL;
  NimFontChooserPrivate *priv;
  gint response;
  priv = chooser->priv;

  if (!priv->cs_dialog) {
    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (chooser));

    priv->cs_dialog = gtk_color_chooser_dialog_new ("Choose font color", NULL);

    if (gtk_widget_is_toplevel (toplevel) && GTK_IS_WINDOW (toplevel)) {
      if (GTK_WINDOW (toplevel) != gtk_window_get_transient_for (GTK_WINDOW (priv->cs_dialog)))
        gtk_window_set_transient_for (GTK_WINDOW (priv->cs_dialog), GTK_WINDOW (toplevel));

      gtk_window_set_modal (GTK_WINDOW (priv->cs_dialog), gtk_window_get_modal (GTK_WINDOW (toplevel)));
    }

    g_signal_connect (priv->cs_dialog, "response", G_CALLBACK (cs_dialog_response), chooser);
    g_signal_connect (priv->cs_dialog, "destroy", G_CALLBACK (cs_dialog_destroy), chooser);
  }

  gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (priv->cs_dialog), TRUE);
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->cs_dialog), &priv->foreground);
  gtk_window_present (GTK_WINDOW (priv->cs_dialog));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_size_changed (GtkWidget *widget, NimFontChooser *chooser)
{
  gint value;
  value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
  g_signal_emit (chooser, signals [SIGNAL_SIZE_CHANGED], 0, value, NULL);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_font_chooser_set_font_name (NimFontChooser *chooser, const gchar *fontname)
{
  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser) && fontname != NULL);

  if (chooser->priv->list_complete)
    nim_font_chooser_append (chooser, fontname, NULL, TRUE);
  else
    chooser->priv->font_name = g_strdup (fontname);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *nim_font_chooser_get_font_name (NimFontChooser *chooser)
{
  gchar *result = NULL;
  GtkTreeIter iter;
  NimFontChooserPrivate *priv;
  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), NULL);
  priv = chooser->priv;

  if (nim_font_chooser_get_active_iter (chooser, &iter)) 
    gtk_tree_model_get (priv->treemodel, &iter, COLUMN_DATA, &result, -1);

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_font_chooser_set_font_size (NimFontChooser *chooser, gint fontsize)
{
  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (chooser->priv->spin_size), fontsize);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint nim_font_chooser_get_font_size (NimFontChooser *chooser)
{
  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), -1);
  return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (chooser->priv->spin_size));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_font_chooser_set_foreground (NimFontChooser *chooser, const gchar *foreground)
{
  NimFontChooserPrivate *priv;
  
  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));

  priv = chooser->priv;
  
  if (color_to_rgba (&priv->foreground, foreground))
    gtk_widget_queue_draw (priv->image_color);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *nim_font_chooser_get_foreground (NimFontChooser *chooser)
{
  gchar *result;
  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), NULL);
  result = rgba_to_color (&chooser->priv->foreground);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_set_active (NimFontChooser *chooser, gint new_active)
{
  GtkTreePath *path = NULL;
  NimFontChooserPrivate *priv;

  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));
  g_return_if_fail (new_active >= -1);

  priv = chooser->priv;

  if (priv->treemodel == NULL)
  {
    priv->last_active_row = new_active;
    if (new_active != -1)
      return;
  }

  if (new_active != -1)
    path = gtk_tree_path_new_from_indices (new_active, -1);

  nim_font_chooser_set_active_internal (chooser, path);

  if (path)
    gtk_tree_path_free (path);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_set_active_internal (NimFontChooser *chooser, GtkTreePath *path)
{
  NimFontChooserPrivate *priv;
  GtkTreePath *active_path;
  gint path_cmp;
  gboolean is_valid_row_reference;

  priv = chooser->priv;
  is_valid_row_reference = gtk_tree_row_reference_valid (priv->active_path);

  if (path && is_valid_row_reference) {
    active_path = gtk_tree_row_reference_get_path (priv->active_path);
    path_cmp = gtk_tree_path_compare (path, active_path);
    gtk_tree_path_free (active_path);
    if (path_cmp == 0)
      return;
  }

  if (priv->active_path) {
      gtk_tree_row_reference_free (priv->active_path);
      priv->active_path = NULL;
  }

  if (!path) {
    gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview)));

    if (!is_valid_row_reference)
      return;
  } else {
      priv->active_path = gtk_tree_row_reference_new (priv->treemodel, path);
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), path, NULL, FALSE);
  }

  nim_font_chooser_set_diaplayed_path (chooser, path);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_set_active_iter (NimFontChooser *chooser, GtkTreeIter *iter)
{
  GtkTreePath *path = NULL;

  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));

  if (iter)
    path = gtk_tree_model_get_path (chooser->priv->treemodel, iter);

  nim_font_chooser_set_active_internal (chooser, path);
  gtk_tree_path_free (path);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_get_active_iter (NimFontChooser *chooser, GtkTreeIter *iter)
{
  GtkTreePath *path;
  gboolean result;

  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), FALSE);

  if (!gtk_tree_row_reference_valid (chooser->priv->active_path))
    return FALSE;

  path = gtk_tree_row_reference_get_path (chooser->priv->active_path);
  result = gtk_tree_model_get_iter (chooser->priv->treemodel, iter, path);
  gtk_tree_path_free (path);

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static cairo_pattern_t* get_checkered_pattern (void)
{
  static unsigned char data[8] = { 0xFF, 0x00, 0x00, 0x00,
                                   0x00, 0xFF, 0x00, 0x00 };
  static cairo_surface_t *checkered = NULL;
  cairo_pattern_t *pattern;

  if (checkered == NULL)
    checkered = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_A8, 2, 2, 4);

  pattern = cairo_pattern_create_for_surface (checkered);
  cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
  cairo_pattern_set_filter (pattern, CAIRO_FILTER_NEAREST);

  return pattern;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean image_color_draw_cb (GtkWidget *widget, cairo_t *cr, NimFontChooser *chooser)
{
  cairo_pattern_t *pattern = NULL;

  cairo_set_source_rgb (cr, CHECK_DARK, CHECK_DARK, CHECK_DARK);
  cairo_paint (cr);

  cairo_set_source_rgb (cr, CHECK_LIGHT, CHECK_LIGHT, CHECK_LIGHT);
  cairo_scale (cr, CHECK_SIZE, CHECK_SIZE);

  pattern = get_checkered_pattern ();
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);

  gdk_cairo_set_source_rgba (cr, &chooser->priv->foreground);
  cairo_paint (cr);

  if (!gtk_widget_is_sensitive (widget)) {
    GdkRGBA color;
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_INSENSITIVE, &color);

    gdk_cairo_set_source_rgba (cr, &color);
    pattern = get_checkered_pattern ();
    cairo_mask (cr, pattern);
    cairo_pattern_destroy (pattern);
  }

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_set_diaplayed_path (NimFontChooser *chooser, GtkTreePath *path)
{
  NimFontChooserPrivate *priv;
  GdkPixbuf *pixbuf = NULL;
  gchar *text = NULL;
  GtkTreeIter iter;
  
  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));
  priv = chooser->priv;

  if (path && gtk_tree_model_get_iter (priv->treemodel, &iter, path)) {
    GdkRGBA rgba;
    GtkStyleContext *context;
    gchar *strcolor;

    gtk_tree_model_get (priv->treemodel, &iter, COLUMN_DATA, &text, -1);
    context = gtk_widget_get_style_context (priv->fontbutton);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &rgba);
    strcolor = rgba_to_color (&rgba);
    strcolor = strcolor != NULL ? strcolor : g_strdup ("#000000ff");
    nim_imaging_make_font_preview (&pixbuf, text,
                                                PREVIEW_SIZE,
                                                PREVIEW_SIZE + ceil (PREVIEW_SIZE / 2.0),
                                                PREVIEW_SIZE,
                                                strcolor,
                                                "#00000000",
                                                "Aa", 0.0);
    g_free (strcolor);
  }

  if (pixbuf == NULL) {
    pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, PREVIEW_SIZE, PREVIEW_SIZE);
    gdk_pixbuf_fill (pixbuf, 0xffffffff);
  }

  if (pixbuf)
    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image_font), pixbuf);

  gtk_widget_set_tooltip_text (priv->fontbutton, text);
  g_signal_emit (chooser, signals [SIGNAL_FONT_SET], 0, text, NULL);
  
  if (GDK_IS_PIXBUF (pixbuf))
    g_object_unref (G_OBJECT (pixbuf));

  if (text)
    g_free (text);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GtkTreePath *nim_font_chooser_get_diaplayed_path (NimFontChooser *chooser)
{
  GtkTreePath *path = NULL;
  NimFontChooserPrivate *priv;

  priv = chooser->priv;
  if (priv->active_path)
    path = gtk_tree_row_reference_get_path (priv->active_path);

  return path;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean _nim_font_chooser_popdown (NimFontChooser *chooser)
{
  NimFontChooserPrivate *priv;
  GtkWidget *toplevel;
  guint32 time;

  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), FALSE);

  priv = chooser->priv;
  time = gtk_get_current_event_time ();
  
  if (!gtk_widget_get_realized (GTK_WIDGET (chooser)))
    return;

  if (priv->grab_pointer != NULL) {
    gtk_device_grab_remove (priv->popup_window, priv->grab_pointer);
//    gdk_device_ungrab (priv->grab_pointer, time);
  } //else
  if (priv->grab_keyboard != NULL) {
    gtk_device_grab_remove (priv->popup_window, priv->grab_keyboard);
//    gdk_device_ungrab (priv->grab_pointer, time);
  }


  priv->grab_pointer = NULL;
  priv->grab_keyboard = NULL;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (priv->popup_window));

  if (GTK_IS_WINDOW (toplevel)) {
    GtkWindowGroup *group;

    group = gtk_window_get_group (GTK_WINDOW (toplevel));
    gtk_window_group_remove_window (group, GTK_WINDOW (priv->popup_window));
    gtk_window_set_transient_for (GTK_WINDOW (priv->popup_window), NULL);
  }
  gtk_widget_hide (priv->popup_window);

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->fontbutton)))
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->fontbutton), FALSE);

  gtk_widget_set_sensitive (priv->fontbutton, TRUE);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean _nim_font_chooser_popup (NimFontChooser *chooser)
{
  GdkDevice *device;

  g_return_val_if_fail (NIM_IS_FONT_CHOOSER (chooser), FALSE);

  device = gtk_get_current_event_device ();

  if (!device) {
    GdkDisplay *display;
    GdkDeviceManager *device_manager;
    GList *devices;

    display = gtk_widget_get_display (GTK_WIDGET (chooser));
    device_manager = gdk_display_get_device_manager (display);
    devices = gdk_device_manager_list_devices (device_manager, GDK_DEVICE_TYPE_MASTER);
    device = devices->data;
  }

  if (device)
    nim_font_chooser_popup_for_device (chooser, device);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_popup (NimFontChooser *chooser)
{
  gdk_threads_add_idle ((GSourceFunc) _nim_font_chooser_popup, chooser);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_popdown (NimFontChooser *chooser)
{
  gdk_threads_add_idle ((GSourceFunc) _nim_font_chooser_popdown, chooser);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean popup_window_key_press_event_cb (GtkWidget *widget,
                                                 GdkEventKey *event,
                                                 NimFontChooser *chooser)
{
  switch (event->keyval) {
    case GDK_KEY_Escape:
    case GDK_KEY_Cancel:
      nim_font_chooser_popdown (chooser);
      return TRUE;
    default:
      break;
  }

  if (!gtk_bindings_activate_event (G_OBJECT (chooser), event))
    gtk_bindings_activate_event (G_OBJECT (chooser), event);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean popup_window_button_press_event_cb (GtkWidget      *widget,
                                                    GdkEventButton *event,
                                                    NimFontChooser *chooser)
{
  NimFontChooserPrivate *priv;

  priv = chooser->priv;

  if (priv->grab_pointer != NULL || priv->grab_keyboard != NULL) {
    int root_x, root_y, width, height;

    gtk_window_get_position (GTK_WINDOW (priv->popup_window), &root_x, &root_y);
    gtk_widget_get_size_request (priv->popup_window, &width, &height);

    if (root_x < event->x_root
                && root_y < event->y_root
                && root_x + width > event->x_root
                && root_y + height > event->y_root)
    {
                
      GtkTreeIter iter;
      GtkTreeModel *treemodel;
      GtkTreeSelection *selection;

      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
      
      if (gtk_tree_selection_get_selected (selection, &treemodel, &iter))
          nim_font_chooser_set_active_iter (chooser, &iter);

      nim_font_chooser_popdown (chooser);
      return TRUE;
    } else {
      nim_font_chooser_popdown (chooser);
      return FALSE;
    }
  }
  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean tree_view_button_release_event_cb (GtkTreeView *tree,
                                      GdkEventButton *event,
                                      NimFontChooser *chooser)
{

  if (event->button == 1) {
    GtkTreeIter iter;
    GtkTreeModel *model = NULL;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (tree);

    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
      nim_font_chooser_set_active_iter (chooser, &iter);
      nim_font_chooser_popdown (chooser);
      return TRUE;
    }
  }
  
  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_list_position (NimFontChooser *chooser,
                                         gint *x, gint *y,
                                         gint *width, gint *height)
{
    GdkScreen *screen;
    GdkWindow *window;
    GdkRectangle monitor;
    GtkAllocation allocation;
    GtkRequisition popup_req;
    GtkScrolledWindow *scrolled;
    gint monitor_num, minimal;
    GtkPolicyType hpolicy, vpolicy;
    NimFontChooserPrivate *priv;
    GtkWidget *widget;

    gint real_x, real_y, real_w, real_h;

    g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser));

    priv = chooser->priv;
    widget = GTK_WIDGET (chooser);
    scrolled = GTK_SCROLLED_WINDOW (priv->scrolled);

    gtk_widget_get_allocation (widget, &allocation);
    window = gtk_widget_get_window (widget);

    gdk_window_get_origin (window, &real_x, &real_y);

    real_x += allocation.x;
    real_y += allocation.y;
    real_w = allocation.width;
    real_h = allocation.height;

    gtk_widget_get_preferred_size (widget, NULL, &popup_req);

    screen = gtk_widget_get_screen (widget);
    monitor_num = gdk_screen_get_monitor_at_window (screen, window);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    popup_req.height = PREVIEW_SIZE * nim_font_chooser_list_length (chooser);
    real_h = MIN (PREVIEW_SIZE * VISIBLE_ROWS, monitor.height);

    if (real_y + real_h > monitor.height)
      real_y -= (real_y + real_h - monitor.height);

    if (real_x + real_w > monitor.width)
      real_x -= (real_x + real_w - monitor.width);

    real_y = MAX (real_y, monitor.y);
    real_x = MAX (real_x, monitor.x);
    vpolicy = (popup_req.height > real_h) ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER;

    *x = real_x;
    *y = real_y;
    *width = real_w;
    *height = real_h;

    hpolicy = GTK_POLICY_AUTOMATIC;
    gtk_scrolled_window_set_policy (scrolled, hpolicy, vpolicy);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean popup_grab_on_window (NimFontChooser *chooser,
                                      GdkWindow *window,
                                      GdkDevice *grab_keyboard,
                                      GdkDevice *grab_pointer,
                                      guint32 grab_time)
{

    if (grab_keyboard != NULL && gdk_device_grab (grab_keyboard,
                                            window,
                                            GDK_OWNERSHIP_WINDOW,
                                            TRUE, 
                                            GDK_KEY_PRESS_MASK |
                                            GDK_KEY_RELEASE_MASK,
                                            NULL,
                                            grab_time) != GDK_GRAB_SUCCESS)
        return FALSE;

    if (grab_pointer != NULL && gdk_device_grab (grab_pointer,
                                            window,
                                            GDK_OWNERSHIP_WINDOW,
                                            TRUE,
                                            GDK_BUTTON_PRESS_MASK |
                                            GDK_BUTTON_RELEASE_MASK |
                                            GDK_ENTER_NOTIFY_MASK |
                                            GDK_LEAVE_NOTIFY_MASK |
                                            GDK_POINTER_MOTION_MASK,
                                            NULL,
                                            grab_time) != GDK_GRAB_SUCCESS) {
        if (grab_keyboard != NULL)
            gdk_device_ungrab (grab_keyboard, grab_time);

        return FALSE;
    }

    return TRUE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_popup_for_device (NimFontChooser *chooser, GdkDevice *device)
{
  gint x, y, width, height;
  GtkWidget *toplevel;
  GdkDevice *grab_keyboard;
  GdkDevice *grab_pointer;
  guint32 grab_time;
  GtkTreePath *path = NULL;
  NimFontChooserPrivate *priv;

  g_return_if_fail (NIM_IS_FONT_CHOOSER (chooser) && GDK_IS_DEVICE (device));

  if (!gtk_widget_get_realized (GTK_WIDGET (chooser)))
      return;

  priv = chooser->priv;

  if (priv->grab_pointer != NULL && priv->grab_keyboard != NULL)
    return;

  grab_time = gtk_get_current_event_time ();

  if (gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD) {
    grab_keyboard = device;
    grab_pointer = gdk_device_get_associated_device (device);
  } else {
    grab_pointer = device;
    grab_keyboard = gdk_device_get_associated_device (device);
  }

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (chooser));

  if (GTK_IS_WINDOW (toplevel))
  {
    GtkWindowGroup *group;
    GtkWindow *toplevel_window;
    toplevel_window = GTK_WINDOW (toplevel);
    group = gtk_window_get_group (toplevel_window);
    gtk_window_group_add_window (group, GTK_WINDOW (priv->popup_window));
    gtk_window_set_transient_for (GTK_WINDOW (priv->popup_window), toplevel_window);
  }

  gtk_widget_show_all (priv->scrolled);
  nim_font_chooser_list_position (chooser, &x, &y, &width, &height);
  gtk_widget_set_size_request (priv->popup_window, width, height);
  gtk_window_move (GTK_WINDOW (priv->popup_window), x, y);
  path = nim_font_chooser_get_diaplayed_path (chooser);

  if (path != NULL)
    gtk_tree_view_expand_to_path (GTK_TREE_VIEW (priv->treeview), path);

  gtk_tree_view_set_hover_expand (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_widget_show_all (priv->popup_window);

  if (path != NULL)
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (priv->treeview), path, NULL, FALSE);

  gtk_widget_grab_focus (priv->popup_window);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->fontbutton), TRUE);

  if (path)
    gtk_tree_path_free (path);

  if (!gtk_widget_has_focus (priv->treeview))
    gtk_widget_grab_focus (priv->treeview);

  if (!popup_grab_on_window (chooser,
                            gtk_widget_get_window (priv->popup_window),
                            grab_keyboard,
                            grab_pointer,
                            grab_time))
  {
    gtk_widget_hide (priv->popup_window);
    return;
  }

  gtk_device_grab_add (priv->popup_window, grab_pointer, TRUE);
  priv->grab_pointer = grab_pointer;
  priv->grab_keyboard = grab_keyboard;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
