/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBConfigurationMenu.h"

#include <libintl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h> // strerror
#include <errno.h>
#include <time.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBConf.h>
#include <QBTuner.h>
#include <sv_tuner.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <Utils/viewport.h>
#include <Utils/authenticators.h>
#include <init.h>
#include <ContextMenus/QBContextMenu.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/confirmationDialog.h>
#include <Windows/channelscanning.h>
#include <Windows/QBNetworkSettingsWindow.h>
#include <Windows/newtv.h>
#include <Windows/SatelliteSetup.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <Logic/MainMenuLogic.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <Logic/VideoOutputLogic.h>
#include <Services/QBProvidersControllerService.h>
#include <main.h>
#include <QBCAS.h>
#include <QBViewRightWeb.h>
#include <QBShellCmds.h>
#include "menuchoice.h"
#include <Windows/mainmenu.h>
#include <Menus/Skins/QBSkinsCarouselItemController.h>
#include <Menus/Skins/QBSkinsCarouselItemChoice.h>
#include <Utils/appType.h>
#include <Windows/QBDiagnosticsWindow.h>
#include <QBSmartCardMonitor.h>
#include <TranslationMerger.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeModelListener.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Hints/hintsGetter.h>

#include "QBConfigurationMenu.h"
#include <Logic/AudioTrackLogic.h>
#include <Logic/QBConfigurationMenuLogic.h>

#include "ConfigurationMenu/QBCallerIDMenu.h"
#include "ConfigurationMenu/QBNetworkSettings.h"
#include "ConfigurationMenu/QBRoutingSettings.h"
#include "ConfigurationMenu/QBIPSecVPNStatusMenu.h"
#include "ConfigurationMenu/QBTimeZoneMenu.h"
#include "QBClosedCaptionMenu.h"

#define CERTS_DIR "/etc/certs/private"
#define CERT_FILE CERTS_DIR "/cert.pem"

struct QBConfigurationMenuDataSourceHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvWidget menuBar;

    QBActiveTree tree;
    QBTreePathMap pathMap;
    SvGenericObject path;

    QBActiveTreeNode nodeParentalControl;
    SvGenericObject nodePathParentalControl;

    QBActiveTreeNode nodeConfirmation;
    SvGenericObject nodePathConfirmation;

    SvHashTable ifaceToNodes;

    QBActiveTreeNode dns1, dns2, modemStatus, modemMac;
    QBActiveTreeNode middlewareStatusNode;
    QBActiveTreeNode certsStatusNode;

    QBActiveTreeNode traxisStatusNode, traxisURLNode, traxisSmartcardIDNode, traxisServiceGroupNode, traxisVoDCategory, traxisDeviceIDType, traxisSummaryNode;
    QBActiveTreeNode viewRightWebStatusNode;

    QBCASCmd cmd;
    QBActiveTreeNode chip_id;

    bool selectTimeZoneNodeHidden;
    QBActiveTreeNode selectTimeZoneNode;

    CubiwareMWCustomerInfoStatus customerInfoMontitorStatus;
    TraxisWebSessionState traxisWebSessionManagerStatus;

    SvFiber rebootFiber;

    struct {
        QBViewRightWebState state;
        int errorCode;
    } viewRightStatus;

    struct {
       QBContextMenu ctx;
       SvWidget licenseLabel;
    } licenseSidemenu;

    SvGenericObject owner;
    struct QBConfigurationMenuCallbacks_s callbacks;
    HDMIService hdmiService;
};

typedef struct QBConfigurationMenuDataSourceHandler_t* QBConfigurationMenuDataSourceHandler;

struct QBConfigurationMenuWidgetHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBXMBItemConstructor itemConstructor;
    SvRID tickMarkOnRID;
    SvRID tickMarkOffRID;
    SvRID iconRID;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    SvWidget dialog;

    struct QBConfigurationMenuHandlerSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;

    QBConfigurationMenuDataSourceHandler dataSource;
};

typedef struct QBConfigurationMenuWidgetHandler_t* QBConfigurationMenuWidgetHandler;

typedef struct QBNetworkSettingsNodes_t *QBNetworkSettingsNodes;
struct QBNetworkSettingsNodes_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvString iface;
    QBActiveTreeNode ip, mask, gateway, mac, parent;
    QBActiveTree tree;
};

typedef enum QBConfigurationMenuTickState_e {
    QBConfigurationMenuTickState__Hidden,
    QBConfigurationMenuTickState__On,
    QBConfigurationMenuTickState__Off
} QBConfigurationMenuTickState;

typedef enum {
    QBConfigurationMenuCertsStatus_ok = 0,           /** No error */
    QBConfigurationMenuCertsStatus_certsInRootOK,    /** Valid certificate in rootfs */
    QBConfigurationMenuCertsStatus_certsInRootError, /** Invalid certificate in rootfs */
    QBConfigurationMenuCertsStatus_certInvalid,      /** Invalid certificate */
    QBConfigurationMenuCertsStatus_certError,        /** Certificate directory not found */
    QBConfigurationMenuCertsStatus_generalError,     /** Can't check certificate status */
} QBConfigurationMenuCertsStatus;

SvLocal bool QBConfigurationMenuIsOutputModeCommon(void)
{
    // temporary hack
    const char* lockModes = QBConfigGet("LOCKMODES");
    return lockModes && strcmp(lockModes, "YES") == 0;
}

SvLocal QBConfigurationMenuCertsStatus
QBConfigurationMenuGetCertsStatus(void)
{
    struct stat sb;
    int ret = lstat(CERTS_DIR, &sb);
    if (ret != 0) {
        SvLogError("%s() :: lstat(%s) call failed : %d / %s", __func__,
                   CERTS_DIR, ret, strerror(errno));
        return QBConfigurationMenuCertsStatus_generalError;
    } else if (!S_ISLNK(sb.st_mode)) {
        ret = stat(CERT_FILE, &sb);
        if (ret != 0) {
            SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__,
                       CERT_FILE, ret, strerror(errno));
            return QBConfigurationMenuCertsStatus_certsInRootError;
        } else {
            SvLogNotice("%s() :: Certs in root OK", __func__);
            return QBConfigurationMenuCertsStatus_certsInRootOK;
        }
    }

    ret = stat(CERTS_DIR, &sb);
    if (ret != 0) {
        SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__, CERTS_DIR, ret, strerror(errno));
        return QBConfigurationMenuCertsStatus_certError;
    }

    ret = stat(CERT_FILE, &sb);
    if (ret != 0) {
        SvLogError("%s() :: stat(%s) call failed : %d / %s", __func__, CERT_FILE, ret, strerror(errno));
        return QBConfigurationMenuCertsStatus_certInvalid;
    }

    SvLogNotice("%s() :: Certs OK", __func__);
    return QBConfigurationMenuCertsStatus_ok;
}

SvLocal SvType QBConfigurationMenuWidgetHandler_getType(void);

SvLocal QBConfigurationMenuTickState QBConfigurationMenuCheckTickState(QBConfigurationMenuWidgetHandler self, QBActiveTreeNode node)
{
    QBConfigurationMenuTickState tickState = QBConfigurationMenuTickState__Hidden;

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString optName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationName"));
    if (!optName && parent)
        optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
    if (!optName)
        return tickState;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));
    const char *value = NULL;
    value = QBConfigGet(SvStringCString(optName));
    SvLogNotice("CheckTickState [%s] + [%s] = [%s]",
            optName ? SvStringCString(optName) : "",
            optVal ? SvStringCString(optVal) : "",
            value);

    if (optVal) {
        if (optName && SvStringEqualToCString(optName, "VIDEOOUTHD")) {
            SvString nodeID = QBActiveTreeNodeGetID(node);
            if (HDMIServiceIsAutomaticModeActive(self->dataSource->hdmiService)) {
                if (SvStringEqualToCString(nodeID, "automaticFormat"))
                    tickState = QBConfigurationMenuTickState__On;
                else
                    tickState = QBConfigurationMenuTickState__Off;
            } else if (HDMIServiceIsOriginalModeActive(self->dataSource->hdmiService)) {
                if (SvStringEqualToCString(nodeID, "originalFormat"))
                    tickState = QBConfigurationMenuTickState__On;
                else
                    tickState = QBConfigurationMenuTickState__Off;
            } else {
                if (SvStringEqualToCString(nodeID, "originalFormat"))
                    tickState = QBConfigurationMenuTickState__Off;
                else if (SvStringEqualToCString(nodeID, "automaticFormat"))
                    tickState = QBConfigurationMenuTickState__Off;
                else if (value && strcasecmp(SvStringCString(optVal), value) == 0)
                    tickState = QBConfigurationMenuTickState__On;
                else
                    tickState = QBConfigurationMenuTickState__Off;
            }
        } else {
            if (value && strcasecmp(SvStringCString(optVal), value) == 0)
                tickState = QBConfigurationMenuTickState__On;
            else
                tickState = QBConfigurationMenuTickState__Off;
        }
    }

    return tickState;
}

SvLocal void QBConfigurationMenuChoosenApply(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_);

