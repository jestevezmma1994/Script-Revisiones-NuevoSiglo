/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "cecService.h"
#include <Services/core/QBCEC.h>
#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <main.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvHashTable.h>
#include <QBInput/QBInputCodes.h>
#include <QBConf.h>
#include <QBViewport.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBPlatformHAL/QBPlatformCEC.h>
#include <QBPlatformHAL/QBPlatformImpl.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformEvent.h>
#include <CUIT/Core/event.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <SvByteOrder.h>

struct QBCecService_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    bool enabled;
    bool isActiveSource;
};

#define QBCecServiceHASCEC "CEC"

SvLocal void QBCecServiceStatusChanged(SvGenericObject self_, const char *key, const char *value);

SvLocal bool QBCecServiceParseConfigGet(const char *value)
{
    return (value && !strcmp(value, "enabled"));
}

SvLocal void
QBCecServiceObservedVolumeChanged(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBCecService self = (QBCecService) self_;
    if (!self->enabled || !SvObjectIsInstanceOf(observedObject, QBVolume_getType())) {
        return;
    }

    QBVolume volume = (QBVolume) observedObject;
    if (!QBVolumeIsMuted(volume, NULL)) {
        unsigned int volumeToSet = QBVIEWPORT_VOL_MAX - 1;
        if (QBVolumeGetCurrentLevel(volume) != volumeToSet) {
            QBVolumeSet(volume, volumeToSet);
        }
    }
}

SvLocal SvType QBCecService_getType(void)
{
    static SvType type = NULL;

    static const struct QBConfigListener_t config_methods = {
        .changed = QBCecServiceStatusChanged,
    };

    static struct QBObserver_ observerMethods = {
        .observedObjectUpdated = QBCecServiceObservedVolumeChanged
    };

    if (unlikely(!type)) {
        type = SvTypeCreateManaged("QBCecService", sizeof(struct QBCecService_),
                                   SvObject_getType(), &type,
                                   QBConfigListener_getInterface(), &config_methods,
                                   QBObserver_getInterface(), &observerMethods,
                                   NULL);
    }

    return type;
}

QBCecService QBCecServiceCreate(AppGlobals appGlobals)
{
    assert(appGlobals);

    QBCecService self = (QBCecService) SvTypeAllocateInstance(QBCecService_getType(), NULL);
    self->appGlobals = appGlobals;

    self->enabled = QBCecServiceParseConfigGet(QBConfigGet(QBCecServiceHASCEC));

    self->isActiveSource = true;

    return self;
}

SvLocal int QBCecServiceGetEnabledHDMICECOutput(void)
{
    int HDMIOutputID = QBPlatformFindOutput(QBOutputType_HDMI, NULL, 0, true, false);
    if (HDMIOutputID < 0) {
        return -1;
    }

    QBVideoOutputConfig config;
    if (QBPlatformGetOutputConfig(HDMIOutputID, &config) != 0) {
        SvLogWarning("%s(): Cannot get output config of output %s", __func__, QBPlatformGetOutputName(HDMIOutputID));
        return -1;
    }

    if (!config.enabled) {
        SvLogWarning("%s(): Output %s not enabled", __func__, QBPlatformGetOutputName(HDMIOutputID));
        return -1;
    }

    if (!(config.capabilities & QBOutputCapability_CEC)) {
        SvLogWarning("%s(): Output %s dosn't have  CEC capabilities", __func__, QBPlatformGetOutputName(HDMIOutputID));
        return -1;
    }

    return HDMIOutputID;
}

