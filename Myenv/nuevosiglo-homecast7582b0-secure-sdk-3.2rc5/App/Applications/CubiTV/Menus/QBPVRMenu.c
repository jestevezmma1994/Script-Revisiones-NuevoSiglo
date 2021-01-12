/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBPVRMenu.h"

#include <libintl.h>
#include <assert.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <settings.h>
#include <TranslationMerger.h>
#include <Services/pvrAgent.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBRecordingEditPane.h>
#include <QBRecordFS/RecMetaKeys.h>
#include <Utils/recordingUtils.h>
#include <QBRecordFS/root.h>
#include <QBRecordFS/file.h>
#include <QBSearch/QBSearchProgressListener.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <QBWidgets/QBComboBox.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBSearchProgressDialog.h>
#include <QBInput/QBInputCodes.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <Logic/timeFormat.h>
#include <QBOSK/QBOSKKey.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentProviderListener.h>
#include <QBPVRSearchProvider.h>
#include <QBMenu/QBMenuEventHandler.h>
#include "menuchoice.h"
#include "main.h"

#define log_debug(fmt, ...)  if(0) SvLogNotice(COLBEG() "%s:%d " fmt  COLEND_COL(blue), __func__,__LINE__,##__VA_ARGS__)

#define MAX_QUOTA_GB        999

enum QBPVRRecordingBitmap{
    QBPVRRecordingBitmap_Scheduled,
    QBPVRRecordingBitmap_Completed,
    QBPVRRecordingBitmap_Recording,
    QBPVRRecordingBitmap_Interrupted,
    QBPVRRecordingBitmap_Error,
    QBPVRRecordingBitmap_Count,
};

typedef enum {
    QBPVRDirectoryNameValidatorStatus_OK,
    QBPVRDirectoryNameValidatorStatus_EMPTY,
    QBPVRDirectoryNameValidatorStatus_EXISTS
} QBPVRDirectoryNameValidatorStatus;

struct QBPVRMenuHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBActiveTree menuTree;
    QBContentTree pvrTree;

    QBPVRRecording currentRecording;
    QBPVRDirectory currentDir;
    QBContentCategory currentCategory;

    struct QBPVRMenuHandlerSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    SvBitmap bitmaps[QBPVRRecordingBitmap_Count];
    SvBitmap lockBitmap;
    SvWidget quotaEdit, quota;
    QBBasicPaneItem quotaItem;
    QBBasicPane options;
    int64_t pvrDisplayedAvailBytes;
    QBContainerPane quotaPane;

    QBActiveTreeNode selectedNode;
    SvGenericObject selectedNodePath;

    struct {
        SvRID scheduled;
        SvRID completed;
        SvRID ongoing;
        SvRID failed;
        SvRID keyword;
        SvRID series;
        SvRID repeat;
        SvRID directory;
    } directoryRIDs;

    SvRID searchIconRID;

    SvString quotaDisplayMode;

    QBContentSearch searchCategory;
    SvObject searchCategoryPath;
    QBSearchProgressDialog searchProgressDialog; ///< context managing search results or lack thereof
};
typedef struct QBPVRMenuHandler_t *QBPVRMenuHandler;

SvLocal void
QBPVRMenuHandlerDestroy(void *self_)
{
    QBPVRMenuHandler self = self_;

    SVRELEASE(self->menuTree);
    SVRELEASE(self->pvrTree);
    SVTESTRELEASE(self->currentRecording);
    SVTESTRELEASE(self->currentDir);
    SVTESTRELEASE(self->currentCategory);
    SVTESTRELEASE(self->sidemenu.ctx);
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->lockBitmap);
    SVTESTRELEASE(self->options);

    SVTESTRELEASE(self->selectedNode);
    SVTESTRELEASE(self->selectedNodePath);

    SVTESTRELEASE(self->quotaDisplayMode);

    SVTESTRELEASE(self->searchCategory);
    SVTESTRELEASE(self->searchCategoryPath);

    SVTESTRELEASE(self->searchProgressDialog);
}

SvLocal void
QBPVRMenuContextMenuCleanup(QBPVRMenuHandler self)
{
    SVTESTRELEASE(self->currentRecording);
    SVTESTRELEASE(self->currentDir);
    SVTESTRELEASE(self->currentCategory);
    self->currentRecording = NULL;
    self->currentDir = NULL;
    self->currentCategory = NULL;
}

SvLocal void
QBPVRMenuContextMenuHideSidemenu(QBPVRMenuHandler self)
{
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBPVRMenuContextMenuHide(QBPVRMenuHandler self)
{
    QBPVRMenuContextMenuHideSidemenu(self);
    QBPVRMenuContextMenuCleanup(self);
}

SvLocal void
QBPVRMenuContextMenuEdit(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if (self->currentRecording) {
        struct QBRecordingEditPaneCreationSettings_ params = {
            .appGlobals = self->appGlobals,
            .settingsFileName = "RecordingEditPane.settings",
            .optionsFileName = "QBRecordingSimpleEditPane.json",
            .contextMenu = self->sidemenu.ctx,
            .level = 2
        };

        QBRecordingEditPane pvrPane = QBRecordingEditPaneCreatePvrWithRec(&params, self->currentRecording, QBRecordingEditPane_Manual, NULL);
        if (pvrPane) {
            QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) pvrPane);
            QBContextMenuShow(self->sidemenu.ctx);
            SVRELEASE(pvrPane);
        }
    }
}

SvLocal void QBPVRMenuContextMenuMoveToDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item) {
    QBPVRMenuHandler self = self_;
    if (self->currentRecording) {
        QBPVRDirectory dir = NULL;
        if (item->metadata)
            dir = (QBPVRDirectory) SvHashTableFind(item->metadata, (SvObject) SVSTRING("dir"));

        QBPVRProviderMoveRecording(self->appGlobals->pvrProvider,
                                   self->currentRecording,
                                   dir);
    }
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void QBPVRMenuContextMenuMove(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    {
        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));

        if (self->currentCategory == NULL || !SvStringEqualToCString(QBContentCategoryGetName((QBContentCategory) self->currentCategory), "Completed Recordings")) {
            SvString name = SvStringCreate(gettext("Completed Recordings"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("remove"), name, QBPVRMenuContextMenuMoveToDir, self);
            SVRELEASE(name);
        }

        SvString sourceDirName = NULL;
        if (self->currentDir) {
            sourceDirName = QBContentCategoryGetName((QBContentCategory) self->currentDir);
        }

        SvArray dirList = QBPVRProviderCreateDirectoriesList(self->appGlobals->pvrProvider);
        SvIterator it = SvArrayIterator(dirList);
        QBPVRDirectory dir = NULL;
        while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
            if (dir->type == QBPVRDirectoryType_normal) {
                SvString categoryName = QBContentCategoryGetName((QBContentCategory) dir);

                if (sourceDirName && SvObjectEquals((SvObject) sourceDirName, (SvObject) categoryName))
                    continue;

                QBBasicPaneItem itemDir = QBBasicPaneAddOption(options, SVSTRING("dir"), categoryName, QBPVRMenuContextMenuMoveToDir, self);
                SvHashTableInsert(itemDir->metadata, (SvObject) SVSTRING("dir"), (SvObject) dir);
            }
        }
        SVTESTRELEASE(dirList);

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
        SVRELEASE(options);
        svSettingsPopComponent();
    }
}

