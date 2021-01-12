/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRecordingEditPane.h"

#include <libintl.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBListProxy.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputCodes.h>
#include <main.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <QBOSK/QBOSKKey.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBNPvrMarginsPane.h>
#include <ContextMenus/QBNPvrOptionsSelectPane.h>
#include <Logic/NPvrLogic.h>
#include <Menus/QBFavoritesMenu.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/QBCalendar.h>
#include <Widgets/npvrDialog.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/mainmenu.h>
#include <Windows/newtv.h>
#include <Widgets/QBTimeInput.h>
#include <Widgets/QBSimpleDialog.h>
#include <Services/QBPVRConflictResolver.h>
#include <Services/QBCASManager.h>
#include <Logic/timeFormat.h>
#include <QBPlayerControllers/utils/content.h>
#include <Utils/QBEventUtils.h>
#include <stringUtils.h>
#include <QBStringUtils.h>
#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRProvider.h>
#include <XMB2/XMBVerticalMenu.h>
#include <settings.h>
#include <QBConf.h>
#include <SvFoundation/SvData.h>
#include <SvJSON/SvJSONParse.h>
#include <SvQuirks/SvRuntimePrefix.h>

#define log_error(fmt, ...)  SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(red), __func__, __LINE__, ## __VA_ARGS__)
#define log_debug(fmt, ...)  if (0) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__)

#define MIN_KEYWORD_LENGTH      3
#define OPTIONS_CONF_MAX_FILE_SIZE 32768

typedef enum {
    RecordingEditPaneOskValidatorStatus_OK,
    RecordingEditPaneOskValidatorStatus_EMPTY,
    RecordingEditPaneOskValidatorStatus_EXISTS
} RecordingEditPaneOskValidatorStatus;

struct QBRecordingEditPane_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBContextMenu contextMenu;
    int level;

    SvWidget sideMenu;

    QBBasicPane options;

    PvrType pvrType;

    bool shown;

    int settingsCtx;

    QBListPane channelSelectionPane;

    SvWidget timeEdit, startTime, endTime;
    QBContainerPane timePane;
    int timeEditOffX;
    int timeEditOffY;
    SvString optionID;

    struct {
        SvTVChannel channel;
        SvLocalTime startTime, endTime;
        SvString keyword;
        bool premiereOption;
        QBnPVRRecordingSpaceRecovery recoveryOption;
        SvString name;
        QBPVRRepeatedRecordingsMode repeat;
    } recordingInfo;

    QBActiveArray sortDataSource;
    QBListPane sortingPane;

    struct {
        QBBasicPaneItem channel;
        QBBasicPaneItem startDate;
        QBBasicPaneItem startTime;
        QBBasicPaneItem endTime;
        QBBasicPaneItem keyword;
        QBBasicPaneItem premiereOption;
        QBBasicPaneItem spaceRecoveryOption;
        QBBasicPaneItem npvrMargins;
        QBBasicPaneItem repeat;
        QBBasicPaneItem duration;
    } items;


    SvWidget calendar;
    SvWidget calendarBg;
    int calendarBgOffX;
    int calendarBgOffY;
    QBOSKRenderer dateEditRenderer;
    QBContainerPane startDatePane;

    QBPVRRecording recording;

    QBRecordingEditPaneType recordingType;
    QBOSKPane oskPane;

    QBPVRConflictResolver resolver;

    SvWidget timeEditWarning;

    QBNPvrOptionsSelectPane premiereModePane;
    QBNPvrOptionsSelectPane spaceRecoveryModePane;

    QBNPvrMarginsPane npvrMarginsPane;
};

/*
 * Typical stuff for pane management
 */

SvLocal void QBRecordingEditPaneDestroy(void *self_)
{
    QBRecordingEditPane self = self_;

    if (self->resolver) {
        SVRELEASE(self->resolver);
    }

    QBContextMenuSetInputEventHandler(self->contextMenu, NULL, NULL);
    SVTESTRELEASE(self->options);
    SVTESTRELEASE(self->channelSelectionPane);
    SVTESTRELEASE(self->recording);
    if (self->dateEditRenderer) {
        QBOSKRendererStop(self->dateEditRenderer, NULL);
        SVRELEASE(self->dateEditRenderer);
    }
    svWidgetDestroy(self->calendarBg);
    SVTESTRELEASE(self->oskPane);
    SVTESTRELEASE(self->recordingInfo.channel);
    SVTESTRELEASE(self->recordingInfo.keyword);
    SVTESTRELEASE(self->recordingInfo.name);

    svWidgetDestroy(self->timeEdit);
    SVTESTRELEASE(self->timePane);
    SVTESTRELEASE(self->optionID);
    SVTESTRELEASE(self->startDatePane);
    SVTESTRELEASE(self->premiereModePane);
    SVTESTRELEASE(self->spaceRecoveryModePane);
    SVTESTRELEASE(self->npvrMarginsPane);
}

SvLocal void QBRecordingEditPaneShow(SvGenericObject self_)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, show);
}

SvLocal void QBRecordingEditPaneHide(SvGenericObject self_, bool immediately)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, hide, immediately);
}

SvLocal void QBRecordingEditPaneSetActive(SvGenericObject self_)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, setActive);
}

SvLocal bool QBRecordingEditPaneValidateDirectoryName(QBRecordingEditPane self);

SvLocal bool QBRecordingEditPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void QBRecordingEditUpdateItems(QBRecordingEditPane self);

SvLocal void QBRecordingEditTimeEditWarningPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBRecordingEditPane self = self_;
    self->timeEditWarning = NULL;
}

SvLocal SvWidget QBRecordingEditPaneCreateTimeWarningPopup(QBRecordingEditPane self)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params,
                                           gettext("Stop time can not be set before current time"),
                                           gettext("PVR end time setup - warning"));
    svSettingsPopComponent();
    return dialog;
}

SvLocal void QBRecordingEditPaneHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;

    if (SvObjectIsInstanceOf((SvObject) event_, QBCalendarDateSelectedEvent_getType())) {
        QBCalendarDateSelectedEvent event = (QBCalendarDateSelectedEvent) event_;

        struct tm local = SvLocalTimeToLocal(self->recordingInfo.startTime);
        local.tm_mday = event->selectedDate.mday;
        local.tm_mon = event->selectedDate.mon;
        local.tm_year = event->selectedDate.year - 1900;
        log_debug("day=%i month=%i year=%i", local.tm_mday, local.tm_mon, local.tm_year);

        int length = SvLocalTimeDiff(self->recordingInfo.endTime, self->recordingInfo.startTime);

        SvLocalTime startTime = SvLocalTimeFromLocal(&local);
        SvLocalTime endTime = SvLocalTimeAdd(startTime, length);
        SvLocalTime currentTimeLocal = SvLocalTimeFromUTC(SvTimeNow());

        time_t endTimeSec = SvLocalTimeToInt(endTime);
        time_t currentTimeSec = SvLocalTimeToInt(currentTimeLocal);

        if (endTimeSec < currentTimeSec) {
            self->timeEditWarning = QBRecordingEditPaneCreateTimeWarningPopup(self);
            QBDialogRun(self->timeEditWarning, self, QBRecordingEditTimeEditWarningPopupCallback);
        } else {
            self->recordingInfo.startTime = startTime;
            self->recordingInfo.endTime = endTime;
        }

        QBRecordingEditUpdateItems(self);
        QBContextMenuPopPane(self->contextMenu);
    }
}