SvLocal SvWidget QBConfigurationMenuHandlerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path,
        SvApplication app, XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBConfigurationMenuWidgetHandler self = (QBConfigurationMenuWidgetHandler) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentId = parent ? QBActiveTreeNodeGetID(parent) : NULL;

    QBXMBItem item = QBXMBItemCreate();

    QBConfigurationMenuTickState tickState = QBConfigurationMenuCheckTickState(self, node);
    if (tickState != QBConfigurationMenuTickState__Hidden) {
        if (tickState == QBConfigurationMenuTickState__On)
            item->iconRID = self->tickMarkOnRID;
        else
            item->iconRID = self->tickMarkOffRID;
    }

    item->loadingRID = self->iconRID;

    if (item->iconRID == SV_RID_INVALID) {
        SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
        item->iconURI.URI = SVTESTRETAIN(iconPath);
        item->iconURI.isStatic = true;
    }

    item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    if (parentId && SvStringEqualToCString(parentId, "MLNG")) {
        SVRETAIN(item->caption);
    } else {
        const char *captionStr, *translatedCaptionStr;
        captionStr = SvStringCString(item->caption);
        translatedCaptionStr = gettext(captionStr);
        if (translatedCaptionStr != captionStr)
            item->caption = SvStringCreate(translatedCaptionStr, NULL);
        else
            SVRETAIN(item->caption);
    }

    if (node == self->dataSource->middlewareStatusNode) {
        int errorCode = 0;

        switch (self->dataSource->customerInfoMontitorStatus) {
            case CubiwareMWCustomerInfoStatus_ok:
                item->subcaption = SvStringCreate(gettext("Ok"), NULL);
                break;
            case CubiwareMWCustomerInfoStatus_error:
                if (self->appGlobals->customerInfoMonitor)
                    errorCode = CubiwareMWCustomerInfoGetErrorCode(self->appGlobals->customerInfoMonitor);
                item->subcaption = SvStringCreateWithFormat("%s #%x", gettext("Error"), errorCode);
                break;
            case CubiwareMWCustomerInfoStatus_unknown:
            default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
        }
    } else if (node == self->dataSource->certsStatusNode) {
        QBConfigurationMenuCertsStatus status = QBConfigurationMenuGetCertsStatus();

        if (status == QBConfigurationMenuCertsStatus_ok)
            item->subcaption = SvStringCreate(gettext("Ok"), NULL);
        else
            item->subcaption = SvStringCreateWithFormat("%s #%x", gettext("error"), status);
    } else if (node == self->dataSource->traxisStatusNode) {
        switch (self->dataSource->traxisWebSessionManagerStatus) {
        case TraxisWebSessionState_initial:
            item->subcaption = SvStringCreate(gettext("Not connected"), NULL);
            break;
        case TraxisWebSessionState_handshake:
            item->subcaption = SvStringCreate(gettext("Waiting for response"), NULL);
            break;
        case TraxisWebSessionState_anonymous:
            item->subcaption = SvStringCreate(gettext("Connected anonymously"), NULL);
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
    } else if (node == self->dataSource->traxisURLNode) {
        SvString traxisURLStr = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Traxis);
        if (traxisURLStr)
            item->subcaption = SVRETAIN(traxisURLStr);
        else
            item->subcaption = SVSTRING("N/A");
    } else if (node == self->dataSource->traxisSmartcardIDNode) {
        SvString smartCardID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
        item->subcaption = smartCardID ? SVRETAIN(smartCardID) : SVSTRING("N/A");
    } else if (node == self->dataSource->traxisServiceGroupNode) {
        const char *groupIDStr = QBConfigGet("EVENTIS_GROUP_ID");
        item->subcaption = groupIDStr ? SvStringCreate(groupIDStr, NULL) : SVSTRING("N/A");
    } else if (node == self->dataSource->traxisVoDCategory) {
        const char *groupIDStr = QBConfigGet("VOD_CATALOG");
        item->subcaption = groupIDStr ? SvStringCreate(groupIDStr, NULL) : SVSTRING("N/A");
    } else if (node == self->dataSource->traxisDeviceIDType) {
        const char *traxisDeviceIDTypeStr = QBConfigGet("TRAXIS_DEVICE_ID_TYPE");
        item->subcaption = traxisDeviceIDTypeStr ? SvStringCreate(traxisDeviceIDTypeStr, NULL) : SVSTRING("N/A");
    } else if (node == self->dataSource->traxisSummaryNode) {
        SvString smartCardID = QBSmartCardMonitorGetSmartCardIdRaw(self->appGlobals->smartCardMonitor);
        const char *groupIDStr = QBConfigGet("EVENTIS_GROUP_ID");
        int groupID = atoi(groupIDStr);
        item->subcaption = SvStringCreateWithFormat("%c%s:%s:%X",
                QBConfigGet("VOD_CATALOG")[0], QBConfigGet("TRAXIS_IP_HEX"), smartCardID ? SvStringCString(smartCardID) : "N/A", groupID);
    } else if (node == self->dataSource->viewRightWebStatusNode) {
        switch (self->dataSource->viewRightStatus.state) {
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
            item->subcaption = SvStringCreateWithFormat("%s #%X",gettext("Error"), self->dataSource->viewRightStatus.errorCode);
            break;
        case QBViewRightWebState_InitializationError:
            item->subcaption = SvStringCreateWithFormat("%s #%X",gettext("Initialization Error"), self->dataSource->viewRightStatus.errorCode);
            break;
        default:
            item->subcaption = SvStringCreate(gettext("Unknown"), NULL);
            break;
        }
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

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;

}

SvLocal void QBConfigurationMenuHandlerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBConfigurationMenuWidgetHandler self = (QBConfigurationMenuWidgetHandler) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBConfigurationMenuWidgetHandlerShowPCDialog(QBConfigurationMenuWidgetHandler self, QBActiveTreeNode node, SvGenericObject nodePath, QBDialogCallback callback)
{
    SvString parentalControlText = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ParentalControlText"));
    if (!parentalControlText) {
        parentalControlText = SVSTRING(gettext_noop("Please enter parental control PIN"));
    }

    svSettingsPushComponent("ParentalControl.settings");
    SVTESTRELEASE(self->dataSource->nodeParentalControl);
    SVTESTRELEASE(self->dataSource->nodePathParentalControl);
    self->dataSource->nodeParentalControl = SVRETAIN(node);
    self->dataSource->nodePathParentalControl = SVRETAIN(nodePath);

    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC_MENU"));
    self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext(SvStringCString(parentalControlText)), true, NULL, NULL);

    svSettingsPopComponent();

    QBDialogRun(self->dialog, self, callback);
}

SvLocal void QBConfigurationMenuWidgetHandlerShowConfirmationDialog(QBConfigurationMenuWidgetHandler self, QBActiveTreeNode node, SvGenericObject nodePath, QBDialogCallback callback)
{
    SvString confirmationText = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ConfirmationText"));
    if (!confirmationText) {
        confirmationText = SVSTRING(gettext_noop("Are you sure?"));
    }
    QBConfirmationDialogParams_t params = {
            .title = gettext("Confirmation Required"),
            .message = gettext(SvStringCString(confirmationText)),
            .local = true,
            .focusOK = false,
            .isCancelButtonVisible = true
    };
    self->dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    self->dataSource->nodeConfirmation = SVRETAIN(node);
    self->dataSource->nodePathConfirmation = SVRETAIN(nodePath);
    QBDialogRun(self->dialog, self, callback);
}

SvLocal void QBConfigurationMenuWidgetHandlerDestroy(void *self_)
{
    QBConfigurationMenuWidgetHandler self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = NULL;
}

SvLocal void QBConfigurationMenuDataSourceHandlerDestroy(void *self_)
{
    QBConfigurationMenuDataSourceHandler self = self_;
    SVTESTRELEASE(self->modemStatus);
    SVTESTRELEASE(self->modemMac);
    SVTESTRELEASE(self->dns1);
    SVTESTRELEASE(self->dns2);
    SVTESTRELEASE(self->chip_id);
    SVTESTRELEASE(self->ifaceToNodes);
    SVTESTRELEASE(self->middlewareStatusNode);
    SVTESTRELEASE(self->viewRightWebStatusNode);
    SVTESTRELEASE(self->traxisStatusNode);
    SVTESTRELEASE(self->traxisDeviceIDType);
    SVTESTRELEASE(self->traxisSmartcardIDNode);
    SVTESTRELEASE(self->traxisServiceGroupNode);
    SVTESTRELEASE(self->traxisVoDCategory);
    SVTESTRELEASE(self->traxisSummaryNode);
    SVTESTRELEASE(self->traxisURLNode);
    SVTESTRELEASE(self->selectTimeZoneNode);
    SVTESTRELEASE(self->nodeConfirmation);
    SVTESTRELEASE(self->nodePathConfirmation);
    SVTESTRELEASE(self->nodeParentalControl);
    SVTESTRELEASE(self->nodePathParentalControl);
    SVTESTRELEASE(self->licenseSidemenu.ctx);
    SVTESTRELEASE(self->hdmiService);
}

SvLocal void QBConfigurationMenuNodeSelectedPINCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBConfigurationMenuWidgetHandler self = self_;
    self->dialog = NULL;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SvGenericObject path = NULL;

        SvString id = QBActiveTreeNodeGetID(self->dataSource->nodeParentalControl);
        QBActiveTreeFindNodeByID(self->dataSource->tree, id, &path);
        QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->dataSource->tree, getIterator, path, 0);
        path = QBTreeIteratorGetCurrentNodePath(&it);
        if (path) {
            SvInvokeInterface(QBMenu, self->dataSource->menuBar->prv, setPosition, path, NULL);
        }

    }
    SVTESTRELEASE(self->dataSource->nodeParentalControl);
    SVTESTRELEASE(self->dataSource->nodePathParentalControl);
    self->dataSource->nodeParentalControl = NULL;
    self->dataSource->nodePathParentalControl = NULL;
}

