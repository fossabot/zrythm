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

/** \file
 */

#include "config.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "audio/engine.h"
#include "gui/widgets/bot_bar.h"
#include "gui/widgets/cpu.h"
#include "project.h"

#include <stdio.h>
#ifdef HAVE_LIBGTOP
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/loadavg.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "utils/cairo.h"

G_DEFINE_TYPE (CpuWidget,
               cpu_widget,
               GTK_TYPE_DRAWING_AREA)

#define BAR_HEIGHT 12
#define BAR_WIDTH 3
#define NUM_BARS 12
#define PADDING 2
#define ICON_SIZE BAR_HEIGHT
#define TOTAL_H \
  (PADDING * 3 + BAR_HEIGHT * 2)
#define TOTAL_W \
  (ICON_SIZE + PADDING * 3 + NUM_BARS * PADDING * 2)

/**
 * Taken from
 * http://zetcode.com/gui/gtk2/customwidget/
 */
static int
cpu_draw_cb (
  GtkWidget * widget,
  cairo_t * cr,
  CpuWidget * self)
{
  GtkStyleContext *context =
  gtk_widget_get_style_context (widget);

  int width =
    gtk_widget_get_allocated_width (widget);
  int height =
    gtk_widget_get_allocated_height (widget);

  gtk_render_background (
    context, cr, 0, 0, width, height);

  /*GdkRGBA color;*/
  /*gdk_rgba_parse (&color, "#00ccff");*/
  /*cairo_set_source_rgba (*/
    /*cr, color.red, color.green, color.blue, 1.0);*/
  cairo_set_source_rgb(cr, 0.6, 1.0, 0);

  cairo_surface_t * surface;
  surface =
    z_cairo_get_surface_from_icon_name (
      "iconfinder_cpu_2561419",
      ICON_SIZE, 0);
  cairo_mask_surface(
    cr, surface, PADDING, PADDING);
  cairo_fill(cr);

  surface =
    z_cairo_get_surface_from_icon_name (
      "wave-square-solid",
      ICON_SIZE, 0);
  cairo_mask_surface(
    cr, surface, PADDING, 2 * PADDING + ICON_SIZE);
  cairo_fill(cr);

  int limit, i;

  /* CPU */
  limit =
    (self->cpu * NUM_BARS) / 100;

  for (i = 1; i <= NUM_BARS; i++)
    {
      if (i <= limit)
        cairo_set_source_rgb(cr, 0.6, 1.0, 0);
      else
        cairo_set_source_rgb(cr, 0.2, 0.4, 0);

      cairo_rectangle(
        cr,
        ICON_SIZE + PADDING + i * PADDING * 2,
        PADDING,
        BAR_WIDTH,
        BAR_HEIGHT);
      cairo_fill(cr);
    }

  /* DSP */
  limit =
    (self->dsp * NUM_BARS) / 100;

  for (i = 1; i <= NUM_BARS; i++)
    {
      if (i <= limit)
        cairo_set_source_rgb(cr, 0.6, 1.0, 0);
      else
        cairo_set_source_rgb(cr, 0.2, 0.4, 0);

      cairo_rectangle(
        cr,
        ICON_SIZE + PADDING + i * PADDING * 2,
        PADDING * 2 + BAR_HEIGHT,
        BAR_WIDTH,
        BAR_HEIGHT);
      cairo_fill(cr);
    }

	return 0;
}

#if defined(HAVE_LIBGTOP)
unsigned long prev_total, prev_idle;
#endif


/**
 * Refreshes DSP load percentage.
 *
 * This is a running average of the time it takes to
 * execute a full process cycle for all JACK clients
 * as a percentage of the real time available per
 * cycle
 */
static gboolean
refresh_dsp_load (
  CpuWidget * self)
{
  if (g_atomic_int_get (&AUDIO_ENGINE->run))
    {

#ifdef __APPLE__
      /* TODO engine not working yet */
#else
      gint64 block_latency =
        (AUDIO_ENGINE->block_length * 1000000) /
        AUDIO_ENGINE->sample_rate;
      self->dsp =
        (int)
        ((double) AUDIO_ENGINE->max_time_taken *
           100.0 /
         (double) block_latency);
#endif
    }
  else
    self->dsp = 0;

  AUDIO_ENGINE->max_time_taken = 0;

  return G_SOURCE_CONTINUE;
}

#if defined(__linux__) && !defined(HAVE_LIBGTOP)
static long double a[4], b[4] = {0,0,0,0}, loadavg;
#endif

#ifdef _WIN32
static unsigned long long
FileTimeToInt64 (const FILETIME * ft)
{
  return
    (((unsigned long long)(ft->dwHighDateTime))<<32) |
    ((unsigned long long)ft->dwLowDateTime);
}
#endif

/**
 * Refreshes CPU load percentage.
 */
