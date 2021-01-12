/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "sanityChecker.h"
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <fibers/c/fibers.h>
#include <Services/core/appState.h>
#include <safeFiles.h>
#include <main.h>
#include <sys/stat.h>
#include <sys/types.h>


struct QBSanityChecker_t {
    struct SvObject_ super_;
    SvFiber fiber;
    SvFiberTimer longTimer;
    bool wasAppWorking;
    AppGlobals appGlobals;
};

typedef enum {
    QBSanityCheckerCounter_crash,
    QBSanityCheckerCounter_start,
    QBSanityCheckerCounter_longRun,
} QBSanityCheckerCounter;

//Needed to handle SIGSEGV etc. we don't have any easy access to AppGlobals in sig handlers
static QBSanityChecker staticInstance;

SvLocal void QBSanityCheckerAppStateChanged(SvGenericObject self_, uint64_t attrs);

SvLocal void QBSanityChecker__dtor__(void* self_)
{
    QBSanityChecker self = self_;
    if (self->fiber)
        SvFiberDestroy(self->fiber);
}

SvLocal SvType QBSanityChecker_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSanityChecker__dtor__
    };
    static SvType type = NULL;
    static const struct QBAppStateServiceListener_t appStateMethods = {
        .stateChanged = QBSanityCheckerAppStateChanged,
    };
    if (!type) {
        SvTypeCreateManaged("QBSanityChecker",
                            sizeof(struct QBSanityChecker_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAppStateServiceListener_getInterface(), &appStateMethods,
                            NULL);
    }

    return type;
}

static const char* fileNames[] = {
    [QBSanityCheckerCounter_start] = "/etc/vod/sanityChecker/startCounter",
    [QBSanityCheckerCounter_crash] = "/etc/vod/sanityChecker/crashCounter",
    [QBSanityCheckerCounter_longRun] = "/etc/vod/sanityChecker/longCounter",
};

SvLocal void QBSanityCheckerResetCounters(QBSanityChecker self)
{
    for (unsigned i = 0; i < sizeof(fileNames) / sizeof(fileNames[0]); i++) {
        QBBufferToFile(fileNames[i], "0");
    }
}

SvLocal void QBSanityCheckerChangeCounter(QBSanityChecker self, QBSanityCheckerCounter cnt, int by)
{
    const char* fileName = fileNames[cnt];
    char *buf;
    QBFileToBuffer(fileName, &buf);
    int currentCnt = 0;
    if (buf) {
        currentCnt = atoi(buf);
        if (currentCnt < 0)
            currentCnt = 0;
    }

    currentCnt += by;
    if (currentCnt < 0)
        currentCnt = 0;

    char buf2[40];
    snprintf(buf2, sizeof(buf2), "%i", currentCnt);
    QBBufferToFile(fileName, buf2);

    free(buf);
}

SvLocal void QBSanityCheckerStep(void *self_)
{
    QBSanityChecker self = self_;
    SvFiberDeactivate(self->fiber);
    if (self->longTimer && SvFiberEventIsActive(self->longTimer)) {
        SvFiberEventDestroy(self->longTimer);
        self->longTimer = NULL;
        QBSanityCheckerResetCounters(self);
    }
}

SvLocal void QBSanityCheckerAppStateChanged(SvGenericObject self_, uint64_t attrs)
{
    QBSanityChecker self = (QBSanityChecker) self_;
    if (!(attrs & QBAppState_InitStateChanged))
        return;

    if (self->wasAppWorking)
        return;

    if (QBAppStateServiceGetAppState(self->appGlobals->appState) == QBAppState_working) {
        QBSanityCheckerChangeCounter(self, QBSanityCheckerCounter_start, -1);
        SvFiberTimerActivateAfter(self->longTimer, SvTimeFromMs(1 * 3600 * 1000));
        self->wasAppWorking = true;
    }
}

QBSanityChecker QBSanityCheckerCreate(AppGlobals appGlobals)
{
    QBSanityChecker self = (QBSanityChecker) SvTypeAllocateInstance(QBSanityChecker_getType(), NULL);
    self->appGlobals = appGlobals;

    staticInstance = self;

    mkdir("/etc/vod/sanityChecker", 0777);

    QBSanityCheckerChangeCounter(self, QBSanityCheckerCounter_start, 1);
    QBSanityCheckerChangeCounter(self, QBSanityCheckerCounter_longRun, 1);

    return self;
}

void QBSanityCheckerStart(QBSanityChecker self)
{
    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBSanityChecker", QBSanityCheckerStep, self);
    self->longTimer = SvFiberTimerCreate(self->fiber);
    QBAppStateServiceAddListener(self->appGlobals->appState, (SvGenericObject) self);
}

void QBSanityCheckerStop(QBSanityChecker self)
{
    QBAppStateServiceRemoveListener(self->appGlobals->appState, (SvGenericObject) self);
    QBSanityCheckerChangeCounter(self, QBSanityCheckerCounter_longRun, -1);
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->longTimer = NULL;
    }
}

void QBSanityCheckerOnCrash(void)
{
    QBSanityCheckerChangeCounter(staticInstance, QBSanityCheckerCounter_crash, 1);
}
