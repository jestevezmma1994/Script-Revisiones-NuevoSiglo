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

#include "QBSecureLogLogic.h"
#include <Services/standbyAgent/QBStandbyAgentService.h>
#include <Services/QBStandbyAgent.h>
#include <Services/QBAppVersion.h>
#include <Services/core/QBNetworkWatcher.h>
#include <Logic/AccessLogic.h>
#include <QBConfig.h>
#include <QBNetManager.h>
#include <QBNetworkMonitor/QBNetworkMonitorListener.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <QBNetworkMonitor.h>
#include <QBSmartCardMonitor.h>
#include <QBApplicationController.h>
#include <QBSecureLogManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvType.h>
#include <Utils/appType.h>
#include <sys/time.h>
#include <time.h>
#include <libintl.h>

// This macro returns name of variable if its value is NULL
#define getNameIfNull(variableName) (variableName ? "" : #variableName " ")

struct QBSecureLogLogic_ {
    struct SvObject_ super_;
    bool started;

    SvString macAddress;
    SvString networkInterface;
    SvString ipAddress;
    SvString swVersion;
    SvString timeZone;
    SvString rcuID;
    SvString profileID;
    SvString chipID;
    SvString smartCardID;
    SvString standbyState;
    SvString mwDomain;
    SvString lastContext;
    SvString actionSource;

    QBNetManager net_manager;
    QBNetworkMonitor networkMonitor;
    QBNetworkWatcher networkWatcher;
    QBSmartCardMonitor smartCardMonitor;
    QBMiddlewareManager middlewareManager;
    QBAppVersion appVersion;
    QBStandbyAgent standbyAgent;
    QBApplicationController controller;
};

SvLocal void QBSecureLogLogicDestroy(void *self_)
{
    QBSecureLogLogic self = (QBSecureLogLogic) self_;

    if (self->started) {
        QBSecureLogLogicStop(self);
    }

    SVTESTRELEASE(self->macAddress);
    SVTESTRELEASE(self->networkInterface);
    SVTESTRELEASE(self->ipAddress);
    SVTESTRELEASE(self->swVersion);
    SVTESTRELEASE(self->timeZone);
    SVTESTRELEASE(self->rcuID);
    SVTESTRELEASE(self->profileID);
    SVTESTRELEASE(self->chipID);
    SVTESTRELEASE(self->smartCardID);
    SVTESTRELEASE(self->standbyState);
    SVTESTRELEASE(self->mwDomain);
    SVTESTRELEASE(self->lastContext);
    SVTESTRELEASE(self->actionSource);
}

SvLocal void QBSecureLogLogicSetMAC(QBSecureLogLogic self, QBNetworkInterface interface)
{
    if (!interface) {
        SVTESTRELEASE(self->macAddress);
        self->macAddress = NULL;
        return;
    }

    SvObject macAddress = NULL;
    if (!(macAddress = QBNetManagerGetAttribute(self->net_manager, QBNetworkInterfaceGetID(interface), QBNetAttr_hwAddr, NULL))) {
        return;
    }
    if (SvObjectIsInstanceOf(macAddress, SvString_getType())) {
        SVTESTRELEASE(self->macAddress);
        self->macAddress = SVRETAIN(macAddress);
    }
}

SvLocal void QBSecureLogLogicSetIP(QBSecureLogLogic self, QBNetworkInterface interface)
{
    SVTESTRELEASE(self->ipAddress);
    self->ipAddress = SVTESTRETAIN(interface ? QBNetworkInterfaceGetIP(interface) : NULL);
}

SvLocal void QBSecureLogLogicSetChipID(QBSecureLogLogic self)
{
    if (!self->smartCardMonitor) {
        return;
    }

    SvString chipID = QBSmartCardMonitorGetChipId(self->smartCardMonitor);
    SVTESTRELEASE(self->chipID);
    self->chipID = SVTESTRETAIN(chipID);
}

