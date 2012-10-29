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
#include "nimimaging.h"
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define PREVIEW_SIZE                  25
#define FILE_CHOOSER_PREVIEW_WIDTH    250
#define FILE_CHOOSER_PREVIEW_HEIGHT   150
#define SEPARATOR_ID                  "<separator>"
#define LAST_ROW                      "Choose file"
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
  GtkWidget *button_font;  // Toggle button to open popup window
  GtkWidget *spin_size;    // Spin button for font size
  GtkWidget *window_popup; // Popup window for menu of font selector
  GtkWidget *treeview;     // Tree of menu
  GtkWidget *image_font;   // Image of font preview
  GtkWidget *image_color;  // Image of foreground color
  GtkTreeModel *treemodel: // Model of the tree view
  GtkTreePath  *selected_path; // Current selected path
  GSList   *exists_other;   // LIst of custom added files
  gint      n_elem;        // Num of the elements on tree view
  gint      last_active_row;
  gchar    *foreground;    // String format of the font color
  gchar    *background;    // String format of the background
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
static void nim_font_shooser_open_filechooser (NimFontChooser *chooser);
static gint nim_font_chooser_list_length (NimFontChooser *chooser);
static gboolean nim_font_chooser_find_similar (NimFontChooser *chooser, const gchar *pattern, GtkTreeIter *iter);
static gboolean nim_font_chooser_append (NimFontChooser *chooser,
                                         const gchar    *new_font,
                                         GtkTreeIter    *iter,
                                         gboolean       set_active);
static gboolean nim_font_chooser_read_fonts (NimFontChooser *chooser);
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
  GtkWidget *scrolled;
  GtkWidget *colorbutton;
  GtkSizeGroup *sizegroup;
  GdkPixbuf *pixbuf;
  
  this->priv = G_TYPE_INSTANCE_GET_PRIVATE (this, NIM_TYPE_FONT_CHOOSER, NimFontChooserPrivate);
  priv = this->priv;

  priv->n_elem = 0;
  priv->last_active_row = 0;
  priv->foreground = NULL;
  priv->background = NULL;
  priv->exists_other = NULL;
  
  priv->treemodel = gtk_list_store_new (COLUMN_LAST, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
  priv->treeview = gtk_tree_view_new_with_model (priv->model);
  column = gtk_tree_view_column_new ();
  celltext = gtk_cell_renderer_text_new ();
  cellpixbuf = gtk_cell_renderer_pixbuf_new ();
  scrolled = gtk_scrolled_window_new (NULL, NULL);
  priv->window_popup = gtk_window_new (GTK_WINDOW_POPUP);

  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), cellpixbuf, FALSE);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (column), celltext, TRUE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->treeview), FALSE);
  gtk_tree_view_set_hover_selection (GTK_TREE_VIEW (priv->treeview), TRUE);
  gtk_container_add (GTK_CONTAINER (scrolled), priv->treeview);
  gtk_container_add (GTK_CONTAINER (priv->popup), scrolled);

  priv->image_font = gtk_image_new ();
  priv->image_color = gtk_image_new ();
  priv->button_font = gtk_toggle_button_new ();
  adjustment = gtk_adjustment_new (26, 8, 1000, 1, 10, 0);
  priv->spin_size = gtk_spin_button_new (adjustment, 0, 0);
  sizegroup = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

  gtk_container_add (GTK_CONTAINER (priv->
  gtk_box_pack_start (GTK_BOX (box), priv->foreground, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (box), priv->fontsize, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (this), priv->fontname, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (this), box, FALSE, FALSE, 0);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (this), GTK_ORIENTATION_HORIZONTAL);
  gtk_box_set_spacing (GTK_BOX (this), 5);

  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column),
                                    cellpixbuf,
                                    "pixbuf",
                                    COLUMN_PIXBUF,
                                    NULL);

  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (column),
                                    celltext,
                                    "text",
                                    COLUMN_TEXT,
                                    NULL);

  g_object_set (G_OBJECT (celltext), "ellipsize",
                                    PANGO_ELLIPSIZE_END,
                                    "ellipsize-set",
                                    TRUE,
                                    NULL);
                                    
  gtk_widget_show_all (GTK_WIDGET (this));

  
  g_signal_connect (priv->fontname, "changed", G_CALLBACK (nim_font_chooser_fontname_changed), this);
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (priv->fontname),
              (GtkTreeViewRowSeparatorFunc) nim_font_chooser_row_separator_func,
              this,
              NULL);
  g_idle_add ((GSourceFunc) nim_font_chooser_read_fonts,  this);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GtkWidget* nim_font_chooser_new (void)
{
  return g_object_new (NIM_TYPE_FONT_CHOOSER, NULL);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//static gboolean nim_font_chooser_find (NimFontChooser *chooser, 
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_row_separator_func (GtkTreeModel *model,
                                                     GtkTreeIter *iter,
                                                     NimFontChooser *chooser)
{
  gchar *value = NULL;
  gboolean response = FALSE;

  gtk_tree_model_get (model, iter, COLUMN_TEXT, &value, -1);
  response = g_strcmp0 (SEPARATOR_ID, value) == 0;

  if (value)
    g_free (value);

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_chooser_fontname_changed (GtkComboBox *combo, NimFontChooser *this)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  NimFontChooserPrivate *priv;

  priv = this->priv;
  model = gtk_combo_box_get_model (combo);

  if (gtk_combo_box_get_active_iter (combo, &iter)) {
    gchar *value = NULL;
    gtk_tree_model_get (model, &iter, COLUMN_TEXT, &value, -1);

    if (g_strcmp0 (value, LAST_ROW) == 0) {
      nim_font_shooser_open_filechooser (this);
    } else {
      priv->last_active_row = gtk_combo_box_get_active (combo);
      g_signal_emit (this, signals [SIGNAL_FONT_SET], 0, value, NULL);
    }
  }
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
//                  FILE_CHOOSER_PREVIEW_HEIGHT),
                  FILE_CHOOSER_PREVIEW_WIDTH,
                  FILE_CHOOSER_PREVIEW_HEIGHT,
                  "#000000ff",
                  "#ffffffff",
                  fontname ? fontname : basename,
                  0.0))