SvType QBRecordingEditPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRecordingEditPaneDestroy
    };
    static const struct QBContextMenuPane_ paneMethods = {
        .show             = QBRecordingEditPaneShow,
        .hide             = QBRecordingEditPaneHide,
        .setActive        = QBRecordingEditPaneSetActive,
        .handleInputEvent = QBRecordingEditPaneHandleInputEvent
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBRecordingEditPaneHandlePeerEvent
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRecordingEditPane",
                            sizeof(struct QBRecordingEditPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &paneMethods,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }
    return type;
}

/*
 * Input event handler
 */
SvLocal void QBRecordingEditConfirm(QBRecordingEditPane self);

SvLocal bool QBRecordingEditPaneGlobalInputEventHandler(void *self_, SvGenericObject src, SvInputEvent e)
{
    QBRecordingEditPane self = self_;
    if (src == (SvGenericObject) self->options) {
        if (e->ch == QBKEY_RIGHT) {
            QBRecordingEditConfirm(self);
            return true;
        }
    }

#if 0
    if (src == (SvGenericObject) self->channelSelectionPane) {
        if (e->ch == QBKEY_LEFT || e->ch == QBKEY_RIGHT) {
            QBRecordingEditPaneAddRemoveChannelsConfirm(self);
            return true;
        }
    }
    if (src == (SvGenericObject) self->sortingPane) {
        if (e->ch == QBKEY_LEFT || e->ch == QBKEY_RIGHT) {
            QBRecordingEditPaneSortConfirm(self);
            return true;
        }
    }
#endif
    return false;
}

SvLocal void QBRecordingEditConfirmCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    log_debug();
    QBRecordingEditPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal bool QBRecordingEditPaneValidateDirectoryName(QBRecordingEditPane self)
{
    SvArray dirList = QBPVRProviderCreateDirectoriesList(self->appGlobals->pvrProvider);
    if (!dirList)
        return true;

    SvIterator it = SvArrayIterator(dirList);
    QBPVRDirectory dir = NULL;
    while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) QBContentCategoryGetName((QBContentCategory) dir), (SvObject) self->recordingInfo.keyword)) {
            SVRELEASE(dirList);
            return false;
        }
    }
    SVRELEASE(dirList);

    return true;
}

SvLocal bool QBRecordingEditPaneValidateNpvrDirectoryName(QBRecordingEditPane self, SvString directoryName, SvString *errorMessageOut)
{
    bool ret = true;
    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);

    SvIterator it = SvArrayIterator(dirList);
    QBnPVRDirectory dir = NULL;
    while ((dir = (QBnPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) dir->name, (SvObject) directoryName)) {
            *errorMessageOut = SvStringCreateWithFormat(gettext("Directory '%s' already exists"), SvStringCString(directoryName));
            ret = false;
        }
    }

    SVRELEASE(dirList);
    return ret;
}

SvLocal void QBRecordingEditPaneShowError(QBRecordingEditPane self, const char *message)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res,
                    self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->contextMenu, self->level + 1, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(message, NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBRecordingEditConfirmCancel, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) options);
    SVRELEASE(options);
    svSettingsPopComponent();
}

SvLocal bool QBRecordingEditPaneValidate(QBRecordingEditPane self)
{
    const char *msg = NULL;
    SvString npvrErrorOut = NULL;

    if (self->recordingType == QBRecordingEditPane_Keyword) {
        if (!self->recordingInfo.keyword || QBUTF8StringGetLength(SvStringCString(self->recordingInfo.keyword)) < MIN_KEYWORD_LENGTH) {
            msg = gettext("Unable to save. Keyword is too short.");
            goto err;
        }
        if (QBStringUtilsIsWhitespace(SvStringCString(self->recordingInfo.keyword))) {
            msg = gettext("Unable to save. Keyword is incorrect.");
            goto err;
        }
        if ((self->pvrType == PvrType_PVR) && QBPVRProviderCheckDirectoryLimit(self->appGlobals->pvrProvider, QBPVRDirectoryType_keyword)) {
            msg = gettext("You have too many directories and keyword recordings.");
            goto err;
        }
        if (self->pvrType == PvrType_NPVR && !QBRecordingEditPaneValidateNpvrDirectoryName(self, self->recordingInfo.keyword, &npvrErrorOut)) {
            msg = SvStringCString(npvrErrorOut);
            goto err;
        }
        if (!QBRecordingEditPaneValidateDirectoryName(self)) {
            msg = gettext("Unable to save. Directory name exists.");
            goto err;
        }
    } else if (self->recordingType == QBRecordingEditPane_Manual) {
        if (QBPVRProviderCheckDirectoryLimit(self->appGlobals->pvrProvider, QBPVRDirectoryType_repeated)) {
            msg = gettext("You have too many directories and scheduled recordings.");
            goto err;
        }
        if (QBPVRProviderCheckScheduledLimit(self->appGlobals->pvrProvider)) {
            msg = gettext("Too many simultaneous recordings.");
            goto err;
        }
    }
    return true;

err:
    QBRecordingEditPaneShowError(self, msg);

    SVTESTRELEASE(npvrErrorOut);
    return false;
}