SvLocal QBPVRDirectoryNameValidatorStatus QBPVRMenuContextMenuValidateDirectoryName(QBPVRMenuHandler self, SvString name) {
    if (!SvStringLength(name))
        return QBPVRDirectoryNameValidatorStatus_EMPTY;

    SvArray dirList = QBPVRProviderCreateDirectoriesList(self->appGlobals->pvrProvider);
    if (!dirList)
        return QBPVRDirectoryNameValidatorStatus_OK;

    SvIterator it = SvArrayIterator(dirList);
    QBPVRDirectory dir = NULL;
    while ((dir = (QBPVRDirectory)SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(QBContentCategoryGetName((QBContentCategory) dir), SvStringCString(name))) {
            SVRELEASE(dirList);
            return QBPVRDirectoryNameValidatorStatus_EXISTS;
        }
    }
    SVRELEASE(dirList);

    return QBPVRDirectoryNameValidatorStatus_OK;

}

SvLocal void QBPVRMenuContextMenuCreateDirectoryKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBPVRMenuHandler self = ptr;

    SvWidget title = QBOSKPaneGetTitle(pane);
    QBPVRDirectoryNameValidatorStatus validation = QBPVRMenuContextMenuValidateDirectoryName(self, input);

    switch (validation) {
        case QBPVRDirectoryNameValidatorStatus_OK:
            svLabelSetText(title, "");
            break;
        case QBPVRDirectoryNameValidatorStatus_EMPTY:
            svLabelSetText(title, gettext("Directory name cannot be empty"));
            break;
        case QBPVRDirectoryNameValidatorStatus_EXISTS:
            svLabelSetText(title, gettext("Directory name already exists"));
            break;
        default:
            svLabelSetText(title, "");
    }

    if (key->type == QBOSKKeyType_enter) {
        if (validation == QBPVRDirectoryNameValidatorStatus_OK) {
            QBPVRDirectoryCreateParams params = (QBPVRDirectoryCreateParams)
                      SvTypeAllocateInstance(QBPVRDirectoryCreateParams_getType(), NULL);
            params->name = SVRETAIN(input);

            QBPVRProviderCreateDirectory(self->appGlobals->pvrProvider, params);
            SVRELEASE(params);

            QBContextMenuHide(self->sidemenu.ctx, false);
        }
    }
}


SvLocal void QBPVRMenuContextMenuCreateDirectory(QBPVRMenuHandler self, int level)
{
    svSettingsPushComponent("PVRDirectoryOSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, level, SVSTRING("OSKPane"),
            QBPVRMenuContextMenuCreateDirectoryKeyTyped, self, &error);
    svSettingsPopComponent();
    if (!error) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal double
QBPVRMenuGetRecordingLastPosition(QBPVRMenuHandler self, QBPVRRecording recording)
{
    QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, recording->id);
    if (!bookmark)
        return 0.0;

    return QBBookmarkGetPosition(bookmark);
}

SvLocal void
QBPVRMenuSetRecordingLastPositionBookmark(QBPVRMenuHandler self, QBAnyPlayerLogic anyPlayerLogic, QBPVRRecording recording, bool resetPosition)
{

    QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, recording->id);
    if (!bookmark) {
        bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, recording->id, 0.0, QBBookmarkType_Recording);
    } else if (resetPosition) {
        QBBookmarkUpdatePosition(bookmark, 0.0);
        QBBookmarkManagerUpdateBookmark(self->appGlobals->bookmarkManager, bookmark);
    }

    if (bookmark)
        QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
}

SvLocal void
QBPVRMenuContextMenuPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if (self->currentRecording) {
        QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
        QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

        if (self->appGlobals->remoteControl) {
            QBPlaybackStateController playbackStateController = QBRemoteControlCreatePlaybackStateController(self->appGlobals->remoteControl, NULL);
            QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
            SVRELEASE(playbackStateController);
        }

        SvString serviceName = SvStringCreate(gettext("PVR"), NULL);
        QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
        SVRELEASE(serviceName);

        if (self->appGlobals->bookmarkManager) {
            bool resetPosition = false;
            if (id && SvStringEqualToCString(id, "playFromTheBeginning"))
                resetPosition = true;

            QBPVRMenuSetRecordingLastPositionBookmark(self, anyPlayerLogic, self->currentRecording, resetPosition);
        }

        SVRELEASE(anyPlayerLogic);

        QBPVRPlayerContextSetRecording(pvrPlayer, self->currentRecording);
        if (id && SvStringEqualToCString(id, "playFromTheBeginning")) {
            QBPVRProviderMarkRecordingViewed(self->appGlobals->pvrProvider, self->currentRecording);
        }
        QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
        SVRELEASE(pvrPlayer);
    }
    QBPVRMenuContextMenuHide(self);
}

SvLocal void
QBPVRMenuContextMenuStop(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if(SvStringEqualToCString(id, "OK") && self->currentRecording) {
        QBPVRProviderStopRecording(self->appGlobals->pvrProvider, self->currentRecording);
    }
    QBPVRMenuContextMenuHide(self);
}

SvLocal void
QBPVRMenuContextMenuDelete(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if(SvStringEqualToCString(id, "OK") && self->currentRecording && !self->currentRecording->lock) {
        QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, self->currentRecording);
    }
    QBPVRMenuContextMenuHide(self);
}

SvLocal void
QBPVRMenuContextMenuDeleteAllFromCategory(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;

    if(!SvStringEqualToCString(id, "OK"))
        goto out;

    QBPVRProviderDeleteAllFromCategory(self->appGlobals->pvrProvider, self->currentCategory);
out:
    QBPVRMenuContextMenuHide(self);
}

SvLocal void QBPVRMenuShowPaneDirsLimitBack(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    QBContextMenuPopPane(self->sidemenu.ctx);
}

SvLocal void QBPVRMenuShowPaneKeywordDirectoriesLimitMessage(QBPVRMenuHandler self, int level) {
    const char * msg = gettext("You have too many directories and recordings.");
    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, level, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(msg, NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBPVRMenuShowPaneDirsLimitBack, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
    QBContextMenuShow(self->sidemenu.ctx);
}

SvLocal void
QBPVRMenuContextMenuSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBPVRMenuHandler self = ptr;
    SvErrorInfo error = NULL;

    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        QBContextMenuHide(self->sidemenu.ctx, false);
        svSettingsPushComponent("searchResults.settings");
        self->searchProgressDialog = QBSearchProgressDialogCreate(self->appGlobals->res, self->appGlobals->controller, &error);
        svSettingsPopComponent();
        if (error) {
            goto err;
        }
        QBSearchProgressDialogSetListener(self->searchProgressDialog, (SvObject) self, &error);
        if (error) {
            goto err;
        }
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_progressState, &error);
        if (error) {
            goto err;
        }
        QBSearchProgressDialogShow(self->searchProgressDialog, &error);
        if (error) {
            goto err;
        }

        SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->searchCategory);
        if (provider) {
            QBContentProviderAddListener((QBContentProvider) provider, (SvObject) self);
            SVRELEASE(provider);
        }
        QBContentSearchStartNewSearch(self->searchCategory, input, self->appGlobals->scheduler);
    }
    return;
err:
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
}

