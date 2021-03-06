/*
 * Copyright (C) 2018-2019 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file
 *
 * Base widget class for Region's.
 */

#ifndef __GUI_WIDGETS_REGION_H__
#define __GUI_WIDGETS_REGION_H__

#include "audio/region.h"
#include "gui/widgets/arranger_object.h"
#include "utils/ui.h"

#include <gtk/gtk.h>

/**
 * @addtogroup widgets
 *
 * @{
 */

#ifdef _WIN32
#define REGION_NAME_FONT "Sans SemiBold 7"
#else
#define REGION_NAME_FONT "Sans SemiBold 8"
#endif
#define REGION_NAME_PADDING_R 5
#define REGION_NAME_BOX_PADDING 2
#define REGION_NAME_BOX_CURVINESS 4.0

/**
 * Returns the lane rectangle for the region.
 */
void
region_get_lane_full_rect (
  ZRegion *       self,
  GdkRectangle * rect);

/**
 * Draws the ZRegion in the given cairo context in
 * relative coordinates.
 *
 * @param cr The cairo context in the region's
 *   drawable coordinates.
 * @param rect Arranger rectangle.
 */
void
region_draw (
  ZRegion *       self,
  cairo_t *      cr,
  GdkRectangle * rect);

#if 0

/**
 * Returns if the current position is for resizing
 * L.
 */
int
region_widget_is_resize_l (
  RegionWidget * self,
  int             x);

/**
 * Returns if the current position is for resizing
 * R.
 */
int
region_widget_is_resize_r (
  RegionWidget * self,
  int             x);

/**
 * Returns if the current position is for resizing
 * loop.
 */
int
region_widget_is_resize_loop (
  RegionWidget * self,
  int             y);

/**
 * Draws the name of the Region.
 *
 * To be called during a cairo draw callback.
 */
void
region_widget_draw_name (
  RegionWidget * self,
  cairo_t *      cr,
  GdkRectangle * rect);

/**
 * Destroys the widget completely.
 */
void
region_widget_delete (RegionWidget *self);

/**
 * Returns if region widgets should show cut lines.
 *
 * To be used to set the region's "show_cut".
 *
 * @param alt_pressed Whether alt is currently
 *   pressed.
 */
int
region_widget_should_show_cut_lines (
  int alt_pressed);

/**
 * Draws the loop points (dashes).
 */
void
region_widget_draw_loop_points (
  RegionWidget * self,
  GtkWidget *    widget,
  cairo_t *      cr,
  GdkRectangle * rect);

/**
 * Draws the background rectangle.
 */
void
region_widget_draw_background (
  RegionWidget * self,
  GtkWidget *    widget,
  cairo_t *      cr,
  GdkRectangle * rect);

/**
 * @}
 */

#endif
#endif
