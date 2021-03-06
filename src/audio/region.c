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

#include "audio/audio_region.h"
#include "audio/automation_region.h"
#include "audio/chord_region.h"
#include "audio/channel.h"
#include "audio/clip.h"
#include "audio/midi_note.h"
#include "audio/midi_region.h"
#include "audio/instrument_track.h"
#include "audio/pool.h"
#include "audio/region.h"
#include "audio/track.h"
#include "gui/widgets/audio_region.h"
#include "gui/widgets/automation_region.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/chord_region.h"
#include "gui/widgets/clip_editor.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/midi_arranger.h"
#include "gui/widgets/midi_region.h"
#include "gui/widgets/region.h"
#include "gui/widgets/timeline_arranger.h"
#include "gui/widgets/timeline_panel.h"
#include "project.h"
#include "utils/arrays.h"
#include "utils/flags.h"
#include "utils/audio.h"
#include "utils/yaml.h"

#include <glib/gi18n.h>

#include <sndfile.h>
#include <samplerate.h>


/**
 * Only to be used by implementing structs.
 *
 * @param is_main Is main Region. If this is 1 then
 *   arranger_object_info_init_main() is called to
 *   create 3 additional regions in obj_info.
 */
void
region_init (
  ZRegion *         self,
  const Position * start_pos,
  const Position * end_pos,
  const int        is_main)
{
  ArrangerObject * obj =
    (ArrangerObject *) self;
  obj->type = ARRANGER_OBJECT_TYPE_REGION;
  obj->can_have_lanes =
    region_type_has_lane (
      self->type);
  obj->has_length = 1;
  obj->can_loop = 1;
  position_set_to_pos (
    &obj->pos, start_pos);
  obj->pos.frames = start_pos->frames;
  position_set_to_pos (
    &obj->end_pos, end_pos);
  obj->end_pos.frames = end_pos->frames;
  position_init (&obj->clip_start_pos);
  long length =
    arranger_object_get_length_in_frames (obj);
  g_warn_if_fail (length > 0);
  position_from_frames (
    &obj->loop_end_pos, length);
  position_init (&obj->loop_start_pos);
  obj->loop_end_pos.frames = length;
  self->linked_region_name = NULL;
}

/**
 * Generates a name for the ZRegion, either using
 * the given AutomationTrack or Track, or appending
 * to the given base name.
 */
void
region_gen_name (
  ZRegion *          region,
  const char *      base_name,
  AutomationTrack * at,
  Track *           track)
{
  int count = 1;

  /* Name to try to assign */
  char * orig_name = NULL;
  if (base_name)
    orig_name =
      g_strdup (base_name);
  else if (at)
    orig_name =
      g_strdup_printf (
        "%s - %s",
        track->name, at->automatable->label);
  else
    orig_name = g_strdup (track->name);

  char * name = g_strdup (orig_name);
  while (region_find_by_name (name))
    {
      g_free (name);
      name =
        g_strdup_printf ("%s %d",
                         orig_name,
                         count++);
    }
  region_set_name (
    region, name);
  g_free (name);
  g_free (orig_name);
}

/**
 * Sets the track lane.
 */
void
region_set_lane (
  ZRegion *    self,
  TrackLane * lane)
{
  g_return_if_fail (lane);
  self->lane = lane;
  self->lane_pos = lane->pos;
  self->track_pos = lane->track_pos;
}

/**
 * Moves the ZRegion to the given Track, maintaining
 * the selection status of the ZRegion and the
 * TrackLane position.
 *
 * Assumes that the ZRegion is already in a
 * TrackLane.
 */
void
region_move_to_track (
  ZRegion *  region,
  Track *   track)
{
  g_return_if_fail (region && track);

  Track * region_track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_if_fail (region_track);

  if (region_track == track)
    return;

  int selected = region_is_selected (region);

  /* create lanes if they don't exist */
  track_create_missing_lanes (
    track, region->lane_pos);

  /* remove the region from its old track */
  track_remove_region (
    region_track,
    region, F_NO_PUBLISH_EVENTS, F_NO_FREE);

  /* add the region to its new track */
  track_add_region (
    track, region, NULL,
    region->lane_pos, F_NO_GEN_NAME,
    F_NO_PUBLISH_EVENTS);
  region_set_lane (
    region, track->lanes[region->lane_pos]);

  /* reselect if necessary */
  arranger_object_select (
    (ArrangerObject *) region, selected,
    F_APPEND);

  /* remove empty lanes if the region was the
   * last on its track lane */
  track_remove_empty_last_lanes (
    region_track);
}