SvLocal void QBRecordingEditPaneOnResolveFinish(void *self_, bool saved)
{
    QBRecordingEditPane self = self_;

    if (saved)
        QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBRecordingEditConfirmSave(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    log_debug();
    QBRecordingEditPane self = self_;
    if (!QBRecordingEditPaneValidate(self)) {
        return;
    }

    if (self->appGlobals->casPVRManager) {
        SvInvokeInterface(QBCASPVRManager, self->appGlobals->casPVRManager, recordingAdded, "QBRecordingEditPane");
    }

    if (self->recordingType == QBRecordingEditPane_Manual) {
        if (self->recording) {
            QBPVRRecordingUpdateParams params = (QBPVRRecordingUpdateParams)
                                                SvTypeAllocateInstance(QBPVRRecordingUpdateParams_getType(), NULL);

            params->updateTime = true;
            params->startTime = SvTimeConstruct(self->recordingInfo.startTime.utc, 0);
            params->endTime = SvTimeConstruct(self->recordingInfo.endTime.utc, 0);
            QBPVRProviderUpdateRecording(self->appGlobals->pvrProvider,
                                         self->recording,
                                         params);
            SVRELEASE(params);
            QBContextMenuHide(self->contextMenu, false);
        } else {
            if (self->recordingInfo.channel) {
                if (self->recordingInfo.repeat != QBPVRRepeatedRecordings_None) {
                    //Repeated recording
                    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
                                                       SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
                    char buf1[256], buf2[256];
                    struct tm local = SvLocalTimeToLocal(self->recordingInfo.startTime);
                    strftime(buf1, sizeof(buf1), QBTimeFormatGetCurrent()->manualRecordingName, &local);
                    local = SvLocalTimeToLocal(self->recordingInfo.endTime);
                    strftime(buf2, sizeof(buf2), QBTimeFormatGetCurrent()->manualRecordingName, &local);
                    params->name = SvStringCreateWithFormat("%s - %s (%s)", buf1, buf2, SvStringCString(self->recordingInfo.channel->name));

                    params->channel = SVTESTRETAIN(self->recordingInfo.channel);
                    params->repeatedInfo.start = SvTimeConstruct(self->recordingInfo.startTime.utc, 0);
                    params->repeatedInfo.duration = SvLocalTimeDiff(self->recordingInfo.endTime, self->recordingInfo.startTime);
                    params->repeatedInfo.mode = self->recordingInfo.repeat;
                    params->type = QBPVRRecordingType_repeated;

                    QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, params);
                    SVRELEASE(params);
                    QBContextMenuHide(self->contextMenu, false);
                } else {
                    //One time recording
                    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
                                                       SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
                    params->channel = SVTESTRETAIN(self->recordingInfo.channel);
                    params->startTime = SvTimeConstruct(self->recordingInfo.startTime.utc, 0);
                    params->stopTime = SvTimeConstruct(self->recordingInfo.endTime.utc, 0);
                    params->type = QBPVRRecordingType_manual;

                    const char *pvrStartMarginStr = QBConfigGet("PVRSTARTMARGIN");
                    int pvrStartMargin = pvrStartMarginStr ? atoi(pvrStartMarginStr) * 60 : 0;
                    const char *pvrEndMarginStr = QBConfigGet("PVRENDMARGIN");
                    int pvrEndMargin = pvrEndMarginStr ? atoi(pvrEndMarginStr) * 60 : 0;
                    params->startMargin = pvrStartMargin;
                    params->endMargin = pvrEndMargin;

                    SVTESTRELEASE(self->resolver);
                    int currentPaneLevel = QBBasicPaneGetLevel(pane);
                    self->resolver = QBPVRConflictResolverCreate(self->appGlobals, self->contextMenu, params, currentPaneLevel + 1);

                    SVRELEASE(params);
                    QBPVRConflictResolverSetFinishCallback(self->resolver, QBRecordingEditPaneOnResolveFinish, self);
                    QBPVRConflictResolverResolve(self->resolver);
                }
            }
        }
    } else {
        //Keyword
        if (self->pvrType == PvrType_PVR) {
            QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
                                               SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
            params->channel = SVTESTRETAIN(self->recordingInfo.channel);
            params->keyword = SVTESTRETAIN(self->recordingInfo.keyword);
            params->type = QBPVRRecordingType_keyword;
            params->recordPremieresOnly = self->recordingInfo.premiereOption;
            params->recovery = self->recordingInfo.recoveryOption;

            QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, params);
            SVRELEASE(params);
        } else if (self->pvrType == PvrType_NPVR) {
            struct QBnPVRRecordingSchedParams_ params = {
                .type    = QBnPVRRecordingType_keyword,
                .keyword = self->recordingInfo.keyword,
            };
            if (QBNPvrLogicIsPremiereFeatureEnabled()) {
                params.recordPremieresOnly = SvValueCreateWithBoolean(self->recordingInfo.premiereOption, NULL);
            }
            if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
                params.recoveryOption = SvValueCreateWithInteger(self->recordingInfo.recoveryOption, NULL);
            }
            if (self->recordingInfo.channel) {
                SvString channelID = (SvString) SvTVChannelGetAttribute(self->recordingInfo.channel, SVSTRING("productID"));
                params.channelId = SVRETAIN(channelID);
            }

            QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleKeyword, &params);
            QBNPvrDialogCreateAndShow(self->appGlobals, request);

            SVTESTRELEASE(params.channelId);
            SVTESTRELEASE(params.recordPremieresOnly);
            SVTESTRELEASE(params.recoveryOption);
        }

        QBContextMenuHide(self->contextMenu, false);
    }
}

SvLocal void QBRecordingEditConfirm(QBRecordingEditPane self)
{
    if (QBRecordingEditPaneValidate(self)) {
        AppGlobals appGlobals = self->appGlobals;
        svSettingsRestoreContext(self->settingsCtx);
        QBBasicPane confirmation = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(confirmation, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer, self->contextMenu,
                        self->level + 1, SVSTRING("BasicPane"));
        SvString option = SvStringCreate(gettext("Save"), NULL);
        QBBasicPaneAddOption(confirmation, SVSTRING("save"), option, QBRecordingEditConfirmSave, self);
        SVRELEASE(option);
        option = SvStringCreate(gettext("Cancel"), NULL);
        QBBasicPaneAddOption(confirmation, SVSTRING("cancel"), option, QBRecordingEditConfirmCancel, self);
        SVRELEASE(option);
        svSettingsPopComponent();
        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) confirmation);
        SVRELEASE(confirmation);
    }
}

SvLocal void QBRecordingEditUpdateUpdateItemSubcation(QBRecordingEditPane self, QBBasicPaneItem item, SvString subcaption)
{
    if (!SvObjectEquals((SvObject) subcaption, (SvObject) item->subcaption)) {
        SVTESTRELEASE(item->subcaption);
        item->subcaption = SVTESTRETAIN(subcaption);
        QBBasicPaneOptionPropagateObjectChange(self->options, item);
    }
}

SvLocal void QBRecordingEditUpdatePremiereOption(QBRecordingEditPane self)
{
    bool premiereOption = self->recordingInfo.premiereOption;
    SvString premiereOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_Premiere, (int) premiereOption);

    QBRecordingEditUpdateUpdateItemSubcation(self, self->items.premiereOption, premiereOptionStr);

    SVRELEASE(premiereOptionStr);
}

SvLocal void QBRecordingEditUpdateRecoveryOption(QBRecordingEditPane self)
{
    QBnPVRRecordingSpaceRecovery recoveryOption = self->recordingInfo.recoveryOption;
    SvString recoveryOptionStr = TranslateOptionValue(QBNPvrOptionsSelectPaneType_SpaceRecovery, (int) recoveryOption);

    QBRecordingEditUpdateUpdateItemSubcation(self, self->items.spaceRecoveryOption, recoveryOptionStr);

    SVRELEASE(recoveryOptionStr);
}

SvLocal void QBRecordingEditUpdateNpvrMarginsOption(QBRecordingEditPane self)
{
    const char *startMargin = QBConfigGet("NPVRSTARTMARGIN");
    const char *endMargin = QBConfigGet("NPVRENDMARGIN");

    SvString marginsString = SvStringCreateWithFormat(gettext("Start: %s min, End: %s min"), startMargin, endMargin);

    QBRecordingEditUpdateUpdateItemSubcation(self, self->items.npvrMargins, marginsString);

    SVRELEASE(marginsString);
}

