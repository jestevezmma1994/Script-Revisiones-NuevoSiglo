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

#include "QBNPvrMenu.h"

#include <Services/npvrAgent.h>
#include "menuchoice.h"

#include <SWL/label.h>
#include <QBOSK/QBOSKKey.h>
#include <QBOSK/QBOSK.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputCodes.h>
#include <TranslationMerger.h>

#include <main.h>
#include <Windows/pvrplayer.h>
#include <Windows/mainmenu.h>
#include <Windows/newtvguide.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBNPvrOptionsSelectPane.h>
#include <ContextMenus/QBRecordingEditPane.h>
#include <Utils/authenticators.h>
#include <Utils/QBNPvrUtils.h>
#include <Utils/QBEventUtils.h>
#include <Utils/QBContentInfo.h>

#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/core/appState.h>

#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>

#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/npvrDialog.h>
#include <QBWidgets/QBComboBox.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>

#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRQuota.h>
#include <SvPlayerKit/SvEPGEvent.h>

#include <CubiwareMWClient/NPvr/CubiwareMWNPvrProvider.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>

#include <Logic/NPvrLogic.h>
#include <Logic/timeFormat.h>

#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>

#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvLocalTime.h>
#include <libintl.h>

#include <Hints/hintsGetter.h>

#define log_debug(fmt, ...)  do { if(1) SvLogNotice(COLBEG() "NPvrMenu :: " fmt  COLEND_COL(blue), ##__VA_ARGS__); } while(0)

#define SvObj  SvGenericObject
#define lli  long long int

#define MAX_PVR_DIRS_COUNT  20
#define MAX_QUOTA_GB        999
#define MAX_QUOTA_HOURS     99


typedef enum {
    QBNPvrRecordingBitmap_Scheduled,
    QBNPvrRecordingBitmap_Completed,
    QBNPvrRecordingBitmap_CompletedPadlock,
    QBNPvrRecordingBitmap_Recording,
    QBNPvrRecordingBitmap_Failed,
    QBNPvrRecordingBitmap_Removed,
    QBNPvrRecordingBitmap_RemovedPadlock,
    QBNPvrRecordingBitmap_Count
} QBNPvrRecordingBitmap;

typedef enum {
    QBNPvrDirectoryNameValidatorStatus_OK,
    QBNPvrDirectoryNameValidatorStatus_EMPTY,
    QBNPvrDirectoryNameValidatorStatus_EXISTS
} QBNPvrDirectoryNameValidatorStatus;

typedef enum {
    QBNPvrMenuActiveOSK_CreateDirectory,
    QBNPvrMenuActiveOSK_RenameDirectory
} QBNPvrMenuActiveOSK;

struct QBNPvrMenuHandler_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTree menuTree;

    QBnPVRRecording currentRecording;
    QBnPVRDirectory currentDir;

    struct {
        QBContextMenu ctx;
    } sidemenu;

    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;

    QBXMBItemConstructor bigItemConstructor;
    QBFrameConstructor* bigFocus;
    QBFrameConstructor* bigInactiveFocus;

    SvRID bitmapRIDs[QBNPvrRecordingBitmap_Count];

    SvWidget quotaEdit;
    SvWidget quota;
    QBContainerPane quotaPane;

    QBBasicPane options;
    QBBasicPaneItem quotaItem;

    QBNPvrOptionsSelectPane recordingSpaceRecoveryModePane;

    QBBasicPane dirPreferencesOptions;
    struct {
        QBBasicPaneItem premiereOption;
        QBBasicPaneItem spaceRecoveryOption;
    } dirPreferencesItems;
    QBNPvrOptionsSelectPane dirPremierePane;
    QBNPvrOptionsSelectPane dirSpaceRecoveryPane;
    struct {
        QBnPVRRecordingSpaceRecovery spaceRecovery;
        QBnPVRRecordingSchedulePremiere premiere;
    } dirPreferencesInfo;

    QBNPvrMenuActiveOSK activeOSK;

    SvWidget parentalPopup;
    bool playFromBeginning;
    bool sideMenuShown;

    SvWeakReference currentRecShown;    ///< npvr recording reference for which extended info is currently displayed
                                        ///< note that currentRecShown can be set but not visible if side menu is shown
    SvWidget extendedInfoLabel;         ///< extended info widget attached to main menu window
};
typedef struct QBNPvrMenuHandler_s *QBNPvrMenuHandler;

SvLocal void
QBNPvrMenuContextChangeQuota(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item) __attribute__ ((unused));

SvLocal void
QBNPvrMenuHandlerDestroy(void *self_)
{
    QBNPvrMenuHandler self = self_;

    SVRELEASE(self->menuTree);
    SVTESTRELEASE(self->currentRecording);
    SVTESTRELEASE(self->currentRecShown);

    SVTESTRELEASE(self->sidemenu.ctx);
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->bigItemConstructor);
    SVRELEASE(self->bigFocus);
    SVTESTRELEASE(self->bigInactiveFocus);

    SVTESTRELEASE(self->options);
    SVTESTRELEASE(self->currentDir);

    SVTESTRELEASE(self->recordingSpaceRecoveryModePane);

    SVTESTRELEASE(self->dirPreferencesOptions);
    SVTESTRELEASE(self->dirPremierePane);
    SVTESTRELEASE(self->dirSpaceRecoveryPane);

    if (self->appGlobals->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, removeDirectoryInfoListener, (SvGenericObject) self, NULL);
}

SvLocal void
QBNPvrMenuRecordingExtendedInfoShow(QBNPvrMenuHandler self, QBnPVRRecording recording)
{
    if (!self->extendedInfoLabel) {
        svSettingsPushComponent("npvrExtendedInfo.settings");
        self->extendedInfoLabel = QBAsyncLabelNew(self->appGlobals->res, "npvrExtendedInfo", self->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->appGlobals->main->window, self->extendedInfoLabel, "npvrExtendedInfo", 1);
        svSettingsPopComponent();
    } else {
        // due to different performance of scrolling up/down extendedInfo widget
        // should be destroyed first to avoid faster/slower text changes
        SvLogWarning("%s():%d Extended info should be hidden first!", __func__, __LINE__);
    }

    SvString extendedInformation = NULL;
    SvString durationTime = SvEPGEventGetDurationTime(recording->event, QBTimeFormatGetCurrent()->gridEPGTime);
    SvString description = NULL;

    SvStringBuffer buffer = SvStringBufferCreate(NULL);
    const char* categoryName = SvEPGEventGetCategoryName(recording->event);
    if (categoryName) {
        SvStringBufferAppendFormatted(buffer, NULL, "%s\n", gettext(categoryName));
    }

    SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, recording->event);
    if (desc) {
        SvString descStr = desc->description ? desc->description : desc->shortDescription;
        if (descStr)
            SvStringBufferAppendFormatted(buffer, NULL, "%s", SvStringCString(descStr));
    }

    description = SvStringBufferCreateContentsString(buffer, NULL);
    SVRELEASE(buffer);

    SvStringBuffer extendedInformationBuff = SvStringBufferCreate(NULL);
    if (desc && desc->title)
        SvStringBufferAppendFormatted(extendedInformationBuff, NULL, "%s ", SvStringCString(desc->title));
    if (durationTime)
        SvStringBufferAppendFormatted(extendedInformationBuff, NULL, "\n%s", SvStringCString(durationTime));
    if (description)
        SvStringBufferAppendFormatted(extendedInformationBuff, NULL, "\n%s", SvStringCString(description));

    extendedInformation = SvStringBufferCreateContentsString(extendedInformationBuff, NULL);
    SVRELEASE(extendedInformationBuff);

    QBAsyncLabelSetText(self->extendedInfoLabel, extendedInformation);
    SVRELEASE(extendedInformation);

    SVTESTRELEASE(durationTime);
    SVTESTRELEASE(description);
}

SvLocal void
QBNPvrMenuRecordingExtendedInfoHide(QBNPvrMenuHandler self)
{
    if (!self->extendedInfoLabel) {
        SvLogWarning("%s():%d Can't hide npvr extended info - not created", __func__, __LINE__);
        return;
    }
    QBAsyncLabelClear(self->extendedInfoLabel);
    svWidgetDetach(self->extendedInfoLabel);
    svWidgetDestroy(self->extendedInfoLabel);
    self->extendedInfoLabel = NULL;
}

SvLocal bool
QBNPvrMenuRecordingExtendedInfoIsVisible(QBNPvrMenuHandler self)
{
    return self->extendedInfoLabel;
}

SvLocal void
QBNPvrMenuContextMenuHide(QBNPvrMenuHandler self)
{
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal SvString
create_bookmark_id(const QBnPVRRecording rec)
{
    return SvStringCreateWithFormat("NPvr:%s", SvStringCString(rec->id));
}

SvLocal bool
start_playback(QBNPvrMenuHandler self, const QBnPVRRecording rec, bool authenticated)
{
    SvContent content = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, createContent, rec);
    if (!content) {
        SvLogError("QBnPVRProvider::createContent -> failed");
        return false;
    };

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);

    if (SvObjectIsInstanceOf(self->appGlobals->nPVRProvider, CubiwareMWNPvrProvider_getType())) {
        QBAnyPlayerLogicSetVodId(anyPlayerLogic, rec->id);
        SvValue productID = SvValueCreateWithString(rec->id, NULL);
        QBContentInfo contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvGenericObject) productID, NULL, self->appGlobals->nPVRProvider, NULL);
        SVRELEASE(productID);
        SvGenericObject controller = (SvGenericObject) QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
        SVRELEASE(contentInfo);
        SvInvokeInterface(QBAnyPlayerController, controller, setContent, (SvGenericObject) content);
        QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
        SVRELEASE(controller);
    }

    SvString bookmarkId = create_bookmark_id(rec);
    if (bookmarkId && self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, bookmarkId);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, bookmarkId, 0.0, QBBookmarkType_Generic);

        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
        if (self->playFromBeginning)
            QBAnyPlayerLogicSetStartPosition(anyPlayerLogic, 0.0);
    }
    SVTESTRELEASE(bookmarkId);

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);
    QBPVRPlayerContextSetMetaDataFromNPvrRecording(pvrPlayer, rec);
    SvString serviceName = SvStringCreate(gettext("PVR"), NULL);
    QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
    SVRELEASE(serviceName);
    SVRELEASE(anyPlayerLogic);

    QBPVRPlayerContextSetContent(pvrPlayer, content);
    SVRELEASE(content);
    if (authenticated)
        QBPVRPlayerContextSetAuthenticated(pvrPlayer);

    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);

    return true;
}

