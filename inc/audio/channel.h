/*
 * Copyright (C) 2018-2019 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __AUDIO_CHANNEL_H__
#define __AUDIO_CHANNEL_H__

/**
 * \file
 *
 * API for Channel, representing a channel strip on the mixer.
 */

#include "config.h"

#include "audio/fader.h"
#include "plugins/plugin.h"
#include "utils/audio.h"
#include "utils/yaml.h"

#include <gdk/gdk.h>

#ifdef HAVE_JACK
#include <jack/jack.h>
#endif

/**
 * @addtogroup audio
 *
 * @{
 */

#define MASTER_POS -1 /* master channel special position */
#define FOREACH_STRIP for (int i = 0; i < STRIP_SIZE; i++)
#define FOREACH_AUTOMATABLE(ch) for (int i = 0; i < ch->num_automatables; i++)
#define MAX_FADER_AMP 1.42f
#define channel_get_fader_automatable(ch) \
  channel_get_automatable (ch, \
                           AUTOMATABLE_TYPE_CHANNEL_FADER)

typedef struct _ChannelWidget ChannelWidget;
typedef struct Track Track;
typedef struct _TrackWidget TrackWidget;

typedef enum ChannelType
{
  CT_AUDIO, ///< audio generated by audio data in region
  CT_MIDI, ///< audio generated by plugin
  CT_BUS, ///< audio received from input
  CT_MASTER ///< master (behaves like a bus)
} ChannelType;

typedef struct Channel
{
  /**
   * Unique IDs useful in serialization.
   *
   * Master must always be 0.
   */
  int                  id;


  /**
   * The channel strip.
   *
   * Note: the first plugin is special in MIDI channels.
   */
  int                  plugin_ids[STRIP_SIZE];
  Plugin *             plugins[STRIP_SIZE]; ///< cache

  /**
   * Type of channel this is.
   */
  ChannelType          type;

  Fader                fader;

  /* These are for plugins to connect to.
   *
   * Processing starts at the first plugin with a clean
   * buffer, and if any ports are connected as that
   * plugin's input, their buffers are added to the first
   * plugin
   */
  StereoPorts *        stereo_in;  ///< l & r input ports, cache

  /**
   * These should be serialized instead of
   * stereo_in.
   */
  //int                  stereo_in_l_id;
  //int                  stereo_in_r_id;

  /**
   * MIDI in port ID.
   *
   * This port is for receiving MIDI signals from
   * an external MIDI source.
   */
  int                  midi_in_id;

  /**
   * MIDI piano roll input port ID.
   *
   * This port is for receiving MIDI signals from
   * the piano roll (i.e., MIDI notes inside
   * regions).
   */
  int                  piano_roll_id;

  /**
   * Cache.
   */
  Port *               midi_in;
  Port *               piano_roll;

  /** Flag used while processing. */
  int                   filled_stereo_in_bufs;


  /*
   * The last plugin should connect to this.
   *
   * Plugins are processed slot-by-slot, and if nothing
   * is connected here it will simply remain an empty
   * buffer, i.e., channel will produce no sound
   * */
  StereoPorts *        stereo_out;

  /** Output channel to route signal to. */
  int                  output_id;

  /** Cache. */
  Channel *            output;

  /** Track associated with this channel. */
  int                  track_id;

  /** Cache. */
  Track *              track;

  //int                  enabled; ///< enabled or not

  /**
   * Automatables for this channel to be generated
   * at run time (amp, pan, mute, etc.).
   */
  int                  automatable_ids[40];
  Automatable *        automatables[40]; ///< cache
  int                  num_automatables;

  //int                  visible; ///< whether visible or not
  ChannelWidget *      widget; ///< the channel widget

  /**
   * Whether record was set automatically when the channel
   * was selected.
   *
   * This is so that it can be unset when selecting another
   * channel. If we don't do this all the channels end up
   * staying on record mode.
   */
  int                  record_set_automatically;
} Channel;

static const cyaml_strval_t
channel_type_strings[] =
{
	{ "Audio",       CT_AUDIO    },
	{ "MIDI",        CT_MIDI   },
	{ "Bus",         CT_BUS    },
	{ "Master",      CT_MASTER   },
};

static const cyaml_schema_field_t
channel_fields_schema[] =
{
	CYAML_FIELD_INT (
    "id", CYAML_FLAG_DEFAULT,
    Channel, id),
  CYAML_FIELD_SEQUENCE_FIXED (
    "plugin_ids", CYAML_FLAG_DEFAULT,
    Channel, plugin_ids,
    &int_schema, STRIP_SIZE),
  CYAML_FIELD_ENUM (
    "type", CYAML_FLAG_DEFAULT,
    Channel, type, channel_type_strings,
    CYAML_ARRAY_LEN (channel_type_strings)),
  CYAML_FIELD_MAPPING (
    "fader", CYAML_FLAG_DEFAULT,
    Channel, fader, fader_fields_schema),
	CYAML_FIELD_MAPPING_PTR (
    "stereo_in", CYAML_FLAG_POINTER,
    Channel, stereo_in,
    stereo_ports_fields_schema),
	CYAML_FIELD_INT (
    "midi_in_id", CYAML_FLAG_DEFAULT,
    Channel, midi_in_id),
	CYAML_FIELD_INT (
    "piano_roll_id", CYAML_FLAG_DEFAULT,
    Channel, piano_roll_id),
	CYAML_FIELD_MAPPING_PTR (
    "stereo_out", CYAML_FLAG_POINTER,
    Channel, stereo_out,
    stereo_ports_fields_schema),
	CYAML_FIELD_INT (
    "output_id", CYAML_FLAG_DEFAULT,
    Channel, output_id),
	CYAML_FIELD_INT (
    "track_id", CYAML_FLAG_DEFAULT,
    Channel, track_id),
  CYAML_FIELD_SEQUENCE_COUNT (
    "automatable_ids", CYAML_FLAG_DEFAULT,
    Channel, automatable_ids, num_automatables,
    &int_schema, 0, CYAML_UNLIMITED),
	CYAML_FIELD_INT (
    "visible", CYAML_FLAG_DEFAULT,
    Channel, track_id),

	CYAML_FIELD_END
};

