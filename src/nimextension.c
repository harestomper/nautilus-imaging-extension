/*
 * nimextension.c
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


#include "nimextension.h"
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-file-info.h>

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void   nim_extension_instance_init  (NimExtension         *sound);
static void   nim_extension_class_init     (NimExtensionClass    *klass);
static GList *nim_extension_get_file_items (NautilusMenuProvider *provider, GtkWidget *window, GList *files);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GType type_list[1];
static GType nim_extension_type = 0;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GList *nim_extension_get_background_items (NautilusMenuProvider  *provider, GtkWidget *window, NautilusFileInfo *file_info)
{
  return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gboolean nautilus_file_is_image (NautilusFileInfo *info)
{
  gchar *mimetype;
  gboolean result;

  mimetype = nautilus_file_info_get_mime_type (info);
  result = g_str_has_prefix (mimetype, "image/");

  g_free (mimetype);

  return result;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GList *nim_filter_files (GList *files)
{
  GList *filelist = NULL, *node;

  for (node = files; node; node = node->next)
  {
    NautilusFileInfo *info;

    info = node->data;
    if (nautilus_file_is_image (info))
      filelist = g_list_append (filelist, info);
  }

  return filelist;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nautilus_menu_item_activated  (NautilusMenuItem *item, GList *files, GtkWidget *parent_window)
{
  GtkWidget *dialog;
  GList *filelist = NULL;

  filelist = nim_filter_files (files);

  if (filelist) {
    dialog = nim_main_dialog_new (GTK_WINDOW (parent_window), filelist);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_hide (dialog);
    gtk_widget_destroy (dialog);

    g_list_free (filelist);
  }

}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static GList *nim_extension_get_file_items (NautilusMenuProvider *provider, GtkWidget *window, GList *files)
{
  NautilusMenuItem *item;
  GList            *scan = NULL;
  GList            *items = NULL;

  if (files == NULL)
    return NULL;

  for (scan = files; scan; scan = scan->next) {
    if (nautilus_file_is_image (scan->data)) {
      item = nautilus_menu_item_new ("NimExtension::imaging",
                                     "Convert images",
                                     "Simple operations on image",
                                     GTK_STOCK_CONVERT);

      g_signal_connect (item, "activate",
            G_CALLBACK (nautilus_menu_item_activated),
            nautilus_file_info_list_copy (files));

      items = g_list_prepend (items, item);
      items = g_list_reverse (items);

      return items;
    }
  }

  return NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_extension_menu_provider_iface_init (NautilusMenuProviderIface *iface)
{
  iface->get_background_items = nim_extension_get_background_items;
  iface->get_file_items = nim_extension_get_file_items;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_extension_instance_init (NimExtension *sound)
{
    return;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_extension_class_init (NimExtensionClass *class)
{
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
GType nim_extension_get_type (void)
{
  return nim_extension_type;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_extension_register_type (GTypeModule *module)
{
  static const GTypeInfo info = {
    sizeof (NimExtensionClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) nim_extension_class_init,
    NULL,
    NULL,
    sizeof (NimExtension),
    0,
    (GInstanceInitFunc) nim_extension_instance_init,
  };

  static const GInterfaceInfo menu_provider_iface_info = {
    (GInterfaceInitFunc) nim_extension_menu_provider_iface_init,
    NULL,
    NULL,
  };

  nim_extension_type = g_type_module_register_type (module,
                                                    G_TYPE_OBJECT,
                                                    "NimExtension",
                                                    &info, 0);

  g_type_module_add_interface (module,
                               nim_extension_type,
                               NAUTILUS_TYPE_MENU_PROVIDER,
                               &menu_provider_iface_info);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nautilus_module_initialize (GTypeModule *module)
{
  g_print ("Initializing nautilus-imaging extension\n");

  nim_extension_register_type (module);
  type_list[0] = NIM_TYPE_EXTENSION;

  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nautilus_module_shutdown (void)
{
  g_print ("Shutting down nautilus-imaging extension\n");
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void  nautilus_module_list_types (const GType **types, int *num_types)
{
  *types = type_list;
  *num_types = G_N_ELEMENTS (type_list);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