SvLocal void
QBNPvrMenuContextCheckParentalControlPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBNPvrMenuHandler self = ptr;
    if (!self->parentalPopup) {
        return;
    }

    self->parentalPopup = NULL;

    if (ret && SvStringEqualToCString(ret, "OK-button") && self->currentRecording) {
        start_playback(self, self->currentRecording, true);
        QBNPvrMenuContextMenuHide(self);
    }
}

SvLocal void
QBNPvrMenuContextCheckParentalControlPIN(QBNPvrMenuHandler self)
{
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC"));
    SvWidget master = NULL;
    svSettingsPushComponent("ParentalControl.settings");
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, &master);
    svSettingsPopComponent();

    self->parentalPopup = dialog;
    QBDialogRun(dialog, self, QBNPvrMenuContextCheckParentalControlPINCallback);
}

SvLocal void
QBNPvrMenuContextMenuPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (self->currentRecording) {
        self->playFromBeginning = id && SvStringEqualToCString(id, "playFromTheBegining");
        if (!QBParentalControlEventShouldBeBlocked(self->appGlobals->pc, self->currentRecording->event)) {
            start_playback(self, self->currentRecording, false);
            QBNPvrMenuContextMenuHide(self);
        } else {
            QBNPvrMenuContextCheckParentalControlPIN(self);
        }
    } else {
        QBNPvrMenuContextMenuHide(self);
    }
}

SvLocal void
QBNPvrMenuContextMenuDelete(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    QBnPVRProviderRequest request = NULL;
    if (SvStringEqualToCString(id, "OK") && self->currentRecording) {
        SvArray recordings = SvArrayCreate(NULL);
        SvArrayAddObject(recordings, (SvGenericObject) self->currentRecording);
        request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteRecordings, recordings);
        SVRELEASE(recordings);
    }

    QBNPvrDialogCreateAndShow(self->appGlobals, request);

    QBNPvrMenuContextMenuHide(self);
}

SvLocal void delete_all(QBNPvrMenuHandler self, QBnPVRRecordingState state)
{
    QBnPVRProviderRequest request = NULL;
    SvArray states = SvArrayCreate(NULL);
    SvArrayAddObject(states, (SvGenericObject) QBnPVRProviderTranslateRecordingStateToString(state));
    if (state == QBnPVRRecordingState_active) {
        // delete ongoing recordings from all directories -> dir->id = NULL
        request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteAllInStatesFromDirectory, NULL, states);
    } else {
        request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteAllInStatesFromDirectory, QBnPVRProviderGetGlobalDirectoryId(), states);
    }
    SVRELEASE(states);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
}

SvLocal void
QBNPvrMenuContextMenuDeleteScheduled(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        delete_all(self, QBnPVRRecordingState_scheduled);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextMenuDeleteActive(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        delete_all(self, QBnPVRRecordingState_active);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextMenuDeleteCompleted(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        delete_all(self, QBnPVRRecordingState_completed);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextMenuDeleteAllFromDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK") && self->currentRecording) {
        SvArray states = SvArrayCreate(NULL);
        SvArrayAddObject(states, (SvGenericObject) QBnPVRProviderTranslateRecordingStateToString(QBnPVRRecordingState_completed));
        SvArrayAddObject(states, (SvGenericObject) QBnPVRProviderTranslateRecordingStateToString(QBnPVRRecordingState_scheduled));
        SvArrayAddObject(states, (SvGenericObject) QBnPVRProviderTranslateRecordingStateToString(QBnPVRRecordingState_active));
        QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteAllInStatesFromDirectory,
                                                          self->currentRecording->directoryId, states);
        SVRELEASE(states);

        QBNPvrDialogCreateAndShow(self->appGlobals, request);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextMenuDeleteFailed(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        delete_all(self, QBnPVRRecordingState_failed);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextCreateDirectory(QBNPvrMenuHandler self, SvString name)
{
    QBnPVRDirectoryCreateParams_ params = {0};
    params.name = name;

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, createDirectory, &params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);
}

SvLocal void
QBNPvrMenuContextDeleteDirectory(QBNPvrMenuHandler self, bool keepContents)
{
    QBnPVRDirectoryDeleteParams_ params = {
        .id = self->currentDir->id,
        .keepContents = keepContents,
    };

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteDirectory, &params);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
}

SvLocal void
QBNPvrMenuContextRenameDirectory(QBNPvrMenuHandler self, SvString name)
{
    QBnPVRDirectoryUpdateParams_ params = QBnPVRDirectoryGetEmptyUpdateParams();
    params.id = self->currentDir->id;
    params.name = name;

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, updateDirectory, &params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);
}

SvLocal void
QBNPvrMenuContextDeleteKeywordRec(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (!SvStringEqualToCString(id, "OK")) {
        goto fini;
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteKeyword, self->currentDir->keywordId);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);

fini:
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextDeleteSeriesRec(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (!SvStringEqualToCString(id, "OK")) {
        goto fini;
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteSeries, self->currentDir->seriesId);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);

fini:
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextDeleteDirectoryWithContents(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (!SvStringEqualToCString(id, "OK")) {
        goto fini;
    }

    if (self->currentDir->keywordId)
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteKeyword, self->currentDir->keywordId);
    if (self->currentDir->seriesId)
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteSeries, self->currentDir->seriesId);
    QBNPvrMenuContextDeleteDirectory(self, false);

fini:
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrRecordingEditPaneUpdateItems(QBNPvrMenuHandler self)
{
    if (QBNPvrLogicIsPremiereFeatureEnabled()) {
        SvString premiereOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_Premiere, (int) self->dirPreferencesInfo.premiere);

        if (!SvObjectEquals((SvObject) premiereOptionStr, (SvObject) self->dirPreferencesItems.premiereOption->subcaption)) {
            SVTESTRELEASE(self->dirPreferencesItems.premiereOption->subcaption);
            self->dirPreferencesItems.premiereOption->subcaption = SVRETAIN(premiereOptionStr);
            QBBasicPaneOptionPropagateObjectChange(self->dirPreferencesOptions, self->dirPreferencesItems.premiereOption);
        }
        SVRELEASE(premiereOptionStr);
    }

    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        SvString recoveryOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_SpaceRecovery, (int) self->dirPreferencesInfo.spaceRecovery);

        if (!SvObjectEquals((SvObject) recoveryOptionStr, (SvObject) self->dirPreferencesItems.spaceRecoveryOption->subcaption)) {
            SVTESTRELEASE(self->dirPreferencesItems.spaceRecoveryOption->subcaption);
            self->dirPreferencesItems.spaceRecoveryOption->subcaption = SVRETAIN(recoveryOptionStr);
            QBBasicPaneOptionPropagateObjectChange(self->dirPreferencesOptions, self->dirPreferencesItems.spaceRecoveryOption);
        }
        SVRELEASE(recoveryOptionStr);
    }
}

SvLocal void QBNPvrMenuContextDirPremiereApproveCallback(void* self_, int optionValue)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;
    self->dirPreferencesInfo.premiere = (optionValue != 0);
    QBNPvrRecordingEditPaneUpdateItems(self);
}

SvLocal void
QBNPvrMenuContextDirCreatePremierePane(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    if (!self->dirPremierePane) {
        self->dirPremierePane = QBNPvrOptionsSelectPaneCreate(self->appGlobals,
                                                              self->sidemenu.ctx,
                                                              3,
                                                              QBNPvrOptionsSelectPaneType_Premiere);
        QBNPvrOptionsSelectPaneSetApproveCallback(self->dirPremierePane, QBNPvrMenuContextDirPremiereApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->dirPremierePane, (int) self->currentDir->schedulePremiereOption);
}

SvLocal void QBNPvrMenuContextDirSpaceRecoveryApproveCallback(void* self_, int optionValue)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;
    if (optionValue == QBnPVRRecordingSpaceRecovery_DeleteOldest) {
        self->dirPreferencesInfo.spaceRecovery = QBnPVRRecordingSpaceRecovery_DeleteOldest;
    } else if (optionValue == QBnPVRRecordingSpaceRecovery_Manual) {
        self->dirPreferencesInfo.spaceRecovery = QBnPVRRecordingSpaceRecovery_Manual;
    }
    QBNPvrRecordingEditPaneUpdateItems(self);
}