//g_print ("%s:%s: %s\n", G_STRLOC, G_STRFUNC, GDK_IS_PIXBUF (pixbuf) ? "Yes" : "No");
  {

//  if (GDK_IS_PIXBUF (pixbuf)) {
//g_print ("%s:%s\n", G_STRLOC, G_STRFUNC);

    preview_widget = gtk_file_chooser_get_preview_widget (filechooser);
    gtk_image_clear (GTK_IMAGE (preview_widget));
    gtk_image_set_from_pixbuf (GTK_IMAGE (preview_widget), pixbuf);
    gtk_file_chooser_set_preview_widget_active (filechooser, TRUE);
    g_object_unref (G_OBJECT (pixbuf));
//  } else {
//    gtk_file_chooser_set_preview_widget_active (filechooser, FALSE);
  }

  if (filename)
    g_free (filename);

  if (basename)
    g_free (basename);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_font_shooser_open_filechooser (NimFontChooser *chooser)
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
        gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), priv->last_active_row);

    if (filename)
      g_free (filename);
  } else {
    gint length = nim_font_chooser_list_length (chooser);
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), length == 0 ? -1 : priv->last_active_row);
  }
  
  gtk_widget_hide (dialog);
  gtk_widget_destroy (dialog);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gint nim_font_chooser_list_length (NimFontChooser *chooser)
{
  gint response = 0;
  GtkTreeModel *model;
  GtkTreeIter iter = {0};
  NimFontChooserPrivate *priv;

  priv = chooser->priv;
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->fontname));

  if (gtk_tree_model_get_iter_first (model, &iter))
  {
    do {
      gchar *value = NULL;
      gtk_tree_model_get (model, &iter, COLUMN_TEXT, &value, -1);

      if (!value || (g_strcmp0 (value, SEPARATOR_ID) != 0 && g_strcmp0 (value, LAST_ROW) != 0))
        response++;

      if (value)
        g_free (value);
    } while (gtk_tree_model_iter_next (model, &iter));
  }
  
  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nim_font_chooser_find_similar (NimFontChooser *chooser, const gchar *pattern, GtkTreeIter *iter)
{
  GtkTreeModel *model;
  NimFontChooserPrivate *priv;
  gboolean response = FALSE;
  GtkTreeIter temp_iter = {0};

  priv = chooser->priv;
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->fontname));

  if (gtk_tree_model_get_iter_first (model, &temp_iter))
  {
    do {
      gchar *value = NULL;
      gtk_tree_model_get (model, &temp_iter, COLUMN_TEXT, &value, -1);

      if (g_strcmp0 (value, pattern) == 0)
      {
          response = TRUE;

          if (iter)
            *iter = temp_iter;
      }

      if (value)
        g_free (value);
        
    } while (!response && gtk_tree_model_iter_next (model, &temp_iter));
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
  gint length;
  GtkTreeModel *model;
  GtkTreeIter new_iter = {0};
  NimFontChooserPrivate *priv;
  gchar *text;

  priv = chooser->priv;
  text = g_path_get_basename (new_font);
  length = nim_font_chooser_list_length (chooser);

  if (g_strcmp0 (new_font, SEPARATOR_ID) == 0 || g_strcmp0 (new_font, LAST_ROW) == 0) {
    GtkTreeIter new_iter = {0};
    model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->fontname));
    gtk_list_store_append (GTK_LIST_STORE (model), &new_iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &new_iter,
                                        COLUMN_PIXBUF, NULL,
                                        COLUMN_TEXT, new_font,
                                        COLUMN_DATA, NULL, -1);
    response = TRUE;
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), -1);

  } else if (nim_font_chooser_find_similar (chooser, text, &new_iter)) {
    if (set_active) {
//      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->fontname), &new_iter);
      response = set_active;
    }
  } else {
    GdkPixbuf *pixbuf = NULL;

    nim_imaging_make_font_preview (&pixbuf,
                                            new_font,
                                            PREVIEW_SIZE,
                                            PREVIEW_SIZE,
                                            PREVIEW_SIZE,
                                            "#000000ff",
                                            "#ffffffff",
                                            "Aa",
                                            0.0);

      if (pixbuf)
      {
        model = gtk_combo_box_get_model (GTK_COMBO_BOX (priv->fontname));
        gtk_list_store_insert (GTK_LIST_STORE (model), &new_iter, length);
        gtk_list_store_set (GTK_LIST_STORE (model), &new_iter,
                                        COLUMN_PIXBUF, pixbuf,
                                        COLUMN_TEXT, text,
                                        COLUMN_DATA, new_font,
                                        -1);
        length++;
        
//        if (set_active)
//          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->fontname), &new_iter);

        g_object_unref (G_OBJECT (pixbuf));
        response = TRUE;
      }

    }

  if (text)
    g_free (text);

  if (response && iter)
    *iter = new_iter;

  if (length == 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), -1);
  else if (response && set_active)
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (priv->fontname), &new_iter);
  else
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), priv->last_active_row);
    
    
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

  nim_font_chooser_append (chooser, SEPARATOR_ID, NULL, FALSE);
  nim_font_chooser_append (chooser, LAST_ROW, NULL, FALSE);

  if (n_ok > 0)
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->fontname), 0);

  if (n_elem > 0)
    g_free (fonts);

  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
