/*
 * nimfontchooser.h
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


#ifndef __NIMFONTCHOOSER_H__
#define __NIMFONTCHOOSER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define NIM_TYPE_FONT_CHOOSER             (nim_font_chooser_get_type ())
#define NIM_FONT_CHOOSER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIM_TYPE_FONT_CHOOSER, NimFontChooser))
#define NIM_FONT_CHOOSER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIM_TYPE_FONT_CHOOSER, NimFontChooserClass))
#define NIM_IS_FONT_CHOOSER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIM_TYPE_FONT_CHOOSER))
#define NIM_IS_FONT_CHOOSER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIM_TYPE_FONT_CHOOSER))
#define NIM_FONT_CHOOSER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIM_TYPE_FONT_CHOOSER, NimFontChooserClass))

typedef struct _NimFontChooser         NimFontChooser;
typedef struct _NimFontChooserClass    NimFontChooserClass;
typedef struct _NimFontChooserPrivate  NimFontChooserPrivate;

struct _NimFontChooser
{
  GtkBox parent;
  /* add your public declarations here */
  NimFontChooserPrivate *priv;
};

struct _NimFontChooserClass
{
  GtkBoxClass parent_class;
};


GType nim_font_chooser_get_type (void);

GtkWidget *nim_font_chooser_new (void);
void nim_font_chooser_set_font_name (NimFontChooser *chooser, const gchar *fontname);
gchar *nim_font_chooser_get_font_name (NimFontChooser *chooser);
void nim_font_chooser_set_font_size (NimFontChooser *chooer, gint fontsize);
gint nim_font_chooser_get_font_size (NimFontChooser *chooer);
void nim_font_chooser_set_foreground (NimFontChooser *chooser, const gchar *foreground);
gchar *nim_font_chooser_get_foreground (NimFontChooser *chooser);


G_END_DECLS

#endif /* __NIMFONTCHOOSER_H__ */
