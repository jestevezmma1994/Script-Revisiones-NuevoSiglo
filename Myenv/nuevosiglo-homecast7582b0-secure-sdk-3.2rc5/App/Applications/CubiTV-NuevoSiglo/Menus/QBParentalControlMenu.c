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

#include "QBParentalControlMenu.h"

#include <libintl.h>
#include <QBConf.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <settings.h>
#include <QBDataModel3/QBTreePath.h>
#include <SvFoundation/SvInterface.h>
#include <QBInput/QBInputCodes.h>

#include <Menus/ConfigurationMenu/QBConfigurationMenuUtils.h>
#include <Windows/newtv.h>
#include <Windows/mainmenu.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <Widgets/pinChangeDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Utils/authenticators.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBComboBox.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBParentalControlLockedTimeEditPane.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <Logic/timeFormat.h>
#include <Menus/menuchoice.h>
#include <SWL/QBFrame.h>
#include <Hints/hintsGetter.h>
#include <TranslationMerger.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <main.h>
#include <QBPCRatings/QBPCRating.h>
#include <QBPCRatings/QBPCList.h>
#include <QBPCRatings/QBPCUtils.h>
#include "QBChannelMenu.h"
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

#define PC_RELOCK_HOURS_MAX_DEFAULT     8

typedef enum PCRatingsSelectionMode_ {
    PCRatingsSelectionMode_Group,     // all ratings lower than selected rating will be unlocked
    PCRatingsSelectionMode_Selective  // in the cleared state: all ratings higher than selected rating will be locked
} PCRatingsSelectionMode;

//Relevant only in case of PCRatingsSelectionMode_Group locking mode
typedef enum PCRatingSelectionMode_ {
    PCRatingSelectionMode_Locking, // selected rating will be locked
    PCRatingSelectionMode_Unlocking  // selected rating will be unlocked
} PCRatingSelectionMode;

typedef enum PCRatingsOrderMode_ {
    PCRatingsOrderMode_LowThenHigh,    // ratings listed from lowest to highest value in menu
    PCRatingsOrderMode_HighThenLow     // ratings listed from highest to lowest value in menu
} PCRatingsOrderMode;

struct QBParentalControlMenuHandler_t {
    struct SvObject_ super_;
    SvGenericObject tvChannels;
    AppGlobals appGlobals;
    SvWidget menuBar;

    SvWidget dialog;

    QBBasicPane limitedTimePane;
    QBContextMenu menuPanel;
    QBContextMenu relockTimeMenu;
    SvWidget relockTimeWidget;
    QBActiveTreeNode nodeParentalControl;
    SvGenericObject nodePathParentalControl;
    SvWidget lockedTimePaneWidget;
    SvHashTable ltTasks;
    QBActiveTreeNode activeNode;

    size_t depth;

    PCRatingsSelectionMode ratingsSelectionMode;
    PCRatingSelectionMode ratingSelectionMode;
    PCRatingsOrderMode ratingsOrderMode;
    bool allowModificationWhenOff;

    QBTreeProxy channelProxy;
};

struct QBParentalControlItemController_t {
    struct SvObject_ super_;

    QBActiveTree menuTree;
    size_t depth;

    SvRID tickMarkOnRID;
    SvRID tickMarkOnInactiveRID;
    SvRID tickMarkOffRID;
    SvRID radioMarkOnRID;
    SvRID radioMarkOnInactiveRID;
    SvRID radioMarkOffRID;
    SvRID activeLockedIconRID;
    SvRID activeUnlockedIconRID;
    SvRID inactiveLockedIconRID;
    SvRID inactiveUnlockedIconRID;

    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;

    QBTextRenderer renderer;
    QBParentalControl pc;
};
typedef struct QBParentalControlItemController_t* QBParentalControlItemController;

SvLocal SvString QBParentalControlMenuCombineLockedTimeSubcaption(QBParentalControlLockedTimeTask task)
{
    char startStr[100] = {'\0'}, endStr[100] = {'\0'};
    struct tm start = {
        .tm_min = task->startTime % 60,
        .tm_hour = task->startTime / 60,
    };
    struct tm end = {
        .tm_min = task->endTime % 60,
        .tm_hour = task->endTime / 60,
    };
    strftime(startStr, sizeof(startStr), QBTimeFormatGetCurrent()->timeEditBox, &start);
    strftime(endStr, sizeof(endStr), QBTimeFormatGetCurrent()->timeEditBox, &end);
    return SvStringCreateWithFormat("%s - %s", startStr, endStr);
}

SvLocal bool
QBParentalControlIsRatingItem(QBActiveTreeNode node)
{
    bool result = false;
    if (!node)
        return result;

    if (QBActiveTreeNodeGetAttribute(node, SVSTRING("PC")))
        result = true;

    return result;
}

SvLocal bool
QBParentalControlNodeContainsRatings(QBActiveTreeNode node)
{
    bool result = false;
    if (!node)
        return result;

    SvIterator it = QBActiveTreeNodeChildNodesIterator(node);
    QBActiveTreeNode child;
    while ((child = (QBActiveTreeNode) SvIteratorGetNext(&it))) {
        if (QBParentalControlIsRatingItem(child)) {
            result = true;
            break;
        }
    }

    return result;
}

SvLocal
SvArray QBParentalControlMenuCreateUsedStandardsList(QBActiveTreeNode node)
{
    SvErrorInfo error = NULL;
    SvString standardsForNode =  (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("PC_STANDARD"));
    if (!standardsForNode)
        return NULL;

    QBPCList listFromNode = QBPCListCreate(&error);
    if (error)
        goto err;

    QBPCListLockRatingsFromString(listFromNode, standardsForNode, &error);
    if (error)
        goto err;

    SvArray usedStandards = QBPCListCreateUsedStandardsList(listFromNode, &error);
    if (error)
        goto err;

    SVRELEASE(listFromNode);
    return usedStandards;

err:
    SVTESTRELEASE(listFromNode);
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return NULL;
}

SvLocal
bool QBParentalControlMenuAreRatingsFromNodeCleared(QBActiveTreeNode node, QBPCList ratingsList)
{
    int notClearedStandards = 0;

    if (!node || !ratingsList)
        return false;

    SvArray usedStandards = QBParentalControlMenuCreateUsedStandardsList(node);
    if (!usedStandards)
        return false;

    SvIterator standardsIterator = SvArrayGetIterator(usedStandards);
    SvValue ratingStandardValue = NULL;
    while ((ratingStandardValue = (SvValue) SvIteratorGetNext(&standardsIterator))) {
        QBPCRatingStandard ratingStandard = SvValueGetInteger(ratingStandardValue);
        if (!QBPCListIsStandardCleared(ratingsList, ratingStandard, NULL))
            ++notClearedStandards;
    }

    SVRELEASE(usedStandards);

    return (notClearedStandards == 0);
}