static const cyaml_schema_value_t
channel_schema =
{
	CYAML_VALUE_MAPPING (
    CYAML_FLAG_POINTER,
	  Channel, channel_fields_schema),
};

void
channel_init_loaded (Channel * channel);

/**
 * Handles the recording logic inside the process cycle.
 */
void
channel_handle_recording (Channel * self);

/**
 * Connects the channel's ports.
 */
void
channel_connect (
  Channel * ch);

void
channel_set_phase (void * channel, float phase);

float
channel_get_phase (void * channel);


void
channel_set_pan (void * _channel, float pan);

/**
 * Adds to (or subtracts from) the pan.
 */
void
channel_add_pan (void * _channel, float pan);

float
channel_get_pan (void * _channel);

float
channel_get_current_l_db (void * _channel);

float
channel_get_current_r_db (void * _channel);

void
channel_set_current_l_db (Channel * channel, float val);

void
channel_set_current_r_db (Channel * channel, float val);

/**
 * Sets fader to 0.0.
 */
void
channel_reset_fader (Channel * channel);

void
channel_prepare_process (Channel * channel);

/**
 * Creates a channel of the given type with the
 * given label.
 *
 * This should not be creating a track. A track
 * should be created from an existing channel.
 *
 * @param add_to_project Whether the channel should
 *   be added to the project or not. This should be
 *   true unless the channel will be transient (e.g.
 *   in an undo action).
 */
Channel *
channel_new (
  ChannelType type,
  char *      label,
  int         add_to_project);

/**
 * The process function prototype.
 * Channels must implement this.
 * It is used to perform processing of the audio signal at every cycle.
 *
 * Normally, the channel will call the process func on each of its plugins
 * in order.
 */
void
channel_process (Channel * channel);

/**
 * Adds given plugin to given position in the strip.
 *
 * The plugin must be already instantiated at this
 * point.
 *
 * @param channel The Channel.
 * @param pos The position in the strip starting
 *   from 0.
 * @param plugin The plugin to add.
 * @param confirm Confirm if an existing plugin
 *   will be overwritten.
 * @param gen_automatables Generatate plugin
 *   automatables.
 *   To be used when creating a new plugin only.
 * @param recalc_graph Recalculate mixer graph.
 *
 * @return 1 if plugin added, 0 if not.
 */
int
channel_add_plugin (
  Channel * channel,
  int       pos,
  Plugin *  plugin,
  int       confirm,
  int       gen_automatables,
  int       recalc_graph);

/**
 * Updates the output of the Channel (where the
 * Channel routes to.
 */
void
channel_update_output (
  Channel * ch,
  Channel * output);

/**
 * Returns the index of the last active slot.
 */
int
channel_get_last_active_slot_index (Channel * channel);

/**
 * Returns the index on the mixer.
 */
int
channel_get_index (Channel * channel);

/**
 * Returns the plugin's strip index on the channel
 */
int
channel_get_plugin_index (Channel * channel,
                          Plugin *  plugin);

/**
 * Connects or disconnects the MIDI editor key press port to the channel's
 * first plugin
 */
void
channel_reattach_midi_editor_manual_press_port (Channel * channel,
                                                int     connect);

/**
 * Convenience method to get the first active plugin in the channel
 */
Plugin *
channel_get_first_plugin (Channel * channel);

/**
 * Convenience function to get the fader automatable of the channel.
 */
Automatable *
channel_get_automatable (Channel *       channel,
                         AutomatableType type);

/**
 * Generates automatables for the channel.
 *
 * Should be called as soon as the track is
 * created.
 */
void
channel_generate_automatables (Channel * channel);

/**
 * Removes a plugin at pos from the channel.
 *
 * If deleting_channel is 1, the automation tracks
 * associated with he plugin are not deleted at
 * this time.
 *
 * This function will always recalculate the graph
 * in order to avoid situations where the plugin
 * might be used during processing.
 *
 * @param deleting_plugin
 * @param deleting_channel
 */
void
channel_remove_plugin (
  Channel * channel,
  int pos,
  int deleting_plugin,
  int deleting_channel);

Channel *
channel_clone (
  Channel * ch);

/**
 * Disconnects the channel from the processing
 * chain.
 *
 * This should be called immediately when the
 * channel is getting deleted, and channel_free
 * should be designed to be called later after
 * an arbitrary delay.
 */
void
channel_disconnect (Channel * channel);

/**
 * Frees the channel.
 *
 * Channels should never be free'd by themselves
 * in normal circumstances. Use track_free to
 * free them.
 */
void
channel_free (Channel * channel);

/**
 * @}
 */

#endif