SvLocal bool QBConfigurationMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return false;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    //This function is used to disallow entering deeper menus. If there are no deeper ones
    //just don't do anything
    QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->tree, getIterator, nodePath, 0);
    SvGenericObject tmpPath = QBTreeIteratorGetCurrentNodePath(&it);
    if (!tmpPath)
        return false;

    SvString parentalControl = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ParentalControl"));
    if (parentalControl && SvStringEqualToCString(parentalControl, "yes") && self->callbacks.authenticationRequested) {
        self->callbacks.authenticationRequested(self->owner, node, nodePath);
        return true;
    }

    return false;
}

SvLocal void QBConfigurationMenuChoosenPINCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBConfigurationMenuWidgetHandler self = self_;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button") && self->dataSource->nodeParentalControl) {
        QBConfigurationMenuChoosenApply((SvGenericObject) self->dataSource, (SvGenericObject) self->dataSource->nodeParentalControl, (SvGenericObject) self->dataSource->nodePathParentalControl);
    }
    self->dialog = NULL;
    SVTESTRELEASE(self->dataSource->nodeParentalControl);
    SVTESTRELEASE(self->dataSource->nodePathParentalControl);
    self->dataSource->nodeParentalControl = NULL;
    self->dataSource->nodePathParentalControl = NULL;
}

SvLocal void QBConfigurationMenuChoosenConfirmationCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBConfigurationMenuWidgetHandler self = self_;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button") && self->dataSource->nodeConfirmation) {
        QBConfigurationMenuChoosenApply((SvGenericObject) self->dataSource, (SvGenericObject) self->dataSource->nodeConfirmation, (SvGenericObject) self->dataSource->nodePathConfirmation);
    }
    self->dialog = NULL;
    SVTESTRELEASE(self->dataSource->nodeConfirmation);
    SVTESTRELEASE(self->dataSource->nodePathConfirmation);
    self->dataSource->nodePathConfirmation = NULL;
    self->dataSource->nodeConfirmation = NULL;
}

SvLocal void QBConfigurationMenuShowSelectTimeZoneMenu(QBConfigurationMenuDataSourceHandler self)
{
    if (self->selectTimeZoneNodeHidden && self->selectTimeZoneNode && self->tree) {
        QBActiveTreeNode parentNode = QBActiveTreeFindNode(self->tree, SVSTRING("Time Zone"));
        QBActiveTreeNodeAddSubTree(parentNode, self->selectTimeZoneNode, NULL);
        QBTreePathMap pathMap = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getPathMap);
        QBTimeZoneMenuRegister(self->menuBar, pathMap, self->appGlobals);
    }
    self->selectTimeZoneNodeHidden = false;

    const char* manualTZ = QBConfigGet("TIMEZONE");
    if (manualTZ) {
        setenv("TZ", manualTZ, 1);
        tzset();
    }
}

SvLocal void QBConfigurationMenuHideSelectTimeZoneMenu(QBConfigurationMenuDataSourceHandler self)
{
    if (!self->selectTimeZoneNodeHidden && self->appGlobals->menuTree && self->selectTimeZoneNode) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(self->selectTimeZoneNode), self->selectTimeZoneNode, NULL);
    }
    self->selectTimeZoneNodeHidden = true;
}

SvLocal void QBConfigurationMenuLicenseOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBConfigurationMenuDataSourceHandler self = self_;

    svWidgetDestroy(self->licenseSidemenu.licenseLabel);
    self->licenseSidemenu.licenseLabel = NULL;
}

SvLocal void QBConfigurationMenuLicenseOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBConfigurationMenuDataSourceHandler self = self_;

    svSettingsPushComponent("Configuration.settings");

    self->licenseSidemenu.licenseLabel = svLabelNewFromSM(self->appGlobals->res, "LicenseLabel");
    svSettingsWidgetAttach(frame, self->licenseSidemenu.licenseLabel, svWidgetGetName(self->licenseSidemenu.licenseLabel), 0);

    svSettingsPopComponent();
}

SvLocal void QBConfigurationMenuLicenseSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBConfigurationMenuDataSourceHandler self = self_;
    svWidgetSetFocus(self->licenseSidemenu.licenseLabel);
}

SvLocal void QBConfigurationMenuLicenseSideMenu(QBConfigurationMenuDataSourceHandler self)
{
    SVTESTRELEASE(self->licenseSidemenu.ctx);
    self->licenseSidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBConfigurationMenuLicenseOnHide,
        .onShow    = QBConfigurationMenuLicenseOnShow,
        .setActive = QBConfigurationMenuLicenseSetActive
    };

    QBContainerPane container = (QBContainerPane)SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(container, self->appGlobals->res, self->licenseSidemenu.ctx, 1, SVSTRING("BasicPane"), &callbacks, self);

    QBContextMenuShow(self->licenseSidemenu.ctx);
    QBContextMenuPushPane(self->licenseSidemenu.ctx, (SvGenericObject) container);
    SVRELEASE(container);
}

SvLocal void QBConfigurationMenuRefreshNode(QBConfigurationMenuDataSourceHandler self, SvString nodeID)
{
    SvObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(self->tree, nodeID, &path);
    size_t count = QBActiveTreeNodeGetChildNodesCount(node);
    QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
}

