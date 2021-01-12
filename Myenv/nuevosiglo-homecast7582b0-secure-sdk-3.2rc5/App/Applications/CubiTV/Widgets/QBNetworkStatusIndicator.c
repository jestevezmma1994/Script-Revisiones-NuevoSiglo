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

#include "Widgets/QBNetworkStatusIndicator.h"

#include <SWL/icon.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#define log_debug(fmt, ...)  do { if (0) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)   do { if (1) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError( COLBEG() fmt COLEND_COL(red),  ##__VA_ARGS__); } while (0)

typedef struct QBNetworkStatusIndicator_t {
    struct SvObject_ super_;
    SvWidget statusIcon;

    SvString ifaceName; //observed interface (NULL for default)
    QBNetManager netManager; //different than NULL indicates, that Indicator is already registered to pointed out netManager
} *QBNetworkStatusIndicator;

//QBNETWORKSTATUSINDICATOR_NET_ATTRIBUTE is for debug purpose
//QBNetAttr_linkStatus
//QBNetAttr_enabled
#define QBNETWORKSTATUSINDICATOR_NET_ATTRIBUTE QBNetAttr_linkStatus

enum QBNetworkStatusIndicatorImage {QBNetworkStatusIndicatorImage_available=0,
                                    QBNetworkStatusIndicatorImage_available_wifi,
                                    QBNetworkStatusIndicatorImage_unavailable,
                                    QBNetworkStatusIndicatorImage_unavailable_wifi};


SvLocal void
QBNetworkStatusIndicatorNetAttributeChanged(SvGenericObject self_, SvString iface, uint64_t mask)
{
    log_debug("%s: Network attribute changed", __FUNCTION__);
    QBNetworkStatusIndicator self = (QBNetworkStatusIndicator) self_;

    SvErrorInfo error = NULL;
    SvValue val = NULL, type = NULL;

    //FIXME: It is showing link and gateway status (AND) instead of link status
    if (mask & ((1ll << QBNETWORKSTATUSINDICATOR_NET_ATTRIBUTE) | (1ll << QBNetAttr_IPv4_gateway) | (1ll << QBNetAttr_type))) {
        log_debug("%s: Mask passed", __FUNCTION__);

        log_debug("%s: Changed interface: [%s]", __FUNCTION__, iface ? SvStringCString(iface) : "---");

        SvString wantedIfaceName = NULL;
        if (!self->ifaceName) {
            wantedIfaceName = QBNetManagerGetDefaultIPv4Interface(self->netManager, NULL);
        } else {
            wantedIfaceName = self->ifaceName;
        }

        log_debug("%s: wanted iface name: [%s]", __FUNCTION__, wantedIfaceName ? SvStringCString(wantedIfaceName) : "---");

        if (iface && wantedIfaceName)
            log_debug("%s: Ifaces(wanted/received) [%s]/[%s]", __FUNCTION__, SvStringCString(wantedIfaceName), SvStringCString(iface));

        if (iface && (!SvObjectEquals((SvObject) wantedIfaceName, (SvObject) iface)))
            return;

        type = (SvValue) QBNetManagerGetAttribute(self->netManager, wantedIfaceName, QBNetAttr_type, NULL);
        val  = (SvValue) QBNetManagerGetAttribute(self->netManager, wantedIfaceName, QBNETWORKSTATUSINDICATOR_NET_ATTRIBUTE, &error);

        if (error)
            goto err;

        if (SvValueGetBoolean(val)) {
            log_debug("%s: Network UP", __FUNCTION__);
            if (SvValueGetInteger(type) == QBNetIfaceType_WiFi)
                svIconSwitch(self->statusIcon, QBNetworkStatusIndicatorImage_available_wifi,
                             QBNetworkStatusIndicatorImage_available_wifi, -1.0);
            else
                svIconSwitch(self->statusIcon, QBNetworkStatusIndicatorImage_available,
                                             QBNetworkStatusIndicatorImage_available, -1.0);
        }
        else {
            log_debug("%s: Network DOWN",__FUNCTION__);
            if (SvValueGetInteger(type) == QBNetIfaceType_WiFi)
                svIconSwitch(self->statusIcon, QBNetworkStatusIndicatorImage_unavailable_wifi,
                             QBNetworkStatusIndicatorImage_unavailable_wifi, -1.0);
            else
                svIconSwitch(self->statusIcon, QBNetworkStatusIndicatorImage_unavailable,
                             QBNetworkStatusIndicatorImage_unavailable, -1.0);
        }
    }

    return;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return;
}

