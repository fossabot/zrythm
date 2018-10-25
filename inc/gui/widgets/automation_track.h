/*
 * gui/widgets/automation_track.h - AutomationTrack view
 *
 * Copyright (C) 2018 Alexandros Theodotou
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __GUI_WIDGETS_AUTOMATION_TRACK_H__
#define __GUI_WIDGETS_AUTOMATION_TRACK_H__

#include <gtk/gtk.h>

#define AUTOMATION_TRACK_WIDGET_TYPE                  (automation_track_widget_get_type ())
#define AUTOMATION_TRACK_WIDGET(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUTOMATION_TRACK_WIDGET_TYPE, AutomationTrackWidget))
#define AUTOMATION_TRACK_WIDGET_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST  ((klass), AUTOMATION_TRACK_WIDGET, AutomationTrackWidgetClass))
#define IS_AUTOMATION_TRACK_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUTOMATION_TRACK_WIDGET_TYPE))
#define IS_AUTOMATION_TRACK_WIDGET_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE  ((klass), AUTOMATION_TRACK_WIDGET_TYPE))
#define AUTOMATION_TRACK_WIDGET_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS  ((obj), AUTOMATION_TRACK_WIDGET_TYPE, AutomationTrackWidgetClass))

typedef struct TrackWidget TrackWidget;
typedef struct AutomationTrack AutomationTrack;
typedef struct DigitalMeterWidget DigitalMeterWidget;
typedef struct Track Track;

typedef struct AutomationTrackWidget
{
  GtkGrid                 parent_instance;
  GtkComboBox *           selector;
  GtkBox *                value_box;
  DigitalMeterWidget *    value;
  GtkToggleButton *       mute_toggle;
  AutomationTrack *       automation_track;
} AutomationTrackWidget;

typedef struct AutomationTrackWidgetClass
{
  GtkGridClass    parent_class;
} AutomationTrackWidgetClass;

/**
 * Creates a new automation_track widget from the given automation_track.
 */
AutomationTrackWidget *
automation_track_widget_new ();


#endif


