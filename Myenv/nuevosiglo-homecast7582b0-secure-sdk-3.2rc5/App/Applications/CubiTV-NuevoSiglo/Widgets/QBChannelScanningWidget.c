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

#include "QBChannelScanningWidget.h"

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/QBFrame.h>
#include <SWL/gauge.h>
#include <SWL/events.h>
#include <SWL/anim.h>
#include <settings.h>
#include <QBApplicationController.h>
#include <QBDVBScanner.h>
#include <QBTuner.h>
#include <QBTunerTypes.h>
#include <sv_tuner.h>
#include <QBTunerLNBConfig.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <tunerReserver.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Services/core/JSONserializer.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/QBStandardRastersManager.h>
#include <SvJSON/SvJSONParse.h>
#include <Widgets/QBFrequencyInput.h>
#include <Widgets/QBNumericInput.h>
#include <Widgets/QBTitle.h>
#include <QBWidgets/QBComboBox.h>
#include <Widgets/customerLogo.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/channelscanning.h>
#include <Windows/SatelliteSetup.h>
#include <Windows/channelscanning/defaultScanParams.h>
#include <QBStandardRastersParams.h>
#include <QBConf.h>
#include <ContextMenus/QBChannelScanningPane.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <QBDVBSatellitesDB.h>

/*
 *
 * CS_NOOP:
 *      StartScanning -> CS_SCANNING
 * CS_SCANNING:
 *      BreakScanning -> CS_NOOP
 *      EndScanning -> CS_FINISHED_SCANNING
 * CS_FINISHED_SCANNING:
 *      Save -> CS_SAVING
 *      BreakSaving -> CS_NOOP
 * CS_SAVING:
 *      FinishedSaving -> CS_FINISHED_SAVING
 * CS_FINISHED_SAVING:
 *      Finish -> CS_NOOP
 *
 * DROP_NOOP:
 *      StartDropChannels -> DROP_AWAIT_CONFIRMATION
 * DROP_AWAIT_CONFIRMATION:
 *      BreakDropChannels -> DROP_NOOP
 *      ConfirmDropChannels -> DROP_DROPPING
 * DROP_DROPPING:
 *      FinishedDrop -> DROP_NOOP
 */

struct QBChannelScanningWidget_ {
    struct SvObject_ super_;

    QBChannelScanningParams params;

    char *name;
    SvWidget widget;
    SvApplication app;
    DVBEPGPlugin dvbEPGPlugin;
    QBDVBSatellitesDB satellitesDB;
    SvEPGManager epgManager;
    QBDVBScanner *scanner;

    SvFiber savingFiber;
    SvFiberTimer savingTimer;

    SvFiber dropFiber;
    SvFiberTimer dropTimer;

    bool successfulScan;
    bool sideMenuSettingsEnabled; // side menu settings enabled

    enum {
        CS_NOOP,
        CS_SCANNING,
        CS_FINISHED_SCANNING,
        CS_SAVING,
        CS_FINISHED_SAVING
    } state;

    enum {
        DROP_NOOP,
        DROP_AWAIT_CONFIRMATION,
        DROP_DROPPING
    } dropState;

    enum {
        FINISH_NORMAL,
        FINISH_STOP_SAVE
    } finishMode;
    /*
     * Used to step from one scanning procedure to next.
     */

    struct {
        int startFreq;
        int endFreq;
        int stepFreq;
        int symbolRate;
        int TVChannelsCount;
        int radioChannelsCount;
    } scanning;

    struct QBTunerProperties tunerProps;

    SvWidget satellite;

    //SvWidget freqJump;
    int freqJumpValue;
    SvWidget freqMax;
    int freqMaxValue;
    SvWidget freqMin;
    int freqMinValue;

    SvWidget symbolRate;

    SvWidget modulation;

    SvWidget polarization;

    SvWidget extra;

    SvWidget useNITSDT;
    SvWidget start;
    SvWidgetId startId;
    SvWidget drop;
    SvWidgetId dropId;
    SvWidget logo;
    SvWidget symbolRateBoxActiveFrame;
    SvWidget symbolRateBoxInactiveFrame;

    const char *scanningTitle;

    bool hasLock;

    SvFiber fiber;
    SvFiberTimer timer;


    SvWidget dialog;
    SvWidgetId dialogId;
    SvWidget button;
    SvWidget breakButton;
    SvWidget cancelButton;
    SvWidget currentStatus;
    SvWidget lockStatus;
    SvBitmap lockOn;
    SvBitmap lockOff;
    SvWidget lockIndicator;
    SvWidget total;
    SvWidget signalStrength;
    SvWidget signalQuality;

    SvWidget dropDialog;
    SvWidgetId dropDialogId;
    SvWidget dropCancel;
    SvWidget dropConfirm;

    SvTimerId timerID;
    int timeLeft;

    SvWidget noTunerDialog;
    SvWidgetId noTunerDialogId;
    SvWidget noTunerConfirm;

    SvWidget satelliteSetupDialog;
    SvWidgetId satelliteSetupDialogId;

    int priority;
    SvWidget failureDialog;
    const char *failureTitle;

    int tunerNum;
    unsigned int settingsCtx;
    struct {
        QBContextMenu ctx;
    } sidemenu;

    SvScheduler scheduler;
};


/*
 * Forward declarations
 */

