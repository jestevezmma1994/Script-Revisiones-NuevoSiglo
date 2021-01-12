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

#include "Widgets/QBNATStatusIndicator.h"

#include <SWL/icon.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <QBAppKit/QBObserver.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#define log_debug(fmt, ...)  do { if (0) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)   do { if (1) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError( COLBEG() fmt COLEND_COL(red),  ##__VA_ARGS__); } while (0)

typedef struct QBNATStatusIndicator_ {
    /// super class
    struct SvObject_ super_;

    SvWidget statusIcon;

    QBNATMonitor natMonitor; //different than NULL indicates, that Indicator is already registered to pointed out natMonitor
} *QBNATStatusIndicator;


enum QBNATStatusIndicatorImage {QBNATStatusIndicatorImage_no_internet=0,
                                QBNATStatusIndicatorImage_internet,
                                QBNATStatusIndicatorImage_internet_UDP};

//NATMonitor
SvLocal void
QBNATStatusIndicatorNATStatusChanged(SvObject self_,
                                     SvObject observable,
                                     SvObject arg)
{
    QBNATStatusIndicator self = (QBNATStatusIndicator)self_;
    log_debug("%s: NAT attribute changed",__FUNCTION__);

    QBNATMonitorStatus  status;
    bool udpReceivingPossible = false;

    QBNATMonitorGetStatus(self->natMonitor, &status);


    if (status.ipIsPublic)
        udpReceivingPossible = true;
    else if (status.mappedIP)
        udpReceivingPossible = status.portPreserved || (!status.portRestricted && !status.ipRestricted);


    if(udpReceivingPossible) {
        log_debug("%s: internet_UDP", __FUNCTION__);
        svIconSwitch(self->statusIcon, QBNATStatusIndicatorImage_internet_UDP,
                     QBNATStatusIndicatorImage_internet_UDP, -1.0);
    }
    else if(status.mappedIP) {
        log_debug("%s: internet", __FUNCTION__);
        svIconSwitch(self->statusIcon, QBNATStatusIndicatorImage_internet,
                     QBNATStatusIndicatorImage_internet, -1.0);
    }
    else {
        log_debug("%s: no_internet", __FUNCTION__);
        svIconSwitch(self->statusIcon, QBNATStatusIndicatorImage_no_internet,
                     QBNATStatusIndicatorImage_no_internet, -1.0);
    }
}

SvLocal int
QBNATStatusIndicatorUnregisterWithNATMonitor(QBNATStatusIndicator self)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }

    if (self->natMonitor) {
        QBNATMonitorRemoveObserver(self->natMonitor, (SvObject) self, &error);
        if (error)
            goto err;

        SVRELEASE(self->natMonitor);
        self->natMonitor = NULL;
    }

    return 0;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return -1;
}

SvLocal int
QBNATStatusIndicatorRegisterWithNATMonitor(QBNATStatusIndicator self, QBNATMonitor natMonitor)
{
    SvErrorInfo error = NULL;

    if (!self || !natMonitor) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }


    //If already registered to some traxis manager
    if (self->natMonitor) {
        QBNATMonitorRemoveObserver(self->natMonitor, (SvObject) self, &error);
        if (error)
            goto err;
        SVRELEASE(self->natMonitor);
        self->natMonitor = NULL;
    }

    QBNATMonitorAddObserver(natMonitor, (SvObject) self, &error);
    if (error)
        goto err;

    self->natMonitor = SVRETAIN(natMonitor);

    return 0;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return -1;
}

SvLocal void
QBNATStatusIndicatorClean(SvApplication app, void *ptr)
{
    QBNATStatusIndicator self = ptr;
    QBNATStatusIndicatorUnregisterWithNATMonitor(self);
    SVRELEASE(self);
}

SvLocal void
QBNATStatusIndicatorDestroy(void *self_)
{
    QBNATStatusIndicator self = self_;
    QBNATStatusIndicatorUnregisterWithNATMonitor(self);
    //statusIcon is released by parent window
}

SvLocal SvType
QBNATStatusIndicator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNATStatusIndicatorDestroy
    };
    static SvType type = NULL;

    static const struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBNATStatusIndicatorNATStatusChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNATStatusIndicator",
                            sizeof(struct QBNATStatusIndicator_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBObserver_getInterface(), &observerMethods,
                            NULL);
    }

    return type;
}

SvWidget
QBNATStatusIndicatorCreate(SvApplication app, const char *widgetName, QBNATMonitor natMonitor)
{
    if (!app || !widgetName || !natMonitor) {
        log_error("%s (%d): Bad arguments", __FUNCTION__, __LINE__);
        return NULL;
    }
    if (!svSettingsIsWidgetDefined(widgetName))
        return NULL;

    SvWidget w = svSettingsWidgetCreate(app, widgetName); //Outer widget
    if (!w) {
        log_info("%s: Can't create widget", __FUNCTION__); //Probably there were no settings (on purpose)
        return NULL;
    }

    QBNATStatusIndicator self = (QBNATStatusIndicator) SvTypeAllocateInstance(QBNATStatusIndicator_getType(), NULL);
    if (unlikely(!self)) {
        log_error("%s (%d): Can't allocate QBNATStatusIndicator", __FUNCTION__, __LINE__);
        goto err;
    }
    self->statusIcon = NULL;
    self->natMonitor = NULL;

    w->prv = self; //widget stores pointer to created GenObj
    w->clean = QBNATStatusIndicatorClean;
    svWidgetSetFocusable(w, false);

    //Widgets construction
    char buf[128];
    snprintf(buf, 128, "%s.Icon", widgetName);

    self->statusIcon = svIconNew(app, buf); //Inner widget (icon)
    if (!self->statusIcon) {
        log_error("%s (%d): Can't create icon", __FUNCTION__, __LINE__);
        goto err;
    }

    svSettingsWidgetAttach(w, self->statusIcon, buf, 1);

    const char *bgName[] = {"bgNoInternet", "bgInternet", "bgInternetUDP",};
    const enum QBNATStatusIndicatorImage bgFeature[] = {QBNATStatusIndicatorImage_no_internet,
                                                        QBNATStatusIndicatorImage_internet,
                                                        QBNATStatusIndicatorImage_internet_UDP,};

    for (size_t i=0; i<sizeof(bgName)/sizeof(*bgName); i++) {
        SvBitmap bitmap = svSettingsGetBitmap(buf, bgName[i]);
        if (unlikely(!bitmap)) {
            log_error("%s (%d): Can't load bitmap", __FUNCTION__, __LINE__);
            goto err;
        }
        svIconSetBitmap(self->statusIcon, bgFeature[i], bitmap);
    }

    svIconSwitch(self->statusIcon, QBNATStatusIndicatorImage_no_internet,
                 QBNATStatusIndicatorImage_no_internet, -1.0);

    int status = QBNATStatusIndicatorRegisterWithNATMonitor(self, natMonitor);
    if (status < 0) {
        log_error("%s (%d): Can't register with NAT monitor", __FUNCTION__, __LINE__);
        goto err;
    }

    QBNATStatusIndicatorNATStatusChanged((SvObject) self, NULL, NULL);

    return w; //outer widget with private GenObj

err:
    svWidgetDestroy(w);
    return NULL;
}