SvLocal void QBConfigurationMenuChoosenApply(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString id = QBActiveTreeNodeGetID(node);
    SvString action = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("action"));

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentId = parent ? QBActiveTreeNodeGetID(parent) : NULL;

    SvString optVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));

    if (id && SvStringEqualToCString(id, "RestoreDefaults")) {
        SvLogError("QBConfigurationMenu: RestoreDefaults : call QBInitFactoryResetAndStopApplication");
        QBInitFactoryResetAndStopApplication(self->appGlobals);
        QBWatchdogRebootAfterTimeout(self->appGlobals->watchdog, WATCHDOG_LONG_TIMEOUT_SEC, NULL);
        return;
    } else if (id && SvStringEqualToCString(id, "TRX")) {
        if (!QBAppTypeIsHybrid()) {
            QBConfigSet("HYBRIDMODE", "HYBRID");
            QBConfigSet("USE_CABLE_MODEM", "NO");
            QBConfigSave();

            QBWatchdogReboot(self->appGlobals->watchdog, NULL);
        }
    } else if (id && SvStringEqualToCString(id, "DVB")) {
        if (!QBAppTypeIsDVB()) {
            QBConfigSet("HYBRIDMODE", "DVB");
            QBConfigSet("USE_CABLE_MODEM", "NO");
            QBConfigSave();

            QBWatchdogReboot(self->appGlobals->watchdog, NULL);
        }
    } else if (id && SvStringEqualToCString(id, "RSTR")) {
        SvLogError("%s():%d User reboot : call QBInitStopApplication()", __func__, __LINE__);
        QBInitStopApplication(self->appGlobals, true, "restart from menu");
        SvLogError("QBConfigurationMenu: RestartApplication : call QBInitStopApplication");
        QBWatchdogReboot(self->appGlobals->watchdog, NULL);
        return;
    } else if (id && SvStringEqualToCString(id, "FormatHardDisk")) {
        FILE *f = fopen("/tmp/formatHardDisk", "w");
        if (f)
            fclose(f);
        f = fopen("/etc/vod/formatHardDisk", "w");
        if (f)
            fclose(f);
        f = fopen("/etc/vod/forceInternalDiskFormat", "w");
        if (f)
            fclose(f);

        SvLogError("QBConfigurationMenu: FormatHardDisk : call QBInitStopApplication");
        QBInitStopApplication(self->appGlobals, true, "format hdd");
    } else if (id && SvStringEqualToCString(id, "PushLogs")) {
        unsigned cookie = rand() % 10000;

        SvString subcaption = SvStringCreateWithFormat("Logs with cookie %i", cookie);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) subcaption);
        SVRELEASE(subcaption);

        QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
        QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));

        char *command;
        asprintf(&command, "push_logs.sh %i", cookie);
        QBShellExec(command);
        free(command);

        return;
    } else if (id && SvStringEqualToCString(id, "NetworkStatus")) {
        if (self->appGlobals->cableModem)
            QBCableModemMonitorPoll(self->appGlobals->cableModem);
        return;
    } else if(id && SvStringEqualToCString(id, "LicenseInformation")) {
        QBConfigurationMenuLicenseSideMenu(self);
        return;
    }
    QBServiceRegistry serviceRegistry = QBServiceRegistryGetInstance();
    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(serviceRegistry,
                                                                                       SVSTRING("VideoOutputLogic"));
    if (parentId && optVal) {
        const char* qlang = QBConfigGet("LANG");
        if (SvStringEqualToCString(parentId, "MLNG") &&
            qlang && !SvStringEqualToCString(optVal, qlang)) {
            QBInitLogicSetLocale(self->appGlobals->initLogic, optVal);
            QBApplicationControllerReinitializeWindows(self->appGlobals->controller, NULL);
        } else if (SvStringEqualToCString(parentId, "AspectRatioSD") || SvStringEqualToCString(parentId, "AspectRatioHD")) {
            const char *outputType = SvStringCString(parentId) + SvStringLength(parentId) - 2;
            QBAspectRatioAgent aspectRatioAgent = (QBAspectRatioAgent) QBServiceRegistryGetService(serviceRegistry,
                                                                                                   SVSTRING("QBAspectRatioAgent"));
            QBAspectRatioAgentSetAspectRatio(aspectRatioAgent, outputType, SvStringCString(optVal));
        } else if (SvStringEqualToCString(parentId, "VideoContentModeSD")) {
            VideoOutputLogicSetVideoContentMode(videoOutputLogic, "SD", SvStringCString(optVal));
            if (QBConfigurationMenuIsOutputModeCommon()) {
                QBConfigSet("VIDEO_CONTENT_MODE_HD", SvStringCString(optVal));
                VideoOutputLogicSetVideoContentMode(videoOutputLogic, "HD", SvStringCString(optVal));
            }
        } else if (SvStringEqualToCString(parentId, "VideoContentModeHD")) {
            VideoOutputLogicSetVideoContentMode(videoOutputLogic, "HD", SvStringCString(optVal));
            if (QBConfigurationMenuIsOutputModeCommon()) {
                QBConfigSet("VIDEO_CONTENT_MODE_SD", SvStringCString(optVal));
                VideoOutputLogicSetVideoContentMode(videoOutputLogic, "SD", SvStringCString(optVal));
            }
        }
    }

    if (action) {
        if (QBConfigurationMenuLogicActionTrigger(self->appGlobals->configurationMenuLogic, action, node)) {
        } else if (SvStringEqualToCString(action, "Diagnostics")) {
            SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));
            if (type) {
                QBWindowContext ctx = QBDiagnosticsWindowContextCreate(self->appGlobals, type);
                if (ctx) {
                    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                    SVRELEASE(ctx);
                }
            }
        } else if (SvStringEqualToCString(action, "Channel scanning")) {
            SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));
            QBWindowContext ctx = QBChannelScanningContextCreate(self->appGlobals, type);

            SvString scanType = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("scanType"));
            if (scanType) {
                QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                              SVSTRING("QBChannelScanningConfManager"));
                QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, scanType);
                if (conf)
                    QBChannelScanningContextLoadConf(ctx, conf);
            }

            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
            return;
        } else if (SvStringEqualToCString(action, "Satellite setup")) {
            QBWindowContext ctx = SatelliteSetupContextCreate(self->appGlobals->res);
            if (ctx) {
                QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
                SVRELEASE(ctx);
            } else {
                SvLogWarning("CubiTV: satellite support is non-functional!");
            }
            return;
        } else if (SvStringEqualToCString(action, "Set TV System")) {
            QBConfigSet("TVSYSTEM", SvStringCString(optVal));
        } else if (SvStringEqualToCString(action, "Enable automatic frame rate selection")) {
            VideoOutputLogicEnableAutomaticFrameRateSelection(videoOutputLogic);
            HDMIServiceEnableAutomaticFrameRateSelection(self->hdmiService);
        } else if (SvStringEqualToCString(action, "Disable automatic frame rate selection")) {
            VideoOutputLogicDisableAutomaticFrameRateSelection(videoOutputLogic);
            HDMIServiceDisableAutomaticFrameRateSelection(self->hdmiService);
        } else if (SvStringEqualToCString(action, "Set HD output mode")) {
            QBOutputStandard newMode = QBOutputStandard_current;
            QBTVSystem systemType = QBPlatformGetTVSystemByName(QBConfigGet("TVSYSTEM"));
            if (systemType == QBTVSystem_unknown) {
                SvLogWarning("CubiTV: invalid TVSYSTEM configuration value. Using PAL");
                systemType = QBTVSystem_PAL;
            }

            newMode = QBPlatformFindModeByName(SvStringCString(optVal), systemType);
            if (newMode == QBOutputStandard_none) {
                if (SvStringEqualToCString(optVal, "480P"))
                    newMode = QBOutputStandard_480p59;
                else if (SvStringEqualToCString(optVal, "576P"))
                    newMode = QBOutputStandard_576p50;
                else if (SvStringEqualToCString(optVal, "720P"))
                    newMode = QBOutputStandard_720p50;
                else if (SvStringEqualToCString(optVal, "1080I"))
                    newMode = QBOutputStandard_1080i50;
                else if (SvStringEqualToCString(optVal, "1080P"))
                    newMode = QBOutputStandard_1080p50;
                else if (SvStringEqualToCString(optVal, "1080P25"))
                    newMode = QBOutputStandard_1080p25;
                else if (SvStringEqualToCString(optVal, "2160P24"))
                    newMode = QBOutputStandard_2160p24;
                else if (SvStringEqualToCString(optVal, "2160P25"))
                    newMode = QBOutputStandard_2160p25;
                else if (SvStringEqualToCString(optVal, "2160P"))
                    newMode = QBOutputStandard_2160p30;
                else if (SvStringEqualToCString(optVal, "2160P30"))
                    newMode = QBOutputStandard_2160p30;
                else if (SvStringEqualToCString(optVal, "4096x2160P"))
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

            QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
            QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));
        } else if (SvStringEqualToCString(action, "Enable HDMI original mode")) {
            QBConfigSet("HDOUTAUTOMATICMODE", "disabled");
            QBConfigSet("HDOUTORIGINALMODE", "enabled");
            QBConfigSave();
            HDMIServiceEnableOriginalMode(self->hdmiService);
            QBOutputStandard selectedMode = HDMIServiceGetSelectedVideoMode(self->hdmiService);
            const char *modeStr = QBPlatformGetModeName(selectedMode);
            if (modeStr)
                optVal = SVAUTORELEASE(SvStringCreate(modeStr, NULL));

            QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
            QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));
        } else if (SvStringEqualToCString(action, "Enable HDMI automatic mode")) {
            QBConfigSet("HDOUTORIGINALMODE", "disabled");
            QBConfigSet("HDOUTAUTOMATICMODE", "enabled");
            QBConfigSave();
            HDMIServiceEnableAutomaticMode(self->hdmiService);
            QBOutputStandard selectedMode = HDMIServiceGetSelectedVideoMode(self->hdmiService);
            const char *modeStr = QBPlatformGetModeName(selectedMode);
            if (modeStr)
                optVal = SVAUTORELEASE(SvStringCreate(modeStr, NULL));

            QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
            QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));
        } else if (SvStringEqualToCString(action, "Setup S/PDIF audio delay")) {
            QBTVLogicEnableAudioDelaySetupMode(self->appGlobals->tvLogic);
            QBApplicationControllerPushContext(self->appGlobals->controller, self->appGlobals->newTV);
        } else if (SvStringEqualToCString(action, "Set AV signal type") || SvStringEqualToCString(action, "Set SCART signal type")) {
            VideoOutputLogicSetSCARTSignalType(videoOutputLogic, SvStringCString(optVal));
        } else if (SvStringEqualToCString(action, "Set RF Modulator")) {
            VideoOutputLogicSetRFModulator(videoOutputLogic, SvStringCString(optVal));
        } else if (SvStringEqualToCString(action, "Network Setup")) {
            QBWindowContext ctx = QBNetworkSettingsContextCreate(self->appGlobals->res,
                                                                 self->appGlobals->scheduler,
                                                                 self->appGlobals->initLogic,
                                                                 self->appGlobals->controller,
                                                                 self->appGlobals->textRenderer,
                                                                 self->appGlobals->networkMonitor,
                                                                 false);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
            return;
        }
    }

    if (parentId) {
        SvString optName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
        if (optName && optVal) {
            QBConfigSet(SvStringCString(optName), SvStringCString(optVal));
            // QBConfigurationMenuConfigChanged is called by QBConfigListener::changed callback
            QBConfigSave();
        }
        if (SvStringEqualToCString(parentId, "SLNG") || SvStringEqualToCString(parentId, "SSNG") || SvStringEqualToCString(parentId, "MLNG")) {
            SvTimeRange timeRange;
            time_t t = SvTimeNow();
            SvTimeRangeInit(&timeRange, t - 100, t + 100);

            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
            int count = SvInvokeInterface(SvEPGChannelView, playlist, getCount);
            SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                             SVSTRING("SvEPGManager"));
            int idx = 0;
            for (idx = 0; idx < count; ++idx) {
                SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, idx);
                SvEPGManagerPropagateEventsChange(epgManager, SvTVChannelGetID(channel), &timeRange, NULL);
            }
        }
    }

    if (action) {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        AudioTrackLogic audioTrackLogic =
            (AudioTrackLogic) QBServiceRegistryGetService(registry, SVSTRING("AudioTrackLogic"));
        if (SvStringEqualToCString(action, "Toggle multichannel mode")) {
            bool isMultiChannelMode = false;
            if (optVal && SvStringEqualToCString(optVal, "YES"))
                isMultiChannelMode = true;
            AudioOutputLogicSetMultiChannelMode((AudioOutputLogic) QBServiceRegistryGetService(registry, SVSTRING("AudioOutputLogic")),
                                                isMultiChannelMode);
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
        } else if (SvStringEqualToCString(action, "Change audio track type")) {
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
        } else if (SvStringEqualToCString(action, "Change audio language")) {
            AudioTrackLogicSetupAudioTrack(audioTrackLogic);
        } else if (SvStringEqualToCString(action, "Check software update")) {
            QBUpgradeLogicCheck(self->appGlobals->upgradeLogic);
        } else if (SvStringEqualToCString(action, "Configure upgrade logic")) {
            QBUpgradeLogicReconfigure(self->appGlobals->upgradeLogic);
        } else if (SvStringEqualToCString(action, "Change time zone") && optVal) {
            setenv("TZ", SvStringCString(optVal), 1);
            tzset();
        } else if (SvStringEqualToCString(action, "Reboot")) {
            QBWatchdogReboot(self->appGlobals->watchdog, NULL);
        } else if (self->callbacks.handleAction) {
            self->callbacks.handleAction(self->owner, action);
        }
    }

    SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));

    if (type && SvStringEqualToCString(type, "XHTMLWindow")) {
        SvString urlVal = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("url"));
        if (self->appGlobals->xhtmlWindowsService && urlVal) {
            if (!id) {
                SvLogWarning("%s: Can't handle chosen product (type: %s). window id doesn't exist. ", __func__, SvStringCString(type));
                return;
            }
            if (SvStringLength(id) == 0) {
                SvLogWarning("%s: Can't handle chosen product (type: %s). Incorrect window id. ", __func__, SvStringCString(type));
                return;
            }

            SvURL url = SvURLCreateWithString(urlVal, NULL);
            if (!url) {
                SvLogWarning("%s: Can't handle chosen product (type: %s)", __func__, SvStringCString(type));
                return;
            }
            QBXHTMLWindowsServiceDisplay(self->appGlobals->xhtmlWindowsService, url, id);
            SVRELEASE(url);
        }
    }

    if (self->middlewareStatusNode && QBActiveTreeNodeGetParentNode(self->middlewareStatusNode) == (QBActiveTreeNode) node_) {
        if (self->appGlobals->customerInfoMonitor)
            CubiwareMWCustomerInfoRefresh(self->appGlobals->customerInfoMonitor);
    }
}