SvLocal void QBSecureLogLogicSetSmartCardID(QBSecureLogLogic self)
{
    if (!self->smartCardMonitor) {
        return;
    }

    SvString smartCardID = QBSmartCardMonitorGetSmartCardId(self->smartCardMonitor);
    SVTESTRELEASE(self->smartCardID);
    self->smartCardID = SVTESTRETAIN(smartCardID);
}

SvLocal void QBSecureLogLogicSetMWAddress(QBSecureLogLogic self)
{
    SvString prefixUrl = NULL;

    prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!prefixUrl) {
        return;
    }

    SvURI uri = SvURICreateWithString(prefixUrl, NULL);
    if (uri) {
        SVTESTRELEASE(self->mwDomain);
        self->mwDomain = SVRETAIN(SvURIAuthority(uri));
        SVRELEASE(uri);
    }
}

SvLocal void QBSecureLogLogicSetTimeZone(QBSecureLogLogic self)
{
    time_t timeInfo = time(NULL);
    struct tm timeStruct = { .tm_year = 0 };
    localtime_r(&timeInfo, &timeStruct);
    SVTESTRELEASE(self->timeZone);
    self->timeZone = SvStringCreate(timeStruct.tm_zone, NULL);
}

SvLocal void QBSecureLogLogicSetSWVersion(QBSecureLogLogic self)
{
    SVTESTRELEASE(self->swVersion);
    self->swVersion = SVRETAIN(QBAppVersionGetFullVersion(self->appVersion));
}

SvLocal void QBSecureLogLogicSetRCU(QBSecureLogLogic self)
{
    /* TODO: mapping of rcpID to proper value */
    SVTESTRELEASE(self->rcuID);
    self->rcuID = SvStringCreate("default", NULL);
}

SvLocal void QBSecureLogLogicSetProfileID(QBSecureLogLogic self)
{
    /* TODO: mapping of profileID to proper value */
    SVTESTRELEASE(self->profileID);
    self->profileID = SvStringCreate("default", NULL);
}

SvLocal void QBSecureLogLogicSetActionSource(QBSecureLogLogic self, SvString eventType)
{
    /* TODO: mapping of actionSource to proper value */
    SVTESTRELEASE(self->actionSource);
    self->actionSource =  SvStringCreate("unknown", NULL);
}

SvLocal void QBSecureLogLogicSetStandbyState(QBSecureLogLogic self)
{
    if (!self->standbyAgent) {
        return;
    }

    SvString standbyState = QBStandbyAgentIsStandby(self->standbyAgent) ? SvStringCreate("active", NULL) : SvStringCreate("on", NULL);
    SVTESTRELEASE(self->standbyState);
    self->standbyState = standbyState;
}

SvLocal void QBSecureLogLogicUpdateInterface(QBSecureLogLogic self)
{
    SVTESTRELEASE(self->networkInterface);
    QBNetworkInterface interface = QBNetworkWatcherGetGatewayOwner(self->networkWatcher, NULL);
    self->networkInterface = SVTESTRETAIN(interface ? QBNetworkInterfaceGetID(interface) : NULL);
    QBSecureLogLogicSetIP(self, interface);
    QBSecureLogLogicSetMAC(self, interface);
}

SvLocal void QBSecureLogLogicSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvLocal void QBSecureLogLogicSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBSecureLogLogic self = (QBSecureLogLogic) self_;
    if (!to) {
        return;
    }

    SVTESTRELEASE(self->lastContext);
    /* TODO: mapping type name to desired value */
    self->lastContext = SvStringCreate(SvTypeGetName(SvObjectGetType((SvObject) to)), NULL);
}

/*
 * standby service virtual methods
 */
SvLocal void QBSecureLogLogicStandbyServiceStart(SvObject self_)
{
    if (!self_) {
        SvLogError("self_ parameter not defined.");
        return;
    }
    QBSecureLogLogicSetStandbyState((QBSecureLogLogic) self_);
}

