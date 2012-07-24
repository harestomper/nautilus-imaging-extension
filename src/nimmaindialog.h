/*
 * nimmaindialog.h
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


#ifndef __NIMMAINDIALOG_H__
#define __NIMMAINDIALOG_H__

#include <gtk/gtk.h>
#include "nimconfig.h"

G_BEGIN_DECLS


#define NIM_TYPE_MAIN_DIALOG             (nim_main_dialog_get_type ())
#define NIM_MAIN_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIM_TYPE_MAIN_DIALOG, NimMainDialog))
#define NIM_MAIN_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIM_TYPE_MAIN_DIALOG, NimMainDialogClass))
#define NIM_IS_MAIN_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIM_TYPE_MAIN_DIALOG))
#define NIM_IS_MAIN_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIM_TYPE_MAIN_DIALOG))
#define NIM_MAIN_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIM_TYPE_MAIN_DIALOG, NimMainDialogClass))

typedef struct _NimMainDialog         NimMainDialog;
typedef struct _NimMainDialogClass    NimMainDialogClass;
typedef struct _NimMainDialogPrivate  NimMainDialogPrivate;

struct _NimMainDialog
{
  GtkDialog parent;
  /* add your public declarations here */
  NimMainDialogPrivate *priv;
};

struct _NimMainDialogClass
{
  GtkDialogClass parent_class;
};


enum {
  NIM_FUNC_RESIZE,
  NIM_FUNC_ROTATE,
  NIM_FUNC_CONVERT,
  NIM_FUNC_WATERMARK,
  NIM_FUNC_THUMB,
  NIM_FUNC_ROUND,
  NIM_FUNC_EFFECT,
  NIM_FUNC_RENAME,
  NIM_FUNC_LAST
};


GType nim_main_dialog_get_type (void);

GtkWidget *nim_main_dialog_new (void);


G_END_DECLS

#endif /* __NIMMAINDIALOG_H__ */