/**
 * Moves the given ZRegion to the given TrackLane.
 *
 * Works with TrackLane's of other Track's as well.
 *
 * Maintains the selection status of the
 * Region.
 *
 * Assumes that the ZRegion is already in a
 * TrackLane.
 */
void
region_move_to_lane (
  ZRegion *    region,
  TrackLane * lane)
{
  g_return_if_fail (region && lane);

  Track * region_track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_if_fail (region_track);

  int selected = region_is_selected (region);
  int is_clip_editor_region =
    region == CLIP_EDITOR->region;

  track_remove_region (
    region_track,
    region, F_NO_PUBLISH_EVENTS, F_NO_FREE);
  track_add_region (
    lane->track, region, NULL,
    lane->pos,
    F_NO_GEN_NAME,
    F_NO_PUBLISH_EVENTS);

  /* reset the clip editor region because
   * track_remove clears it */
  if (is_clip_editor_region)
    clip_editor_set_region (
      CLIP_EDITOR, region);

  arranger_object_select (
    (ArrangerObject *) region, selected,
    F_APPEND);
  region_set_lane (region, lane);
  g_warn_if_fail (
    lane->pos == region->lane_pos);

  track_create_missing_lanes (
    region_track, lane->pos);
  track_remove_empty_last_lanes (region_track);
}

/**
 * Sets the automation track.
 */
void
region_set_automation_track (
  ZRegion *          self,
  AutomationTrack * at)
{
  g_return_if_fail (at);
  self->at = at;
  self->at_index = at->index;
  self->track_pos = at->track->pos;
}

void
region_get_type_as_string (
  RegionType type,
  char *     buf)
{
  g_return_if_fail (
    type >= 0 && type <= REGION_TYPE_CHORD);
  switch (type)
    {
    case REGION_TYPE_MIDI:
      strcpy (buf, _("MIDI"));
      break;
    case REGION_TYPE_AUDIO:
      strcpy (buf, _("Audio"));
      break;
    case REGION_TYPE_AUTOMATION:
      strcpy (buf, _("Automation"));
      break;
    case REGION_TYPE_CHORD:
      strcpy (buf, _("Chord"));
      break;
    }
}

/**
 * Returns if the given ZRegion type can exist
 * in TrackLane's.
 */
int
region_type_has_lane (
  const RegionType type)
{
  return
    type == REGION_TYPE_MIDI ||
    type == REGION_TYPE_AUDIO;
}

/**
 * Looks for the ZRegion under the given name.
 *
 * Warning: very expensive function.
 */
ZRegion *
region_find_by_name (
  const char * name)
{
  int i, j, k;
  Track * track;
  AutomationTracklist * atl;
  AutomationTrack * at;
  TrackLane * lane;
  ZRegion * r;
  for (i = 0; i < TRACKLIST->num_tracks; i++)
    {
      track = TRACKLIST->tracks[i];
      atl = &track->automation_tracklist;
      g_warn_if_fail (track);

      for (k = 0; k < track->num_lanes; k++)
        {
          lane = track->lanes[k];

          for (j = 0; j < lane->num_regions; j++)
            {
              r = lane->regions[j];
              if (!g_strcmp0 (r->name, name))
                return r;
            }
        }

      if (track->type == TRACK_TYPE_CHORD)
        {
          for (j = 0; j < track->num_chord_regions;
               j++)
            {
              r = track->chord_regions[j];
              if (!g_strcmp0 (r->name, name))
                return r;
            }
        }

      for (j = 0; j < atl->num_ats; j++)
        {
          at = atl->ats[j];

          for (k = 0; k < at->num_regions; k++)
            {
              r = at->regions[k];
              if (!g_strcmp0 (r->name, name))
                return r;
            }
        }
    }

  return NULL;
}

/**
 * Returns the MidiNote matching the properties of
 * the given MidiNote.
 *
 * Used to find the actual MidiNote in the region
 * from a cloned MidiNote (e.g. when doing/undoing).
 */
MidiNote *
region_find_midi_note (
  ZRegion * r,
  MidiNote * clone)
{
  MidiNote * mn;
  for (int i = 0; i < r->num_midi_notes; i++)
    {
      mn = r->midi_notes[i];

      if (midi_note_is_equal (
            clone, mn))
        return mn;
    }

  return NULL;
}

