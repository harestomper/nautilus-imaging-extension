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
#include <math.h>
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
                                        gdouble      angle,
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
                                 gdouble       angle,
                                 gboolean  enable_bg)
{
  MagickWand *image_wand;
  MagickWand *result_wand;

  image_wand = NewMagickWand ();

  if (MagickReadImage (image_wand, filename) == MagickTrue)
  {
    nim_imaging_effect_from_wand (&image_wand, effect, off_x, off_y, radius, sigma, angle, enable_bg);
    return image_wand;
  }

  if (image_wand)
    image_wand = DestroyMagickWand (image_wand);

  return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static MagickWand* nim_imaging_text (const gchar *text, const gchar *font_desc, const gchar *fg_color)
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
  PixelSetColor (foreground, fg_color);
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
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean magick_is_animation (MagickWand *wand)
{
  const gchar *format;
  gchar *tmpfmt;
  gboolean response;

  format = MagickGetImageFormat (wand);
  tmpfmt = g_ascii_strdown (format, -1);
  
  response    = (g_strcmp0 (tmpfmt, "gif") == 0
              || g_strcmp0 (tmpfmt, "mng") == 0
              || g_strcmp0 (tmpfmt, "miff") == 0);
  g_free (tmpfmt);

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_imaging_rotate_from_wand (MagickWand **wand, gint angle, const gchar *bg_color)
{
  MagickWand *result;
  PixelWand *background;
  gboolean response = FALSE;

  if (magick_is_animation (*wand)) {
    result = MagickCoalesceImages (*wand);
  } else {
    result = CloneMagickWand (*wand);
  }

  background = NewPixelWand ();
  PixelSetColor (background, bg_color != NULL ? bg_color : "none");
  MagickResetIterator (result);

  while (MagickNextImage (result) != MagickFalse)
    response = MagickRotateImage (result, background, angle) == MagickTrue;

  if (response)
  {
    *wand = DestroyMagickWand (*wand);
    *wand = result;
  } else {
    result = DestroyMagickWand (result);
  }

  background = DestroyPixelWand (background);
  
  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_rotate (const gchar *filename, gint angle, const gchar *bg_color)
{
  MagickWand *result;
  gboolean response = FALSE;

  result = NewMagickWand ();

  if (MagickReadImage (result, filename) == MagickTrue) {
    if (nim_imaging_rotate_from_wand (&result, angle, bg_color)) {
      return result;
    }
  }

  result = DestroyMagickWand (result);
  return NULL;
}
//------------------------------------------------------------------------------

enum {
  NIM_RESIZE_BOTH,
  NIM_RESIZE_WIDTH,
  NIM_RESIZE_HEIGHT,
  NIM_RESIZE_CROP,
  NIM_RESIZE_CUSTOM,
  NIM_RESIZE_LAST
};

//------------------------------------------------------------------------------
gboolean nim_imaging_resize_from_wand (MagickWand **wand,
                                      gint width,
                                      gint height,
                                      gint resize_mode,
                                      gboolean thumbnail,
                                      FilterTypes filter,
                                      gdouble factor)
{
  gboolean response = FALSE;
  MagickWand *image;
  gdouble src_w, src_h, aspect, k;
  gdouble crop_x = 0, crop_y = 0;
  gdouble crop_w, crop_h;
  gdouble dest_w, dest_h;
  const gchar *format;
  gchar *tmp_fmt;
  gboolean isanimation = FALSE;

  if (IsMagickWand (*wand) == MagickFalse)
    return FALSE;

  if (resize_mode < 0 || resize_mode >= NIM_RESIZE_LAST)
    return FALSE;


  if (isanimation) {
    image = MagickCoalesceImages (*wand);
    MagickResetIterator (image);
  } else {
    image = CloneMagickWand (*wand);
  }

  src_w = (gdouble) MagickGetImageWidth (image);
  src_h = (gdouble) MagickGetImageHeight (image);
  aspect = src_w / src_h;
  crop_w = dest_w = (gdouble) width;
  crop_h = dest_h = (gdouble) height;

  if (resize_mode == NIM_RESIZE_HEIGHT) {
    dest_w = dest_h * aspect;
  } else if (resize_mode == NIM_RESIZE_WIDTH) {
    dest_h = dest_w / aspect;
  } else if (resize_mode == NIM_RESIZE_BOTH) {
    k = MIN (dest_w / src_w, dest_h / src_h);
    dest_w = round (src_w * k);
    dest_h = round (src_h * k);
      
  } else if (resize_mode == NIM_RESIZE_CROP) {
    k = MAX (dest_w / src_w, dest_h / src_h);
    dest_w = src_w * k;
    dest_h = src_h * k;

    if (dest_w < crop_w) {
      dest_w = crop_w;
      dest_h = crop_w / aspect;
    }
    if (dest_h < crop_h) {
      dest_h = crop_h;
      dest_w = crop_h * aspect;
    }

    crop_x = ABS (crop_w - dest_w) / 2.0;
    crop_y = ABS (crop_h - dest_h) / 2.0;
  }

  dest_w = floor (dest_w);
  dest_h = floor (dest_h);
  crop_w = floor (crop_w);
  crop_h = floor (crop_h);
  crop_x = ceil (crop_x);
  crop_y = ceil (crop_y);

  MagickResetIterator (image);

  if (thumbnail || isanimation) {
    while (MagickNextImage (image) != MagickFalse)
      response = MagickThumbnailImage (image, dest_w, dest_h) == MagickTrue;

  } else if (dest_w == src_w && dest_h == src_h) {
    response = TRUE;
  } else {
    response = MagickResizeImage (image, dest_w, dest_h, filter, factor) == MagickTrue;
  }

  if (response && resize_mode == NIM_RESIZE_CROP && (crop_w != src_w || crop_h != src_h)) {
    MagickResetIterator (image);

    while (MagickNextImage (image) != MagickFalse)
      response = MagickCropImage (image, crop_w, crop_h, crop_x, crop_y) == MagickTrue;
  }

  if (response) {
    *wand = DestroyMagickWand (*wand);
    *wand = image;
  } else {
    image = DestroyMagickWand (image);
  }

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_resize (const gchar *filename,
                                gint width,
                                gint height,
                                gint resize_mode,
                                gboolean thumbnail,
                                FilterTypes filter,
                                gdouble factor)
{
  MagickWand *wand = NULL;
  gboolean response = FALSE;

  wand = NewMagickWand ();

  if (MagickReadImage (wand, filename) == MagickTrue)
    response = nim_imaging_resize_from_wand (&wand, width, height, resize_mode, thumbnail, filter, factor);

  if (!response) {
    wand = DestroyMagickWand (wand);
    wand = NULL;
  }

  return wand;
    
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------



int main(int argc, char **argv)
{
  MagickWand *wand;
  gint function = 0, angle = 0, width, height, mode;
  const gchar *filename, *color, *font, *text;
  gchar *format;
  gdouble corners [NIM_CORNER_LAST] = {15.0, 15.0, 15.0, 15.0};

  MagickWandGenesis ();

  gtk_init (&argc, &argv);

//  if ((wand = nim_imaging_round_corners (argv [1], corners)) != NULL) 
//  {
//    nim_imaging_effect_from_wand (&wand, NIM_EFFECT_SHADOW, 20, 20, 12.0, 6.0, TRUE);

  if (argc >= 3) {
    function = (gint) g_ascii_strtoll (argv [1], NULL, 10);

    switch (function) {
      case 0: //resize
        
        mode = (gint) g_ascii_strtoll (argv [2], NULL, 10);
        width = (gint) g_ascii_strtoll (argv [3], NULL, 10);
        height = (gint) g_ascii_strtoll (argv [4], NULL, 10);
        filename = argv [5];
        wand = nim_imaging_resize (filename, width, height, mode, TRUE,
                                                                        LanczosFilter, 1.0);
                                                                        //UndefinedFilter, 0);
        break;
      case 1: // rotate
g_print ("Rotate\n");
        filename = argv [3];
        angle = (gint) g_ascii_strtoll (argv [2], NULL, 10);
        wand = nim_imaging_rotate (filename, angle * -1, "#aaaaaaaa");
        break;
      case 2: // text marker
        font = argv [2];
        color = argv [3];
        text = argv [4];
        wand = nim_imaging_text (text, font, color);
      case 3: // image marker
      default:
        g_print ("Unknown function\n");
        return 0;
    }
    
//    wand = nim_imaging_draw_text (NULL, NULL, 0, NULL);
//    show_image (wand);
    format = MagickGetImageFormat (wand);
    filename = g_strdup_printf ("result-image.%s", g_ascii_strdown (format, -1));
//    MagickDisplayImage (wand, ":0");
    MagickWriteImages (wand, filename, MagickTrue);
    wand = DestroyMagickWand (wand);
  }

  MagickWandTerminus ();

  return 0;
}