SvLocal void QBParentalControlItemControllerCreateCaptionsItem(QBParentalControlItemController self, QBActiveTreeNode node, QBXMBItem item)
{
    SvString val = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    if (val) {
        const char* captionStr = SvStringCString(val);
        const char* translatedCaptionStr = *captionStr ? gettext(captionStr) : "";
        if (translatedCaptionStr != captionStr) {
            val = SvStringCreate(translatedCaptionStr, NULL);
        } else {
            SVRETAIN(val);
        }
        item->caption = val;
    }

    val = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption"));
    if (val && SvStringLength(val) != 0) {
        const char* subcaptionStr = SvStringCString(val);
        const char* translatedSubcaptionStr = gettext(subcaptionStr);
        if (translatedSubcaptionStr != subcaptionStr) {
            val = SvStringCreate(translatedSubcaptionStr, NULL);
        } else {
            SVRETAIN(val);
        }
        item->subcaption = val;
    }
}

SvLocal void QBParentalControlItemControllerCreateMenuItem(QBParentalControlItemController self, QBActiveTreeNode node, SvApplication app, QBXMBItem item, QBActiveTreeNode parent)
{
    SvString parentId = parent ? QBActiveTreeNodeGetID(parent) : NULL;

    SvString rating = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("PC"));

    if (parentId && SvStringEqualToCString(parentId, "PCLockingStatus")) {
        QBParentalControlState state = QBParentalControlGetState(self->pc);
        SvString nodeState = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("PC_STATE"));

        if (nodeState) {
            if (((state == QBParentalControlState_On) && SvStringEqualToCString(nodeState, "ON")) ||
                ((state == QBParentalControlState_Off_AutoRelock) && SvStringEqualToCString(nodeState, "OFF_AUTO")) ||
                ((state == QBParentalControlState_Off_RelockManually) && SvStringEqualToCString(nodeState, "OFF"))) {
                item->iconRID = self->radioMarkOnRID;
            } else {
                item->iconRID = self->radioMarkOffRID;
            }
        } else {
            item->iconRID = self->radioMarkOffRID;
        }
    }
    // BEGIN AMERELES Bloquear títulos según control parental
    else if (parentId && SvStringEqualToCString(parentId, "PCHideTitles"))
    {
        const char* PC_HIDE_TITLES = QBConfigGet("PC_HIDE_TITLES");
        SvString nodeState = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("PC_HIDE_TITLES"));
        
        if (nodeState && PC_HIDE_TITLES)
        {
            if (SvStringEqualToCString(nodeState, PC_HIDE_TITLES))
            {
                item->iconRID = self->radioMarkOnRID;
            }
            else
            {
                item->iconRID = self->radioMarkOffRID;
            }
        }
        else
        {
            item->iconRID = self->radioMarkOffRID;
        }
    }
    // END AMERELES Bloquear títulos según control parental
    else if (parentId && QBParentalControlNodeContainsRatings(QBActiveTreeNodeGetParentNode(node)) && rating) {
        if (!SvStringEqualToCString(rating, "CLEAR")) {
            switch (QBParentalControlGetRatingStatus(self->pc, rating)) {
                case QBParentalControlRatingStatus_InactiveUnlocked:
                    item->iconRID = self->inactiveUnlockedIconRID;
                    break;
                case QBParentalControlRatingStatus_ActiveUnlocked:
                    item->iconRID = self->activeUnlockedIconRID;
                    break;
                case QBParentalControlRatingStatus_InactiveLocked:
                    item->iconRID = self->inactiveLockedIconRID;
                    break;
                case QBParentalControlRatingStatus_ActiveLocked:
                    item->iconRID = self->activeLockedIconRID;
                    break;
            }
        } else {
            //TODO: Assessment of node rating standard to check selected standard data
            SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("staticIcon"));
            if (iconPath) {
                item->iconURI.URI = SVTESTRETAIN(iconPath);
                item->iconURI.isStatic = true;
            } else {
                if (QBParentalControlMenuAreRatingsFromNodeCleared(QBActiveTreeNodeGetParentNode(node), QBParentalControlGetRating(self->pc))) {
                    item->iconRID = (QBParentalControlGetState(self->pc) == QBParentalControlState_On) ? (self->tickMarkOnRID) : (self->tickMarkOnInactiveRID);
                } else {
                    item->iconRID = self->tickMarkOffRID;
                }
            }
        }
    } else if (parentId && SvStringEqualToCString(parentId, "PCLockedTime")) {
        QBParentalControlLockedTimeTask task = NULL;
        if ((task = (QBParentalControlLockedTimeTask) QBActiveTreeNodeGetAttribute(node, SVSTRING("task")))) {
            if (!SvStringEqualToCString(QBActiveTreeNodeGetID(node), "LockedTimeEmptyNode")) {
                if (task->state) {
                    item->iconRID = self->radioMarkOnRID;
                } else {
                    item->iconRID = self->radioMarkOffRID;
                }
            }
            item->caption = SvStringCreate(gettext(QBParentalControlLimitedTimeDayToString(task->option)), NULL);
            SvString subcaption = QBParentalControlMenuCombineLockedTimeSubcaption(task);
            item->subcaption = SvStringCreate(gettext(SvStringCString(subcaption)), NULL);
            SVRELEASE(subcaption);
        }
    } else {
        QBConfigurationMenuTickState tickState = QBConfigurationMenuCheckTickState(node);
        if (tickState != QBConfigurationMenuTickState__Hidden) {
            SvString tickType = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("tickType"));
            if (tickType && SvStringEqualToCString(tickType, "RADIO")) {
                if (tickState == QBConfigurationMenuTickState__On)
                    item->iconRID = self->radioMarkOnRID;
                else
                    item->iconRID = self->radioMarkOffRID;
            } else {
                if (tickState == QBConfigurationMenuTickState__On)
                    item->iconRID = self->tickMarkOnRID;
                else
                    item->iconRID = self->tickMarkOffRID;
            }
        }

        if (!item->iconRID) {
            SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
            if (iconPath) {
                item->iconURI.URI = SVTESTRETAIN(iconPath);
                item->iconURI.isStatic = true;
            }
        }
    }
}

SvLocal SvWidget
QBParentalControlItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_,
                                          SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBParentalControlItemController self = (QBParentalControlItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        return NULL;
    }

    if (!path) {
        QBActiveTreeFindNodeByID(self->menuTree, QBActiveTreeNodeGetID(node), &path);
    }

    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVTESTRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    QBParentalControlItemControllerCreateCaptionsItem(self, node, item);

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    size_t nodeDepth = path ? SvInvokeInterface(QBTreePath, path, getLength) : 0;
    while (nodeDepth > self->depth + 2 && parent) {
        parent = QBActiveTreeNodeGetParentNode(parent);
        nodeDepth--;
    }

    QBParentalControlItemControllerCreateMenuItem(self, node, app, item, parent);

    SvWidget ret = NULL;
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);

    return ret;
}

SvLocal void QBParentalControlItemControllerSetItemState(SvGenericObject self_, SvWidget item, XMBMenuState state, bool isFocused)
{
    QBParentalControlItemController self = (QBParentalControlItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item, state, isFocused);
}

SvLocal void QBParentalControlItemControllerDestroy(void *self_)
{
    QBParentalControlItemController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
}