/**
 * Gets the AutomationTrack using the saved index.
 */
AutomationTrack *
region_get_automation_track (
  ZRegion * region)
{
  Track * track =
    arranger_object_get_track (
      (ArrangerObject *) region);
  g_return_val_if_fail (
    track->automation_tracklist.num_ats >
    region->at_index, NULL);

  return
    track->automation_tracklist.ats[
      region->at_index];
}

/**
 * Print region info for debugging.
 */
void
region_print (
  const ZRegion * self)
{
  char * str =
    g_strdup_printf (
      "%s [%s] - track pos %d - lane pos %d",
      self->name,
      region_type_bitvals[self->type].name,
      self->track_pos,
      self->lane_pos);
  g_message ("%s", str);
  g_free (str);
}

/**
 * Sets ZRegion name (without appending anything to
 * it) to all associated regions.
 */
void
region_set_name (
  ZRegion * self,
  char *   name)
{
  arranger_object_set_name ((ArrangerObject *) self, name);

  for (int i = 0; i < self->num_midi_notes; i++)
    {
      midi_note_set_region (
        self->midi_notes[i], self);
    }
  for (int i = 0; i < self->num_chord_objects; i++)
    {
      chord_object_set_region (
        self->chord_objects[i], self);
    }
  for (int i = 0; i < self->num_aps; i++)
    {
      automation_point_set_region_and_index (
        self->aps[i], self,
        self->aps[i]->index);
    }
}

/**
 * Returns the region at the given position in the
 * given Track.
 *
 * @param at The automation track to look in.
 * @param track The track to look in, if at is
 *   NULL.
 * @param pos The position.
 */
ZRegion *
region_at_position (
  Track    *        track,
  AutomationTrack * at,
  Position *        pos)
{
  ZRegion * region;
  if (track)
    {
      TrackLane * lane;
      for (int i = 0; i < track->num_lanes; i++)
        {
          lane = track->lanes[i];
          for (int j = 0; j < lane->num_regions; j++)
            {
              region = lane->regions[j];
              ArrangerObject * r_obj =
                (ArrangerObject *) region;
              if (position_is_after_or_equal (
                    pos, &r_obj->pos) &&
                  position_is_before_or_equal (
                    pos, &r_obj->end_pos))
                {
                  return region;
                }
            }
        }
    }
  else if (at)
    {
      for (int i = 0; i < at->num_regions; i++)
        {
          region = at->regions[i];
          ArrangerObject * r_obj =
            (ArrangerObject *) region;
          if (position_is_after_or_equal (
                pos, &r_obj->pos) &&
              position_is_before_or_equal (
                pos, &r_obj->end_pos))
            {
              return region;
            }
        }
    }
  return NULL;
}

/**
 * Returns if the position is inside the region
 * or not.
 *
 * @param gframes Global position in frames.
 * @param inclusive Whether the last frame should
 *   be counted as part of the region.
 */
int
region_is_hit (
  const ZRegion * region,
  const long     gframes,
  const int      inclusive)
{
  const ArrangerObject * r_obj =
    (const ArrangerObject *) region;
  return
    r_obj->pos.frames <=
      gframes &&
    ((inclusive &&
      r_obj->end_pos.frames >=
        gframes) ||
     (!inclusive &&
      r_obj->end_pos.frames >
        gframes));
}

/**
 * Returns if any part of the ZRegion is inside the
 * given range, inclusive.
 */
int
region_is_hit_by_range (
  const ZRegion * region,
  const long     gframes_start,
  const long     gframes_end,
  const int      end_inclusive)
{
  const ArrangerObject * obj =
    (const ArrangerObject *) region;
  /* 4 cases:
   * - region start is inside range
   * - region end is inside range
   * - start is inside region
   * - end is inside region
   */
  if (end_inclusive)
    {
      return
        (gframes_start <=
           obj->pos.frames &&
         gframes_end >=
           obj->pos.frames) ||
        (gframes_start <=
           obj->end_pos.frames &&
         gframes_end >=
           obj->end_pos.frames) ||
        region_is_hit (region, gframes_start, 1) ||
        region_is_hit (region, gframes_end, 1);
    }
  else
    {
      return
        (gframes_start <=
           obj->pos.frames &&
         gframes_end >
           obj->pos.frames) ||
        (gframes_start <
           obj->end_pos.frames &&
         gframes_end >
           obj->end_pos.frames) ||
        region_is_hit (region, gframes_start, 0) ||
        region_is_hit (region, gframes_end, 0);
    }
}

