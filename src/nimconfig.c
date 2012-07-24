/*
 * nimconfig.c
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

#include "nimconfig.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
struct _NimConfigPrivate
{
  GKeyFile *kf;
};
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_config_finalize (GObject *object);
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
G_DEFINE_TYPE (NimConfig, nim_config, G_TYPE_OBJECT)
static NimConfig *default_nim_config = NULL;
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_config_class_init (NimConfigClass *klass)
{
  GObjectClass *g_object_class;

  g_object_class = G_OBJECT_CLASS (klass);

  g_object_class->finalize = nim_config_finalize;

  g_type_class_add_private ((gpointer)klass, sizeof (NimConfigPrivate));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_config_finalize (GObject *object)
{
  NimConfig *self;

  g_return_if_fail (NIM_IS_CONFIG (object));

  self = NIM_CONFIG (object);

  g_key_file_free (self->priv->kf);
  
  G_OBJECT_CLASS (nim_config_parent_class)->finalize (object);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static void nim_config_init (NimConfig *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, NIM_TYPE_CONFIG, NimConfigPrivate);
  self->priv->kf = NULL;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static gchar *nim_config_build_filename (void)
{
  return g_build_filename (G_DIR_SEPARATOR_S,
                           g_get_user_config_dir (),
                           "nautilus-imaging-extension",
                           NIM_CFG_FILENAME,
                           NULL);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
static NimConfig* nim_config_new (void)
{
  NimConfig *this;
  NimConfigPrivate *priv;
  GObject *gobject;
  GError *error = NULL;
  gchar* filename = NULL;

  gobject = g_object_new (NIM_TYPE_CONFIG, NULL);
  this = NIM_CONFIG (gobject);
  priv = this->priv;

  priv->kf = g_key_file_new ();
  filename = nim_config_build_filename ();

  g_key_file_load_from_file (priv->kf, filename, 0, &error);

  if (error) {
    g_warning ("%s:%s", G_STRLOC, error->message);
    g_error_free (error);
  }

  g_free (filename);
  return this;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
NimConfig* nim_config_default (void)
{
    if (default_nim_config == NULL)
        default_nim_config = nim_config_new ();

    return default_nim_config;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_sync (void)
{
  NimConfig *this;
  gchar *contents;
  gchar *filename;
  gsize length;
  gchar *dirname;
  GError *error = NULL;

  this = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (this));

  contents = g_key_file_to_data (this->priv->kf, &length, &error);
  filename = nim_config_build_filename ();
  dirname = g_path_get_dirname (filename);

  if (error) {
    g_debug ("%s: %s", G_STRLOC, error->message);
    g_error_free (error);
    error = NULL;
  }

  if (!g_file_test (dirname, G_FILE_TEST_IS_DIR)) {

    if (g_mkdir_with_parents (dirname, 0775) == -1)
      g_warning ("%s: %s", G_STRLOC, strerror (errno));
  }

  g_file_set_contents (filename, contents, (gssize) length, &error);

  if (error) {
    g_debug ("%s: %s", G_STRLOC, error->message);
    g_error_free (error);
    error = NULL;
  }

  g_free (filename);
  g_free (dirname);
  g_free (contents);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint nim_config_get_int (const gchar *group, const gchar *key, gint dfval)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_val_if_fail (NIM_IS_CONFIG (config), dfval);
  priv = config->priv;

  if (g_key_file_has_key (priv->kf, group, key, NULL))
    return g_key_file_get_integer (priv->kf, group, key, NULL);
  else
    return dfval;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_set_int (const gchar *group, const gchar *key, gint value)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (config));
  priv = config->priv;

  g_key_file_set_integer (priv->kf, group, key, value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gint64 nim_config_get_int64 (const gchar *group, const gchar *key, gint64 dfval)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_val_if_fail (NIM_IS_CONFIG (config), dfval);
  priv = config->priv;

  if (g_key_file_has_key (priv->kf, group, key, NULL))
    return g_key_file_get_int64 (priv->kf, group, key, NULL);
  else
    return dfval;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_set_int64 (const gchar *group, const gchar *key, gint64 value)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (config));
  priv = config->priv;

  g_key_file_set_int64 (priv->kf, group, key, value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gdouble nim_config_get_double (const gchar *group, const gchar *key, gdouble dfval)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_val_if_fail (NIM_IS_CONFIG (config), dfval);
  priv = config->priv;

  if (g_key_file_has_key (priv->kf, group, key, NULL))
    return g_key_file_get_double (priv->kf, group, key, NULL);
  else
    return dfval;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_set_double (const gchar *group, const gchar *key, gdouble value)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (config));
  priv = config->priv;

  g_key_file_set_double (priv->kf, group, key, value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gboolean nim_config_get_bool (const gchar *group, const gchar *key, gint64 dfval)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_val_if_fail (NIM_IS_CONFIG (config), dfval);
  priv = config->priv;

  if (g_key_file_has_key (priv->kf, group, key, NULL))
    return g_key_file_get_boolean (priv->kf, group, key, NULL);
  else
    return dfval;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_set_bool (const gchar *group, const gchar *key, gint64 value)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (config));
  priv = config->priv;

  g_key_file_set_boolean (priv->kf, group, key, value);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
gchar* nim_config_get_string (const gchar *group, const gchar *key, const gchar *dfval)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_val_if_fail (NIM_IS_CONFIG (config), g_strdup (dfval));
  priv = config->priv;

  if (g_key_file_has_key (priv->kf, group, key, NULL))
    return g_key_file_get_string (priv->kf, group, key, NULL);
  else
    return g_strdup (dfval);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void nim_config_set_string   (const gchar *group, const gchar *key, const gchar *value)
{
  NimConfig *config;
  NimConfigPrivate *priv;

  config = nim_config_default ();
  g_return_if_fail (NIM_IS_CONFIG (config));
  priv = config->priv;

  if (value)
    g_key_file_set_string (priv->kf, group, key, value);
  else
    g_key_file_set_string (priv->kf, group, key, "");
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
