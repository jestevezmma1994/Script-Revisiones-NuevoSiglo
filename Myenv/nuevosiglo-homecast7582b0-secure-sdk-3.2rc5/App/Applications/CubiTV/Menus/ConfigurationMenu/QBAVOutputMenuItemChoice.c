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

#include "QBAVOutputMenuItemChoice.h"

#include <libintl.h>

#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBConfig.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBApplicationController.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Logic/AudioOutputLogic.h>
#include <Logic/AudioTrackLogic.h>
#include <Services/HDMIService.h>
#include <Logic/TVLogic.h>
#include <Logic/VideoOutputLogic.h>
#include <main.h>
#include <Services/core/aspectRatioAgent.h>
#include "QBConfigurationMenuUtils.h"

struct QBAVOutputMenuItemChoice_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBActiveTree tree;

    QBConfigurationMenuRestrictedNodesHandler restrictedNodesHandler;
    HDMIService hdmiService;
};

SvLocal void
QBAVOutputMenuItemNodeHandleSetHdOutputModeAction(QBAVOutputMenuItemChoice self, QBActiveTreeNode node, SvString *optVal)
{
    QBOutputStandard newMode = QBOutputStandard_current;
    QBTVSystem systemType = QBTVSystem_PAL;
    const char *value = QBConfigGet("TVSYSTEM");
    if (value) {
        if (strcasecmp(value, "NTSC") == 0) {
            systemType = QBTVSystem_NTSC;
        } else if (strcasecmp(value, "PAL") == 0) {
            systemType = QBTVSystem_PAL;
        } else {
            SvLogWarning("CubiTV: invalid TVSYSTEM configuration value");
        }
    }

    newMode = QBPlatformFindModeByName(SvStringCString(*optVal), systemType);
    if (newMode == QBOutputStandard_none) {
        if (SvStringEqualToCString(*optVal, "480P"))
            newMode = QBOutputStandard_480p59;
        else if (SvStringEqualToCString(*optVal, "576P"))
            newMode = QBOutputStandard_576p50;
        else if (SvStringEqualToCString(*optVal, "720P"))
            newMode = QBOutputStandard_720p50;
        else if (SvStringEqualToCString(*optVal, "1080I"))
            newMode = QBOutputStandard_1080i50;
        else if (SvStringEqualToCString(*optVal, "1080P"))
            newMode = QBOutputStandard_1080p50;
        else if (SvStringEqualToCString(*optVal, "1080P25"))
            newMode = QBOutputStandard_1080p25;
        else if (SvStringEqualToCString(*optVal, "2160P24"))
            newMode = QBOutputStandard_2160p24;
        else if (SvStringEqualToCString(*optVal, "2160P25"))
            newMode = QBOutputStandard_2160p25;
        else if (SvStringEqualToCString(*optVal, "2160P"))
            newMode = QBOutputStandard_2160p30;
        else if (SvStringEqualToCString(*optVal, "2160P30"))
            newMode = QBOutputStandard_2160p30;
        else if (SvStringEqualToCString(*optVal, "4096x2160P"))
            newMode = QBOutputStandard_4096x2160p24;
    }

    if (newMode != QBOutputStandard_current) {
        if (HDMIServiceIsOriginalModeActive(self->hdmiService)) {
            QBConfigSet("HDOUTORIGINALMODE", "disabled");
            HDMIServiceDisableOriginalMode(self->hdmiService);
        }
        if (HDMIServiceIsAutomaticModeActive(self->hdmiService)) {
            QBConfigSet("HDOUTAUTOMATICMODE", "disabled");
            HDMIServiceDisableAutomaticMode(self->hdmiService);
        }
        HDMIServiceSetVideoMode(self->hdmiService, newMode);
    }
}

