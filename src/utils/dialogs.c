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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __UTILS_DIALOGS_H__
#define __UTILS_DIALOGS_H__

#include "gui/widgets/main_window.h"
#include "project.h"
#include "utils/dialogs.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

/**
 * Creates and returns an open project dialog.
 */
GtkDialog * dialogs_get_open_project_dialog (
  GtkWindow * parent)
{
  GtkFileChooserAction action =
    GTK_FILE_CHOOSER_ACTION_OPEN;

  GtkWidget * dialog =
    gtk_file_chooser_dialog_new (
      _("Open Project"),
      GTK_WINDOW (MAIN_WINDOW),
      action,
      _("_Cancel"),
      GTK_RESPONSE_CANCEL,
      _("_Open"),
      GTK_RESPONSE_ACCEPT,
      NULL);

  gtk_file_chooser_set_filename (
    GTK_FILE_CHOOSER (dialog),
    PROJECT->dir);

  return GTK_DIALOG (dialog);
}

/**
 * Creates and returns the overwrite plugin dialog.
 */
GtkDialog * dialogs_get_overwrite_plugin_dialog (
  GtkWindow * parent)
{
  GtkDialogFlags flags =
    GTK_DIALOG_MODAL |
    GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkDialog * dialog =
    GTK_DIALOG (
      gtk_dialog_new_with_buttons (
        _("Overwrite Plugin"),
        GTK_WINDOW (MAIN_WINDOW),
        flags,
        _("_OK"),
        GTK_RESPONSE_ACCEPT,
        _("_Cancel"),
        GTK_RESPONSE_REJECT,
        NULL));
  GtkWidget * box =
    gtk_dialog_get_content_area (dialog);
  GtkWidget * label =
    g_object_new (
      GTK_TYPE_LABEL,
      "label",
      _("A plugin already exists at the selected "
        "slot. Overwrite it?"),
      "margin-start", 8,
      "margin-end", 8,
      "margin-top", 4,
      "margin-bottom", 8,
      NULL);
  gtk_widget_set_visible (label, 1);
  gtk_container_add (
    GTK_CONTAINER (box), label);
  return dialog;
}

#endif