SvLocal void
QBPVRMenuPushOSKPane(QBPVRMenuHandler self)
{
    //Prepare OSK
    svSettingsPushComponent("OSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 1, SVSTRING("OSKPane"),
                  QBPVRMenuContextMenuSearchKeyTyped, self, &error);
    svSettingsPopComponent();
    if (!error) {
        SvString lastSearchKeyword = QBContentSearchGetKeyword(self->searchCategory);

        if (lastSearchKeyword)
            QBOSKPaneSetRawInput(oskPane, SvStringCString(lastSearchKeyword));

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(oskPane);
}

SvLocal void
QBPVRMenuPushRecordingEditPane(QBPVRMenuHandler self, QBRecordingEditPaneType type, int level)
{
    svSettingsPushComponent("BasicPane.settings");

    struct QBRecordingEditPaneCreationSettings_ params = {
        .appGlobals = self->appGlobals,
        .settingsFileName = "RecordingEditPane.settings",
        .optionsFileName = "QBRecordingEditPane.json",
        .contextMenu = self->sidemenu.ctx,
        .level = level
    };

    QBRecordingEditPane pane = QBRecordingEditPaneCreatePvrWithRec(&params, NULL, type, NULL);
    if (pane) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }

    svSettingsPopComponent();
}

SvLocal bool
QBPVRMenuNodeSelected(SvObject self_,
                      SvObject node,
                      SvObject nodePath)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;

    if (SvObjectIsInstanceOf(node, QBContentSearch_getType())) {
        SVTESTRELEASE(self->searchCategory);
        SVTESTRELEASE(self->searchCategoryPath);
        self->searchCategory = SVRETAIN(node);
        self->searchCategoryPath = SVTESTRETAIN(nodePath);
        QBPVRMenuPushOSKPane(self);
        return true;
    } else if (SvObjectIsInstanceOf(node, QBPVRRecording_getType())) {
        QBPVRRecording recording = (QBPVRRecording) node;
        if (recording->playable) {
            QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
            QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

            if (self->appGlobals->remoteControl) {
                QBPlaybackStateController playbackStateController = QBPlaybackStateControllerCreate(self->appGlobals->scheduler, self->appGlobals->middlewareManager);
                QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
                SVRELEASE(playbackStateController);
            }

            SvString serviceName = SvStringCreate(gettext("PVR"), NULL);
            QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
            SVRELEASE(serviceName);
            if (self->appGlobals->bookmarkManager)
                QBPVRMenuSetRecordingLastPositionBookmark(self, anyPlayerLogic, recording, false);
            SVRELEASE(anyPlayerLogic);
            QBPVRPlayerContextSetRecording(pvrPlayer, recording);
            QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
            SVRELEASE(pvrPlayer);
        }
    }
    return false;
}

SvLocal void
QBPVRMenuContextStopDirectory(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if(SvStringEqualToCString(id, "OK")) {
        QBPVRProviderStopDirectory(self->appGlobals->pvrProvider, self->currentDir);
    }
    QBPVRMenuContextMenuHide(self);
}

SvLocal void QBPVRMenuContextMenuRenameDirectoryKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBPVRMenuHandler self = ptr;
    if (key->type == QBOSKKeyType_enter) {
        SvWidget title = QBOSKPaneGetTitle(pane);
        switch(QBPVRMenuContextMenuValidateDirectoryName(self, input)) {
            case QBPVRDirectoryNameValidatorStatus_OK:
                svLabelSetText(title, "");
                QBPVRDirectoryUpdateParams params = (QBPVRDirectoryUpdateParams)
                          SvTypeAllocateInstance(QBPVRDirectoryUpdateParams_getType(), NULL);
                params->updateName = true;
                params->name = SVRETAIN(input);
                QBPVRProviderUpdateDirectory(self->appGlobals->pvrProvider, self->currentDir, params);
                SVRELEASE(params);
                QBContextMenuHide(self->sidemenu.ctx, false);
                break;
            case QBPVRDirectoryNameValidatorStatus_EMPTY:
                svLabelSetText(title, gettext("Directory name cannot be empty"));
                break;
            case QBPVRDirectoryNameValidatorStatus_EXISTS:
                svLabelSetText(title, gettext("Directory name already exists"));
                break;
        }
    }
}


struct QBQuotaInput_t {
    SvWidget w;
    SvWidget comboBox;
    int quotaStep;
    int inputPos;
    int maxQuota;
};

typedef struct QBQuotaInput_t *QBQuotaInput;

SvLocal SvString QBQuotaInputComboBoxPrepareQuota(void *self_, SvWidget combobox, SvGenericObject value)
{
    log_debug("");
    QBQuotaInput self = self_;
    int quota = SvValueGetInteger((SvValue) value);

    if (!quota && !self->maxQuota) {
        return SvStringCreate(gettext("unknown quota"), NULL);
    }
    if (!quota || (self->maxQuota && quota >= self->maxQuota))
        return SvStringCreate(gettext("no quota limit"), NULL);
    return SvStringCreateWithFormat("%iGB", quota);
}


SvLocal bool QBQuotaInputComboBoxInputQuota(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    log_debug("");
    QBQuotaInput self = target;
    if (key < '0' || key > '9')
        return false;
    int digit = key - '0';

    const int QuotaMax = MAX_QUOTA_GB;

    SvValue value = (SvValue)QBComboBoxGetValue(combobox);
    int QuotaValue = self->inputPos ? SvValueGetInteger(value) : 0;
    if(QuotaValue > QuotaMax / 10)
        QuotaValue = 0;
    QuotaValue = QuotaValue*10 + digit;
    self->inputPos = 1;
    value = SvValueCreateWithInteger(QuotaValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal int QBQuotaInputComboBoxChangeQuota(void * target, SvWidget combobox, SvGenericObject value, int key)
{
    log_debug("");
    QBQuotaInput self = target;
    self->inputPos = 0;
    int Quota = SvValueGetInteger((SvValue)value);
    if (Quota == self->maxQuota)
        Quota = 0;
    Quota += key == QBKEY_RIGHT ? self->quotaStep : -1;
    return Quota / self->quotaStep;
}


SvLocal void QBQuotaInputComboBoxCurrentQuota(void *target, SvWidget combobox, SvGenericObject value)
{
    log_debug("");
}

SvLocal void QBQuotaInputComboBoxOnFinishQuota(void *target, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue)
{
    log_debug("");
}

SvLocal void
QBQuotaInputSetFocus(SvWidget w)
{
    QBQuotaInput prv = (QBQuotaInput)w->prv;
    svWidgetSetFocus(prv->comboBox);
}

SvLocal SvWidget
QBQuotaInputNew(SvApplication app, QBPVRProvider pvrProvider, const char *widgetName)
{
    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    QBQuotaInput prv = calloc(sizeof(*prv), 1);

    w->prv = prv;
    char nameBuf[1024];
    snprintf(nameBuf, sizeof(nameBuf), "%s.comboBox", widgetName);
    prv->comboBox = QBComboBoxNewFromSM(app, nameBuf);
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


    prv->maxQuota = QBPVRProviderGetCurrentQuota(pvrProvider).limit / (1024*1024*1024);

    int max = prv->maxQuota > 0 ? prv->maxQuota : 999;
    int idx=0;
    prv->quotaStep = 10;
    for(idx=0; idx<=max; idx+=prv->quotaStep) {
        SvGenericObject v =(SvGenericObject)SvValueCreateWithInteger(idx, NULL);
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
    QBQuotaInput prv = (QBQuotaInput)w->prv;
    SvValue value = (SvValue)QBComboBoxGetValue(prv->comboBox);
    int ret = SvValueGetInteger(value);
    if (ret == prv->maxQuota)
        ret = 0;
    return ret;
}

SvLocal void
QBQuotaEditSetLimit(SvWidget w, int m)
{
    QBQuotaInput prv = (QBQuotaInput)w->prv;
    if(!m)
        m = prv->maxQuota;
    SvValue value = SvValueCreateWithInteger(m, NULL);
    QBComboBoxSetValue(prv->comboBox, (SvGenericObject)value);
    SVRELEASE(value);
}

SvLocal void QBPVRMenuContextQuotaOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBPVRMenuHandler self = self_;
    if(!self->quotaEdit)
        return;
    svWidgetDetach(self->quotaEdit);
    svWidgetDestroy(self->quotaEdit);
    self->quotaEdit = NULL;
    SVRELEASE(self->quotaPane);
}

SvLocal void QBPVRMenuContextQuotaOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBPVRMenuHandler self = self_;
    if(!self->quotaEdit)
        return;
    svWidgetAttach(frame, self->quotaEdit, self->quotaEdit->off_x, self->quotaEdit->off_y, 0);
}

SvLocal void QBPVRMenuContextQuotaSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBPVRMenuHandler self = self_;
    if(!self->quotaEdit)
        return;

    QBQuotaInputSetFocus(self->quota);
}


