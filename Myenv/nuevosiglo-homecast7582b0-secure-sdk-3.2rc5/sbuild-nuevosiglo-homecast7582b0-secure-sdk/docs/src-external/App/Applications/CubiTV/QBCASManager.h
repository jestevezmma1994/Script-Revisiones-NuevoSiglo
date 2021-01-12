/*****************************************************************************
** Cubiware K.K. Software License Version 1.1
**
** Copyright (C) 2009-2015 Cubiware K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware K.K.
**
** Any User wishing to make use of this Software must contact Cubiware K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBDRMPOPUPMANAGER_H_
#define QBDRMPOPUPMANAGER_H_

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBPVRRecording.h>
#include <stdbool.h>

typedef const struct QBCASPopupManager_ {
    void (*playbackEvent)(SvGenericObject self_, SvString name, void *arg);
    void (*contentChanged)(SvGenericObject self_);
    void (*visibilityChanged)(SvGenericObject self_, bool hidden);
    void (*start)(SvGenericObject self_);
    void (*stop)(SvGenericObject self_);
} *QBCASPopupManager;

SvInterface QBCASPopupManager_getInterface(void);

typedef const struct QBCASPVRManager_ {
    void (*recordingAdded)(SvGenericObject self_, const char* caller);
    void (*start)(SvGenericObject self_);
    void (*stop)(SvGenericObject self_);
} *QBCASPVRManager;

SvInterface QBCASPVRManager_getInterface(void);

typedef const struct QBCASPVRPlaybackMonitor_ {
    /**
     * PVR recording playback started
     *
     * @param[in] self_         PVR playback monitor handle
     * @param[in] recording     recording
     **/
    void (*playbackStarted)(SvGenericObject self_, QBPVRRecording recording);

    /**
     * PVR recording playback stopped
     *
     * @param[in] self_         PVR playback monitor handle
     * @param[in] recording     recording
     **/
    void (*playbackStopped)(SvGenericObject self_, QBPVRRecording recording);
} *QBCASPVRPlaybackMonitor;

SvInterface QBCASPVRPlaybackMonitor_getInterface(void);

void QBCASCreateManagers(AppGlobals appGlobals);
SvGenericObject QBCASCreateAccessPlugin(AppGlobals appGlobals);

#endif /* QBDRMPOPUPMANAGER_H_ */
