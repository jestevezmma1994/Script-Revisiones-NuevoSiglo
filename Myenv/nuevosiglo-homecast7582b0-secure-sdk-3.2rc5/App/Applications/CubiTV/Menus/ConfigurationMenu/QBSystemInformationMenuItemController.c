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

#include "QBSystemInformationMenuItemController.h"

#include <errno.h>
#include <libintl.h>
#include <sys/stat.h>

#include <QBCAS.h>
#include <QBCASInfo.h>
#include <QBSmartCardMonitor.h>
#include <QBViewRightWeb.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <QBConf.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBTypes.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreePath.h>
#include <Services/core/QBMiddlewareManager.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <main.h>
#include <Services/core/QBNetworkWatcher.h>
#include <Services/QBViewRightManagers/QBViewRightWebManager.h>
#include <Services/core/QBPushReceiver.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Utils/loadFile.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <SvJSON/SvJSONParse.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <stringUtils.h>
#include <safeFiles.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#define CERTS_DIR "/etc/certs/private"
#define CERT_FILE CERTS_DIR "/cert.pem"

#define PUSH_RECEIVER_ACTION_NAME SVSTRING("push_receiver_status")

#define ADDRESS_TRANSLATION_FILE_PATH "/usr/local/share/CubiTV/Configurations/middlewareUrls"

#if SV_LOG_LEVEL > 0
    #define moduleName "QBConfigurationMenu"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "");
    #define log_debug(fmt, ...)     do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...)   do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif /* SV_LOG_LEVEL */

struct QBSystemInformationMenuItemController_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTree tree;

    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvRID iconRID, searchIconRID;

    CubiwareMWCustomerInfoStatus customerInfoMontitorStatus;
    QBPushReceiverStatus pushServerStatus;
    TraxisWebSessionState traxisWebSessionManagerStatus;

    bool isNetworkAvailable;

    struct {
        QBViewRightWebState state;
        int errorCode;
    } viewRightStatus;

    QBActiveTreeNode middlewareStatusNode;
    QBActiveTreeNode middlewareAddressNode;
    QBActiveTreeNode pushServerStatusNode;
    QBActiveTreeNode certsStatusNode;

    QBActiveTreeNode traxisStatusNode, traxisURLNode, traxisSmartcardIDNode, traxisServiceGroupNode, traxisVoDCategory, traxisDeviceIDType, traxisSummaryNode;
    QBActiveTreeNode viewRightWebStatusNode;

    QBCASCmd cmd;
    QBActiveTreeNode chip_id;
};

SvLocal void
QBSystemInformationMenuItemControllerDestroy(void *self_)
{
    QBSystemInformationMenuItemController self = self_;
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->itemConstructor);
    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBNetworkWatcher"));
    if (networkWatcher) {
        QBNetworkWatcherRemoveListener(networkWatcher, (SvObject) self, NULL);
    }
    SVTESTRELEASE(self->chip_id);
    SVTESTRELEASE(self->middlewareStatusNode);
    SVTESTRELEASE(self->middlewareAddressNode);
    SVTESTRELEASE(self->pushServerStatusNode);
    SVTESTRELEASE(self->viewRightWebStatusNode);
    SVTESTRELEASE(self->traxisStatusNode);
    SVTESTRELEASE(self->traxisDeviceIDType);
    SVTESTRELEASE(self->traxisSmartcardIDNode);
    SVTESTRELEASE(self->traxisServiceGroupNode);
    SVTESTRELEASE(self->traxisVoDCategory);
    SVTESTRELEASE(self->traxisSummaryNode);
    SVTESTRELEASE(self->traxisURLNode);

    SVRELEASE(self->tree);
}

typedef enum {
    QBSystemInformationMenuCertsStatus_ok = 0,           /** No error */
    QBSystemInformationMenuCertsStatus_certsInRootOK,    /** Valid certificate in rootfs */
    QBSystemInformationMenuCertsStatus_certsInRootError, /** Invalid certificate in rootfs */
    QBSystemInformationMenuCertsStatus_certInvalid,      /** Invalid certificate */
    QBSystemInformationMenuCertsStatus_certError,        /** Certificate directory not found */
    QBSystemInformationMenuCertsStatus_generalError,     /** Can't check certificate status */
} QBSystemInformationMenuCertsStatus;