SvLocal SvType QBParentalControlItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBParentalControlItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t methods = {
        .createItem = QBParentalControlItemControllerCreateItem,
        .setItemState = QBParentalControlItemControllerSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBParentalControlItemController",
                            sizeof(struct QBParentalControlItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal SvGenericObject QBParentalControlItemControllerCreate(QBTextRenderer renderer, QBParentalControl pc, QBActiveTree menuTree, size_t depth)
{
    QBParentalControlItemController self = (QBParentalControlItemController) SvTypeAllocateInstance(QBParentalControlItemController_getType(), NULL);

    svSettingsPushComponent("ParentalControl.settings");

    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem.Item", renderer);
    if (svSettingsIsWidgetDefined("MenuItem.focus")) {
        self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    }
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus")) {
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    }

    self->menuTree = menuTree;
    self->depth = depth;
    self->tickMarkOnRID = svSettingsGetResourceID("MenuItem.Item", "tickMarkOn");
    self->tickMarkOnInactiveRID = svSettingsGetResourceID("MenuItem.Item", "tickMarkOnInactive");
    self->tickMarkOffRID = svSettingsGetResourceID("MenuItem.Item", "tickMarkOff");
    self->radioMarkOnRID = svSettingsGetResourceID("MenuItem.Item", "radioMarkOn");
    self->radioMarkOnInactiveRID = svSettingsGetResourceID("MenuItem.Item", "radioMarkOnInactive");
    self->radioMarkOffRID = svSettingsGetResourceID("MenuItem.Item", "radioMarkOff");
    self->activeLockedIconRID = svSettingsGetResourceID("MenuItem.Item", "activeLockedIcon");
    self->activeUnlockedIconRID = svSettingsGetResourceID("MenuItem.Item", "activeUnlockedIcon");
    self->inactiveLockedIconRID = svSettingsGetResourceID("MenuItem.Item", "inactiveLockedIcon");
    self->inactiveUnlockedIconRID = svSettingsGetResourceID("MenuItem.Item", "inactiveUnlockedIcon");

    self->renderer = renderer;
    self->pc = pc;

    svSettingsPopComponent();

    return (SvGenericObject) self;
}

SvLocal void QBParentalControlMenuCheckPin(QBParentalControlMenuHandler self, QBDialogCallback callback)
{
    if (self->dialog) {
        return;
    }

    svSettingsPushComponent("ParentalControl.settings");

    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC_MENU"));
    self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, NULL);
    QBDialogRun(self->dialog, self, callback);

    svSettingsPopComponent();
}

SvLocal void QBParentalControlMenuPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBParentalControlMenuHandler self = self_;
    self->dialog = NULL;
}

SvLocal void QBParentalControlMenuChangePIN(QBParentalControlMenuHandler self, SvString id)
{
    if (self->dialog) {
        return;
    }

    SvString domain = NULL;
    if (SvStringEqualToCString(id, "PCChangePurchasePin")) {
        domain = SVSTRING("VOD_AUTH");
    } else {
        domain = SVSTRING("PC_MENU");
    }

    svSettingsPushComponent("ParentalControl.settings");
    self->dialog = QBPinChangeDialogCreate(self->appGlobals, "PinChangeDialog", domain, true, NULL);
    svSettingsPopComponent();
    QBDialogRun(self->dialog, self, QBParentalControlMenuPopupCallback);
}

SvLocal void QBParentalControlMenuLockedTimeLimitWarningCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBParentalControlMenuHandler self = self_;
    QBContextMenuPopPane(self->menuPanel);
}


SvLocal void QBParentalControlMenuLockedTimeShowPaneWithWarning(QBParentalControlMenuHandler self, SvString msg)
{
    if (!self || !msg || !self->menuPanel) {
        SvLogInfo("[%s] Error(self:%p, msg:%p, menuPanel:%p)", __func__, self, msg,
            (self && self->menuPanel ? self->menuPanel : NULL));
    }
    svSettingsPushComponent("BasicPane.settings");

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
        self->menuPanel, 2, SVSTRING("BasicPane"));

    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBParentalControlMenuLockedTimeLimitWarningCallback, self);
    SVRELEASE(option);

    QBBasicPaneSetTitle(options, msg);

    QBContextMenuPushPane(self->menuPanel, (SvGenericObject) options);

    svSettingsPopComponent();
    SVRELEASE(options);
}

SvLocal SvString RelockTimeComboPrepare(void *self_, SvWidget combobox, SvGenericObject value)
{
    int timePoint = SvValueGetInteger((SvValue) value);

    return SvStringCreateWithFormat(ngettext("%d hour", "%d hours", timePoint), timePoint);
}

SvLocal bool RelockTimeComboInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    if (e->ch != QBKEY_ENTER) {
        return false;
    }

    QBParentalControlMenuHandler self = self_;

    SvValue value = (SvValue)QBComboBoxGetValue(combobox);
    QBParentalControlSetAutoRelockTime(self->appGlobals->pc, SvValueGetInteger(value));

    QBContextMenuHide(self->relockTimeMenu, false);

    return true;
}

SvLocal SvWidget RelockTimeWidgetCreate(QBParentalControlMenuHandler self, SvApplication app)
{
    self->relockTimeWidget = svSettingsWidgetCreate(app, "RelockTime");

    SvWidget title = QBAsyncLabelNew(self->appGlobals->res, "RelockTime.Title", self->appGlobals->textRenderer);

    svSettingsWidgetAttach(self->relockTimeWidget, title, svWidgetGetName(title), 1);
    QBAsyncLabelSetCText(title, gettext("Set Auto Relock Time"));

    SvWidget combo = QBComboBoxNewFromSM(app, "RelockTime.ComboBox");

    svSettingsWidgetAttach(self->relockTimeWidget, combo, svWidgetGetName(combo), 1);

    svWidgetSetFocusable(combo, true);
    QBComboBoxCallbacks cb = { RelockTimeComboPrepare, RelockTimeComboInput, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(combo, self, cb);

    int maxHours = PC_RELOCK_HOURS_MAX_DEFAULT;
    const char* maxHoursStr = QBConfigGet("PC_RELOCK_HOURS_MAX");
    if (maxHoursStr) {
        maxHours = atoi(maxHoursStr);
    }

    SvArray values = SvArrayCreate(NULL);
    int idx=0;
    for (idx = 1; idx <= maxHours; idx++) {
        SvGenericObject v =(SvGenericObject)SvValueCreateWithInteger(idx, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(combo, values);
    SVRELEASE(values);

    QBComboBoxSetPosition(combo, QBParentalControlGetAutoRelockTime(self->appGlobals->pc) - 1);

    return self->relockTimeWidget;
}

SvLocal void QBParentalControlMenuSetRelockTimeMenuOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBParentalControlMenuHandler self = self_;

    svWidgetDestroy(self->relockTimeWidget);
    self->relockTimeWidget = NULL;
}

SvLocal void QBParentalControlMenuSetRelockTimeMenuOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBParentalControlMenuHandler self = self_;

    svSettingsPushComponent("ParentalControl.settings");

    self->relockTimeWidget = RelockTimeWidgetCreate(self, self->appGlobals->res);
    svSettingsWidgetAttach(frame, self->relockTimeWidget, svWidgetGetName(self->relockTimeWidget), 0);

    svSettingsPopComponent();
}

SvLocal void QBParentalControlMenuSetRelockTimeMenuSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBParentalControlMenuHandler self = self_;
    svWidgetSetFocus(self->relockTimeWidget);
}