SvLocal void fakeClean(SvApplication app, void *prv)
{

}


SvLocal bool QBPVRMenuContextQuotaEditInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBPVRMenuHandler self = w->prv;
    if(e->ch == QBKEY_ENTER) {
        int64_t newQuota = QBQuotaEditGetLimit(self->quota);
        if(self->currentDir) {
            QBPVRDirectoryUpdateParams params = (QBPVRDirectoryUpdateParams)
                      SvTypeAllocateInstance(QBPVRDirectoryUpdateParams_getType(), NULL);
            params->updateQuota = true;
            params->quotaLimit = newQuota;
            QBPVRProviderUpdateDirectory(self->appGlobals->pvrProvider,
                                         self->currentDir,
                                         params);
            SVRELEASE(params);
        }
        QBContextMenuPopPane(self->sidemenu.ctx);
        return true;
    }
    return false;
}

SvLocal bool QBPVRMenuQuotaPaneInputEventHandler(void *self_, SvGenericObject src, SvInputEvent e)
{
    if (e->ch != QBKEY_BACK)
        return true;

    return false;
}

SvLocal void QBPVRMenuContextChangeQuota(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;

    if(!self->currentDir)
        return;

    static struct QBContainerPaneCallbacks_t callbacks = {
            .onHide = QBPVRMenuContextQuotaOnHide,
            .onShow = QBPVRMenuContextQuotaOnShow,
            .setActive = QBPVRMenuContextQuotaSetActive
    };

    self->quotaPane = QBContainerPaneCreateFromSettings("CalcPane.settings", self->appGlobals->res,
            self->sidemenu.ctx, 2, SVSTRING("CalcPane"), &callbacks, self);
    svSettingsPushComponent("Pvr.settings");

    SvWidget quotaEdit = svSettingsWidgetCreate(self->appGlobals->res, "QuotaEdit");
    SvWidget quota = QBQuotaInputNew(self->appGlobals->res, self->appGlobals->pvrProvider, "quota");
    svSettingsWidgetAttach(quotaEdit, quota, "quota", 1);
    quotaEdit->off_x = svSettingsGetInteger("QuotaEdit", "xOffset", 0);
    quotaEdit->off_y = svSettingsGetInteger("QuotaEdit", "yOffset", 0);
    quotaEdit->prv = self;
    quotaEdit->clean = fakeClean;
    svWidgetSetInputEventHandler(quotaEdit, QBPVRMenuContextQuotaEditInputEventHandler);

    SvWidget w = QBAsyncLabelNew(self->appGlobals->res, "quota.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(quotaEdit, w, svWidgetGetName(w), 1);
    QBAsyncLabelSetCText(w, gettext("Quota"));

    QBQuotaEditSetLimit(quota, self->currentDir->quotaLimit);
    self->quotaEdit = quotaEdit;
    self->quota = quota;
    svSettingsPopComponent();

    QBContextMenuSetInputEventHandler(self->sidemenu.ctx, self, QBPVRMenuQuotaPaneInputEventHandler);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->quotaPane);
}

SvLocal QBBasicPane
QBPVRMenuContextCreateSideMenuForRecording(QBPVRMenuHandler self);

SvLocal void
QBPVRMenuContextChangeLock(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;

    if (self->currentRecording) {
        bool shouldBeLocked = id && SvStringEqualToCString(id, "lock");
        QBPVRProviderLockRecording(self->appGlobals->pvrProvider, self->currentRecording, shouldBeLocked);
    }
}

SvLocal void
QBPVRMenuContextRenameDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    svSettingsPushComponent("PVRDirectoryOSKPane.settings");
    QBOSKPane oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(oskPane, self->appGlobals->scheduler, self->sidemenu.ctx, 2, SVSTRING("OSKPane"),
            QBPVRMenuContextMenuRenameDirectoryKeyTyped, self, &error);
    if (!error) {
        svSettingsPopComponent();

        QBOSKPaneSetInput(oskPane, QBContentCategoryGetName((QBContentCategory) self->currentDir));

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) oskPane);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }

    SVRELEASE(oskPane);
}


SvLocal void
QBPVRMenuContextDeleteDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if(SvStringEqualToCString(id, "OK")) {
        QBPVRDirectoryDeleteParams params = (QBPVRDirectoryDeleteParams)
                  SvTypeAllocateInstance(QBPVRDirectoryDeleteParams_getType(), NULL);

        QBPVRProviderDeleteDirectory(self->appGlobals->pvrProvider,
                                     self->currentDir,
                                     params);
        SVRELEASE(params);
    }
    QBPVRMenuContextMenuHide(self);
}

SvLocal void
QBPVRMenuContextScheduleManual(void *self_, SvString id, QBBasicPane _pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    QBPVRMenuPushRecordingEditPane(self, QBRecordingEditPane_Manual, 2);
}

SvLocal void
QBPVRMenuContextScheduleKeyword(void *self_, SvString id, QBBasicPane _pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    QBPVRMenuPushRecordingEditPane(self, QBRecordingEditPane_Keyword, 2);
}

SvLocal void
QBPVRMenuContextCreateDir(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRMenuHandler self = self_;
    if (QBPVRProviderCheckDirectoryLimit(self->appGlobals->pvrProvider, QBPVRDirectoryType_normal)) {
        QBPVRMenuShowPaneKeywordDirectoriesLimitMessage(self, 2);
    } else {
        QBPVRMenuContextMenuCreateDirectory(self, 2);
    }
}

