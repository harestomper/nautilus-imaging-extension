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
    NIM_CORNER_TL,
    NIM_CORNER_TR,
    NIM_CORNER_BR,
    NIM_CORNER_BL,
    NIM_CORNER_LAST
};


MagickWand* nim_imaging_round_corners (gchar *filename, const gdouble corners [NIM_CORNER_LAST]);
GdkPixbuf*  nim_imaging_convert_wand_to_pixbuf (MagickWand *wand);
gboolean    nim_imaging_convert_to_gif (gchar **filelist, int n_elem, gint delay, gboolean combine);
gchar*      nim_imaging_get_path_to_test_image (int tp);



G_END_DECLS

#endif /* __NIMIMAGING__  */
