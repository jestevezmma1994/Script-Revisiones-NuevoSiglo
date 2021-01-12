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

#include "QBNewConfigurationMenu.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <settings.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBMenu/QBMenu.h>
#include <main.h>
#include <Services/QBSkinManager.h>
#include <Utils/appType.h>
#include <Utils/authenticators.h>
#include <Widgets/authDialog.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <QBPlatformHAL/QBPlatformStandby.h>
#include <QBAppKit/QBServiceRegistry.h>

#include "QBClosedCaptionMenu.h"
#include "ConfigurationMenu/QBAVOutputMenuItemChoice.h"
#include "ConfigurationMenu/QBAVOutputMenuItemController.h"
#include "ConfigurationMenu/QBChannelInstallationMenuItemChoice.h"
#include "ConfigurationMenu/QBConfigurationMenuDefaultItemController.h"
#include "ConfigurationMenu/QBNetworkInformationMenuItemController.h"
#include "ConfigurationMenu/QBIPSecVPNStatusMenu.h"
#include "ConfigurationMenu/QBSystemInformationMenuItemChoice.h"
#include "ConfigurationMenu/QBSystemInformationMenuItemController.h"
#include "ConfigurationMenu/QBSystemSettingsMenuItemChoice.h"
#include "ConfigurationMenu/QBUserInterfaceMenuItemChoice.h"
#include "ConfigurationMenu/QBCallerIDMenu.h"
#include "ConfigurationMenu/QBDeviceNameMenu.h"
#include "ConfigurationMenu/QBNetworkSettings.h"
#include "ConfigurationMenu/QBRoutingSettings.h"
#include "ConfigurationMenu/QBTimeZoneMenu.h"
#include "Skins/QBSkinsCarouselItemChoice.h"
#include "Skins/QBSkinsCarouselItemController.h"

SvLocal void
QBConfigurationMenuRegisterSkinMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("Skin"), &path) || !appGlobals->skinManager)
        return;

    svSettingsPushComponent("Carousel_WEBTV.settings");
    SvObject itemController = (SvObject) QBSkinsCarouselMenuItemControllerCreate(appGlobals);
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, itemController, NULL);
    svSettingsPopComponent();
    SVRELEASE(itemController);

    SvObject dataSource = (SvObject) QBSkinManagerGetSkins(appGlobals->skinManager);
    QBTreeProxy tree = QBTreeProxyCreate(dataSource, NULL, NULL);

    SvObject itemChoice = (SvObject) QBSkinsCarouselMenuChoiceNew(appGlobals, appGlobals->skinManager);
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvObject) tree, path, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);

    SVRELEASE(itemChoice);
    SVRELEASE(tree);
}

SvLocal void
QBConfigurationMenuRegisterAVOutputMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("OUTC"), &path))
        return;

    SvObject itemController = (SvObject) QBAVOutputMenuItemControllerCreate(appGlobals, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, itemController, NULL);
    SVRELEASE(itemController);

    SvObject itemChoice = (SvObject) QBAVOutputMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterSystemInformationMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("SYIN"), &path))
        return;

    SvObject itemController = (SvObject) QBSystemInformationMenuItemControllerCreate(appGlobals, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, itemController, NULL);
    SVRELEASE(itemController);

    SvObject itemChoice = (SvObject) QBSystemInformationMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterHelpInfoWindow(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("HelpInfo"), &path))
        return;

    SvObject itemChoice = (SvObject) QBSystemInformationMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterSubscriberInfoWindow(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("SubscriberInfo"), &path))
        return;

    SvObject itemChoice = (SvObject) QBSystemInformationMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterNetworkInformationMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("NetworkStatus"), &path))
        return;

    SvObject itemController = (SvObject) QBNetworkInformationMenuItemControllerCreate(appGlobals, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, itemController, NULL);
    SVRELEASE(itemController);
}