SvLocal QBSystemInformationMenuCertsStatus
QBSystemInformationMenuGetCertsStatus(void)
{
    struct stat sb;
    int ret = lstat(CERTS_DIR, &sb);
    if (ret != 0) {
        SvLogError("%s() :: lstat(%s) call failed : %d / %s", __func__,
                   CERTS_DIR, ret, strerror(errno));
        return QBSystemInformationMenuCertsStatus_generalError;
    } else if (!S_ISLNK(sb.st_mode)) {
        ret = stat(CERT_FILE, &sb);
        if (ret != 0) {
            SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__,
                       CERT_FILE, ret, strerror(errno));
            return QBSystemInformationMenuCertsStatus_certsInRootError;
        } else {
            SvLogNotice("%s() :: Certs in root OK", __func__);
            return QBSystemInformationMenuCertsStatus_certsInRootOK;
        }
    }

    ret = stat(CERTS_DIR, &sb);
    if (ret != 0) {
        SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__, CERTS_DIR, ret, strerror(errno));
        return QBSystemInformationMenuCertsStatus_certError;
    }

    ret = stat(CERT_FILE, &sb);
    if (ret != 0) {
        SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__, CERT_FILE, ret, strerror(errno));
        return QBSystemInformationMenuCertsStatus_certInvalid;
    }

    SvLogNotice("%s() :: Certs OK", __func__);
    return QBSystemInformationMenuCertsStatus_ok;
}

SvLocal void
QBSystemInformationMenuNodeUpdate(QBActiveTreeNode node, const char* captionStr, SvObject value)
{
    if (!node)
        return;
    SvString caption = captionStr ? SvStringCreate(captionStr, NULL) : NULL;
    SvString stringVal = NULL;

    if (value && SvObjectIsInstanceOf(value, SvString_getType()) && SvStringLength((SvString) value) > 0) {
        stringVal = (SvString) value;
        SVRETAIN(stringVal);
    } else {
        stringVal = SvStringCreate(gettext("N/A"), NULL);
    }

    if (caption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) caption);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) stringVal);

    SVRELEASE(stringVal);
    SVTESTRELEASE(caption);
}

SvLocal void
QBSystemInformationMenuItemControllerHandleMiddlewareStatusNode(QBSystemInformationMenuItemController self,
                                                                QBXMBItem item)
{
    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBNetworkWatcher"));
    if (networkWatcher && !self->isNetworkAvailable) {
        item->subcaption = SvStringCreate(gettext("Connection problem"), NULL);
        return;
    }

    int errorCode = 0;

    switch (self->customerInfoMontitorStatus) {
        case CubiwareMWCustomerInfoStatus_ok:
            item->subcaption = SvStringCreate(gettext("Ok"), NULL);
            break;
        case CubiwareMWCustomerInfoStatus_error:
            if (self->appGlobals->customerInfoMonitor)
                errorCode = CubiwareMWCustomerInfoGetErrorCode(self->appGlobals->customerInfoMonitor);
            item->subcaption = SvStringCreateWithFormat("%s #%04X", gettext("Error"), errorCode);
            break;
        case CubiwareMWCustomerInfoStatus_unknown:
        default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
    }
}

SvLocal void
QBSystemInformationMenuItemControllerHandlePushServerStatusNode(QBSystemInformationMenuItemController self,
                                                                QBXMBItem item)
{
    assert(item->subcaption == NULL);

    int errorCode = QBWebSocketError_notset;

    switch (self->pushServerStatus) {
        case QBPushReceiver_connected:
            item->subcaption = SvStringCreate(gettext("Ok"), NULL);
            break;
        case QBPushReceiver_disconnected:
            if (self->appGlobals->pushReceiver) {
                errorCode = QBPushReceiverGetLastError(self->appGlobals->pushReceiver);
            }
            item->subcaption = SvStringCreateWithFormat("%s #%04X", gettext("Error"), errorCode);
            break;
        case QBPushReceiver_unknown:
        default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
    }
}

