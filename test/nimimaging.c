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
*/

#include <stdio.h>
#include <wand/magick_wand.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

enum {
    NIM_CORNER_TL,
    NIM_CORNER_TR,
    NIM_CORNER_BR,
    NIM_CORNER_BL,
    NIM_CORNER_LAST
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void window_destroy_cb (GtkWidget *widget)
{
  gtk_main_quit ();
}
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
void show_image (MagickWand *wand)
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
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define CLAMP_RADIUS(w, h, r) ((MIN (w, h) / 2) < ABS (r) ? MIN (w, h) / 2 : ABS (r))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_imaging_round_corners_from_wand (MagickWand **wand, const gdouble corners [NIM_CORNER_LAST])
{
  gboolean response = FALSE;
  MagickWand *result_wand = NULL;
  PixelWand *pixel_wand = NULL;
  DrawingWand *draw_wand = NULL;
  gdouble width, height;
  gdouble x, y;
  gdouble ctl, cbl, ctr, cbr;

  if (IsMagickWand (*wand) == MagickFalse) {
    return FALSE;
  }

  result_wand = NewMagickWand ();
  pixel_wand = NewPixelWand ();
  draw_wand = NewDrawingWand ();

  width = MagickGetImageWidth (*wand) - 1;
  height =  MagickGetImageHeight (*wand) - 1;

  if (width > 1 && height > 1)
  {
    ctl = CLAMP_RADIUS (width, height, corners [NIM_CORNER_TL]);
    cbl = CLAMP_RADIUS (width, height, corners [NIM_CORNER_BL]);
    ctr = CLAMP_RADIUS (width, height, corners [NIM_CORNER_TR]);
    cbr = CLAMP_RADIUS (width, height, corners [NIM_CORNER_BR]);

    PixelSetColor (pixel_wand, "none");
    MagickNewImage (result_wand, width + 2, height + 2, pixel_wand);

    PixelSetColor (pixel_wand, "white");
    DrawSetFillColor (draw_wand, pixel_wand);

    DrawPathStart (draw_wand);
    DrawPathMoveToAbsolute (draw_wand, ctl, 0);
    DrawPathLineToAbsolute (draw_wand, width - ctr, 0.0);
    x = corners [NIM_CORNER_TR] >= 0 ? width : width - ctr;
    y = corners [NIM_CORNER_TR] >= 0 ? 0.0 : ctr;

    DrawPathCurveToQuadraticBezierAbsolute (draw_wand, x, y, width, ctr);
    DrawPathLineToAbsolute (draw_wand, width, height - cbr);
    x = corners [NIM_CORNER_BR] >= 0 ? width : width - cbr;
    y = corners [NIM_CORNER_BR] >= 0 ? height : height - cbr;

    DrawPathCurveToQuadraticBezierAbsolute (draw_wand, x, y, width - cbr, height);
    DrawPathLineToAbsolute (draw_wand, cbl, height);
    x = corners [NIM_CORNER_BL] >= 0 ? 0.0 : cbl;
    y = corners [NIM_CORNER_BL] >= 0 ? height : height - cbl;
  
    DrawPathCurveToQuadraticBezierAbsolute (draw_wand, x, y, 0.0, height - cbl);
    DrawPathLineToAbsolute (draw_wand, 0.0, ctl);
    x = corners [NIM_CORNER_TL] >= 0 ? 0.0 : ctl;
    y = corners [NIM_CORNER_TL] >= 0 ? 0.0 : ctl;
  
    DrawPathCurveToQuadraticBezierAbsolute (draw_wand, x, y, ctl, 0.0);
    DrawPathClose (draw_wand);
    DrawPathFinish (draw_wand);

    MagickDrawImage (result_wand, draw_wand);
    response = MagickCompositeImage (result_wand, *wand, InCompositeOp, 0, 0) == MagickTrue;
  }

  if (response) {
    *wand = DestroyMagickWand (*wand);
    *wand = result_wand;
  } else {
    result_wand = DestroyMagickWand (result_wand);
  }

  if (draw_wand)
    DestroyDrawingWand (draw_wand);

  if (pixel_wand)
    DestroyPixelWand (pixel_wand);

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_round_corners (gchar *filename, const gdouble corners [NIM_CORNER_LAST])
{
  gboolean response = FALSE;
  MagickWand *image_wand = NULL;

  image_wand = NewMagickWand ();
  
  if (MagickReadImage (image_wand, filename) == MagickTrue)
  {
    nim_imaging_round_corners_from_wand (&image_wand, corners);
    return image_wand;
  }

  if (image_wand)
    image_wand = DestroyMagickWand (image_wand);

  return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
enum {
  NIM_EFFECT_SHADOW,
  NIM_EFFECT_BLUR,
  NIM_EFFECT_SHARPEN
};
  
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_imaging_effect_from_wand (MagickWand   **wand,
                                        gint        effect,
                                        gdouble       offx,
                                        gdouble       offy,
                                        gdouble     radius,
                                        gdouble      sigma,
                                        gboolean enable_bg)
{
  gboolean response = FALSE;
  MagickWand *result_wand = NULL;
  MagickWand *shadow_wand;
  MagickWand *bground_wand;
  PixelWand *shadow_color;
  PixelWand *bground_color;
  gint new_w, new_h, im_x, im_y, sh_x = 0, sh_y = 0;
  gint im_w, im_h, sh_w, sh_h;

  if (IsMagickWand (*wand) == MagickFalse) {
    return FALSE;
  }


  switch (effect) {

    case NIM_EFFECT_BLUR:
      result_wand = CloneMagickWand (*wand);
      response = MagickAdaptiveBlurImage (result_wand, radius, sigma) == MagickTrue;
      break;

    case NIM_EFFECT_SHARPEN:
      result_wand = CloneMagickWand (*wand);
      response = MagickSharpenImage (result_wand, radius, sigma) == MagickTrue;
      break;

    case NIM_EFFECT_SHADOW:
    default:

      shadow_wand = CloneMagickWand (*wand);
      bground_wand = NewMagickWand ();
      shadow_color = NewPixelWand ();
      bground_color = NewPixelWand ();

      PixelSetColor (shadow_color, "black");
      PixelSetColor (bground_color, enable_bg ? "#ffffffff" : "#ffffff00");

      MagickSetImageBackgroundColor (shadow_wand, shadow_color);
      MagickShadowImage (shadow_wand, 80.0, sigma, 0.0, 0.0);

      new_w = sh_w = MagickGetImageWidth (shadow_wand);
      new_h = sh_h = MagickGetImageHeight (shadow_wand);
      im_w = MagickGetImageWidth (*wand);
      im_h = MagickGetImageHeight (*wand);
      im_x = (sh_w - im_w) / 2 - offx;
      im_y = (sh_h - im_h) / 2 - offy;

      if (im_x + im_w > sh_w)
        new_w += ((im_x + im_w) - sh_w);

      if (im_y + im_h > sh_h)
        new_h += ((im_y + im_h) - sh_h);

      if (im_x < 0.0) {
        new_w += ABS (im_x);
        sh_x = ABS (im_x);
        im_x = 0;
      }
      
      if (im_y < 0.0) {
        new_h += ABS (im_y);
        sh_y = ABS (im_y);
        im_y = 0;
      }

      MagickNewImage (bground_wand, new_w, new_h, bground_color);
      MagickSetBackgroundColor (bground_wand, bground_color);
      MagickResetIterator (bground_wand);

      response = MagickCompositeImage (bground_wand, shadow_wand, OverCompositeOp, sh_x, sh_y) == MagickTrue
              && MagickCompositeImage (bground_wand, *wand, OverCompositeOp, im_x, im_y) == MagickTrue;

      DestroyPixelWand (shadow_color);
      DestroyPixelWand (bground_color);
      DestroyMagickWand (shadow_wand);
      result_wand = bground_wand;

      break;
  }
  
  if (response) {
    *wand = DestroyMagickWand (*wand);
    *wand = result_wand;
  } else if (result_wand) {
    result_wand = DestroyMagickWand (result_wand);
  }

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_effect  (gchar     *filename,
                                 gint         effect,
                                 gdouble       off_x,
                                 gdouble       off_y,
                                 gdouble      radius,
                                 gdouble       sigma,
                                 gboolean  enable_bg)
{
  MagickWand *image_wand;
  MagickWand *result_wand;

  image_wand = NewMagickWand ();

  if (MagickReadImage (image_wand, filename) == MagickTrue)
  {
    nim_imaging_effect_from_wand (&image_wand, effect, off_x, off_y, radius, sigma, enable_bg);
    return image_wand;
  }

  if (image_wand)
    image_wand = DestroyMagickWand (image_wand);

  return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static MagickWand* nim_imaging_draw_text (const gchar *text,
                                          const gchar *font_desc,
                                          guint font_size,
                                          const gchar *fg_color)
{
  MagickWand *result_wand;
  PixelWand *foreground;
  PixelWand *background;
  DrawingWand *draw_wand;

  result_wand = NewMagickWand ();
  foreground = NewPixelWand ();
  background = NewPixelWand ();
  draw_wand = NewDrawingWand ();

  PixelSetColor (background, "#00000000");
  PixelSetColor (foreground, "#00000077");
  MagickNewImage (result_wand, font_size * 2, 150, background);
  DrawSetFillColor (draw_wand, foreground);
  //DrawSetFillOpacity (draw_wand, 10);
  DrawSetFont (draw_wand, "Ubuntu-Bold");
  DrawSetFontSize (draw_wand, 72);
  DrawSetTextAntialias (draw_wand, MagickTrue);
  DrawAnnotation (draw_wand, 25, 65, "Magick");
  MagickDrawImage (result_wand, draw_wand);
  MagickTrimImage (result_wand, 0);

  draw_wand = DestroyDrawingWand (draw_wand);
  background = DestroyPixelWand (background);
  foreground = DestroyPixelWand (foreground);

  return result_wand;
  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------



int main(int argc, char **argv)
{
  MagickWand *wand;
  gdouble corners [NIM_CORNER_LAST] = {15.0, 15.0, 15.0, 15.0};

  MagickWandGenesis ();

  gtk_init (&argc, &argv);

//  if ((wand = nim_imaging_round_corners (argv [1], corners)) != NULL) 
//  {
//    nim_imaging_effect_from_wand (&wand, NIM_EFFECT_SHADOW, 20, 20, 12.0, 6.0, TRUE);
    wand = nim_imaging_draw_text (NULL, NULL, 0, NULL);
    show_image (wand);
    MagickWriteImage (wand, "mask_result-sharpen.png");
    wand = DestroyMagickWand (wand);
//  }

  MagickWandTerminus ();

  return 0;
}
