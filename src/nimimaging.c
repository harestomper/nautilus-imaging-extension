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
//#include <pango.h>
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
    } else {
      g_object_ref (G_OBJECT (pixbuf));
    }
  }

  return pixbuf;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define CLAMP_RADIUS(w, h, r) ceil (((MIN (w, h) / 2) < ABS (r) ? MIN (w, h) / 2.0 : ABS (r)))
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
  MagickWand *temp_wand;
  gint new_w, new_h, im_x, im_y, sh_x = 0, sh_y = 0;
  gint im_w, im_h, sh_w, sh_h;
  gint negative = MagickFalse;

  if (IsMagickWand (*wand) == MagickFalse) {
    return FALSE;
  }

  result_wand = CloneMagickWand (*wand);

  switch (effect) {

    case NIM_EFFECT_ROLL:
      response = MagickRollImage (result_wand, offx, offy) == MagickTrue;
      break;

    case NIM_EFFECT_BLUR:
      response = MagickBlurImage (result_wand, radius, sigma) == MagickTrue;
      break;

    case NIM_EFFECT_RADIAL_BLUR:
      response = MagickRadialBlurImage (result_wand, angle) == MagickTrue;
      break;

    case NIM_EFFECT_SHARPEN:
      response = MagickSharpenImage (result_wand, radius, sigma) == MagickTrue;
      break;

    case NIM_EFFECT_MOTION:
      response = MagickMotionBlurImage (result_wand, radius, sigma, angle);
      break;

    case NIM_EFFECT_OIL:
      response = MagickOilPaintImage (result_wand, radius);
      break;

    case NIM_EFFECT_SKETCH:
      response = MagickSketchImage (result_wand, radius, sigma, angle);
      break;
      
    case NIM_EFFECT_SPREAD:
      response = MagickSpreadImage (result_wand, radius);
      break;
      
    case NIM_EFFECT_ENHANCE:
      response = MagickEnhanceImage (result_wand);
      break;
      
    case NIM_EFFECT_NORMALIZE:
      response = MagickNormalizeImage (result_wand);
      break;
      
    case NIM_EFFECT_EQUALIZE:
      response = MagickEqualizeImage (result_wand);
      break;
      
    case NIM_EFFECT_FLIP:
      response = MagickFlipImage (result_wand);
      break;
      
    case NIM_EFFECT_FLOP:
      response = MagickFlopImage (result_wand);
      break;
      
    case NIM_EFFECT_NEGATIVE_MONO:
      negative = MagickTrue;

    case NIM_EFFECT_MONO:
      temp_wand = MagickFxImage (result_wand,  "R * 299 / 1000 + G * 687 / 1000 + B * 114 / 1000");
      DestroyMagickWand (result_wand);
      result_wand = temp_wand;
      response = TRUE;

      if (negative == MagickFalse)
        break;
      
    case NIM_EFFECT_NEGATIVE:
      response = MagickNegateImage (result_wand, MagickFalse);
      break;

    case NIM_EFFECT_CHARCOAL:
      response = MagickCharcoalImage (result_wand, radius, sigma);
      break;

    case NIM_EFFECT_EDGE:
      response = MagickEdgeImage (result_wand, radius);
      break;

    case NIM_EFFECT_EMBROSS:
      response = MagickEmbossImage (result_wand, radius, sigma);
      break;

    case NIM_EFFECT_GAUSSIAN:
      response = MagickGaussianBlurImage (result_wand, radius, sigma);
      break;
    
    case NIM_EFFECT_SHADOW:
    default:

      shadow_wand = result_wand;
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
gchar* nim_imaging_find_file (int tp)
{
  gchar *result = NULL;
  const gchar *dirname;
  const gchar *const *temp_data_dirs = NULL;
  gchar **datadirs;
  const gchar *target = NULL;
  gint n, n_elem = 0;

  switch (tp) {
    case NIM_FIND_IMAGE:
      target = "test-image.png";

    case NIM_FIND_UI:
      if (target == NULL)
        target = "common.ui";

      temp_data_dirs = g_get_system_data_dirs ();
      n_elem = g_strv_length ((gchar**) temp_data_dirs);
      datadirs = g_strdupv ((gchar **) temp_data_dirs);
      break;

    case NIM_FIND_CONFIG:
      target = "settings.conf";
      n_elem = 2;
      datadirs = g_new0 (gchar*, n_elem);
      datadirs [0] = g_strdup (g_get_user_config_dir ());
      datadirs [1] = NULL;
      break;

    default:
      return NULL;
  }
  
  for (n = 0; n < n_elem; n++)
  {
    dirname = datadirs [n];
    result = g_build_filename (G_DIR_SEPARATOR_S, dirname, "nautilus-imaging", target, NULL);

    if (g_file_test (result, G_FILE_TEST_EXISTS)) {
      break;
    } else {
      g_free (result);
      result = NULL;
    }
  }

  g_strfreev (datadirs);
  
  return result;
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
gboolean magick_is_animation (MagickWand *wand)
{
  const gchar *format;
  gchar *tmpfmt;
  gboolean response;

  format = MagickGetImageFormat (wand);
  tmpfmt = g_ascii_strdown (format, -1);
  
  response = (g_strcmp0 (tmpfmt, "gif") == 0
           || g_strcmp0 (tmpfmt, "mng") == 0
           || g_strcmp0 (tmpfmt, "miff") == 0);
  g_free (tmpfmt);

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
    //  0 character width
    //  1 character height
    //  2 ascender
    //  3 descender
    //  4 text width
    //  5 text height
    //  6 maximum horizontal advance
enum {
  TEXT_INFO_CHW,
  TEXT_INFO_CHH,
  TEXT_INFO_ASC,
  TEXT_INFO_DSC,
  TEXT_INFO_WIDTH,
  TEXT_INFO_HEIGHT,
  TEXT_INFO_ADVANCE
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
MagickWand* nim_imaging_draw_text_simple (const gchar *text,
                                          const gchar *fontname,
                                          gint fontsize,
                                          const gchar *fground,
                                          const gchar *bground,
                                          gboolean stroke,
                                          gint linewidth)
{
  MagickWand *result_wand;
  MagickWand *temp_wand;
  PixelWand *background;
  PixelWand *foreground;
  DrawingWand *draw_wand;
  gchar *font_name;
  gint font_size;
  gdouble *textsize;
  gdouble width, height;

  if (text == NULL || fontname == NULL)
    return NULL;

  result_wand = NewMagickWand ();
  temp_wand = NewMagickWand ();
  background = NewPixelWand ();
  foreground = NewPixelWand ();
  draw_wand = NewDrawingWand ();

  PixelSetColor (foreground, (fground != NULL) ? fground : "#ffffffff");
  PixelSetColor (background, (bground != NULL) ? bground : "#00000000");
  DrawSetFillColor (draw_wand, foreground);

  MagickNewImage (temp_wand, 1, 1, background);
  DrawSetFont (draw_wand, fontname);
  DrawSetFontSize (draw_wand, fontsize);
  DrawSetTextAntialias (draw_wand, MagickTrue);
  textsize = MagickQueryMultilineFontMetrics (temp_wand, draw_wand, text);
  width = textsize [TEXT_INFO_WIDTH] +  textsize [TEXT_INFO_ADVANCE] / 2.0;
  height = textsize [TEXT_INFO_ASC] - textsize [TEXT_INFO_DSC];

  if (width > NIM_MIN_FONT_SIZE && height > NIM_MIN_FONT_SIZE) {
      MagickNewImage (result_wand, width, height, background);
      MagickAnnotateImage (result_wand, draw_wand, 0, textsize [TEXT_INFO_ASC], 0.0, text);
      MagickDrawImage (result_wand, draw_wand);
    //  nim_imaging_effect_from_wand (&result_wand, NIM_EFFECT_GAUSSIAN, 0, 0, 3.0, 1.0, 0.0, FALSE);
      MagickTrimImage (result_wand, 0);
  } else {
    result_wand = DestroyMagickWand (result_wand);
    result_wand = NULL;
  }
    
  temp_wand = DestroyMagickWand (temp_wand);
  background = DestroyPixelWand (background);
  foreground = DestroyPixelWand (foreground);
  draw_wand = DestroyDrawingWand (draw_wand);
//  if (font_name) g_free (font_name);
  g_free (textsize);

  return result_wand;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_imaging_make_font_preview (GdkPixbuf **pixbuf,
                                          const gchar *fontname,
                                          gint         fontsize,
                                          gint         preview_width,
                                          gint         preview_height,
                                          const gchar *foreground,
                                          const gchar *background,
                                          const gchar *preview_text,
                                          gdouble      angle)
{
  MagickWand *wand = NULL;
  GdkPixbuf *tmppixbuf = NULL;
  gboolean response = FALSE;

  if ((wand = nim_imaging_draw_text_simple (preview_text,
                                            fontname,
                                            fontsize,
                                            foreground,
                                            background,
                                            FALSE,
                                            1)) != NULL)
  {

    if (angle != 0.0)
      nim_imaging_rotate_from_wand (&wand, angle, background);
      
    if (nim_imaging_resize_from_wand (&wand,
                                      preview_width,
                                      preview_height,
                                      NIM_RESIZE_BOTH,
                                      TRUE,
                                      UndefinedFilter,
                                      1.0))
                                      {
                                        
      tmppixbuf = nim_imaging_convert_wand_to_pixbuf (wand);
      if (GDK_IS_PIXBUF (tmppixbuf)) {
        *pixbuf = tmppixbuf;
        response = TRUE;
      }
    }
  }

  if (wand)
    wand = DestroyMagickWand (wand);

  return response;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
guint32 color_to_uint (const gchar *str, gint *n_elem)
{
  guint32 result = 0;
  char *start, *end;
  gint len = 0, shift, value;

  for (start = (char*) str; *start != '\0' && *start != '#' && *start != 'x'; start++);
  for (end = start; *end != '\0' && *(end + 1) != '\0' && end - start < 8; end++, len++);
  for (; start < end ; end--)
  {
    shift = (len - (end - start)) * 4;
    value = g_ascii_xdigit_value (*end);
    result += (value << shift);
  }
  if (n_elem)
    *n_elem = len;
  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean color_to_rgba (GdkRGBA *rgba, const gchar *xstring)
{
  gint n_elem;
  guint32 value;

  value = color_to_uint (xstring, &n_elem);

  if (n_elem >= 6 && value > 0) {
    gint shift = 0;
    shift = (n_elem - 8) * 4;
    rgba->alpha = (shift < 0) ? 1.0 : (gdouble) ((value >> (shift)) & 0xff) / 255.;
    rgba->blue =  ((value >> (shift + 8)) & 0xff) / 255.;
    rgba->green = ((value >> (shift + 16)) & 0xff) / 255.;
    rgba->red =   ((value >> (shift + 24)) & 0xff) / 255.;
    return TRUE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar *rgba_to_color (GdkRGBA *rgba)
{
  gchar *result;
  result = g_strdup_printf ("#%02x%02x%02x%02x", 
                            (guint8) (ceil (rgba->red * 255.)),
                            (guint8) (ceil (rgba->green * 255.)),
                            (guint8) (ceil (rgba->blue * 255.)),
                            (guint8) (ceil (rgba->alpha * 255.)));
  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