SvLocal void QBRecordingEditUpdateItems(QBRecordingEditPane self)
{
    SvString channelName = self->recordingInfo.channel ? SVRETAIN(self->recordingInfo.channel->name) : NULL;
    log_debug("channelName = %s", channelName ? SvStringCString(channelName) :  "(null)");
    if (!channelName) {
        if (self->recordingType == QBRecordingEditPane_Manual)
            channelName = SvStringCreate(gettext("(none)"), NULL);
        else
            channelName = SvStringCreate(gettext("(all)"), NULL);
    }
    if (self->items.channel && !SvObjectEquals((SvObject) channelName, (SvObject) self->items.channel->subcaption)) {
        log_debug("");
        SVTESTRELEASE(self->items.channel->subcaption);
        self->items.channel->subcaption = SVTESTRETAIN(channelName);
        QBBasicPaneOptionPropagateObjectChange(self->options, self->items.channel);
    }
    SVTESTRELEASE(channelName);

    char buf[1024];
    struct tm tm = SvLocalTimeToLocal(self->recordingInfo.startTime);
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->recordingEditPaneTime, &tm);
    log_debug("self->recordingInfo.startTime %s", buf);
    SvString startTime = SvStringCreate(buf, NULL);
    if (self->items.startTime && !SvObjectEquals((SvObject) startTime, (SvObject) self->items.startTime->subcaption)) {
        log_debug("");
        SVTESTRELEASE(self->items.startTime->subcaption);
        self->items.startTime->subcaption = SVTESTRETAIN(startTime);
        QBBasicPaneOptionPropagateObjectChange(self->options, self->items.startTime);
    }
    SVRELEASE(startTime);
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->recordingEditPaneDate, &tm);
    SvString startDate = SvStringCreate(buf, NULL);
    if (self->items.startDate && !SvObjectEquals((SvObject) startDate, (SvObject) self->items.startDate->subcaption)) {
        log_debug("");
        SVTESTRELEASE(self->items.startDate->subcaption);
        self->items.startDate->subcaption = SVTESTRETAIN(startDate);
        QBBasicPaneOptionPropagateObjectChange(self->options, self->items.startDate);
    }
    SVRELEASE(startDate);

    tm = SvLocalTimeToLocal(self->recordingInfo.endTime);
    strftime(buf, sizeof(buf), QBTimeFormatGetCurrent()->recordingEditPaneTime, &tm);
    SvString endTime = SvStringCreate(buf, NULL);
    if (self->items.endTime && !SvObjectEquals((SvObject) endTime, (SvObject) self->items.endTime->subcaption)) {
        log_debug("");
        SVTESTRELEASE(self->items.endTime->subcaption);
        self->items.endTime->subcaption = SVTESTRETAIN(endTime);
        QBBasicPaneOptionPropagateObjectChange(self->options, self->items.endTime);
    }
    SVRELEASE(endTime);

    if (SvLocalTimeDiff(self->recordingInfo.endTime, self->recordingInfo.startTime) >= 0) {
        SvTimeBreakDown(SvTimeConstruct(SvLocalTimeDiff(self->recordingInfo.endTime, self->recordingInfo.startTime), 0), false, &tm);
        strftime(buf, sizeof(buf), "%k:%M", &tm);
        SvString duration = SvStringCreate(buf, NULL);
        if (self->items.duration && !SvObjectEquals((SvObject) duration, (SvObject) self->items.duration->subcaption)) {
            SVTESTRELEASE(self->items.duration->subcaption);
            self->items.duration->subcaption = SVTESTRETAIN(duration);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.duration);
        }
        SVRELEASE(duration);
    } else {
        if (self->items.duration) {
            SVTESTRELEASE(self->items.duration->subcaption);
        }
    }

    if (self->items.keyword) {
        log_debug("");
        SvString keyword = self->recordingInfo.keyword;
        if (!SvObjectEquals((SvObject) keyword, (SvObject) self->items.keyword->subcaption)) {
            log_debug("");
            SVTESTRELEASE(self->items.keyword->subcaption);
            self->items.keyword->subcaption = SVTESTRETAIN(keyword);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.keyword);
        }
    }

    if (QBNPvrLogicIsPremiereFeatureEnabled() && self->items.premiereOption) {
        QBRecordingEditUpdatePremiereOption(self);
    }

    if (QBNPvrLogicIsRecoveryFeatureEnabled() && self->items.spaceRecoveryOption) {
        QBRecordingEditUpdateRecoveryOption(self);
    }

    if (QBNPvrLogicIsSetMarginsFeatureEnabled() && self->items.npvrMargins) {
        QBRecordingEditUpdateNpvrMarginsOption(self);
    }

    if (self->items.repeat) {
        log_debug("");
        SvString repeat = QBPVRRepeatedRecordingsModeCreateName(self->recordingInfo.repeat, true);
        if (!SvObjectEquals((SvObject) repeat, (SvObject) self->items.repeat->subcaption)) {
            log_debug("");
            SVTESTRELEASE(self->items.repeat->subcaption);
            self->items.repeat->subcaption = SVTESTRETAIN(repeat);
            QBBasicPaneOptionPropagateObjectChange(self->options, self->items.repeat);
        }
        SVRELEASE(repeat);
    }
}

SvLocal void QBRecordingEditStartDateOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;
    svWidgetAttach(frame, self->calendarBg, self->calendarBgOffX, self->calendarBgOffY, 0);
}

SvLocal void QBRecordingEditStartDateSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;
    log_debug("");
    svWidgetSetFocus(self->calendar);
}

SvLocal void QBRecordingEditStartDateOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;
    svWidgetDetach(self->calendarBg);
}


SvLocal void QBRecordingEditStartTimeOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;

    svWidgetDetach(self->timeEdit);
}

SvLocal void QBRecordingEditStartTimeOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;
    log_debug("off=[%i, %i]", self->startTime->off_x, self->startTime->off_y);
    svWidgetAttach(frame, self->timeEdit, self->timeEditOffX, self->timeEditOffY, 0);
}

SvLocal void QBRecordingEditStartTimeSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBRecordingEditPane self = self_;
    if (self->optionID && SvStringEqualToCString(self->optionID, "startTime"))
        QBTimeInputSetFocus(self->startTime);
    else
        QBTimeInputSetFocus(self->endTime);
}

SvLocal bool QBRecordingEditTimeEditInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBRecordingEditPane self = w->prv;
    if (e->ch == QBKEY_UP) {
        QBTimeInputSetFocus(self->startTime);
        return true;
    }
    if (e->ch == QBKEY_DOWN) {
        QBTimeInputSetFocus(self->endTime);
        return true;
    }
    if (e->ch == QBKEY_ENTER) {
        struct tm local = SvLocalTimeToLocal(self->recordingInfo.startTime);
        local.tm_hour = 0;
        local.tm_min = 0;
        local.tm_sec = 0;
        SvLocalTime startTimeDateOnly = SvLocalTimeFromLocal(&local);

        SvLocalTime startTimeLocal = SvLocalTimeAdd(startTimeDateOnly, 60 * QBTimeEditGetMinutes(self->startTime));
        SvLocalTime endTimeLocal = SvLocalTimeAdd(startTimeDateOnly, 60 * QBTimeEditGetMinutes(self->endTime));
        SvLocalTime currentTimeLocal = SvLocalTimeFromUTC(SvTimeNow());

        time_t startTimeSec = SvLocalTimeToInt(startTimeLocal);
        time_t endTimeSec = SvLocalTimeToInt(endTimeLocal);
        time_t currentTimeSec = SvLocalTimeToInt(currentTimeLocal);

        if (startTimeSec >= endTimeSec) {
            endTimeLocal = SvLocalTimeAdd(endTimeLocal, 24 * 60 * 60);
            endTimeSec = SvLocalTimeToInt(endTimeLocal);
        }

        self->recordingInfo.startTime = startTimeLocal;

        if (endTimeSec < currentTimeSec) {
            self->timeEditWarning = QBRecordingEditPaneCreateTimeWarningPopup(self);
            QBDialogRun(self->timeEditWarning, self, QBRecordingEditTimeEditWarningPopupCallback);
        } else {
            self->recordingInfo.endTime = endTimeLocal;
        }

        log_debug("start=%i end=%i", SvLocalTimeToInt(self->recordingInfo.startTime), SvLocalTimeToInt(self->recordingInfo.endTime));

        QBRecordingEditUpdateItems(self);
        QBContextMenuPopPane(self->contextMenu);
        return true;
    }
    return false;
}

