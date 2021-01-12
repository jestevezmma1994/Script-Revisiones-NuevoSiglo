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

#include "QBViewRightWebManager.h"
#include <main.h>
#include <QBViewRightWeb.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <QBConf.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvString.h>
#include <sys/file.h>
#include <sys/stat.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 2, "QBViewRightWebServiceLogLevel", "");

#define VIEWRIGHT_WEB_STORAGE_PATH "/etc/vod/casData/"

#define log_error(fmt, ...) if (env_log_level() >= 1) { SvLogError(COLBEG() "QBViewRightWeb :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); }
#define log_warn(fmt, ...) if (env_log_level() >= 2) { SvLogWarning(COLBEG() "QBViewRightWeb :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); }
#define log_state(fmt, ...) if (env_log_level() >= 3) { SvLogWarning(COLBEG() "QBViewRightWeb :: %s " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); }
#define log_fun(fmt, ...) if (env_log_level() >= 4) { SvLogNotice(COLBEG() "QBViewRightWeb :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }
#define log_debug(fmt, ...) if (env_log_level() >= 5) { SvLogNotice(COLBEG() "QBViewRightWeb :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }


struct QBViewRightWebService_s {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvWeakReference viewRightWeb;
    SvWeakReference listener;
    SvData uniqueIdentifier;
    QBViewRightWebState state;
};

SvLocal void QBViewRightWebService__dtor__(void *self_)
{
    log_fun();
    QBViewRightWebService self = (QBViewRightWebService) self_;
    SVTESTRELEASE(self->listener);
    SVTESTRELEASE(self->uniqueIdentifier);
}

SvLocal void QBViewRightWebServiceNotifyListener(QBViewRightWebService self, int errorCode)
{
    if (self->listener) {
        SvObject listener = SvWeakReferenceTakeReferredObject(self->listener);
        SvInvokeInterface(QBViewRightWebListener, listener, stateChanged, self->state, errorCode);
        SVRELEASE(listener);
    }
}

SvLocal void QBViewRightWebServiceStateChanged(SvGenericObject self_, QBViewRightWebState newState, int errorCode)
{
    log_fun();
    QBViewRightWebService self = (QBViewRightWebService) (self_);

    QBViewRightWeb viewRightWeb = (QBViewRightWeb) SvWeakReferenceTakeReferredObject(self->viewRightWeb);

    if (!self->uniqueIdentifier && newState != QBViewRightWebState_Created && newState != QBViewRightWebState_InitializationProcess) {
        SVTESTRELEASE(self->uniqueIdentifier);
        self->uniqueIdentifier = QBViewRightWebGetUniqueIdentifier(viewRightWeb);
    }

    self->state = newState;
    QBViewRightWebServiceNotifyListener(self, errorCode);
    SVRELEASE(viewRightWeb);
}

SvLocal SvType QBViewRightWebService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightWebService__dtor__
    };

    static const struct QBViewRightWebListener_s viewRightWebListenerMethods = {
        .stateChanged = QBViewRightWebServiceStateChanged
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBViewRightWebService",
                            sizeof(struct QBViewRightWebService_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBViewRightWebListener_getInterface(), &viewRightWebListenerMethods,
                            NULL);
    }
    return type;
}

QBViewRightWebService QBViewRightWebServiceCreate(AppGlobals appGlobals)
{
    log_fun();
    QBViewRightWebService self = (QBViewRightWebService) SvTypeAllocateInstance(QBViewRightWebService_getType(), NULL);
    self->appGlobals = appGlobals;

    return self;
}

void QBViewRightWebServiceSetListener(QBViewRightWebService self, SvGenericObject listener)
{
    SVTESTRELEASE(self->listener);
    self->listener = SvWeakReferenceCreateWithObject(listener, NULL);

    SvInvokeInterface(QBViewRightWebListener, listener, stateChanged, self->state, 0);
}

void QBViewRightWebServiceStart(QBViewRightWebService self)
{
    log_fun();
    const char * vcasServerIp = QBConfigGet("VCAS_BOOT_ADDRESS");

    struct stat sb;
    int status = stat(VIEWRIGHT_WEB_STORAGE_PATH, &sb);
    if (status == -1) {
        log_debug("Directory (%s) does not exist, will be created", VIEWRIGHT_WEB_STORAGE_PATH);
        mkdir(VIEWRIGHT_WEB_STORAGE_PATH, 0777);
    }

    SvString vcasServerIpStr = SvStringCreateWithFormat("%s", vcasServerIp);
    SvURL url = SvURLCreateWithString(vcasServerIpStr,  NULL);
    SVRELEASE(vcasServerIpStr);
    QBViewRightWebInit(url, SVSTRING(VIEWRIGHT_WEB_STORAGE_PATH));
    SVRELEASE(url);

    QBViewRightWeb viewRightWeb = QBViewRightWebGetInstance();
    if (viewRightWeb) {
        QBViewRightWebSetListener(viewRightWeb, (SvObject) self);
        self->viewRightWeb = SvWeakReferenceCreateWithObject((SvObject) viewRightWeb, NULL);
    } else {
        self->state = QBViewRightWebState_InitializationError;
        QBViewRightWebServiceNotifyListener(self, -1);
    }
}

void QBViewRightWebServiceStop(QBViewRightWebService self)
{
    log_fun();
    SVTESTRELEASE(self->viewRightWeb);
    QBViewRightWebDeinit();
}
