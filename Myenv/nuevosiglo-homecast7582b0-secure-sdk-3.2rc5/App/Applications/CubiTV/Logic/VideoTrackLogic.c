/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#include <Logic/VideoTrackLogic.h>

#include <SvPlayerControllers/SvPlayerVideoTrack.h>
#include <SvPlayerControllers/SvPlayerTrackControllerListener.h>
#include <SvPlayerControllers/SvPlayerTaskControllersListener.h>

#include <Logic/QBLogicFactory.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <fibers/c/fibers.h>
#include <errno.h>

SvLocal SvString
VideoTrackLogicGetName(SvObject self_)
{
    return SVSTRING("VideoTrackLogic");
}

SvLocal SvImmutableArray
VideoTrackLogicGetDependencies(SvObject self_)
{
    return NULL;
}

SvLocal QBAsyncServiceState
VideoTrackLogicGetState(SvObject self_)
{
    VideoTrackLogic self = (VideoTrackLogic) self_;
    return self->serviceState;
}

SvLocal void
VideoTrackLogicStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    VideoTrackLogic self = (VideoTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_running;
}

SvLocal void
VideoTrackLogicStop(SvObject self_, SvErrorInfo *errorOut)
{
    VideoTrackLogic self = (VideoTrackLogic) self_;
    self->serviceState = QBAsyncServiceState_idle;
}

void
VideoTrackLogicSetPlayerTaskControllers(VideoTrackLogic self, SvPlayerTaskControllers controllers)
{
    // remove old listener and release old controllers
    if (self->videoTrackController) {
        SvPlayerTrackControllerRemoveListener(self->videoTrackController, (SvObject) self);
        SVRELEASE(self->videoTrackController);
        self->videoTrackController = NULL;
    }

    if (self->playerTaskControllers) {
        SvPlayerTaskControllersRemoveListener(self->playerTaskControllers, (SvObject) self);
        SVRELEASE(self->playerTaskControllers);
        self->playerTaskControllers = NULL;
    }

    // set new controllers and add listeners to new controllers objects
    if (controllers) {
        // player task controller
        self->playerTaskControllers = SVRETAIN(controllers);
        SvPlayerTaskControllersAddListener(self->playerTaskControllers, (SvObject) self, NULL);

        // video controller
        SvPlayerTrackController currentVideoTrackController =
            SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_video);

        if (currentVideoTrackController) {
            self->videoTrackController = SVRETAIN(currentVideoTrackController);
            SvPlayerTrackControllerAddListener(self->videoTrackController, (SvObject) self, NULL);
        }
    }
}

SvPlayerTaskControllers VideoTrackLogicGetPlayerTaskControllers(VideoTrackLogic self)
{
    return self->playerTaskControllers;
}

SvLocal void
VideoTrackLogicDestroy(void *self_)
{
    VideoTrackLogic self = self_;

    // remove listeners
    if (self->videoTrackController) {
        SvPlayerTrackControllerRemoveListener(self->videoTrackController, (SvObject) self);
    }

    if (self->playerTaskControllers) {
        SvPlayerTaskControllersRemoveListener(self->playerTaskControllers, (SvObject) self);
    }

    // release all objects
    SVTESTRELEASE(self->videoTrackController);
    SVTESTRELEASE(self->playerTaskControllers);
}

SvLocal void
VideoTrackLogicControllersUpdated(SvObject self_)
{
    VideoTrackLogic self = (VideoTrackLogic) self_;

    // update video track controller
    SvPlayerTrackController currentVideoTrackController =
        SvPlayerTaskControllersGetTrackController(self->playerTaskControllers, SvPlayerTrackControllerType_video);

    if (self->videoTrackController != currentVideoTrackController) {
        // release previous one and set new video controller
        if (self->videoTrackController) {
            SvPlayerTrackControllerRemoveListener(self->videoTrackController, (SvObject) self);
            SVRELEASE(self->videoTrackController);
            self->videoTrackController = NULL;
        }

        if (currentVideoTrackController) {
            self->videoTrackController = SVRETAIN(currentVideoTrackController);
            SvPlayerTrackControllerAddListener(self->videoTrackController, (SvObject) self, NULL);
        }
    }
}