SvLocal QBBasicPane
QBPVRMenuContextCreateSideMenuForRecording(QBPVRMenuHandler self)
{
    if (self && self->currentRecording) {
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);

        svSettingsPushComponent("BasicPane.settings");
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));
        SvString itemsFilename = SvStringCreate("PVRMenuContextPane.json",NULL);
        QBBasicPaneLoadOptionsFromFile(options, itemsFilename);
        SVRELEASE(itemsFilename);

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

        if (self->currentRecording->playable) {
            if (QBPVRMenuGetRecordingLastPosition(self, self->currentRecording) != 0.0) {
                option = SvStringCreate(gettext("Continue"), NULL);
                QBBasicPaneAddOption(options, SVSTRING("continue"), option, QBPVRMenuContextMenuPlay, self);
                SVRELEASE(option);

                option = SvStringCreate(gettext("View from the beginning"), NULL );
            } else if (!self->currentRecording->viewed){
                // play record first time
                option = SvStringCreate(gettext("View"), NULL );
            } else {
                option = SvStringCreate(gettext("View again"), NULL );
            }

            QBBasicPaneAddOption(options, SVSTRING("playFromTheBeginning"), option, QBPVRMenuContextMenuPlay, self);
            SVRELEASE(option);
        }
        if (self->currentRecording->state == QBPVRRecordingState_active) {
            option = SvStringCreate(gettext("Stop"), NULL);
            QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("stop"), option, SVSTRING("BasicPane"), confirmation, ids, QBPVRMenuContextMenuStop, self);
            SVRELEASE(option);
        }

        if ((self->currentRecording->state == QBPVRRecordingState_active ||
            self->currentRecording->state == QBPVRRecordingState_scheduled) &&
            (self->currentRecording->type == QBPVRRecordingType_manual)) {
            option = SvStringCreate(gettext("Edit"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("edit"), option, QBPVRMenuContextMenuEdit, self);
            SVRELEASE(option);
        }

        if (self->currentRecording->lock) {
            option = SvStringCreate(gettext("Unlock"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("unlock"), option, QBPVRMenuContextChangeLock, self);
            SVRELEASE(option);
        } else {
            option = SvStringCreate(gettext("Lock"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("lock"), option, QBPVRMenuContextChangeLock, self);
            SVRELEASE(option);
        }

        if (!self->currentRecording->lock) {
            option = SvStringCreate(gettext("Delete recording"), NULL);
            QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("delete"), option, SVSTRING("BasicPane"), confirmation, ids, QBPVRMenuContextMenuDelete, self);
            SVRELEASE(option);

            if (self->currentRecording->state != QBPVRRecordingState_failed &&
                self->currentRecording->state != QBPVRRecordingState_missed &&
                self->currentRecording->state != QBPVRRecordingState_interrupted) {
                option = SvStringCreate(gettext("Move"), NULL);
                QBBasicPaneAddOption(options, SVSTRING("move"), option, QBPVRMenuContextMenuMove, self);
                SVRELEASE(option);
            }
        }

#if 1   // the options below relate to special folders only (Scheduled, Ongoing, Completed and Failed Recordings)
        if (self->currentCategory) {
            SvString id = QBContentCategoryGetId(self->currentCategory);
            SvString optionId = NULL;
            if (SvStringEqualToCString(id, SCHEDULED_CATEGORY_ID)) {
                optionId = SVSTRING("delete all scheduled");
                option = SvStringCreate(gettext("Delete all scheduled"), NULL);
            } else if (SvStringEqualToCString(id, ONGOING_CATEGORY_ID)) {
                optionId = SVSTRING("delete all active");
                option = SvStringCreate(gettext("Delete all active"), NULL);
            } else if (SvStringEqualToCString(id, COMPLETED_CATEGORY_ID)) {
                optionId = SVSTRING("delete all completed");
                option = SvStringCreate(gettext("Delete all completed"), NULL);
            } else if (SvStringEqualToCString(id, FAILED_CATEGORY_ID)) {
                optionId = SVSTRING("delete all failed");
                option = SvStringCreate(gettext("Delete all failed"), NULL);
            } else {
                SvLogError("%s() unhandled category.", __func__);
                assert(0);
            }
            QBBasicPaneAddOptionWithConfirmation(options, optionId, option, SVSTRING("BasicPane"), confirmation, ids, QBPVRMenuContextMenuDeleteAllFromCategory, self);
            SVRELEASE(option);
        }
#endif

        if (self->currentRecording->event) {
            option = SvStringCreate(gettext("More info"), NULL);
            SvArray events = SvArrayCreate(NULL);
            SvArrayAddObject(events, (SvObject) self->currentRecording->event);
            QBExtendedInfoPane extendedInfo = QBExtendedInfoPaneCreateFromSettings("ReminderEditorPane.settings", self->appGlobals,
                    self->sidemenu.ctx, SVSTRING("ExtendedInfo"), 2, events);
            SVRELEASE(events);
            QBBasicPaneAddOptionWithSubpane(options, SVSTRING("more info"), option, (SvObject) extendedInfo);
            SVRELEASE(option);
            SVRELEASE(extendedInfo);
        }

        svSettingsPopComponent();

        SVRELEASE(ids);
        SVRELEASE(confirmation);

        return options;
    }

    return NULL;
}

SvLocal void
QBPVRMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    SvGenericObject node;

    node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    if (!node || SvObjectIsInstanceOf(node, QBContentSearch_getType())) {
        return;
    }

    if (SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));
        SVTESTRELEASE(self->options);
        self->options = options;

        SvString option;
        option = SvStringCreate(gettext("Schedule manual recording"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("scheduleManualRecording"), option, QBPVRMenuContextScheduleManual, self);
        SVRELEASE(option);

        option = SvStringCreate(gettext("Schedule keyword recording"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("scheduleKeywordRecording"), option, QBPVRMenuContextScheduleKeyword, self);
        SVRELEASE(option);

        option = SvStringCreate(gettext("Create new directory"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("createNewDirectory"), option, QBPVRMenuContextCreateDir, self);
        SVRELEASE(option);

        if (SvObjectIsInstanceOf(node, QBPVRDirectory_getType())) {
            QBPVRDirectory dir = (QBPVRDirectory) node;
            SVTESTRELEASE(self->currentDir);
            self->currentDir = SVRETAIN(dir);
            SVTESTRELEASE(self->currentRecording);
            self->currentRecording = NULL;

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

            if (dir->type == QBPVRDirectoryType_keyword) {
                option = SvStringCreate(gettext("Stop keyword recording"), NULL);
                QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteKeywordRecording"), option,
                        SVSTRING("BasicPane"), confirmation, ids, QBPVRMenuContextStopDirectory, self);
                SVRELEASE(option);
            }

            if (dir->type == QBPVRDirectoryType_repeated) {
                option = SvStringCreate(gettext("Stop repeated recording"), NULL);
                QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("deleteRepeatedRecording"), option,
                        SVSTRING("BasicPane"), confirmation, ids, QBPVRMenuContextStopDirectory, self);
                SVRELEASE(option);
            }

            if (dir->type != QBPVRDirectoryType_keyword) {
                option = SvStringCreate(gettext("Rename directory"), NULL);
                QBBasicPaneAddOption(options, SVSTRING("renameDir"), option, QBPVRMenuContextRenameDir, self);
                SVRELEASE(option);
            }

            option = SvStringCreate(gettext("Delete directory"), NULL);
            QBBasicPaneItem item = QBBasicPaneAddOptionWithConfirmation(options, SVSTRING("removeDir"), option, SVSTRING("BasicPane"),
                    confirmation, ids, QBPVRMenuContextDeleteDir, self);
            svSettingsPushComponent("Pvr.settings");
            SvString msg = SvStringCreate(gettext(svSettingsGetString("PvrMsgs", "deletedDirWarning")), NULL);
            QBBasicPaneSetTitle((QBBasicPane)item->subpane, msg);
            svSettingsPopComponent();
            SVRELEASE(msg);
            SVRELEASE(option);

            if (self->currentDir->type != QBPVRDirectoryType_normal) {
                option = SvStringCreate(gettext("Quota"), NULL);
                self->quotaItem = QBBasicPaneAddOption(options, SVSTRING("quota"), option,
                        QBPVRMenuContextChangeQuota, self);
                SVRELEASE(option);
            }

            SVRELEASE(ids);
            SVRELEASE(confirmation);
        }

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
        QBContextMenuShow(self->sidemenu.ctx);

        svSettingsPopComponent();
    } else if (SvObjectIsInstanceOf(node, QBPVRRecording_getType())) {
        QBPVRRecording recording = (QBPVRRecording) node;
        SVTESTRELEASE(self->currentDir);
        self->currentDir = NULL;
        SVTESTRELEASE(self->currentCategory);
        self->currentCategory = NULL;

        SvObject parentPath = SvObjectCopy(nodePath, NULL);
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
        SvObject parent = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, parentPath);
        SVRELEASE(parentPath);
        if (recording->directory) {
            self->currentDir = (QBPVRDirectory) SvWeakReferenceTakeReferredObject(recording->directory);
        } else if (parent &&
                   SvObjectIsInstanceOf(parent, QBContentCategory_getType()) &&
                   !SvObjectIsInstanceOf(parent, QBPVRDirectory_getType()) &&
                   !SvObjectIsInstanceOf(parent, QBContentSearch_getType())) {
            self->currentCategory = SVRETAIN(parent);
        }

        SVTESTRELEASE(self->currentRecording);
        self->currentRecording = SVRETAIN(recording);

        SVTESTRELEASE(self->options);
        self->options = QBPVRMenuContextCreateSideMenuForRecording(self);

        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->options);
        QBContextMenuShow(self->sidemenu.ctx);
    }
}