SvLocal void
QBSystemInformationMenuItemControllerHandleTraxisStatusNode(QBSystemInformationMenuItemController self,
                                                            QBXMBItem item)
{
    switch (self->traxisWebSessionManagerStatus) {
        case TraxisWebSessionState_initial:
            item->subcaption = SvStringCreate(gettext("Not connected"), NULL);
            break;
        case TraxisWebSessionState_handshake:
            item->subcaption = SvStringCreate(gettext("Waiting for response"), NULL);
            break;
        case TraxisWebSessionState_active:
            item->subcaption = SvStringCreate(gettext("Connected"), NULL);
            break;
        case TraxisWebSessionState_stopped:
            item->subcaption = SvStringCreate(gettext("Stopped"), NULL);
            break;
        default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
    }
}

SvLocal void
QBSystemInformationMenuItemControllerHandleViewRightWebStatusNode(QBSystemInformationMenuItemController self,
                                                                  QBXMBItem item)
{
    switch (self->viewRightStatus.state) {
        case QBViewRightWebState_ConnectedAndProvisioned:
            item->subcaption = SvStringCreate(gettext("Connected"), NULL);
            break;
        case QBViewRightWebState_BadCertificate:
            item->subcaption = SvStringCreate(gettext("Bad certificate"), NULL);
            break;
        case QBViewRightWebState_Created:
        case QBViewRightWebState_InitializationProcess:
            item->subcaption = SvStringCreate(gettext("Connecting..."), NULL);
            break;
        case QBViewRightWebState_NotConnected:
            item->subcaption = SvStringCreate(gettext("Not connected"), NULL);
            break;
        case QBViewRightWebState_NotProvisioned:
            item->subcaption = SvStringCreate(gettext("No subscription"), NULL);
            break;
        case QBViewRightWebState_Error:
            item->subcaption = SvStringCreateWithFormat("%s #%X", gettext("Error"), self->viewRightStatus.errorCode);
            break;
        case QBViewRightWebState_InitializationError:
            item->subcaption = SvStringCreateWithFormat("%s #%X", gettext("Initialization Error"), self->viewRightStatus.errorCode);
            break;
        default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
    }
}

// It returns common name for previously defined addresses
SvLocal SvString
QBConfigurationMenuUrlNameCreate(SvString address)
{
    SvString translatedAddress = NULL;
    if (!address) {
        log_error("Address not defined");
        return translatedAddress;
    }

    SvHashTable dict = NULL;
    SvString confFile = SvStringCreateWithFormat("%s/%s", SvGetRuntimePrefix(), ADDRESS_TRANSLATION_FILE_PATH);
    dict = (SvHashTable) SvJSONParseFile(SvStringCString(confFile), false, NULL);
    if (!dict || !SvObjectIsInstanceOf((SvObject) dict, SvHashTable_getType())) {
        log_error("Can't parse translation file: %s", SvStringCString(confFile));
        goto error;
    }

    SvValue addressValue =  (SvValue) SvHashTableFind(dict, (SvObject)address);
    if (!addressValue) {
        log_debug("No match found for given domain: %s", SvStringCString(address));
        goto error;
    }

    if (!SvValueIsString(addressValue)) {
        log_error("Value stored in dictionary is not a string");
        goto error;
    }
    translatedAddress = SVRETAIN(SvValueGetString(addressValue));

error:
    SVTESTRELEASE(confFile);
    SVTESTRELEASE(dict);
    return translatedAddress;
}

// This function set the subcaption of a node to domain name of current
// MW or common name if it was previously defined.
SvLocal void
QBSystemInformationMenuItemControllerHandleMiddlewareAddressNode(QBSystemInformationMenuItemController self, QBXMBItem item)
{
    SvString prefixUrl = NULL;
    SvString authority = NULL;
    SvString commonName = NULL;

    if (self->appGlobals->middlewareManager) {
        prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    }

    if (!prefixUrl) {
        item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
        return;
    }

    SvURI uri = SvURICreateWithString(prefixUrl, NULL);
    if (uri) {
        authority = SvURIAuthority(uri);
        commonName = QBConfigurationMenuUrlNameCreate(authority);
        if (commonName) {
            item->subcaption = commonName;
        } else if (authority) {
            item->subcaption = SVRETAIN(authority);
        } else {
            item->subcaption = SVSTRING("N/A");
        }
        SVRELEASE(uri);
    } else {
        item->subcaption = SVRETAIN(prefixUrl);
    }
}