static int
refresh_cpu_load (
  CpuWidget * self)
{
#ifdef HAVE_LIBGTOP
  glibtop_cpu cpu;
  glibtop_get_cpu (&cpu);
  self->cpu =
    (int)
    (100.f -
      (float) (cpu.idle - prev_idle) /
      (float) (cpu.total - prev_total) * 100.f);

  prev_total = cpu.total;
  prev_idle = cpu.idle;
#elif defined(__linux__)
  /* ======= non libgtop ====== */
  FILE *fp;

  fp = fopen("/proc/stat","r");
  fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
  fclose(fp);

  loadavg = ((a[0]+a[1]+a[2]) - (b[0]+b[1]+b[2])) / ((a[0]+a[1]+a[2]+a[3]) - (b[0]+b[1]+b[2]+b[3]));
  self->cpu = (int) ((double) loadavg * (double) 100);

  b[0] = a[0];
  b[1] = a[1];
  b[2] = a[2];
  b[3] = a[3];
  /* ========== end ========= */
#elif defined(_WIN32)
  FILETIME idleTime, kernelTime, userTime;

  if (GetSystemTimes (
        &idleTime, &kernelTime, &userTime))
    {
      unsigned long long idleTicks =
        FileTimeToInt64(&idleTime);
      unsigned long long totalTicks =
        FileTimeToInt64(&kernelTime) +
        FileTimeToInt64(&userTime);
      static unsigned long long _previousTotalTicks = 0;
      static unsigned long long _previousIdleTicks = 0;

      unsigned long long totalTicksSinceLastTime = totalTicks-_previousTotalTicks;
      unsigned long long idleTicksSinceLastTime  = idleTicks-_previousIdleTicks;

      self->cpu =
        (int)
        (1.0f - (
          (totalTicksSinceLastTime > 0) ?
            ((float)idleTicksSinceLastTime) /
              totalTicksSinceLastTime : 0));

      _previousTotalTicks = totalTicks;
      _previousIdleTicks  = idleTicks;
    }
  else
    self->cpu = -1.0f;
#endif

  char ttip[100];
  sprintf (
    ttip,
    "CPU: %d%%\nDSP: %d%%",
    self->cpu, self->dsp);
  gtk_widget_set_tooltip_text (
    (GtkWidget *) self, ttip);
  gtk_widget_queue_draw ((GtkWidget *) self);

  return G_SOURCE_CONTINUE;
}

static gboolean
on_motion (GtkWidget * widget, GdkEvent *event)
{
  CpuWidget * self = Z_CPU_WIDGET (widget);

  if (gdk_event_get_event_type (event) ==
        GDK_ENTER_NOTIFY)
    {
      gtk_widget_set_state_flags (
        GTK_WIDGET (self),
        GTK_STATE_FLAG_PRELIGHT, 0);
    }
  else if (gdk_event_get_event_type (event) ==
             GDK_LEAVE_NOTIFY)
    {
      gtk_widget_unset_state_flags (
        GTK_WIDGET (self),
        GTK_STATE_FLAG_PRELIGHT);
    }

  return FALSE;
}

/**
 * Creates a new Cpu widget and binds it to the
 * given value.
 */
void
cpu_widget_setup (
  CpuWidget * self)
{
  self->cpu_source_id =
    g_timeout_add_seconds (
      1, (GSourceFunc) refresh_cpu_load, self);
  self->dsp_source_id =
    g_timeout_add_seconds (
      1, (GSourceFunc) refresh_dsp_load, self);
}

static void
finalize (
  CpuWidget * self)
{
  /* remove the timeout callbacks */
  g_source_remove (
    self->cpu_source_id);
  g_source_remove (
    self->dsp_source_id);

  G_OBJECT_CLASS (
    cpu_widget_parent_class)->
      finalize (G_OBJECT (self));
}

static void
cpu_widget_init (CpuWidget * self)
{
  self->cpu = 0;
  self->dsp = 0;

  gtk_widget_set_size_request (
    GTK_WIDGET (self), TOTAL_W, TOTAL_H);

  /* connect signals */
  g_signal_connect (
    G_OBJECT (self), "draw",
    G_CALLBACK (cpu_draw_cb), self);
  g_signal_connect (
    G_OBJECT (self), "enter-notify-event",
    G_CALLBACK (on_motion),  self);
  g_signal_connect (
    G_OBJECT(self), "leave-notify-event",
    G_CALLBACK (on_motion),  self);
}

static void
cpu_widget_class_init (CpuWidgetClass * _klass)
{
  GtkWidgetClass * klass =
    GTK_WIDGET_CLASS (_klass);
  gtk_widget_class_set_css_name (
    klass, "cpu");

  GObjectClass * oklass = G_OBJECT_CLASS (klass);
  oklass->finalize =
    (GObjectFinalizeFunc) finalize;
}