SvLocal SvBitmap
QBPVRRecordingCreateBitmap(QBPVRMenuHandler self, QBPVRRecording recording)
{
    SvBitmap icon = self->bitmaps[QBPVRRecordingBitmap_Error];
    switch (recording->state){
        case QBPVRRecordingState_completed:
        case QBPVRRecordingState_stoppedManually:
            icon = self->bitmaps[QBPVRRecordingBitmap_Completed];
            break;
        case QBPVRRecordingState_interrupted:
        case QBPVRRecordingState_stoppedNoSpace:
        case QBPVRRecordingState_stoppedNoSource:
            icon = self->bitmaps[QBPVRRecordingBitmap_Interrupted];
            break;
        case QBPVRRecordingState_failed:
        case QBPVRRecordingState_missed:
            icon = self->bitmaps[QBPVRRecordingBitmap_Error];
            break;
        case QBPVRRecordingState_active:
            icon = self->bitmaps[QBPVRRecordingBitmap_Recording];
            break;
        case QBPVRRecordingState_scheduled:
            icon = self->bitmaps[QBPVRRecordingBitmap_Scheduled];
            break;
        default:
            break;
    }
    return SVRETAIN(icon);
}

SvLocal SvString
QBPVRRecordingCreateAvailabilityInformation(QBPVRRecording recording)
{
    if (recording->expirationTime.us <= 0)
        return NULL;

    struct tm expiration = SvLocalTimeToLocal(SvLocalTimeFromUTC(SvTimeGetSeconds(recording->expirationTime)));

    char buffer[127];
    strftime(buffer, sizeof(buffer), QBTimeFormatGetCurrent()->PVRAvailableTime, &expiration);

    return SvStringCreate(buffer, NULL);
}

struct QBPVRItem_t {
    struct QBXMBItemInfo_t item;
    SvWidget lockIcon;
};
typedef struct QBPVRItem_t* QBPVRItem;

SvLocal SvType
QBPVRItem_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRItem",
                            sizeof(struct QBPVRItem_t),
                            QBXMBItemInfo_getType(),
                            &type,
                            NULL);
    }

    return type;
}

SvLocal QBPVRItem
QBPVRItemCreate(void)
{
    return (QBPVRItem)SvTypeAllocateInstance(QBPVRItem_getType(), NULL);
}

SvLocal void QBPVRItemInfoPositionItems(QBXMBItemInfo _item)
{
    QBPVRItem item = (QBPVRItem)_item;
    QBXMBItemInfoPositionItems(_item);
    if(item->lockIcon) {
        item->lockIcon->off_x = _item->title.w->off_x + _item->title.w->width + item->lockIcon->width / 2;
        item->lockIcon->off_y = _item->title.w->off_y;
        int focusWidth = item->lockIcon->off_x + item->lockIcon->width * 2;
        if(focusWidth > _item->currentFocusWidth) {
            QBXMBItemInfoSetFocusWidth(_item, focusWidth);
        }
    }
}