SvLocal bool
QBAVOutputMenuItemNodeHandleAction(QBAVOutputMenuItemChoice self, QBActiveTreeNode node, SvString *optVal)
{
    bool handled = false;
    SvString action = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("action"));
    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoOutputLogic"));

    if (action) {
        if (SvStringEqualToCString(action, "Set TV System")) {
            VideoOutputLogicTVSystemChanged(videoOutputLogic, QBPlatformGetTVSystemByName(SvStringCString(*optVal)));
            handled = true;
        } else if (SvStringEqualToCString(action, "Enable automatic frame rate selection")) {
            VideoOutputLogicEnableAutomaticFrameRateSelection(videoOutputLogic);
            HDMIServiceEnableAutomaticFrameRateSelection(self->hdmiService);
            handled = true;
        } else if (SvStringEqualToCString(action, "Disable automatic frame rate selection")) {
            VideoOutputLogicDisableAutomaticFrameRateSelection(videoOutputLogic);
            HDMIServiceDisableAutomaticFrameRateSelection(self->hdmiService);
            handled = true;
        } else if (SvStringEqualToCString(action, "Set HD output mode")) {
            QBAVOutputMenuItemNodeHandleSetHdOutputModeAction(self, node, optVal);
            handled = true;
        } else if (SvStringEqualToCString(action, "Enable HDMI original mode")) {
            QBConfigSet("HDOUTAUTOMATICMODE", "disabled");
            QBConfigSet("HDOUTORIGINALMODE", "enabled");
            QBConfigSave();
            HDMIServiceEnableOriginalMode(self->hdmiService);
            QBOutputStandard selectedMode = HDMIServiceGetSelectedVideoMode(self->hdmiService);
            const char *modeStr = QBPlatformGetModeName(selectedMode);
            if (modeStr)
                *optVal = SVAUTORELEASE(SvStringCreate(modeStr, NULL));
            handled = true;
        } else if (SvStringEqualToCString(action, "Enable HDMI automatic mode")) {
            QBConfigSet("HDOUTORIGINALMODE", "disabled");
            QBConfigSet("HDOUTAUTOMATICMODE", "enabled");
            QBConfigSave();
            HDMIServiceEnableAutomaticMode(self->hdmiService);
            QBOutputStandard selectedMode = HDMIServiceGetSelectedVideoMode(self->hdmiService);
            const char *modeStr = QBPlatformGetModeName(selectedMode);
            if (modeStr)
                *optVal = SVAUTORELEASE(SvStringCreate(modeStr, NULL));
            handled = true;
        } else if (SvStringEqualToCString(action, "Setup S/PDIF audio delay")) {
            QBTVLogicEnableAudioDelaySetupMode(self->appGlobals->tvLogic);
            QBApplicationControllerPushContext(self->appGlobals->controller, self->appGlobals->newTV);
            handled = true;
        } else if (SvStringEqualToCString(action, "Set AV signal type") || SvStringEqualToCString(action, "Set SCART signal type")) {
            VideoOutputLogicSetSCARTSignalType(videoOutputLogic, SvStringCString(*optVal));
            handled = true;
        } else if (SvStringEqualToCString(action, "Set RF Modulator")) {
            VideoOutputLogicSetRFModulator(videoOutputLogic, SvStringCString(*optVal));
            handled = true;
        } else if (SvStringEqualToCString(action, "Toggle multichannel mode")) {
            bool isMultiChannelMode = false;
            if (*optVal && SvStringEqualToCString(*optVal, "YES"))
                isMultiChannelMode = true;
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            AudioOutputLogicSetMultiChannelMode((AudioOutputLogic) QBServiceRegistryGetService(registry, SVSTRING("AudioOutputLogic")),
                                                isMultiChannelMode);
            AudioTrackLogic audioTrackLogic =
                (AudioTrackLogic) QBServiceRegistryGetService(registry, SVSTRING("AudioTrackLogic"));
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
        }
    }

    return handled;
}

SvLocal bool
QBAVOutputMenuItemNodeApply(void *self_,
                            SvObject node_,
                            SvObject nodePath_)
{
    if (!SvObjectIsInstanceOf((SvObject) node_, QBActiveTreeNode_getType()))
        return false;

    bool handled = false;
    QBAVOutputMenuItemChoice self = (QBAVOutputMenuItemChoice) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));

    if (QBAVOutputMenuItemNodeHandleAction(self, node, &optVal))
        handled = true;

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode) node);
    if (!parent)
        return handled;

    QBServiceRegistry serviceRegistry = QBServiceRegistryGetInstance();
    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(serviceRegistry,
                                                                                       SVSTRING("VideoOutputLogic"));
    QBAspectRatioAgent aspectRatioAgent = (QBAspectRatioAgent) QBServiceRegistryGetService(serviceRegistry,
                                                                                           SVSTRING("QBAspectRatioAgent"));

    SvString parentId = QBActiveTreeNodeGetID(parent);
    if (parentId) {
        if (SvStringEqualToCString(parentId, "AspectRatioSD") ||
            SvStringEqualToCString(parentId, "AspectRatioHD")) {
            const char *outputType = SvStringCString(parentId) + SvStringLength(parentId) - 2;
            QBAspectRatioAgentSetAspectRatio(aspectRatioAgent, outputType, SvStringCString(optVal));
            handled = true;
        } else if (SvStringEqualToCString(parentId, "VideoContentModeSD") ||
                   SvStringEqualToCString(parentId, "VideoContentModeHD")) {
            const char *outputType = SvStringCString(parentId) + SvStringLength(parentId) - 2;
            VideoOutputLogicSetVideoContentMode(videoOutputLogic, outputType, SvStringCString(optVal));
            handled = true;
        } else if (SvStringEqualToCString(parentId, "AspectRatio")) {
            QBAspectRatioAgentSetAspectRatio(aspectRatioAgent, "SD", SvStringCString(optVal));
            QBAspectRatioAgentSetAspectRatio(aspectRatioAgent, "HD", SvStringCString(optVal));
            handled = true;
        } else if (SvStringEqualToCString(parentId, "VideoContentMode")) {
            VideoOutputLogicSetVideoContentMode(videoOutputLogic, "SD", SvStringCString(optVal));
            VideoOutputLogicSetVideoContentMode(videoOutputLogic, "HD", SvStringCString(optVal));
            handled = true;
        }
    }

    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
    if (!optName || !optVal)
        return handled;

    // It's 'temporary' hack only for generic release we should fix it in the CI. See on Jira CUB-4016 and related tasks.
    // BEGIN_THE_HACK
    if (SvStringEqualToCString(optName, "VIDEOOUTHD")) {
        HDMIService hdmiService = (HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService"));
        if (hdmiService) {
            const char* newModeStr = SvStringCString(optVal);
            const char* tvSystemStr = QBConfigGet("TVSYSTEM");
            QBTVSystem tvSystem = QBPlatformGetTVSystemByName(tvSystemStr);
            QBOutputStandard standard = QBPlatformFindModeByName(newModeStr, tvSystem);
            if (standard == QBOutputStandard_none) {
                SvLogWarning("%s(): not found output standard for mode: '%s' and system: '%s'", __func__, newModeStr, tvSystemStr);
                return true;
            }

            int outputID = QBPlatformFindOutput(QBOutputType_HDMI, NULL, 0, true, false);
            if (outputID < 0) {
                SvLogWarning("%s(): can't find HDMI output", __func__);
                return true;
            }
            bool isSupported = false;
            if (QBPlatformOutputIsModeSupported(outputID, standard, &isSupported) < 0) {
                SvLogWarning("%s(): platform error during check mode support", __func__);
                return true;
            }

            if (!isSupported) {
                SvLogWarning("%s(): can't set mode (%s) not supported by platform", __func__, newModeStr);
                return true;
            }
        }
    }
    // END_THE_HACK

    handled = true;
    QBConfigSet(SvStringCString(optName), SvStringCString(optVal));
    QBConfigSave();
    SvObject parentPath = SvObjectCopy(nodePath_, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree,
                                            getIterator, parentPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->tree, parentPath, 0, nodesCount, NULL);
    SVRELEASE(parentPath);

    return handled;
}