SvLocal void QBParentalControlMenuSetRelockTimeMenu(QBParentalControlMenuHandler self)
{
    SVTESTRELEASE(self->relockTimeMenu);
    self->relockTimeMenu = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBParentalControlMenuSetRelockTimeMenuOnHide,
        .onShow    = QBParentalControlMenuSetRelockTimeMenuOnShow,
        .setActive = QBParentalControlMenuSetRelockTimeMenuSetActive
    };
    QBContainerPane options = (QBContainerPane)SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(options, self->appGlobals->res, self->relockTimeMenu, 1, SVSTRING("BasicPane"), &callbacks, self);

    QBContextMenuPushPane(self->relockTimeMenu, (SvGenericObject) options);
    QBContextMenuShow(self->relockTimeMenu);
    SVRELEASE(options);
}

SvLocal void QBParentalControlMenuHandlerDestroy(void *self_)
{
    QBParentalControlMenuHandler self = self_;
    SVRELEASE(self->tvChannels);
    SVRELEASE(self->channelProxy);
    SVTESTRELEASE(self->relockTimeMenu);
    SVTESTRELEASE(self->ltTasks);
    SVTESTRELEASE(self->menuPanel);
    SVTESTRELEASE(self->nodeParentalControl);
    SVTESTRELEASE(self->nodePathParentalControl);
    SVTESTRELEASE(self->activeNode);
    self->activeNode = NULL;
}

SvLocal ChannelChoiceTickState QBParentalControlChannelIsTicked(void *self_, SvGenericObject channel)
{
    QBParentalControlMenuHandler self = self_;

    QBParentalControlChannelLockingStatus status = QBParentalControlGetChannelLockingStatus(self->appGlobals->pc, channel);
    if (status == QBParentalControlChannelLockingStatus_NotLocked) {
        return ChannelChoiceTickState_Hidden;
    }

    if (status == QBParentalControlChannelLockingStatus_ActiveLocked) {
        return ChannelChoiceTickState_On;
    }

    return ChannelChoiceTickState_Off;
}

SvLocal SvString QBParentalControlCreateCaption(void *self_, SvGenericObject channel_)
{
    if (!SvObjectIsInstanceOf(channel_, SvTVChannel_getType()))
        return NULL;

    SvTVChannel channel = (SvTVChannel) channel_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject allChannels = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    SvString caption = SvStringCreateWithFormat("%03i %s", SvInvokeInterface(SvEPGChannelView, allChannels, getChannelNumber, channel), SvStringCString(channel->name));

    return caption;
}

SvGenericObject QBParentalControlMenuCreateLockedTimeNode(AppGlobals appGlobals, QBParentalControlLockedTimeTask task)
{
    if(!appGlobals || !task) {
        return NULL;
    }

    QBActiveTreeNode node = NULL;
    QBActiveTreeNode parent = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("PCLockedTime"));
    if(parent) {
        node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("task"), (SvGenericObject) task);
        QBActiveTreeAddSubTree(appGlobals->menuTree, (QBActiveTreeNode)parent, node, NULL);
    }
    return (SvGenericObject) node;
}

SvLocal void QBParentalControlMenuLockedTimeAddEmptyNode(AppGlobals appGlobals)
{
    if(!appGlobals) {
        return;
    }
    SvGenericObject path = NULL;
    QBActiveTreeNode node = NULL;

    if (QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LockedTimeEmptyNode"))) {
        return;
    }
    QBActiveTreeNode parent = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("PCLockedTime"), &path);
    if(!path) {
        return;
    }

    node = QBActiveTreeNodeCreate(SVSTRING("LockedTimeEmptyNode"), NULL, NULL);
    if (!node) {
        return;
    }
    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) SVSTRING(gettext_noop("List is empty")));
    QBActiveTreeAddSubTree(appGlobals->menuTree, (QBActiveTreeNode) parent, node, NULL);

    SVRELEASE(node);
}

void QBParentalControlMenuLockedTimeRemoveEmptyNode(AppGlobals appGlobals)
{
    if (!appGlobals) {
        return;
    }
    QBActiveTreeNode node = NULL;
    if (!(node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("LockedTimeEmptyNode")))) {
        return;
    }
    QBActiveTreeRemoveSubTree(appGlobals->menuTree, node, NULL);
}

SvLocal void QBParentalControlMenuShowLockedTimePane(QBParentalControlMenuHandler self, QBParentalControlLockedTimeTask task)
{
    if (self) {
        svSettingsPushComponent("ParentalControl.settings");
        QBParentalControlLockedTimeEditPane extendedInfo =  (QBParentalControlLockedTimeEditPane)
            SvTypeAllocateInstance(QBParentalControlLockedTimeEditPane_getType(), NULL);
        QBParentalControlLockedTimeEditPaneInit(extendedInfo, self->appGlobals, self->menuPanel, SVSTRING("LockedTimeExtendedInfo"), 2, self, task, self->activeNode);
        QBContextMenuPushPane(self->menuPanel, (SvGenericObject) extendedInfo);
        SVRELEASE(extendedInfo);
        svSettingsPopComponent();
    }
}

SvLocal void QBParentalControlMenuLockedTimeActionCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;
    if (SvStringEqualToCString(item->id, "add")) {
        if (QBParentalControlGetLockedTimeTasksCount(self->appGlobals->pc) < QBPC_LT_MAX_TASKS) {
            QBParentalControlMenuShowLockedTimePane(self, NULL);
        } else {
            svSettingsPushComponent("ParentalControl.settings");
            SvString msg = SvStringCreate(gettext(svSettingsGetString("LockedTime.messages", "messageTasksLimitExceeded")), NULL);
            QBParentalControlMenuLockedTimeShowPaneWithWarning(self, msg);
            SVRELEASE(msg);
            svSettingsPopComponent();
        }
    } else if (SvStringEqualToCString(item->id, "edit")) {
        QBParentalControlLockedTimeTask task = NULL;
        if (self->activeNode && (task = (QBParentalControlLockedTimeTask) SvHashTableFind(self->ltTasks, (SvGenericObject)self->activeNode))) {
            QBParentalControlMenuShowLockedTimePane(self, task);
        }
    }
}

SvLocal void QBParentalControlMenuSetActiveNode(QBParentalControlMenuHandler self, QBActiveTreeNode node);

