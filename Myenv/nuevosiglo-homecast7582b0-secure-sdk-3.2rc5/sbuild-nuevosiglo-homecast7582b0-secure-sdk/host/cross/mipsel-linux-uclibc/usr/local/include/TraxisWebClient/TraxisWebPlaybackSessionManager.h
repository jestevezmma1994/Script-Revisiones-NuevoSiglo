/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TRAXIS_WEB_PLAYBACK_SESSION_MANAGER_H_
#define TRAXIS_WEB_PLAYBACK_SESSION_MANAGER_H_

#include <SvDataBucket2/SvDBRawObject.h>
#include <SvFoundation/SvInterface.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <fibers/c/fibers.h>

typedef enum {
    TraxisWebPlaybackSessionState_creating,
    TraxisWebPlaybackSessionState_created,
    TraxisWebPlaybackSessionState_destroying,
    TraxisWebPlaybackSessionState_destroyed,
    TraxisWebPlaybackSessionState_error,
} TraxisWebPlaybackSessionState;

typedef enum {
    TraxisWebPlaybackSessionError_none,
    TraxisWebPlaybackSessionError_incorrectServerResponse,
    TraxisWebPlaybackSessionError_serverConnectionError,
    TraxisWebPlaybackSessionError_unauthorized,
} TraxisWebPlaybackSessionError;

typedef enum {
    TraxisWebPlaybackSessionEndCode_unknown = 0,
    TraxisWebPlaybackSessionEndCode_streamEnd = 1,
    TraxisWebPlaybackSessionEndCode_byCustomer = 2,
    TraxisWebPlaybackSessionEndCode_notAuthorized = 4,
    TraxisWebPlaybackSessionEndCode_playbackError = 5,

} TraxisWebPlaybackSessionEndCode;

typedef struct TraxisWebPlaybackSessionManager_s* TraxisWebPlaybackSessionManager;
typedef struct TraxisWebPlaybackSession_s* TraxisWebPlaybackSession;

struct TraxisWebPlaybackSessionListener_ {
    void (*stateChanged)(SvObject self_, TraxisWebPlaybackSession session);
};
typedef struct TraxisWebPlaybackSessionListener_* TraxisWebPlaybackSessionListener;

struct TraxisWebPlaybackSessionUpdateParams {
    double pos;
    double speed;
   int trackId;
};
typedef struct TraxisWebPlaybackSessionUpdateParams TraxisWebPlaybackSessionUpdateParams;

SvInterface TraxisWebPlaybackSessionListener_getInterface(void);

TraxisWebPlaybackSessionManager TraxisWebPlaybackSessionManagerCreate(TraxisWebSessionManager sessionMan);

void TraxisWebPlaybackSessionManagerStart(TraxisWebPlaybackSessionManager self, SvScheduler scheduler);

void TraxisWebPlaybackSessionManagerStop(TraxisWebPlaybackSessionManager self);

TraxisWebPlaybackSession TraxisWebPlaybackSessionManagerCreateSession(TraxisWebPlaybackSessionManager self, SvDBRawObject content);

void TraxisWebPlaybackSessionManagerDestroySession(TraxisWebPlaybackSessionManager self, TraxisWebPlaybackSession session, TraxisWebPlaybackSessionEndCode code, SvString message);

void TraxisWebPlaybackSessionUpdate(TraxisWebPlaybackSession self, TraxisWebPlaybackSessionUpdateParams* params);

TraxisWebPlaybackSessionState TraxisWebPlaybackSessionGetState(TraxisWebPlaybackSession self);

TraxisWebPlaybackSessionError TraxisWebPlaybackSessionGetError(TraxisWebPlaybackSession self);

SvString TraxisWebPlaybackSessionGetErrorStr(TraxisWebPlaybackSession self);

SvDBRawObject TraxisWebPlaybackSessionGetContent(TraxisWebPlaybackSession self);

void TraxisWebPlaybackSessionAddListener(TraxisWebPlaybackSession self, SvObject listener);
void TraxisWebPlaybackSessionRemoveListener(TraxisWebPlaybackSession self, SvObject listener);

#endif
