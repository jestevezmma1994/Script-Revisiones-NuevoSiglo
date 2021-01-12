/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBNPvrRecordingEditPane.h"

#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBNPvrMarginsPane.h>
#include <ContextMenus/QBNPvrOptionsSelectPane.h>
#include <ContextMenus/QBOSKPane.h>
#include <Logic/NPvrLogic.h>
#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <QBConf.h>
#include <QBInput/QBInputCodes.h>
#include <QBOSK/QBOSKKey.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <Utils/QBEventUtils.h>
#include <stringUtils.h>
#include <Widgets/npvrDialog.h>
#include <libintl.h>
#include <main.h>
#include <settings.h>

#define MIN_DIRECTORY_LENGTH 3

struct QBNPvrRecordingEditPane_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContextMenu contextMenu;
    int level;
    QBNPvrRecordingEditPaneType paneType;
    int settingsContext;
    SvEPGEvent event;

    QBBasicPane options;

    struct {
        QBBasicPaneItem premiereOption;
        QBBasicPaneItem spaceRecoveryOption;
        QBBasicPaneItem npvrMargins;
        QBBasicPaneItem name;
        QBBasicPaneItem save;
    } items;

    QBOSKPane oskPane;
    QBNPvrOptionsSelectPane premiereModePane;
    QBNPvrOptionsSelectPane spaceRecoveryModePane;
    QBNPvrMarginsPane npvrMarginsPane;

    struct {
        SvString name;
        bool premiereOption;
        QBnPVRRecordingSpaceRecovery recoveryOption;
    } recordInfo;
};

SvLocal void QBNPvrRecordingEditPane__dtor__(void* self_)
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) self_;

    QBContextMenuSetInputEventHandler(self->contextMenu, NULL, NULL);

    SVRELEASE(self->options);
    SVTESTRELEASE(self->oskPane);
    SVTESTRELEASE(self->premiereModePane);
    SVTESTRELEASE(self->spaceRecoveryModePane);
    SVTESTRELEASE(self->npvrMarginsPane);
    SVTESTRELEASE(self->recordInfo.name);
    SVTESTRELEASE(self->event);
}

SvLocal SvType QBNPvrRecordingEditPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrRecordingEditPane__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrRecordingEditPane",
                            sizeof(struct QBNPvrRecordingEditPane_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBNPvrRecordingEditPaneUpdateItems(QBNPvrRecordingEditPane self)
{
    //nPVR margins
    if (QBNPvrLogicIsSetMarginsFeatureEnabled()) {
        const char* startMargin = QBConfigGet("NPVRSTARTMARGIN");
        const char* endMargin = QBConfigGet("NPVRENDMARGIN");

        SvString marginsString = SvStringCreateWithFormat(gettext("Start: %s min, End: %s min"), startMargin, endMargin);

        if (!SvObjectEquals((SvObject) marginsString, (SvObject) self->items.npvrMargins->subcaption)) {
            SVTESTRELEASE(self->items.npvrMargins->subcaption);
            self->items.npvrMargins->subcaption = SVRETAIN(marginsString);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.npvrMargins);
        }

        SVRELEASE(marginsString);
    }

    //nPVR space recovery
    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        QBnPVRRecordingSpaceRecovery recoveryOption = self->recordInfo.recoveryOption;
        SvString recoveryOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_SpaceRecovery, (int) recoveryOption);

        if (!SvObjectEquals((SvObject) recoveryOptionStr, (SvObject) self->items.spaceRecoveryOption->subcaption)) {
            SVTESTRELEASE(self->items.spaceRecoveryOption->subcaption);
            self->items.spaceRecoveryOption->subcaption = SVRETAIN(recoveryOptionStr);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.spaceRecoveryOption);
        }

        SVRELEASE(recoveryOptionStr);
    }

    if (self->paneType == QBNPvrRecordingEditPaneType_Series) {
        //name
        SvString name = self->recordInfo.name;
        if (!SvObjectEquals((SvObject) name, (SvObject) self->items.name->subcaption)) {
            SVTESTRELEASE(self->items.name->subcaption);
            self->items.name->subcaption = SVTESTRETAIN(name);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.name);
        }

        //premiere
        if (QBNPvrLogicIsPremiereFeatureEnabled()) {
            bool premiereOption = self->recordInfo.premiereOption;
            SvString premiereOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_Premiere, (int) premiereOption);

            if (!SvObjectEquals((SvObject) premiereOptionStr, (SvObject) self->items.premiereOption->subcaption)) {
                SVTESTRELEASE(self->items.premiereOption->subcaption);
                self->items.premiereOption->subcaption = SVRETAIN(premiereOptionStr);
                QBBasicPaneOptionPropagateObjectChange(self->options, self->items.premiereOption);
            }

            SVRELEASE(premiereOptionStr);
        }
    }
}