SvLocal SvWidget
QBPVRMenuHandlerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;

    bool lock = false;
    SvString caption = NULL;
    SvString subcaption = NULL;
    SvBitmap icon = NULL;
    SvRID iconRID = SV_RID_INVALID;

    if (SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory category = (QBContentCategory) node_;
        if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
            caption = SvStringCreate(gettext("Search"), NULL);
        } else {
            caption = QBContentCategoryGetName(category);
            if (caption) {
                const char *captionStr, *translatedCaptionStr;
                captionStr = SvStringCString(caption);
                translatedCaptionStr = gettext(captionStr);
                if (translatedCaptionStr != captionStr)
                    caption = SvStringCreate(translatedCaptionStr, NULL);
                else
                    SVRETAIN(caption);
            }
        }


        if (SvObjectIsInstanceOf(node_, QBPVRDirectory_getType())) {
            size_t childrenCount = QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category);
            subcaption = SvStringCreateWithFormat(ngettext("%i recording", "%i recordings", childrenCount), (int) childrenCount);
            switch (((QBPVRDirectory) node_)->type) {
                case QBPVRDirectoryType_keyword:
                    iconRID = self->directoryRIDs.keyword;
                    break;
                case QBPVRDirectoryType_series:
                    iconRID = self->directoryRIDs.series;
                    break;
                case QBPVRDirectoryType_repeated:
                    iconRID = self->directoryRIDs.repeat;
                    break;
                default:
                    iconRID = self->directoryRIDs.directory;
                    break;
            }
        } else if (SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
            iconRID = self->searchIconRID;
        } else {
            /// categories
            SvString id = QBContentCategoryGetId(category);
            size_t childrenCount = QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category);
            if (SvStringEqualToCString(id, COMPLETED_CATEGORY_ID)) {
                if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->quotaUnits == QBPVRQuotaUnits_bytes) {
                    if (QBPVRProviderCanRecord(self->appGlobals->pvrProvider)) {
                        SvString displayMode = self->quotaDisplayMode;
                        if (!displayMode)
                            displayMode = SVSTRING("HD");

                        QBPVRQuota quota = QBPVRProviderGetCurrentQuota(self->appGlobals->pvrProvider);
                        int bytesPerSecSD = QBPVRLogicGetSDVideoAverageBitrate(self->appGlobals->pvrLogic) / 8;
                        int bytesPerSecHD = QBPVRLogicGetHDVideoAverageBitrate(self->appGlobals->pvrLogic) / 8;
                        int hoursSD = (int) (((quota.limit - quota.used) / bytesPerSecSD / 3600));
                        int hoursHD = (int) (((quota.limit - quota.used) / bytesPerSecHD / 3600));
                        int percent = 0;

                        self->pvrDisplayedAvailBytes = (int64_t) (quota.limit - quota.used);
                        if (quota.limit > 0)
                            percent = 100 - (quota.used * 100) / quota.limit;

                        SvStringBuffer buf = SvStringBufferCreate(NULL);
                        SvStringBufferAppendFormatted(buf, NULL, ngettext("%i recording", "%i recordings", childrenCount), (int) childrenCount);
                        SvStringBufferAppendFormatted(buf, NULL, ", ");
                        if (SvStringEqualToCString(displayMode, "SDHD") && hoursSD) {
                            SvStringBufferAppendFormatted(buf, NULL, ngettext("about %d SD hour or ", "about %d SD hours or ", hoursSD), hoursSD);
                            if (hoursHD)
                                SvStringBufferAppendFormatted(buf, NULL, ngettext("%d HD hour left", "%d HD hours left", hoursHD), hoursHD);
                            else
                                SvStringBufferAppendFormatted(buf, NULL, "%s", gettext("less than HD hour left"));
                        } else if (hoursSD && SvStringEqualToCString(displayMode, "SD")) {
                            SvStringBufferAppendFormatted(buf, NULL, ngettext("about %d SD hour left", "about %d SD hours left", hoursSD), hoursSD);
                        } else if (hoursHD && SvStringEqualToCString(displayMode, "HD")) {
                            SvStringBufferAppendFormatted(buf, NULL, ngettext("about %d HD hour left", "about %d HD hours left", hoursHD), hoursHD);
                        } else if (SvStringEqualToCString(displayMode, "percent")) {
                            SvStringBufferAppendFormatted(buf, NULL, gettext("%i percent left"), percent);
                        } else {
                            SvStringBufferAppendCString(buf, gettext("less than hour left"), NULL);
                        }

                        subcaption = SvStringBufferCreateContentsString(buf, NULL);
                        SVRELEASE(buf);
                    } else {
                        self->pvrDisplayedAvailBytes = 0;
                        subcaption = SvStringCreateWithFormat(ngettext("%i recording", "%i recordings", childrenCount), (int) childrenCount);
                    }
                } else {
                    SvLogError("%s(): not implemented quota units other than bytes.", __func__);
                }
            } else {
                subcaption = SvStringCreateWithFormat(ngettext("%i recording", "%i recordings", childrenCount), (int) childrenCount);
            }

            if (SvStringEqualToCString(id, ONGOING_CATEGORY_ID)) {
                iconRID = self->directoryRIDs.ongoing;
            } else if (SvStringEqualToCString(id, SCHEDULED_CATEGORY_ID)) {
                iconRID = self->directoryRIDs.scheduled;
            } else if (SvStringEqualToCString(id, COMPLETED_CATEGORY_ID)) {
                iconRID = self->directoryRIDs.completed;
            } else if (SvStringEqualToCString(id, FAILED_CATEGORY_ID)) {
                iconRID = self->directoryRIDs.failed;
            }
        }
    } else if (SvObjectIsInstanceOf(node_, QBPVRRecording_getType())) {
        QBPVRRecording recording = (QBPVRRecording) node_;
        caption = QBRecordingUtilsCreateName(self->appGlobals, recording, true, false);
        SvString timeInformation = QBRecordingUtilsCreateTimeInformation(self->appGlobals->pvrProvider,
                                                                         recording);
        SvString availabilityInformation = QBPVRRecordingCreateAvailabilityInformation(recording);
        if (availabilityInformation) {
            subcaption = SvStringCreateWithFormat(gettext("%s, Available to: %s"), SvStringCString(timeInformation), SvStringCString(availabilityInformation));
            SVRELEASE(timeInformation);
            SVRELEASE(availabilityInformation);
        } else {
            subcaption = timeInformation;
        }
        icon = QBPVRRecordingCreateBitmap(self, recording);
        lock = recording->lock;
    } else {
        return NULL;
    }

    assert(caption);
    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->caption = caption;
    item->subcaption = subcaption;
    item->icon = icon;
    item->iconRID = iconRID;

    QBPVRItem itemInfo = QBPVRItemCreate();
    SvWidget ret = QBXMBItemConstructorInitItem(self->itemConstructor, item, app, initialState, (QBXMBItemInfo)itemInfo);
    if(lock) {
        itemInfo->item.positionCallback = QBPVRItemInfoPositionItems;
        itemInfo->lockIcon = svWidgetCreateBitmap(app, self->lockBitmap->width, self->lockBitmap->height, self->lockBitmap);
        assert(itemInfo->lockIcon != NULL);
        svWidgetAttach(ret, itemInfo->lockIcon, 0, 0, 10);
        QBPVRItemInfoPositionItems((QBXMBItemInfo)itemInfo);
        if (initialState == XMBMenuState_inactive)
            itemInfo->lockIcon->alpha = ALPHA_TRANSPARENT;
    }
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBPVRMenuHandlerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    QBPVRItem itemInfo = item_->prv;
    if (itemInfo->lockIcon) {
        if (state == XMBMenuState_inactive)
            itemInfo->lockIcon->alpha = ALPHA_TRANSPARENT;
        else
            itemInfo->lockIcon->alpha = ALPHA_SOLID;
    }
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBPVRMenuHandlerRecordingAdded(SvObject self_,
                               QBPVRRecording rec)
{
}

SvLocal void
QBPVRMenuHandlerRecordingRemoved(SvObject self_,
                                 QBPVRRecording rec)
{
}

SvLocal void
QBPVRMenuHandlerRecordingChanged(SvObject self_,
                                 QBPVRRecording rec,
                                 QBPVRRecording oldRecording)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;

    SVTESTRELEASE(self->options);
    self->options = QBPVRMenuContextCreateSideMenuForRecording(self);

    // after locking we should be on unlock position and vice versa
    if (self->options) {
        SVAUTOSTRING(lockUnlockStr, rec->lock ? "unlock" : "lock");
        QBBasicPaneSetPosition(self->options, lockUnlockStr, true);
        QBContextMenuSwitchPane(self->sidemenu.ctx, (SvGenericObject) self->options);
    }
}

SvLocal void
QBPVRMenuHandlerRecordingRestricted(SvObject self_,
                              QBPVRRecording rec)
{
}

SvLocal void
QBPVRMenuHandlerQuotaChanged(SvObject self_, QBPVRQuota quota)
{
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    if (QBPVRProviderCanRecord(self->appGlobals->pvrProvider)) {
        int64_t diffBytes = llabs(self->pvrDisplayedAvailBytes - (int64_t) (quota.limit - quota.used));
         if (diffBytes < 100*1024*1024)
             return;
    }

    QBContentTreePropagateStaticItemsChange(self->pvrTree, NULL);
}

SvLocal void
QBPVRMenuHandlerDirectoryAdded(SvObject self_, QBPVRDirectory dir)
{
}

SvLocal void
QBPVRMenuHandlerSearchStarted(SvObject self_, QBContentCategory category)
{
    SvErrorInfo error = NULL;
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_progressState, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBPVRMenuHandlerSearchEnded(SvObject self_, QBContentCategory category)
{
    SvErrorInfo error = NULL;
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;

    size_t childrenCount = QBContentCategoryGetLength(category) - QBContentCategoryGetStaticCount(category);
    if (childrenCount) {
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_finishedState, &error);
        if (error) {
            goto err;
        }
        QBSearchProgressDialogHide(self->searchProgressDialog, &error);
        if (error) {
            goto err;
        }
        // go inside search category
        SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
        QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->menuTree, getIterator, self->searchCategoryPath, 0);
        SvObject path = QBTreeIteratorGetCurrentNodePath(&iter);
        SvInvokeInterface(QBMenu, menuBar->prv, setPosition, path, NULL);
    } else {
        QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
        if (error) {
            goto err;
        }
    }
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->searchCategory);
    if (provider) {
        QBContentProviderRemoveListener((QBContentProvider) provider, (SvObject) self);
        SVRELEASE(provider);
    }
    return;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
}