SvLocal void QBParentalControlMenuLockedTimeRemoveActionCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBParentalControlMenuHandler self = self_;
    if(SvStringEqualToCString(id, "OK")) {
        SvArray tasks = QBParentalControlGetLockedTimeTasks(self->appGlobals->pc);
        if (tasks && SvArrayCount(tasks) == 1) { //< last task
            QBParentalControlMenuLockedTimeAddEmptyNode(self->appGlobals);
        }
        QBParentalControlLockedTimeTask task = (QBParentalControlLockedTimeTask) SvHashTableFind(self->ltTasks,
            (SvGenericObject)self->activeNode);
        QBParentalControlLockedTimeRemoveTask(self->appGlobals->pc, task);
        SvHashTableRemove(self->ltTasks, (SvGenericObject)self->activeNode);
        QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, self->activeNode, NULL);
        QBContextMenuHide(self->menuPanel, false);
        QBParentalControlMenuSetActiveNode(self, NULL);
    } else {
        QBContextMenuPopPane(self->menuPanel);
    }
}

SvLocal void QBParentalControlMenuLockedTimeChoosen(QBParentalControlMenuHandler self)
{
    SVTESTRELEASE(self->menuPanel);

    self->menuPanel = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler,
        self->appGlobals->textRenderer, self->menuPanel, 1, SVSTRING("BasicPane"));

    SvString option = NULL;

    option = SvStringCreate(gettext("Add"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("add"), option, QBParentalControlMenuLockedTimeActionCallback, self);
    SVRELEASE(option);

    SvArray tasks = QBParentalControlGetLockedTimeTasks(self->appGlobals->pc);
    if (tasks && SvArrayCount(tasks) > 0) {

        SvArray confirmation = SvArrayCreate(NULL);
        SvArray ids = SvArrayCreate(NULL);
        option = SvStringCreate(gettext("OK"), NULL);
        SvArrayAddObject(confirmation, (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
        SVRELEASE(option);

        option = SvStringCreate(gettext("Cancel"), NULL);
        SvArrayAddObject(confirmation, (SvGenericObject) option);
        SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
        SVRELEASE(option);

        option = SvStringCreate(gettext("Remove"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("remove"), option,
            SVSTRING("BasicPane"), confirmation, ids, QBParentalControlMenuLockedTimeRemoveActionCallback, self);
        SVRELEASE(option);

        option = SvStringCreate(gettext("Edit"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("edit"), option, QBParentalControlMenuLockedTimeActionCallback, self);
        SVRELEASE(option);
        SVRELEASE(ids);
        SVRELEASE(confirmation);
    }

    QBContextMenuPushPane(self->menuPanel, (SvGenericObject) options);
    QBBasicPaneSetPosition(options, SVSTRING("add"), true);
    QBContextMenuShow(self->menuPanel);

    svSettingsPopComponent();
    SVRELEASE(options);
}

SvLocal
void QBParentalControlMenuClearRatingsFromNode(QBActiveTreeNode node, QBPCList ratingsList)
{
    if (!node || !ratingsList)
        return;

    SvArray enabledStandard = QBParentalControlMenuCreateUsedStandardsList(node);
    if (!enabledStandard)
        return;

    SvIterator standardsIterator = SvArrayGetIterator(enabledStandard);
    SvValue ratingStandardValue = NULL;
    while ((ratingStandardValue = (SvValue) SvIteratorGetNext(&standardsIterator))) {
        QBPCRatingStandard ratingStandard = SvValueGetInteger(ratingStandardValue);
        QBPCListClearStandard(ratingsList, ratingStandard, NULL);
    }
    SVRELEASE(enabledStandard);
}

/**
 * Pointer to function handling selection of ratings from given node.
 *
 * Given menu position may contain ratings from one or few standards,
 * what's more: they can be selected in different ways. This pointer
 * enables passing specific function handling given selection mode, e.g.
 * - selective (selected rating is toggled - on/off)
 * - group (lower ratings than selected are being locked, higher - unlocked)
 *
 * @param ratingsList ratings list handle
 * @param currentRating rating to process
 * @param selectionMode selection mode - locking / unlocking selected rating, applicable only in group mode
 * @param currentNodeIndex index of currently processed node (in group mode all sub-nodes of "ratings container" node are processed)
 * @param selectedNodeIndex index of node which was selected in menu
 */
typedef void (*selectionModeHandler)(QBPCList ratingsList, QBPCRating currentRating,
                                     PCRatingSelectionMode selectionMode, int currentNodeIndex, int selectedNodeIndex, bool isLowThenHigh);
/**
 * Function is used to handle menu positions that contain (possibly) few ratings like:
 * TV-PG/PG
 * G/TV-G
 * Function calls selectionModeHandler to perform appropriate action for currently
 * selected PCRatingSelectionMode.
 */
SvLocal
void QBParentalControlMenuHandleMultipleRatingsSelection(QBPCList ratingsList, PCRatingSelectionMode selectionMode,
        int currentNodeIndex, int selectedNodeIndex, SvString ratingsString, selectionModeHandler selectedNodeAction, bool isLowThenHigh)
{
    if (!ratingsString)
        return;
    SvArray ratingsFromNode = QBPCUtilsCreateRatingsFromString(ratingsString);
    SvIterator ratingsFromNodeIterator = SvArrayGetIterator(ratingsFromNode);
    QBPCRating currentRating = NULL;
    while ((currentRating = (QBPCRating) SvIteratorGetNext(&ratingsFromNodeIterator))) {
        selectedNodeAction(ratingsList, currentRating, selectionMode, currentNodeIndex, selectedNodeIndex, isLowThenHigh);
    }
    SVRELEASE(ratingsFromNode);
}

SvLocal
void QBParentalControlMenuHandleGroupMode(QBPCList ratingsList, QBPCRating currentRating,
                                          PCRatingSelectionMode mode, int currentNodeIndex, int selectedNodeIndex, bool isLowThenHigh)
{
    SvErrorInfo error = NULL;
    if (mode == PCRatingSelectionMode_Locking) {
        if (currentNodeIndex == selectedNodeIndex) {
            QBPCListLockRating(ratingsList, currentRating, &error);
        }
        if ((currentNodeIndex > selectedNodeIndex && isLowThenHigh) ||
            (currentNodeIndex < selectedNodeIndex && !isLowThenHigh)) {
            QBPCListLockRating(ratingsList, currentRating, &error);
        }
        else if ((currentNodeIndex < selectedNodeIndex && isLowThenHigh) ||
                 (currentNodeIndex > selectedNodeIndex && !isLowThenHigh)) {
            QBPCListUnlockRating(ratingsList, currentRating, &error);
        }
    } else if (mode == PCRatingSelectionMode_Unlocking) {
        if (currentNodeIndex == selectedNodeIndex) {
            QBPCListUnlockRating(ratingsList, currentRating, &error);
        }
        if ((currentNodeIndex > selectedNodeIndex && isLowThenHigh)||
            (currentNodeIndex < selectedNodeIndex && !isLowThenHigh)){
            QBPCListLockRating(ratingsList, currentRating, &error);
        }
        else if ((currentNodeIndex < selectedNodeIndex && isLowThenHigh) ||
                 (currentNodeIndex > selectedNodeIndex && !isLowThenHigh)) {
            QBPCListUnlockRating(ratingsList, currentRating, &error);
        }
    }
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal
void QBParentalControlMenuHandleSelectiveMode(QBPCList ratingsList, QBPCRating currentRating,
                                              PCRatingSelectionMode mode, int currentNodeIndex, int selectedNodeIndex, bool isLowThenHigh)
{
    SvErrorInfo error = NULL;
    if (QBPCListIsRatingLocked(ratingsList, currentRating, &error))
        QBPCListUnlockRating(ratingsList, currentRating, &error);
    else
        QBPCListLockRating(ratingsList, currentRating, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal bool QBParentalControlMenuRatingChosen(QBParentalControlMenuHandler self, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    if (QBParentalControlGetState(self->appGlobals->pc) != QBParentalControlState_On && !self->allowModificationWhenOff) {
        return false;
    }

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString PCVal =  (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("PC"));
    SvArray enabledStandards = NULL;

    QBPCList currentList = QBParentalControlGetRating(self->appGlobals->pc);
    QBPCList ratings = QBPCListCreate(NULL);
    if (currentList) {
        SvString currentListRatings = QBPCListCreateLockedRatingsString(currentList, NULL);
        if (currentListRatings) {
            QBPCListLockRatingsFromString(ratings, currentListRatings, NULL);
            SVRELEASE(currentListRatings);
        }
    }

    if (PCVal && SvStringEqualToCString(PCVal, "CLEAR")) {
        QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
        QBParentalControlMenuClearRatingsFromNode(parentNode, ratings);

        // all ratings are cleared, so all standards are disabled
        enabledStandards = SvArrayCreate(NULL);
    } else {
        if (self->ratingsSelectionMode == PCRatingsSelectionMode_Group || (self->ratingsSelectionMode == PCRatingsSelectionMode_Selective
                && QBParentalControlMenuAreRatingsFromNodeCleared(QBActiveTreeNodeGetParentNode(node), ratings))) {
            // iterate nodes
            QBActiveTreeNode parentNode = QBActiveTreeNodeGetParentNode(node);
            SvIterator childNodesIterator = QBActiveTreeNodeChildNodesIterator(parentNode);
            QBActiveTreeNode currNode = NULL;
            while ((currNode = (QBActiveTreeNode) SvIteratorGetNext(&childNodesIterator))) {
                SvString currentNodeString = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) currNode, SVSTRING("PC"));
                int selectedNodeIndex = QBActiveTreeNodeGetChildNodeIndex(parentNode, node);
                int currentNodeIndex = QBActiveTreeNodeGetChildNodeIndex(parentNode, currNode);
                QBParentalControlMenuHandleMultipleRatingsSelection(ratings, self->ratingSelectionMode,
                                                                    currentNodeIndex, selectedNodeIndex,
                                                                    currentNodeString, QBParentalControlMenuHandleGroupMode,
                                                                    self->ratingsOrderMode == PCRatingsOrderMode_LowThenHigh);
            }
        } else if (self->ratingsSelectionMode == PCRatingsSelectionMode_Selective) {
            QBParentalControlMenuHandleMultipleRatingsSelection(ratings, self->ratingSelectionMode, 0, 0, PCVal,
                                                                QBParentalControlMenuHandleSelectiveMode,
                                                                self->ratingsOrderMode == PCRatingsOrderMode_LowThenHigh);
        }

        enabledStandards = QBParentalControlMenuCreateUsedStandardsList(QBActiveTreeNodeGetParentNode(node));
    }

    QBParentalControlLogicSelectedRatingsChanged(self->appGlobals->parentalControlLogic, ratings, enabledStandards);
    SVTESTRELEASE(enabledStandards);

    SvString newRatingsStr = QBPCListCreateLockedRatingsString(ratings, NULL);
    const char *currentRatingsStr = QBConfigGet("PC");

    if (!SvStringEqualToCString(newRatingsStr, currentRatingsStr)) {
        SvErrorInfo error = NULL;
        QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC"), &error);
        if (error) {
            error = SvErrorInfoCreateWithExtendedInfo(error, "QBAccessManagerInvalidateAuthentication failed");
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
        }
    }

    QBConfigSet("PC", SvStringCString(newRatingsStr));
    QBConfigSave();
    SVRELEASE(ratings);
    SVRELEASE(newRatingsStr);

    return true;
}

SvLocal bool QBParentalControlMenuLockingStateChanged(QBParentalControlMenuHandler self, SvGenericObject node_)
{
    bool updateNodes = false;

    SvString PC_STATE_Val =  (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("PC_STATE"));
    if (PC_STATE_Val) {
        QBParentalControlSetState(self->appGlobals->pc, PC_STATE_Val);
        updateNodes = true;
    }

    SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
    if (id && SvStringEqualToCString(id, "PCLockingStatusOffAuto")) {
        QBParentalControlMenuSetRelockTimeMenu(self);
    }

    return updateNodes;
}

// BEGIN AMERELES Bloquear títulos según control parental
SvLocal bool QBParentalControlMenuPCHideTitlesChanged(QBParentalControlMenuHandler self, SvGenericObject node_)
{
    bool updateNodes = false;

    SvString PC_HIDE_TITLES_Val = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("PC_HIDE_TITLES"));
    if (PC_HIDE_TITLES_Val)
    {
        QBConfigSet("PC_HIDE_TITLES", SvStringCString(PC_HIDE_TITLES_Val));
        updateNodes = true;
    }

    return updateNodes;
}
// END AMERELES Bloquear títulos según control parental

SvLocal bool QBParentalControlMenuLockedTimesChanged(QBParentalControlMenuHandler self, SvGenericObject node_)
{
    QBParentalControlLockedTimeTask task = NULL;
    if ((task = (QBParentalControlLockedTimeTask)SvHashTableFind(self->ltTasks, node_))) {
        task->state = !task->state;
        QBParentalControlAddAndSaveLockedTimeTask(self->appGlobals->pc, task);
    }
    return true;
}

SvLocal void QBParentalControlMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
        if (id && (SvStringEqualToCString(id, "PCChangePin") || (SvStringEqualToCString(id, "PCChangePurchasePin")))) {
            QBParentalControlMenuChangePIN(self, id);
            return;
        }

        if (!nodePath_) {
            QBActiveTreeFindNodeByID(self->appGlobals->menuTree, QBActiveTreeNodeGetID((QBActiveTreeNode) node_), &nodePath_);
        }

        size_t nodeDepth = nodePath_ ? SvInvokeInterface(QBTreePath, nodePath_, getLength) : 0;

        SvString rating = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("PC"));

        bool updateNodes = false;
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode)node_);
        QBActiveTreeNode directParent = parent;
        while (nodeDepth > self->depth + 2 && parent) {
            parent = QBActiveTreeNodeGetParentNode(parent);
            nodeDepth--;
        }
        if (!parent) {
            return;
        }

        SvString parentID = QBActiveTreeNodeGetID((QBActiveTreeNode)parent);
        if (!parentID) {
            return;
        }

        if (SvStringEqualToCString(parentID, "PCLockingStatus")) {
            updateNodes = QBParentalControlMenuLockingStateChanged(self, node_);
        } else if (SvStringEqualToCString(parentID, "PCHideTitles")) { // AMERELES Bloquear títulos según control parental
            updateNodes = QBParentalControlMenuPCHideTitlesChanged(self, node_);
        } else if (SvStringEqualToCString(parentID, "PCLockedTime")) {
            updateNodes = QBParentalControlMenuLockedTimesChanged(self, node_);
        } else if (QBParentalControlNodeContainsRatings(directParent) && rating) {
            updateNodes = QBParentalControlMenuRatingChosen(self, node_, nodePath_, position);
        }

        SvString optVal = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("configurationValue"));
        SvString optName = (parent) ? (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName")) : NULL;
        if (optVal && optName) {
            QBConfigSet(SvStringCString(optName), SvStringCString(optVal));
            QBConfigSave();
            updateNodes = true;
        }

        if (updateNodes) {
            SvGenericObject parentPath = SvObjectCopy(nodePath_, NULL);
            SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
            QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, parentPath, 0);
            size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
            QBActiveTreePropagateNodesChange(self->appGlobals->menuTree, parentPath, 0, nodesCount, NULL);
            SVRELEASE(parentPath);
        }
    } else if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        SvTVChannel tvChannel = (SvTVChannel) node_;
        if (SvTVChannelGetAttribute(tvChannel, SVSTRING("blocked"))) {
            SvTVChannelRemoveAttribute(tvChannel, SVSTRING("blocked"));
        } else {
            SvTVChannelSetAttribute(tvChannel, SVSTRING("blocked"), (SvObject) SVSTRING("blocked"));
            SvErrorInfo error = NULL;
            SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
            if (SvObjectEquals((SvObject) channel, (SvObject) tvChannel)) {
                QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC"), &error);
                if (error) {
                    error = SvErrorInfoCreateWithExtendedInfo(error, "QBAccessManagerInvalidateAuthentication failed");
                    SvErrorInfoWriteLogMessage(error);
                    SvErrorInfoDestroy(error);
                }
            }
        }

        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                              SVSTRING("SvEPGManager"));
        SvEPGManagerPropagateChannelChange(epgManager, tvChannel, NULL);

        size_t idx = SvInvokeInterface(SvEPGChannelView, self->tvChannels, getChannelIndex, tvChannel);
        SvEPGViewPropagateChannelsChange((SvEPGView) self->tvChannels, idx, 1);
    }
}

