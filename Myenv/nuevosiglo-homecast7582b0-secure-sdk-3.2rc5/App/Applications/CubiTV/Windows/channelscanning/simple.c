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

#include "simple.h"
#include "defaultScanParams.h"

#include <libintl.h>
#include <QBConf.h>
#include <QBWindowContext.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <SWL/events.h>
#include <unistd.h>
#include <sv_tuner.h>
#include <QBDVBScanner.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <QBWidgets/QBDialog.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <Windows/channelscanning.h>
#include <main.h>
#include <Logic/GUILogic.h>


struct QBChannelScanningSimpleContext_t {
    struct QBChannelScanningContext_t super_;
};
typedef struct QBChannelScanningSimpleContext_t * QBChannelScanningSimpleContext;

struct QBChannelScanningSimpleInfo_t {
    int priority;

    SvWidget window;
    SvWidget logo;

    SvWidget instruction;
    SvWidget installButton;

    SvWidget noTuner;

    QBDVBScanner* scanner;

    struct {
        SvTimerId poll;
        SvTimerId saving;
        SvWidget dialog;
        bool hasLock;
        SvWidget lockStatus;
        SvWidget button;
    } progress;

    enum {
        CS_BEGIN,
        CS_STARTED,
        CS_FAILED,
        CS_SUCCEDED,
        CS_SAVING,
        CS_SAVED
    } state;

    bool successfulScan;

    QBChannelScanningSimpleContext ctx;
    int videoCnt, radioCnt;
};
typedef struct QBChannelScanningSimpleInfo_t * QBChannelScanningSimpleInfo;

SvLocal void QBChannelScanningSimpleShowResults(QBChannelScanningSimpleInfo cti, int videoCnt, int radioCnt);
SvLocal void QBChannelScanningSimpleNoTuner(QBChannelScanningSimpleInfo cti);
SvLocal bool QBChannelScanningSimpleProgressDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode);

static const float pollTime = 0.2;

SvLocal void QBChannelScanningSimpleStopTimers(QBChannelScanningSimpleInfo cti)
{
    SvApplication app = cti->window->app;
    svAppTimerStop(app, cti->progress.poll);
    cti->progress.poll = 0;
    svAppTimerStop(app, cti->progress.saving);
    cti->progress.saving = 0;
}

SvLocal void QBChannelScanningSimpleSaveFinish(QBChannelScanningSimpleInfo cti)
{
    AppGlobals appGlobals = cti->ctx->super_.appGlobals;
    QBChannelScanningSimpleStopTimers(cti);

    QBChannelScanningLogicScanningFinished(cti->ctx->super_.appGlobals->channelScanningLogic, NULL, cti->successfulScan);

    QBApplicationControllerSwitchToRoot(appGlobals->controller);
}

SvLocal void QBChannelScanningSimpleSetupSaveChannels(QBChannelScanningSimpleInfo cti)
{
    svButtonSetCaption(cti->progress.button, gettext("Saving..."));
}

SvLocal void QBChannelScanningSimpleSaveChannels(QBChannelScanningSimpleInfo cti)
{
    QBChannelScanningLogicProcessFoundChannels(cti->ctx->super_.appGlobals->channelScanningLogic, NULL, cti->scanner);
    QBDVBScannerDestroy(cti->scanner);
    cti->scanner = NULL;
    cti->state = CS_SAVING;

    cti->progress.saving = svAppTimerStart(cti->window->app, cti->window, pollTime, 0);

    QBChannelScanningSimpleSetupSaveChannels(cti);
}

SvLocal void QBChannelScanningSimpleRestart(QBChannelScanningSimpleInfo cti)
{
    svWidgetSetHidden(cti->instruction, false);
    svWidgetSetHidden(cti->installButton, false);
    svWidgetSetFocus(cti->installButton);
    cti->state = CS_BEGIN;
    if(cti->scanner)
        QBDVBScannerDestroy(cti->scanner);
    cti->scanner = NULL;
    QBChannelScanningSimpleStopTimers(cti);
    if (cti->noTuner) {
        QBDialogBreak(cti->noTuner);
        cti->noTuner = NULL;
    }
    if (cti->progress.dialog) {
        QBDialogBreak(cti->progress.dialog);
        cti->progress.dialog = NULL;
    }
}