SvLocal void QBNPvrRecordingEditPaneSeriesNameKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBNPvrRecordingEditPane self = ptr;
    if (key->type == QBOSKKeyType_enter) {
        SVTESTRELEASE(self->recordInfo.name);
        self->recordInfo.name = SVRETAIN(input);
        QBContextMenuPopPane(self->contextMenu);
        QBNPvrRecordingEditPaneUpdateItems(self);
    }
}

SvLocal void QBNPvrRecordingEditPaneSetSeriesNameValue(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    if (!self->oskPane) {
        svSettingsRestoreContext(self->settingsContext);
        self->oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
        SvErrorInfo error = NULL;
        QBOSKPaneInit(self->oskPane, self->appGlobals->scheduler, self->contextMenu, self->level + 1, SVSTRING("OSKPane"), QBNPvrRecordingEditPaneSeriesNameKeyTyped, self, &error);
        svSettingsPopComponent();
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            return;
        }
    }
    QBOSKPaneSetInput(self->oskPane, self->recordInfo.name);

    QBContextMenuPushPane(self->contextMenu, (SvObject) self->oskPane);
}

SvLocal void QBNPvrRecordingEditPaneNpvrPremiereApproveCallback(void* self_, int optionValue)
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) self_;
    self->recordInfo.premiereOption = (optionValue != 0);

    QBNPvrRecordingEditPaneUpdateItems(self);
}

SvLocal void QBNPvrRecordingEditPaneChangePremiere(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    if (!self->premiereModePane) {
        self->premiereModePane = QBNPvrOptionsSelectPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1,
            QBNPvrOptionsSelectPaneType_Premiere);

        QBNPvrOptionsSelectPaneSetApproveCallback(self->premiereModePane, QBNPvrRecordingEditPaneNpvrPremiereApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->premiereModePane, (int) self->recordInfo.premiereOption);
}

SvLocal void QBNPvrRecordingEditPaneNpvrSpaceRecoveryApproveCallback(void* self_, int optionValue)
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) self_;
    if (optionValue == QBnPVRRecordingSpaceRecovery_DeleteOldest) {
        self->recordInfo.recoveryOption = QBnPVRRecordingSpaceRecovery_DeleteOldest;
    } else if (optionValue == QBnPVRRecordingSpaceRecovery_Manual) {
        self->recordInfo.recoveryOption = QBnPVRRecordingSpaceRecovery_Manual;
    }

    QBNPvrRecordingEditPaneUpdateItems(self);
}

SvLocal void QBNPvrRecordingEditPaneChangeSpaceRecovery(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    if (!self->spaceRecoveryModePane) {
        self->spaceRecoveryModePane = QBNPvrOptionsSelectPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1,
            QBNPvrOptionsSelectPaneType_SpaceRecovery);

        QBNPvrOptionsSelectPaneSetApproveCallback(self->spaceRecoveryModePane, QBNPvrRecordingEditPaneNpvrSpaceRecoveryApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->spaceRecoveryModePane, (int) self->recordInfo.recoveryOption);
}

SvLocal void QBNPvrRecordingEditPaneNpvrMarginsHideCallback(void* self_)
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) self_;
    QBNPvrRecordingEditPaneUpdateItems(self);
}

SvLocal void QBNPvrRecordingEditPaneSetNpvrMargins(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    if (!self->npvrMarginsPane) {
        self->npvrMarginsPane = QBNPvrMarginsPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1);

        QBNPvrMarginsPaneSetApproveCallback(self->npvrMarginsPane, QBNPvrRecordingEditPaneNpvrMarginsHideCallback, self);
    }
    QBNPvrMarginsPaneShow(self->npvrMarginsPane);
}