SvLocal void QBChannelScanningWidgetStartSaving(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetStartScanning(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetBreak(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetFinish(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetFinishSaving(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetBreakSaving(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetStartDropChannels(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetBreakDropChannels(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetConfirmDropChannels(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetFinishDrop(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetEndScanning(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetNoTuner(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetNoTunerBreak(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetFailureBreak(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetShowSideMenu(QBChannelScanningWidget self);

/*
 * Manual scanning stuff
 */

SvLocal void QBChannelScanningWidgetUpdateScanningTitle(QBChannelScanningWidget self)
{
    const int channelsCnt = QBDVBScannerGetNumberOfChannels(self->scanner);

    char *title;
    asprintf(&title, ngettext("%s: %i channel found", "%s: %i channels found", channelsCnt), self->scanningTitle, channelsCnt);
    QBDialogSetTitle(self->dialog, title);
    free(title);
}

SvLocal char *QBChannelScanningWidgetCreateScanningStatusText(int freq, int symbolRate, QBTunerModulation modulation, QBTunerPolarization polarization)
{
    char symbolRateStr[128];
    char modulationStr[128];
    char polarizationStr[128];

    if (symbolRate >= 0)
        snprintf(symbolRateStr, sizeof(symbolRateStr), gettext("Symbol Rate: %d"), symbolRate);
    else
        symbolRateStr[0] = 0;

    if (modulation != QBTunerModulation_unknown)
        snprintf(modulationStr, sizeof(modulationStr), gettext("Modulation: %s"), QBTunerModulationToString(modulation));
    else
        modulationStr[0] = 0;

    if (polarization == QBTunerPolarization_Horizontal)
        strncpy(polarizationStr, gettext(" H"), sizeof(polarizationStr));
    else if (polarization == QBTunerPolarization_Vertical)
        strncpy(polarizationStr, gettext(" V"), sizeof(polarizationStr));
    else
        polarizationStr[0] = 0;

    char *text;
    asprintf(&text, gettext("Frequency: %d.%02d MHz%s %s %s"),
             freq / 1000, (freq % 1000) / 10,
             polarizationStr,
             modulationStr,
             symbolRateStr);
    return text;
}

SvLocal void QBChannelScanningWidgetUpdateScanningStatus(QBChannelScanningWidget self, QBDVBScanner* scanner)
{
    if (self->currentStatus) {
        int symbolRate = -1;
        QBTunerModulation modulation = QBTunerModulation_unknown;
        QBTunerPolarization polarization = QBTunerPolarization_unknown;

        if (!self->tunerProps.automatic.symbol_rate)
            symbolRate = QBDVBScannerGetSymbolRate(self->scanner);

        if (!self->tunerProps.automatic.modulation)
            modulation = QBDVBScannerGetModulation(scanner);

        if (!self->tunerProps.automatic.polarization)
            polarization = QBDVBScannerGetPolarization(scanner);

        int freq = QBDVBScannerGetFreq(scanner);
        char *statusText = QBChannelScanningWidgetCreateScanningStatusText(freq, symbolRate, modulation, polarization);
        svLabelSetText(self->currentStatus, statusText);
        free(statusText);
    }
}

SvLocal void QBChannelScanningWidgetCleanAfterDialogDestory(QBChannelScanningWidget self)
{
    self->dialogId = -1;
    self->currentStatus = NULL;
    self->cancelButton = NULL;
}

SvLocal void QBChannelScanningWidgetCleanAfterDropDialogDestroy(QBChannelScanningWidget self)
{
    self->dropDialogId = -1;
    self->dropConfirm = NULL;
    self->dropCancel = NULL;
}

SvLocal void QBChannelScanningWidgetQBDVBScannerSetupOnStep(QBChannelScanningWidget self, QBDVBScanner* scanner)
{
    QBChannelScanningWidgetUpdateScanningTitle(self);
    QBChannelScanningWidgetUpdateScanningStatus(self, scanner);

    //Update progress
    if (self->scanning.startFreq != self->scanning.endFreq) {
        if (self->total) {
            int freq = QBDVBScannerGetFreq(scanner);
            double steps = (self->scanning.endFreq - self->scanning.startFreq) / self->scanning.stepFreq + 1;
            double step = (freq - self->scanning.startFreq) / self->scanning.stepFreq + 1;
            svGaugeSetValue(self->total, (step - 1) / steps * 100.0);
        }
    }

    QBDialogRealign(self->dialog);
}

SvLocal void QBChannelScanningWidgetStep(void *self_)
{
    QBChannelScanningWidget self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(250));

    bool hasLock = false;
    int strength = 0;
    int quality = 0;

    int tunerNum = -1;
    if (self->scanner)
        tunerNum = QBDVBScannerGetTunerNum(self->scanner);

    if (tunerNum >= 0) {
        struct QBTunerStatus status;
        struct sv_tuner_state* tuner = sv_tuner_get(tunerNum);
        sv_tuner_get_status(tuner, &status);
        hasLock = status.full_lock;
        strength = status.signal_strength;
        quality = status.signal_quality;
    }
    if (self->signalStrength)
        svGaugeSetValue(self->signalStrength, strength);
    if (self->signalQuality)
        svGaugeSetValue(self->signalQuality, quality);

    if (hasLock != self->hasLock) {
        self->hasLock = hasLock;
        if (self->lockStatus)
            svLabelSetText(self->lockStatus, hasLock ? gettext("Lock status - OK") : gettext("Lock status - NO SIGNAL"));
        if (self->lockIndicator)
            svWidgetSetBitmap(self->lockIndicator, hasLock ? self->lockOn : self->lockOff);
    }

    QBChannelScanningWidgetUpdateScanningTitle(self);
}

SvLocal bool QBChannelScanningWidgetInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBChannelScanningWidget self = w->prv;

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);

    if (QBChannelScanningLogicInputEventHandler(channelScanningLogic, &(e->fullInfo))) {
        return true;
    }

    if (!QBInputEventIsKeyEvent(e->type)) {
        return false;
    }

    if (e->ch == QBKEY_VOLUP || e->ch == QBKEY_VOLDN || e->ch == QBKEY_MUTE || e->ch == QBKEY_VOUT) {
        return false;
    }
    if (e->ch == QBKEY_FUNCTION) {
        if (!QBContextMenuIsShown(self->sidemenu.ctx) && self->sideMenuSettingsEnabled) {
            QBChannelScanningWidgetShowSideMenu(self);
        }
        return true;
    }

    if (self->state == CS_SCANNING && e->ch != QBKEY_ENTER) {
        return true;
    }

    if (self->state == CS_FINISHED_SCANNING && (e->ch != QBKEY_ENTER && e->ch != QBKEY_LEFT && e->ch != QBKEY_RIGHT)) {
        return true;
    }

    if (self->state == CS_SAVING || self->dropState == DROP_DROPPING) {
        return true;
    }
    return false;
}

SvLocal void QBChannelScanningWidgetTimerEventHandler(SvWidget w, SvTimerEvent ev)
{
    QBChannelScanningWidget self = w->prv;
    if (ev->id != self->timerID)
        return;

    if (self->successfulScan)
        QBChannelScanningWidgetStartSaving(self);
    else
        QBChannelScanningWidgetBreakSaving(self);
}

SvLocal void QBChannelScanningWidgetStop(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetSafeStop(QBChannelScanningWidget self);
SvLocal void QBChannelScanningWidgetSetupStartSaving(QBChannelScanningWidget self);

SvLocal bool QBChannelScanningWidgetDialogNotificationCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningWidget self = ptr;
    if (self->dialog) {
        if (self->state == CS_SCANNING) {
            if (!buttonTag)
                return true;

            if (SvStringEqualToCString(buttonTag, "button")) {
                QBChannelScanningWidgetBreak(self);
            } else if (SvStringEqualToCString(buttonTag, "break-button")) {
                self->finishMode = FINISH_STOP_SAVE;
                QBChannelScanningWidgetSetupStartSaving(self);
                QBChannelScanningWidgetSafeStop(self);
            }
        } else if (self->state == CS_FINISHED_SCANNING) {
            if (!buttonTag)
                return true;

            if (SvStringEqualToCString(buttonTag, "button")) {
                QBChannelScanningWidgetStartSaving(self);
            } else if (SvStringEqualToCString(buttonTag, "cancel-button")) {
                QBChannelScanningWidgetBreakSaving(self);
                QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(),
                                                                                                              QBChannelScanningLogic_getType(),
                                                                                                              NULL);
                if (!self->successfulScan)
                    QBChannelScanningLogicScanningFinished(channelScanningLogic, self->params->conf, self->successfulScan);
            }
        }
        return true;
    }
    return false;
}

SvLocal bool QBChannelScanningWidgetDropDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningWidget self = ptr;

    if (self->dropDialog) {
        if (!buttonTag)
            return true;

        if (self->dropState == DROP_AWAIT_CONFIRMATION && SvStringEqualToCString(buttonTag, "cancel")) {
            QBChannelScanningWidgetBreakDropChannels(self);
            return true;
        } else if (self->dropState == DROP_AWAIT_CONFIRMATION && SvStringEqualToCString(buttonTag, "confirm")) {
            QBChannelScanningWidgetConfirmDropChannels(self);
            return true;
        }
    }
    return false;
}

SvLocal bool QBChannelScanningWidgetSatelliteSetupDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningWidget self = ptr;

    if (self->satelliteSetupDialog) {
        if (!buttonTag)
            return true;

        if (SvStringEqualToCString(buttonTag, "Setup")) {
            QBWindowContext satelliteSetup = SatelliteSetupContextCreateSingleSatelliteMode(self->app, (QBDVBSatelliteDescriptor) QBComboBoxGetValue(self->satellite));
            if (satelliteSetup) {
                QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                           SVSTRING("QBApplicationController"));
                QBApplicationControllerPushContext(controller, satelliteSetup);
                SVRELEASE(satelliteSetup);
            }
        }
        SvWidget setupDialog = self->satelliteSetupDialog;
        self->satelliteSetupDialog = NULL;
        QBDialogBreak(setupDialog);
        self->satelliteSetupDialogId = 0;

        return true;
    }
    return false;
}

SvLocal bool QBChannelScanningWidgetNoTunerDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningWidget self = ptr;

    if (self->noTunerDialog) {
        if (!buttonTag)
            return true;

        QBChannelScanningWidgetNoTunerBreak(self);
        return true;
    }
    return false;
}

SvLocal void QBChannelScanningWidgetUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBChannelScanningWidget self = w->prv;

    if (e->code == SV_EVENT_BUTTON_PUSHED && src == self->startId) {
        QBChannelScanningWidgetStartScanning(self);
        return;
    }

    if (e->code == SV_EVENT_BUTTON_PUSHED && src == self->dropId) {
        QBChannelScanningWidgetStartDropChannels(self);
        return;
    }
}

SvLocal SvWidget
QBChannelScanningWidgetDialogCreate(QBChannelScanningWidget self, const char * name)
{
    QBApplicationController controller = (QBApplicationController)QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                              SVSTRING("QBApplicationController"));
    QBDialogParameters params = {
        .app        = self->app,
        .controller = controller,
        .widgetName = name,
        .ownerId    = svWidgetGetId(self->widget),
    };
    return QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
}
/*
 * Satellite not setup yet
 */


SvLocal void QBChannelScanningWidgetSatelliteNotSetup(QBChannelScanningWidget self)
{
    QBDVBSatelliteDescriptor desc = (QBDVBSatelliteDescriptor) QBComboBoxGetValue(self->satellite);
    const char *title;
    char *buf;
    char *titleBuf;
    svSettingsRestoreContext(self->settingsCtx);
    asprintf(&buf, "%s.SatelliteNotSetupDialog", self->name);
    self->satelliteSetupDialog = QBChannelScanningWidgetDialogCreate(self, buf);
    self->satelliteSetupDialogId = svWidgetGetId(self->satelliteSetupDialog);
    QBDialogAddPanel(self->satelliteSetupDialog, SVSTRING("content"), NULL, 1);
    free(buf);

    asprintf(&buf, "%s.SatelliteNotSetupDialog", self->name);
    title = gettext(svSettingsGetString(buf, "title"));
    free(buf);
    if (title) {
        asprintf(&titleBuf, title, SvStringCString(QBDVBSatelliteDescriptorGetName(desc)));
        QBDialogSetTitle(self->satelliteSetupDialog, titleBuf);
        free(titleBuf);
    }
    QBDialogSetNotificationCallback(self->satelliteSetupDialog, self, QBChannelScanningWidgetSatelliteSetupDialogNotificationCallback);
    asprintf(&buf, "%s.SatelliteNotSetupDialog", self->name);
    QBDialogAddLabel(self->satelliteSetupDialog, SVSTRING("content"), gettext(svSettingsGetString(buf, "info")), SVSTRING("info"), 1);
    free(buf);
    QBDialogAddButton(self->satelliteSetupDialog, SVSTRING("Setup"), gettext("Setup"), 1);
    QBDialogAddButton(self->satelliteSetupDialog, SVSTRING("Cancel"), gettext("Cancel"), 1);

    QBDialogRun(self->satelliteSetupDialog, NULL, NULL);
    svSettingsPopComponent();
}


/*
 * No tuner available
 */

SvLocal void QBChannelScanningWidgetNoTunerSetup(QBChannelScanningWidget self)
{
    char *buf;
    asprintf(&buf, "%s.Dialog", self->name);
    svSettingsRestoreContext(self->settingsCtx);
    self->noTunerDialog = QBChannelScanningWidgetDialogCreate(self, buf);
    free(buf);
    self->noTunerDialogId = svWidgetGetId(self->noTunerDialog);
    QBDialogAddPanel(self->noTunerDialog, SVSTRING("content"), NULL, 1);
    asprintf(&buf, "%s.NoTunerPopup", self->name);
    QBDialogSetTitle(self->noTunerDialog, gettext(svSettingsGetString(buf, "title")));
    QBDialogSetNotificationCallback(self->noTunerDialog, self, QBChannelScanningWidgetNoTunerDialogNotificationCallback);
    QBDialogAddLabel(self->noTunerDialog, SVSTRING("content"), gettext(svSettingsGetString(buf, "info")), SVSTRING("info"), 1);
    self->noTunerConfirm = QBDialogAddButton(self->noTunerDialog, SVSTRING("Close"), gettext("Close"), 1);
    free(buf);
    svSettingsPopComponent();
    QBDialogRun(self->noTunerDialog, NULL, NULL);
}

SvLocal void QBChannelScanningWidgetNoTuner(QBChannelScanningWidget self)
{
    QBChannelScanningWidgetBreak(self);
    QBChannelScanningWidgetNoTunerSetup(self);
}

SvLocal void QBChannelScanningWidgetNoTunerBreak(QBChannelScanningWidget self)
{
    QBDialogBreak(self->noTunerDialog);
}

/*
 * Drop channels
 */

SvLocal void QBChannelScanningWidgetDropStep(void *self_)
{
    QBChannelScanningWidget self = self_;
    SvFiberDeactivate(self->dropFiber);
    SvFiberEventDeactivate(self->dropTimer);
    if (SvInvokeInterface(SvEPGChannelListPlugin, self->dvbEPGPlugin, isPropagatingChannels) ||
        SvEPGChannelListIsPropagatingChannels(SvEPGManagerGetChannelsList(self->epgManager))) {
        SvFiberTimerActivateAfter(self->dropTimer, SvTimeFromMs(250));
        return;
    }

    SvFiberDestroy(self->dropFiber);
    self->dropFiber = NULL;

    //Drop reminders
    QBReminder reminder = (QBReminder) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                   SVSTRING("QBReminder"));
    QBReminderRemoveAll(reminder);

    QBChannelScanningWidgetFinishDrop(self);
}

SvLocal void QBChannelScanningWidgetSetupStartDropChannels(QBChannelScanningWidget self)
{
    char *buf;
    asprintf(&buf, "%s.Dialog", self->name);
    svSettingsRestoreContext(self->settingsCtx);
    self->dropDialog = QBChannelScanningWidgetDialogCreate(self, buf);
    free(buf);
    self->dropDialogId = svWidgetGetId(self->dropDialog);
    QBDialogAddPanel(self->dropDialog, SVSTRING("content"), NULL, 1);
    asprintf(&buf, "%s.DropPopup", self->name);
    QBDialogSetTitle(self->dropDialog, gettext(svSettingsGetString(buf, "title")));
    QBDialogAddLabel(self->dropDialog, SVSTRING("content"), gettext(svSettingsGetString(buf, "question")), SVSTRING("question"), 1);
    QBDialogSetNotificationCallback(self->dropDialog, self, QBChannelScanningWidgetDropDialogNotificationCallback);
    self->dropConfirm = QBDialogAddButton(self->dropDialog, SVSTRING("confirm"), gettext("Confirm"), 1);
    self->dropCancel = QBDialogAddButton(self->dropDialog, SVSTRING("cancel"), gettext("Cancel"), 1);
    free(buf);

    QBDialogRun(self->dropDialog, NULL, NULL);

    svSettingsPopComponent();
}

SvLocal void QBChannelScanningWidgetStartDropChannels(QBChannelScanningWidget self)
{
    self->dropState = DROP_AWAIT_CONFIRMATION;

    QBChannelScanningWidgetSetupStartDropChannels(self);
}

SvLocal void QBChannelScanningWidgetSetupBreakDropChannels(QBChannelScanningWidget self)
{
    QBDialogBreak(self->dropDialog);
    self->dropDialog = NULL;
    QBChannelScanningWidgetCleanAfterDropDialogDestroy(self);
}

SvLocal void QBChannelScanningWidgetBreakDropChannels(QBChannelScanningWidget self)
{
    self->dropState = DROP_NOOP;
    QBChannelScanningWidgetSetupBreakDropChannels(self);
}

SvLocal void QBChannelScanningWidgetSetupConfirmDropChannels(QBChannelScanningWidget self)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBDialogRemoveItem(self->dropDialog, SVSTRING("question"));
    QBDialogRemoveItem(self->dropDialog, SVSTRING("confirm"));
    QBDialogRemoveItem(self->dropDialog, SVSTRING("cancel"));

    char *buf;
    asprintf(&buf, "%s.DropPopup", self->name);
    QBDialogAddLabel(self->dropDialog, SVSTRING("content"), gettext(svSettingsGetString(buf, "wait")), SVSTRING("please wait"), 1);
    QBDialogSetTitle(self->dropDialog, gettext(svSettingsGetString(buf, "waitTitle")));
    free(buf);
    asprintf(&buf, "%s.DropPopup.Animation", self->name);
    SvWidget animation = svSettingsWidgetCreate(self->widget->app, buf);
    float duration = svSettingsGetDouble(buf, "stepDuration", 0.2f);
    int bmpCnt = svSettingsGetInteger(buf, "steps", 0);
    if (bmpCnt > 0) {
        SvBitmap* bitmaps = calloc(bmpCnt, sizeof(SvBitmap));
        int i;
        for (i = 0; i < bmpCnt; i++) {
            char frameName[16];
            snprintf(frameName, sizeof(frameName), "frame%i", i);
            bitmaps[i] = SVRETAIN(svSettingsGetBitmap(buf, frameName));
        }
        SvEffect effect = svEffectAnimNew(animation, bitmaps, bmpCnt, SV_EFFECT_ANIM_FORWARD, SV_ANIM_LOOP_INFINITE, duration);
        for (i = 0; i < bmpCnt; i++)
            SVRELEASE(bitmaps[i]);
        free(bitmaps);
        svAppRegisterEffect(self->widget->app, effect);
    }

    QBDialogAddWidget(self->dropDialog, SVSTRING("content"), animation, SVSTRING("animation"), 2, buf);
    free(buf);

    svSettingsPopComponent();
}

SvLocal void QBChannelScanningWidgetConfirmDropChannels(QBChannelScanningWidget self)
{
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    DVBEPGPlugin channelListPlugin = (DVBEPGPlugin) QBGlobalStorageGetItem(globalStorage,
                                                                           DVBEPGPlugin_getType(),
                                                                           NULL);
    if (!channelListPlugin)
        return;
    DVBEPGPluginDropAllChannels(channelListPlugin);

    self->dropState = DROP_DROPPING;

    self->dropFiber = SvFiberCreate(self->scheduler, NULL, "QBChannelScanningWidgetDropChannels", QBChannelScanningWidgetDropStep, self);
    self->dropTimer = SvFiberTimerCreate(self->dropFiber);
    SvFiberActivate(self->dropFiber);

    QBChannelScanningWidgetSetupConfirmDropChannels(self);
}

SvLocal void QBChannelScanningWidgetSetupFinishDrop(QBChannelScanningWidget self)
{
    QBDialogBreak(self->dropDialog);
    self->dropDialog = NULL;
    QBChannelScanningWidgetCleanAfterDropDialogDestroy(self);
}

SvLocal void QBChannelScanningWidgetFinishDrop(QBChannelScanningWidget self)
{
    self->dropState = DROP_NOOP;
    QBTVLogic tvLogic = (QBTVLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(),
                                                           QBTVLogic_getType(),
                                                           NULL);
    QBTVLogicPlayChannelFromPlaylist(tvLogic, NULL, NULL, NULL);
    QBChannelScanningWidgetSetupFinishDrop(self);
}

SvLocal void QBChannelScanningWidgetClear(QBChannelScanningWidget self)
{
    self->currentStatus = NULL;
    if (self->scanner) {
        QBDVBScannerDestroy(self->scanner);
        self->scanner = NULL;
    }
}

SvLocal void QBChannelScanningWidgetStop(QBChannelScanningWidget self)
{
    if (self->scanner) {
        QBDVBScannerStop(self->scanner);
    }
}

SvLocal void QBChannelScanningWidgetSafeStop(QBChannelScanningWidget self)
{
    if (self->scanner) {
        QBDVBScannerSafeStop(self->scanner);
    }
}

SvLocal void QBChannelScanningWidgetGenerateAllFrequencies(QBChannelScanningWidget self, int **freq, int **band, int *freqCnt)
{
    if (band)
        *band = NULL;
    if (self->freqJumpValue <= 0 || self->scanning.startFreq == self->scanning.endFreq) {
        *freqCnt = 1;
        *freq = malloc((*freqCnt) * sizeof(int));
        (*freq)[0] = self->scanning.startFreq;
    } else {
        *freqCnt = (self->scanning.endFreq - self->scanning.startFreq) / (self->freqJumpValue * 1000) + 1;
        *freq = malloc((*freqCnt) * sizeof(int));
        int i, j;
        for (i = 0, j = self->scanning.startFreq; j <= self->scanning.endFreq; j += self->freqJumpValue * 1000, i++) {
            (*freq)[i] = j;
        }
    }
}

SvLocal SvString get_string(SvHashTable hashTable, SvString key)
{
    SvGenericObject val = SvHashTableFind(hashTable, (SvGenericObject) key);
    if (val && SvObjectIsInstanceOf(val, SvString_getType())) {
        return (SvString) val;
    }
    return NULL;
}


/*
 * For handling of CS_SCANNING -> CS_FINISHED_SCANNING state change.
 */

SvLocal void QBChannelScanningWidgetSetupResult(QBChannelScanningWidget self)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject channelView = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    SvGenericObject radioView = QBPlaylistManagerGetById(playlists, SVSTRING("RadioChannels"));
    unsigned int pluginID = SvInvokeInterface(SvEPGChannelListPlugin, self->dvbEPGPlugin, getID);

    int videoCnt = channelView ? SvInvokeInterface(SvEPGChannelView, channelView, getCountForPluginID, pluginID) : 0;
    int radioCnt = radioView   ? SvInvokeInterface(SvEPGChannelView, radioView, getCount)   : 0;

    /*
     * We should ask DVBEPGPlugin about these actually.
     */
    const int videoFoundCnt = self->scanning.TVChannelsCount;
    const int radioFoundCnt = self->scanning.radioChannelsCount;

    svSettingsRestoreContext(self->settingsCtx);

    QBDialogRemoveItem(self->dialog, SVSTRING("lock status"));
    QBDialogRemoveItem(self->dialog, SVSTRING("currentStatus"));
    QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
    QBDialogRemoveItem(self->dialog, SVSTRING("total"));
    QBDialogRemoveItem(self->dialog, SVSTRING("signal status"));

    const char* resultTitle = self->successfulScan ? gettext("Results - Success") : gettext("Results - Failure");
    QBDialogSetTitle(self->dialog, resultTitle);

    char *buf;
    asprintf(&buf, "%s.Results", self->name);
    self->timeLeft = svSettingsGetInteger(buf, "timeout", 0);
    free(buf);
    if (self->timeLeft > 0)
        self->timerID = svAppTimerStart(self->app, self->widget, self->timeLeft, true);

    asprintf(&buf, "%s.Results.Box", self->name);
    SvWidget results = NULL;
    results = svSettingsWidgetCreate(self->app, buf);
    free(buf);
    SvWidget videoTick = NULL;
    if (self->successfulScan && (videoCnt + videoFoundCnt)) {
        asprintf(&buf, "%s.Results.Success", self->name);
        videoTick = svSettingsWidgetCreate(self->app, buf);
        free(buf);
    } else {
        asprintf(&buf, "%s.Results.Failure", self->name);
        videoTick = svSettingsWidgetCreate(self->app, buf);
        free(buf);
    }
    asprintf(&buf, "%s.Results.Video.Tick", self->name);
    svSettingsWidgetAttach(results, videoTick, buf, 1);
    free(buf);

    asprintf(&buf, "%s.Results.Video.Description", self->name);
    SvWidget videoResult = svLabelNewFromSM(self->app, buf);
    char *videoStr;
    asprintf(&videoStr, gettext("%i (%i new or updated) TV channels"), videoCnt, videoFoundCnt);
    svLabelSetText(videoResult, videoStr);
    free(videoStr);

    svSettingsWidgetAttach(results, videoResult, buf, 1);
    free(buf);
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);
    if (QBChannelScanningLogicIsRadioEnabled(channelScanningLogic)) {
        SvWidget radioTick = NULL;
        if (self->successfulScan && (radioCnt + radioFoundCnt)) {
            asprintf(&buf, "%s.Results.Success", self->name);
            radioTick = svSettingsWidgetCreate(self->app, buf);
            free(buf);
        } else {
            asprintf(&buf, "%s.Results.Failure", self->name);
            radioTick = svSettingsWidgetCreate(self->app, buf);
            free(buf);
        }
        asprintf(&buf, "%s.Results.Radio.Tick", self->name);
        svSettingsWidgetAttach(results, radioTick, buf, 1);
        free(buf);

        asprintf(&buf, "%s.Results.Radio.Description", self->name);
        SvWidget radioResult = svLabelNewFromSM(self->app, buf);
        char *radioStr;
        asprintf(&radioStr, gettext("%i (%i new or updated) Radio channels"), radioCnt, radioFoundCnt);
        svLabelSetText(radioResult, radioStr);
        free(radioStr);
        svSettingsWidgetAttach(results, radioResult, buf, 1);
        free(buf);
    }

    SvHashTable extraResults = QBChannelScanningLogicGetResults(channelScanningLogic);
    if (extraResults) {
        SvGenericObject key = NULL;
        SvIterator it = SvHashTableKeysIterator(extraResults);
        while ((key = SvIteratorGetNext(&it))) {
            SvHashTable stepResults = (SvHashTable) SvHashTableFind(extraResults, key);
            if (!SvObjectIsInstanceOf((SvObject) stepResults, SvHashTable_getType())) {
                continue;
            }

            SvValue statusV = (SvValue) SvHashTableFind(stepResults, (SvGenericObject) SVSTRING("status"));
            bool status = false;
            if (statusV && SvObjectIsInstanceOf((SvObject) statusV, SvValue_getType()) && SvValueIsBoolean(statusV)) {
                status = SvValueGetBoolean(statusV);
            }

            SvString tickWidgetName = get_string(stepResults, SVSTRING("tickWidgetName"));
            SvString descWidgetName = get_string(stepResults, SVSTRING("descWidgetName"));
            SvString resultDescription = get_string(stepResults, SVSTRING("resultDescription"));
            if (tickWidgetName && descWidgetName && resultDescription) {
                SvWidget wdg = NULL;
                if (status) {
                    asprintf(&buf, "%s.Results.Success", self->name);
                    wdg = svSettingsWidgetCreate(self->app, buf);
                    free(buf);
                } else {
                    asprintf(&buf, "%s.Results.Failure", self->name);
                    wdg = svSettingsWidgetCreate(self->app, buf);
                    free(buf);
                }
                svSettingsWidgetAttach(results, wdg, SvStringCString(tickWidgetName), 1);

                SvWidget resultWdg = svLabelNewFromSM(self->app, SvStringCString(descWidgetName));
                svLabelSetText(resultWdg, SvStringCString(resultDescription));
                svSettingsWidgetAttach(results, resultWdg, SvStringCString(descWidgetName), 1);
            }
        }
    }

    asprintf(&buf, "%s.Results.Box", self->name);
    QBDialogAddWidget(self->dialog, SVSTRING("content"), results, SVSTRING("results"), 1, buf);
    free(buf);

    if (!self->params->params.forceAutomatic || !self->params->params.automatic)
        self->cancelButton = QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), gettext("Cancel"), 1);

    if (!self->successfulScan) {
        if (!self->cancelButton) {
            self->cancelButton = QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), gettext("Cancel"), 1);
        }
        QBDialogRemoveItem(self->dialog, SVSTRING("button"));
        self->button = NULL;
    } else {
        svButtonSetCaption(self->button, gettext("Save"));
    }

    QBDialogAlignButtons(self->dialog);

    svSettingsPopComponent();

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
}