SvLocal void
QBSystemInformationMenuItemControllerHandleNode(QBSystemInformationMenuItemController self,
                                                QBActiveTreeNode node,
                                                QBXMBItem item)
{
    const char *captionStr, *translatedCaptionStr;
    captionStr = SvStringCString(item->caption);
    translatedCaptionStr = gettext(captionStr);
    if (translatedCaptionStr != captionStr)
        item->caption = SvStringCreate(translatedCaptionStr, NULL);
    else
        SVRETAIN(item->caption);

    if (node == self->middlewareStatusNode) {
        QBSystemInformationMenuItemControllerHandleMiddlewareStatusNode(self, item);
    } else if (node == self->middlewareAddressNode) {
        QBSystemInformationMenuItemControllerHandleMiddlewareAddressNode(self, item);
    } else if (node == self->pushServerStatusNode) {
        QBSystemInformationMenuItemControllerHandlePushServerStatusNode(self, item);
    } else if (node == self->certsStatusNode) {
        QBSystemInformationMenuCertsStatus status = QBSystemInformationMenuGetCertsStatus();

        if (status == QBSystemInformationMenuCertsStatus_ok)
            item->subcaption = SvStringCreate(gettext("Ok"), NULL);
        else
            item->subcaption = SvStringCreateWithFormat("%s #%x", gettext("error"), status);
    } else if (node == self->traxisStatusNode) {
        QBSystemInformationMenuItemControllerHandleTraxisStatusNode(self, item);
    } else if (node == self->traxisURLNode) {
        SvString traxisURLStr = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Traxis);
        if (traxisURLStr)
            item->subcaption = SVRETAIN(traxisURLStr);
        else
            item->subcaption = SVSTRING("N/A");
    } else if (node == self->traxisSmartcardIDNode) {
        SvString smartCardID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
        item->subcaption = smartCardID ? SVRETAIN(smartCardID) : SVSTRING("N/A");
    } else if (node == self->traxisServiceGroupNode) {
        const char *groupIDStr = QBConfigGet("EVENTIS_GROUP_ID");
        item->subcaption = groupIDStr ? SvStringCreate(groupIDStr, NULL) : SVSTRING("N/A");
    } else if (node == self->traxisVoDCategory) {
        const char *groupIDStr = QBConfigGet("VOD_CATALOG");
        item->subcaption = groupIDStr ? SvStringCreate(groupIDStr, NULL) : SVSTRING("N/A");
    } else if (node == self->traxisDeviceIDType) {
        const char *traxisDeviceIDTypeStr = QBConfigGet("TRAXIS_DEVICE_ID_TYPE");
        item->subcaption = traxisDeviceIDTypeStr ? SvStringCreate(traxisDeviceIDTypeStr, NULL) : SVSTRING("N/A");
    } else if (node == self->traxisSummaryNode) {
        SvString smartCardID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
        const char *groupIDStr = QBConfigGet("EVENTIS_GROUP_ID");
        int groupID = atoi(groupIDStr);
        item->subcaption = SvStringCreateWithFormat("%c%s:%s:%X",
            QBConfigGet("VOD_CATALOG")[0], QBConfigGet("TRAXIS_IP_HEX"), smartCardID ? SvStringCString(smartCardID) : "N/A", groupID);
    } else if (node == self->viewRightWebStatusNode) {
        QBSystemInformationMenuItemControllerHandleViewRightWebStatusNode(self, item);
    } else {
        const char *subcaptionStr, *translatedSubcaptionStr;
        item->subcaption = (SvString) (QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
        if (item->subcaption) {
            subcaptionStr = SvStringCString(item->subcaption);
            translatedSubcaptionStr = gettext(subcaptionStr);
            if (translatedSubcaptionStr != subcaptionStr)
                item->subcaption = SvStringCreate(translatedSubcaptionStr, NULL);
            else
                SVRETAIN(item->subcaption);
        }
    }
}

