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

#include <stdio.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <math.h>
#include "nimimaging.h"


gboolean  nim_imaging_convert_to_gif (gchar **filelist, int n_elem, gint delay, gboolean combine)
{
  gboolean response = FALSE;
  int n = 0;
  MagickWand *result = NULL;

  MagickWandGenesis ();

  result = NewMagickWand ();

  for (n = 0; n < n_elem; n++) {
    MagickWand *wand = NewMagickWand ();

    if (MagickReadImage (wand, filelist [n]) == MagickTrue) {
      MagickAddImage (result, wand);
      MagickSetOption (result, "loop", "1");
      MagickSetImageDelay (result, delay);
      response = TRUE;
    }

    DestroyMagickWand (wand);
  }

  if (response)
    MagickWriteImages (result, "result.gif", MagickTrue);

  result = DestroyMagickWand (result);
  
  MagickWandTerminus ();

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*
static void window_destroy_cb (GtkWidget *widget)
{
  gtk_main_quit ();
}
*/
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GdkPixbuf *nim_imaging_convert_wand_to_pixbuf (MagickWand *wand)
{
  GdkPixbuf *pixbuf = NULL;
  gint width;
  gint height;
  gint rowstride;
  gint row;
  guchar *pixels;
  gboolean result = FALSE;

  if (IsMagickWand (wand) == MagickTrue)
  {
    width = MagickGetImageWidth (wand);
    height = MagickGetImageHeight (wand);
    pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    pixels = gdk_pixbuf_get_pixels (pixbuf);
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    MagickSetImageDepth (wand, 8);

    for (row = 0; row < height; row++)
    {
        guchar *data = pixels + row * rowstride;
        if (MagickExportImagePixels (wand, 0, row, width, 1, "RGBA", CharPixel, data) == MagickTrue)
          result = TRUE;
    }

    if (!result)
    {
      g_object_unref (G_OBJECT (pixbuf));
      pixbuf = NULL;
    }
  }

  return pixbuf;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/*
static void show_image (MagickWand *wand)
{
  GdkPixbuf *pixbuf;
  GtkWidget *image;
  GtkWidget *window;

  if ((pixbuf = nim_imaging_convert_wand_to_pixbuf (wand)) != NULL)
  {
    image = gtk_image_new_from_pixbuf (pixbuf);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_add (GTK_CONTAINER (window), image);
    gtk_widget_show_all (window);
    g_signal_connect (window, "destroy", G_CALLBACK (window_destroy_cb), NULL);

    g_object_unref (G_OBJECT (pixbuf));
    gtk_main ();
  }
}
*/
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define CLAMP_RADIUS(w, h, r) ceil (((MIN (w, h) / 2) < ABS (r) ? MIN (w, h) / 2.0 : ABS (r)))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_round_corners (gchar *filename, const gdouble corners [NIM_CORNER_LAST])
{
  gboolean response = FALSE;
  MagickWand *m_wand = NULL;
  MagickWand *l_wand = NULL;
  PixelWand *p_wand = NULL;
  DrawingWand *d_wand = NULL;
  gdouble width, height;
  gdouble x, y;
  gdouble ctl, cbl, ctr, cbr;

  m_wand = NewMagickWand ();
  l_wand = NewMagickWand ();
  p_wand = NewPixelWand ();
  d_wand = NewDrawingWand ();

  if (MagickReadImage (l_wand, filename) == MagickTrue)
  {
    width = MagickGetImageWidth (l_wand) - 1;
    height =  MagickGetImageHeight (l_wand) - 1;

    if (width > 1 && height > 1)
    {
      ctl = CLAMP_RADIUS (width, height, corners [NIM_CORNER_TL]);
      cbl = CLAMP_RADIUS (width, height, corners [NIM_CORNER_BL]);
      ctr = CLAMP_RADIUS (width, height, corners [NIM_CORNER_TR]);
      cbr = CLAMP_RADIUS (width, height, corners [NIM_CORNER_BR]);

      PixelSetColor (p_wand, "none");
      MagickNewImage (m_wand, width + 2, height + 2, p_wand);

      PixelSetColor (p_wand, "white");
      DrawSetFillColor (d_wand, p_wand);

      DrawPathStart (d_wand);
      DrawPathMoveToAbsolute (d_wand, ctl, 0);
      DrawPathLineToAbsolute (d_wand, width - ctr, 0.0);
      x = corners [NIM_CORNER_TR] >= 0 ? width : width - ctr;
      y = corners [NIM_CORNER_TR] >= 0 ? 0.0 : ctr;

      DrawPathCurveToQuadraticBezierAbsolute (d_wand, x, y, width, ctr);
      DrawPathLineToAbsolute (d_wand, width, height - cbr);
      x = corners [NIM_CORNER_BR] >= 0 ? width : width - cbr;
      y = corners [NIM_CORNER_BR] >= 0 ? height : height - cbr;

      DrawPathCurveToQuadraticBezierAbsolute (d_wand, x, y, width - cbr, height);
      DrawPathLineToAbsolute (d_wand, cbl, height);
      x = corners [NIM_CORNER_BL] >= 0 ? 0.0 : cbl;
      y = corners [NIM_CORNER_BL] >= 0 ? height : height - cbl;
    
      DrawPathCurveToQuadraticBezierAbsolute (d_wand, x, y, 0.0, height - cbl);
      DrawPathLineToAbsolute (d_wand, 0.0, ctl);
      x = corners [NIM_CORNER_TL] >= 0 ? 0.0 : ctl;
      y = corners [NIM_CORNER_TL] >= 0 ? 0.0 : ctl;
    
      DrawPathCurveToQuadraticBezierAbsolute (d_wand, x, y, ctl, 0.0);
      DrawPathClose (d_wand);
      DrawPathFinish (d_wand);

      MagickDrawImage (m_wand, d_wand);
      response = MagickCompositeImage (m_wand, l_wand, InCompositeOp, 0, 0) == MagickTrue;
    }
  }
  
  if (l_wand)
    DestroyMagickWand (l_wand);

  if (d_wand)
    DestroyDrawingWand (d_wand);

  if (p_wand)
    DestroyPixelWand (p_wand);

  if (!response && m_wand) {
    DestroyMagickWand (m_wand);
    m_wand = NULL;
  }

  return m_wand;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* nim_imaging_get_path_to_test_image (int tp)
{
  gchar *result = NULL;
  const gchar *dirname;
  const gchar *const *datadirs;
  const gchar *target;
  gint n;

  if (tp == 0)
    target = "test-image.png";
  else
    target = "common.ui";
    
  datadirs = g_get_system_data_dirs ();
  for (n = 0; ; n++)
  {
    dirname = datadirs [n];

    if (dirname == NULL)
      break;
      
    result = g_build_filename (G_DIR_SEPARATOR_S, dirname, "nautilus-imaging", target, NULL);

    if (g_file_test (result, G_FILE_TEST_EXISTS)) {
      break;
    } else {
      g_free (result);
      result = NULL;
    }
  }

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//int main(int argc, char **argv)
//{
//  MagickWand *wand;
//  gdouble corners [NIM_CORNER_LAST] = {-30.0, 15.0, -30.0, 15.0};

//  MagickWandGenesis ();

//  gtk_init (&argc, &argv);

//  if ((wand = nim_imaging_round_corners (argv [1], corners)) != NULL)
//  {
//    show_image (wand);
//    wand = DestroyMagickWand (wand);
////      MagickWriteImage (m_wand, "mask_result.png");
//  }

//  MagickWandTerminus ();

//  return 0;
//}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