SvLocal void
VideoTrackLogicTracksUpdated(SvObject self_, SvPlayerTrackController controller)
{
    VideoTrackLogic self = (VideoTrackLogic) self_;

    VideoTrackLogicSetupVideoTrack(self);
}

SvLocal void
VideoTrackLogicCurrentTrackChanged(SvObject self, SvPlayerTrackController controller, unsigned int idx)
{
}

SvType
VideoTrackLogic_getType(void)
{
    static SvType type = NULL;

    static const struct VideoTrackLogicVTable_ logicVTable = {
        .super_                                = {
            .destroy                           = VideoTrackLogicDestroy
        }
    };

    static struct QBAsyncService_ asyncServiceMethods = {
        .getName         = VideoTrackLogicGetName,
        .getDependencies = VideoTrackLogicGetDependencies,
        .getState        = VideoTrackLogicGetState,
        .start           = VideoTrackLogicStart,
        .stop            = VideoTrackLogicStop
    };

    static const struct SvPlayerTaskControllersListener_ taskControllersListenerMethods = {
        .controllersUpdated = VideoTrackLogicControllersUpdated,
    };

    static const struct SvPlayerTrackControllerListener_ trackControllerListenerMethods = {
        .tracksUpdated       = VideoTrackLogicTracksUpdated,
        .currentTrackChanged = VideoTrackLogicCurrentTrackChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateVirtual("VideoTrackLogic",
                            sizeof(struct VideoTrackLogic_),
                            SvObject_getType(),
                            sizeof(logicVTable),
                            &logicVTable,
                            &type,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            SvPlayerTaskControllersListener_getInterface(), &taskControllersListenerMethods,
                            SvPlayerTrackControllerListener_getInterface(), &trackControllerListenerMethods,
                            NULL);
    }

    return type;
}

VideoTrackLogic
VideoTrackLogicCreate(SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    VideoTrackLogic self = NULL;

    SvType logicClass =
        QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(), VideoTrackLogic_getType(), &error);
    if (!logicClass) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidArgument, error,
                                           "unable to find video track logic");
        goto out;
    }

    self = (VideoTrackLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "unable to allocate %s", SvTypeGetName(logicClass));
        goto out;
    }

out:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

static int calcVideoTrackValue(const SvPlayerVideoTrack track)
{
    SvString codec = SvPlayerVideoTrackGetCodec(track);
    if (codec && strcasecmp(SvStringCString(codec), "h264") == 0)
        return 5000 + SvPlayerVideoTrackGetAVCLevel(track);

    return 1000;
}

static int selectVideoTrack(VideoTrackLogic self)
{
    if (unlikely(!self->playerTaskControllers)) {
        SvLogError("unable to select video track, no SvPlayerTask controller");
        return false;
    }

    if (!self->videoTrackController)
        return -1;

    int max_i = -1;
    int max_value = -1;

    SvArray tracks = SvPlayerTrackControllerGetTracks(self->videoTrackController);
    for (size_t i = 0; i < SvArrayGetCount(tracks); i++) {
        SvPlayerVideoTrack track = (SvPlayerVideoTrack) SvArrayGetObjectAtIndex(tracks, i);
        int val = calcVideoTrackValue(track);
        if (val > max_value) {
            max_i = i;
            max_value = val;
        };
    }
    return max_i;
}

bool VideoTrackLogicSetupVideoTrack(VideoTrackLogic self)
{
    if (unlikely(!self->playerTaskControllers)) {
        SvLogError("unable to select video track, no SvPlayerTask controller");
        return false;
    }

    if (self->videoTrackController) {
        int trackIndex = selectVideoTrack(self);
        SvPlayerTrackControllerSetTrackByIndex(self->videoTrackController, trackIndex);
    }

    return true;
}