SvLocal void QBCecServiceCECReceiveMessage(QBCecService self, QBCECMessage msg)
{
    struct QBCECStatus_ CECStatus;

    int outputID = QBCecServiceGetEnabledHDMICECOutput();

    if (outputID < 0) {
        return;
    }

    if (QBPlatformGetOutputCECStatus(outputID, &CECStatus) != 0) {
        return;
    }

    // Polling Message
    if (msg->len == 0) {
        return;
    }

    // We want only messages sent to us
    if (msg->dest != CECStatus.logicalAddr && msg->dest != QBPlatformCECDeviceAddress_Broadcast) {
        return;
    }

    switch (msg->msg[0]) {
        case QBPlatformCECOpcode_GiveOSDName:
        {
            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_SetOSDName);
            QBCECCallParametersAddString(params, QBCECCallParametersKey_OSDNAME, "STB");
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, msg->src);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;
        case QBPlatformCECOpcode_GivePhysicalAddress:
        {
            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_ReportPhysicalAddress);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_PHYSICALADDR, CECStatus.physicalAddr);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DEVICETYPE, QBPlatformCECDeviceType_Tuner);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_Broadcast);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;
        case QBPlatformCECOpcode_RequestActiveSource:
        {
            // send iff not in standbay
            // if there is no standbyAgent we assume that we are not in standby
            if (self->appGlobals->standbyAgent && QBStandbyAgentIsStandby(self->appGlobals->standbyAgent)) {
                break;
            }

            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_ActiveSource);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_PHYSICALADDR, CECStatus.physicalAddr);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_Broadcast);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;
        case QBPlatformCECOpcode_GiveDevicePowerStatus:
        {
            QBPlatformCECPowerStatus powerStatus = QBPlatformCECPowerStatus_On;
            if (self->appGlobals->standbyAgent && QBStandbyAgentIsStandby(self->appGlobals->standbyAgent)) {
                powerStatus = QBPlatformCECPowerStatus_Standby;
            }
            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_ReportPowerStatus);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_POWERSTATUS, powerStatus);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, msg->src);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;
        case QBPlatformCECOpcode_GetCECVersion:
        {
            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_CECVersion);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_CECVERSION, QBPlatformCECCECVersion_Version1_3a);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, msg->src);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;

        case QBPlatformCECOpcode_Standby:
        {
            if (self->appGlobals->standbyAgent) {
                QBStandbyStateData standbayStateData = QBStandbyStateDataCreate(QBStandbyState_standby, (SvObject) self, false, SVSTRING("CEC"));
                QBStandbyAgentSetWantedState(self->appGlobals->standbyAgent, standbayStateData);
                SVRELEASE(standbayStateData);
            }
        }
        break;
        case QBPlatformCECOpcode_SetMenuLanguage:
            break;
        default:
        {
            if (msg->dest == QBPlatformCECDeviceAddress_Broadcast) {
                break;
            }

            QBCECCallParameters params = QBCECCallParametersCreate();
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_FeatureAbort);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODEARG, msg->msg[0]);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_ABORTREASON, QBPlatformCECAbortReason_Refused);
            QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, msg->src);
            QBCECSendMessage(self->appGlobals->cec, params);
            SVRELEASE(params);
        }
        break;
    }
}

SvLocal void QBCecServiceCECAction(QBCecService self, QBPlatformCECAction action)
{
    if (SvObjectIsInstanceOf((SvObject) action, QBPlatformCECActionMessageReceived_getType())) {
        QBCecServiceCECReceiveMessage(self, QBPlatformCECActionMessageReceivedGetMessage((QBPlatformCECActionMessageReceived) action));
    } else if (SvObjectIsInstanceOf((SvObject) action, QBPlatformCECActionReady_getType())) {
        if (self->appGlobals->standbyAgent == NULL || !QBStandbyAgentIsStandby(self->appGlobals->standbyAgent)) {
            QBCecServiceOneTouchPlay(self);
        }
    }
}

SvLocal void QBCecServiceHALEventCallback(void *base_,
                                          QBPlatformEvent event,
                                          long long int outputID,
                                          SvObject action)
{
    QBCecService self = base_;

    if (!self->appGlobals->cec) {
        return;
    }

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }

    // We support here only events which argA is outputID
    if (outputID != HDMIOutputID) {
        return;
    }

    switch (event) {
        case QBPlatformEvent_outputChanged:
            QBPlatformOutputCECEnable(HDMIOutputID, self->enabled);
            break;
        case QBPlatformEvent_cecAction:
            QBCecServiceCECAction(self, (QBPlatformCECAction) action);
            break;
        default:
            break;
    }
}

void QBCecServiceStart(QBCecService self)
{
    assert(self);

    QBConfigAddListener((SvGenericObject) self, QBCecServiceHASCEC);

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }

    QBPlatformAddEventCallback(&QBCecServiceHALEventCallback, self);
    QBPlatformOutputCECEnable(HDMIOutputID, self->enabled);
}