SvLocal void
QBNPvrMenuContextDirCreateSpaceRecoveryPane(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    if (!self->dirSpaceRecoveryPane) {
        self->dirSpaceRecoveryPane = QBNPvrOptionsSelectPaneCreate(self->appGlobals,
                                                                   self->sidemenu.ctx,
                                                                   3,
                                                                   QBNPvrOptionsSelectPaneType_SpaceRecovery);
        QBNPvrOptionsSelectPaneSetApproveCallback(self->dirSpaceRecoveryPane, QBNPvrMenuContextDirSpaceRecoveryApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->dirSpaceRecoveryPane, (int) self->currentDir->scheduleRecoveryOption);
}

SvLocal void
QBNPvrMenuContextDirPreferencesSave(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    QBnPVRDirectory dir = self->currentDir;

    struct QBnPVRRecordingScheduleUpdateParams_ params = {
        .recoveryOption = NULL,
        .recordPremieresOnly = NULL,
    };

    SvString scheduleId = NULL;
    if (dir->type == QBnPVRDirectoryType_Keyword) {
        scheduleId = dir->keywordId;
    } else if (dir->type == QBnPVRDirectoryType_Series) {
        scheduleId = dir->seriesId;
    }
    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        params.recoveryOption = SvValueCreateWithInteger(self->dirPreferencesInfo.spaceRecovery, NULL);
    }
    if (QBNPvrLogicIsPremiereFeatureEnabled()) {
        params.recordPremieresOnly = SvValueCreateWithBoolean(self->dirPreferencesInfo.premiere, NULL);
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, updateSchedule, scheduleId, &params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);

    SVTESTRELEASE(params.recoveryOption);
    SVTESTRELEASE(params.recordPremieresOnly);

    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBNPvrMenuContextDirPreferences(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane dirPreferencesOptions = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(dirPreferencesOptions, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));
    SVTESTRELEASE(self->dirPreferencesOptions);
    self->dirPreferencesOptions = dirPreferencesOptions;

    SvString name = NULL;

    if (QBNPvrLogicIsPremiereFeatureEnabled()) {
        name = SvStringCreate(gettext("Premiere"), NULL);
        self->dirPreferencesItems.premiereOption = QBBasicPaneAddOption(self->dirPreferencesOptions, SVSTRING("premiere"), name, QBNPvrMenuContextDirCreatePremierePane, self);
        SVRELEASE(name);
        self->dirPreferencesInfo.premiere = self->currentDir->schedulePremiereOption;
    }

    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        name = SvStringCreate(gettext("Space recovery"), NULL);
        self->dirPreferencesItems.spaceRecoveryOption = QBBasicPaneAddOption(self->dirPreferencesOptions, SVSTRING("spaceRecovery"), name, QBNPvrMenuContextDirCreateSpaceRecoveryPane, self);
        SVRELEASE(name);
        self->dirPreferencesInfo.spaceRecovery = self->currentDir->scheduleRecoveryOption;
    }

    name = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(self->dirPreferencesOptions, SVSTRING("save"), name, QBNPvrMenuContextDirPreferencesSave, self);
    SVRELEASE(name);

    QBNPvrRecordingEditPaneUpdateItems(self);
    QBBasicPaneSetPosition(self->dirPreferencesOptions, SVSTRING("save"), true);

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->dirPreferencesOptions);
    svSettingsPopComponent();
}

SvLocal void
QBNPvrMenuContextSetQuota(QBNPvrMenuHandler self, int newQuota)
{
    QBnPVRProviderQuotaUpdate params = {
        .maxBytes = 0,
        .maxSeconds = 0,
    };

    QBnPVRProviderCapabilities caps = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getCapabilities);

    if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_bytes) {
        params.maxBytes = newQuota;
    } else if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_seconds) {
        params.maxSeconds = newQuota * 60 * 60;
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, setQuota,
                                                      self->currentDir->id, params, QBnPVRQuotaPolicy_deleteNone);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
}

SvLocal QBNPvrDirectoryNameValidatorStatus
QBNPvrMenuContextMenuValidateDirectoryName(QBNPvrMenuHandler self, SvString name)
{
    if (SvStringLength(name) == 0) {
        return QBNPvrDirectoryNameValidatorStatus_EMPTY;
    }

    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);

    const char* nameCStr = SvStringCString(name);
    SvIterator it = SvArrayIterator(dirList);
    QBnPVRDirectory dir = NULL;
    while ((dir = (QBnPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(dir->name, nameCStr)) {
            SVRELEASE(dirList);
            return QBNPvrDirectoryNameValidatorStatus_EXISTS;
        }
    }

    SVRELEASE(dirList);
    return QBNPvrDirectoryNameValidatorStatus_OK;
}

SvLocal void
QBNPvrMenuContextMenuDirectoryNameKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBNPvrMenuHandler self = ptr;
    if (key->type == QBOSKKeyType_enter) {
        SvWidget title = QBOSKPaneGetTitle(pane);
        switch (QBNPvrMenuContextMenuValidateDirectoryName(self, input)) {
            case QBNPvrDirectoryNameValidatorStatus_OK:
                svLabelSetText(title, "");
                if (self->activeOSK == QBNPvrMenuActiveOSK_CreateDirectory) {
                    QBNPvrMenuContextCreateDirectory(self, input);
                } else if (self->activeOSK == QBNPvrMenuActiveOSK_RenameDirectory) {
                    QBNPvrMenuContextRenameDirectory(self, input);
                }
                QBContextMenuHide(self->sidemenu.ctx, false);
                break;
            case QBNPvrDirectoryNameValidatorStatus_EMPTY:
                svLabelSetText(title, gettext("Directory name cannot be empty"));
                break;
            case QBNPvrDirectoryNameValidatorStatus_EXISTS:
                svLabelSetText(title, gettext("Directory name already exists"));
                break;
        }
    }
}

SvLocal void
QBNPvrMenuContextCreateOSKPane(QBNPvrMenuHandler self, int level)
{
    svSettingsPushComponent("PVRDirectoryOSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, level, SVSTRING("OSKPane"),
                  QBNPvrMenuContextMenuDirectoryNameKeyTyped, self, &error);
    svSettingsPopComponent();
    if (!error) {
        if (self->activeOSK == QBNPvrMenuActiveOSK_RenameDirectory) {
            QBOSKPaneSetInput(oskPane, self->currentDir->name);
        }

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void QBNPvrMenuShowPaneDirsLimitBack(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    QBContextMenuPopPane(self->sidemenu.ctx);
}

SvLocal void
QBNPvrMenuShowPaneDirsLimit(QBNPvrMenuHandler self, int level)
{
    const char *msg = gettext("You have too many directories.");
    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, level, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(msg, NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBNPvrMenuShowPaneDirsLimitBack, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
    QBContextMenuShow(self->sidemenu.ctx);
}

SvLocal bool
QBNPvrMenuDirsLimitReached(QBNPvrMenuHandler self)
{
    int dirsCount = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoriesCount);
    return dirsCount < MAX_PVR_DIRS_COUNT;
}

SvLocal void
QBNPvrMenuContextCreateDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    int currentPaneLevel = QBBasicPaneGetLevel(pane);

    if (QBNPvrMenuDirsLimitReached(self)) {
        QBNPvrMenuShowPaneDirsLimit(self, currentPaneLevel + 1);
    } else {
        self->activeOSK = QBNPvrMenuActiveOSK_CreateDirectory;
        QBNPvrMenuContextCreateOSKPane(self, currentPaneLevel + 1);
    }
}

SvLocal void
QBNPvrMenuContextRenameDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    int currentPaneLevel = QBBasicPaneGetLevel(pane);

    self->activeOSK = QBNPvrMenuActiveOSK_RenameDirectory;
    QBNPvrMenuContextCreateOSKPane(self, currentPaneLevel + 1);
}

SvLocal void
QBNPvrMenuContextDeleteDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        QBNPvrMenuContextDeleteDirectory(self, false);
    }
    QBNPvrMenuContextMenuHide(self);
}

SvLocal void
QBNPvrMenuContextDeleteDirKeepContents(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (SvStringEqualToCString(id, "OK")) {
        QBNPvrMenuContextDeleteDirectory(self, true);
    }
    QBNPvrMenuContextMenuHide(self);
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
// Quota

struct QBQuotaInput_t {
    SvWidget w;
    SvWidget comboBox;
    int quotaStep;
    int inputPos;
    int maxQuota;
    QBnPVRQuotaUnits directoryQuotaUnits;
};

typedef struct QBQuotaInput_t *QBQuotaInput;

SvLocal SvString
QBQuotaInputComboBoxPrepareQuota(void *self_, SvWidget combobox, SvGenericObject value)
{
    QBQuotaInput self = self_;
    int quota = SvValueGetInteger((SvValue) value);

    if (!quota || quota >= self->maxQuota) {
        return SVSTRING(gettext_noop("unlimited"));
    }

    if (self->directoryQuotaUnits == QBnPVRQuotaUnits_bytes) {
        return SvStringCreateWithFormat(gettext("%iGB"), quota);
    } else if (self->directoryQuotaUnits == QBnPVRQuotaUnits_seconds) {
        return SvStringCreateWithFormat(gettext("%ih"), quota);
    }

    return SVSTRING(gettext_noop("unlimited"));
}

SvLocal bool
QBQuotaInputComboBoxInputQuota(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBQuotaInput self = target;
    if (key < '0' || key > '9') {
        return false;
    }
    int digit = key - '0';

    int QuotaMax = 0;
    if (self->directoryQuotaUnits == QBnPVRQuotaUnits_bytes) {
        QuotaMax = MAX_QUOTA_GB;
    } else if (self->directoryQuotaUnits == QBnPVRQuotaUnits_seconds) {
        QuotaMax = MAX_QUOTA_HOURS;
    }

    SvValue value = (SvValue) QBComboBoxGetValue(combobox);
    int QuotaValue = self->inputPos ? SvValueGetInteger(value) : 0;
    if (QuotaValue > QuotaMax / 10) {
        QuotaValue = 0;
    }
    QuotaValue = QuotaValue * 10 + digit;
    self->inputPos = 1;

    value = SvValueCreateWithInteger(QuotaValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal int
QBQuotaInputComboBoxChangeQuota(void * target, SvWidget combobox, SvGenericObject value, int key)
{
    QBQuotaInput self = target;
    self->inputPos = 0;
    int Quota = SvValueGetInteger((SvValue) value);
    Quota += key == QBKEY_RIGHT ? self->quotaStep : -1;
    return Quota / self->quotaStep;
}

SvLocal void
QBQuotaInputComboBoxCurrentQuota(void *target, SvWidget combobox, SvGenericObject value)
{
}

SvLocal void
QBQuotaInputComboBoxOnFinishQuota(void *target, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue)
{
}

SvLocal void
QBQuotaInputSetFocus(SvWidget w)
{
    QBQuotaInput prv = (QBQuotaInput) w->prv;
    svWidgetSetFocus(prv->comboBox);
}

SvLocal SvWidget
QBQuotaInputNew(AppGlobals appGlobals, const char *widgetName)
{
    QBnPVRDirectory globalDirectory = SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, getDirectoryById, NULL);
    if (!globalDirectory) {
        return NULL;
    }

    QBnPVRProviderCapabilities caps = SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, getCapabilities);

    SvWidget w = svSettingsWidgetCreate(appGlobals->res, widgetName);
    QBQuotaInput prv = calloc(sizeof(*prv), 1);

    prv->directoryQuotaUnits = caps.directoryQuotaUnits;

    w->prv = prv;
    char nameBuf[1024];
    snprintf(nameBuf, sizeof(nameBuf), "%s.comboBox", widgetName);
    prv->comboBox = QBComboBoxNewFromSM(appGlobals->res, nameBuf);
    svWidgetSetFocusable(prv->comboBox, true);
    QBComboBoxCallbacks cb = { QBQuotaInputComboBoxPrepareQuota,
                               QBQuotaInputComboBoxInputQuota,
                               QBQuotaInputComboBoxChangeQuota,
                               QBQuotaInputComboBoxCurrentQuota,
                               NULL,
                               QBQuotaInputComboBoxOnFinishQuota };
    QBComboBoxSetCallbacks(prv->comboBox, prv, cb);

    SvArray values = SvArrayCreate(NULL);
    prv->maxQuota = 0;

    if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_bytes) {
        prv->maxQuota = globalDirectory->quota.maxBytes / (1024 * 1024 * 1024);
        prv->quotaStep = 10;
    } else if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_seconds) {
        prv->maxQuota = globalDirectory->quota.maxSeconds / (60 * 60);
        prv->quotaStep = 1;
    }

    for (int idx = 0; idx <= prv->maxQuota; idx += prv->quotaStep) {
        SvGenericObject v = (SvGenericObject) SvValueCreateWithInteger(idx, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(prv->comboBox, values);
    SVRELEASE(values);

    svSettingsWidgetAttach(w, prv->comboBox, nameBuf, 1);

    return w;
}

SvLocal int
QBQuotaEditGetLimit(SvWidget w)
{
    QBQuotaInput prv = (QBQuotaInput) w->prv;
    SvValue value = (SvValue) QBComboBoxGetValue(prv->comboBox);

    int ret = SvValueGetInteger(value);

    if (ret == prv->maxQuota) {
        ret = 0;
    }

    return ret;
}

SvLocal void
QBQuotaEditSetLimit(SvWidget w, int m)
{
    QBQuotaInput prv = (QBQuotaInput) w->prv;
    if (m == 0) {
        m = prv->maxQuota;
    }

    SvValue value = SvValueCreateWithInteger(m, NULL);
    QBComboBoxSetValue(prv->comboBox, (SvGenericObject) value);
    SVRELEASE(value);
}

SvLocal void
QBNPvrMenuContextUpdateQuota(QBNPvrMenuHandler self)
{
    if (!self->currentDir || !self->quotaItem) {
        return;
    }

    SVTESTRELEASE(self->quotaItem->subcaption);
    self->quotaItem->subcaption = NULL;

    QBnPVRProviderCapabilities caps = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getCapabilities);

    int quota = 0;
    if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_bytes) {
        quota = self->currentDir->quota.maxBytes;
        self->quotaItem->subcaption = quota ? SvStringCreateWithFormat(gettext("%iGB"), quota) : SvStringCreate(gettext("unlimited"), NULL);
    } else if (caps.directoryQuotaUnits == QBnPVRQuotaUnits_seconds) {
        quota = self->currentDir->quota.maxSeconds / (60 * 60);
        self->quotaItem->subcaption = quota ? SvStringCreateWithFormat(gettext("%ih"), quota) : SvStringCreate(gettext("unlimited"), NULL);
    }

    QBBasicPaneOptionPropagateObjectChange(self->options, self->quotaItem);
}