SvLocal void QBChannelScanningWidgetFailureBreak(QBChannelScanningWidget self)
{
    QBDialogBreak(self->failureDialog);
}

SvLocal bool QBChannelScanningWidgetScanningFailedPopupNotificationCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningWidget self = (QBChannelScanningWidget) ptr;

    if (self->failureDialog) {
        if (!buttonTag) {
            return false;
        }
        QBChannelScanningWidgetFailureBreak(self);
        return true;
    }
    return false;
}

SvLocal void QBChannelScanningWidgetScanningFailedPopupCallback(void *self_,
                                                                SvWidget dialog,
                                                                SvString buttonTag,
                                                                unsigned keyCode)
{
    QBChannelScanningWidget self = self_;
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);
    QBChannelScanningLogicScanningFinished(channelScanningLogic, self->params->conf, false);
}

SvLocal void QBChannelScanningWidgetScanningFailed(QBChannelScanningWidget self)
{
    char* buf;
    asprintf(&buf, "%s.Dialog", self->name);
    svSettingsRestoreContext(self->settingsCtx);
    self->failureDialog = QBChannelScanningWidgetDialogCreate(self, buf);
    free(buf);
    QBDialogAddPanel(self->failureDialog, SVSTRING("content"), NULL, 1);
    asprintf(&buf, "%s.FailurePopup", self->name);
    QBDialogSetTitle(self->failureDialog, gettext(svSettingsGetString(buf, "title")));
    QBDialogAddLabel(self->failureDialog, SVSTRING("content"), gettext(svSettingsGetString(buf, "info")), SVSTRING("info"), 1);
    QBDialogAddButton(self->failureDialog, SVSTRING("OK"), gettext("OK"), 1);
    free(buf);
    svSettingsPopComponent();
    QBDialogSetNotificationCallback(self->failureDialog, self, QBChannelScanningWidgetScanningFailedPopupNotificationCallback);
    QBDialogRun(self->failureDialog, self, QBChannelScanningWidgetScanningFailedPopupCallback);
}