SvLocal int
QBNetworkStatusIndicatorUnregisterWithNetManager(QBNetworkStatusIndicator self)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }

    if (self->netManager) {
        QBNetManagerRemoveListener(self->netManager,(SvGenericObject) self, self->ifaceName, &error);
        if (error)
            goto err;

        SVRELEASE(self->netManager);
        self->netManager = NULL;
        SVTESTRELEASE(self->ifaceName);
        self->ifaceName = NULL;
    }

    return 0;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return -1;
}

SvLocal int
QBNetworkStatusIndicatorRegisterWithNetManager(QBNetworkStatusIndicator self, QBNetManager netManager, SvString ifaceName)
{
    SvErrorInfo error = NULL;

    if (!self || !netManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }

    //TODO: ifaceName check against ifaceNameList

    //If already registered to some net manager
    if (self->netManager) {
        QBNetManagerRemoveListener(self->netManager,(SvGenericObject) self, self->ifaceName, &error);
        if (error)
            goto err;
        SVRELEASE(self->netManager);
        self->netManager = NULL;
    }

    QBNetManagerAddListener(netManager, (SvGenericObject) self, ifaceName, &error);
    if (error)
        goto err;

    self->netManager = SVRETAIN(netManager);
    SVTESTRETAIN(ifaceName);
    SVTESTRELEASE(self->ifaceName);
    self->ifaceName = ifaceName;

    return 0;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return -1;
}

SvLocal void
QBNetworkStatusIndicatorClean(SvApplication app, void *ptr)
{
    QBNetworkStatusIndicator self = ptr;
    QBNetworkStatusIndicatorUnregisterWithNetManager(self); //It would also release ifaceName
    SVRELEASE(self);
}

SvLocal void
QBNetworkStatusIndicator__dtor__(void *self_)
{
    QBNetworkStatusIndicator self = self_;
    QBNetworkStatusIndicatorUnregisterWithNetManager(self); //It would also release ifaceName
    //statusIcon is released by parent window
}

SvLocal SvType
QBNetworkStatusIndicator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNetworkStatusIndicator__dtor__
    };
    static SvType type = NULL;

    static const struct QBNetListener_t netMethods = {
        .netAttributeChanged = QBNetworkStatusIndicatorNetAttributeChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkStatusIndicator",
                            sizeof(struct QBNetworkStatusIndicator_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBNetListener_getInterface(), &netMethods,
                            NULL);
    }

    return type;
}


SvWidget
QBNetworkStatusIndicatorCreate(SvApplication app, const char* widgetName, QBNetManager netManager, SvString ifaceName)
{
    if (!app || !netManager || !widgetName) {
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

    QBNetworkStatusIndicator self = (QBNetworkStatusIndicator) SvTypeAllocateInstance(QBNetworkStatusIndicator_getType(), NULL);
    if (unlikely(!self)) {
        log_error("%s (%d): Can't allocate QBNetworkStatusIndicator", __FUNCTION__, __LINE__);
        goto err;
    }

    self->statusIcon = NULL;
    self->ifaceName = NULL;
    self->netManager = NULL;

    w->prv = self;
    w->clean = QBNetworkStatusIndicatorClean;
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

    const char *bgName[] = {"bgAvailable", "bgAvailableWiFi", "bgUnavailable", "bgUnavailableWiFi",};
    const enum QBNetworkStatusIndicatorImage bgFeature[] = { QBNetworkStatusIndicatorImage_available,
                                                             QBNetworkStatusIndicatorImage_available_wifi,
                                                             QBNetworkStatusIndicatorImage_unavailable,
                                                             QBNetworkStatusIndicatorImage_unavailable_wifi, };

    for (size_t i=0; i<sizeof(bgName)/sizeof(*bgName); i++) {
        SvBitmap bitmap = svSettingsGetBitmap(buf, bgName[i]);
        if (unlikely(!bitmap)) {
            log_error("%s (%d): Can't load bitmap", __FUNCTION__, __LINE__);
            goto err;
        }
        svIconSetBitmap(self->statusIcon, bgFeature[i], bitmap);
    }

    svIconSwitch(self->statusIcon, QBNetworkStatusIndicatorImage_unavailable,
                 QBNetworkStatusIndicatorImage_unavailable, -1.0);

    int status = QBNetworkStatusIndicatorRegisterWithNetManager(self, netManager, ifaceName);
    if (status < 0) {
        log_error("%s (%d): Can't register with net manager", __FUNCTION__, __LINE__);
        goto err;
    }

    QBNetworkStatusIndicatorNetAttributeChanged((SvGenericObject)self, ifaceName, 1ll << QBNETWORKSTATUSINDICATOR_NET_ATTRIBUTE); //Indicator init

    return w; //outer widget with private GenObj

err:
    svWidgetDestroy(w);
    return NULL;
}