SvLocal void QBRecordingEditTimeEditFakeClean(SvApplication app, void *prv)
{
}

SvLocal void QBRecordingEditChangeStartDate(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;

    if ((self->pvrType == PvrType_PVR) &&
        self->recording && (self->recording->state != QBPVRRecordingState_scheduled)) {
        return;
    }

    if (!self->startDatePane) {
        static struct QBContainerPaneCallbacks_t callbacks_start = {
            .onHide    = QBRecordingEditStartDateOnHide,
            .onShow    = QBRecordingEditStartDateOnShow,
            .setActive = QBRecordingEditStartDateSetActive,
        };
        AppGlobals appGlobals = self->appGlobals;
        struct tm local = SvLocalTimeToLocal(self->recordingInfo.startTime);

        self->startDatePane = QBContainerPaneCreateFromSettings("CalendarPane.settings", appGlobals->res,
                                                                self->contextMenu, self->level + 1, SVSTRING("CalendarPane"),
                                                                &callbacks_start, self);
        svSettingsRestoreContext(self->settingsCtx);

        self->calendarBg = svSettingsWidgetCreate(appGlobals->res, "calendarBg");
        self->calendarBgOffX = svSettingsGetInteger("calendarBg", "xOffset", 0);
        self->calendarBgOffY = svSettingsGetInteger("calendarBg", "yOffset", 0);
        self->calendarBg->off_x = self->calendarBgOffX;
        self->calendarBg->off_y = self->calendarBgOffY;

        self->calendar = QBCalendarNew(appGlobals->res, "Calendar", appGlobals->textRenderer, NULL);
        svSettingsWidgetAttach(self->calendarBg, self->calendar, "Calendar", 0);
        self->dateEditRenderer = QBOSKRendererCreate(NULL);
        QBCalendarSetCurrentDate(self->calendar, &local, self->dateEditRenderer, NULL);
        QBOSKRendererStart(self->dateEditRenderer, appGlobals->scheduler, NULL);

        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, QBCalendarDateSelectedEvent_getType(), self->calendar->prv, NULL);

        svSettingsPopComponent();
    }

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) self->startDatePane);
}

SvLocal void QBRecordingEditTimePaneInit(QBRecordingEditPane self)
{
    static struct QBContainerPaneCallbacks_t callbacks_start = {
        .onHide    = QBRecordingEditStartTimeOnHide,
        .onShow    = QBRecordingEditStartTimeOnShow,
        .setActive = QBRecordingEditStartTimeSetActive,
    };

    AppGlobals appGlobals = self->appGlobals;

    self->timePane = QBContainerPaneCreateFromSettings("CalcPane.settings", appGlobals->res,
                                                       self->contextMenu, self->level + 1, SVSTRING("CalcPane"),
                                                       &callbacks_start, self);
    svSettingsRestoreContext(self->settingsCtx);

    self->timeEdit = svSettingsWidgetCreate(appGlobals->res, "timeEdit");

    self->startTime = QBTimeInputNew(appGlobals->res, 5, "startTime");
    svSettingsWidgetAttach(self->timeEdit, self->startTime, "startTime", 1);
    self->endTime = QBTimeInputNew(appGlobals->res, 5, "startTime");
    svSettingsWidgetAttach(self->timeEdit, self->endTime, "endTime", 1);

    self->timeEditOffX = svSettingsGetInteger("timeEdit", "xOffset", 0);
    self->timeEditOffY = svSettingsGetInteger("timeEdit", "yOffset", 0);
    self->timeEdit->off_x = self->timeEditOffX;
    self->timeEdit->off_y = self->timeEditOffY;
    self->timeEdit->prv = self;
    self->timeEdit->clean = QBRecordingEditTimeEditFakeClean;
    svWidgetSetInputEventHandler(self->timeEdit, QBRecordingEditTimeEditInputEventHandler);

    SvWidget w = QBAsyncLabelNew(appGlobals->res, "startTime.desc", appGlobals->textRenderer);
    svSettingsWidgetAttach(self->timeEdit, w, svWidgetGetName(w), 1);
    QBAsyncLabelSetCText(w, gettext("Start time"));
    w = QBAsyncLabelNew(appGlobals->res, "endTime.desc", appGlobals->textRenderer);
    svSettingsWidgetAttach(self->timeEdit, w, svWidgetGetName(w), 1);
    QBAsyncLabelSetCText(w, gettext("End time"));

    QBTimeEditSetTime(self->startTime, self->recordingInfo.startTime.utc);
    QBTimeEditSetTime(self->endTime, self->recordingInfo.endTime.utc);
    svSettingsPopComponent();
}

SvLocal void QBRecordingEditChangeTime(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    SVTESTRELEASE(self->optionID);
    self->optionID = SVTESTRETAIN(id);
    QBTimeEditSetTime(self->startTime, self->recordingInfo.startTime.utc);
    QBTimeEditSetTime(self->endTime, self->recordingInfo.endTime.utc);
    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) self->timePane);
}

SvLocal void QBRecordingEditPaneRepeatModeChanged(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    SvValue _mode = (SvValue) SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("mode"));
    QBPVRRepeatedRecordingsMode mode = SvValueGetInteger(_mode);
    log_debug("mode = %i", mode);
    self->recordingInfo.repeat = mode;

    QBContextMenuPopPane(self->contextMenu);
    QBRecordingEditUpdateItems(self);
}

SvLocal void add_repeat(QBRecordingEditPane self, QBBasicPane pane, QBPVRRepeatedRecordingsMode _mode)
{
    SvString caption = QBPVRRepeatedRecordingsModeCreateName(_mode, true);
    SvString id = QBPVRRepeatedRecordingsModeCreateName(_mode, false);
    QBBasicPaneItem item = QBBasicPaneAddOption(pane, id, caption, QBRecordingEditPaneRepeatModeChanged, self);
    SVRELEASE(caption);
    SVRELEASE(id);
    if (item) {
        SvValue mode = SvValueCreateWithInteger(_mode, NULL);
        SvHashTableInsert(item->metadata, (SvGenericObject) SVSTRING("mode"), (SvGenericObject) mode);
        SVRELEASE(mode);
    }
}

SvLocal void QBRecordingEditPaneChangeRepeat(void *self_, SvString id, QBBasicPane _pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    AppGlobals appGlobals = self->appGlobals;

    log_debug("");

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane pane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(pane, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer,
                    self->contextMenu, self->level + 1, SVSTRING("BasicPane"));

    add_repeat(self, pane, QBPVRRepeatedRecordings_None);
    add_repeat(self, pane, QBPVRRepeatedRecordings_Daily);
    add_repeat(self, pane, QBPVRRepeatedRecordings_Weekly);
    add_repeat(self, pane, QBPVRRepeatedRecordings_Weekends);
    add_repeat(self, pane, QBPVRRepeatedRecordings_Weekdays);

    SvString rid = QBPVRRepeatedRecordingsModeCreateName(self->recordingInfo.repeat, false);
    QBBasicPaneSetPosition(pane, rid, true);
    SVRELEASE(rid);

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) pane);
    SVRELEASE(pane);

    svSettingsPopComponent();
}

