/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef PLAYER_OPTS_GENERIC_INPUT_H
#define PLAYER_OPTS_GENERIC_INPUT_H

/// Usage:
/// int accuracy_sec = 5; // try to provide at least 5 second accuracy, use -1 to disable.
/// set_opt(player, PLAYER_OPT_GENERIC_INPUT_NEED_PROGRESS, &accuracy_sec);
/// Instruct the player to gather progress info, that can be later acquired with "GET_PROGRESS".
#define PLAYER_OPT_GENERIC_INPUT_NEED_PROGRESS   "generic_input:progress:need"

/// Usage:
/// double progress = -1.0;
/// int res = set_opt(player, PLAYER_OPT_GENERIC_INPUT_GET_PROGRESS, &progress);
/// if (res == 0) {
///   assert(progress >= 0.0);
///   assert(progress <= 1.0);
/// };
/// Get playback progress.
/// Progress means fraction of content already processed, either by means of time, or size.
/// e.g. 0 means just started. 1 means already finished.
/// Note, that this is just a statistic, and so might be off from the real value.
/// Use eos() callbacks to detect real end of content streaming.
#define PLAYER_OPT_GENERIC_INPUT_GET_PROGRESS   "generic_input:progress:get"

/// Instruct player to drop all data from input source.
#define PLAYER_OPT_GENERIC_INPUT_DROP           "generic_input:drop"

/// Usage:
/// int bitrate;
/// set_opt(player, PLAYER_OPT_GENERIC_INPUT_GET_BITRATE, &bitrate)
///
/// Get average input data bitrate calculated since playback start.
#define PLAYER_OPT_GENERIC_INPUT_GET_BITRATE    "generic_input:bitrate:get"

#endif // #ifndef PLAYER_OPTS_GENERIC_INPUT_H