SvLocal void QBParentalControlMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;

    SvGenericObject node;
    if (!(node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath))) {
        return;
    }

    if (SvObjectIsInstanceOf(node, QBActiveTreeNode_getType())) {
        SvString nodeID = QBActiveTreeNodeGetID((QBActiveTreeNode)node);
        if (nodeID && !SvStringEqualToCString(nodeID, "PCLockedTime")) {
            QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode((QBActiveTreeNode)node);
            if (parent) {
                SvString parentID = QBActiveTreeNodeGetID((QBActiveTreeNode)parent);
                if (parentID && (SvStringEqualToCString(parentID, "PCLockedTime"))) {
                    QBParentalControlMenuSetActiveNode(self, (QBActiveTreeNode)node);
                    QBParentalControlMenuLockedTimeChoosen(self);
                }
            }
        }
    }
}

SvLocal void QBConfigurationMenuNodeSelectedPINCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBParentalControlMenuHandler self = self_;
    self->dialog = NULL;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        SvGenericObject path = NULL;

        SvString id = QBActiveTreeNodeGetID(self->nodeParentalControl);
        QBActiveTreeFindNodeByID(self->appGlobals->menuTree, id , &path);
        QBTreeIterator it = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);
        path = QBTreeIteratorGetCurrentNodePath(&it);
        if (path) {
            SvInvokeInterface(QBMenu, self->menuBar->prv, setPosition, path, NULL);
        }
        QBParentalControlMenuSetActiveNode(self, (QBActiveTreeNode) QBTreeIteratorGetNextNode(&it));

        // only the first level nodes are PIN protected so the PC_MENU authentication can be invalidated immediately
        QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC_MENU"), NULL);
    }
    SVTESTRELEASE(self->nodeParentalControl);
    SVTESTRELEASE(self->nodePathParentalControl);
    self->nodeParentalControl = NULL;
    self->nodePathParentalControl = NULL;
}

