/*
 * gui/widgets/channel_slot.c - Channel slot
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

/** \file
 */

#include "audio/channel.h"
#include "gui/widget_manager.h"
#include "gui/widgets/channel_slot.h"

G_DEFINE_TYPE (ChannelSlotWidget, channel_slot_widget, GTK_TYPE_DRAWING_AREA)

static void
on_drag_data_received (GtkWidget        *widget,
               GdkDragContext   *context,
               gint              x,
               gint              y,
               GtkSelectionData *data,
               guint             info,
               guint             time,
               gpointer          user_data)
{
  ChannelSlotWidget * channel_slot = CHANNEL_SLOT_WIDGET (widget);
  Channel * channel = channel_slot->channel;
  /*int index = channel_get_last_active_slot_index (channel);*/
  /*if (index == MAX_PLUGINS - 1)*/
    /*{*/
      /*[> TODO <]*/
      /*g_error ("Channel is full");*/
      /*return;*/
    /*}*/

  Plugin_Descriptor * descr = *(gpointer *) gtk_selection_data_get_data (data);

  Plugin * plugin = plugin_create_from_descr (descr);

  plugin_instantiate (plugin);

  /* add to specific channel */
  channel_add_plugin (channel, channel_slot->slot_index, plugin);
  gtk_widget_queue_draw (widget);
}

static int
draw_cb (GtkWidget * widget, cairo_t * cr, void* data)
{
  guint width, height;
  GtkStyleContext *context;
  ChannelSlotWidget * self = (ChannelSlotWidget *) widget;
  context = gtk_widget_get_style_context (widget);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);


  int padding = 2;
  cairo_text_extents_t te;
  Plugin * plugin = self->channel->strip[self->slot_index];
  if (plugin)
    {
      /* fill background */
      cairo_set_source_rgba (cr, 0.4, 1.0, 0.4, 1.0);
      cairo_move_to (cr, padding, padding);
      cairo_line_to (cr, padding, height - padding);
      cairo_line_to (cr, width - padding, height - padding);
      cairo_line_to (cr, width - padding, padding);
      cairo_fill(cr);

      /* fill text */
      cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
      cairo_select_font_face (cr, "Arial",
                              CAIRO_FONT_SLANT_NORMAL,
                              CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size (cr, 12.0);
      cairo_text_extents (cr, plugin->descr->name, &te);
      cairo_move_to (cr, 20,
                     te.height / 2 - te.y_bearing);
      cairo_show_text (cr, plugin->descr->name);
    }
  else
    {
      /* fill background */
      cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 1.0);
      cairo_move_to (cr, padding, padding);
      cairo_line_to (cr, padding, height - padding);
      cairo_line_to (cr, width - padding, height - padding);
      cairo_line_to (cr, width - padding, padding);
      cairo_fill(cr);

      /* fill text */
      const char * txt = "empty slot";
      cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1.0);
      cairo_select_font_face (cr, "Arial",
                              CAIRO_FONT_SLANT_ITALIC,
                              CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size (cr, 12.0);
      cairo_text_extents (cr, txt, &te);
      cairo_move_to (cr, 20,
                     te.height / 2 - te.y_bearing);
      cairo_show_text (cr, txt);

    }

  //highlight if grabbed or if mouse is hovering over me
  /*if (self->hover)*/
    /*{*/
      /*cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.12 );*/
      /*cairo_new_sub_path (cr);*/
      /*cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);*/
      /*cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);*/
      /*cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);*/
      /*cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);*/
      /*cairo_close_path (cr);*/
      /*cairo_fill (cr);*/
    /*}*/

}

/**
 * Creates a new ChannelSlot widget and binds it to the given value.
 */
ChannelSlotWidget *
channel_slot_widget_new (int slot_index, Channel * channel)
{
  ChannelSlotWidget * self = g_object_new (CHANNEL_SLOT_WIDGET_TYPE, NULL);
  self->slot_index = slot_index;
  self->channel = channel;

  gtk_widget_set_size_request (GTK_WIDGET (self), -1, 24);

  /* connect signals */
  g_signal_connect (G_OBJECT (self), "draw",
                    G_CALLBACK (draw_cb), self);
  g_signal_connect (GTK_WIDGET (self),
                    "drag-data-received",
                    G_CALLBACK(on_drag_data_received), NULL);

  gtk_drag_dest_set (GTK_WIDGET (self),
                            GTK_DEST_DEFAULT_ALL,
                            WIDGET_MANAGER->entries,
                            WIDGET_MANAGER->num_entries,
                            GDK_ACTION_COPY);

  return self;
}

static void
channel_slot_widget_init (ChannelSlotWidget * self)
{
}

static void
channel_slot_widget_class_init (ChannelSlotWidgetClass * klass)
{
}