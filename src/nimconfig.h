/*
 * nimconfig.h
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


#ifndef __NIMCONFIG_H__
#define __NIMCONFIG_H__

#include <glib-object.h>

G_BEGIN_DECLS


#define NIM_TYPE_CONFIG             (nim_config_get_type ())
#define NIM_CONFIG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NIM_TYPE_CONFIG, NimConfig))
#define NIM_CONFIG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), NIM_TYPE_CONFIG, NimConfigClass))
#define NIM_IS_CONFIG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NIM_TYPE_CONFIG))
#define NIM_IS_CONFIG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), NIM_TYPE_CONFIG))
#define NIM_CONFIG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), NIM_TYPE_CONFIG, NimConfigClass))

typedef struct _NimConfig         NimConfig;
typedef struct _NimConfigClass    NimConfigClass;
typedef struct _NimConfigPrivate  NimConfigPrivate;

struct _NimConfig
{
  GObject parent;
  /* add your public declarations here */
  NimConfigPrivate *priv;
};

struct _NimConfigClass
{
  GObjectClass parent_class;
};


GType nim_config_get_type (void);

NimConfig*  nim_config_default      (void);

void        nim_config_sync         (void);

gint        nim_config_get_int      (const gchar *group, const gchar *key, gint dfval);
void        nim_config_set_int      (const gchar *group, const gchar *key, gint value);

gint64      nim_config_get_int64    (const gchar *group, const gchar *key, gint64 dfval);
void        nim_config_get_int64    (const gchar *group, const gchar *key, gint64 value);

gdouble     nim_config_get_double   (const gchar *group, const gchar *key, gdouble dfval);
void        nim_config_set_double   (const gchar *group, const gchar *key, gdouble value);

gboolean    nim_config_get_bool     (const gchar *group, const gchar *key, gint64 dfval);
void        nim_config_set_bool     (const gchar *group, const gchar *key, gint64 value);

gchar*      nim_config_get_string   (const gchar *group, const gchar *key, const gchar *dfval);
void        nim_config_set_string   (const gchar *group, const gchar *key, const gchar *value);

#define NIM_CFG_GRP_ROTATE  "Rotation"
#define NIM_CFG_GRP_ReSIZE  "Resize"
#define NIM_CFG_GRT_WATER   "Watermark"
#define NIM_CFG_GRP_THUMB   "Thumbnails"
#define NIM_CFG_GRP_CORNER  "RoundCorners"
#define NIM_CFG_GRP_SHADOW  "Shadows"
#define NIM_CFG_GRP_RENAME  "Renamer"
#define NIM_CFG_GRP_COMMON  "Common"

#define NIM_CFG_FILENAME    "nautilus-imaging.conf"
#define NIM_CFG_LAST_FOLDER "last_folder"
#define NIM_CFG_ASPECT      "aspect_ratio"
#define NIM_CFG_WIDTH       "last_width"
#define NIM_CFG_HEIGHT      "last_height"
#define NIM_CFG_ENABLED     "enabled"
#define NIM_CFG_MODIFIER    "name_modifier"
#define NIM_CFG_FIXED_ANGLE "fixed_angle"
#define NIM_CFG_ANGLE       "last_angle"
#define NIM_CFG_FIXED       "fixed"
#define NIM_CFG_CUSTOM      "custom"
#define NIM_CFG_ADD_SUFFIX  "append_suffix"
#define NIM_CFG_WATER_TEXT  "water_text"
#define NIM_CFG_DRAW_MODE   "drawing_mode"
#define NIM_CFG_COLOR       "color"
#define NIM_CFG_FONT_TYPE   "font_type"
#define NIM_CFG_FIT_MODE    "fit_mode"
#define NIM_CFG_MODE        "last_mode"
#define NIM_CFG_RADIUS      "radius"
#define NIM_CFG_CORNERS     "active_corners"
#define NIM_CFG_OFFSET      "shadow_offset"
#define NIM_CFG_SIGMA       "shadow_sigma"


G_END_DECLS

#endif /* __NIMCONFIG_H__ */
