/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef PLAYER_OPTS_SUBS_SNIFFER_H
#define PLAYER_OPTS_SUBS_SNIFFER_H

#include <SvPlayerKit/SvClosedCaptionSubtitles.h>
#include <SvPlayerKit/SvChbuf.h>
#include <stdint.h>

struct player_opt_closed_caption_subs_sniffer_s {
    void (*push_data) (void *target, const QBClosedCaptionData *data);
    void (*flush) (void *target);
    void *target;
};

/// Instruct the player to start returning closed caption subtitles with the given function "push_data".
#define PLAYER_OPT_CLOSED_CAPTION_SUBS_SNIFFER_START   "closed_caption_subs_sniffer:start"
/// Instruct the player to stop returning closed caption subtitles with the given function "push_data".
#define PLAYER_OPT_CLOSED_CAPTION_SUBS_SNIFFER_STOP    "closed_caption_subs_sniffer:stop"


struct player_opt_subs_sniffer_s {
    void (*push_data) (void *target, SvChbuf chdata, uint64_t pts, short stream_id, int64_t duration90k);
    void (*flush) (void *target);
    void *target;
};

/// Usage:
/// const struct player_opt_subs_sniffer_s  params = {
///   .push_data = &some_fun,
///   .flush = &some_other_fun,
///   .target = some_object,
/// };
/// int res = set_opt(player, PLAYER_OPT_SUBS_SNIFFER_START, &params);
/// if (res < 0) {
///   // not supported!
/// };
/// Instruct the player to start returning subtitles with the given function "push_data".
#define PLAYER_OPT_SUBS_SNIFFER_START   "subs_sniffer:start"
/// Instruct the player to stop returning subtitles with the given function "push_data".
#define PLAYER_OPT_SUBS_SNIFFER_STOP    "subs_sniffer:stop"

#endif // #ifndef PLAYER_OPTS_SUBS_SNIFFER_H