SvLocal void QBChannelScanningWidgetEndScanning(QBChannelScanningWidget self)
{
    self->scanning.radioChannelsCount = QBDVBScannerGetNumberOfRadioChannels(self->scanner);
    self->scanning.TVChannelsCount = QBDVBScannerGetNumberOfTVChannels(self->scanner);
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);
    self->successfulScan = QBChannelScanningLogicIsScanSuccessful(channelScanningLogic, self->params->conf, self->scanner);
    QBChannelScanningWidgetStop(self);

    SvLogNotice("Scanning : finished scanning");

    self->state = CS_FINISHED_SCANNING;

    if (self->breakButton) {
        QBDialogRemoveItem(self->dialog, SVSTRING("break-button"));
        self->breakButton = NULL;
    }

    if ((self->params->params.forceAutomatic && self->params->params.automatic) ||
        (self->params->params.forceAutosave && self->params->params.autosave)) {
        if (!self->successfulScan) {
            QBChannelScanningWidgetBreak(self);
            if (self->params->params.withFailureDialog) {
                QBChannelScanningWidgetScanningFailed(self);
            } else {
                QBChannelScanningLogicScanningFinished(channelScanningLogic, self->params->conf, false);
            }
        } else {
            QBChannelScanningWidgetStartSaving(self);
        }
    } else
        QBChannelScanningWidgetSetupResult(self);
}

/*
 * For handling CS_FINISHED_SCANNING -> CS_SAVING.
 */

SvLocal void QBChannelScanningWidgetSavingStep(void *self_)
{
    QBChannelScanningWidget self = self_;
    SvFiberDeactivate(self->savingFiber);
    SvFiberEventDeactivate(self->savingTimer);
    //If DVBEPGPlugin said it has processed everything
    if (!SvInvokeInterface(SvEPGChannelListPlugin, self->dvbEPGPlugin, isPropagatingChannels) &&
        !SvEPGChannelListIsPropagatingChannels(SvEPGManagerGetChannelsList(self->epgManager))) {
        SvFiberDestroy(self->savingFiber);
        self->savingFiber = NULL;
        QBChannelScanningWidgetFinishSaving(self);
    } else {
        SvFiberTimerActivateAfter(self->savingTimer, SvTimeFromMs(250));
    }
}

SvLocal void QBChannelScanningWidgetSetupStartSaving(QBChannelScanningWidget self)
{
    if (self->timerID) {
        svAppTimerStop(self->app, self->timerID);
        self->timerID = 0;
    }

    if (self->button) {
        if (self->breakButton)
            svButtonSetCaption(self->breakButton, gettext("Saving..."));
        else
            svButtonSetCaption(self->button, gettext("Saving..."));
        if (self->button)
            svButtonSetDisabled(self->button, true);
        QBDialogAlignButtons(self->dialog);
    }
}

SvLocal void QBChannelScanningWidgetStartSaving(QBChannelScanningWidget self)
{

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);
    QBChannelScanningLogicProcessFoundChannels(channelScanningLogic, self->params->conf, self->scanner);

    QBChannelScanningWidgetClear(self);

    self->savingFiber = SvFiberCreate(self->scheduler, NULL, "QBChannelScanningWidgetSaving", QBChannelScanningWidgetSavingStep, self);
    self->savingTimer = SvFiberTimerCreate(self->savingFiber);
    SvFiberActivate(self->savingFiber);

    self->state = CS_SAVING;

    QBChannelScanningWidgetSetupStartSaving(self);
}

/*
 * For handling CS_FINISHED_SCANNING -> CS_NOOP
 */

SvLocal void QBChannelScanningWidgetSetupBreakSaving(QBChannelScanningWidget self)
{
    if (self->timerID) {
        svAppTimerStop(self->app, self->timerID);
        self->timerID = 0;
    }

    if (self->dialog)
        QBDialogBreak(self->dialog);
    self->dialog = NULL;
    QBChannelScanningWidgetCleanAfterDialogDestory(self);
}

SvLocal void QBChannelScanningWidgetBreakSaving(QBChannelScanningWidget self)
{
    QBChannelScanningWidgetClear(self);

    self->state = CS_NOOP;

    QBChannelScanningWidgetSetupBreakSaving(self);
}

/*
 * For handling CS_SAVING -> CS_FINISHED_SAVING
 */

SvLocal void QBChannelScanningWidgetSetupFinishSaving(QBChannelScanningWidget self)
{
    if (self->dialog)
        QBDialogBreak(self->dialog);
    self->dialog = NULL;
    QBChannelScanningWidgetCleanAfterDialogDestory(self);
    QBChannelScanningWidgetFinish(self);
}

SvLocal void QBChannelScanningWidgetFinishSaving(QBChannelScanningWidget self)
{
    self->state = CS_FINISHED_SAVING;

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                  QBChannelScanningLogic_getType(),
                                                                                                  NULL);
    QBChannelScanningLogicScanningFinished(channelScanningLogic, self->params->conf, self->successfulScan);
    QBChannelScanningWidgetSetupFinishSaving(self);
}

/*
 * For handling CS_FINISHED_SAVING -> CS_NOOP
 */

SvLocal void QBChannelScanningWidgetSetupFinish(QBChannelScanningWidget self)
{
}

SvLocal void QBChannelScanningWidgetFinish(QBChannelScanningWidget self)
{
    self->state = CS_NOOP;

    QBChannelScanningWidgetSetupFinish(self);
}

/*
 * For handling CS_SCANNING -> CS_FINISHED_SCANNING state change
 * For handling CS_SCANNING -> CS_SCANNING state loop
 */

SvLocal void QBChannelScanningWidgetQBDVBScannerOnFinish(void *self_, QBDVBScanner* scanner)
{
    QBChannelScanningWidget self = self_;
    if (self->finishMode == FINISH_STOP_SAVE) {
        QBChannelScanningWidgetStartSaving(self);
    } else {
        QBChannelScanningWidgetEndScanning(self);
    }
    self->finishMode = FINISH_NORMAL;
}

SvLocal void QBChannelScanningWidgetQBDVBScannerOnStep(void *self_, QBDVBScanner* scanner)
{
    QBChannelScanningWidget self = self_;
    QBChannelScanningWidgetQBDVBScannerSetupOnStep(self, scanner);
}

SvLocal void QBChannelScanningWidgetQBDVBScannerOnRevoke(void *self_, QBDVBScanner* scanner)
{
    QBChannelScanningWidget self = self_;
    QBChannelScanningWidgetBreak(self);
    QBChannelScanningWidgetNoTuner(self);
}

/*
 * For handling CS_NOOP -> CS_SCANNING state change
 */

