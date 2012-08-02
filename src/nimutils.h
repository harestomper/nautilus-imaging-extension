/*
 * nimutils.h
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


#ifndef _NIMUTILS_H_
#define _NIMUTILS_H_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

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

gboolean nim_utils_check_path (const gchar *filename, gboolean create_parents);

gboolean nim_utils_resize           (const gchar *filein,
                                     const gchar *fileout,
                                     gint width,
                                     gint height,
                                     gint mode);
                                     
gboolean nim_utils_rotate           (const gchar *filein,
                                     const gchar *fileout,
                                     gint angle,
                                     const gchar *color);
                                     
gboolean nim_utils_thumb            (const gchar *filein,
                                     const gchar *fileout,
                                     gint width,
                                     gint height,
                                     gint mode);
                                     
gboolean nim_utils_effect           (const gchar  *filein,
                                     const gchar *fileout,
                                     gint            mode,
                                     gint          offset,
                                     gdouble       radius,
                                     gdouble       sigma);
                                     
gboolean nim_utils_convert          (const gchar     *filein,
                                     const gchar    *fileout,
                                     const gchar *new_format,
                                     gint           quality);
                                     
gboolean nim_utils_watermark_image  (const gchar  *filein,
                                     const gchar *fileout,
                                     const gchar  *marker,
                                     gint            mode,
                                     gint           angle,
                                     gint        opacity);
                                     
gboolean nim_utils_watermark_text   (const gchar     *filein,
                                     const gchar    *fileout,
                                     const gchar       *text,
                                     gint               mode,
                                     gint              angle,
                                     gint           fontsize,
                                     const gchar       *font,
                                     const gchar *font_color,
                                     gint           opacity);

gboolean nim_utils_round_corners    (const gchar  *filein,
                                     const gchar *fileout,
                                     gint         radius);


G_END_DECLS

#endif  /* _NIMUTILS_H_ */