SvLocal void QBRecordingEditPaneKeywordKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBRecordingEditPane self = ptr;
    log_debug();
    if (key->type == QBOSKKeyType_enter) {
        log_debug("newkeyword = %s", SvStringCString(input));
        SVTESTRELEASE(self->recordingInfo.keyword);
        self->recordingInfo.keyword = SVRETAIN(input);
        QBContextMenuPopPane(self->contextMenu);
        QBRecordingEditUpdateItems(self);
    }
}

SvLocal void QBRecordingEditPaneChangeKeyword(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    log_debug("");
    QBRecordingEditPane self = self_;

    if (!self->oskPane) {
        svSettingsRestoreContext(self->settingsCtx);
        self->oskPane = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
        SvErrorInfo error = NULL;
        QBOSKPaneInit(self->oskPane, self->appGlobals->scheduler, self->contextMenu,
                      self->level + 1, SVSTRING("OSKPane"), QBRecordingEditPaneKeywordKeyTyped, self, &error);
        svSettingsPopComponent();
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            SVRELEASE(self->oskPane);
            self->oskPane = NULL;
        }
    }

    if (self->oskPane) {
        QBOSKPaneSetInput(self->oskPane, self->recordingInfo.keyword);
        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) self->oskPane);
    }
}

SvLocal void QBRecordingEditPaneNpvrPremiereApproveCallback(void *self_, int optionValue)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    self->recordingInfo.premiereOption = (optionValue != 0);

    QBRecordingEditUpdateItems(self);
}

SvLocal void QBRecordingEditPaneChangePremiere(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    if (!self->premiereModePane) {
        self->premiereModePane = QBNPvrOptionsSelectPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1,
            QBNPvrOptionsSelectPaneType_Premiere);

        QBNPvrOptionsSelectPaneSetApproveCallback(self->premiereModePane, QBRecordingEditPaneNpvrPremiereApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->premiereModePane, (int) self->recordingInfo.premiereOption);
}

SvLocal void QBRecordingEditPaneNpvrSpaceRecoveryApproveCallback(void *self_, int optionValue)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    if (optionValue == QBnPVRRecordingSpaceRecovery_DeleteOldest) {
        self->recordingInfo.recoveryOption = QBnPVRRecordingSpaceRecovery_DeleteOldest;
    } else if (optionValue == QBnPVRRecordingSpaceRecovery_Manual) {
        self->recordingInfo.recoveryOption = QBnPVRRecordingSpaceRecovery_Manual;
    }

    QBRecordingEditUpdateItems(self);
}

SvLocal void QBRecordingEditPaneChangeSpaceRecovery(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    if (!self->spaceRecoveryModePane) {
        self->spaceRecoveryModePane = QBNPvrOptionsSelectPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1,
            QBNPvrOptionsSelectPaneType_SpaceRecovery);

        QBNPvrOptionsSelectPaneSetApproveCallback(self->spaceRecoveryModePane, QBRecordingEditPaneNpvrSpaceRecoveryApproveCallback, self);
    }
    QBNPvrOptionsSelectPaneShow(self->spaceRecoveryModePane, (int) self->recordingInfo.recoveryOption);
}

SvLocal void QBRecordingEditPaneNpvrMarginsHideCallback(void *self_)
{
    QBRecordingEditPane self = (QBRecordingEditPane) self_;
    QBRecordingEditUpdateItems(self);
}

SvLocal void QBRecordingEditPaneSetNpvrMargins(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    if (!self->npvrMarginsPane) {
        self->npvrMarginsPane = QBNPvrMarginsPaneCreate(
            self->appGlobals,
            self->contextMenu,
            self->level + 1);

        QBNPvrMarginsPaneSetApproveCallback(self->npvrMarginsPane, QBRecordingEditPaneNpvrMarginsHideCallback, self);
    }
    QBNPvrMarginsPaneShow(self->npvrMarginsPane);
}


SvLocal ChannelChoiceTickState QBRecordingEditPaneAddRemoveChannelsIsTicked(void *self_, SvGenericObject channel)
{
    QBRecordingEditPane self = self_;
    if (!SvObjectIsInstanceOf(channel, SvTVChannel_getType())) {
        channel = NULL;
    }

    if ((SvTVChannel) channel == self->recordingInfo.channel) {
        return ChannelChoiceTickState_On;
    }

    return ChannelChoiceTickState_Off;
}

SvLocal SvString QBRecordingEditPaneAddRemoveChannelsCreateCaption(void *self_, SvGenericObject channel_)
{
    if (!SvObjectIsInstanceOf(channel_, SvTVChannel_getType())) {
        if (!SvObjectIsInstanceOf(channel_, SvString_getType())) {
            return NULL;
        }

        return SvStringCreate(gettext("(all)"), NULL);
    }

    SvTVChannel channel = (SvTVChannel) channel_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    // AMERELES [4123] Servicios Code Download aparecen cuando programas una grabación
    //SvGenericObject allChannels = QBPlaylistManagerGetById(playlists, SVSTRING("PVRChannels"));
    SvGenericObject allChannels = QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels"));

    SvString caption = NULL;
    if (SvObjectIsInstanceOf(channel_, SvString_getType())) {
        caption = SVRETAIN(channel_);
    } else if (SvObjectIsInstanceOf(channel_, SvTVChannel_getType())) {
        caption = SvStringCreateWithFormat("%03i %s", SvInvokeInterface(SvEPGChannelView, allChannels, getChannelNumber, channel), SvStringCString(channel->name));
    }
    return caption;
}

SvLocal void QBRecordingEditPaneChannelSelected(void *self_, QBListPane pane, SvGenericObject channel, int pos)
{
    QBRecordingEditPane self = self_;
    log_debug("");
    if (!SvObjectIsInstanceOf(channel, SvTVChannel_getType())) {
        channel = NULL;
    }

    SVTESTRELEASE(self->recordingInfo.channel);
    self->recordingInfo.channel = SVTESTRETAIN(channel);
    QBContextMenuPopPane(self->contextMenu);
    QBRecordingEditUpdateItems(self);
}