SvLocal void
QBNPvrMenuContextQuotaOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMenuHandler self = self_;
    if (!self->quotaEdit) {
        return;
    }

    QBNPvrMenuContextUpdateQuota(self);
    svWidgetDetach(self->quotaEdit);
    svWidgetDestroy(self->quotaEdit);
    self->quotaEdit = NULL;
    SVRELEASE(self->quotaPane);
}

SvLocal void
QBNPvrMenuContextQuotaOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMenuHandler self = self_;
    if (!self->quotaEdit) {
        return;
    }

    QBNPvrMenuContextUpdateQuota(self);
    svWidgetAttach(frame, self->quotaEdit, self->quotaEdit->off_x, self->quotaEdit->off_y, 0);
}

SvLocal void
QBNPvrMenuContextQuotaSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMenuHandler self = self_;
    if (!self->quotaEdit) {
        return;
    }

    QBQuotaInputSetFocus(self->quota);
}

SvLocal void fakeClean(SvApplication app, void *prv)
{
}

SvLocal bool
QBNPvrMenuContextQuotaEditInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBNPvrMenuHandler self = w->prv;
    if (e->ch != QBKEY_ENTER) {
        return false;
    }

    int newQuota = QBQuotaEditGetLimit(self->quota);
    if (self->currentDir) {
        QBNPvrMenuContextSetQuota(self, newQuota);
    }

    QBContextMenuPopPane(self->sidemenu.ctx);

    return true;
}

SvLocal void
QBNPvrMenuContextChangeQuota(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    if (!self->currentDir) {
        return;
    }

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBNPvrMenuContextQuotaOnHide,
        .onShow    = QBNPvrMenuContextQuotaOnShow,
        .setActive = QBNPvrMenuContextQuotaSetActive
    };

    svSettingsPushComponent("NPvr.settings");
    SvWidget quota = QBQuotaInputNew(self->appGlobals, "quota");
    if (!quota) {
        svSettingsPopComponent();
        return;
    }

    SvWidget quotaEdit = svSettingsWidgetCreate(self->appGlobals->res, "QuotaEdit");
    svSettingsWidgetAttach(quotaEdit, quota, "quota", 1);
    quotaEdit->off_x = svSettingsGetInteger("QuotaEdit", "xOffset", 0);
    quotaEdit->off_y = svSettingsGetInteger("QuotaEdit", "yOffset", 0);
    quotaEdit->prv = self;
    quotaEdit->clean = fakeClean;
    svWidgetSetInputEventHandler(quotaEdit, QBNPvrMenuContextQuotaEditInputEventHandler);

    SvWidget w = QBAsyncLabelNew(self->appGlobals->res, "quota.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(quotaEdit, w, svWidgetGetName(w), 1);
    QBAsyncLabelSetCText(w, gettext("Quota"));

    QBQuotaEditSetLimit(quota, ((QBQuotaInput) (quota->prv))->maxQuota);
    self->quotaEdit = quotaEdit;
    self->quota = quota;
    svSettingsPopComponent();

    self->quotaPane = QBContainerPaneCreateFromSettings("CalcPane.settings", self->appGlobals->res,
                                                        self->sidemenu.ctx, 2, SVSTRING("CalcPane"), &callbacks, self);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->quotaPane);
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

SvLocal void
QBNPvrMenuContextMenuMoveToDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    QBnPVRDirectory dir = NULL;
    if (self->currentRecording) {
        if (item->metadata) {
            dir = (QBnPVRDirectory) SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("dir"));
        }

        QBnPVRRecordingUpdateParams params = {
            .directoryId = NULL,
            .recordingId = self->currentRecording->id,
        };

        if (dir) {
            params.directoryId = dir->id;
        } else if (item->id && SvStringEqualToCString(item->id, "globalDir")) {
            params.directoryId = SVSTRING("0");
        }
        QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, updateRecording, params);
        QBNPvrDialogCreateAndShow(self->appGlobals, request);
    }
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBNPvrMenuContextMenuMove(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));

    //don't show if already in default directory. null currentDir possible
    if (self->currentDir && !QBnPVRProviderDirectoryIsGlobal(self->currentDir)) {
        SvString name = SvStringCreate(gettext("Default directory"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("globalDir"), name, QBNPvrMenuContextMenuMoveToDir, self);
        SVRELEASE(name);
    }

    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);

    SvIterator it = SvArrayIterator(dirList);
    QBnPVRDirectory dir = NULL;
    while ((dir = (QBnPVRDirectory) SvIteratorGetNext(&it))) {
        if (dir->keywordId || dir->seriesId) {
            // recordings can't be moved to keyword/series dirs
            continue;
        }
        if (self->currentDir && dir->id == self->currentDir->id) {
            // don't show current directory
            continue;
        }

        QBBasicPaneItem itemDir = QBBasicPaneAddOption(options, SVSTRING("dir"), dir->name, QBNPvrMenuContextMenuMoveToDir, self);
        SVTESTRELEASE(itemDir->metadata);
        itemDir->metadata = SvHashTableCreate(5, NULL);
        SvHashTableInsert(itemDir->metadata, (SvGenericObject) SVSTRING("dir"), (SvGenericObject) dir);
    }
    SVRELEASE(dirList);

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal void QBNPvrMenuContextMenuRecordingSpaceRecoveryApproveCallback(void* self_, int optionValue)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;
    QBnPVRRecordingUpdateParams params = {
        .recordingId = self->currentRecording->id,
    };

    if (optionValue == (int) QBnPVRRecordingSpaceRecovery_DeleteOldest) {
        params.recoveryOption = SvValueCreateWithInteger((int) QBnPVRRecordingSpaceRecovery_DeleteOldest, NULL);
    } else if (optionValue == (int) QBnPVRRecordingSpaceRecovery_Manual) {
        params.recoveryOption = SvValueCreateWithInteger((int) QBnPVRRecordingSpaceRecovery_Manual, NULL);
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, updateRecording, params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);

    SVTESTRELEASE(params.recoveryOption);

    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void QBNPvrMenuContextMenuRecordingPreferencesCreateSpaceRecoveryPane(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;
    if (!self->recordingSpaceRecoveryModePane) {
        self->recordingSpaceRecoveryModePane = QBNPvrOptionsSelectPaneCreate(self->appGlobals,
                                                                             self->sidemenu.ctx,
                                                                             3,
                                                                             QBNPvrOptionsSelectPaneType_SpaceRecovery);
        QBNPvrOptionsSelectPaneSetApproveCallback(self->recordingSpaceRecoveryModePane, QBNPvrMenuContextMenuRecordingSpaceRecoveryApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->recordingSpaceRecoveryModePane, (int) self->currentRecording->recoveryOption);
}

SvLocal void
QBNPvrMenuContextMenuRecordingPreferences(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));

    SvString name = SvStringCreate(gettext("Space recovery"), NULL);
    QBBasicPaneItem preferencesItem = QBBasicPaneAddOption(options, SVSTRING("spaceRecovery"), name, QBNPvrMenuContextMenuRecordingPreferencesCreateSpaceRecoveryPane, self);
    SVRELEASE(name);

    SvString recoveryOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_SpaceRecovery, (int) self->currentRecording->recoveryOption);
    SVTESTRELEASE(preferencesItem->subcaption);
    preferencesItem->subcaption = SVRETAIN(recoveryOptionStr);
    QBBasicPaneOptionPropagateObjectChange(options, preferencesItem);
    SVRELEASE(recoveryOptionStr);

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal void
QBNPvrMenuUpdateCurrentDirFromRecordingPath(QBNPvrMenuHandler self, SvGenericObject nodePath)
{
    SvGenericObject path = SvObjectCopy(nodePath, NULL);
    SvInvokeInterface(QBTreePath, path, truncate, -1);
    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
    SVRELEASE(path);

    if (SvObjectIsInstanceOf((SvObject) node, QBActiveTreeNode_getType())) {
        SVTESTRELEASE(self->currentDir);
        self->currentDir = NULL;
        QBnPVRDirectory dir = (QBnPVRDirectory) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("nPVRDirectory"));
        if (dir && !QBnPVRProviderDirectoryIsGlobal(dir)) {
            self->currentDir = SVRETAIN(dir);
        }
    }
}

