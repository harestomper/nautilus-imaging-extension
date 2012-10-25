/*
 * nimdialog.h
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


#ifndef __NIMDIALOG_H__
#define __NIMDIALOG_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS


#define NIM_TYPE_DIALOG             (nim_dialog_get_type ())
#define NIM_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIM_TYPE_DIALOG, NimDialog))
#define NIM_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIM_TYPE_DIALOG, NimDialogClass))
#define NIM_IS_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIM_TYPE_DIALOG))
#define NIM_IS_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIM_TYPE_DIALOG))
#define NIM_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIM_TYPE_DIALOG, NimDialogClass))

typedef struct _NimDialog         NimDialog;
typedef struct _NimDialogClass    NimDialogClass;
typedef struct _NimDialogPrivate  NimDialogPrivate;

struct _NimDialog
{
  GObject parent;
  /* add your public declarations here */
  NimDialogPrivate *priv;
};

struct _NimDialogClass
{
  GObjectClass parent_class;
};


GType nim_dialog_get_type (void);

NimDialog *nim_dialog_new (GtkWindow *parent_window, gint dialog_type);
gint nim_dialog_run (NimDialog *this);

gint nim_dialog_config_get_data (NimDialog *this);


G_END_DECLS

#endif /* __NIMDIALOG_H__ */