SvLocal void QBRecordingEditPaneChangeChannel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBRecordingEditPane self = self_;
    SvGenericObject playlist = NULL;
    AppGlobals appGlobals = self->appGlobals;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (self->pvrType == PvrType_PVR) {
        if (self->recording && (self->recording->state != QBPVRRecordingState_scheduled)) {
            return;
        }

        // AMERELES [4123] Servicios Code Download aparecen cuando programas una grabación
        //playlist = QBPlaylistManagerGetById(playlists, SVSTRING("PVRChannels"));
        playlist = QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels"));
    }

    if (self->pvrType == PvrType_NPVR) {
        playlist = QBPlaylistManagerGetById(playlists, SVSTRING("IPChannels"));
    }
    if (!playlist) {
        log_error("Unable to get channels list");
        return;
    }

    QBListPane channels = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked          = QBRecordingEditPaneAddRemoveChannelsIsTicked,
        .createNodeCaption = QBRecordingEditPaneAddRemoveChannelsCreateCaption
    };
    QBChannelChoiceController constructor = QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings", appGlobals->textRenderer,
                                                                                        playlist, self, &callbacks);

    svSettingsRestoreContext(self->settingsCtx);
    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBRecordingEditPaneChannelSelected
    };

    int channelsShift = 0;
    {
        QBActiveArray extra = QBActiveArrayCreate(1, NULL);
        if (self->recordingType == QBRecordingEditPane_Keyword) {
            SvString str = SvStringCreate(gettext("(all)"), NULL);
            QBActiveArrayAddObject(extra, (SvObject) str, NULL);
            channelsShift++;
            SVRELEASE(str);
        }
        QBTreeProxy treeProxy = QBTreeProxyCreate((SvGenericObject) playlist, (SvGenericObject) extra, NULL);
        QBListProxy listProxy = QBListProxyCreate((SvGenericObject) treeProxy, NULL, NULL);
        SVRELEASE(treeProxy);
        SVRELEASE(extra);

        QBListPaneInit(channels, appGlobals->res, (SvGenericObject) listProxy, (SvGenericObject) constructor, &listCallbacks,
                       self, self->contextMenu, self->level + 1, SVSTRING("ListPane"));
        SVRELEASE(listProxy);
    }

    svSettingsPopComponent();
    SVRELEASE(constructor);

    SvWidget menu = QBListPaneGetMenu(channels);
    int pos = SvInvokeInterface(QBListModel, playlist, getLength) - 1;
    for (; pos >= 0; pos--) {
        if ((SvObject) self->recordingInfo.channel == SvInvokeInterface(QBListModel, playlist, getObject, pos))
            break;
    }
    XMBVerticalMenuSetPosition(menu, channelsShift + pos, true, NULL);

    SVTESTRELEASE(self->channelSelectionPane);
    self->channelSelectionPane = channels;

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) channels);
}

/*
 * Initialization
 */

void QBRecordingEditPaneSetTimeRange(QBRecordingEditPane self, SvLocalTime start, SvLocalTime stop)
{
    self->recordingInfo.startTime = start;
    self->recordingInfo.endTime = stop;
    QBRecordingEditUpdateItems(self);
}

void QBRecordingEditPaneSetRepeatMode(QBRecordingEditPane self, QBPVRRepeatedRecordingsMode mode)
{
    self->recordingInfo.repeat = mode;
    QBRecordingEditUpdateItems(self);
}

void QBRecordingEditPaneSetName(QBRecordingEditPane self, SvString name)
{
    SVTESTRELEASE(self->recordingInfo.name);
    self->recordingInfo.name = SVTESTRETAIN(name);
    QBRecordingEditUpdateItems(self);
}

void QBRecordingEditPaneSetKeyword(QBRecordingEditPane self, SvString keyword)
{
    SVTESTRELEASE(self->recordingInfo.keyword);
    self->recordingInfo.keyword = SVTESTRETAIN(keyword);
    QBRecordingEditUpdateItems(self);
}

SvLocal void
QBRecordingEditPaneInitSetDefaultRecordingInfoData(QBRecordingEditPane self)
{
    self->recordingInfo.startTime = SvLocalTimeFromUTC(SvTimeNow());
    self->recordingInfo.endTime = SvLocalTimeAdd(self->recordingInfo.startTime, 3600);
    self->recordingInfo.keyword = SVSTRING("");
    self->recordingInfo.repeat = QBPVRRepeatedRecordings_None;
}

SvLocal void
QBRecordingEditPaneInitSetupEditPane(QBRecordingEditPane self, QBRecordingEditPaneCreationSettings creationSettings)
{
    AppGlobals appGlobals = self->appGlobals;
    if (creationSettings->settingsFileName) { // push component if needed
        svSettingsPushComponent(creationSettings->settingsFileName);
    }

    self->settingsCtx = svSettingsSaveContext();

    self->options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->options, appGlobals->res, appGlobals->scheduler,
                    appGlobals->textRenderer, self->contextMenu, self->level, SVSTRING("BasicPane"));

    SvString filename = SvStringCreate(creationSettings->optionsFileName, NULL);
    QBBasicPaneLoadOptionsFromFile(self->options, filename);
    SVRELEASE(filename);

    SvString name = SvStringCreate(gettext("Save"), NULL);
    QBBasicPaneAddOption(self->options, SVSTRING("save"), name, QBRecordingEditConfirmSave, self);
    SVRELEASE(name);

    name = SvStringCreate(gettext("Channel"), NULL);
    self->items.channel = QBBasicPaneAddOption(self->options, SVSTRING("channel"), name, QBRecordingEditPaneChangeChannel, self);
    SVRELEASE(name);

    // NPvr - only keyword recordings are editable
    if ((self->pvrType == PvrType_PVR) && (self->recordingType == QBRecordingEditPane_Manual)) {
        QBRecordingEditTimePaneInit(self);
        name = SvStringCreate(gettext("Date"), NULL);
        self->items.startDate = QBBasicPaneAddOption(self->options, SVSTRING("date"), name, QBRecordingEditChangeStartDate, self);
        SVRELEASE(name);
        name = SvStringCreate(gettext("Start time"), NULL);
        self->items.startTime = QBBasicPaneAddOption(self->options, SVSTRING("startTime"), name, QBRecordingEditChangeTime, self);
        SVRELEASE(name);
        name = SvStringCreate(gettext("End time"), NULL);
        self->items.endTime = QBBasicPaneAddOption(self->options, SVSTRING("endTime"), name, QBRecordingEditChangeTime, self);
        SVRELEASE(name);
        name = SvStringCreate(gettext("Duration"), NULL);
        self->items.duration = QBBasicPaneAddOption(self->options, SVSTRING("duration"), name, NULL, self);
        SVRELEASE(name);

        name = SvStringCreate(gettext("Repeat"), NULL);
        self->items.repeat = QBBasicPaneAddOption(self->options, SVSTRING("repeat"), name, QBRecordingEditPaneChangeRepeat, self);
        SVRELEASE(name);
    } else {
        name = SvStringCreate(gettext("Keyword"), NULL);
        self->items.keyword = QBBasicPaneAddOption(self->options, SVSTRING("keyword"), name, QBRecordingEditPaneChangeKeyword, self);
        SVRELEASE(name);

        if (QBNPvrLogicIsPremiereFeatureEnabled()) {
            name = SvStringCreate(gettext("Premiere"), NULL);
            self->items.premiereOption = QBBasicPaneAddOption(self->options, SVSTRING("premiere"), name, QBRecordingEditPaneChangePremiere, self);
            SVRELEASE(name);
        }

        if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
            name = SvStringCreate(gettext("Space recovery"), NULL);
            self->items.spaceRecoveryOption = QBBasicPaneAddOption(self->options, SVSTRING("spaceRecovery"), name, QBRecordingEditPaneChangeSpaceRecovery, self);
            SVRELEASE(name);
        }

        if (QBNPvrLogicIsSetMarginsFeatureEnabled()) {
            name = SvStringCreate(gettext("nPVR margins"), NULL);
            self->items.npvrMargins = QBBasicPaneAddOption(self->options, SVSTRING("NpvrMargins"), name, QBRecordingEditPaneSetNpvrMargins, self);
            SVRELEASE(name);
        }
    }

    QBRecordingEditUpdateItems(self);

    QBContextMenuSetInputEventHandler(self->contextMenu, self, QBRecordingEditPaneGlobalInputEventHandler);

    QBBasicPaneSetPosition(self->options, SVSTRING("save"), true);

    svSettingsPopComponent();
}

