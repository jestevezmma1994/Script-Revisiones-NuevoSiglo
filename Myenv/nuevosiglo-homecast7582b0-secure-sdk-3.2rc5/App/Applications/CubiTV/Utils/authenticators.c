/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <QBSmartcard2Interface.h>
#include <QBCAS.h>
#include <QBConf.h>
#include <Widgets/authDialog.h>
#include "authenticators.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvInterface.h>
#include <string.h>

struct QBAccessManagerAuthenticator_t {
    struct SvObject_ super_;
    QBAccessManager accessMgr;
    SvString domain;
    SvWidget master;
    SvFiber fiber;
    SvFiberTimer timer;
    SvObject context;
};
typedef struct QBAccessManagerAuthenticator_t* QBAccessManagerAuthenticator;

struct QBMaturityRatingChangeAuthenticator_t {
    struct SvObject_ super_;
    QBAccessManager accessMgr;
    SvString domain;
    SvWidget master;
    int wantedMaturity;
    QBCASCmd cmd;
    bool cancelPinCheck;
};
typedef struct QBMaturityRatingChangeAuthenticator_t* QBMaturityRatingChangeAuthenticator;


SvLocal void QBAccessManagerAuthenticator__dtor__(void *self_)
{
    QBAccessManagerAuthenticator self = self_;
    SVTESTRELEASE(self->domain);
    SVTESTRELEASE(self->accessMgr);
    SVTESTRELEASE(self->context);
    SvFiberDestroy(self->fiber);
}

SvLocal void QBAccessManagerAuthenticatorStep(void *self_)
{
    QBAccessManagerAuthenticator self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    QBAuthStatus status = QBAccessManagerCheckAuthentication(self->accessMgr, self->domain, NULL);
    if(status == QBAuthStatus_OK) {
        QBAuthDialogPinAccepted(self->master);
        return;
    } else if(status == QBAuthStatus_invalid) {
        QBAuthDialogPinDeclined(self->master);
        return;
    } else if (status == QBAuthStatus_rejected) {
        QBAuthDialogPinRejected(self->master);
        return;
    } else if (status == QBAuthStatus_failed) {
        QBAuthDialogPinFailed(self->master);
        return;
    } else if (status == QBAuthStatus_blocked) {
        QBAuthDialogPinBlocked(self->master);
        return;
    } else if (status == QBAuthStatus_notAuthorised) {
        QBAuthDialogNotAuthorized(self->master);
        return;
    }

    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(0, 400));
}

SvLocal void QBAccessManagerAuthenticatorCheckPin(SvObject self_, SvWidget master, const char *pin)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) self_;
    QBAuthStatus status;
    SvString passwd;
    self->master = master;

    passwd = SvStringCreate(pin, NULL);
    status = QBAccessManagerAuthenticate(self->accessMgr, self->domain, passwd, self->context, NULL);
    SVRELEASE(passwd);

    if (status == QBAuthStatus_OK) {
        QBAuthDialogPinAccepted(self->master);
    } else if (status == QBAuthStatus_inProgress) {
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(0, 400));
    } else if (status == QBAuthStatus_invalid) {
        QBAuthDialogPinDeclined(self->master);
    } else if (status == QBAuthStatus_failed) {
        QBAuthDialogPinFailed(self->master);
    } else if (status == QBAuthStatus_notAuthorised) {
        QBAuthDialogNotAuthorized(self->master);
    } else {
        SvLogError("%s():%d: status not handled = %d", __func__, __LINE__, status);
    }
}

SvLocal void QBAccessManagerAuthenticatorCancelPinCheck(SvObject self_)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) self_;

    if (self->fiber) {
        SvFiberDeactivate(self->fiber);
    }

    if (self->timer) {
        SvFiberEventDeactivate(self->timer);
    }
}

SvLocal QBAccessCode
QBAccessManagerAuthenticatorCheckAccess(SvObject self_, SvWidget helper)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) self_;
    return QBAccessManagerCheckAccess(self->accessMgr, self->domain);
}

SvLocal void QBAccessManagerAuthenticatorCheckAuthParams(SvObject self_, QBAuthParams *params)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) self_;
    QBAccessManagerGetAuthParamsByDomain(self->accessMgr, params, self->domain, NULL);
}

SvLocal SvString QBAccessManagerAuthenticatorGetDomain(SvObject self_)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) self_;
    return self->domain;
}