SvLocal void QBConfigurationMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;
    SvWidget menuBar = self->menuBar;
    QBConfigurationMenuWidgetHandler widgetHandler = (QBConfigurationMenuWidgetHandler) SvInvokeInterface(QBMenu, menuBar->prv, getItemControllerForPath, self->path, NULL);

    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    SvString parentalControl = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ParentalControl"));
    SvString confirmation = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("Confirmation"));

    if (parentalControl && SvStringEqualToCString(parentalControl, "yes")) {
        QBConfigurationMenuWidgetHandlerShowPCDialog(widgetHandler, node, nodePath_, QBConfigurationMenuChoosenPINCallback);
    } else if (confirmation && SvStringEqualToCString(confirmation, "yes")) {
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
        SvString configurationName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
        if (configurationName) {
            SvString value = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("configurationValue"));
            const char* baseValue = QBConfigGet(SvStringCString(configurationName));
            if (baseValue && !SvStringEqualToCString(value, baseValue)) {
                QBConfigurationMenuWidgetHandlerShowConfirmationDialog(widgetHandler, node, nodePath_, QBConfigurationMenuChoosenConfirmationCallback);
            }
        } else {
            QBConfigurationMenuWidgetHandlerShowConfirmationDialog(widgetHandler, node, nodePath_, QBConfigurationMenuChoosenConfirmationCallback);
        }
    } else {
        QBConfigurationMenuChoosenApply(self_, node_, nodePath_);
    }
}

SvLocal void QBConfigurationMenuFillNetworkInterface(QBConfigurationMenuDataSourceHandler self, QBActiveTreeNode mainNode, SvString iface);

SvLocal void QBNetworkSettingsNodeUpdate(QBActiveTreeNode node, const char*, SvGenericObject value);

SvLocal void QBConfigurationMenuInterfaceChanged(SvGenericObject self_, SvString iface, uint64_t mask)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;
    AppGlobals appGlobals = self->appGlobals;

    if (mask & (1ll << QBNetAttr_dns1)) {
        SvGenericObject dns1 = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_dns1, NULL);
        QBNetworkSettingsNodeUpdate(self->dns1, NULL, dns1);
    }
    if (mask & (1ll << QBNetAttr_dns2)) {
        SvGenericObject dns2 = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_dns2, NULL);
        QBNetworkSettingsNodeUpdate(self->dns2, NULL, dns2);
    }
    if ((mask & (1ll << QBNetAttr_name))) {
        SvGenericObject path = NULL;
        SvString ifaceID = SvStringCreateWithFormat("_iface:%s_", SvStringCString(iface));
        QBActiveTreeNode ifaceNode = QBActiveTreeFindNodeByID(self->tree, ifaceID, &path);
        if (path) {
            QBActiveTreeRemoveSubTree(self->tree, ifaceNode, NULL);
            QBNetworkSettingsNodes nodes = (QBNetworkSettingsNodes) SvHashTableFind(self->ifaceToNodes, (SvGenericObject) iface);
            QBNetManagerRemoveListener(appGlobals->net_manager, (SvGenericObject) nodes, iface, NULL);
            QBNetManagerRemoveListener(appGlobals->net_manager, (SvGenericObject) self, iface, NULL);
            SvHashTableRemove(self->ifaceToNodes, (SvGenericObject) iface);
        } else {
            QBActiveTreeNode mainNode = QBActiveTreeFindNodeByID(self->tree, SVSTRING("NetworkStatus"), &path);
            QBConfigurationMenuFillNetworkInterface(self, mainNode, iface);
            QBNetManagerAddListener(appGlobals->net_manager, (SvGenericObject) self, iface, NULL);
        }
        SVRELEASE(ifaceID);
    }
}

SvLocal void QBConfigurationMenuCableModemChanged(SvGenericObject self_, uint64_t mask)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    if (mask & QBCableModemAttr_Status) {
        SvGenericObject status = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Status);
        QBNetworkSettingsNodeUpdate(self->modemStatus, NULL, status);
        QBConfigurationMenuRefreshNode(self, SVSTRING("NetworkStatus"));
    }
    if (mask & QBCableModemAttr_Mac) {
        SvGenericObject status = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Mac);
        QBNetworkSettingsNodeUpdate(self->modemMac, NULL, status);
        QBConfigurationMenuRefreshNode(self, SVSTRING("NetworkStatus"));
    }

}

SvLocal void
QBConfigurationMenuSetMiddlewareStatus(QBConfigurationMenuDataSourceHandler self, CubiwareMWCustomerInfoStatus status)
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
QBConfigurationMenuSetTraxisWebSessionManagerStatus(QBConfigurationMenuDataSourceHandler self, TraxisWebSessionState state)
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
QBConfigurationMenuSetViewRightWebStatus(QBConfigurationMenuDataSourceHandler self, QBViewRightWebState state, int errorCode)
{
    QBActiveTreeNode node = self->viewRightWebStatusNode;

    if (!node) {
        SvLogError("%s: no node", __func__);
        return;
    }

    self->viewRightStatus.state = state;
    self->viewRightStatus.errorCode = errorCode;
    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void
QBConfigurationMenuCustomerInfoMonitorStatusChanged(SvGenericObject self_)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(self->appGlobals->customerInfoMonitor);
    QBConfigurationMenuSetMiddlewareStatus(self, status);
}

SvLocal void
QBConfigurationMenuCustomerInfoMonitorInfoChanged(SvGenericObject self_, SvString customerId, SvHashTable customerInfo)
{
}

SvLocal void
QBConfigurationMenuTraxisWebSessionManagerStateChanged(SvGenericObject self_, TraxisWebSessionState state)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;
    QBConfigurationMenuSetTraxisWebSessionManagerStatus(self, state);
}

SvLocal void
QBConfigurationMenuTraxisWebSessionManagerLanguageChanged(SvGenericObject self_, SvString language)
{

}

SvLocal void
QBConfigurationMenuTraxisWebSessionManagerAuthLevelChaned(SvGenericObject self_, TraxisWebAuthLevel authLevel)
{

}

SvLocal void
QBConfigurationMenuViewRightWebStateChanged(SvGenericObject self_, QBViewRightWebState state, int errorCode)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;
    QBConfigurationMenuSetViewRightWebStatus(self, state, errorCode);
}

SvLocal SvString QBConfigurationMenuGetHintsForPath(SvObject self_, SvObject path)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    QBActiveTreeNode currentNode = (QBActiveTreeNode) SvInvokeInterface(QBTreeModel, self->tree, getNode, path);
    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(currentNode);

    if (parent) {
        SvString parentID = (SvString) QBActiveTreeNodeGetID(parent);
        if (parentID && (SvStringEqualToCString(parentID, "VideoContentModeHD") ||
                         SvStringEqualToCString(parentID, "VideoContentModeSD")) &&
                QBConfigurationMenuIsOutputModeCommon()) {
            return SVSTRING("commonOutputMode_hint");
        }
    }

    return NULL;
}

SvLocal void QBConfigurationMenuConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    SvObject currentPath;
    {
        SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
        currentPath = SvInvokeInterface(QBMenu, menuBar->prv, getPosition);
    }
    QBActiveTreeNode currentNode;
    {
        SvObject currentNode_ = SvInvokeInterface(QBTreeModel, self->tree, getNode, currentPath);
        if (!SvObjectIsInstanceOf(currentNode_, QBActiveTreeNode_getType()))
            return;
        currentNode = (QBActiveTreeNode) currentNode_;
    }
    QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(currentNode);
    if (!parentNode)
        return;
    {
        SvString configurationName = (SvString) QBActiveTreeNodeGetAttribute(parentNode, SVSTRING("configurationName"));
        if (!configurationName || !SvStringEqualToCString(configurationName, key))
            return;
    }

    QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));
}