/**
 * Copies the data from src to dest.
 *
 * Used when doing/undoing changes in name,
 * clip start point, loop start point, etc.
 */
void
region_copy (
  ZRegion * src,
  ZRegion * dest)
{
  g_free (dest->name);
  dest->name = g_strdup (src->name);

  ArrangerObject * src_obj =
    (ArrangerObject *) src;
  ArrangerObject * dest_obj =
    (ArrangerObject *) dest;

  dest_obj->clip_start_pos = src_obj->clip_start_pos;
  dest_obj->loop_start_pos = src_obj->loop_start_pos;
  dest_obj->loop_end_pos = src_obj->loop_end_pos;
}

/**
 * Converts frames on the timeline (global)
 * to local frames (in the clip).
 *
 * If normalize is 1 it will only return a position
 * from 0 to loop_end (it will traverse the
 * loops to find the appropriate position),
 * otherwise it may exceed loop_end.
 *
 * @param timeline_frames Timeline position in
 *   frames.
 *
 * @return The local frames.
 */
long
region_timeline_frames_to_local (
  ZRegion * region,
  const long     timeline_frames,
  const int      normalize)
{
  long diff_frames;

  ArrangerObject * r_obj =
    (ArrangerObject *) region;

  if (normalize)
    {
      if (region)
        {
          diff_frames =
            timeline_frames -
            r_obj->pos.frames;
          long loop_end_frames =
            r_obj->loop_end_pos.frames;
          long clip_start_frames =
            r_obj->clip_start_pos.frames;
          long loop_size =
            arranger_object_get_loop_length_in_frames (
              r_obj);
          g_return_val_if_fail (
            loop_size > 0, 0);

          diff_frames += clip_start_frames;

          while (diff_frames >= loop_end_frames)
            {
              diff_frames -= loop_size;
            }
        }
      else
        {
          diff_frames = 0;
        }

      return diff_frames;
    }
  else
    {
      if (region)
        {
          diff_frames =
            timeline_frames -
            r_obj->pos.frames;
        }
      else
        {
          diff_frames = 0;
        }

      return diff_frames;
    }
}

/**
 * Generates the filename for this region.
 *
 * MUST be free'd.
 *
 * FIXME logic needs changing
 */
char *
region_generate_filename (ZRegion * region)
{
  return
    g_strdup_printf (
      REGION_PRINTF_FILENAME,
      region->lane->track->name,
      region->name);
}

/**
 * Disconnects the region and anything using it.
 *
 * Does not free the ZRegion or its children's
 * resources.
 */
void
region_disconnect (
  ZRegion * self)
{
  if (CLIP_EDITOR->region == self)
    {
      CLIP_EDITOR->region = NULL;
      EVENTS_PUSH (ET_CLIP_EDITOR_REGION_CHANGED,
                   NULL);
    }
  if (TL_SELECTIONS)
    {
      arranger_selections_remove_object (
        (ArrangerSelections *) TL_SELECTIONS,
        (ArrangerObject *) self);
    }
  for (int i = 0; i < self->num_midi_notes; i++)
    {
      MidiNote * mn = self->midi_notes[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) MA_SELECTIONS,
        (ArrangerObject *) mn);
    }
  for (int i = 0; i < self->num_chord_objects; i++)
    {
      ChordObject * c = self->chord_objects[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) CHORD_SELECTIONS,
        (ArrangerObject *) c);
    }
  for (int i = 0; i < self->num_aps; i++)
    {
      AutomationPoint * ap = self->aps[i];
      arranger_selections_remove_object (
        (ArrangerSelections *) AUTOMATION_SELECTIONS,
        (ArrangerObject *) ap);
    }
  if (ZRYTHM_HAVE_UI)
    {
      /*ARRANGER_WIDGET_GET_PRIVATE (MW_TIMELINE);*/
      /*if (ar_prv->start_object ==*/
            /*(ArrangerObject *) self)*/
        /*ar_prv->start_object = NULL;*/
    }
}

SERIALIZE_SRC (ZRegion, region)
DESERIALIZE_SRC (ZRegion, region)
PRINT_YAML_SRC (ZRegion, region)