SvLocal SvWidget
QBSystemInformationMenuItemControllerCreateItem(SvObject self_,
                                                SvObject node_,
                                                SvObject path,
                                                SvApplication app,
                                                XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    QBXMBItem item = QBXMBItemCreate();

    item->loadingRID = self->iconRID;

    SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    item->iconURI.URI = SVTESTRETAIN(iconPath);
    item->iconURI.isStatic = true;

    item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    QBSystemInformationMenuItemControllerHandleNode(self, node, item);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBSystemInformationMenuItemControllerSetItemState(SvObject self_,
                                                  SvWidget item_,
                                                  XMBMenuState state,
                                                  bool isFocused)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBSystemInformationMenuSetMiddlewareStatus(QBSystemInformationMenuItemController self, CubiwareMWCustomerInfoStatus status)
{
    QBActiveTreeNode node = self->middlewareStatusNode;

    if (!node) {
        SvLogError("%s: no node", __func__);
        return;
    }

    self->customerInfoMontitorStatus = status;
    QBActiveTreePropagateNodeChange(self->tree, node, NULL);
}

SvLocal void
QBSystemInformationMenuSetTraxisWebSessionManagerStatus(QBSystemInformationMenuItemController self, TraxisWebSessionState state)
{
    QBActiveTreeNode node = self->middlewareStatusNode;

    if (!node) {
        SvLogError("%s: no node", __func__);
        return;
    }

    self->traxisWebSessionManagerStatus = state;
    QBActiveTreePropagateNodeChange(self->tree, node, NULL);
}

SvLocal void
QBSystemInformationMenuSetViewRightWebStatus(QBSystemInformationMenuItemController self, QBViewRightWebState state, int errorCode)
{
    QBActiveTreeNode node = self->viewRightWebStatusNode;

    if (!node) {
        SvLogError("%s: no node", __func__);
        return;
    }

    self->viewRightStatus.state = state;
    self->viewRightStatus.errorCode = errorCode;
    QBActiveTreePropagateNodeChange(self->tree, node, NULL);
}

SvLocal void
QBSystemInformationMenuCustomerInfoMonitorStatusChanged(SvObject self_)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;

    CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(self->appGlobals->customerInfoMonitor);
    QBSystemInformationMenuSetMiddlewareStatus(self, status);
}

SvLocal void
QBSystemInformationMenuCustomerInfoMonitorInfoChanged(SvObject self_, SvString customerId, SvHashTable customerInfo)
{
}

SvLocal void
QBSystemInformationMenuPushReceiverDataReceived(SvObject self_,
                                                SvString type,
                                                SvGenericObject data_)
{
}

SvLocal void
QBSystemInformationMenuPushReceiverStatusChanged(SvObject self_,
                                                 QBPushReceiverStatus status)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;

    QBActiveTreeNode node = self->pushServerStatusNode;
    if (!node) {
        SvLogError("%s: no node", __func__);
        return;
    }

    self->pushServerStatus = status;
    QBActiveTreePropagateNodeChange(self->tree, node, NULL);
}

SvLocal void
QBSystemInformationMenuTraxisWebSessionManagerStateChanged(SvObject self_, TraxisWebSessionState state)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;
    QBSystemInformationMenuSetTraxisWebSessionManagerStatus(self, state);
}

SvLocal void
QBSystemInformationMenuTraxisWebSessionManagerLanguageChanged(SvObject self_, SvString language)
{
}

SvLocal void
QBSystemInformationMenuTraxisWebSessionManagerAuthLevelChanged(SvGenericObject self, TraxisWebAuthLevel level)
{
}

SvLocal void
QBSystemInformationMenuViewRightWebStateChanged(SvObject self_, QBViewRightWebState state, int errorCode)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;
    QBSystemInformationMenuSetViewRightWebStatus(self, state, errorCode);
}

SvLocal void
QBSystemInformationMenuFillTunerSettings(QBActiveTreeNode parent_node, QBActiveTree menuTree)
{
}