SvLocal void QBChannelScanningWidgetStartScanningSetupStart(QBChannelScanningWidget self)
{
    svSettingsRestoreContext(self->settingsCtx);
    char *buf;
    asprintf(&buf, "%s.Popup", self->name);
    self->scanningTitle = gettext(svSettingsGetString(buf, "title"));
    free(buf);
    asprintf(&buf, "%s.Dialog", self->name);
    SvWidget dialog = QBChannelScanningWidgetDialogCreate(self, buf);
    free(buf);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, self->scanningTitle);
    QBDialogSetNotificationCallback(dialog, self, QBChannelScanningWidgetDialogNotificationCallback);

    asprintf(&buf, "%s.Popup.Animation", self->name);
    if (svSettingsIsWidgetDefined(buf)) {
        SvWidget animation = svSettingsWidgetCreate(self->app, buf);
        float duration = svSettingsGetDouble(buf, "stepDuration", 0.2f);
        int bmpCnt = svSettingsGetInteger(buf, "steps", 0);
        if (bmpCnt > 0) {
            SvBitmap* bitmaps = calloc(bmpCnt, sizeof(SvBitmap));
            int i;
            for (i = 0; i < bmpCnt; i++) {
                char *frameName;
                asprintf(&frameName, "frame%i", i);
                bitmaps[i] = SVRETAIN(svSettingsGetBitmap(buf, frameName));
                free(frameName);
            }
            SvEffect effect = svEffectAnimNew(animation, bitmaps, bmpCnt, SV_EFFECT_ANIM_FORWARD, SV_ANIM_LOOP_INFINITE, duration);
            for (i = 0; i < bmpCnt; i++)
                SVRELEASE(bitmaps[i]);
            free(bitmaps);
            svAppRegisterEffect(self->widget->app, effect);
        }

        QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 1, buf);
        free(buf);
    }

    asprintf(&buf, "%s.Popup.Progress", self->name);
    if (self->scanning.startFreq != self->scanning.endFreq) {
        if (svSettingsIsWidgetDefined(buf)) {
            self->total = svGaugeNewFromSM(self->app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
            QBDialogAddWidget(dialog, SVSTRING("content"), self->total, SVSTRING("total"), 2, buf);
        }
    }
    free(buf);

    asprintf(&buf, "%s.Popup.LockStatus", self->name);
    if (svSettingsIsWidgetDefined(buf)) {
        free(buf);
        asprintf(&buf, "%s.Popup.LockStatus.Box", self->name);
        SvWidget lockStatusBox = svSettingsWidgetCreate(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.LockStatus", self->name);
        if (!self->lockOn)
            self->lockOn = SVRETAIN(svSettingsGetBitmap(buf, "lockOn"));
        if (!self->lockOff)
            self->lockOff = SVRETAIN(svSettingsGetBitmap(buf, "lockOff"));
        free(buf);
        asprintf(&buf, "%s.Popup.LockStatus.Icon", self->name);
        self->lockIndicator = svSettingsWidgetCreate(self->app, buf);
        free(buf);
        svWidgetSetBitmap(self->lockIndicator, self->lockOff);
        asprintf(&buf, "%s.Popup.LockStatus.Label", self->name);
        self->lockStatus = svLabelNewFromSM(self->app, buf);
        free(buf);
        svLabelSetText(self->lockStatus, gettext("Lock status - NO SIGNAL"));
        asprintf(&buf, "%s.Popup.LockStatus.Icon", self->name);
        svSettingsWidgetAttach(lockStatusBox, self->lockIndicator, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.LockStatus.Label", self->name);
        svSettingsWidgetAttach(lockStatusBox, self->lockStatus, buf, 0);
        free(buf);

        QBDialogAddWidget(dialog, SVSTRING("content"), lockStatusBox, SVSTRING("lock status"), 4, NULL);
    }

    asprintf(&buf, "%s.Popup.Status", self->name);
    if (svSettingsIsWidgetDefined(buf)) {
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Box", self->name);
        SvWidget statusLabelBox = svSettingsWidgetCreate(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.Low", self->name);
        SvWidget statusStrengthLow = svLabelNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.High", self->name);
        SvWidget statusStrengthHigh = svLabelNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Low", self->name);
        SvWidget statusQualityLow = svLabelNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.High", self->name);
        SvWidget statusQualityHigh = svLabelNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.Title", self->name);
        SvWidget statusStrengthTitle = svLabelNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Title", self->name);
        SvWidget statusQualityTitle = svLabelNewFromSM(self->app, buf);
        free(buf);

        asprintf(&buf, "%s.Popup.Status.Strength.Gauge", self->name);
        self->signalStrength = svGaugeNewFromSM(self->app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Gauge", self->name);
        self->signalQuality = svGaugeNewFromSM(self->app, buf, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 100, 0);
        free(buf);


        asprintf(&buf, "%s.Popup.Status.Box", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusLabelBox, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.Low", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusStrengthLow, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.High", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusStrengthHigh, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Low", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusQualityLow, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.High", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusQualityHigh, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.Title", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusStrengthTitle, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Title", self->name);
        svSettingsWidgetAttach(statusLabelBox, statusQualityTitle, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Strength.Gauge", self->name);
        svSettingsWidgetAttach(statusLabelBox, self->signalStrength, buf, 0);
        free(buf);
        asprintf(&buf, "%s.Popup.Status.Quality.Gauge", self->name);
        svSettingsWidgetAttach(statusLabelBox, self->signalQuality, buf, 0);
        free(buf);

        QBDialogAddWidget(dialog, SVSTRING("content"), statusLabelBox, SVSTRING("signal status"), 5, NULL);

        char *statusText = QBChannelScanningWidgetCreateScanningStatusText(0, -1, QBTunerModulation_unknown, QBTunerPolarization_unknown);
        self->currentStatus = QBDialogAddLabel(dialog, SVSTRING("content"), statusText, SVSTRING("currentStatus"), 3);
        free(statusText);
    }

    if (!self->params->params.forceAutomatic || !self->params->params.automatic) {
        self->button = QBDialogAddButton(dialog, SVSTRING("button"), gettext("Cancel"), 1);
        asprintf(&buf, "%s.Popup", self->name);
        bool hasBreakButton = svSettingsGetBoolean(buf, "hasBreakButton", false);
        free(buf);
        if (hasBreakButton)
            self->breakButton = QBDialogAddButton(dialog, SVSTRING("break-button"), gettext("Finish"), 1);
    }

    self->dialog = dialog;

    self->dialogId = svWidgetGetId(dialog);

    QBDialogRun(self->dialog, NULL, NULL);

    self->hasLock = false;

    SvFiberActivate(self->fiber);
}

SvLocal void QBChannelScanningWidgetPrepareData(QBChannelScanningWidget self, QBDVBScannerParams* params)
{
    int bandwidth = self->freqJumpValue > 0 ? self->freqJumpValue : 8; //temp hack

    self->scanning.startFreq = self->freqMin ? QBFrequencyInputGetValue(self->freqMin) : self->freqMinValue;
    self->scanning.startFreq = self->freqMin ? QBFrequencyInputGetValue(self->freqMin) : self->freqMinValue;
    //self->scanning.endFreq = self->freqMax ? QBFrequencyInputGetValue(self->freqMax) : (self->freqMin ? QBFrequencyInputGetValue(self->freqMin) : self->freqMinValue);
    self->scanning.endFreq = self->scanning.startFreq; // AMERELES Remoción de parámetros en Escaneo Manual
    self->scanning.stepFreq = bandwidth;
    self->scanning.symbolRate = QBNumericInputGetValue(self->symbolRate);

    if (!self->params->conf || !(self->params->conf->forceUsePredefinedTransponders && self->params->conf->usePredefinedTransponders)) {
        if ((!self->params->conf || !self->params->conf->modulationCnt) && self->modulation) {
            QBTunerModulation modValue = SvValueGetInteger((SvValue) QBComboBoxGetValue(self->modulation));
            if (modValue != QBTunerModulation_default) {
                params->modulationCnt = 1;
                params->modulations = malloc(sizeof(QBTunerModulation));
                params->modulations[0] = modValue;
            }
        }

        if (self->polarization) {
            QBTunerPolarization polValue = SvValueGetInteger((SvValue) QBComboBoxGetValue(self->polarization));
            if (polValue != QBTunerPolarization_default) {
                params->polarizations = malloc(sizeof(QBTunerPolarization));
                params->polarizations[0] = polValue;
                params->polarizationCnt = 1;
            }
        }

        if (self->extra) {
            int extraVal = SvValueGetInteger((SvValue) QBComboBoxGetValue(self->extra));
            if (extraVal != -1) {
                params->extraVal = malloc(sizeof(int));
                params->extraVal[0] = extraVal;
                params->extraValCnt = 1;
            }
        }
        if ((!self->params->conf || !self->params->conf->symbolRateCnt) && self->symbolRate) {
            params->symbolRates = malloc(sizeof(int));
            params->symbolRates[0] = QBNumericInputGetValue(self->symbolRate);
            params->symbolRateCnt = 1;
        }

        if ((!self->params->conf || !self->params->conf->rangeCnt) && (self->freqMin)) {
            QBChannelScanningWidgetGenerateAllFrequencies(self, &params->freq, &params->forced.band, &params->freqCnt);
        }
    }

    if (self->satellite) {
        QBDVBSatelliteDescriptor satelliteDescriptor = (QBDVBSatelliteDescriptor) QBComboBoxGetValue(self->satellite);
        if (satelliteDescriptor) {
            SVTESTRELEASE(params->satelliteID);
            params->satelliteID = SVRETAIN(QBDVBSatelliteDescriptorGetID(satelliteDescriptor));
        }
    }

    params->onlyActual = false;
    params->chasing.chaseChannels = false;
    params->band = bandwidth;

    int NID, ONID, BOUQUETID, confTmp = 0, status = 0;
    status = QBConfigGetInteger("NID", &confTmp);
    NID = status ? -1 : confTmp;
    status = QBConfigGetInteger("ONID", &confTmp);
    ONID = status ? -1 : confTmp;
    status = QBConfigGetInteger("BOUQUET_ID", &confTmp);
    BOUQUETID = status ? -1 : confTmp;

    const char* scanStandard = QBConfigGet("SCANSTANDARD");
    const char* defaultSatelliteID = QBConfigGet("DEFAULTSATELLITE");
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBStandardRastersManager standardRastersManager = (QBStandardRastersManager)
        QBGlobalStorageGetItem(globalStorage, QBStandardRastersManager_getType(), NULL);
    QBStandardRastersConfig rastersConfig = QBStandardRastersManagerGetConfig(standardRastersManager);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBChannelScanningConfFillParams(self->params->conf, params, rastersConfig, tunerReserver, NID, ONID, BOUQUETID, scanStandard, self->satellitesDB, defaultSatelliteID);
}

SvLocal void QBChannelScanningWidgetStartScanning(QBChannelScanningWidget self)
{
    if (self->state == CS_SCANNING)
        return;

    if (self->freqMax && QBFrequencyInputGetValue(self->freqMin) > QBFrequencyInputGetValue(self->freqMax)) {
        QBFrequencyInputSetValue(self->freqMax, QBFrequencyInputGetValue(self->freqMin));
    }

    QBTunerLNBConfig LNB = NULL;
    if (self->satellite) {
        QBDVBSatelliteDescriptor satelliteDescriptor = (QBDVBSatelliteDescriptor) QBComboBoxGetValue(self->satellite);
        if (satelliteDescriptor) {
            LNB = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(satelliteDescriptor));
        }
    } else {
        if (self->params->conf && self->params->conf->forceSatelliteID) {
            LNB = QBTunerGetLNBConfigByID(self->params->conf->satelliteID);
        }
    }

    QBTunerResv* resv = QBTunerResvCreate(SVSTRING("Channel scanning manual check"), self, NULL);
    resv->params.tunerOwnership = true;
    resv->params.priority = self->priority;

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    bool canObtain = QBTunerReserverCanObtain(tunerReserver, resv, NULL);
    SVRELEASE(resv);

    if ((self->satellite || (self->params->conf && self->params->conf->forceSatelliteID)) && !LNB) {
        QBChannelScanningWidgetSatelliteNotSetup(self);
    } else if (!canObtain) {
        QBChannelScanningWidgetNoTuner(self);
    } else {
        self->state = CS_SCANNING;

        QBDVBScannerParams params = QBDVBScannerParamsGetEmpty();
        if (LNB)
            params.satelliteID = SVRETAIN(LNB->ID);
        QBChannelScanningWidgetPrepareData(self, &params);
        params.target = self;
        params.onFinish = QBChannelScanningWidgetQBDVBScannerOnFinish;
        params.onStep = QBChannelScanningWidgetQBDVBScannerOnStep;
        params.onRevoke = QBChannelScanningWidgetQBDVBScannerOnRevoke;
        params.satellitesDB = self->satellitesDB;
        params.reserver = tunerReserver;
        params.priority = self->priority;

        self->scanner = QBDVBScannerCreate(self->scheduler, self->params->conf ? self->params->conf->DVBScannerPlugins : NULL);

        QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
        QBChannelScanningLogic channelScanningLogic = (QBChannelScanningLogic) QBGlobalStorageGetItem(globalStorage,
                                                                                                      QBChannelScanningLogic_getType(),
                                                                                                      NULL);
        QBChannelScanningLogicSetDVBScannerPlugins(channelScanningLogic, self->scanner);
        QBDVBScannerStart(self->scanner, &params);

        QBChannelScanningWidgetStartScanningSetupStart(self);
    }

    QBChannelScanningConfUserParams userParams;
    int freqMin = self->freqMin ? QBFrequencyInputGetValue(self->freqMin) : self->freqMinValue;
    userParams = QBChannelScanningConfUserParamsCreate(freqMin,
                                                       self->freqMax ? QBFrequencyInputGetValue(self->freqMax) : freqMin,
                                                       self->freqJumpValue,
                                                       QBNumericInputGetValue(self->symbolRate),
                                                       self->modulation ? QBComboBoxGetPosition(self->modulation) : -1);
    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConfManagerSetUserParams(channelScanningConf, self->params->conf, userParams);
    if (LNB) {
        QBConfigSet("DEFAULTSATELLITE", SvStringCString(LNB->ID));
        QBConfigSave();
    }
    SVRELEASE(userParams);
}

/*
 * For handling CS_SCANNING -> CS_NOOP state change
 */

SvLocal void QBChannelScanningWidgetBreakSetup(QBChannelScanningWidget self)
{
    if (self->dialog)
        QBDialogBreak(self->dialog);
    self->dialog = NULL;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
}

SvLocal void QBChannelScanningWidgetBreak(QBChannelScanningWidget self)
{
    QBChannelScanningWidgetClear(self);

    self->state = CS_NOOP;

    QBChannelScanningWidgetBreakSetup(self);
}

/*
 * For handling ComboBoxes
 */
SvLocal SvString QBChannelScanningWidgetComboBoxPrepareSatellite(void *self_, SvWidget combobox, SvGenericObject value)
{
    SvString name = QBDVBSatelliteDescriptorGetName((QBDVBSatelliteDescriptor) value);
    return SVRETAIN(name);
}

//SvLocal SvString QBChannelScanningWidgetComboBoxPrepareJumpFreq(void *self_, SvWidget combobox, SvGenericObject value)
//{
//    int freq = SvValueGetInteger((SvValue) value);
//    return SvStringCreateWithFormat("%iMHz", freq);
//}

SvLocal SvString QBChannelScanningWidgetComboBoxPreparePolarization(void *self_, SvWidget combobox, SvGenericObject value)
{
    int val = SvValueGetInteger((SvValue) value);
    if (val == QBTunerPolarization_Vertical)
        return SvStringCreate(gettext("Vertical"), NULL);
    else if (val == QBTunerPolarization_Horizontal)
        return SvStringCreate(gettext("Horizontal"), NULL);
    else
        return SvStringCreate(gettext("Any"), NULL);
}

SvLocal SvString QBChannelScanningWidgetComboBoxPrepareModulation(void *self_, SvWidget combobox, SvGenericObject value)
{
    int val = SvValueGetInteger((SvValue) value);
    if (val != QBTunerModulation_default)
        return SvStringCreate(QBTunerModulationToString(val), NULL);
    else
        return SvStringCreate(gettext("Any"), NULL);
}

SvLocal SvString QBChannelScanningWidgetComboBoxPrepareExtra(void *self_, SvWidget combobox, SvGenericObject value)
{
    QBChannelScanningWidget self = self_;
    int val = SvValueGetInteger((SvValue) value);
    if (val == -1)
        return SvStringCreate(gettext("Any"), NULL);
    else
        return SvStringCreate((*self->tunerProps.names.extra_val_name)[val], NULL);
}

SvLocal SvString QBChannelScanningWidgetComboBoxPrepareString(void *self_, SvWidget combobox, SvGenericObject value)
{
    return (SvString) value;
}

//SvLocal int QBChannelScanningWidgetComboBoxChangeFreqJump(void * target, SvWidget combobox, SvGenericObject value, int key)
//{
//    int freqJump = SvValueGetInteger((SvValue) value);
//    if (freqJump <= 6 || (freqJump < 8 && key == QBKEY_LEFT))
//        return 0;
//    else
//        return 1;
//}

//SvLocal bool QBChannelScanningWidgetComboBoxInputFreqJump(void *target, SvWidget combobox, SvInputEvent e)
//{
//    QBChannelScanningWidget self = target;
//    const int key = e->ch;
//    if (key <= '0' || key > '9')
//        return false;
//
//    int freqJumpValue = (key - '0');
//
//    if ((freqJumpValue < self->tunerProps.allowed.band_min) || (freqJumpValue > self->tunerProps.allowed.band_max))
//        return false;
//
//    SvValue value = SvValueCreateWithInteger(freqJumpValue, NULL);
//    QBComboBoxSetValue(combobox, (SvGenericObject) value);
//    SVRELEASE(value);
//
//    return true;
//}

//SvLocal void QBChannelScanningWidgetComboBoxCurrentFreqJump(void *target, SvWidget combobox, SvGenericObject value)
//{
//    QBChannelScanningWidget self = target;
//    self->freqJumpValue = SvValueGetInteger((SvValue) value);
//
//    QBFrequencyInputUpdateRange(self->freqMin, self->freqJumpValue);
//    if (self->freqMax) {
//        QBFrequencyInputUpdateRange(self->freqMax, self->freqJumpValue);
//    }
//}

/*
 * Window creation and destruction
 */

SvLocal int
SatelliteSetupCompareSatellites(void *null_,
                                SvGenericObject satA_,
                                SvGenericObject satB_)
{
    SvString fileNameA, fileNameB;
    fileNameA = QBDVBSatelliteDescriptorGetFileName((QBDVBSatelliteDescriptor) satA_);
    fileNameB = QBDVBSatelliteDescriptorGetFileName((QBDVBSatelliteDescriptor) satB_);
    return strcmp(SvStringCString(fileNameA), SvStringCString(fileNameB));
}

SvLocal void QBChannelScanningWidgetShowSideMenu(QBChannelScanningWidget self)
{
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));

    struct QBChannelScanningPaneParams params = { .app = self->app, .scheduler = self->scheduler, .textRenderer = textRenderer };
    QBChannelScanningPane pane = QBChannelScanningPaneCreateFromSettings("BasicPane.settings", "QBChannelScanningManualPane.json", &params, self->sidemenu.ctx);
    if (pane) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) pane);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(pane);
    }
}

