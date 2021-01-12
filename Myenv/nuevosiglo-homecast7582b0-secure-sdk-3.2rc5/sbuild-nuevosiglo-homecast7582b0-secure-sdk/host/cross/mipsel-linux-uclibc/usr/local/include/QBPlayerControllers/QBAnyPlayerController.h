/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBANYPLAYERCONTROLLER_H_
#define QBANYPLAYERCONTROLLER_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <SvPlayerKit/SvContent.h>

/*
 * Controller
 */
struct QBAnyPlayerController_ {
    void (*start)(SvObject self_);
    void (*stop)(SvObject self_);
    void (*changeSpeed)(SvObject self_, double speed);
    void (*setPlayer)(SvObject self_, SvObject player);
    void (*setContent)(SvObject self_, SvObject content);
    void (*finish)(SvObject self_);
    void (*onPlaybackFinished)(SvObject self_, bool looped);
    bool (*handleInput)(SvObject self_, SvInputEvent event);
    void (*gotEvent)(SvObject self_, SvString name, void *arg);
    void (*onError)(SvObject self_);
    void (*setOpt)(SvObject self_, SvString name, void *arg);
};
typedef struct QBAnyPlayerController_* QBAnyPlayerController;

SvInterface QBAnyPlayerController_getInterface(void);

/*
 * Controller Listener
 */

typedef enum {
    QBAnyPlayerControllerEventID_entitlementExpired = 0,
    QBAnyPlayerControllerEventID_notEntitled,
    QBAnyPlayerControllerEventID_error,
    QBAnyPlayerControllerEventID_noTunerAccess,
    QBAnyPlayerControllerEventID_playlistFinished,
    QBAnyPlayerControllerEventID_playlistError,
    QBAnyPlayerControllerEventID_finished
} QBAnyPlayerControllerEventID;

struct QBAnyPlayerControllerListener_ {
    void (*onControllerEvent)(SvObject self_, int eventId, SvObject data);
};
typedef struct QBAnyPlayerControllerListener_* QBAnyPlayerControllerListener;

SvInterface QBAnyPlayerControllerListener_getInterface(void);

#endif

