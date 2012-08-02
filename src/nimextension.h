/*
 * nimextension.h
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


#ifndef _NIM_EXTENSION_H_
#define _NIM_EXTENSION_H_

#include "nimmain.h"

G_BEGIN_DECLS


#define NIM_TYPE_EXTENSION       (nim_extension_get_type ())
#define NIM_EXTENSION(obj)       (G_TYPE_CHECK_INSTANCE_CAST ((o), NIM_TYPE_EXTENSION, NimExtension))
#define NIM_IS_EXTENSION(obj)    (G_TYPE_CHECK_INSTANCE_TYPE ((o), NIM_TYPE_EXTENSION)

typedef struct _NimExtension         NimExtension;
typedef struct _NimExtensionClass    NimExtensionClass;

struct _NimExtension {
  GObject parent_object;
};

struct _NimExtensionClass {
  GObjectClass parent_class;
};

GType nim_extension_get_type      (void);
void  nim_extension_register_type (GTypeModule *module);
void  nautilus_module_initialize (GTypeModule  *module);
void  nautilus_module_shutdown   (void);
void  nautilus_module_list_types (const GType **types, int *num_types);

G_END_DECLS

#endif  /* _NIM_EXTENSION_H_ */