SvLocal SvType QBConfigurationMenuDataSourceHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConfigurationMenuDataSourceHandlerDestroy
    };
    static SvType type = NULL;

    static const struct QBMenuChoice_t menuMethods = {
            .choosen = QBConfigurationMenuChoosen
    };

    static const struct QBNetListener_t netMethods = {
            .netAttributeChanged = QBConfigurationMenuInterfaceChanged,
    };

    static const struct QBCableModemListener_t cableModemMethods = {
            .cableModemAttributeChanged = QBConfigurationMenuCableModemChanged,
    };

    static const struct TraxisWebSessionStateListener_ traxisStateListenerMethods = {
            .stateChanged = QBConfigurationMenuTraxisWebSessionManagerStateChanged,
            .languageChanged = QBConfigurationMenuTraxisWebSessionManagerLanguageChanged,
            .authLevelChanged = QBConfigurationMenuTraxisWebSessionManagerAuthLevelChaned,
    };

    static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
        .customerInfoChanged = QBConfigurationMenuCustomerInfoMonitorInfoChanged,
        .statusChanged = QBConfigurationMenuCustomerInfoMonitorStatusChanged
    };

    static const struct QBMenuEventHandler_ selectMethods = {
            .nodeSelected = QBConfigurationMenuNodeSelected,
    };

    static const struct QBViewRightWebListener_s viewRightWebListenerMethods = {
        .stateChanged = QBConfigurationMenuViewRightWebStateChanged
    };

    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBConfigurationMenuGetHintsForPath
    };

    static const struct QBConfigListener_t configListenerMethods = {
        .changed = QBConfigurationMenuConfigChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConfigurationMenuDataSourceHandler",
                            sizeof(struct QBConfigurationMenuDataSourceHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                QBNetListener_getInterface(), &netMethods,
                QBCableModemListener_getInterface(), &cableModemMethods,
                CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                TraxisWebSessionStateListener_getInterface(), &traxisStateListenerMethods,
                QBMenuEventHandler_getInterface(), &selectMethods,
                QBViewRightWebListener_getInterface(), &viewRightWebListenerMethods,
                QBHintsGetter_getInterface(), &hintsMethods,
                QBConfigListener_getInterface(), &configListenerMethods,
                NULL);
    }

    return type;
}

SvLocal SvType QBConfigurationMenuWidgetHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConfigurationMenuWidgetHandlerDestroy
    };
    static SvType type = NULL;

    static const struct XMBItemController_t methods = {
            .createItem = QBConfigurationMenuHandlerCreateItem,
            .setItemState = QBConfigurationMenuHandlerSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConfigurationMenuWidgetHandler",
                            sizeof(struct QBConfigurationMenuWidgetHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                NULL);
    }

    return type;
}

SvLocal void QBNetworkSettingsNodesDestroy(void *self_)
{
    QBNetworkSettingsNodes self = (QBNetworkSettingsNodes) self_;
    SVRELEASE(self->ip);
    SVRELEASE(self->mask);
    SVRELEASE(self->gateway);
    SVRELEASE(self->mac);
    SVRELEASE(self->iface);
}

SvLocal void QBNetworkSettingsNodeUpdate(QBActiveTreeNode node, const char* captionStr, SvGenericObject value)
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
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) caption);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) stringVal);

    SVRELEASE(stringVal);
    SVTESTRELEASE(caption);
}

SvLocal void QBNetworkSettingsNodesFill(QBNetworkSettingsNodes self, SvString iface, uint64_t update)
{
    AppGlobals appGlobals = self->appGlobals;

    if (update & (1ll << QBNetAttr_IPv4_address)) {
        SvGenericObject ip = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_address, NULL);
        QBNetworkSettingsNodeUpdate(self->ip, NULL, ip);
    }
    if (update & (1ll << QBNetAttr_IPv4_netMask)) {
        SvGenericObject mask = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_netMask, NULL);
        QBNetworkSettingsNodeUpdate(self->mask, NULL, mask);
    }

    if (update & (1ll << QBNetAttr_IPv4_gateway)) {
        SvGenericObject gateway = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_IPv4_gateway, NULL);
        QBNetworkSettingsNodeUpdate(self->gateway, NULL, gateway);
    }

    if (update & (1ll << QBNetAttr_hwAddr)) {
        SvGenericObject mac = QBNetManagerGetAttribute(appGlobals->net_manager, iface, QBNetAttr_hwAddr, NULL);
        QBNetworkSettingsNodeUpdate(self->mac, NULL, mac);
    }
}

SvLocal void QBNetworkSettingsNodesChanged(SvGenericObject self_, SvString iface, uint64_t mask)
{
    QBNetworkSettingsNodes self = (QBNetworkSettingsNodes) self_;
    QBNetworkSettingsNodesFill(self, self->iface, mask);

    SvGenericObject path = NULL;
    QBActiveTreeFindNodeByID(self->tree, QBActiveTreeNodeGetID(self->parent), &path);
    if (path) {
        size_t count = QBActiveTreeNodeGetChildNodesCount(self->parent);
        QBActiveTreePropagateNodesChange(self->tree, path, 0, count, NULL);
    }
}

SvLocal SvType QBNetworkSettingsNodes_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNetworkSettingsNodesDestroy
    };
    static SvType type = NULL;
    static const struct QBNetListener_t methods = {
            .netAttributeChanged = QBNetworkSettingsNodesChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNetworkSettingsNodes",
                            sizeof(struct QBNetworkSettingsNodes_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBNetListener_getInterface(), &methods,
                NULL);
    }

    return type;
}

SvLocal void QBConfigurationMenuFillNetworkInterface(QBConfigurationMenuDataSourceHandler self, QBActiveTreeNode mainNode, SvString iface)
{
    AppGlobals appGlobals = self->appGlobals;

    //Find proper index for that interface
    SvIterator it = SvHashTableKeysIterator(self->ifaceToNodes);
    int idx = 0;
    SvString itiface;
    while ((itiface = (SvString) SvIteratorGetNext(&it))) {
        if (strcmp(SvStringCString(itiface), SvStringCString(iface)) > 0)
            idx++;
    }

    SvString id = SvStringCreateWithFormat("_iface:%s_", SvStringCString(iface));
    QBActiveTreeNode parent_node = QBActiveTreeNodeCreate(id, NULL, NULL);
    QBActiveTreeNodeSetAttribute(parent_node, SVSTRING("caption"), (SvGenericObject) iface);
    QBActiveTreeInsertSubTree(self->tree, mainNode, parent_node, idx, NULL);
    SVRELEASE(id);

    QBNetworkSettingsNodes nodes = (QBNetworkSettingsNodes) SvTypeAllocateInstance(
            QBNetworkSettingsNodes_getType(), NULL);

    nodes->appGlobals = appGlobals;
    nodes->ip = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkSettingsNodeUpdate(nodes->ip, gettext_noop("IP"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->ip, NULL);

    nodes->mask = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkSettingsNodeUpdate(nodes->mask, gettext_noop("Network mask"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->mask, NULL);

    nodes->gateway = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkSettingsNodeUpdate(nodes->gateway, gettext_noop("Gateway"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->gateway, NULL);

    nodes->mac = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBNetworkSettingsNodeUpdate(nodes->mac, gettext_noop("MAC address"), NULL);
    QBActiveTreeAddSubTree(self->tree, parent_node, nodes->mac, NULL);

    nodes->parent = mainNode;
    nodes->iface = SVRETAIN(iface);
    nodes->tree = self->tree;
    QBNetworkSettingsNodesChanged((SvGenericObject) nodes, iface, -1);
    QBNetManagerAddListener(appGlobals->net_manager, (SvGenericObject) nodes, iface, NULL);

    SvHashTableInsert(self->ifaceToNodes, (SvGenericObject) iface, (SvGenericObject) nodes);

    SVRELEASE(nodes);
    SVRELEASE(parent_node);
}

SvLocal void QBConfigurationMenuFillNetworkSettings(QBConfigurationMenuDataSourceHandler self, AppGlobals appGlobals, QBActiveTreeNode mainNode)
{
    self->ifaceToNodes = SvHashTableCreate(20, NULL);

    SvArray iface_tab = QBNetManagerCreateInterfacesList(appGlobals->net_manager, NULL);

    QBActiveTreeNode dns1 = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNode dns2 = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNode modemStatus = NULL;
    QBActiveTreeNode modemMac = NULL;

    if (iface_tab) {
        SvIterator it = SvArrayIterator(iface_tab);
        SvString iface;
        while ((iface = (SvString) SvIteratorGetNext(&it))) {
            QBConfigurationMenuFillNetworkInterface(self, mainNode, iface);
        }
    }

    SvGenericObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(self->tree, SVSTRING("CableModem"), &path);
    if (path && self->appGlobals->cableModem) {
        modemStatus = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        modemMac = QBActiveTreeNodeCreate(NULL, NULL, NULL);

        SvGenericObject cableModemStatus = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Status);
        QBNetworkSettingsNodeUpdate(modemStatus, gettext_noop("Cable modem status"), cableModemStatus);
        SvGenericObject cableModemMac = QBCableModemMonitorGetAttribute(self->appGlobals->cableModem, QBCableModemAttr_Mac);
        QBNetworkSettingsNodeUpdate(modemMac, gettext_noop("HFC MAC ID"), cableModemMac);

        QBActiveTreeAddSubTree(self->tree, node, modemStatus, NULL);
        QBActiveTreeAddSubTree(self->tree, node, modemMac, NULL);
    }

    QBNetworkSettingsNodeUpdate(dns1, gettext_noop("DNS1"), NULL);
    QBNetworkSettingsNodeUpdate(dns2, gettext_noop("DNS2"), NULL);

    QBActiveTreeAddSubTree(self->tree, mainNode, dns1, NULL);
    QBActiveTreeAddSubTree(self->tree, mainNode, dns2, NULL);
    self->dns1 = dns1;
    self->dns2 = dns2;
    self->modemStatus = modemStatus;
    self->modemMac = modemMac;
    SVTESTRELEASE(iface_tab);

    QBConfigurationMenuInterfaceChanged((SvGenericObject) self, NULL, (1ll << QBNetAttr_dns1) | (1ll << QBNetAttr_dns2));
}

SvLocal void QBConfigurationMenuFillTunerSettings(QBActiveTreeNode parent_node, QBActiveTree menuTree)
{
#if 0
    struct QBTunerProperties prop;
    QBTunerGetProperties(&prop);

    QBActiveTreeNode node = NULL;
    SvString caption = NULL;

    caption = SvStringCreateWithFormat("Software: %s", prop.sw_version);
    node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject)caption);
    QBActiveTreeAddSubTree(menuTree, parent_node, node, NULL);
    SVRELEASE(node);
    SVRELEASE(caption);
#endif
}

