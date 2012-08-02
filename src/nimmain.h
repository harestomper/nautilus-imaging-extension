/*
 * nimmain.h
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

#ifndef _NIMMAIN_H_
#define _NIMMAIN_H_

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <wand/MagickWand.h>
// #include <glib/gi18n-lib.h>



G_BEGIN_DECLS

enum {
  NIM_RESIZE_WIDTH,
  NIM_RESIZE_HEIGHT,
  NIM_RESIZE_BOTH,
  NIM_RESIZE_NO_ASPECT,
  NIM_RESIZE_CROP
};

enum {
  NIM_ROTATE_90,
  NIM_ROTATE_180,
  NIM_ROTATE_270,
  NIM_ROTATE_CUSTOM
};

enum {
  NIM_WATERMARK_X_START  = (1 << 1),
  NIM_WATERMARK_X_CENTER = (1 << 2),
  NIM_WATERMARK_X_END    = (1 << 3),
  NIM_WATERMARK_Y_START  = (1 << 4),
  NIM_WATERMARK_Y_CENTER = (1 << 5),
  NIM_WATERMARK_Y_END    = (1 << 6),
  NIM_WATERMARK_TILE     = (1 << 7)
};

enum {
  NIM_EFFECT_OFFSET_SHADOW,
  NIM_OFFECT_DROP_SHADOW,
  NIM_EFFECT_BLUR,
  NIM_EFFECT_OUTLINE
};

enum {
  NIM_CORNER_TOP_LEFT     = (1 << 1),
  NIM_CORNER_TOP_RIGTH    = (1 << 2),
  NIM_CORNER_BOTTOM_RIGTH = (1 << 3),
  NIM_CORNER_BUTTON_LEFT  = (1 << 4)
};

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
  
G_END_DECLS

#endif /* _NIMMAIN_H_ */