SvLocal void QBChannelScanningSimpleQBDVBScannerOnFinish(void *cti_, QBDVBScanner* scanner)
{
    QBChannelScanningSimpleInfo cti = cti_;

    cti->videoCnt = QBDVBScannerGetNumberOfTVChannels(scanner);
    cti->radioCnt = QBDVBScannerGetNumberOfRadioChannels(scanner);
    SvLogNotice("DVB Scan finished: Found tv %i radio %i", cti->videoCnt, cti->radioCnt);
    if(!cti->videoCnt && !cti->radioCnt)
        cti->state = CS_FAILED;
    else
        cti->state = CS_SUCCEDED;

    QBDVBScannerStop(scanner);
    QBChannelScanningSimpleStopTimers(cti);
    QBChannelScanningSimpleShowResults(cti, cti->videoCnt, cti->radioCnt);
}

SvLocal void QBChannelScanningSimpleQBDVBScannerOnRevoke(void *cti_, QBDVBScanner* scanner)
{
    QBChannelScanningSimpleInfo cti = cti_;
    QBChannelScanningSimpleNoTuner(cti);
}

SvLocal void QBChannelScanningSimpleStartSetup(QBChannelScanningSimpleInfo cti)
{
    svSettingsPushComponent("ChannelScanningSimple.settings");
    AppGlobals appGlobals = cti->ctx->super_.appGlobals;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(cti->window),
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString("Popup", "title")));
    cti->progress.lockStatus = QBDialogAddLabel(dialog, SVSTRING("content"), gettext("Signal lost - check cable"), SVSTRING("lock status"), 2);
    cti->progress.button = QBDialogAddButton(dialog, SVSTRING("button"), gettext("Cancel"), 1);

    SvWidget animation = QBWaitAnimationCreate(appGlobals->res, "Popup.Animation");

    QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");
    QBDialogSetNotificationCallback(dialog, cti, QBChannelScanningSimpleProgressDialogNotificationCallback);

    cti->progress.dialog = dialog;

    svSettingsPopComponent();

    svWidgetSetHidden(cti->instruction, true);
    svWidgetSetHidden(cti->installButton, true);
    cti->progress.hasLock = false;

    QBDialogRun(cti->progress.dialog, NULL, NULL);
}

SvLocal void QBChannelScanningSimpleStart(QBChannelScanningSimpleInfo cti)
{
    if (!cti->ctx->super_.appGlobals->epgPlugin.DVB.channelList) {
        SvLogError("%s(): DVBEPGPlugin not available", __func__);
        return;
    }

    QBTunerResv* resv = QBTunerResvCreate(SVSTRING("Channel scanning simple check"), cti, NULL);
    resv->params.tunerOwnership = true;
    resv->params.priority = QBTUNERRESERVER_PRIORITY_CHANNEL_SCAN;

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    bool canObtain = QBTunerReserverCanObtain(tunerReserver, resv, NULL);
    if (!canObtain) {
        QBChannelScanningSimpleNoTuner(cti);
    } else {
        cti->progress.poll = svAppTimerStart(cti->window->app, cti->window, pollTime, 0);

        cti->state = CS_STARTED;

        static QBDVBScannerParams params = {
            .chosenONID = -1,
        };

        QBChannelScanningContext super = (QBChannelScanningContext) cti->ctx;

        params.onlyActual = (super->conf && super->conf->forceOnlyActual) ? super->conf->onlyActual : false;
        params.dropNotFoundInNIT = (super->conf && super->conf->forceDropNotFoundInNIT) ? super->conf->dropNotFoundInNIT : false;
        params.chasing.chaseChannels = (super->conf && super->conf->forceChaseChannels) ? super->conf->chaseChannels : QBDVBChaseChannelsType_recursiveNIT;

        params.freq = malloc(sizeof(int));
        params.freq[0] = 0;
        const char *scanFreq = QBConfigGet("SCAN_FREQ") ?: QBConfigGet("DVB_FREQ");
        if (scanFreq)
            params.freq[0] = atoi(scanFreq);
        params.freqCnt = 1;
        params.modulations = malloc(sizeof(QBTunerModulation));
        params.modulations[0] = QBTunerModulationFromQAM(DEFAULT_SCAN_QAM);
        params.modulationCnt = 1;
        params.symbolRates = malloc(sizeof(int));
        params.symbolRates[0] = DEFAULT_SCAN_SYMBOL_RATE;
        params.symbolRateCnt = 1;
        params.target = cti;
        params.onFinish = QBChannelScanningSimpleQBDVBScannerOnFinish;
        params.onRevoke = QBChannelScanningSimpleQBDVBScannerOnRevoke;
        params.onStep = NULL;
        params.satellitesDB = cti->ctx->super_.satellitesDB;
        params.reserver = tunerReserver;
        params.priority = QBTUNERRESERVER_PRIORITY_CHANNEL_SCAN;
        params.tunerNum = -1;

        cti->scanner = QBDVBScannerCreate(cti->ctx->super_.appGlobals->scheduler, super->conf ? super->conf->DVBScannerPlugins : NULL);
        QBChannelScanningLogicSetDVBScannerPlugins(cti->ctx->super_.appGlobals->channelScanningLogic, cti->scanner);
        QBDVBScannerStart(cti->scanner, &params);

        QBChannelScanningSimpleStartSetup(cti);
    }
    SVRELEASE(resv);
}

