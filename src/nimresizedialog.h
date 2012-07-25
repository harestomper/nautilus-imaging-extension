/*
 * nimresizedialog.h
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


#ifndef __NIMRESIZEDIALOG_H__
#define __NIMRESIZEDIALOG_H__

#include <gtk/gtk.h>
#include "nimconfig.h"

G_BEGIN_DECLS


#define NIM_TYPE_RESIZE_DIALOG             (nim_resize_dialog_get_type ())
#define NIM_RESIZE_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIM_TYPE_RESIZE_DIALOG, NimResizeDialog))
#define NIM_RESIZE_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIM_TYPE_RESIZE_DIALOG, NimResizeDialogClass))
#define NIM_IS_RESIZE_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIM_TYPE_RESIZE_DIALOG))
#define NIM_IS_RESIZE_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIM_TYPE_RESIZE_DIALOG))
#define NIM_RESIZE_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIM_TYPE_RESIZE_DIALOG, NimResizeDialogClass))

typedef struct _NimResizeDialog         NimResizeDialog;
typedef struct _NimResizeDialogClass    NimResizeDialogClass;
typedef struct _NimResizeDialogPrivate  NimResizeDialogPrivate;

struct _NimResizeDialog
{
  GtkDialog parent;
  NimResizeDialogPrivate *priv;
};

struct _NimResizeDialogClass
{
  GtkDialogClass parent_class;
};


GType nim_resize_dialog_get_type (void);

GtkWidget *nim_resize_dialog_new (GtkWindow *parent_window, GList *filelist);


G_END_DECLS

#endif /* __NIMRESIZEDIALOG_H__ */