SvLocal void QBNPvrRecordingEditPaneConfirmSave(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    struct QBnPVRRecordingSchedParams_ params = {
        .type = QBnPVRRecordingType_none,
    };

    if (self->paneType == QBNPvrRecordingEditPaneType_Manual) {
        params.type = QBnPVRRecordingType_manual;
    } else if (self->paneType == QBNPvrRecordingEditPaneType_Series) {
        params.type = QBnPVRRecordingType_series;
        params.seriesID = self->recordInfo.name;
    }
    params.event = self->event;
    if (QBNPvrLogicIsPremiereFeatureEnabled()) {
        params.recordPremieresOnly = SvValueCreateWithBoolean(self->recordInfo.premiereOption, NULL);
    }
    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        params.recoveryOption = SvValueCreateWithInteger(self->recordInfo.recoveryOption, NULL);
    }

    QBnPVRProviderRequest request = NULL;
    if (self->paneType == QBNPvrRecordingEditPaneType_Manual) {
        request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleRecording, &params);
        QBNPvrAgentTrackRequest(self->appGlobals->npvrAgent, request, self->event); //TODO check if necessary
    } else if (self->paneType == QBNPvrRecordingEditPaneType_Series) {
        request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleSeries, &params);
    }

    SVTESTRELEASE(params.recordPremieresOnly);
    SVTESTRELEASE(params.recoveryOption);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBNPvrRecordingEditPaneConfirmCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBNPvrRecordingEditPaneShowError(QBNPvrRecordingEditPane self, const char* message)
{
    svSettingsRestoreContext(self->settingsContext);
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, self->level + 1, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(message, NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBNPvrRecordingEditPaneConfirmCancel, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->contextMenu, (SvObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal bool QBNPvrRecordingEditPaneValidateSeriesDirectory(QBNPvrRecordingEditPane self)
{
    const char *msg = NULL;
    char *dynamicMessage = NULL;

    if (SvStringLength(self->recordInfo.name) == 0) {
        msg = gettext("Series directory name cannot be empty");
        goto err;
    } else if (SvStringLength(self->recordInfo.name) < MIN_DIRECTORY_LENGTH) {
        asprintf(&dynamicMessage, gettext("Series directory name must be at least %d characters"), MIN_DIRECTORY_LENGTH);
        msg = dynamicMessage;
        goto err;
    } else if (QBStringUtilsIsWhitespace(SvStringCString(self->recordInfo.name))) {
        msg = "Directory name cannot start with a space";
        goto err;
    }

    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);

    const char* nameCStr = SvStringCString(self->recordInfo.name);
    SvIterator it = SvArrayIterator(dirList);
    QBnPVRDirectory dir = NULL;
    while ((dir = (QBnPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(dir->name, nameCStr)) {
            SVRELEASE(dirList);
            asprintf(&dynamicMessage, gettext("Directory '%s' already exists"), nameCStr);
            msg = dynamicMessage;
            goto err;
        }
    }
    SVRELEASE(dirList);

    return true;

err:
    QBNPvrRecordingEditPaneShowError(self, msg);
    free(dynamicMessage);
    return false;
}

SvLocal void QBNPvrRecordingEditPaneSave(QBNPvrRecordingEditPane self)
{
    if (self->paneType == QBNPvrRecordingEditPaneType_Series && !QBNPvrRecordingEditPaneValidateSeriesDirectory(self)) {
        return;
    }

    //if the recording is customizable then we don't need additional confirmation
    if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals)) {
        QBNPvrRecordingEditPaneConfirmSave(self, NULL, NULL, NULL);
        return;
    }

    svSettingsRestoreContext(self->settingsContext);
    QBBasicPane confirmation = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(confirmation, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, self->level + 1, SVSTRING("BasicPane"));
    SvString option = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(confirmation, SVSTRING("save"), option, QBNPvrRecordingEditPaneConfirmSave, self);
    SVRELEASE(option);
    option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(confirmation, SVSTRING("cancel"), option, QBNPvrRecordingEditPaneConfirmCancel, self);
    SVRELEASE(option);
    svSettingsPopComponent();
    QBContextMenuPushPane(self->contextMenu, (SvObject) confirmation);
    SVRELEASE(confirmation);
}

SvLocal void QBNPvrRecordingEditPaneSavePressed(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) self_;
    QBNPvrRecordingEditPaneSave(self);
}

