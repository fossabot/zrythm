/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>
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
 * Inspector port widget.
 */

#ifndef __GUI_WIDGETS_INSPECTOR_PORT_H__
#define __GUI_WIDGETS_INSPECTOR_PORT_H__

#include "utils/resources.h"

#include <gtk/gtk.h>

#define INSPECTOR_PORT_WIDGET_TYPE \
  (inspector_port_widget_get_type ())
G_DECLARE_FINAL_TYPE (
  InspectorPortWidget,
  inspector_port_widget,
  Z, INSPECTOR_PORT_WIDGET,
  GtkOverlay)

typedef struct _BarSliderWidget BarSliderWidget;

/**
 * @addtogroup widgets
 *
 * @{
 */

/**
 * A Port to show in the inspector for Plugin's.
 */
typedef struct _InspectorPortWidget
{
  GtkOverlay        parent_instance;

  /** The bar slider. */
  BarSliderWidget * bar_slider;

  /** Last MIDI event trigger time, for MIDI ports. */
  gint64            last_midi_trigger_time;

  /**
   * Last time the tooltip changed.
   *
   * Used to avoid excessive updating of the
   * tooltip text.
   */
  gint64            last_tooltip_change;

  /** Caches from the port. */
  float             minf;
  float             maxf;
  float             zerof;

  /** Port name cache. */
  char              port_str[400];

  /** Port this is for. */
  Port *            port;

  /** Jack button to expose port to jack. */
  GtkToggleButton * jack;

  /** MIDI button to select MIDI CC sources. */
  GtkToggleButton * midi;

  /** Multipress guesture for double click. */
  GtkGestureMultiPress * double_click_gesture;

  /** Multipress guesture for right click. */
  GtkGestureMultiPress * right_click_gesture;
} InspectorPortWidget;

void
inspector_port_widget_refresh (
  InspectorPortWidget * self);

/**
 * Creates a new widget.
 */
InspectorPortWidget *
inspector_port_widget_new (
  Port * port);

/**
 * @}
 */

#endif