SvLocal void QBChannelScanningWidgetHideSideMenu(void *self_, QBContextMenu ctx)
{
    QBContextMenuHide(ctx, false);
}

SvLocal void QBChannelScanningWidgetCreateSetupAll(QBChannelScanningWidget self, const char *name, SvApplication app, bool useRatioSettings)
{
    sv_tuner_get_properties(0, &self->tunerProps);  //all tuners are the same
    char *buf;
    QBTunerType tunerType = QBTunerType_unknown;
    sv_tuner_get_type(0, &tunerType);

    QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                               SVSTRING("QBApplicationController"));

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", controller, self->app, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBChannelScanningWidgetHideSideMenu, self);

    self->sideMenuSettingsEnabled = svSettingsGetBoolean(self->name, "sideMenuSettingsEnabled", true);

    asprintf(&buf, "%s.TunerTypeHint", self->name);
    if (svSettingsIsWidgetDefined(buf)) {
        const char *text = svSettingsGetString(buf, QBTunerTypeToString(tunerType));
        if (text) {
            SvWidget hint = useRatioSettings ? svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height) : svLabelNewFromSM(self->app, buf);
            if (hint) {
                svLabelSetText(hint, gettext(text));
                if (useRatioSettings)
                    svSettingsWidgetAttachWithRatio(self->widget, hint, buf, 1);
                else
                    svSettingsWidgetAttach(self->widget, hint, buf, 1);
            }
        }
    }
    free(buf);
    QBTunerStandard standard = QBTunerStandard_unknown;
    if (self->tunerProps.allowed.standard_cnt) {
        const char* standardString = QBConfigGet("SCANSTANDARD");
        if (standardString) {
            standard = QBTunerStandardFromString(standardString);
        }
    }

    SvWidget satelliteBox = NULL, satelliteDesc = NULL;
    SvWidget freqMinBox = NULL, freqMinDesc = NULL;
    SvWidget freqMaxBox = NULL, freqMaxDesc = NULL;
    //SvWidget freqJump = NULL, freqJumpDesc = NULL;
    QBChannelScanningConfUserParams userParams = NULL;

    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    userParams = SVTESTRETAIN((QBChannelScanningConfUserParams) QBChannelScanningConfManagerGetUserParams(channelScanningConf, self->params->conf));

    if (!userParams) {
        int symbolRate = self->tunerProps.defaults.symbol_rate;
        if (self->params->conf && self->params->conf->symbolRateCnt) {
            symbolRate = self->params->conf->symbolRate[0];
        }

        int freqMin = self->tunerProps.allowed.freq_min;
        int freqMax = self->tunerProps.allowed.freq_max;
        int band = self->tunerProps.defaults.band;
        if (self->params->conf && self->params->conf->rangeCnt) {
            QBChannelScanningConfRange range = &self->params->conf->range[0];
            freqMin = range->freqMin;
            freqMax = range->freqMax;
            band = range->freqJump;
        }
        userParams = QBChannelScanningConfUserParamsCreate(freqMin, freqMax, band, symbolRate, 0);
    }

    if (tunerType == QBTunerType_sat) {
        asprintf(&buf, "%s.UltraWideComboBox", self->name);
        if (useRatioSettings)
            satelliteBox = QBComboBoxNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            satelliteBox = QBComboBoxNewFromSM(self->app, buf);
        free(buf);
        self->satellite = satelliteBox;

        if (self->satellitesDB && QBDVBSatellitesDBIsLoaded(self->satellitesDB)) {
            SvImmutableArray src = QBDVBSatellitesDBGetSatellites(self->satellitesDB);
            size_t cnt = SvImmutableArrayCount(src);
            SvArray satellites = SvArrayCreateWithCapacity(cnt, NULL);
            SvIterator satIter = SvImmutableArrayIterator(src);
            SvArrayAddObjects(satellites, &satIter);
            // sort satellites from west to east
            SvArraySortWithCompareFn(satellites, SatelliteSetupCompareSatellites, NULL);

            QBComboBoxCallbacks cb = {
                .prepare = QBChannelScanningWidgetComboBoxPrepareSatellite
            };
            QBComboBoxSetCallbacks(satelliteBox, self, cb);

            QBComboBoxSetContent(satelliteBox, satellites);
            const char *satelliteID = self->params->conf->forceSatelliteID ? SvStringCString(self->params->conf->satelliteID) : NULL;
            const char *defSatID = QBConfigGet("DEFAULTSATELLITE");
            if (satelliteID || defSatID) {
                QBDVBSatelliteDescriptor desc = NULL;
                unsigned int i = 0;
                for (i = 0; i < SvArrayCount(satellites); i++) {
                    desc = (QBDVBSatelliteDescriptor) SvArrayAt(satellites, i);
                    QBTunerLNBConfig LNB = QBTunerGetLNBConfigByID(QBDVBSatelliteDescriptorGetID(desc));
                    if (LNB &&
                        ((satelliteID && SvStringEqualToCString(LNB->ID, satelliteID)) ||
                         (defSatID && SvStringEqualToCString(LNB->ID, defSatID)))) {
                        QBComboBoxSetPosition(satelliteBox, i);
                        break;
                    }
                }
                if (i == SvImmutableArrayCount(satellites))
                    QBComboBoxSetPosition(satelliteBox, 0);
            } else {
                QBComboBoxSetPosition(satelliteBox, 0);
            }
            SVRELEASE(satellites);
            SVRELEASE(src);
        }
        asprintf(&buf, "%s.Satellite.Description", self->name);
        if (useRatioSettings)
            satelliteDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            satelliteDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
    }

    bool isRangeNeeded = true;
    bool isModulationNeeded = true;
    bool isPolarizationNeeded = true;
    bool isSymbolRateNeeded = true;
    bool isExtraValNeeded = true;
    if (self->params->conf) {
        isModulationNeeded = !self->params->conf->modulation && (!self->params->conf->automaticScanParams || !SvHashTableFind(self->params->conf->automaticScanParams, (SvGenericObject) SVSTRING("modulation")));
        isPolarizationNeeded = !self->params->conf->polarization && (!self->params->conf->automaticScanParams || !SvHashTableFind(self->params->conf->automaticScanParams, (SvGenericObject) SVSTRING("polarization")));
        isExtraValNeeded = (self->tunerProps.allowed.extra_cnt > 0) && (!self->params->conf->automaticScanParams || !SvHashTableFind(self->params->conf->automaticScanParams, (SvGenericObject) SVSTRING("extraVal")));
        if (!SvStringEqualToCString(self->params->conf->id, "manual")) {
            isRangeNeeded = !self->params->conf->range && (!self->params->conf->automaticScanParams || !SvHashTableFind(self->params->conf->automaticScanParams, (SvObject) SVSTRING("range")));
            isSymbolRateNeeded = !self->params->conf->symbolRate && (!self->params->conf->automaticScanParams || !SvHashTableFind(self->params->conf->automaticScanParams, (SvObject) SVSTRING("symbolRate")));
        }
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBStandardRastersManager standardRastersManager = (QBStandardRastersManager)
        QBGlobalStorageGetItem(globalStorage, QBStandardRastersManager_getType(), NULL);
    SvArray standardRastersParamsArray = NULL;
    if (((isRangeNeeded) || (!self->tunerProps.automatic.modulation && isModulationNeeded)) && standardRastersManager && self->params->conf && self->params->conf->useStandardRasters && self->params->conf->forceUseStandardRasters) {
        QBStandardRastersConfig rastersConfig = QBStandardRastersManagerGetConfig(standardRastersManager);
        standardRastersParamsArray = QBStandardRastersConfigCreateParams(rastersConfig,
                                                                         tunerType,
                                                                         standard,
                                                                         0);
    }

    int stdBand = userParams->range.freqJump;
    int stdBandPosition = 0;

    if (isRangeNeeded) {
        if (tunerType == QBTunerType_sat) {
            asprintf(&buf, "%s.Frequency.Input", self->name);
            if (useRatioSettings)
                freqMinBox = QBFrequencyInputNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMinBox = QBFrequencyInputNewFromSM(self->app, buf);
            free(buf);
            self->freqMin = freqMinBox;
            asprintf(&buf, "%s.Frequency.Description", self->name);
            if (useRatioSettings)
                freqMinDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMinDesc = svLabelNewFromSM(self->app, buf);
            free(buf);
        } else {
            asprintf(&buf, "%s.FrequencyMin.Input", self->name);
            if (useRatioSettings)
                freqMinBox = QBFrequencyInputNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMinBox = QBFrequencyInputNewFromSM(self->app, buf);
            free(buf);
            self->freqMin = freqMinBox;

            asprintf(&buf, "%s.FrequencyMin.Description", self->name);
            if (useRatioSettings)
                freqMinDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMinDesc = svLabelNewFromSM(self->app, buf);
            free(buf);
/* AMERELES Remoción de parámetros en Escaneo Manual
            asprintf(&buf, "%s.FrequencyMax.Input", self->name);
            if (useRatioSettings)
                freqMaxBox = QBFrequencyInputNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMaxBox = QBFrequencyInputNewFromSM(self->app, buf);
            free(buf);


            self->freqMax = freqMaxBox;
            asprintf(&buf, "%s.FrequencyMax.Description", self->name);
            if (useRatioSettings)
                freqMaxDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            else
                freqMaxDesc = svLabelNewFromSM(self->app, buf);
            free(buf);
*/
        }

        SvArray values = NULL;

        if (tunerType != QBTunerType_sat) {
            // there is no frequency jump combo box in manual satellite scanning
            //asprintf(&buf, "%s.ComboBox", self->name);
            //if (useRatioSettings)
            //    freqJump = QBComboBoxNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            //else
            //    freqJump = QBComboBoxNewFromSM(self->app, buf);
            //free(buf);
            //self->freqJump = freqJump;

            //QBComboBoxCallbacks cb = {
            //    .prepare = QBChannelScanningWidgetComboBoxPrepareJumpFreq,
            //    .input =  QBChannelScanningWidgetComboBoxInputFreqJump,
            //    .change = QBChannelScanningWidgetComboBoxChangeFreqJump,
            //    .current = QBChannelScanningWidgetComboBoxCurrentFreqJump
            //};
            //QBComboBoxSetCallbacks(freqJump, self, cb);

            values = SvArrayCreate(NULL);

            if ((standardRastersParamsArray != NULL) && (SvArrayCount(standardRastersParamsArray) != 0)) {
                bool stdBandFound = false;
                int i = 0;
                QBStandardRastersParams standardRastersParams = NULL;
                SvIterator standardRastersParamsArrayIter = SvArrayIterator(standardRastersParamsArray);
                while ((standardRastersParams = (QBStandardRastersParams) SvIteratorGetNext(&standardRastersParamsArrayIter))) {
                    if ((self->tunerProps.allowed.band_min <= standardRastersParams->band) && (self->tunerProps.allowed.band_max >= standardRastersParams->band)) {
                        SvValue bandValue = SvValueCreateWithInteger(standardRastersParams->band, NULL);
                        SvArrayAddObject(values, (SvGenericObject) bandValue);
                        if (userParams->range.freqJump == standardRastersParams->band) {
                            stdBandFound = true;
                            stdBandPosition = i;
                        }
                        SVRELEASE(bandValue);
                    }
                    ++i;
                }
                if (SvArrayCount(values) == 0) {
                    for (i = self->tunerProps.allowed.band_min; i <= self->tunerProps.allowed.band_max; i++) {
                        SvValue value = SvValueCreateWithInteger(i, NULL);
                        SvArrayAddObject(values, (SvGenericObject) value);
                        SVRELEASE(value);
                        if (userParams->range.freqJump == i) {
                            stdBandFound = true;
                            stdBandPosition = i - self->tunerProps.allowed.band_min;
                        }
                    }
                }
                if (!stdBandFound) {
                    stdBand = SvValueGetInteger((SvValue) SvArrayObjectAtIndex(values, 0));
                    stdBandPosition = 0;
                } else {
                    stdBand = userParams->range.freqJump;
                }
            } else {
                bool stdBandFound = false;
                for (int i = self->tunerProps.allowed.band_min; i <= self->tunerProps.allowed.band_max; i++) {
                    SvValue value = SvValueCreateWithInteger(i, NULL);
                    SvArrayAddObject(values, (SvGenericObject) value);
                    SVRELEASE(value);
                    if (userParams->range.freqJump == i)
                        stdBandFound = true;
                    stdBandPosition = i - self->tunerProps.allowed.band_min;
                }
                if (!stdBandFound) {
                    stdBand = SvValueGetInteger((SvValue) SvArrayObjectAtIndex(values, 0));
                    stdBandPosition = 0;
                } else {
                    stdBand = userParams->range.freqJump;
                }
            }
            self->freqJumpValue = stdBand;
            //QBComboBoxSetContent(freqJump, values);
            //QBComboBoxSetPosition(freqJump, stdBandPosition);
            SVRELEASE(values);

            //asprintf(&buf, "%s.FrequencyJump.Description", self->name);
            //if (useRatioSettings)
            //    freqJumpDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
            //else
            //    freqJumpDesc = svLabelNewFromSM(self->app, buf);
            //free(buf);
        } else {
            stdBand = ((userParams->range.freqJump < self->tunerProps.allowed.band_min) || (userParams->range.freqJump > self->tunerProps.allowed.band_max)) ? self->tunerProps.allowed.band_min : userParams->range.freqJump;
            self->freqJumpValue = stdBand;
        }

        QBStandardRastersParams standardRastersParams = NULL;
        if (standardRastersManager && standardRastersParamsArray && SvArrayCount(standardRastersParamsArray)) {
            standardRastersParams = (QBStandardRastersParams) SvArrayObjectAtIndex(standardRastersParamsArray, stdBandPosition);
        }

        self->freqMinValue = (userParams->range.freqMin < self->tunerProps.allowed.freq_min ? self->tunerProps.allowed.freq_min : userParams->range.freqMin);
        if (standardRastersParams && self->freqMinValue < standardRastersParams->freq_min) {
            self->freqMinValue = standardRastersParams->freq_min;
        }
        QBFrequencyInputSetParameters(self->freqMin,
                                      standardRastersParams,
                                      self->params->conf,
                                      self->freqMinValue,
                                      stdBand);

        if (self->freqMax) {
            self->freqMaxValue = (userParams->range.freqMax > self->tunerProps.allowed.freq_max ? self->tunerProps.allowed.freq_max : userParams->range.freqMax);
            if (standardRastersParams && self->freqMaxValue > standardRastersParams->freq_max) {
                self->freqMaxValue = standardRastersParams->freq_max;
            }
            QBFrequencyInputSetParameters(self->freqMax,
                                          standardRastersParams,
                                          self->params->conf,
                                          self->freqMaxValue,
                                          stdBand);
        }
    }

    SvWidget symbolRate = NULL;
    SvWidget symbolRateDesc = NULL;

    if (!self->tunerProps.automatic.symbol_rate && isSymbolRateNeeded) {
        asprintf(&buf, "%s.SymbolRate", self->name);
        if (useRatioSettings)
            symbolRate = QBNumericInputNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            symbolRate = QBNumericInputNewFromSM(self->app, buf);
        free(buf);
        asprintf(&buf, "%s.SymbolRate.Description", self->name);
        if (useRatioSettings)
            symbolRateDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            symbolRateDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
    }

    SvWidget modulation = NULL;
    SvWidget modulationDesc = NULL;

    if (!self->tunerProps.automatic.modulation && isModulationNeeded) {
        asprintf(&buf, "%s.ComboBox", self->name);
        if (useRatioSettings)
            modulation = QBComboBoxNewFromRatio(app, buf, self->widget->width, self->widget->height);
        else
            modulation = QBComboBoxNewFromSM(self->app, buf);
        free(buf);
        self->modulation = modulation;

        QBComboBoxCallbacks cb = {
            .prepare = QBChannelScanningWidgetComboBoxPrepareModulation
        };
        QBComboBoxSetCallbacks(modulation, NULL, cb);

        SvArray values = SvArrayCreate(NULL);
        SvValue value = SvValueCreateWithInteger(QBTunerModulation_default, NULL);
        SvArrayAddObject(values, (SvGenericObject) value);
        SVRELEASE(value);
        int initPosition = 0;

        int modulationCnt = 0;
        SvArray ignoredMod = NULL;
        if (self->params->conf && self->params->conf->ignoredModulationCnt) {
            modulationCnt = self->params->conf->ignoredModulationCnt;
            if (modulationCnt) {
                ignoredMod = SvArrayCreate(NULL);
                for (int i = 0; i < modulationCnt; i++) {
                    SvValue mod = SvValueCreateWithInteger(self->params->conf->ignoredModulation[i], NULL);
                    SvArrayAddObject(ignoredMod, (SvGenericObject) mod);
                    SVRELEASE(mod);
                }
            }
        }

        if (standardRastersParamsArray && SvArrayCount(standardRastersParamsArray)) {
            QBStandardRastersParams standardRastersParams = (QBStandardRastersParams) SvArrayObjectAtIndex(standardRastersParamsArray, 0);
            SvIterator modulationsIter = SvArrayIterator(standardRastersParams->modulations);
            SvGenericObject modulationValue;
            while ((modulationValue = SvIteratorGetNext(&modulationsIter))) {
                SVAUTOINTVALUE(mod, QBTunerModulationFromString(SvValueGetStringAsCString((SvValue) modulationValue, NULL)));
                if (ignoredMod && SvArrayContainsObject(ignoredMod, (SvGenericObject) mod))
                    continue;

                for (int i = 0; i < self->tunerProps.allowed.modulation_cnt; i++) {
                    if ((int) self->tunerProps.allowed.modulation_tab[i] == SvValueGetInteger(mod)) {
                        value = SvValueCreateWithInteger(self->tunerProps.allowed.modulation_tab[i], NULL);
                        SvArrayAddObject(values, (SvGenericObject) value);
                        SVRELEASE(value);
                        if (self->tunerProps.allowed.modulation_tab[i] == userParams->modulation)
                            initPosition = i + 1;
                    }
                }
            }
        }

        if (SvArrayCount(values) <= 1) {
            for (int i = 0; i < self->tunerProps.allowed.modulation_cnt; i++) {
                SVAUTOINTVALUE(mod, self->tunerProps.allowed.modulation_tab[i]);
                if (ignoredMod && SvArrayContainsObject(ignoredMod, (SvGenericObject) mod))
                    continue;

                value = SvValueCreateWithInteger(self->tunerProps.allowed.modulation_tab[i], NULL);
                SvArrayAddObject(values, (SvGenericObject) value);
                SVRELEASE(value);
                if (self->tunerProps.allowed.modulation_tab[i] == userParams->modulation)
                    initPosition = i + 1;
            }
        }

        QBComboBoxSetContent(modulation, values);
        QBComboBoxSetPosition(modulation, initPosition);
        SVRELEASE(values);
        SVTESTRELEASE(ignoredMod);

        asprintf(&buf, "%s.Modulation.Description", self->name);
        if (useRatioSettings)
            modulationDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            modulationDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
    }

    SvWidget polarization = NULL;
    SvWidget polarizationDesc = NULL;

    if (!self->tunerProps.automatic.polarization && isPolarizationNeeded) {
        asprintf(&buf, "%s.ComboBox", self->name);
        if (useRatioSettings)
            polarization = QBComboBoxNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            polarization = QBComboBoxNewFromSM(self->app, buf);
        free(buf);
        self->polarization = polarization;

        QBComboBoxCallbacks cb = {
            .prepare = QBChannelScanningWidgetComboBoxPreparePolarization
        };
        QBComboBoxSetCallbacks(polarization, NULL, cb);

        SvArray values = SvArrayCreate(NULL);
        int i;

        SvValue value = SvValueCreateWithInteger(QBTunerPolarization_default, NULL);
        SvArrayAddObject(values, (SvGenericObject) value);
        SVRELEASE(value);

        for (i = 0; i < self->tunerProps.allowed.polarization_cnt; i++) {
            value = SvValueCreateWithInteger(self->tunerProps.allowed.polarization_tab[i], NULL);
            SvArrayAddObject(values, (SvGenericObject) value);
            SVRELEASE(value);
        }
        QBComboBoxSetContent(polarization, values);
        QBComboBoxSetPosition(polarization, 0);
        SVRELEASE(values);

        asprintf(&buf, "%s.Polarization.Description", self->name);
        if (useRatioSettings)
            polarizationDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            polarizationDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
    }

    SvWidget extraDesc = NULL;
    SvWidget extra = NULL;

    if (!self->tunerProps.automatic.extra_val && isExtraValNeeded) {
        asprintf(&buf, "%s.ComboBox", self->name);
        if (useRatioSettings)
            extra = QBComboBoxNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            extra = QBComboBoxNewFromSM(self->app, buf);
        free(buf);

        self->extra = extra;

        QBComboBoxCallbacks cb = {
            .prepare = QBChannelScanningWidgetComboBoxPrepareExtra
        };
        QBComboBoxSetCallbacks(extra, self, cb);

        SvArray values = SvArrayCreate(NULL);

        SvValue value = SvValueCreateWithInteger(-1, NULL);
        SvArrayAddObject(values, (SvGenericObject) value);
        SVRELEASE(value);

        for (int i = 0; i < self->tunerProps.allowed.extra_cnt; i++) {
            value = SvValueCreateWithInteger(i, NULL);
            SvArrayAddObject(values, (SvGenericObject) value);
            SVRELEASE(value);
        }

        QBComboBoxSetContent(extra, values);
        QBComboBoxSetPosition(extra, 0);
        SVRELEASE(values);


        asprintf(&buf, "%s.Extra.Description", self->name);
        if (useRatioSettings)
            extraDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            extraDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
        svLabelSetText(extraDesc, self->tunerProps.names.extra_name ? : gettext("Extra"));
    }

    SvWidget useNITSDT = NULL, useNITSDTDesc = NULL;

    asprintf(&buf, "%s.useNITSDT.Description", self->name);
    if (svSettingsIsWidgetDefined(buf)) {
        free(buf);
        asprintf(&buf, "%s.ComboBox", self->name);
        if (useRatioSettings)
            useNITSDT = QBComboBoxNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            useNITSDT = QBComboBoxNewFromSM(self->app, buf);
        free(buf);

        QBComboBoxCallbacks cb = {
            .prepare = QBChannelScanningWidgetComboBoxPrepareString
        };
        QBComboBoxSetCallbacks(useNITSDT, NULL, cb);

        SvArray values = SvArrayCreate(NULL);
        SvArrayAddObject(values, (SvGenericObject) SvStringCreate(gettext("Yes"), NULL));
        SvArrayAddObject(values, (SvGenericObject) SvStringCreate(gettext("No"), NULL));
        QBComboBoxSetContent(useNITSDT, values);

        asprintf(&buf, "%s.UseNITSDT.Description", self->name);
        if (useRatioSettings)
            useNITSDTDesc = svLabelNewFromRatio(self->app, buf, self->widget->width, self->widget->height);
        else
            useNITSDTDesc = svLabelNewFromSM(self->app, buf);
        free(buf);
    } else {
        free(buf);
    }

    asprintf(&buf, "%s.Input", self->name);
    size_t yOffset, ySpacing;
    if (useRatioSettings) {
        yOffset = (int) ((double) self->widget->height * svSettingsGetDouble(buf, "yOffsetRatio", 0.0));
        ySpacing = (int) ((double) self->widget->height * svSettingsGetDouble(buf, "ySpacingRatio", 1.0));
    } else {
        yOffset = svSettingsGetInteger(buf, "yOffset", 0);
        ySpacing = svSettingsGetInteger(buf, "ySpacing", 10);
    }
    free(buf);
    if (satelliteBox) {
        asprintf(&buf, "%s.Satellite.ComboBox", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, satelliteBox, xOffset, yOffset, 1);
        asprintf(&buf, "%s.Satellite.Description", self->name);
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        if (satelliteDesc)
            svWidgetAttach(self->widget, satelliteDesc, xOffset, yOffset - (satelliteDesc->height - satelliteBox->height) / 2, 1);
        yOffset += satelliteBox->height + ySpacing;
    }
/* AMERELES Remoción de parámetros en Escaneo Manual
    if (freqJump) {
        asprintf(&buf, "%s.FrequencyJump.ComboBox", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqJump, xOffset, yOffset, 1);
        asprintf(&buf, "%s.FrequencyJump.Description", self->name);
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqJumpDesc, xOffset, yOffset - (freqJumpDesc->height - freqJump->height) / 2, 1);

        yOffset += freqJump->height + ySpacing;
    }
*/
    if (freqMinBox) {
        asprintf(&buf, "%s.FrequencyMin.Input", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqMinBox, xOffset, yOffset, 1);
        asprintf(&buf, "%s.FrequencyMin.Description", self->name);
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqMinDesc, xOffset, yOffset - (freqMinDesc->height - freqMinBox->height) / 2, 1);

        yOffset += freqMinBox->height + ySpacing;
    }

    if (freqMaxBox) {
        asprintf(&buf, "%s.FrequencyMax.Input", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqMaxBox, xOffset, yOffset, 1);
        asprintf(&buf, "%s.FrequencyMax.Description", self->name);
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, freqMaxDesc, xOffset, yOffset - (freqMaxDesc->height - freqMaxBox->height) / 2, 1);

        yOffset += freqMaxBox->height + ySpacing;
    }

    if (polarization) {
        asprintf(&buf, "%s.Polarization.ComboBox", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, polarization, xOffset, yOffset, 1);
    }

    if (polarizationDesc) {
        asprintf(&buf, "%s.Polarization.Description", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, polarizationDesc, xOffset, yOffset
                       - (polarization ? (polarizationDesc->height - polarization->height) / 2 : 0), 1);
    }

    if (polarization)
        yOffset += polarization->height + ySpacing;

    if (symbolRate) {
        asprintf(&buf, "%s.SymbolRate", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1.0));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        svWidgetAttach(self->widget, symbolRate, xOffset, yOffset, 1);
        free(buf);
    }

    if (symbolRateDesc) {
        asprintf(&buf, "%s.SymbolRate.Description", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1.0));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        svWidgetAttach(self->widget, symbolRateDesc, xOffset, yOffset - (symbolRateDesc->height - symbolRate->height) / 2, 1);
        free(buf);
    }

    if (symbolRate)
        yOffset += symbolRate->height + ySpacing;

    if (modulation) {
        asprintf(&buf, "%s.Modulation.ComboBox", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, modulation, xOffset, yOffset, 1);
    }

    if (modulationDesc) {
        asprintf(&buf, "%s.Modulation.Description", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, modulationDesc, xOffset, yOffset
                       - (modulation ? (modulationDesc->height - modulation->height) / 2 : 0), 1);
    }

    if (modulation)
        yOffset += modulation->height + ySpacing;

    if (extraDesc) {
        asprintf(&buf, "%s.Extra.Description", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1.0));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, extraDesc, xOffset, yOffset, 1);
    }

    if (extra) {
        asprintf(&buf, "%s.Extra.ComboBox", self->name);
        size_t xOffset;
        if (useRatioSettings)
            xOffset = (int) ((double) self->widget->width * svSettingsGetDouble(buf, "xOffsetRatio", -1.0));
        else
            xOffset = svSettingsGetInteger(buf, "xOffset", -1);
        free(buf);
        svWidgetAttach(self->widget, extra, xOffset, yOffset, 1);
    }

    if (extra)
        yOffset += extra->height + ySpacing;

    if (useNITSDT) {
        asprintf(&buf, "%s.UseNITSDT.ComboBox", self->name);
        if (useRatioSettings)
            svSettingsWidgetAttachWithRatio(self->widget, useNITSDT, buf, 1);
        else
            svSettingsWidgetAttach(self->widget, useNITSDT, buf, 1);
        free(buf);
        asprintf(&buf, "%s.UseNITSDT.Description", self->name);
        if (useRatioSettings)
            svSettingsWidgetAttachWithRatio(self->widget, useNITSDTDesc, buf, 1);
        else
            svSettingsWidgetAttach(self->widget, useNITSDTDesc, buf, 1);
        free(buf);
    }

    if (satelliteBox)
        svWidgetSetFocusable(satelliteBox, true);
    //if (freqJump)
    //    svWidgetSetFocusable(freqJump, true);
    if (modulation)
        svWidgetSetFocusable(modulation, true);
    if (polarization)
        svWidgetSetFocusable(polarization, true);
    if (symbolRate)
        svWidgetSetFocusable(symbolRate, true);
    if (extra)
        svWidgetSetFocusable(extra, true);
    if (useNITSDT)
        svWidgetSetFocusable(useNITSDT, true);

    self->symbolRate = symbolRate;

    int initialValue = userParams->symbolRate < 0 ? (int) DEFAULT_SCAN_SYMBOL_RATE : userParams->symbolRate;
    if (symbolRate) {
        QBNumericInputSetValueAndRange(symbolRate,
                                       initialValue,
                                       self->tunerProps.allowed.symbol_rate_min,
                                       self->tunerProps.allowed.symbol_rate_max);
    }

    self->useNITSDT = useNITSDT;

    self->fiber = SvFiberCreate(self->scheduler, NULL, "QBChannelScanningWidget", QBChannelScanningWidgetStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    SVTESTRELEASE(userParams);
    SVTESTRELEASE(standardRastersParamsArray);
}