SvLocal bool QBParentalControlNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()) || self->dialog) {
        return false;
    }

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString parentalControl = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("ParentalControl"));
    if (parentalControl && SvStringEqualToCString(parentalControl,"yes")) {
        SVTESTRELEASE(self->nodeParentalControl);
        SVTESTRELEASE(self->nodePathParentalControl);
        self->nodeParentalControl = SVRETAIN(node);
        self->nodePathParentalControl = SVRETAIN(nodePath);
        QBParentalControlMenuCheckPin(self, QBConfigurationMenuNodeSelectedPINCallback);
        return true;
    }

    return false;
}

SvLocal void
QBParentalControlMainMenuChanged(SvGenericObject self_, QBMainMenuState state, SvGenericObject nodePath_)
{
    if (state != QBMainMenuState_Destroyed) {
        return;
    }

    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvGenericObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(self->appGlobals->menuTree, SVSTRING("STPC"), &path);
    if (path && node) {
        SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);
    }
}

SvLocal SvString
QBParentalControlGetHintsForPath(SvGenericObject self_, SvGenericObject path)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;

    QBActiveTreeNode currentNode = (QBActiveTreeNode) SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
    if (currentNode && SvObjectIsInstanceOf((SvObject) currentNode, QBActiveTreeNode_getType())) {
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(currentNode);
        if (parent) {
            SvString parentID = (SvString) QBActiveTreeNodeGetID(parent);
            if (parentID && (SvStringEqualToCString(parentID, "PCLockedTime"))) {
                return SVSTRING("lockedTime_hint");
            }
        }
    } else if (currentNode && SvObjectIsInstanceOf((SvObject) currentNode, SvTVChannel_getType())) {
        return SVSTRING("blockedChannels_hint");
    }
    return NULL;
}

SvLocal void QBParentalControlSelectChannel(SvGenericObject self_, SvTVChannel channel)
{
    QBParentalControlMenuHandler self = (QBParentalControlMenuHandler) self_;
    SvGenericObject nodePath_ = NULL;
    QBActiveTreeFindNodeByID(self->appGlobals->menuTree, SVSTRING("PCBlockedChannels"), &nodePath_);
    if (!nodePath_)
        return;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->channelProxy, getIterator, NULL, 0);
    SvGenericObject node = NULL;
    int position = 0;
    while ((node = QBTreeIteratorGetNextNode(&iter))) {
        if (SvObjectIsInstanceOf(node, SvTVChannel_getType()) && channel == (SvTVChannel) node)
            break;
        position++;
    }

    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvInvokeInterface(QBMenu, menuBar->prv, setSubmenuPosition, nodePath_, position, false);
}

