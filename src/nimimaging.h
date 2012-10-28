/*
 * nimimafing.h
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


#ifndef __NIMIMAGING__
#define __NIMIMAGING__

#include <wand/magick_wand.h>

G_BEGIN_DECLS

enum {
  NIM_ROTATE_90,
  NIM_ROTATE_180,
  NIM_ROTATE_270,
  NIM_ROTATE_CUSTOM
};

enum {
  NIM_FUNCTION_ROTATE,
  NIM_FUNCTION_RESIZE,
  NIM_FUNCTION_CONVERT,
  NIM_FUNCTION_MARK,
  NIM_FUNCTION_CORNERS,
  NIM_FUNCTION_EFFECTS,
  NIM_FUNCTION_LAST
};


enum {
    NIM_CORNER_TL,
    NIM_CORNER_TR,
    NIM_CORNER_BR,
    NIM_CORNER_BL,
    NIM_CORNER_LAST
};

enum {
  NIM_EFFECT_NORMALIZE,
  NIM_EFFECT_ENHANCE,
  NIM_EFFECT_EQUALIZE,
  NIM_EFFECT_SHARPEN,
  NIM_EFFECT_MONO,
  NIM_EFFECT_SHADOW,
  NIM_EFFECT_BLUR,
  NIM_EFFECT_MOTION,
  NIM_EFFECT_RADIAL_BLUR,
  NIM_EFFECT_OIL,
  NIM_EFFECT_SKETCH,
  NIM_EFFECT_SPREAD,
  NIM_EFFECT_FLIP,
  NIM_EFFECT_FLOP,
  NIM_EFFECT_NEGATIVE,
  NIM_EFFECT_NEGATIVE_MONO,
  NIM_EFFECT_CHARCOAL,
  NIM_EFFECT_EDGE,
  NIM_EFFECT_EMBROSS,
  NIM_EFFECT_GAUSSIAN,
  NIM_EFFECT_ROLL,
};

enum {
  NIM_FIND_IMAGE,
  NIM_FIND_UI,
  NIM_FIND_CONFIG
};  

enum {
  NIM_RESIZE_BOTH,
  NIM_RESIZE_WIDTH,
  NIM_RESIZE_HEIGHT,
  NIM_RESIZE_CROP,
  NIM_RESIZE_CUSTOM,
  NIM_RESIZE_LAST
};

enum {
  NIM_RESIZE_FILTER_NONE,
  NIM_RESIZE_FILTER_CATROM,
  NIM_RESIZE_FILTER_HANNING,
  NIM_RESIZE_FILTER_MITCHELL,
  NIM_RESIZE_FILTER_SINC,
  NIM_RESIZE_FILTER_BLACKMAN,
  NIM_RESIZE_FILTER_GAUSSIAN,
  NIM_RESIZE_FILTER_HERMMITE,
  NIM_RESIZE_FILTER_QUANDRAT,
  NIM_RESIZE_FILTER_TRIANGLE,
  NIM_RESIZE_FILTER_BOX,
  NIM_RESIZE_FILTER_LANCZOS,
  NIM_RESIZE_FILTER_BESSEL
};

enum {
  NIM_WATER_METHOD_TILE,
  NIM_WATER_METHOD_FIT,
  NIM_WATER_METHOD_ALIGN,
  NIM_WATER_METHOD_ABS, 
};

enum {
  NIM_WATER_TEXT_EFFECT_FLAT,
  NIM_WATER_TEXT_EFFECT_SHADOWED,
  NIM_WATER_TEXT_EFFECT_RELIEF,
  NIM_WATER_TEXT_EFFECT_CONTOUR,
};
  

enum {
  NIM_CONVERT_TYPE_PNG,
  NIM_CONVERT_TYPE_JPEG,
  NIM_CONVERT_TYPE_GIF,
  NIM_CONVERT_TYPE_TIFF,
};

enum {
  NIM_SAVE_OVER,
  NIM_SAVE_MARK,
  NIM_SAVE_OTHER
};


MagickWand* nim_imaging_round_corners           (gchar *filename, const gdouble corners [NIM_CORNER_LAST]);

gboolean    nim_imaging_round_corners_from_wand (MagickWand **wand, const gdouble corners [NIM_CORNER_LAST]);

GdkPixbuf*  nim_imaging_convert_wand_to_pixbuf  (MagickWand *wand);

MagickWand* nim_imaging_effect                  (gchar     *filename,
                                                 gint         effect,
                                                 gdouble       off_x,
                                                 gdouble       off_y,
                                                 gdouble      radius,
                                                 gdouble       sigma,
                                                 gdouble       angle,
                                                 gboolean enable_bg);

gboolean nim_imaging_effect_from_wand           (MagickWand    **wand,
                                                  gint         effect,
                                                  gdouble        offx,
                                                  gdouble        offy,
                                                  gdouble      radius,
                                                  gdouble       sigma,
                                                  gdouble       angle,
                                                  gboolean enable_bg);

gboolean    nim_imaging_convert_to_gif          (gchar  **filelist,
                                                 int        n_elem,
                                                 gint        delay,
                                                 gboolean combine);

gchar*      nim_imaging_find_file  (int tp);

gboolean nim_imaging_rotate_from_wand (MagickWand **wand, gint angle, const gchar *bg_color);
MagickWand* nim_imaging_rotate (const gchar *filename, gint angle, const gchar *bg_color);

MagickWand* nim_imaging_resize (const gchar *filename,
                                gint width,
                                gint height,
                                gint resize_mode,
                                gboolean thumbnail,
                                FilterTypes filter,
                                gdouble factor);

gboolean nim_imaging_resize_from_wand (MagickWand **wand,
                                      gint width,
                                      gint height,
                                      gint resize_mode,
                                      gboolean thumbnail,
                                      FilterTypes filter,
                                      gdouble factor);

gboolean magick_is_animation (MagickWand *wand);
MagickWand* nim_imaging_draw_text_simple (const gchar *text, const gchar *fontname, gint fontsize, const gchar *foreground);


G_END_DECLS

#endif /* __NIMIMAGING__  */
