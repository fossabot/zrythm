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
 * Zrythm test helper.
 */

#ifndef __TEST_HELPERS_ZRYTHM_H__
#define __TEST_HELPERS_ZRYTHM_H__

#include "audio/chord_track.h"
#include "audio/engine_dummy.h"
#include "audio/marker_track.h"
#include "audio/tracklist.h"
#include "project.h"
#include "utils/cairo.h"
#include "utils/objects.h"
#include "utils/flags.h"
#include "utils/ui.h"
#include "zrythm.h"

#include <glib.h>
#include <glib/gi18n.h>

/**
 * @addtogroup tests
 *
 * @{
 */

/** Time to run fishbowl, in seconds */
#define DEFAULT_FISHBOWL_TIME 20

/** Compares 2 Position pointers. */
#define g_assert_cmppos(a,b) \
  g_assert_cmpmem ( \
    a, sizeof (Position), b, sizeof (Position))

#ifndef G_APPROX_VALUE
#define G_APPROX_VALUE(a, b, epsilon) \
  (((a) > (b) ? (a) - (b) : (b) - (a)) < (epsilon))
#endif

#ifndef g_assert_cmpfloat_with_epsilon
#define g_assert_cmpfloat_with_epsilon( \
  n1,n2,epsilon) \
  G_STMT_START { \
    double __n1 = (n1), __n2 = (n2), __epsilon = (epsilon); \
    if (G_APPROX_VALUE (__n1,  __n2, __epsilon)) ; else \
      g_assertion_message_cmpnum (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
      #n1 " == " #n2 " (+/- " #epsilon ")", __n1, "==", __n2, 'f'); \
  } G_STMT_END
#endif

/**
 * To be called by every test's main to initialize
 * Zrythm to default values.
 */
void
test_helper_zrythm_init ()
{
  object_utils_init ();

  ZRYTHM = calloc (1, sizeof (Zrythm));
  ZRYTHM->symap = symap_new ();
  ZRYTHM->testing = 1;
  ZRYTHM->have_ui = 0;
  PROJECT = calloc (1, sizeof (Project));
  AUDIO_ENGINE->block_length = 256;

  project_init_selections (PROJECT);

  AUDIO_ENGINE->audio_backend =
    AUDIO_BACKEND_DUMMY;
  AUDIO_ENGINE->midi_backend =
    MIDI_BACKEND_DUMMY;
  engine_init (AUDIO_ENGINE, 0);
  engine_update_frames_per_tick (
    AUDIO_ENGINE, 4, 140, 44000);
  undo_manager_init (&PROJECT->undo_manager);

  /* init pinned tracks */
  Track * track =
    chord_track_new ();
  tracklist_append_track (
    TRACKLIST, track, F_NO_PUBLISH_EVENTS,
    F_NO_RECALC_GRAPH);
  track->pinned = 1;
  TRACKLIST->chord_track = track;
  track =
    marker_track_default ();
  tracklist_append_track (
    TRACKLIST, track, F_NO_PUBLISH_EVENTS,
    F_NO_RECALC_GRAPH);
  track->pinned = 1;
  TRACKLIST->marker_track = track;

  /* add master channel to mixer and tracklist */
  track =
    track_new (
      TRACK_TYPE_MASTER, _("Master"),
      F_WITHOUT_LANE);
  tracklist_append_track (
    TRACKLIST, track, F_NO_PUBLISH_EVENTS,
    F_NO_RECALC_GRAPH);
  TRACKLIST->master_track = track;
  tracklist_selections_add_track (
    TRACKLIST_SELECTIONS, track);
}

/**
 * To be called after test_helper_zrythm_init() to
 * initialize the UI (GTK).
 */
void
test_helper_zrythm_gui_init (
  int argc, char *argv[])
{
  ZRYTHM->have_ui = 1;
  if (gtk_init_check (&argc, &argv))
    {
      gtk_init (&argc, &argv);
    }
  else
    {
      g_test_skip ("No display found");
      exit (77);
    }

  /* set theme */
  g_object_set (gtk_settings_get_default (),
                "gtk-theme-name",
                "Matcha-dark-sea",
                NULL);
  g_message ("set theme");

  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/breeze-icons");
  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/fork-awesome");
  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/font-awesome");
  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/zrythm");
  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/ext");
  gtk_icon_theme_add_resource_path (
    gtk_icon_theme_get_default (),
    "/org/zrythm/Zrythm/app/icons/gnome-builder");

  g_message ("set resource paths");

  // set default css provider
  GtkCssProvider * css_provider =
    gtk_css_provider_new();
  gtk_css_provider_load_from_resource (
    css_provider,
    "/org/zrythm/Zrythm/app/theme.css");
  gtk_style_context_add_provider_for_screen (
          gdk_screen_get_default (),
          GTK_STYLE_PROVIDER (css_provider),
          800);
  g_object_unref (css_provider);
  g_message ("set default css provider");

  CAIRO_CACHES = z_cairo_caches_new ();
  UI_CACHES = ui_caches_new ();
}

/**
 * @}
 */

#endif