#define SvObj  SvGenericObject
#define TEXT_STR(_str_)  SVSTRING(gettext_noop(_str_))

SvLocal QBActiveTreeNode QBConfigurationMenu_AddSimpleNode(QBActiveTreeNode parentNode,
        SvString caption, SvString subcaption)
{
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    if (caption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObj) caption);
    if (subcaption)
        QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObj) subcaption);

    QBActiveTreeNodeAddSubTree(parentNode, node, NULL);
    SVRELEASE(node);
    return node;
}

SvLocal void QBConfigurationMenu_QBNATMonitorChanged(void *self_, QBNATMonitor monitor)
{
    QBConfigurationMenuDataSourceHandler self = self_;

    SvGenericObject mainPath = NULL;
    QBActiveTreeNode mainNode = QBActiveTreeFindNodeByID(self->tree, TEXT_STR("NATStatus"), &mainPath);
    if (!mainPath)
        return;

    /// remove all nodes except the first (iface)
    size_t cnt = QBActiveTreeNodeGetChildNodesCount(mainNode);
    for (; cnt > 1; cnt--) {
        QBActiveTreeNode node = QBActiveTreeNodeGetChildByIndex(mainNode, 1);
        QBActiveTreeNodeRemoveChildNode(mainNode, node);
    };

    /// get current status
    QBNATMonitorStatus status;
    QBNATMonitorGetStatus(self->appGlobals->natMonitor, &status);

    /// fill-out the nodes:

    /// iface : this node should be preserved at all times
    SvString iface = status.localIface;
    if (!iface)
        iface = SVSTRING("---");
    QBActiveTreeNode ifaceNode = NULL;
    if (cnt > 0) {
        ifaceNode = QBActiveTreeNodeGetChildByIndex(mainNode, 0);
    } else {
        ifaceNode = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        QBActiveTreeNodeSetAttribute(ifaceNode, SVSTRING("caption"), (SvObj) TEXT_STR("Default interface"));
        QBActiveTreeAddSubTree(self->tree, mainNode, ifaceNode, NULL);
        SVRELEASE(ifaceNode);
    };
    QBActiveTreeNodeSetAttribute(ifaceNode, SVSTRING("subcaption"), (SvObj) iface);

    /// ip
    SvString ip = status.localIP;
    if (!ip)
        ip = SVSTRING("---");
    QBConfigurationMenu_AddSimpleNode(mainNode, SVSTRING(gettext_noop("IP")), ip);

    /// udp status (node)
    QBActiveTreeNode udpNode = QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("UDP receiving"), NULL);

    bool udpReceivingPossible = false;

    /// stun stuff
    if (status.ipIsPublic)
    {
        udpReceivingPossible = true;
    }
    else if (status.mappedIP)
    {

        QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("Public IP"), status.mappedIP);
        QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("UDP port behind NAT"),
                status.portPreserved ? TEXT_STR("Preserved") : TEXT_STR("Randomized"));
        if (status.ipRestricted)
            QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("STUN IP restricted"), NULL);
        if (status.portRestricted)
            QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("STUN Port Restricted"), NULL);

        udpReceivingPossible = status.portPreserved || (!status.portRestricted && !status.ipRestricted);
    }
    else
    {
        QBConfigurationMenu_AddSimpleNode(mainNode, TEXT_STR("No STUN Server Connection"), NULL);
    };

    /// udp status (subcaption)
    QBActiveTreeNodeSetAttribute(udpNode, SVSTRING("subcaption"),
                                 (SvObj)(udpReceivingPossible ? TEXT_STR("OK") : TEXT_STR("Not Possible")));

    /// propagate changes in gui
    QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(mainNode));
}

static void callback_info(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBConfigurationMenuDataSourceHandler self = self_;
    self->cmd = NULL;

    if (status != 0) {
        SvLogNotice("%s: Unable to get chip id", __func__);
        return;
    }

    QBNetworkSettingsNodeUpdate(self->chip_id, NULL, (SvGenericObject)info->chip_id);

    QBActiveTreeNode node = QBActiveTreeFindNode(self->tree, SVSTRING("chip_id"));
    QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
    QBConfigurationMenuRefreshNode(self, QBActiveTreeNodeGetID(parentNode));
}

SvLocal void QBConfigurationMenu_cb_smartcard_state(void* self_, QBCASSmartcardState state)
{
    QBConfigurationMenuDataSourceHandler self = (QBConfigurationMenuDataSourceHandler) self_;

    if (state == QBCASSmartcardState_correct || state == QBCASSmartcardState_removed) {
        if (self->cmd) {
            SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
            self->cmd = NULL;
        }
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &callback_info, self, &self->cmd);
    }
}

static const struct QBCASCallbacks_s s_smartcard_callbacks = {
  .smartcard_state  = &QBConfigurationMenu_cb_smartcard_state,
};