SvLocal bool QBNPvrRecordingEditPaneInputEventHandler(void *self_, SvObject src, SvInputEvent e)
{
    QBNPvrRecordingEditPane self = self_;
    if (src == (SvObject) self->options) {
        if (e->ch == QBKEY_RIGHT) {
            QBNPvrRecordingEditPaneSave(self);
            return true;
        }
    }
    return false;
}

QBNPvrRecordingEditPane QBNPvrRecordingEditPaneCreate(
    int settingsContext,
    AppGlobals appGlobals,
    QBContextMenu ctxMenu,
    int level,
    QBNPvrRecordingEditPaneType paneType
    )
{
    QBNPvrRecordingEditPane self = (QBNPvrRecordingEditPane) SvTypeAllocateInstance(
        QBNPvrRecordingEditPane_getType(),
        NULL
        );

    self->appGlobals = appGlobals;
    self->contextMenu = ctxMenu;
    self->level = level;
    self->paneType = paneType;
    self->settingsContext = settingsContext;

    svSettingsRestoreContext(self->settingsContext);

    self->options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(
        self->options,
        self->appGlobals->res,
        self->appGlobals->scheduler,
        self->appGlobals->textRenderer,
        self->contextMenu,
        self->level,
        SVSTRING("BasicPane")
        );

    SvString name = NULL;

    name = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(self->options, SVSTRING("save"), name, QBNPvrRecordingEditPaneSavePressed, self);
    SVRELEASE(name);

    if (self->paneType == QBNPvrRecordingEditPaneType_Series) {
        name = SvStringCreate(gettext("Series directory name"), NULL);
        self->items.name = QBBasicPaneAddOption(self->options, SVSTRING("seriesDirName"), name, QBNPvrRecordingEditPaneSetSeriesNameValue, self);
        SVRELEASE(name);

        if (QBNPvrLogicIsPremiereFeatureEnabled()) {
            name = SvStringCreate(gettext("Premiere"), NULL);
            self->items.premiereOption = QBBasicPaneAddOption(self->options, SVSTRING("premiere"), name, QBNPvrRecordingEditPaneChangePremiere, self);
            SVRELEASE(name);
        }
    }

    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        name = SvStringCreate(gettext("Space recovery"), NULL);
        self->items.spaceRecoveryOption = QBBasicPaneAddOption(self->options, SVSTRING("spaceRecovery"), name, QBNPvrRecordingEditPaneChangeSpaceRecovery, self);
        SVRELEASE(name);
    }

    if (QBNPvrLogicIsSetMarginsFeatureEnabled()) {
        name = SvStringCreate(gettext("nPVR margins"), NULL);
        self->items.npvrMargins = QBBasicPaneAddOption(self->options, SVSTRING("NpvrMargins"), name, QBNPvrRecordingEditPaneSetNpvrMargins, self);
        SVRELEASE(name);
    }

    svSettingsPopComponent();

    QBContextMenuSetInputEventHandler(self->contextMenu, self, QBNPvrRecordingEditPaneInputEventHandler);

    return self;
}

void QBNPvrRecordingEditPaneSetEvent(QBNPvrRecordingEditPane self, SvEPGEvent event)
{
    SvErrorInfo error = NULL;

    if (!event) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed");
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return;
    }

    SVTESTRELEASE(self->event);
    self->event = SVRETAIN(event);

    if (self->paneType == QBNPvrRecordingEditPaneType_Series) {
        SvString episodeName = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic,
                                                             self->appGlobals->langPreferences,
                                                             self->event);
        SVTESTRELEASE(self->recordInfo.name);
        self->recordInfo.name = SVRETAIN(episodeName);
    }
}

void QBNPvrRecordingEditPaneShow(QBNPvrRecordingEditPane self)
{
    QBNPvrRecordingEditPaneUpdateItems(self);
    QBBasicPaneSetPosition(self->options, SVSTRING("save"), true);
    QBContextMenuPushPane(self->contextMenu, (SvObject) self->options);
}