SvLocal void
QBNPvrMenuShowRecordingSideMenu(QBNPvrMenuHandler self, QBnPVRRecording rec, SvGenericObject nodePath)
{
    if (rec->state == QBnPVRRecordingState_removed)
        return;

    SvString bookmarkId = create_bookmark_id(rec);
    bool hasLastPos = self->appGlobals->bookmarkManager && QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, bookmarkId);
    SVRELEASE(bookmarkId);

    QBNPvrMenuRecordingExtendedInfoHide(self);

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));
    SVTESTRELEASE(self->options);
    self->options = options;

    SVTESTRELEASE(self->currentRecording);
    self->currentRecording = SVRETAIN(rec);

    QBNPvrMenuUpdateCurrentDirFromRecordingPath(self, nodePath);

    SvArray confirmation = SvArrayCreate(NULL);
    SvArray ids = SvArrayCreate(NULL);
    SvString option = SvStringCreate(gettext("OK"), NULL);
    SvArrayAddObject(confirmation, (SvGenericObject) option);
    SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
    SVRELEASE(option);

    option = SvStringCreate(gettext("Cancel"), NULL);
    SvArrayAddObject(confirmation, (SvGenericObject) option);
    SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
    SVRELEASE(option);

    if (rec->playable) {
        if (hasLastPos) {
            option = SvStringCreate(gettext("Continue"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("continue"), option, QBNPvrMenuContextMenuPlay, self);
            SVRELEASE(option);
        }

        option = SvStringCreate(gettext("Play from the beginning"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("playFromTheBegining"), option, QBNPvrMenuContextMenuPlay, self);
        SVRELEASE(option);
    }

    option = SvStringCreate(gettext("Delete"), NULL);
    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDelete, self);
    SVRELEASE(option);

    if (self->currentDir) {
        option = SvStringCreate(gettext("Delete all"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteAllFromDir, self);
        SVRELEASE(option);
    } else if (rec->state == QBnPVRRecordingState_scheduled) {
        option = SvStringCreate(gettext("Delete all scheduled"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all scheduled"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteScheduled, self);
        SVRELEASE(option);
    } else if (rec->state == QBnPVRRecordingState_active) {
        option = SvStringCreate(gettext("Delete all active"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all active"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteActive, self);
        SVRELEASE(option);
    } else if (rec->state == QBnPVRRecordingState_completed) {
        option = SvStringCreate(gettext("Delete all completed"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteCompleted, self);
        SVRELEASE(option);
    } else if (rec->state == QBnPVRRecordingState_failed) {
        option = SvStringCreate(gettext("Delete all failed"), NULL);
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete all failed"), option, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteFailed, self);
        SVRELEASE(option);
    }

    if (self->currentRecording) {
        if (rec->state != QBnPVRRecordingState_failed) {
            option = SvStringCreate(gettext("Move"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("move"), option, QBNPvrMenuContextMenuMove, self);
            SVRELEASE(option);

            if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
                option = SvStringCreate(gettext("Preferences"), NULL);
                QBBasicPaneAddOption(options, SVSTRING("preferences"), option, QBNPvrMenuContextMenuRecordingPreferences, self);
                SVRELEASE(option);
            }
        }
    }

#if 0
    SvEPGEvent event = NULL;
    QBRecordingUtilsGetEvent(recording, &event);
    if (event) {
        option = SvStringCreate(gettext("More info"), NULL);
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvGenericObject) event);
        QBExtendedInfoPane extendedInfo = QBExtendedInfoPaneCreateFromSettings("ReminderEditorPane.settings", self->appGlobals,
                                                                               self->sidemenu.ctx, SVSTRING("ExtendedInfo"), 2, events);
        SVRELEASE(event);
        SVRELEASE(events);
        QBBasicPaneAddOptionWithSubpane(options, SVSTRING("more info"), option, (SvGenericObject) extendedInfo);
        SVRELEASE(option);
        SVRELEASE(extendedInfo);
    }
#endif

    svSettingsPopComponent();

    QBContextMenuShow(self->sidemenu.ctx);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);

    self->sideMenuShown = true;

    SVRELEASE(ids);
    SVRELEASE(confirmation);
}

SvLocal void
QBNPvrMenuSideMenuAddDirOptions(QBNPvrMenuHandler self, QBBasicPane options, QBnPVRDirectory dir)
{
    SVTESTRELEASE(self->currentDir);
    self->currentDir = SVRETAIN(dir);
    SVTESTRELEASE(self->currentRecording);
    self->currentRecording = NULL;

    SvArray confirmation = SvArrayCreate(NULL);
    SvArray ids = SvArrayCreate(NULL);
    SvString option = SvStringCreate(gettext("OK"), NULL);
    SvArrayAddObject(confirmation, (SvGenericObject) option);
    SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
    SVRELEASE(option);

    option = SvStringCreate(gettext("Cancel"), NULL);
    SvArrayAddObject(confirmation, (SvGenericObject) option);
    SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
    SVRELEASE(option);

    if (QBnPVRDirectoryIsKeyword(dir)) {
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteKeywordRecording"), NULL,
                                             SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextDeleteKeywordRec, self);
    } else if (QBnPVRDirectoryIsSeries(dir)) {
        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteSeriesRecording"), NULL,
                                             SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextDeleteSeriesRec, self);
    } else {
        QBBasicPaneAddOption(options, SVSTRING("renameDir"), NULL, QBNPvrMenuContextRenameDir, self);

        QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("removeDir"), NULL, SVSTRING("BasicPane"),
                                             confirmation, ids, QBNPvrMenuContextDeleteDir, self);

        //check if folder empty
        SvArray recordings = SvArrayCreate(NULL);
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listRecordingsByDirectory, dir->id, recordings);
        if (SvArrayCount(recordings)) {
            QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("removeDirKeepContents"), NULL, SVSTRING("BasicPane"),
                                                 confirmation, ids, QBNPvrMenuContextDeleteDirKeepContents, self);
        }
        SVRELEASE(recordings);
    }

    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteAll"), NULL, SVSTRING("BasicPane"),
            confirmation, ids, QBNPvrMenuContextDeleteDirectoryWithContents, self);

    if ((QBnPVRDirectoryIsKeyword(dir) || QBnPVRDirectoryIsSeries(dir)) &&
        (QBNPvrLogicIsPremiereFeatureEnabled() || QBNPvrLogicIsRecoveryFeatureEnabled())) {
        SvString name = SvStringCreate(gettext("Preferences"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("dirPreferences"), name, QBNPvrMenuContextDirPreferences, self);
        SVRELEASE(name);
    }

    // To enable quota for the series/keyword folders uncomment the code below (remove unused attributes also):
//    if (!scheduledRecsDir) {    // keyword or series
//        self->quotaItem = QBBasicPaneAddOption(options, SVSTRING("quota"), NULL, QBNPvrMenuContextChangeQuota, self);
//    } else {
//        self->quotaItem = NULL;
//    }

    self->quotaItem = NULL;
    SVRELEASE(ids);
    SVRELEASE(confirmation);
}