void QBCecServiceStop(QBCecService self)
{
    assert(self);

    QBConfigRemoveListener((SvGenericObject) self, QBCecServiceHASCEC);

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }
    QBPlatformOutputCECEnable(HDMIOutputID, false);
    QBPlatformRemoveEventCallback(&QBCecServiceHALEventCallback, self);
}

SvLocal QBPlatformCECUICommand QBCecServiceMapKey(uint32_t code)
{
    switch (code) {
        case QBKEY_VOLUP:
            return QBPlatformCECUICommand_VolumeUp;
        case QBKEY_VOLDN:
            return QBPlatformCECUICommand_VolumeDown;
        default:
            return QBPlatformCECUICommand__last__;
    }
}

bool QBCecServiceInputEventHandler(QBCecService self, const QBInputEvent *e)
{
    assert(self);
    assert(e);

    if (!self->enabled || !self->appGlobals->cec) {
        return false;
    }

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return false;
    }

    QBPlatformCECUICommand command = QBCecServiceMapKey(e->u.key.code);
    if (command == QBPlatformCECUICommand__last__) {
        return false;
    }

    if (e->type == QBInputEventType_keyTyped) {
        QBCECCallParameters params = QBCECCallParametersCreate();
        QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_UserControlPressed);
        QBCECCallParametersAddInteger(params, QBCECCallParametersKey_BUTTON, command);
        QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_TV);
        QBCECSendMessage(self->appGlobals->cec, params);
        SVRELEASE(params);
        return true;
    } else if (e->type == QBInputEventType_keyReleased) {
        QBCECCallParameters params = QBCECCallParametersCreate();
        QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_UserControlReleased);
        QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_TV);
        QBCECSendMessage(self->appGlobals->cec, params);
        SVRELEASE(params);
        return true;
    }

    return false;
}

void QBCecServiceOneTouchPlay(QBCecService self)
{
    assert(self);

    if (!self->enabled) {
        return;
    }

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }

    struct QBCECStatus_ CECStatus;
    if (QBPlatformGetOutputCECStatus(HDMIOutputID, &CECStatus) != 0) {
        return;
    }

    QBCECCallParameters params = QBCECCallParametersCreate();
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_TextViewOn);
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_TV);
    QBCECSendMessage(self->appGlobals->cec, params);
    SVRELEASE(params);

    params = QBCECCallParametersCreate();
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_ActiveSource);
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_PHYSICALADDR, CECStatus.physicalAddr);
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_Broadcast);
    QBCECSendMessage(self->appGlobals->cec, params);
    SVRELEASE(params);
}

void QBCecServiceBroadcastStandby(QBCecService self)
{
    assert(self);

    if (!self->enabled || !self->isActiveSource) {
        return;
    }

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }

    QBCECCallParameters params = QBCECCallParametersCreate();
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_OPCODE, QBPlatformCECOpcode_Standby);
    QBCECCallParametersAddInteger(params, QBCECCallParametersKey_DESTINATION, QBPlatformCECDeviceAddress_Broadcast);
    QBCECSendMessage(self->appGlobals->cec, params);
    SVRELEASE(params);
}

SvLocal void QBCecServiceStatusChanged(SvGenericObject self_, const char *key, const char *value)
{
    assert(self_);
    assert(key);
    assert(strcmp(key, QBCecServiceHASCEC) == 0);

    QBCecService self = (QBCecService) self_;

    self->enabled = QBCecServiceParseConfigGet(value);

    QBVolume volume = (QBVolume) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBVolume"));
    if (volume) {
        if (self->enabled) {
            if (!QBVolumeIsMuted(volume, NULL)) {
                QBVolumeSet(volume, QBVIEWPORT_VOL_MAX - 1);
            }
            QBObservableAddObserver((QBObservable) volume, (SvObject) self, NULL);
        } else {
            QBObservableRemoveObserver((QBObservable) volume, (SvObject) self, NULL);
            QBVolumeRestoreToUserSettings(volume);
        }
    }

    int HDMIOutputID = QBCecServiceGetEnabledHDMICECOutput();
    if (HDMIOutputID < 0) {
        return;
    }
    QBPlatformOutputCECEnable(HDMIOutputID, self->enabled);
}