SvLocal void
QBChannelScanningWidgetDestroy(void *self_)
{
    QBChannelScanningWidget self = self_;
    if (self->fiber)
        SvFiberDestroy(self->fiber);
    if (self->scanner)
        QBDVBScannerDestroy(self->scanner);
    if (self->savingFiber)
        SvFiberDestroy(self->savingFiber);
    if (self->dropFiber)
        SvFiberDestroy(self->dropFiber);

    SVTESTRELEASE(self->sidemenu.ctx);
    SVTESTRELEASE(self->lockOn);
    SVTESTRELEASE(self->lockOff);

    SVRELEASE(self->params);

    free(self->name);

    SVTESTRELEASE(self->epgManager);
    SVTESTRELEASE(self->dvbEPGPlugin);
    SVTESTRELEASE(self->satellitesDB);
}

SvLocal SvType QBChannelScanningWidget_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ vtable = {
        .destroy = QBChannelScanningWidgetDestroy
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelScanningWidget",
                            sizeof(struct QBChannelScanningWidget_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal void QBChannelScanningWidgetCleanup(SvApplication app, void *prv_)
{
    QBChannelScanningWidget self = prv_;
    if (self->symbolRateBoxInactiveFrame)
        svWidgetDestroy(self->symbolRateBoxInactiveFrame);
    if (self->symbolRateBoxActiveFrame)
        svWidgetDestroy(self->symbolRateBoxActiveFrame);
    SVRELEASE(self);
}

SvLocal void
QBChannelScanningWidgetCreateStartAndStopButtons(QBChannelScanningWidget self, bool useRatioSettings)
{
   char *buf = NULL;
   asprintf(&buf, "%s.Start", self->name);
   SvWidget start = NULL;
   if (useRatioSettings)
       start = svButtonNewFromRatio(self->app, buf, gettext("Start"), 0, svWidgetGetId(self->widget), self->widget->width, self->widget->height);
   else
       start = svButtonNewFromSM(self->app, buf, gettext("Start"), 0, svWidgetGetId(self->widget));
   free(buf);
   SvWidget drop = NULL;

   asprintf(&buf, "%s.Drop", self->name);
   if (useRatioSettings)
       drop = svButtonNewFromRatio(self->app, buf, gettext("Drop"), 0, svWidgetGetId(self->widget), self->widget->width, self->widget->height);
   else
       drop = svButtonNewFromSM(self->app, buf, gettext("Drop"), 0, svWidgetGetId(self->widget));
   free(buf);

   asprintf(&buf, "%s.Start", self->name);
   if (useRatioSettings)
       svSettingsWidgetAttachWithRatio(self->widget, start, buf, 1);
   else
       svSettingsWidgetAttach(self->widget, start, buf, 1);
   free(buf);
   asprintf(&buf, "%s.Drop", self->name);
   if (drop) {
       if (useRatioSettings)
           svSettingsWidgetAttachWithRatio(self->widget, drop, buf, 1);
       else
           svSettingsWidgetAttach(self->widget, drop, buf, 1);
   }
   free(buf);

   self->start = start;
   self->drop = drop;
   self->startId = svWidgetGetId(start);
   svWidgetSetFocus(start);
   if (drop)
       self->dropId = svWidgetGetId(drop);
}

SvLocal SvWidget
QBChannelScanningWidgetCreateFromGivenSettingsType(SvApplication app,
                                                   SvScheduler scheduler,
                                                   const char *name,
                                                   QBChannelScanningParams params,
                                                   bool useRatioSettings,
                                                   int parentWidth,
                                                   int parentHeight,
                                                   SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!app || !scheduler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed: app = %p, scheduler = %p", app, scheduler);
        goto err1;
    }

    if (!name) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL name passed");
        goto err1;
    }

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"SvEPGManager\") failed");
        goto err1;
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    DVBEPGPlugin dvbEPGPlugin = (DVBEPGPlugin)
        QBGlobalStorageGetItem(globalStorage, DVBEPGPlugin_getType(), &error);
    if (!dvbEPGPlugin) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBGlobalStorageGetItem(\"DVBEPGPlugin_getType\") failed");
        goto err1;
    }

    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    QBChannelScanningWidget self = (QBChannelScanningWidget) SvTypeAllocateInstance(QBChannelScanningWidget_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBChannelScanningWidget");
        goto err1;
    }

    self->epgManager = SVRETAIN(epgManager);
    self->dvbEPGPlugin = SVRETAIN(dvbEPGPlugin);
    self->satellitesDB = SVTESTRETAIN(satellitesDB);
    self->settingsCtx = svSettingsSaveContext();
    self->name = strdup(name);
    self->app = app;
    self->params = SVRETAIN(params);
    self->scheduler = scheduler;

    SvWidget widget = NULL;
    if (useRatioSettings)
        widget = svSettingsWidgetCreateWithRatio(app, name, parentWidth, parentHeight);
    else {
        char* buf;
        asprintf(&buf, "%s.Window", name);
        //We are checking if Window wrapper for CubiTV is defined
        if (svSettingsIsWidgetDefined(buf))
            widget = svSettingsWidgetCreate(app, buf);
        else
            widget = svSettingsWidgetCreate(app, name);
        free(buf);
    }

    if (!widget) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate widget");
        goto err2;
    }
    self->widget = widget;
    widget->prv = self;
    svWidgetSetFocusable(widget, false);
    widget->clean = QBChannelScanningWidgetCleanup;
    svWidgetSetUserEventHandler(widget, QBChannelScanningWidgetUserEventHandler);
    svWidgetSetInputEventHandler(widget, QBChannelScanningWidgetInputEventHandler);
    svWidgetSetTimerEventHandler(widget, QBChannelScanningWidgetTimerEventHandler);

    QBChannelScanningWidgetCreateStartAndStopButtons(self, useRatioSettings);
    QBChannelScanningWidgetCreateSetupAll(self, name, self->app, useRatioSettings);
    self->state = CS_NOOP;

    self->priority = self->params->params.forcePriority ? self->params->params.priority : QBTUNERRESERVER_PRIORITY_CHANNEL_SCAN;

    self->timeLeft = -1;

    self->currentStatus = NULL;
    self->lockStatus = NULL;
    self->lockIndicator = NULL;
    self->total = NULL;
    self->signalStrength = NULL;
    self->signalQuality = NULL;

    if ((self->params->params.forceAutostart && self->params->params.autostart) || (self->params->params.forceAutomatic && self->params->params.automatic))
        QBChannelScanningWidgetStartScanning(self);
    return widget;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvWidget QBChannelScanningWidgetCreate(SvApplication app,
                                       SvScheduler scheduler,
                                       const char *name,
                                       QBChannelScanningParams params,
                                       SvErrorInfo *errorOut)
{
    return QBChannelScanningWidgetCreateFromGivenSettingsType(app, scheduler, name, params, false, 0, 0, errorOut);
}

SvWidget QBChannelScanningWidgetCreateFromRatio(SvApplication app,
                                                SvScheduler scheduler,
                                                const char *name,
                                                QBChannelScanningParams params,
                                                int parentWidth,
                                                int parentHeight,
                                                SvErrorInfo *errorOut)
{
    return QBChannelScanningWidgetCreateFromGivenSettingsType(app, scheduler, name, params, true, parentWidth, parentHeight, errorOut);
}

bool QBChannelScanningWidgetHandleInputEvent(SvWidget w, SvInputEvent e)
{
    assert(w);
    return QBChannelScanningWidgetInputEventHandler(w, e);
}

bool QBChannelScanningWidgetSideMenuEnabled(SvWidget w)
{
    assert(w);
    QBChannelScanningWidget self = w->prv;
    assert(self);
    return self->sideMenuSettingsEnabled;
}