SvLocal SvString QBParentalControlGetPlaylistID(SvGenericObject self_)
{
    return SVSTRING("TVChannels");
}

SvLocal SvType QBParentalControlMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBParentalControlMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBParentalControlMenuChoosen
    };
    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBParentalControlMenuContextChoosen
    };
    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBParentalControlNodeSelected
    };
    static const struct QBMainMenuChanges_t mainMenuMethods = {
        .changed = QBParentalControlMainMenuChanged
    };
    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBParentalControlGetHintsForPath
    };
    static const struct QBChannelMenu_ channelMenuMethods = {
        .selectChannel = QBParentalControlSelectChannel,
        .getPlaylistID = QBParentalControlGetPlaylistID
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBParentalControlMenuHandler",
                            sizeof(struct QBParentalControlMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            QBMainMenuChanges_getInterface(), &mainMenuMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            QBChannelMenu_getInterface(), &channelMenuMethods,
                            NULL);
    }

    return type;
}

void QBParentalControlMenuUpdateLockedTimeTask(QBParentalControlMenuHandler self, QBParentalControlLockedTimeTask srcTask, QBActiveTreeNode node)
{
    if (!self || !node || !self->ltTasks || !srcTask) {
        SvLogError("[%s] Cannot update active node(self:%p, node:%p, ltTasks:%p, task:%p)",
            __func__, self, node, (self ? self->ltTasks : NULL), srcTask);
        return;
    }

    QBParentalControlLockedTimeTask task = NULL;
    if ( (task = (QBParentalControlLockedTimeTask)SvHashTableFind(self->ltTasks, (SvGenericObject)node)) ) {
        task->startTime = srcTask->startTime;
        task->endTime = srcTask->endTime;
        task->state = srcTask->state;
    } else {
        SvHashTableInsert(self->ltTasks, (SvGenericObject)node, (SvGenericObject)srcTask);
    }
}

SvLocal void QBParentalControlMenuSetActiveNode(QBParentalControlMenuHandler self, QBActiveTreeNode node)
{
    if (!self) {
        SvLogError("[%s] Error(self:%p)", __func__, self);
        return;
    }
    SVTESTRELEASE(self->activeNode);
    self->activeNode = SVTESTRETAIN(node);
}

void QBParentalControlMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvGenericObject path = NULL;
    QBActiveTreeNode node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("PCBlockedChannels"), &path);

    SvGenericObject ratingsPath = NULL;
    QBActiveTreeNode ratingsNode = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("PCLockedRatings"), &ratingsPath);
    if (!path || !ratingsPath) {
        return;
    }


    QBParentalControlMenuHandler handler = (QBParentalControlMenuHandler)
            SvTypeAllocateInstance(QBParentalControlMenuHandler_getType(), NULL);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    handler->tvChannels = SVRETAIN(QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels")));
    handler->appGlobals = appGlobals;
    handler->menuBar = menuBar;
    handler->ltTasks = SvHashTableCreate(10, NULL);

    svSettingsPushComponent("ParentalControl.settings");
    int autoRelockTime = svSettingsGetInteger("RelockTime", "defaultAutoRelockTime", 1);
    QBParentalControlSetAutoRelockTime(appGlobals->pc, autoRelockTime);
    svSettingsPopComponent();

    handler->channelProxy = QBTreeProxyCreate((SvGenericObject) handler->tvChannels, (SvGenericObject) node, NULL);
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvGenericObject) handler->channelProxy, path, NULL);

    QBMainMenuAddManualChannelSelectionListener(appGlobals->main, (SvGenericObject) handler, (SvGenericObject) node);

    handler->ratingsSelectionMode = PCRatingsSelectionMode_Group;
    SvString ratingsSelectionModeStr = (SvString) QBActiveTreeNodeGetAttribute(ratingsNode, SVSTRING("PCRatingsSelectionMode"));
    if (ratingsSelectionModeStr && SvStringEqualToCString(ratingsSelectionModeStr,"Selective")) {
        handler->ratingsSelectionMode = PCRatingsSelectionMode_Selective;
    }

    handler->ratingSelectionMode = PCRatingSelectionMode_Unlocking;
    SvString ratingSelectionModeStr = (SvString) QBActiveTreeNodeGetAttribute(ratingsNode, SVSTRING("PCRatingSelectionMode"));
    if (ratingSelectionModeStr && SvStringEqualToCString(ratingSelectionModeStr, "Locking")) {
        handler->ratingSelectionMode = PCRatingSelectionMode_Locking;
    }

    handler->ratingsOrderMode = PCRatingsOrderMode_LowThenHigh;
    SvString ratingsOrderModeStr = (SvString) QBActiveTreeNodeGetAttribute(ratingsNode, SVSTRING("PCRatingsOrderMode"));
    if (ratingsOrderModeStr && SvStringEqualToCString(ratingsOrderModeStr, "HighThenLow")) {
        handler->ratingsOrderMode = PCRatingsOrderMode_HighThenLow;
    }

    handler->allowModificationWhenOff = false;
    SvValue allowModificationWhenOffVal = (SvValue) QBActiveTreeNodeGetAttribute(ratingsNode, SVSTRING("AllowModificationsWhenOff"));
    if (allowModificationWhenOffVal && SvValueIsBoolean(allowModificationWhenOffVal)) {
        handler->allowModificationWhenOff = SvValueGetBoolean(allowModificationWhenOffVal);
    }


    static struct QBChannelChoiceControllerCallbacks_t channelChoice = {
        .isTicked = QBParentalControlChannelIsTicked,
        .createNodeCaption = QBParentalControlCreateCaption
    };
    QBChannelChoiceController channelController = QBChannelChoiceControllerCreateFromSettings("ChannelBlockMenu.settings", handler->appGlobals->textRenderer, handler->tvChannels, handler, &channelChoice);

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) channelController, NULL);
    SVRELEASE(channelController);

    path = NULL;
    QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("STPC"), &path);
    if (path) {
        handler->depth = SvInvokeInterface(QBTreePath, path, getLength);
        SvGenericObject mainController = QBParentalControlItemControllerCreate(appGlobals->textRenderer, appGlobals->pc, appGlobals->menuTree, handler->depth);
        SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) mainController, NULL);
        QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
        SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) handler, NULL);
        SVRELEASE(handler);
        SVRELEASE(mainController);
    }
    SvArray tasks = QBParentalControlGetLockedTimeTasks(appGlobals->pc);
    if (tasks && SvArrayCount(tasks) > 0) {
        SvIterator it = SvArrayIterator(tasks);
        QBParentalControlLockedTimeTask task = NULL;
        while ((task = (QBParentalControlLockedTimeTask) SvIteratorGetNext(&it))) {
            SvGenericObject ret = QBParentalControlMenuCreateLockedTimeNode(appGlobals, task);
            if (ret) {
                SvHashTableInsert(handler->ltTasks, ret, (SvGenericObject)task);
                SVRELEASE(ret);
            }
        }
    } else {
        QBParentalControlMenuLockedTimeAddEmptyNode(appGlobals);
    }
}
