/*
 * nimutils.c
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


#include <wand/MagickWand.h>
#include "nimutils.h"

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
#define magick_wand_new()                     (NewMagickWand ())
#define magick_wand_init()                    (MagickWandGenesis ())
#define magick_wand_uninit()                  (MagickWandTerminus ())
#define magick_wand_load(wand, fname)         (MagickReadImage ((wand), (fname)))
#define magick_wand_get_width(wand)           (gint)  (MagickGetImageWidth ((wand)))
#define magick_wand_get_height(wand)          (gint)  (MagickGetImageHeight ((wand)))
#define magick_wand_get_hash(wand)            (gchar*)(MagickGetImageSignature ((wand)))
#define magick_wand_get_format(wand)          (gchar*)(MagickGetImageFormat ((wand)))
#define magick_wand_destroy(wand)             (DestroyMagickWand ((wand)))
#define magick_wand_rotate(wand, pixels, angle) (MagickRotateImage ((wand), (pixels), angle))
#define magick_wand_write(wand, filename)     (MagickWriteImage ((wand), (filename)))
#define magick_wand_thumb(wand, w, h)         (MagickThumbnailImage ((wand), (size_t) w, (size_t) h))
#define magick_wand_scale(wand, w, h)         (MagickScaleImage ((wand), (size_t) w, (size_t) h))
#define magick_wand_region(wand, x, y, w, h)  (MagickGetImageRegion ((wand), w, h, x, y));
#define magick_pixel_new()                    (NewPixelWand ())
#define magick_pixel_destroy(pixel_wand)      (DestroyPixelWand ((pixel_wand)))
#define magick_wand_ping_image(wand, filename) (MagickPingImage ((wand), (filename)))
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_check_path (const gchar *filename, gboolean create_parents)
{
  gchar *dirname;
  gboolean result;

  g_return_val_if_fail (filename != NULL, FALSE);

  if (create_parents) {
    dirname = g_path_get_basename (filename);
    result = g_mkdir_with_parents (dirname, 0775) > -1;
    g_debug ("%s:%s:%s", G_STRLOC, dirname, strerror (errno));
    g_free (dirname);
  } else {
    result = g_file_test (filename, G_FILE_TEST_EXISTS);
  }

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_resize           (const gchar  *filein,
                                     const gchar *fileout,
                                     gint           width,
                                     gint          height,
                                     gint            mode)
{
  
  if (!(nim_utils_check_path (filein, FALSE) && nim_utils_check_path (fileout, TRUE)))
    return FALSE;

  
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_rotate           (const gchar  *filein,
                                     const gchar *fileout,
                                     gint           angle,
                                     const gchar   *color)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_thumb            (const gchar  *filein,
                                     const gchar *fileout,
                                     gint           width,
                                     gint          height,
                                     gint            mode)
{
}                                     
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_effect           (const gchar  *filein,
                                     const gchar *fileout,
                                     gint            mode,
                                     gint          offset,
                                     gdouble       radius,
                                     gdouble        sigma)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_convert          (const gchar     *filein,
                                     const gchar    *fileout,
                                     const gchar *new_format,
                                     gint            quality)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_watermark_image  (const gchar  *filein,
                                     const gchar *fileout,
                                     const gchar  *marker,
                                     gint            mode,
                                     gint           angle,
                                     gint         opacity)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_watermark_text   (const gchar     *filein,
                                     const gchar    *fileout,
                                     const gchar       *text,
                                     gint               mode,
                                     gint              angle,
                                     gint           fontsize,
                                     const gchar       *font,
                                     const gchar *font_color,
                                     gint            opacity)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_utils_round_corners    (const gchar  *filein,
                                     const gchar *fileout,
                                     gint          radius)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