SvLocal void
QBRecordingEditPaneStoreCommonInputData(QBRecordingEditPane self,
                                        QBRecordingEditPaneCreationSettings creationSettings)
{
    self->appGlobals = creationSettings->appGlobals;
    self->contextMenu = creationSettings->contextMenu;
    self->level = creationSettings->level;
}

SvLocal void
QBRecordingEditPaneValidateCreationSettings(QBRecordingEditPaneCreationSettings creationSettings,
                                            SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!creationSettings->appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Parameter appGlobals is NULL.");
        goto err;
    }

    if (!creationSettings->settingsFileName) {
        SvLogNotice("Parameter settingsFileName is NULL.");
    }

    if (!creationSettings->optionsFileName) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Parameter logicOptionsFileName is NULL.");
        goto err;
    }

    if (!creationSettings->contextMenu) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Parameter contextMenu is NULL.");
        goto err;
    }

err:
    SvErrorInfoPropagate(error, errorOut);
}

QBRecordingEditPane
QBRecordingEditPaneCreatePvrWithRec(QBRecordingEditPaneCreationSettings creationSettings,
                                    QBPVRRecording recording,
                                    QBRecordingEditPaneType recordingType,
                                    SvErrorInfo *errorOut)
{
    QBRecordingEditPane self = NULL;
    SvErrorInfo error = NULL;

    QBRecordingEditPaneValidateCreationSettings(creationSettings, &error);
    if (error) {
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    self = (QBRecordingEditPane) SvTypeAllocateInstance(QBRecordingEditPane_getType(), NULL);
    self->pvrType = PvrType_PVR;
    self->recordingType = recordingType;
    QBRecordingEditPaneStoreCommonInputData(self, creationSettings);

    AppGlobals appGlobals = self->appGlobals;

    QBRecordingEditPaneInitSetDefaultRecordingInfoData(self);

    if (recording) {
        self->recording = SVRETAIN(recording);
        SvValue channelIdVal = SvValueCreateWithString(recording->channelId, NULL);
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        self->recordingInfo.channel = SVTESTRETAIN(SvEPGManagerFindTVChannel(epgManager, channelIdVal, NULL));
        SVRELEASE(channelIdVal);
        self->recordingInfo.startTime = SvLocalTimeFromUTC(SvTimeGetSeconds(recording->startTime));
        self->recordingInfo.endTime = SvLocalTimeFromUTC(SvTimeGetSeconds(recording->endTime));
    } else {
        self->recordingInfo.channel = SVTESTRETAIN(SvTVContextGetCurrentChannel(appGlobals->newTV));
        if (SvTVChannelGetAttribute(self->recordingInfo.channel, SVSTRING("adaptive_streaming"))) {
            SVTESTRELEASE(self->recordingInfo.channel);
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            // AMERELES [4123] Servicios Code Download aparecen cuando programas una grabación
            //SvObject pvrChannels = QBPlaylistManagerGetById(playlists, SVSTRING("PVRChannels"));
            SvObject pvrChannels = QBPlaylistManagerGetById(playlists, SVSTRING("AllChannels"));
            self->recordingInfo.channel = SVTESTRETAIN(SvInvokeInterface(SvEPGChannelView, pvrChannels, getByIndex, 0));
        }
    }

    QBRecordingEditPaneInitSetupEditPane(self, creationSettings);

    return self;
}

QBRecordingEditPane
QBRecordingEditPaneCreatePvrWithChannel(QBRecordingEditPaneCreationSettings creationSettings,
                                        SvTVChannel channel,
                                        QBRecordingEditPaneType recordingType,
                                        SvErrorInfo *errorOut)
{
    QBRecordingEditPane self = NULL;
    SvErrorInfo error = NULL;

    QBRecordingEditPaneValidateCreationSettings(creationSettings, &error);
    if (error) {
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    if (!channel) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "Parameter channel is NULL.");
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    self = (QBRecordingEditPane) SvTypeAllocateInstance(QBRecordingEditPane_getType(), NULL);
    self->pvrType = PvrType_PVR;
    self->recordingType = recordingType;
    QBRecordingEditPaneStoreCommonInputData(self, creationSettings);

    QBRecordingEditPaneInitSetDefaultRecordingInfoData(self);

    self->recordingInfo.channel = SVRETAIN(channel);

    QBRecordingEditPaneInitSetupEditPane(self, creationSettings);

    return self;
}


QBRecordingEditPane
QBRecordingEditPaneCreateNPvrKeywordWithEvent(QBRecordingEditPaneCreationSettings creationSettings,
                                              SvEPGEvent event,
                                              SvErrorInfo *errorOut)
{
    QBRecordingEditPane self = NULL;
    SvErrorInfo error = NULL;

    QBRecordingEditPaneValidateCreationSettings(creationSettings, &error);
    if (error) {
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    self = (QBRecordingEditPane) SvTypeAllocateInstance(QBRecordingEditPane_getType(), NULL);
    self->pvrType = PvrType_NPVR;
    self->recordingType = QBRecordingEditPane_Keyword;
    QBRecordingEditPaneStoreCommonInputData(self, creationSettings);

    AppGlobals appGlobals = self->appGlobals;

    QBRecordingEditPaneInitSetDefaultRecordingInfoData(self);

    if (event) {
        SVTESTRELEASE(self->recordingInfo.keyword);
        SvString eventName = QBEventUtilsGetTitleFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, event);
        self->recordingInfo.keyword = SVRETAIN(eventName);

        if (!self->recordingInfo.channel) {
            SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                 SVSTRING("SvEPGManager"));
            self->recordingInfo.channel = SVTESTRETAIN(SvEPGManagerFindTVChannel(epgManager, event->channelID, NULL));
        }
    }

    if (!self->recordingInfo.channel) {
        // fallback - take current channel if it comes from IP
        SvTVChannel channel = SvTVContextGetCurrentChannel(appGlobals->newTV);
        unsigned int ipPluginID = SvInvokeInterface(SvEPGChannelListPlugin, appGlobals->epgPlugin.IP.channelList, getID);
        if (channel && channel->pluginID == ipPluginID) {
            self->recordingInfo.channel = SVRETAIN(channel);
        } else {
            // or take first IP channel
            QBServiceRegistry registry = QBServiceRegistryGetInstance();
            QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
            SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("IPChannels"));
            if (playlist) {
                SvTVChannel firstIPChannel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, 0);
                self->recordingInfo.channel = SVTESTRETAIN(firstIPChannel);
            }
        }
    }

    if (!self->recordingInfo.channel) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound, "No IP channels.");
        SvErrorInfoPropagate(error, errorOut);
        SVRELEASE(self);
        return NULL;
    }

    QBRecordingEditPaneInitSetupEditPane(self, creationSettings);

    return self;
}