SvLocal void QBSecureLogLogicStandbyServiceStop(SvObject self_)
{
    if (!self_) {
        SvLogError("self_ parameter not defined.");
        return;
    }
    QBSecureLogLogicSetStandbyState((QBSecureLogLogic) self_);
}

SvLocal QBStandbyStateData QBSecureLogLogicStandbyServiceCreateStateData(SvObject self_)
{
    return QBStandbyStateDataCreate(QBStandbyState_unknown, self_, false, SVSTRING("QBSecureLogLogic"));
}

SvLocal void QBSecureLogLogicStandbyServiceResetState(SvObject self_, bool popupClose)
{
}

SvLocal void QBSecureLogLogicSmartcardStatusChanged(SvObject self_)
{
    QBSecureLogLogic self = (QBSecureLogLogic) self_;
    QBSecureLogLogicSetChipID(self);
    QBSecureLogLogicSetSmartCardID(self);
}

SvLocal void QBSecureLogLogicMWDataChanged(SvObject self_, QBMiddlewareManagerType middlewareType)
{
    QBSecureLogLogicSetMWAddress((QBSecureLogLogic) self_);
}

SvLocal void QBSecureLogLogicInterfaceAdded(SvObject self_, QBNetworkInterface interface)
{
    QBSecureLogLogicUpdateInterface((QBSecureLogLogic) self_);
}

SvLocal void QBSecureLogLogicInterfaceRemoved(SvObject self_, SvString interfaceID)
{
    QBSecureLogLogicUpdateInterface((QBSecureLogLogic) self_);
}

SvLocal void QBSecureLogLogicInterfaceStateChanged(SvObject self_, SvString interfaceID)
{
    QBSecureLogLogicUpdateInterface((QBSecureLogLogic) self_);
}

SvLocal void QBSecureLogLogicIPSecVPNStateChanged(SvObject self_)
{
}