SvLocal
SvType QBAccessManagerAuthenticator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAccessManagerAuthenticator__dtor__
    };
    static SvType type = NULL;
    static const struct QBAuthDialogAuthenticator_t methods = {
        .checkPin = QBAccessManagerAuthenticatorCheckPin,
        .cancelPinCheck = QBAccessManagerAuthenticatorCancelPinCheck,
        .checkAccess = QBAccessManagerAuthenticatorCheckAccess,
        .checkAuthParams = QBAccessManagerAuthenticatorCheckAuthParams,
        .getDomain = QBAccessManagerAuthenticatorGetDomain,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAccessManagerAuthenticator",
                            sizeof(struct QBAccessManagerAuthenticator_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAuthDialogAuthenticator_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvObject QBAuthenticateViaAccessManagerWithContext(SvScheduler scheduler, QBAccessManager accessMgr, SvString domain, SvObject context)
{
    QBAccessManagerAuthenticator self = (QBAccessManagerAuthenticator) SvTypeAllocateInstance(QBAccessManagerAuthenticator_getType(), NULL);
    self->domain = SVRETAIN(domain);
    self->accessMgr = SVRETAIN(accessMgr);

    self->fiber = SvFiberCreate(scheduler, NULL, "AccessManagerAuthenticate", QBAccessManagerAuthenticatorStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    self->context = SVTESTRETAIN(context);

    return (SvObject) self;
}

SvObject QBAuthenticateViaAccessManager(SvScheduler scheduler, QBAccessManager accessMgr, SvString domain)
{
    return QBAuthenticateViaAccessManagerWithContext(scheduler, accessMgr, domain, NULL);
}

SvLocal void QBMaturityRatingChangeAuthenticatorCallback(void *self_, const QBICSmartcardCmd* cmd, int status)
{
    QBMaturityRatingChangeAuthenticator self = self_;
    if (!self->cancelPinCheck) {
        if(status == QBICSmartcardPinStatus_ok) {
            QBAccessManagerOnAuthenticationStatusChange(self->accessMgr, self->domain, QBAuthStatus_OK);
            QBAuthDialogPinAccepted(self->master);
        } else if (status == QBICSmartcardPinStatus_invalid){
            QBAccessManagerOnAuthenticationStatusChange(self->accessMgr, self->domain, QBAuthStatus_invalid);
            QBAuthDialogPinDeclined(self->master);
        } else if (status == QBICSmartcardPinStatus_error) {
            QBAccessManagerOnAuthenticationStatusChange(self->accessMgr, self->domain, QBAuthStatus_failed);
            QBAuthDialogPinFailed(self->master);
        }
    }
    self->cmd = NULL;
}

SvLocal void QBMaturityRatingChangeAuthenticatorCheckPin(SvObject self_, SvWidget master, const char *pin)
{
    QBMaturityRatingChangeAuthenticator self = (QBMaturityRatingChangeAuthenticator) self_;
    self->master = master;
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), changeMaturityRating, QBMaturityRatingChangeAuthenticatorCallback, self, &self->cmd, (const unsigned char*) pin, self->wantedMaturity);
}

SvLocal void QBMaturityRatingChangeAuthenticatorCancelPinCheck(SvObject self_)
{
    QBMaturityRatingChangeAuthenticator self = (QBMaturityRatingChangeAuthenticator) self_;
    self->cancelPinCheck = true;
}

SvLocal QBAccessCode QBMaturityRatingChangeAuthenticatorCheckAccess(SvObject self_, SvWidget helper)
{
    return QBAccess_confirmationRequired;
}

SvLocal void QBMaturityRatingChangeAuthenticatorCheckAuthParams(SvObject self, QBAuthParams *params)
{
    memset(params, 0, sizeof(*params));
}

SvLocal SvString QBMaturityRatingChangeAuthenticatorGetDomain(SvObject self_)
{
    QBMaturityRatingChangeAuthenticator self = (QBMaturityRatingChangeAuthenticator) self_;
    return self->domain;
}

SvLocal void QBMaturityRatingChangeAuthenticator__dtor__(void *self_)
{
    QBMaturityRatingChangeAuthenticator self = self_;
    SVTESTRELEASE(self->domain);
    if (self->cmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
}

SvLocal
SvType QBMaturityRatingChangeAuthenticator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMaturityRatingChangeAuthenticator__dtor__
    };
    static SvType type = NULL;
    static const struct QBAuthDialogAuthenticator_t methods = {
        .checkPin = QBMaturityRatingChangeAuthenticatorCheckPin,
        .cancelPinCheck = QBMaturityRatingChangeAuthenticatorCancelPinCheck,
        .checkAccess = QBMaturityRatingChangeAuthenticatorCheckAccess,
        .checkAuthParams = QBMaturityRatingChangeAuthenticatorCheckAuthParams,
        .getDomain = QBMaturityRatingChangeAuthenticatorGetDomain,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMaturityRatingChangeAuthenticator",
                            sizeof(struct QBMaturityRatingChangeAuthenticator_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAuthDialogAuthenticator_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvObject QBAuthenticateForConaxMaturityRating(QBAccessManager accessMgr, int wantedMaturity)
{
    // TODO: this class should be moved to Conax specific file
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        SvLogError("%s :: QBAuthenticateForConaxMaturityRating should be only created for Conax CAS type", __func__);
        return NULL;
    }

    QBMaturityRatingChangeAuthenticator self = (QBMaturityRatingChangeAuthenticator) SvTypeAllocateInstance(QBMaturityRatingChangeAuthenticator_getType(), NULL);
    self->wantedMaturity = wantedMaturity;
    self->accessMgr = accessMgr;
    self->domain = SVSTRING("ConaxMaturity");
    self->cancelPinCheck = false;

    return (SvObject) self;
}