SvLocal SvString get_string(SvHashTable hashTable, SvString key)
{
    SvGenericObject val = SvHashTableFind(hashTable, (SvGenericObject)key);
    if (val && SvObjectIsInstanceOf(val, SvString_getType())) {
        return (SvString)val;
    }
    return NULL;
}

SvLocal void QBChannelScanningSimpleShowResults(QBChannelScanningSimpleInfo cti, int videoCnt, int radioCnt)
{
    QBDialogRemoveItem(cti->progress.dialog, SVSTRING("lock status"));
    QBDialogRemoveItem(cti->progress.dialog, SVSTRING("animation"));
    svButtonSetCaption(cti->progress.button, "OK");

    bool successfulScan = QBChannelScanningLogicIsScanSuccessful(cti->ctx->super_.appGlobals->channelScanningLogic, NULL, cti->scanner);
    cti->successfulScan = successfulScan;

    const char* resultTitle = successfulScan ? gettext("Results - Success") : gettext("Results - Failure");
    QBDialogSetTitle(cti->progress.dialog, resultTitle);

    SvApplication app = cti->progress.dialog->app;

    svSettingsPushComponent("ChannelScanningSimple.settings");

    SvWidget results = svSettingsWidgetCreate(app, "Results");
    SvWidget videoTick = NULL;
    if(successfulScan && videoCnt)
        videoTick = svSettingsWidgetCreate(app, "Results.Success");
    else
        videoTick = svSettingsWidgetCreate(app, "Results.Failure");
    svSettingsWidgetAttach(results, videoTick, "Results.Video.Tick", 1);

    SvWidget videoResult = svLabelNewFromSM(app, "Results.Video.Description");
    char *videoStr;
    asprintf(&videoStr, ngettext("%i tv channel", "%i tv channels", videoCnt), videoCnt);
    svLabelSetText(videoResult, videoStr);
    free(videoStr);
    svSettingsWidgetAttach(results, videoResult, "Results.Video.Description", 1);

    if (QBChannelScanningLogicIsRadioEnabled(cti->ctx->super_.appGlobals->channelScanningLogic)) {
        SvWidget radioTick = NULL;
        if(successfulScan && radioCnt)
            radioTick = svSettingsWidgetCreate(app, "Results.Success");
        else
            radioTick = svSettingsWidgetCreate(app, "Results.Failure");
        svSettingsWidgetAttach(results, radioTick, "Results.Radio.Tick", 1);

        SvWidget radioResult = svLabelNewFromSM(app, "Results.Radio.Description");
        char *radioStr;
        asprintf(&radioStr, ngettext("%i radio channel", "%i radio channels", radioCnt), radioCnt);
        svLabelSetText(radioResult, radioStr);
        free(radioStr);
        svSettingsWidgetAttach(results, radioResult, "Results.Radio.Description", 1);
    }

    SvHashTable extraResults = QBChannelScanningLogicGetResults(cti->ctx->super_.appGlobals->channelScanningLogic);
    if (extraResults) {
        SvGenericObject key = NULL;
        SvIterator it = SvHashTableKeysIterator(extraResults);
        while ((key = SvIteratorGetNext(&it))) {
            SvHashTable stepResults = (SvHashTable) SvHashTableFind(extraResults, key);
            if (!SvObjectIsInstanceOf((SvObject) stepResults, SvHashTable_getType())) {
                continue;
            }

            SvValue statusV  = (SvValue) SvHashTableFind(stepResults, (SvGenericObject)SVSTRING("status"));
            bool status = false;
            if (statusV && SvObjectIsInstanceOf((SvObject) statusV, SvValue_getType()) && SvValueIsBoolean(statusV)) {
                status = SvValueGetBoolean(statusV);
            }

            SvString tickWidgetName = get_string(stepResults, SVSTRING("tickWidgetName"));
            SvString descWidgetName = get_string(stepResults, SVSTRING("descWidgetName"));
            SvString resultDescription = get_string(stepResults, SVSTRING("resultDescription"));
            if (tickWidgetName && descWidgetName && resultDescription) {
                SvWidget wdg = NULL;
                if(status)
                    wdg = svSettingsWidgetCreate(app, "Results.Success");
                else
                    wdg = svSettingsWidgetCreate(app, "Results.Failure");
                svSettingsWidgetAttach(results, wdg, SvStringCString(tickWidgetName), 1);

                SvWidget resultWdg = svLabelNewFromSM(app, SvStringCString(descWidgetName));
                svLabelSetText(resultWdg, SvStringCString(resultDescription));
                svSettingsWidgetAttach(results, resultWdg, SvStringCString(descWidgetName), 1);
            }
        }
    }

    QBDialogAddWidget(cti->progress.dialog, SVSTRING("content"), results, SVSTRING("results"), 1, "Results");

    svSettingsPopComponent();
}

SvLocal bool QBChannelScanningSimpleInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBChannelScanningSimpleInfo cti = w->prv;
    QBChannelScanningContext super = (QBChannelScanningContext) cti->ctx;

    if (e->ch == QBKEY_BACK || e->ch == QBKEY_MENU) {
        if (cti->state == CS_STARTED || super->force) {
            return true;
        }
    }
    return svWidgetInputEventHandler(w, e);
}

SvLocal bool QBChannelScanningSimpleProgressDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningSimpleInfo cti = ptr;

    if(cti->progress.dialog) {
       if(!buttonTag)
           return false;

        if(cti->state == CS_STARTED) {
            QBChannelScanningSimpleRestart(cti);
        } else if(cti->state == CS_SUCCEDED) {
            QBChannelScanningSimpleSaveChannels(cti);
        } else if(cti->state == CS_FAILED) {
            QBChannelScanningSimpleRestart(cti);
        }

        SvWidget progressDialog = cti->progress.dialog;
        cti->progress.dialog = NULL;

        if (progressDialog) {
            QBDialogBreak(progressDialog);
        }
        return true;
   }
    return false;
}

SvLocal bool QBChannelScanningSimpleNoTunerDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBChannelScanningSimpleInfo cti = ptr;

    if (cti->noTuner) {
        if (!buttonTag)
            return false;

        QBDialogBreak(cti->noTuner);
        cti->noTuner = NULL;
        return true;
    }
    return false;
}

SvLocal void QBChannelScanningSimpleUserEventHandler(SvWidget w, SvWidgetId src, SvUserEvent e)
{
    QBChannelScanningSimpleInfo cti = w->prv;
    if(e->code == SV_EVENT_BUTTON_PUSHED) {
        if(src == svWidgetGetId(cti->installButton)) {
            QBChannelScanningSimpleStart(cti);
        }
        return;
    }
}

SvLocal void QBChannelScanningSimpleTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBChannelScanningSimpleInfo cti = w->prv;
    if(cti->progress.poll == e->id) {
        if(cti->progress.dialog) {
            sv_tuner_t* tuner = sv_tuner_get(0);
            struct QBTunerStatus status;
            sv_tuner_get_status(tuner, &status);
            if(status.full_lock != cti->progress.hasLock) {
                cti->progress.hasLock = status.full_lock;
                svLabelSetText(cti->progress.lockStatus, status.full_lock ? gettext("Signal - OK") : gettext("Signal lost - check cable"));
            }
        }
        return;
    } else if(cti->progress.saving == e->id) {
        if(!SvInvokeInterface(SvEPGChannelListPlugin, cti->ctx->super_.appGlobals->epgPlugin.DVB.channelList, isPropagatingChannels) &&
           !SvEPGChannelListIsPropagatingChannels(SvEPGManagerGetChannelsList(cti->ctx->super_.epgManager))) {
            QBChannelScanningSimpleSaveFinish(cti);
            return;
        }
    }
}