SvLocal void
QBNPvrMenuContextScheduleKeyword(void *self_, SvString id, QBBasicPane _pane, QBBasicPaneItem item)
{
    QBNPvrMenuHandler self = self_;

    struct QBRecordingEditPaneCreationSettings_ params = {
        .appGlobals       = self->appGlobals,
        .settingsFileName = "RecordingEditPane.settings",
        .optionsFileName  = "QBRecordingEditPane.json",
        .contextMenu      = self->sidemenu.ctx,
        .level            = 2
    };

    QBRecordingEditPane pane = QBRecordingEditPaneCreateNPvrKeywordWithEvent(&params, NULL, NULL);
    if (pane) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal void
QBNPvrMenuShowSideMenu(QBNPvrMenuHandler self, SvGenericObject node)
{
    if (SvObjectIsInstanceOf((SvObject) node, QBActiveTreeNode_getType())) {
        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));
        SvString itemsFilename = SvStringCreate("NPVRMenuContextPane.json", NULL);
        QBBasicPaneLoadOptionsFromFile(options, itemsFilename);
        SVRELEASE(itemsFilename);
        SVTESTRELEASE(self->options);
        self->options = options;

        QBBasicPaneAddOption(options, SVSTRING("scheduleKeywordRecording"), NULL, QBNPvrMenuContextScheduleKeyword, self);

        QBBasicPaneAddOption(options, SVSTRING("createNewDirectory"), NULL, QBNPvrMenuContextCreateDir, self);

        QBnPVRDirectory dir = (QBnPVRDirectory) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, SVSTRING("nPVRDirectory"));
        if (dir) {
            QBNPvrMenuSideMenuAddDirOptions(self, options, dir);
        } else {
            SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode)node);
            if (id) {
                SvArray confirmation = SvArrayCreate(NULL);
                SvArray ids = SvArrayCreate(NULL);
                SvString option = SvStringCreate(gettext("OK"), NULL);
                SvArrayAddObject(confirmation, (SvGenericObject) option);
                SvArrayAddObject(ids, (SvGenericObject) SVSTRING("OK"));
                SVRELEASE(option);

                option = SvStringCreate(gettext("Cancel"), NULL);
                SvArrayAddObject(confirmation, (SvGenericObject) option);
                SvArrayAddObject(ids, (SvGenericObject) SVSTRING("CANCEL"));
                SVRELEASE(option);

                if (SvStringEqualToCString(id, "npvr-ongoing")) {
                    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteAll"), NULL, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteActive, self);
                } else if (SvStringEqualToCString(id, "npvr-scheduled")) {
                    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteAll"), NULL, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteScheduled, self);
                } else if (SvStringEqualToCString(id, "npvr-completed")) {
                    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteAll"), NULL, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteCompleted, self);
                } else if (SvStringEqualToCString(id, "npvr-failed")) {
                    QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteAll"), NULL, SVSTRING("BasicPane"), confirmation, ids, QBNPvrMenuContextMenuDeleteFailed, self);
                }

                SVRELEASE(ids);
                SVRELEASE(confirmation);
            }
        }

        if (QBBasicPaneGetOptionsCount(options) > 0) {
            QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
            QBContextMenuShow(self->sidemenu.ctx);
        }

        svSettingsPopComponent();
    }
}

SvLocal void
QBNPvrMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    if (!node) {
        return;
    }

    if (SvObjectIsInstanceOf((SvObject) node, QBnPVRRecording_getType())) {
        QBnPVRRecording rec = (QBnPVRRecording) node;
        QBNPvrMenuShowRecordingSideMenu(self, rec, nodePath);
    } else {
        QBNPvrMenuShowSideMenu(self, node);
    }
}

SvLocal void
QBNPvrMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath, int position)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    if (SvObjectIsInstanceOf(node_, QBnPVRRecording_getType())) {
        QBnPVRRecording rec = (QBnPVRRecording) node_;
        QBNPvrMenuShowRecordingSideMenu(self, rec, nodePath);
    } else if (SvObjectIsInstanceOf(node_, QBnPVRDirectory_getType())) {
        QBnPVRProviderCapabilities caps = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getCapabilities);
        if (caps.directories) {
            QBnPVRDirectory dir = (QBnPVRDirectory) node_;
            SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, refreshDirectory, dir->id);
        }
    }
}

SvLocal SvRID
QBNPvrRecordingCreateBitmap(QBNPvrMenuHandler self, const QBnPVRRecording rec)
{
    QBNPvrRecordingBitmap npvrRecordingBitmap = QBNPvrRecordingBitmap_Scheduled;

    switch (rec->state) {
        case QBnPVRRecordingState_completed:
            if (QBNPvrLogicIsRecoveryFeatureEnabled() && rec->recoveryOption == QBnPVRRecordingSpaceRecovery_Manual)
                npvrRecordingBitmap = QBNPvrRecordingBitmap_CompletedPadlock;
            else
                npvrRecordingBitmap = QBNPvrRecordingBitmap_Completed;
            break;
        case QBnPVRRecordingState_active:
            npvrRecordingBitmap = QBNPvrRecordingBitmap_Recording;
            break;
        case QBnPVRRecordingState_scheduled:
            npvrRecordingBitmap = QBNPvrRecordingBitmap_Scheduled;
            break;
        case QBnPVRRecordingState_failed:
            npvrRecordingBitmap = QBNPvrRecordingBitmap_Failed;
            break;
        // TODO: if necessary, prepare new icons for this!
        case QBnPVRRecordingState_missed:
        case QBnPVRRecordingState_removed:
            if (rec->deleteReason == QBnPVRRecordingDeleteReason_Manual)
                npvrRecordingBitmap = QBNPvrRecordingBitmap_RemovedPadlock;
            else
                npvrRecordingBitmap = QBNPvrRecordingBitmap_Removed;
            break;
    }
    return self->bitmapRIDs[npvrRecordingBitmap];
}

SvLocal bool
QBNPvrRecordingDescSchouldBeBlocked(QBNPvrMenuHandler self, QBnPVRRecording recording)
{
    return recording->isAdult && QBParentalControlLogicAdultIsBlocked(self->appGlobals->parentalControlLogic);
}

SvLocal bool
QBNPvrRecordingGetDesc(QBNPvrMenuHandler self, QBnPVRRecording recording, SvEPGEventDesc *desc)
{
    if (!self->appGlobals || !recording) {
        return false;
    }

    if (QBNPvrRecordingDescSchouldBeBlocked(self, recording)) {
        *desc = QBParentalControlGetBlockedDescription(self->appGlobals->pc);
    } else {
        *desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, recording->event);
    }

    return true;
}

SvLocal SvString
QBNPvrRecordingCreateCaption(QBNPvrMenuHandler self, const QBnPVRRecording rec, bool printEvent, bool printStatus)
{
    const char *stateTag = "";

    if (printStatus) {
        switch (rec->state) {
            case QBnPVRRecordingState_scheduled:
                stateTag = gettext("[scheduled]"); break;
            case QBnPVRRecordingState_active:
                stateTag = gettext("[active]");    break;
            case QBnPVRRecordingState_completed:
                stateTag = gettext("[completed]"); break;
            case QBnPVRRecordingState_failed:
                stateTag = gettext("[failed]");    break;
            case QBnPVRRecordingState_missed:
                stateTag = gettext("[missed]");    break;
            case QBnPVRRecordingState_removed:
                stateTag = gettext("[removed]");   break;
        }
    }

    SvEPGEventDesc desc = NULL;
    QBNPvrRecordingGetDesc(self, rec, &desc);

    SvString channelName = rec ? rec->channelName : NULL;
    SvString eventName = desc ? desc->title : NULL;

    SvString caption;
    if (eventName && channelName)
        caption = SvStringCreateWithFormat("%s - %s %s",
                                           SvStringCString(channelName), SvStringCString(eventName), stateTag);
    else if (eventName)
        caption = SvStringCreateWithFormat("%s %s", SvStringCString(eventName), stateTag);
    else if (channelName)
        caption = SvStringCreateWithFormat("%s %s", SvStringCString(channelName), stateTag);
    else
        caption = SvStringCreateWithFormat("%s %s", "No name", stateTag);

    return caption;
}

static void sprintfQuotaValue(char* buf, size_t bufSize, int64_t num, int64_t scaleNom, int64_t scaleDenom)
{
    if (num < 0) {
        snprintf(buf, bufSize, "???");
        return;
    };

    num = num * 10 * scaleNom / scaleDenom;
    if (num % 10) {
        snprintf(buf, bufSize, "%lld.%d", (lli) num / 10, (int) (num % 10));
    } else {
        snprintf(buf, bufSize, "%lld", (lli) num / 10);
    };
}

static SvString createQuotaSubcaption(QBNPvrMenuHandler self, QBActiveTreeNode node, QBnPVRDirectory directory)
{
    SvString subcaption = NULL;
    QBnPVRProviderCapabilities caps = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getCapabilities);
    SvString customQuotaFormatV = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("CustomQuotaFormat"));

    /// TODO: how to translate this with plural forms etc. when we sometimes have fractions??? - Deffer
    char usedStr[32], totalStr[32];
    const char* unitStr = "units";
    const char* header = gettext("nPVR quota used");
    int64_t quotaTimeDivisor, quotaSizeDivisor;
    SvString displayQuotaTimeUnits, displayQuotaSizeUnits;
    switch (caps.directoryQuotaUnits) {
        case QBnPVRQuotaUnits_seconds:
            if (customQuotaFormatV && SvStringEqualToCString(customQuotaFormatV, "yes")) {
                SvString format = QBNPvrLogicCreateCustomQuotaFormat(directory->quota.usedSeconds, directory->quota.maxSeconds, QBnPVRQuotaUnits_seconds);
                subcaption = SvStringCreateWithFormat("%s : %s", header, SvStringCString(format));
                SVRELEASE(format);
            } else {
                displayQuotaTimeUnits = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("displayQuotaTimeUnits"));
                unitStr = displayQuotaTimeUnits ? SvStringCString(displayQuotaTimeUnits) : gettext("hours");
                SvValue quotaTimeDivisorV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("quotaTimeDivisor"));
                quotaTimeDivisor = quotaTimeDivisorV ? (int64_t) SvValueGetInteger(quotaTimeDivisorV) : 60 * 60;
                sprintfQuotaValue(usedStr, sizeof(usedStr), directory->quota.usedSeconds, 1, quotaTimeDivisor);
                sprintfQuotaValue(totalStr, sizeof(totalStr), directory->quota.maxSeconds, 1, quotaTimeDivisor);
            }
            break;
        case QBnPVRQuotaUnits_bytes:
            displayQuotaSizeUnits = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("displayQuotaSizeUnits"));
            unitStr = displayQuotaSizeUnits ? SvStringCString(displayQuotaSizeUnits) : gettext("MB");
            SvValue quotaSizeDivisorV = (SvValue) QBActiveTreeNodeGetAttribute(node, SVSTRING("quotaSizeDivisor"));
            quotaSizeDivisor = quotaSizeDivisorV ? (int64_t) SvValueGetInteger(quotaSizeDivisorV) : 1024 * 1024;
            sprintfQuotaValue(usedStr, sizeof(usedStr), directory->quota.usedBytes, 1, quotaSizeDivisor);
            sprintfQuotaValue(totalStr, sizeof(totalStr), directory->quota.maxBytes, 1, quotaSizeDivisor);
            break;
    }

    if (!subcaption) {
        subcaption = SvStringCreateWithFormat("%s : %s / %s %s", header, usedStr, totalStr, unitStr);
    }

    return subcaption;
}

