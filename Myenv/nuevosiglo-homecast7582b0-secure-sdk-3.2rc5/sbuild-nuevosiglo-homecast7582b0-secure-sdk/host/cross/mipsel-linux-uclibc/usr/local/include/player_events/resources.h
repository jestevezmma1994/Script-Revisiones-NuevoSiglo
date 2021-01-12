/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef PLAYER_EVENTS_RESOURCES_H
#define PLAYER_EVENTS_RESOURCES_H

#include <stdbool.h>

struct player_resource_stun_s {
    bool use_any_server;    /// [out]
    bool use_specific_server; /// [out]
    bool use_local_port;    /// [out]
    const char *mapped_ip;  /// [out]
};

struct player_event_resources_needed_s {
    bool tuner;  /// [in]
    int tuner_num; /// [out]

    bool stun; /// [in]
    struct player_resource_stun_s stun_out; /// [out]
};

/// Inform the application, that to continue playback, specific resources are needed.
/// The application needs to fill out requested [out] fields.
///
/// Params is: struct player_event_resources_needed_s*
#define PLAYER_EVENT_RESOURCES_NEEDED   "resources:needed"

#endif // #ifndef PLAYER_EVENTS_RESOURCES_H