SvLocal void QBChannelScanningSimpleNoTuner(QBChannelScanningSimpleInfo cti)
{
    QBChannelScanningSimpleRestart(cti);

    svSettingsPushComponent("ChannelScanningSimple.settings");
    AppGlobals appGlobals = cti->ctx->super_.appGlobals;
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(cti->window),
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString("Popup.NoTuner", "title")));
    QBDialogAddLabel(dialog, SVSTRING("content"), gettext(svSettingsGetString("Popup.NoTuner", "info")), SVSTRING("info"), 1);
    QBDialogAddButton(dialog, SVSTRING("Close"), gettext("Close"), 1);
    cti->noTuner = dialog;
    QBDialogSetNotificationCallback(dialog, cti, QBChannelScanningSimpleNoTunerDialogNotificationCallback);
    QBDialogRun(dialog, NULL, NULL);

    svSettingsPopComponent();
}

SvLocal void QBChannelScanningSimpleClean(SvApplication app, void *ptr)
{
    QBChannelScanningSimpleInfo cti = ptr;
    QBChannelScanningSimpleStopTimers(cti);
    if (cti->scanner)
        QBDVBScannerDestroy(cti->scanner);
    free(cti);
}

SvLocal void QBChannelScanningSimpleReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBChannelScanningSimpleContext self = (QBChannelScanningSimpleContext) self_;
    QBChannelScanningSimpleInfo info = (QBChannelScanningSimpleInfo) self->super_.super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("CustomerLogo"))) {
        SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->super_.appGlobals->initLogic);
        if (logoWidgetName) {
            info->logo = QBCustomerLogoReplace(info->logo, self->super_.super_.window, logoWidgetName, 1);
        }
    }
}

SvLocal void QBChannelScanningSimpleCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBChannelScanningSimpleContext self = (QBChannelScanningSimpleContext) self_;

    svSettingsPushComponent("ChannelScanningSimple.settings");

    SvWidget window = QBGUILogicCreateBackgroundWidget(self->super_.appGlobals->guiLogic, "ChannelScanningWindow", NULL);
    svWidgetSetName(window, "ChannelScanningSimple");
    QBChannelScanningSimpleInfo cti = calloc(1, sizeof(struct QBChannelScanningSimpleInfo_t));
    cti->ctx = self;

    SvWidget windowTitle = svLabelNewFromSM(app, "TitleLabel");
    SvWidget instruction = svLabelNewFromSM(app, "Instruction");
    SvWidget installButton = svButtonNewFromSM(app, "InstallButton", gettext(svSettingsGetString("InstallButton", "caption")), 0, svWidgetGetId(window));

    svSettingsWidgetAttach(window, windowTitle, "TitleLabel", 1);
    svSettingsWidgetAttach(window, instruction, "Instruction", 1);
    svSettingsWidgetAttach(window, installButton, "InstallButton", 1);

    cti->window = window;
    cti->instruction = instruction;
    cti->installButton = installButton;

    window->prv = cti;
    svWidgetSetInputEventHandler(window, QBChannelScanningSimpleInputEventHandler);
    svWidgetSetUserEventHandler(window, QBChannelScanningSimpleUserEventHandler);
    svWidgetSetTimerEventHandler(window, QBChannelScanningSimpleTimerEventHandler);
    window->clean = QBChannelScanningSimpleClean;

    svSettingsPopComponent();

    self->super_.super_.window = window;
    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->super_.appGlobals->initLogic);
    if (logoWidgetName)
        cti->logo = QBCustomerLogoAttach(self->super_.super_.window, logoWidgetName, 1);

    svWidgetSetFocus(installButton);

    if (self->super_.force || self->super_.forceAutostart)
        QBChannelScanningSimpleStart(cti);
}

SvLocal void QBChannelScanningSimpleDestroyWindow(QBWindowContext self)
{
    QBChannelScanningSimpleRestart(self->window->prv);
    svWidgetDestroy(self->window);
    self->window = NULL;
}

SvLocal SvType QBChannelScanningSimpleContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .reinitializeWindow = QBChannelScanningSimpleReinitializeWindow,
        .createWindow       = QBChannelScanningSimpleCreateWindow,
        .destroyWindow      = QBChannelScanningSimpleDestroyWindow
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelScanningSimpleContext",
                            sizeof(struct QBChannelScanningSimpleContext_t),
                            QBChannelScanningContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}


QBWindowContext QBChannelScanningSimpleContextCreate(AppGlobals appGlobals)
{
    QBChannelScanningSimpleContext ctx = (QBChannelScanningSimpleContext) SvTypeAllocateInstance(QBChannelScanningSimpleContext_getType(), NULL);

    QBChannelScanningContextInit((QBChannelScanningContext) ctx, appGlobals);

    return (QBWindowContext) ctx;
}