SvGenericObject QBConfigurationMenuDataSourceHandlerCreate(SvWidget menuBar, AppGlobals appGlobals, QBActiveTree menuTree, QBTreePathMap pathMap)
{
    QBConfigurationMenuDataSourceHandler handler = (QBConfigurationMenuDataSourceHandler)
            SvTypeAllocateInstance(QBConfigurationMenuDataSourceHandler_getType(), NULL);

    handler->appGlobals = appGlobals;
    handler->tree = menuTree;
    handler->pathMap = pathMap;
    handler->owner = NULL;
    handler->callbacks.authenticationRequested = NULL;
    handler->callbacks.handleAction = NULL;
    QBActiveTreeNode node = NULL;
    node = QBActiveTreeFindNode(menuTree, SVSTRING("TunerStatus"));
    if (node)
        QBConfigurationMenuFillTunerSettings(node, menuTree);

    QBUpgradeService upgradeService = QBUpgradeServiceCreate(NULL);
    if (upgradeService) {
        node = QBActiveTreeFindNode(menuTree, SVSTRING("FWVR"));
        if (node) {
            SvString version = QBUpgradeServiceGetLocalInformativeVersion(upgradeService);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) version);
        }
        node = QBActiveTreeFindNode(menuTree, SVSTRING("PLTF"));
        if (node) {
            SvString platform = QBUpgradeServiceGetBoardName(upgradeService);
            QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) platform);
        }
        SVRELEASE(upgradeService);
    }

    node = QBActiveTreeFindNode(menuTree, SVSTRING("NetworkStatus"));
    if (node) {
        QBConfigurationMenuFillNetworkSettings(handler, appGlobals, node);

        if (appGlobals->net_manager)
            QBNetManagerAddListener(appGlobals->net_manager, (SvGenericObject) handler, NULL, NULL);
        if (appGlobals->cableModem)
            QBCableModemMonitorAddListener(appGlobals->cableModem, (SvGenericObject) handler);
    }

    node = QBActiveTreeFindNode(menuTree, SVSTRING("NATStatus"));
    if (node && appGlobals->natMonitor) {
        QBConfigurationMenu_QBNATMonitorChanged(handler, appGlobals->natMonitor);
        QBNATMonitorRegisterCallback(appGlobals->natMonitor, QBConfigurationMenu_QBNATMonitorChanged, handler);
    }

    node = QBActiveTreeFindNode(menuTree, SVSTRING("MiddlewareStatus"));
    if (node && appGlobals->customerInfoMonitor) {
        CubiwareMWCustomerInfoStatus status = CubiwareMWCustomerInfoGetStatus(appGlobals->customerInfoMonitor);
        handler->middlewareStatusNode = SVRETAIN(node);
        CubiwareMWCustomerInfoAddListener(appGlobals->customerInfoMonitor, (SvGenericObject) handler);
        QBConfigurationMenuSetMiddlewareStatus(handler, status);
    }

    node = QBActiveTreeFindNode(menuTree, SVSTRING("TraxisStatus"));
    if (node && appGlobals->traxisWebSessionManager) {
        handler->traxisStatusNode = SVRETAIN(node);
        TraxisWebSessionManagerAddListener(appGlobals->traxisWebSessionManager, (SvGenericObject) handler, NULL);
    }


    node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ViewRightWebStatus"));
    if (node && appGlobals->viewRightWeb) {
        handler->viewRightWebStatusNode = SVRETAIN(node);
        QBViewRightWebServiceSetListener(appGlobals->viewRightWeb, (SvGenericObject) handler);
        TraxisWebSessionManagerAddListener(appGlobals->traxisWebSessionManager, (SvGenericObject) handler, NULL);
    }

    handler->traxisURLNode = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisURL")));
    handler->traxisSmartcardIDNode = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisSmartCardID")));
    handler->traxisServiceGroupNode = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisServiceGroup")));
    handler->traxisVoDCategory = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisVoDCategory")));
    handler->traxisDeviceIDType = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisDeviceIDType")));
    handler->traxisSummaryNode = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("TraxisSummary")));


    handler->chip_id = SVTESTRETAIN(QBActiveTreeFindNode(menuTree, SVSTRING("chip_id")));
    if (handler->chip_id) {
        QBNetworkSettingsNodeUpdate(handler->chip_id, NULL, NULL);
        SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&s_smartcard_callbacks, handler, "QBConfigurationMenu");
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &callback_info, handler, &handler->cmd);
    }

    handler->menuBar = menuBar;

    handler->path = NULL;
    QBActiveTreeFindNodeByID(handler->tree, SVSTRING("STGS"), &handler->path);
    if (!handler->path) {
        SvLogError("%s() Node not found (handler->path)", __func__);
        SVRELEASE(handler);
        return NULL;
    }

    QBTreePathMapInsert(pathMap, handler->path, (SvGenericObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, handler->path, (SvGenericObject) handler, NULL);

    QBConfigAddListener((SvObject) handler, NULL);
    handler->hdmiService = (HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService"));
    if (!handler->hdmiService) {
        SvLogError("%s(): Cannot get HDMIService from QBServiceRegistry!", __func__);
        SVRELEASE(handler);
        return NULL;
    }
    SVRETAIN(handler->hdmiService);

    return (SvGenericObject) handler;
}

SvLocal void
QBConfigurationMenuWidgetHandlerParentalControlDialogRequested(SvGenericObject self_, QBActiveTreeNode node, SvGenericObject nodePath)
{
    QBConfigurationMenuWidgetHandler widgetHandler = (QBConfigurationMenuWidgetHandler) self_;

    if (!widgetHandler->dialog)
        QBConfigurationMenuWidgetHandlerShowPCDialog(widgetHandler, node, nodePath, QBConfigurationMenuNodeSelectedPINCallback);
}

SvLocal void
QBConfigurationMenuWidgetHandlerHandleAction(SvGenericObject self_, SvString action)
{
    QBConfigurationMenuWidgetHandler widgetHandler = (QBConfigurationMenuWidgetHandler) self_;

    if (SvStringEqualToCString(action, "Enable automatic time zone")) {
        QBConfigurationMenuHideSelectTimeZoneMenu(widgetHandler->dataSource);
    } else if (SvStringEqualToCString(action, "Disable automatic time zone")) {
        QBConfigurationMenuShowSelectTimeZoneMenu(widgetHandler->dataSource);
    }
}

SvLocal void QBConfigurationMenuWidgetConstructorRegister(QBConfigurationMenuDataSourceHandler handler, SvWidget menuBar, QBTreePathMap pathMap)
{

    QBConfigurationMenuWidgetHandler widgetHandler = (QBConfigurationMenuWidgetHandler)
            SvTypeAllocateInstance(QBConfigurationMenuWidgetHandler_getType(), NULL);

    widgetHandler->appGlobals = handler->appGlobals;
    widgetHandler->dataSource = handler;
    svSettingsPushComponent("Configuration.settings");

    widgetHandler->tickMarkOnRID = svSettingsGetResourceID("MenuItem", "tickMarkOn");
    widgetHandler->tickMarkOffRID = svSettingsGetResourceID("MenuItem", "tickMarkOff");
    widgetHandler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    widgetHandler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", handler->appGlobals->textRenderer);
    widgetHandler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        widgetHandler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    widgetHandler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", handler->appGlobals->controller, handler->appGlobals->res,
            SVSTRING("SideMenu"));

    widgetHandler->dialog = NULL;

    struct QBConfigurationMenuCallbacks_s confMenuCallbacks = {
            .authenticationRequested = QBConfigurationMenuWidgetHandlerParentalControlDialogRequested,
            .handleAction = QBConfigurationMenuWidgetHandlerHandleAction
    };
    QBConfigurationMenuDataSourceHandlerRegisterCallbacks((SvGenericObject) handler, (SvGenericObject) widgetHandler, &confMenuCallbacks);

    svSettingsPopComponent();

    SvGenericObject path = NULL;

    if (QBActiveTreeFindNodeByID(handler->tree, SVSTRING("Skin"), &path) && handler->appGlobals->skinManager) {
        svSettingsPushComponent("Carousel_WEBTV.settings");
        SvGenericObject itemController = (SvGenericObject) QBSkinsCarouselMenuItemControllerCreate(handler->appGlobals);
        SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, itemController, NULL);
        svSettingsPopComponent();
        SVRELEASE(itemController);

        SvGenericObject dataSource = (SvGenericObject) QBSkinManagerGetSkins(handler->appGlobals->skinManager);
        QBTreeProxy tree = QBTreeProxyCreate(dataSource, NULL, NULL);

        SvGenericObject itemChoice = (SvGenericObject) QBSkinsCarouselMenuChoiceNew(handler->appGlobals, handler->appGlobals->skinManager);

        QBActiveTreeMountSubTree(handler->tree, (SvGenericObject) tree, path, NULL);

        QBTreePathMapInsert(pathMap, path, itemChoice, NULL);

        SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, itemChoice, NULL);

        SVRELEASE(itemChoice);
        SVRELEASE(tree);
    }

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, handler->path, (SvGenericObject) widgetHandler, NULL);
    SVRELEASE(widgetHandler);
}

SvLocal void
QBConfigurationMenuSetupTimeZoneSubmenu(QBActiveTree menuTree, QBConfigurationMenuDataSourceHandler handler, SvWidget menuBar, QBTreePathMap pathMap)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(menuTree, SVSTRING("TimeZone"));
    if (!node) {
        return;
    }

    handler->selectTimeZoneNode = SVRETAIN(node);
    const char *autoTZ = QBConfigGet("AUTOTZ");
    if (!autoTZ || !strcmp(autoTZ, "YES")) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(handler->selectTimeZoneNode), handler->selectTimeZoneNode, NULL);
        handler->selectTimeZoneNodeHidden = true;
    } else {
        handler->selectTimeZoneNodeHidden = false;
        QBTimeZoneMenuRegister(menuBar, pathMap, handler->appGlobals);
    }
}

SvLocal void
QBConfigurationMenuSetupSatelliteSubmenu(QBActiveTree menuTree)
{
    QBActiveTreeNode node = QBActiveTreeFindNode(menuTree, SVSTRING("SatelliteSetup"));
    if (!node) {
        return;
    }

    QBTunerType currentType = QBTunerType_unknown;
    if (!sv_tuner_get_type(0, &currentType) && currentType != QBTunerType_sat) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    }
}

SvLocal void
QBConfigurationMenuSetupVPNSwitcherSubmenu(QBActiveTree menuTree)
{
    QBActiveTreeNode vpn = QBActiveTreeFindNode(menuTree, SVSTRING("VPN"));
    if (!vpn)
        return;

    const char* vpnConfiguration = QBConfigGet("VPN");
    bool vpnEnabled = (vpnConfiguration && strcmp(vpnConfiguration, "disabled"));
    SvObject iconPath = QBActiveTreeNodeGetAttribute(vpn, vpnEnabled ? SVSTRING("iconOn") : SVSTRING("iconOff"));
    QBActiveTreeNodeSetAttribute(vpn, SVSTRING("icon"), (SvObject) iconPath);
    SvValue enabled = SvValueCreateWithBoolean(vpnEnabled, NULL);
    QBActiveTreeNodeSetAttribute(vpn, SVSTRING("vpnEnabled"), (SvObject) enabled);
    SVRELEASE(enabled);
}

void QBConfigurationMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("STGS"), &path);
    if (!path) {
        return;
    }

    QBConfigurationMenuLogicAdjustMenuTree(appGlobals->configurationMenuLogic, appGlobals->menuTree);

    QBConfigurationMenuDataSourceHandler handler = (QBConfigurationMenuDataSourceHandler) QBConfigurationMenuDataSourceHandlerCreate(menuBar, appGlobals,
                                                                                                                                     appGlobals->menuTree, pathMap);
    QBConfigurationMenuSetupTimeZoneSubmenu(appGlobals->menuTree, handler, menuBar, pathMap);
    QBConfigurationMenuSetupSatelliteSubmenu(appGlobals->menuTree);

    QBTimeZoneMenuRegister(menuBar, pathMap, appGlobals);
    QBNewNetworkSettingsMenuRegister(menuBar, pathMap, appGlobals);
    QBRoutingSettingsMenuRegister(menuBar, pathMap, appGlobals->res,
                                  appGlobals->menuTree, appGlobals->textRenderer,
                                  appGlobals->networkMonitor,
                                  appGlobals->controller);
    QBIPSecVPNStatusMenuRegister(menuBar, pathMap, appGlobals);
    QBClosedCaptionMenuRegister(menuBar, pathMap, appGlobals);
    QBCallerIDMenuRegister(menuBar, pathMap, appGlobals);

    QBConfigurationMenuWidgetConstructorRegister(handler, menuBar, pathMap);
    QBConfigurationMenuSetupVPNSwitcherSubmenu(appGlobals->menuTree);

    SVRELEASE(handler);
}

void QBConfigurationMenuDataSourceHandlerRegisterCallbacks(SvGenericObject dataSourceHandler, SvGenericObject owner, QBConfigurationMenuCallbacks callbacks)
{
    QBConfigurationMenuDataSourceHandler handler = (QBConfigurationMenuDataSourceHandler) dataSourceHandler;
    handler->owner = owner;
    handler->callbacks = *callbacks;
}