SvLocal void
QBPVRMenuHandlerErrorOccured(SvObject self_, SvErrorInfo errorInfo)
{
    SvErrorInfo error = NULL;
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    QBSearchProgressDialogSetState(self->searchProgressDialog, QBSearchProgressDialog_noResultState, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVTESTRELEASE(self->searchProgressDialog);
        self->searchProgressDialog = NULL;
    }
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->searchCategory);
    if (provider) {
        QBContentProviderRemoveListener((QBContentProvider) provider, (SvObject) self);
        SVRELEASE(provider);
    }
}

SvLocal void
QBPVRMenuHandlerOkCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    QBPVRMenuPushOSKPane(self);
}

SvLocal void
QBPVRMenuHandlerCancelCallback(SvObject self_, QBSearchProgressDialog searchProgressDialog)
{
    SvErrorInfo error = NULL;
    QBPVRMenuHandler self = (QBPVRMenuHandler) self_;
    QBSearchProgressDialogHide(searchProgressDialog, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVTESTRELEASE(self->searchProgressDialog);
    self->searchProgressDialog = NULL;
    SvObject provider = QBContentCategoryTakeProvider((QBContentCategory) self->searchCategory);
    if (provider) {
        QBContentProviderStop((QBContentProvider) provider);
        QBContentProviderClear((QBContentProvider) provider);
        SVRELEASE(provider);
    }
    QBPVRMenuPushOSKPane(self);
}

SvLocal SvType
QBPVRMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVRMenuHandlerDestroy
    };
    static const struct QBMenuEventHandler_ eventHandlerMethods = {
        .nodeSelected = QBPVRMenuNodeSelected,
    };
    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBPVRMenuContextChoosen
    };
    static const struct XMBItemController_t methods = {
        .createItem   = QBPVRMenuHandlerCreateItem,
        .setItemState = QBPVRMenuHandlerSetItemState
    };
    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded      = QBPVRMenuHandlerRecordingAdded,
        .recordingChanged    = QBPVRMenuHandlerRecordingChanged,
        .recordingRemoved    = QBPVRMenuHandlerRecordingRemoved,
        .recordingRestricted = QBPVRMenuHandlerRecordingRestricted,
        .quotaChanged        = QBPVRMenuHandlerQuotaChanged,
        .directoryAdded      = QBPVRMenuHandlerDirectoryAdded,
    };
    static const struct QBContentProviderListener_ contentProviderListener = {
        .activityStarted = QBPVRMenuHandlerSearchStarted,
        .activityEnded   = QBPVRMenuHandlerSearchEnded,
        .errorOccured    = QBPVRMenuHandlerErrorOccured
    };
    static const struct QBSearchProgressDialogListener_ searchProgressDialogListener = {
        .onOkPressed     = QBPVRMenuHandlerOkCallback,
        .onCancelPressed = QBPVRMenuHandlerCancelCallback
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRMenuHandler",
                            sizeof(struct QBPVRMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &eventHandlerMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            XMBItemController_getInterface(), &methods,
                            QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                            QBContentProviderListener_getInterface(), &contentProviderListener,
                            QBSearchProgressDialogListener_getInterface(), &searchProgressDialogListener,
                            NULL);
    }

    return type;
}

void
QBPVRMenuRegister(SvWidget menuBar,
                  QBTreePathMap pathMap,
                  AppGlobals appGlobals)
{
    QBPVRMenuHandler handler;
    SvGenericObject path = NULL;

    if (!appGlobals->pvrProvider) {
        // PVR not supported
        return;
    }

    if (!appGlobals->pvrAgent) {
        // PVR not supported
        return;
    }

    const QBPVRProviderCapabilities *providerCapabilities = QBPVRProviderGetCapabilities(appGlobals->pvrProvider);
    if (providerCapabilities->type != QBPVRProviderType_disk) {
        return;
    }

    if (!(path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("PVR")))) {
        return;
    }

    handler = (QBPVRMenuHandler) SvTypeAllocateInstance(QBPVRMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;
    handler->menuTree = SVRETAIN(appGlobals->menuTree);

    handler->pvrTree = SVRETAIN(QBPVRProviderGetTree(appGlobals->pvrProvider));
    QBActiveTreeMountSubTree(appGlobals->menuTree, (SvObject) handler->pvrTree, path, NULL);

    QBGUILogicPVRMenuContextRegisterCallbacks(appGlobals->guiLogic, (SvGenericObject)handler);


    svSettingsPushComponent("Pvr.settings");
    handler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    handler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        handler->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");

    handler->searchIconRID = svSettingsGetResourceID("PVRRecordingBitmap", "search");

    handler->directoryRIDs.scheduled = svSettingsGetResourceID("DirectoryBitmaps",
                                                               "scheduled");
    handler->directoryRIDs.completed = svSettingsGetResourceID("DirectoryBitmaps",
                                                               "completed");
    handler->directoryRIDs.ongoing = svSettingsGetResourceID("DirectoryBitmaps",
                                                             "ongoing");
    handler->directoryRIDs.failed = svSettingsGetResourceID("DirectoryBitmaps",
                                                            "failed");

    handler->directoryRIDs.keyword = svSettingsGetResourceID("DirectoryBitmaps",
                                                             "keyword");
    handler->directoryRIDs.series = svSettingsGetResourceID("DirectoryBitmaps",
                                                            "series");
    handler->directoryRIDs.repeat = svSettingsGetResourceID("DirectoryBitmaps",
                                                            "repeat");
    handler->directoryRIDs.directory = svSettingsGetResourceID("DirectoryBitmaps",
                                                               "directory");

    handler->bitmaps[QBPVRRecordingBitmap_Completed] = svSettingsGetBitmap("PVRRecordingBitmap", "ok");
    handler->bitmaps[QBPVRRecordingBitmap_Recording] = svSettingsGetBitmap("PVRRecordingBitmap", "recording");
    handler->bitmaps[QBPVRRecordingBitmap_Error] = svSettingsGetBitmap("PVRRecordingBitmap", "error");
    handler->bitmaps[QBPVRRecordingBitmap_Scheduled] = svSettingsGetBitmap("PVRRecordingBitmap", "scheduled");
    handler->bitmaps[QBPVRRecordingBitmap_Interrupted] = svSettingsGetBitmap("PVRRecordingBitmap", "interrupted");
    handler->lockBitmap = svSettingsGetBitmap("PVRRecordingBitmap", "lock");

    const char *quotaDisplayMode = svSettingsGetString("quota", "displayMode");
    handler->quotaDisplayMode = quotaDisplayMode ? SvStringCreate(quotaDisplayMode, NULL) : NULL;

    SVTESTRETAIN(handler->lockBitmap);

    svSettingsPopComponent();

    handler->selectedNode = NULL;
    handler->selectedNodePath = NULL;

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) handler, NULL);

    handler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenuRecordings.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenuRecordings"));
    QBPVRProviderAddListener(appGlobals->pvrProvider, (SvObject) handler);

    SVRELEASE(handler);

    SVRELEASE(path);
}
