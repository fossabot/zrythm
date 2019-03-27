/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __UNDO_MOVE_MIDI_ARRANGER_SELECTIONS_ACTION_H__
#define __UNDO_MOVE_MIDI_ARRANGER_SELECTIONS_POS_ACTION_H__

#include "actions/undoable_action.h"


typedef struct MoveMidiArrangerSelectionsAction
{
  UndoableAction              parent_instance;

  /** Ticks moved. */
  long        ticks;

  /** Delta of note value. */
  int         delta;

  /** Notes acting upon. */
  int         note_ids[600];
  int         num_notes;
} MoveMidiArrangerSelectionsAction;

UndoableAction *
move_midi_arranger_selections_action_new (
  long ticks,
  int  delta);

void
move_midi_arranger_selections_action_do (
	MoveMidiArrangerSelectionsAction * self);

void
move_midi_arranger_selections_action_undo (
	MoveMidiArrangerSelectionsAction * self);

void
move_midi_arranger_selections_action_free (
	MoveMidiArrangerSelectionsAction * self);

#endif