SvLocal void
QBSystemInformationMenuSmartcardInfoCallback(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBSystemInformationMenuItemController self = self_;
    self->cmd = NULL;

    if (status != 0) {
        SvLogNotice("%s: Unable to get chip id", __func__);
        return;
    }

    QBSystemInformationMenuNodeUpdate(self->chip_id, NULL, (SvObject) info->chip_id);

    SvObject nodePath = QBActiveTreeCreateNodePath(self->tree, SVSTRING("chip_id"));
    if (!nodePath)
        return;

    SvObject parentPath = SvObjectCopy(nodePath, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    QBActiveTreeNode node = QBActiveTreeFindNodeByPath(self->tree, parentPath);
    size_t count = QBActiveTreeNodeGetChildNodesCount(node);
    QBActiveTreePropagateNodesChange(self->tree, parentPath, 0, count, NULL);
    SVRELEASE(parentPath);

    SVRELEASE(nodePath);
}

SvLocal void
QBSystemInformationMenuSmartcardStateCallback(void* self_, QBCASSmartcardState state)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;

    if (state == QBCASSmartcardState_correct || state == QBCASSmartcardState_removed) {
        if (self->cmd) {
            SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
            self->cmd = NULL;
        }
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBSystemInformationMenuSmartcardInfoCallback, self, &self->cmd);
    }
}

static const struct QBCASCallbacks_s s_smartcard_callbacks = {
    .smartcard_state = &QBSystemInformationMenuSmartcardStateCallback,
};

SvLocal void QBconfigurationMenuNetworkWatcherStateChanged(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController) self_;
    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBNetworkWatcher"));
    if (networkWatcher) {
        QBNetworkWatcherOverallDiagnosis diagnosis = QBNetworkWatcherGetOverallDiagnosis(networkWatcher);
        self->isNetworkAvailable = (diagnosis == QBNetworkWatcherOverallDiagnosis_ok);
    }

    if (self->middlewareStatusNode) {
        QBActiveTreePropagateNodeChange(self->tree, self->middlewareStatusNode, NULL);
    }
    if (self->pushServerStatusNode) {
        QBActiveTreePropagateNodeChange(self->tree, self->pushServerStatusNode, NULL);
    }
}

SvLocal SvType
QBSystemInformationMenuItemController_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSystemInformationMenuItemControllerDestroy
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem   = QBSystemInformationMenuItemControllerCreateItem,
        .setItemState = QBSystemInformationMenuItemControllerSetItemState,
    };

    static const struct TraxisWebSessionStateListener_ traxisStateListenerMethods = {
        .stateChanged    = QBSystemInformationMenuTraxisWebSessionManagerStateChanged,
        .languageChanged = QBSystemInformationMenuTraxisWebSessionManagerLanguageChanged,
        .authLevelChanged = QBSystemInformationMenuTraxisWebSessionManagerAuthLevelChanged,
    };

    static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
        .customerInfoChanged = QBSystemInformationMenuCustomerInfoMonitorInfoChanged,
        .statusChanged       = QBSystemInformationMenuCustomerInfoMonitorStatusChanged
    };

    static const struct QBPushReceiverListener_t pushReceiverListener = {
        .dataReceived       = QBSystemInformationMenuPushReceiverDataReceived,
        .statusChanged      = QBSystemInformationMenuPushReceiverStatusChanged
    };

    static const struct QBViewRightWebListener_s viewRightWebListenerMethods = {
        .stateChanged = QBSystemInformationMenuViewRightWebStateChanged
    };

    static struct QBObserver_ observableNetworkWatcher = {
        .observedObjectUpdated = QBconfigurationMenuNetworkWatcherStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSystemInformationMenuItemController",
                            sizeof(struct QBSystemInformationMenuItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                            QBPushReceiverListener_getInterface(), &pushReceiverListener,
                            TraxisWebSessionStateListener_getInterface(), &traxisStateListenerMethods,
                            QBViewRightWebListener_getInterface(), &viewRightWebListenerMethods,
                            QBObserver_getInterface(), &observableNetworkWatcher,
                            NULL);
    }

    return type;
}

SvLocal SvString
QBSystemInformationMenuItemControllerCreateFirmwareBuildID(QBSystemInformationMenuItemController self)
{
    SvString buildId = NULL;

    char *rawBuildId = NULL;
    int rawBuildIdLength = QBFileToBuffer("/etc/SRM_BUILD_ID", &rawBuildId);
    if (rawBuildIdLength > 0) {
        char *strippedBuildId = malloc(rawBuildIdLength + 1);
        if (strippedBuildId) {
            if (QBStringUtilsStripWhitespace(strippedBuildId, rawBuildIdLength + 1, rawBuildId) > 0) {
                buildId = SvStringCreateWithFormat("CURRENT %.6s", strippedBuildId);
            }
            free(strippedBuildId);
        }
    }
    free(rawBuildId);

    return buildId;
}