SvLocal SvString
QBNPvrMenuHandlerCreateNPvrRecordingTimeInformationString(const QBnPVRRecording rec)
{
    if (!rec || !rec->event) {
        return NULL;
    }

    struct tm start = SvLocalTimeToLocal(SvLocalTimeFromUTC(rec->event->startTime));

    char buffer[127];
    size_t total = 0;
    total += strftime(buffer + total, sizeof(buffer) - total, QBTimeFormatGetCurrent()->PVRStartTime, &start);

    if (rec->state == QBnPVRRecordingState_active || rec->state == QBnPVRRecordingState_completed) {
        int duration = SvTimeGetSeconds(SvTimeSub(rec->endTime, rec->startTime));
        int seconds = duration % 60;
        int minutes = (duration / 60) % 60;
        int hours = duration / 3600;

        snprintf(buffer + total, sizeof(buffer) - total, " (%02d:%02d:%02d)", hours, minutes, seconds);
    }

    return SvStringCreate(buffer, NULL);
}

SvLocal SvString
QBNPvrMenuHandlerCreateRecordingInformationString(QBNPvrMenuHandler self, const QBnPVRRecording rec)
{
    SvStringBuffer strBuffer = SvStringBufferCreate(NULL);

    SvString timeInfoStr = QBNPvrMenuHandlerCreateNPvrRecordingTimeInformationString(rec);
    if (timeInfoStr) {
        SvStringBufferAppendCString(strBuffer, SvStringCString(timeInfoStr), NULL);
        SVRELEASE(timeInfoStr);
    }

    SvString recInfo = SvStringBufferCreateContentsString(strBuffer, NULL);
    SVRELEASE(strBuffer);

    return recInfo;
}

struct QBNPvrItemInfo_ {
    struct QBXMBItemInfo_t super_;
    QBnPVRRecording recording;
};
typedef struct QBNPvrItemInfo_ * QBNPvrItemInfo;

SvLocal SvType
QBNPvrItemInfo_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrItemInfo",
                            sizeof(struct QBNPvrItemInfo_),
                            QBXMBItemInfo_getType(),
                            &type,
                            NULL);
    }

    return type;
}

SvLocal QBNPvrItemInfo
QBNPvrItemInfoCreate(void)
{
    return (QBNPvrItemInfo) SvTypeAllocateInstance(QBNPvrItemInfo_getType(), NULL);
}

SvLocal SvWidget
QBNPvrMenuHandlerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path,
                            SvApplication app, XMBMenuState initialState)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    SvString caption = NULL;
    SvString subcaption = NULL;
    SvBitmap icon = NULL;
    SvString iconURL = NULL;
    SvRID iconRID = SV_RID_INVALID;
    QBXMBItem item = NULL;
    QBXMBItemConstructor constructor = NULL;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;

        caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
        const char* captionStr = SvStringCString(caption);
        const char* translatedCaptionStr = *captionStr ? gettext(captionStr) : "";
        if (translatedCaptionStr != captionStr)
            caption = SvStringCreate(translatedCaptionStr, NULL);
        else
            SVRETAIN(caption);

        SvGenericObject icon_ = (SvGenericObject) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
        if (SvObjectIsInstanceOf(icon_, SvBitmap_getType())) {
            icon = SVRETAIN(icon_);
        } else if (SvObjectIsInstanceOf(icon_, SvString_getType())) {
            iconURL = SVRETAIN(icon_);
        }

        bool displayQuota = false;
        SvString id = QBActiveTreeNodeGetID(node);
        if (id) {
            QBActiveArray array = NULL;

            if (SvStringEqualToCString(id, "npvr-ongoing")) {
                array = QBNPvrAgentGetOnGoingRecordingsList(self->appGlobals->npvrAgent);
            } else if (SvStringEqualToCString(id, "npvr-scheduled")) {
                array = QBNPvrAgentGetScheduledRecordingsList(self->appGlobals->npvrAgent);
                displayQuota = true;
            } else if (SvStringEqualToCString(id, "npvr-completed")) {
                array = QBNPvrAgentGetCompletedRecordingsList(self->appGlobals->npvrAgent);
                displayQuota = true;
            } else if (SvStringEqualToCString(id, "npvr-failed")) {
                array = QBNPvrAgentGetFailedRecordingsList(self->appGlobals->npvrAgent);
            } else if (SvStringEqualToCString(id, "npvr-deleted")) {
                array = QBNPvrAgentGetRemovedRecordingsList(self->appGlobals->npvrAgent);
            } else if (SvStringEqualToCString(id, "npvr-server-status")) {
                QBnPVRDirectory directory = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, NULL);
                if (directory) {
                    subcaption = createQuotaSubcaption(self, node, directory);
                }
            } else {
                subcaption = SVTESTRETAIN(QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
            }

            if (!subcaption) {
                subcaption = QBNPvrUtilsCreateRecordingsNumberAndQuotaSubcaption(array, displayQuota);
            }
        }

        item = QBXMBItemCreate();
        constructor = self->itemConstructor;
        item->focus = SVRETAIN(self->focus);
        item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    } else if (SvObjectIsInstanceOf(node_, QBnPVRRecording_getType())) {
        const QBnPVRRecording rec = (const QBnPVRRecording) node_;
        caption = QBNPvrRecordingCreateCaption(self, rec, true, false);
        subcaption = QBNPvrMenuHandlerCreateRecordingInformationString(self, rec);

        iconRID = QBNPvrRecordingCreateBitmap(self, rec);

        item = QBXMBItemCreate();
        constructor = self->itemConstructor;
        item->focus = SVRETAIN(self->focus);
        item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    } else {
        SvLogError("%s : bad object type [%s]", __func__, SvObjectGetTypeName(node_));
        return NULL;
    }

    if (!item) {
        SvLogError("%s : item wasn't created", __func__);
        return NULL;
    }

    assert(caption);

    item->caption = caption;
    item->subcaption = subcaption;
    item->icon = icon;
    item->iconURI.URI = iconURL;
    item->iconURI.isStatic = true;
    item->iconRID = iconRID;

    QBNPvrItemInfo itemInfo = QBNPvrItemInfoCreate();
    if (SvObjectIsInstanceOf(node_, QBnPVRRecording_getType())) {
        itemInfo->recording = (QBnPVRRecording) node_;
    }
    SvWidget ret = QBXMBItemConstructorInitItem(constructor, item, app, initialState, (QBXMBItemInfo) itemInfo);
    SVRELEASE(item);
    return ret;
}


SvLocal void
QBNPvrMenuHandlerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;
    QBNPvrItemInfo itemInfo = item_->prv;

    QBnPVRRecording currentRec = NULL;
    if (self->currentRecShown)
        currentRec = (QBnPVRRecording) SvWeakReferenceTakeReferredObject(self->currentRecShown);

    if (XMBMenuState_normal == state && isFocused) {
        // this item is focused
        if (!itemInfo->recording || currentRec != itemInfo->recording) {
            // not recording node or contains different recording than currently displayed
            if (QBNPvrMenuRecordingExtendedInfoIsVisible(self))
                QBNPvrMenuRecordingExtendedInfoHide(self);
            SVTESTRELEASE(self->currentRecShown);
            self->currentRecShown = NULL;
        }
        if (itemInfo->recording && currentRec != itemInfo->recording) {
            // self->currentRecShown is null here and extended info is hidden
            // but needs to be shown for new recording
            assert(!self->currentRecShown && !self->extendedInfoLabel);
            QBNPvrMenuRecordingExtendedInfoShow(self, itemInfo->recording);
            self->currentRecShown = SvWeakReferenceCreateWithObject((SvObject) itemInfo->recording, NULL);
        }
    } else if (itemInfo->recording && itemInfo->recording == currentRec) {
        // node is not focused but this is currently displayed recording -> hide it
        if (QBNPvrMenuRecordingExtendedInfoIsVisible(self))
            QBNPvrMenuRecordingExtendedInfoHide(self);
        SVTESTRELEASE(self->currentRecShown);
        self->currentRecShown = NULL;
    }

    SVTESTRELEASE(currentRec);

    QBXMBItemConstructorSetItemState(itemInfo->super_.constructor, item_, state, isFocused);
}

SvLocal void
QBNPvrMenuHandlerServerStatsChanged(SvGenericObject self_, QBnPVRDirectory directory)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    if (directory->name) {
        QBNPvrMenuContextUpdateQuota(self);
        return;
    }

    SvString id = SVSTRING("npvr-server-status");

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, id);
    if (!node)
        return;

    SvString subcaption = createQuotaSubcaption(self, node, directory);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObj) subcaption);
    SVRELEASE(subcaption);

    QBActiveTreePropagateNodeChange(self->menuTree, node, NULL);
}

SvLocal SvWidget QBNPvrMenuHandlerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;
    QBActiveTreeNode node = NULL;
    const char* widget_name = NULL;
    int childrenCount = 0;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        node = (QBActiveTreeNode) node_;
        QBnPVRDirectory dir = (QBnPVRDirectory) QBActiveTreeNodeGetAttribute(node, SVSTRING("nPVRDirectory"));
        if (dir) {
            QBActiveArray recordingsArray = QBPVRRecordingsTreeGetRecordings((SvGenericObject) self->appGlobals->recordingsTree, dir->id);
            childrenCount = QBActiveArrayCount(recordingsArray);
        } else {
            if (node == QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("npvr-scheduled"))) {
                QBActiveArray recordingsList = QBNPvrAgentGetScheduledRecordingsList(self->appGlobals->npvrAgent);
                childrenCount = QBActiveArrayCount(recordingsList);
            } else if (node == QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("npvr-ongoing"))) {
                QBActiveArray recordingsList = QBNPvrAgentGetOnGoingRecordingsList(self->appGlobals->npvrAgent);
                childrenCount = QBActiveArrayCount(recordingsList);
            } else if (node == QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("npvr-completed"))) {
                QBActiveArray recordingsList = QBNPvrAgentGetCompletedRecordingsList(self->appGlobals->npvrAgent);
                childrenCount = QBActiveArrayCount(recordingsList);
            } else if (node == QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("npvr-failed"))) {
                QBActiveArray recordingsList = QBNPvrAgentGetFailedRecordingsList(self->appGlobals->npvrAgent);
                childrenCount = QBActiveArrayCount(recordingsList);
            } else if (node == QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("npvr-deleted"))) {
                QBActiveArray recordingsList = QBNPvrAgentGetRemovedRecordingsList(self->appGlobals->npvrAgent);
                childrenCount = QBActiveArrayCount(recordingsList);
            }
        }

        if (QBActiveTreeNodeGetAttribute(node, SVSTRING("npvrMenuItem:big"))) {
            widget_name = "Big";
        }
    }

    if (childrenCount == 0) {
        return NULL;
    }

    if (widget_name) {
        svSettingsPushComponent("NPvr.settings");
        SvWidget ret = XMBVerticalMenuNew(app, widget_name, NULL);
        svSettingsPopComponent();
        return ret;
    }

    return XMBVerticalMenuNew(app, "menuBar.menu", NULL);
}