SvLocal SvType QBSecureLogLogic_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSecureLogLogicDestroy
    };

    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBSecureLogLogicSwitchStarted,
        .ended = QBSecureLogLogicSwitchEnded
    };

    static const struct QBStandbyAgentService_ standbyServiceMethods = {
        .createStateData = QBSecureLogLogicStandbyServiceCreateStateData,
        .resetState      = QBSecureLogLogicStandbyServiceResetState,
        .start           = QBSecureLogLogicStandbyServiceStart,
        .stop            = QBSecureLogLogicStandbyServiceStop,
    };

    static const struct QBNetworkMonitorListener_ networkMonitorMethods = {
        .interfaceAdded         = QBSecureLogLogicInterfaceAdded,
        .interfaceRemoved       = QBSecureLogLogicInterfaceRemoved,
        .interfaceStateChanged  = QBSecureLogLogicInterfaceStateChanged,
        .ipSecVPNStateChanged   = QBSecureLogLogicIPSecVPNStateChanged,
    };

    static const struct QBSmartCardMonitorListener_t smartCardMethods = {
        .statusChanged = QBSecureLogLogicSmartcardStatusChanged
    };

    static const struct QBMiddlewareManagerListener_t mwManagerMethods = {
        .middlewareDataChanged = QBSecureLogLogicMWDataChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBSecureLogLogic", sizeof(struct QBSecureLogLogic_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBStandbyAgentService_getInterface(), &standbyServiceMethods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            QBSmartCardMonitorListener_getInterface(), &smartCardMethods,
                            QBNetworkMonitorListener_getInterface(), &networkMonitorMethods,
                            QBMiddlewareManagerListener_getInterface(), &mwManagerMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBSecureLogLogicInit(QBSecureLogLogic self)
{
    self->started = false;

    self->macAddress = NULL;
    self->networkInterface = NULL;
    self->ipAddress = NULL;
    self->swVersion = NULL;
    self->timeZone = NULL;
    self->rcuID = NULL;
    self->profileID = NULL;
    self->chipID = NULL;
    self->smartCardID = NULL;
    self->standbyState = SvStringCreate("unknown", NULL);
    self->mwDomain = NULL;
    self->lastContext = SvStringCreate("unknown", NULL);
    self->actionSource = SvStringCreate("unknown", NULL);

    self->net_manager = NULL;
    self->smartCardMonitor = NULL;
    self->middlewareManager = NULL;
    self->appVersion = NULL;
    self->standbyAgent = NULL;
    self->controller = NULL;
}

QBSecureLogLogic QBSecureLogLogicCreate(void)
{
    QBSecureLogLogic self = (QBSecureLogLogic) SvTypeAllocateInstance(QBSecureLogLogic_getType(), NULL);
    QBSecureLogLogicInit(self);
    return self;
}

SvLocal void QBSecureLogLogicSetAll(QBSecureLogLogic self)
{
    QBSecureLogLogicSetChipID(self);
    QBSecureLogLogicSetSmartCardID(self);
    QBSecureLogLogicSetMWAddress(self);
    QBSecureLogLogicSetTimeZone(self);
    QBSecureLogLogicSetSWVersion(self);
    QBSecureLogLogicSetRCU(self);
    QBSecureLogLogicSetProfileID(self);
    QBSecureLogLogicSetStandbyState(self);

    QBNetworkMonitorRecallInterfaces(self->networkMonitor, (SvObject) self);
}

void QBSecureLogLogicStart(QBSecureLogLogic self, QBSecureLogLogicParams params)
{

    if (!self || !params) {
        SvLogError("Can't start: %s%snotdefined", getNameIfNull(self), getNameIfNull(params));
        return;
    }

    if (!params->net_manager || !params->middlewareManager ||
        !params->appVersion || !params->controller || !params->networkMonitor) {
        SvLogError("Can't start: %s%s%s%s%snot defined",
                    getNameIfNull(params->net_manager), getNameIfNull(params->middlewareManager),
                    getNameIfNull(params->appVersion), getNameIfNull(params->controller),
                    getNameIfNull(params->networkMonitor));
        return;
    }

    if (self->started) {
        SvLogError("Logic already started");
        return;
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(registry, SVSTRING("QBNetworkWatcher"));
    if (!networkWatcher) {
        SvLogError("%s(): Cannot get networkWatcher service", __func__);
        return;
    }
    self->networkWatcher = SVRETAIN(networkWatcher);

    self->net_manager = SVRETAIN(params->net_manager);
    self->middlewareManager = SVRETAIN(params->middlewareManager);
    self->appVersion = SVRETAIN(params->appVersion);
    self->controller = params->controller; // not an object
    self->smartCardMonitor = SVTESTRETAIN(params->smartCardMonitor);
    self->standbyAgent = SVTESTRETAIN(params->standbyAgent);
    self->networkMonitor = SVRETAIN(params->networkMonitor);

    if (self->standbyAgent) {
        QBStandbyAgentAddService(self->standbyAgent, (SvObject) self);
    }

    if (self->smartCardMonitor) {
        QBSmartCardMonitorAddListener(self->smartCardMonitor, (SvObject) self);
    }

    QBNetworkMonitorAddListener(self->networkMonitor, (SvObject) self);
    QBApplicationControllerAddListener(self->controller, (SvObject) self);
    QBMiddlewareManagerAddListener(self->middlewareManager, (SvObject) self);

    QBSecureLogLogicSetAll(self);

    QBSecureLogManagerSetCustomHeaderFunction(&QBSecureLogLogicCreateCustomHeader);
    QBSecureLogManagerSetCustomHelloContentFunction(&QBSecureLogLogicCreatHelloMsgContent);

    self->started = true;
}

void QBSecureLogLogicStop(QBSecureLogLogic self)
{
    if (!self) {
        SvLogError("Can't stop logic: self argument not defined");
        return;
    }

    if (!self->started) {
        SvLogError("Logic already stopped");
        return;
    }

    if (self->smartCardMonitor) {
        QBSmartCardMonitorRemoveListener(self->smartCardMonitor, (SvObject) self);
    }

    QBNetworkMonitorRemoveListener(self->networkMonitor, (SvObject) self);
    QBMiddlewareManagerRemoveListener(self->middlewareManager, (SvObject) self);
    QBApplicationControllerRemoveListener(self->controller, (SvObject) self);

    SVRELEASE(self->net_manager);
    SVTESTRELEASE(self->smartCardMonitor);
    SVRELEASE(self->middlewareManager);
    SVRELEASE(self->appVersion);
    SVTESTRELEASE(self->standbyAgent);
    SVRELEASE(self->networkWatcher);
    SVRELEASE(self->networkMonitor);
    self->controller = NULL;

    self->started = false;
}

SvString QBSecureLogLogicCreatHelloMsgContent(SvObject self_)
{
    QBSecureLogLogic self = (QBSecureLogLogic)self_;
    if (!self_) {
        SvLogError("SecureLogLogic object is set to null");
        return NULL;
    }

    if (!self->started) {
        SvLogError("SecureLogLogic is not started");
        return NULL;
    }

    return NULL;
}

SvLocal const char* QBSecureLogLogicGetSafeCString(SvString string)
{
    return string ? SvStringGetCString(string) : "n/a";
}

/*
 * This function creates header that contains as follows:
 *      MAC             - MAC address of STB,
 *      INTERFACE       - local network interface in that is in use, e.g. eth0
 *      IP              - IP address of INTERFACE
 *      SW_VERSION      - full software version
 *      TIMEZONE        - current timezone
 *      DEVICE_ID       - SCID or CHIP_ID of STB
 *      STANDBY_STATE   - standby state (on, active)
 *      MW_DOMAIN       - address of current MW
 *      RCU_ID          - type of RCU which was used as last (default, kids, ...)
 *      PROFILE_ID      - profile id (default, kids, ...)
 *      LAST_CONTEXT    - name of last context
 *      ACTION_SOURCE   - type of event source (user, stb, unknown)
 *
 * Example of created header:
 *        d0:50:99:33:30:a7 eth0 192.168.208.60 columbus-homecast7429b0-cubitv-10.4rc2 UTC n/a on mwdev.cubiware.com default default SvTVContext unknown
 *
 */
SvString QBSecureLogLogicCreateCustomHeader(SvObject self_, SvString eventType)
{
    QBSecureLogLogic self = (QBSecureLogLogic)self_;
    if (!self_) {
        SvLogError("SecureLogLogic object is set to null");
        return SvStringCreate("", NULL);
    }

    if (!self->started) {
        SvLogError("SecureLogLogic is not started");
        return SvStringCreate("", NULL);
    }

    QBSecureLogLogicSetActionSource(self, eventType);

    return SvStringCreateWithFormat("%s %s %s %s %s %s %s %s %s %s %s %s ",
            QBSecureLogLogicGetSafeCString(self->macAddress),
            QBSecureLogLogicGetSafeCString(self->networkInterface),
            QBSecureLogLogicGetSafeCString(self->ipAddress),
            QBSecureLogLogicGetSafeCString(self->swVersion),
            QBSecureLogLogicGetSafeCString(self->timeZone),
            self->smartCardID ? SvStringGetCString(self->smartCardID) : QBSecureLogLogicGetSafeCString(self->chipID),
            QBSecureLogLogicGetSafeCString(self->standbyState),
            QBSecureLogLogicGetSafeCString(self->mwDomain),
            QBSecureLogLogicGetSafeCString(self->rcuID),
            QBSecureLogLogicGetSafeCString(self->profileID),
            QBSecureLogLogicGetSafeCString(self->lastContext),
            QBSecureLogLogicGetSafeCString(self->actionSource));
}