QBSystemInformationMenuItemController
QBSystemInformationMenuItemControllerCreate(AppGlobals appGlobals, QBActiveTree tree, SvErrorInfo *errorOut)
{
    QBSystemInformationMenuItemController self = (QBSystemInformationMenuItemController)
        SvTypeAllocateInstance(QBSystemInformationMenuItemController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);
    self->settingsCtx = svSettingsSaveContext();

    svSettingsPushComponent("Configuration.settings");

    self->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    self->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBNetworkWatcher"));
    if (networkWatcher) {
        QBNetworkWatcherAddListener(networkWatcher, (SvObject) self, NULL);
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(tree, SVSTRING("TunerStatus"));
    if (node)
        QBSystemInformationMenuFillTunerSettings(node, tree);

    QBUpgradeService upgradeService = QBUpgradeServiceCreate(NULL);
    if (upgradeService) {
        node = QBActiveTreeFindNode(tree, SVSTRING("FWVR"));
        if (node) {
            SvString version = QBUpgradeServiceGetLocalInformativeVersion(upgradeService);
            SvString buildId = NULL;
            if (SvStringEqualToCString(version, "CURRENT")) {
                buildId = QBSystemInformationMenuItemControllerCreateFirmwareBuildID(self);
            }
            QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) (buildId ? buildId : version));
            SVTESTRELEASE(buildId);
        }
        node = QBActiveTreeFindNode(tree, SVSTRING("PLTF"));
        if (node) {
            SvString platform = QBUpgradeServiceGetBoardName(upgradeService);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) platform);
        }
        SVRELEASE(upgradeService);
    }

    node = QBActiveTreeFindNode(tree, SVSTRING("MiddlewareStatus"));
    if (node && appGlobals->customerInfoMonitor) {
        CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(appGlobals->customerInfoMonitor);
        self->middlewareStatusNode = SVRETAIN(node);
        CubiwareMWCustomerInfoAddListener(appGlobals->customerInfoMonitor, (SvObject) self);
        QBSystemInformationMenuSetMiddlewareStatus(self, status);
    }

    self->middlewareAddressNode = SVTESTRETAIN(QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("MiddlewareAddress")));

    node = QBActiveTreeFindNode(tree, SVSTRING("PushServerStatus"));
    if (node && appGlobals->pushReceiver) {
        self->pushServerStatus = QBPushReceiver_unknown;
        self->pushServerStatusNode = SVRETAIN(node);
        SvErrorInfo error = NULL;
        QBPushReceiverAddListener(appGlobals->pushReceiver, (SvObject) self, PUSH_RECEIVER_ACTION_NAME, &error);
    }

    node = QBActiveTreeFindNode(tree, SVSTRING("TraxisStatus"));
    if (node && appGlobals->traxisWebSessionManager) {
        self->traxisStatusNode = SVRETAIN(node);
        TraxisWebSessionManagerAddListener(appGlobals->traxisWebSessionManager, (SvObject) self, NULL);
    }


    node = QBActiveTreeFindNode(tree, SVSTRING("ViewRightWebStatus"));
    if (node && appGlobals->viewRightWeb) {
        self->viewRightWebStatusNode = SVRETAIN(node);
        QBViewRightWebServiceSetListener(appGlobals->viewRightWeb, (SvObject) self);
    }

    self->traxisURLNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisURL")));
    self->traxisSmartcardIDNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisSmartCardID")));
    self->traxisServiceGroupNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisServiceGroup")));
    self->traxisVoDCategory = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisVoDCategory")));
    self->traxisDeviceIDType = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisDeviceIDType")));
    self->traxisSummaryNode = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("TraxisSummary")));


    self->chip_id = SVTESTRETAIN(QBActiveTreeFindNode(tree, SVSTRING("chip_id")));
    if (self->chip_id) {
        QBSystemInformationMenuNodeUpdate(self->chip_id, NULL, NULL);
        SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) &s_smartcard_callbacks, self, "QBSystemInformationMenu");
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBSystemInformationMenuSmartcardInfoCallback, self, &self->cmd);
    }


    svSettingsPopComponent();

    return self;
}