SvLocal void
QBConfigurationMenuRegisterSystemSettingsMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("SYST"), &path))
        return;

    SvObject itemChoice = (SvObject) QBSystemSettingsMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterUserInterfaceMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("LOUT"), &path))
        return;

    SvObject itemChoice = (SvObject) QBUserInterfaceMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterLanguagesMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("LNGS"), &path))
        return;

    SvObject itemChoice = (SvObject) QBUserInterfaceMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterCallerIdMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    if (!QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("CallerId"), &path))
        return;

    SvObject itemChoice = (SvObject) QBUserInterfaceMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterChannelInstallationMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    QBActiveTreeNode node = NULL;
    if (!(node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("ChannelInstallation"), &path)))
        return;

    // remove channel installation if box is in ip mode
    if (QBAppTypeIsIP()) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        return;
    }

    // remove satellite setup node
    node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("SatelliteSetup"));
    if (node && !QBAppTypeIsSAT()) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    }

    SvObject itemChoice = (SvObject) QBChannelInstallationMenuItemChoiceCreate(appGlobals, menu, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setEventHandlerForPath, path, itemChoice, NULL);
    SVRELEASE(itemChoice);
}

SvLocal void
QBConfigurationMenuRegisterStandbySettingsMenu(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode node = NULL;

    if (!(QBPlatformGetFrontPanelFlags() & QBFrontPanelCapability_clock)) {
        // remove option "Frontpanel Clock In Standby"
        if ((node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("Frontpanel clock")))) {
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
        }
    }

    // nothing to do if STB supports passive standby
    if (QBPlatformGetStandbyCapabilities() & QBStandbyCapability_passive) {
        return;
    }

    // remove option "Passive Standby" in submenu "RCU Power Button"
    if ((node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("RCPASSIVE")))) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    }

    // remove option "Prompt" in submenu "RCU Power Button"
    if ((node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("RCCOMBINED")))) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    }

    // remove option "Passive Standby" in submenu "Front Panel Power Button"
    if ((node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("FPPASSIVE")))) {
        QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
    }
}

SvLocal void
QBConfigurationMenuRegisterMenus(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBConfigurationMenuRegisterSkinMenu(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterAVOutputMenu(menu, pathMap, appGlobals);

    QBConfigurationMenuRegisterNetworkInformationMenu(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterSystemInformationMenu(menu, pathMap, appGlobals);

    QBConfigurationMenuRegisterHelpInfoWindow(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterSubscriberInfoWindow(menu, pathMap, appGlobals);

    QBTimeZoneMenuRegister(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterSystemSettingsMenu(menu, pathMap, appGlobals);

    QBConfigurationMenuRegisterUserInterfaceMenu(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterChannelInstallationMenu(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterLanguagesMenu(menu, pathMap, appGlobals);
    QBClosedCaptionMenuRegister(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterStandbySettingsMenu(menu, pathMap, appGlobals);

    QBDeviceNameMenuRegister(pathMap, appGlobals);
    QBNewNetworkSettingsMenuRegister(menu, pathMap, appGlobals);
    QBRoutingSettingsMenuRegister(menu, pathMap, appGlobals->res,
                                  appGlobals->menuTree, appGlobals->textRenderer,
                                  appGlobals->networkMonitor,
                                  appGlobals->controller);
    QBIPSecVPNStatusMenuRegister(menu, pathMap, appGlobals);
    QBCallerIDMenuRegister(menu, pathMap, appGlobals);
    QBConfigurationMenuRegisterCallerIdMenu(menu, pathMap, appGlobals);
}

void
QBNewConfigurationMenuRegister(SvWidget menu, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = NULL;
    QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("NEWSTGS"), &path);
    if (!path) {
        return;
    }

    QBConfigurationMenuLogicAdjustMenuTree(appGlobals->configurationMenuLogic, appGlobals->menuTree);
    SvObject itemController = (SvObject) QBConfigurationMenuDefaultItemControllerCreate(appGlobals, appGlobals->menuTree, NULL);
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, itemController, NULL);
    SVRELEASE(itemController);

    QBConfigurationMenuRegisterMenus(menu, pathMap, appGlobals);
}