SvLocal void QBNPvrMenuHandlerAppStateChanged(SvGenericObject self_, uint64_t attrs)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    if (!(attrs & QBAppState_ParentalControlStateChanged)) {
        return;
    }

    QBActiveArray completedRecordings = QBNPvrAgentGetCompletedRecordingsList(self->appGlobals->npvrAgent);
    QBActiveArrayPropagateObjectsChange(completedRecordings, 0, QBActiveArrayCount(completedRecordings), NULL);

    QBActiveArray ongoingRecordings = QBNPvrAgentGetOnGoingRecordingsList(self->appGlobals->npvrAgent);
    QBActiveArrayPropagateObjectsChange(ongoingRecordings, 0, QBActiveArrayCount(ongoingRecordings), NULL);

    QBActiveArray scheduledRecordings = QBNPvrAgentGetScheduledRecordingsList(self->appGlobals->npvrAgent);
    QBActiveArrayPropagateObjectsChange(scheduledRecordings, 0, QBActiveArrayCount(scheduledRecordings), NULL);

    QBActiveArray failedRecordings = QBNPvrAgentGetFailedRecordingsList(self->appGlobals->npvrAgent);
    QBActiveArrayPropagateObjectsChange(failedRecordings, 0, QBActiveArrayCount(failedRecordings), NULL);

    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);
    size_t dirsCount = SvArrayCount(dirList);
    for (size_t dirIdx = 0; dirIdx < dirsCount; ++dirIdx) {
        QBnPVRDirectory dir = (QBnPVRDirectory) SvArrayAt(dirList, dirIdx);
        QBActiveArray dirRecordings = QBPVRRecordingsTreeGetRecordings((SvGenericObject) self->appGlobals->recordingsTree, dir->id);
        QBActiveArrayPropagateObjectsChange(dirRecordings, 0, QBActiveArrayCount(dirRecordings), NULL);
    }
    SVRELEASE(dirList);
}

SvLocal SvString QBNPvrMenuHandlerGetHintsForPath(SvGenericObject self_, SvGenericObject path)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    SvGenericObject currentNode = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
    if (currentNode && SvObjectIsInstanceOf(currentNode, QBnPVRRecording_getType())) {
        QBnPVRRecording node = (QBnPVRRecording) currentNode;
        if (node->state == QBnPVRRecordingState_removed)
            return NULL;
    }
    return SVSTRING("npvr_hint");
}

SvLocal SvType
QBNPvrMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrMenuHandlerDestroy
    };
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBNPvrMenuChoosen,
    };
    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBNPvrMenuContextChoosen,
    };

    static const struct XMBItemController_t xmbItemMethods = {
        .createItem   = QBNPvrMenuHandlerCreateItem,
        .setItemState = QBNPvrMenuHandlerSetItemState,
    };

    static const struct XMBMenuController_t menuControllerMethods = {
        .createSubMenu = QBNPvrMenuHandlerCreateSubMenu,
    };

    static const struct QBnPVRProviderDirectoryListener_ nPVRDirectoryListenerMethods = {
        .added   = QBNPvrMenuHandlerServerStatsChanged,
        .changed = QBNPvrMenuHandlerServerStatsChanged,
        .removed = QBNPvrMenuHandlerServerStatsChanged
    };

    static const struct QBAppStateServiceListener_t appMethods = {
        .stateChanged = QBNPvrMenuHandlerAppStateChanged
    };

    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBNPvrMenuHandlerGetHintsForPath
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrMenuHandler",
                            sizeof(struct QBNPvrMenuHandler_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            XMBMenuController_getInterface(), &menuControllerMethods,
                            XMBItemController_getInterface(), &xmbItemMethods,
                            QBnPVRProviderDirectoryListener_getInterface(), &nPVRDirectoryListenerMethods,
                            QBAppStateServiceListener_getInterface(), &appMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBNPvrMenuAddRecordingsList(AppGlobals appGlobals,
                            QBActiveArray recordingsList,
                            QBActiveTreeNode node,
                            SvGenericObject nodePath)
{
    QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) recordingsList, NULL, NULL);
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvGenericObject) proxy, nodePath, NULL);
    SVRELEASE(proxy);
}

SvLocal void QBNPvrMenuHandlerOnSideMenuClose(void *self_, QBContextMenu ctx)
{
    QBNPvrMenuHandler self = (QBNPvrMenuHandler) self_;

    QBContextMenuHide(ctx, false);

    // extended info may be shown before onClose callback is called (e.g. quick rec removal)
    if (!QBNPvrMenuRecordingExtendedInfoIsVisible(self) && self->currentRecShown) {
        QBnPVRRecording currentRec = (QBnPVRRecording) SvWeakReferenceTakeReferredObject(self->currentRecShown);
        if (currentRec) {
            QBNPvrMenuRecordingExtendedInfoShow(self, currentRec);
            SVRELEASE(currentRec);
        }
    }

    if (self->quotaItem) {
        self->quotaItem = NULL;
    }

    self->sideMenuShown = false;

    SVTESTRELEASE(self->currentRecording);
    self->currentRecording = NULL;
}

void
QBNPvrMenuRegister(SvWidget menuBar,
                   QBTreePathMap pathMap,
                   AppGlobals appGlobals)
{
    QBActiveTreeNode node;
    SvGenericObject subPath = NULL;
    SvGenericObject mainPath = NULL;

    if (!appGlobals->npvrAgent) {
        // nPVR not supported
        return;
    }

    node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("NPvr"), &mainPath);
    if (!node)
        return;

    if ((node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("npvr-scheduled"), &subPath))) {
        QBActiveArray recordingsList = QBNPvrAgentGetScheduledRecordingsList(appGlobals->npvrAgent);
        QBNPvrMenuAddRecordingsList(appGlobals, recordingsList, node, subPath);
    }
    if ((node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("npvr-ongoing"), &subPath))) {
        QBActiveArray recordingsList = QBNPvrAgentGetOnGoingRecordingsList(appGlobals->npvrAgent);
        QBNPvrMenuAddRecordingsList(appGlobals, recordingsList, node, subPath);
    }
    if ((node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("npvr-completed"), &subPath))) {
        QBActiveArray recordingsList = QBNPvrAgentGetCompletedRecordingsList(appGlobals->npvrAgent);
        QBNPvrMenuAddRecordingsList(appGlobals, recordingsList, node, subPath);
    }
    if ((node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("npvr-failed"), &subPath))) {
        QBActiveArray recordingsList = QBNPvrAgentGetFailedRecordingsList(appGlobals->npvrAgent);
        QBNPvrMenuAddRecordingsList(appGlobals, recordingsList, node, subPath);
    }
    if ((node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("npvr-deleted"), &subPath))) {
        QBActiveArray recordingsList = QBNPvrAgentGetRemovedRecordingsList(appGlobals->npvrAgent);
        QBNPvrMenuAddRecordingsList(appGlobals, recordingsList, node, subPath);
    }

    QBNPvrMenuHandler handler = (QBNPvrMenuHandler) SvTypeAllocateInstance(QBNPvrMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;
    handler->menuTree = SVRETAIN(appGlobals->menuTree);

    SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, addDirectoryInfoListener, (SvGenericObject) handler, NULL);

    svSettingsPushComponent("NPvr.settings");
    handler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    handler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        handler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");

    handler->bigItemConstructor = QBXMBItemConstructorCreate("Big.MenuItem", appGlobals->textRenderer);
    handler->bigFocus = QBFrameConstructorFromSM("Big.MenuItem.focus");
    if (svSettingsIsWidgetDefined("Big.MenuItem.inactiveFocus")) {
        handler->bigInactiveFocus = QBFrameConstructorFromSM("Big.MenuItem.inactiveFocus");
    }

    handler->bitmapRIDs[QBNPvrRecordingBitmap_Completed] = svSettingsGetResourceID("PVRRecordingBitmap", "ok");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_CompletedPadlock] = svSettingsGetResourceID("PVRRecordingBitmap", "okPadlock");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_Recording] = svSettingsGetResourceID("PVRRecordingBitmap", "recording");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_Scheduled] = svSettingsGetResourceID("PVRRecordingBitmap", "scheduled");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_Failed] = svSettingsGetResourceID("PVRRecordingBitmap", "error");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_Removed] = svSettingsGetResourceID("PVRRecordingBitmap", "deleted");
    handler->bitmapRIDs[QBNPvrRecordingBitmap_RemovedPadlock] = svSettingsGetResourceID("PVRRecordingBitmap", "deletedPadlock");
    svSettingsPopComponent();

    QBTreePathMapInsert(pathMap, mainPath, (SvGenericObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, mainPath, (SvObject) handler, NULL);

    handler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenuRecordings.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenuRecordings"));
    QBContextMenuSetCallbacks(handler->sidemenu.ctx, QBNPvrMenuHandlerOnSideMenuClose, handler);

    SVRELEASE(handler);

    QBnPVRDirectory directory = SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, getDirectoryById, NULL);
    if (directory) {
        QBNPvrMenuHandlerServerStatsChanged((void *) handler, directory);
    }

    QBAppStateServiceAddListener(appGlobals->appState, (SvGenericObject) handler);
}
