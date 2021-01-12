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

#ifndef PLAYER_EVENTS_DECRYPTION_H
#define PLAYER_EVENTS_DECRYPTION_H

#include <stdbool.h>

struct player_event_decryption_failed_s {
    int error_code;
    bool external_reason;
};

/// Inform the application, that decryption has failed in the last few moments.
/// The content is not being decrypted (is being discarded).
/// This notification can arrive again (after some time), or even repeatedly (with some frequency,
/// usually at least one second), if the "non-decryptable" condition is still present.
/// The content might become decryptable again (after some time), but this notification
/// has nothing to do with it.
///
/// Params is: const struct player_event_decryption_failed_s*
#define PLAYER_EVENT_DECRYPTION_FAILED   "decryption:failed"

/// Inform the application that the decryption is now working properly (or that no decryption is needed anymore).
/// Only sent if DECRYPTION_FAILED was sent previously.
/// Params is: null
#define PLAYER_EVENT_DECRYPTION_OK   "decryption:ok"


/// Inform the application that the content needs to be decrypted.
/// Params is: null
#define PLAYER_EVENT_DECRYPTION_ON   "decryption:on"

/// Inform the application that the content does NOT need to be decrypted.
/// Params is: null
#define PLAYER_EVENT_DECRYPTION_OFF   "decryption:off"


/// Inform the application that descrambled content is not valid. This could be a result of bad CWs
#define PLAYER_EVENT_DECRYPTION_DESCRAMBLING_FAILED "decryption:descrambling_failed"

/// Inform the application that descrambled content is valid.
#define PLAYER_EVENT_DECRYPTION_DESCRAMBLING_OK "decryption:descrambling_ok"

/// Inform the application that we don't support any of CA descriptors found in PMT
#define PLAYER_EVENT_DECRYPTION_NONE_SUPPORTED_CAID_FOUND "decryption:none_supported_caid_found"

#endif // #ifndef PLAYER_EVENTS_DECRYPTION_H