SvLocal bool
QBAVOutputMenuItemNodeSelected(SvObject self_,
                               SvObject node_,
                               SvObject nodePath_)
{
    QBAVOutputMenuItemChoice self = (QBAVOutputMenuItemChoice) self_;
    bool handled = QBConfigurationMenuHandleItemNodeSelection(self->restrictedNodesHandler, node_, nodePath_);
    if (!handled)
        handled = QBAVOutputMenuItemNodeApply((void *) self_, node_, nodePath_);

    return handled;
}

SvLocal void
QBAVOutputMenuItemChoiceDestroy(void *self_)
{
    QBAVOutputMenuItemChoice self = self_;

    SVRELEASE(self->tree);
    SVTESTRELEASE(self->restrictedNodesHandler);
    SVTESTRELEASE(self->hdmiService);
}

SvLocal SvType
QBAVOutputMenuItemChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAVOutputMenuItemChoiceDestroy
    };

    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBAVOutputMenuItemNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBAVOutputMenuItemChoice",
                            sizeof(struct QBAVOutputMenuItemChoice_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            NULL);
    }

    return type;
}

SvLocal SvArray QBAVOutputMenuItemChoiceNodesIdToRemoveArrayCreate(QBAVOutputMenuItemChoice self)
{
    SvArray nodesToRemove = SvArrayCreate(NULL);

    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoOutputLogic"));
    if (VideoOutputLogicIsOutputConfigurationSeparated(videoOutputLogic)) {
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("AspectRatio"));
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("VideoContentMode"));
    } else {
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("AspectRatioSD"));
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("AspectRatioHD"));
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("VideoContentModeSD"));
        SvArrayAddObject(nodesToRemove, (SvObject) SVSTRING("VideoContentModeHD"));
    }

    return nodesToRemove;
}

QBAVOutputMenuItemChoice
QBAVOutputMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!tree) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL tree passed");
        goto out;
    }

    QBAVOutputMenuItemChoice self = (QBAVOutputMenuItemChoice) SvTypeAllocateInstance(QBAVOutputMenuItemChoice_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBMainMenuItemController");
        goto out;
    }
    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);

    SvArray nodesToRemove = QBAVOutputMenuItemChoiceNodesIdToRemoveArrayCreate(self);
    SvIterator it = SvArrayIterator(nodesToRemove);

    SvString nodeId = NULL;
    while ((nodeId = (SvString) SvIteratorGetNext(&it))) {
        QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, nodeId);
        if (node) {
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        }
    }
    SVRELEASE(nodesToRemove);

    self->restrictedNodesHandler =
        QBConfigurationMenuRestrictedNodesHandlerCreate(appGlobals, (void *) self,
                                                        QBAVOutputMenuItemNodeApply,
                                                        menu, tree, &error);
    if (!self->restrictedNodesHandler) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBConfigurationMenuRestrictedNodesHandler");
        SVRELEASE(self);
        goto out;
    }

    self->hdmiService = (HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService"));
    if (!self->hdmiService) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_notFound, error,
                                           "Cannot get HDMIService from QBServiceRegistry");
        SVRELEASE(self);
        goto out;
    }
    SVRETAIN(self->hdmiService);

    return self;
out:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}
