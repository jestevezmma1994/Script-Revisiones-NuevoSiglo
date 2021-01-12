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

#include <Widgets/QBUSBPVRDialog.h>
#include <Services/core/hotplugMounts.h>
#include <QBWidgets/QBDialog.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/gauge.h>
#include <SWL/fade.h>
#include <SWL/anim.h>
#include <main.h>
#include <libintl.h>
#include <SvFoundation/SvString.h>

struct QBUSBPVRDialog_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvWidget helper;
    SvWidget dialog;
    SvWidget progressBar;

    SvHashTable testNumberToTestId;
    SvHashTable results;

    int lastPassedTestId;
    SvString testInfo;
    bool speedTestStarted;
    bool errorEncountered;
    bool onlyOnePartitionFormat;
    QBUSBPerfTester perfTester;
    unsigned int settingsCtx;
};

SvLocal void QBUSBPVRDialogShowNewMessage(QBUSBPVRDialog self, const char *message)
{
    QBDialogRemoveItem(self->dialog, SVSTRING("message"));
    QBDialogAddLabel(self->dialog, SVSTRING("content"), message, SVSTRING("message"), 1);
}

SvLocal void QBUSBPVRDialogUSBTestResultGathered(SvObject self_, QBUSBPerfTestResult result)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    SvValue testNumberVal = SvValueCreateWithInteger(result.testNumber, NULL);
    SvValue passedVal = SvValueCreateWithBoolean(result.passed, NULL);
    SvHashTableInsert(self->results, (SvObject) testNumberVal, (SvObject) passedVal);
    SVRELEASE(testNumberVal);
    SVRELEASE(passedVal);

    svGaugeSetValue(self->progressBar, result.testNumber + 1);
}

SvLocal void QBUSBPVRDialogUSBNextTestStarted(SvObject self_, QBUSBPerfTest test)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    if (!self->speedTestStarted) {
        QBDialogAddWidget(self->dialog, SVSTRING("content"), self->progressBar, SVSTRING("total"), 3, "Dialog.Progress");
    }
    self->speedTestStarted = true;
    SvString runMsg = QBUSBPerfTestGetString(test, SVSTRING("running"));
    char *msg = gettext(runMsg ? SvStringGetCString(runMsg) : "unknown test");
    QBUSBPVRDialogShowNewMessage(self, msg);

}

SvLocal void QBUSBPVRDialogUSBAllTestsFinished(SvObject self_, SvArray tests, int lastPassedTestId)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    self->lastPassedTestId = lastPassedTestId;
    QBUSBPerfTesterRemoveListener(self->perfTester, self_);
    QBHotplugMountAgentRemoveListener(self->appGlobals->hotplugMountAgent, (SvObject)self);
    svGaugeSetValue(self->progressBar, QBUSBPerfTesterGetTestsCount(self->perfTester));
    QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
    int operatorLimitId = QBPVRGetOperatorRecordLimit(self->appGlobals->pvrLogic);

    char *msg;
    if (lastPassedTestId >= 0) {
        int testIndex = 0;
        for (testIndex = 0; testIndex < lastPassedTestId; testIndex++) {
            if ((operatorLimitId >= 0) &&
                (operatorLimitId < QBPVRLogicGetRecLimitFromUSBPerfTest(self->appGlobals->pvrLogic, (QBUSBPerfTest) SvArrayGetObjectAtIndex(tests, testIndex + 1)))) {
                break;
            }
        }

        SvString endMsg = testIndex < 0 ? NULL : QBUSBPerfTestGetString((QBUSBPerfTest)SvArrayGetObjectAtIndex(tests, testIndex), SVSTRING("capable"));
        msg = gettext(endMsg ? SvStringGetCString(endMsg) : "unknown test");

    } else {
        msg = gettext("USB Speed Test failed. Try to reinsert you USB drive and run the test again.");
    }

    self->testInfo = SvStringCreate(msg, NULL);

    SvEffect effect = svEffectFadeNew(self->progressBar, ALPHA_TRANSPARENT, 1, 1.5, SV_EFFECT_FADE_LOGARITHM);
    svAppRegisterEffect(self->appGlobals->res, effect);
    if (self->errorEncountered) {
        QBDialogRemoveItem(self->dialog, SVSTRING("warning"));
        QBUSBPVRDialogShowNewMessage(self, gettext("USB Speed Test failed. Try to reinsert you USB drive and run the test again."));
        SvWidget okButton = QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("OK"), 1);
        svWidgetSetFocus(okButton);
    }
}

SvLocal void QBUSBPVRDialog__dtor__(void *self_)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    SVTESTRELEASE(self->perfTester);
    SVTESTRELEASE(self->results);
    SVTESTRELEASE(self->testInfo);
}

SvLocal void QBUSBPVRDialogShowFormatting(QBUSBPVRDialog self, QBPVRMounterState state);

SvLocal void QBUSBPVRDialogMounterResult(SvGenericObject self_, QBDiskStatus disk)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    if (disk && disk->disk && disk->disk->devId) {
        QBUSBPVRDialogShowFormatting(self, disk->state);
    }
}

SvLocal void QBUSBPVRDialogPartitionAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk, QBPartitionInfo part)
{
}

SvLocal void QBUSBPVRDialogDiskAdded(SvGenericObject self_, SvGenericObject path, QBDiskInfo disk)
{
}

SvLocal void QBUSBPVRDialogPartitionRemoved(SvGenericObject self_, SvGenericObject path, SvString remId, SvString diskId)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) self_;
    self->errorEncountered = true;
}


SvLocal SvType QBUSBPVRDialog_getType(void)
{
    static SvType type = NULL;

    static const struct QBUSBPerfTesterListener_t usbTesterListener = {
        .testFinished     = QBUSBPVRDialogUSBTestResultGathered,
        .testStarted      = QBUSBPVRDialogUSBNextTestStarted,
        .allTestsFinished = QBUSBPVRDialogUSBAllTestsFinished,
    };

    static const struct QBPVRMounterListener_t pvrMounter = {
        .mounterResultReceived = QBUSBPVRDialogMounterResult
    };

    static const struct QBHotplugMountAgentListener_t hotplugMethods = {
        .partitionAdded = QBUSBPVRDialogPartitionAdded,
        .partitionRemoved = QBUSBPVRDialogPartitionRemoved,
        .diskAdded = QBUSBPVRDialogDiskAdded,
        .diskRemoved = QBUSBPVRDialogPartitionRemoved
    };

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBUSBPVRDialog__dtor__
    };

    if (!type) {
        SvTypeCreateManaged("QBUSBPVRDialog",
                            sizeof(struct QBUSBPVRDialog_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBUSBPerfTesterListener_getInterface(), &usbTesterListener,
                            QBHotplugMountAgentListener_getInterface(), &hotplugMethods,
                            QBPVRMounterListener_getInterface(), &pvrMounter,
                            NULL);
    }
    return type;
}

SvLocal SvWidget QBUSBPVRDialogCreateAnimationWidget(QBUSBPVRDialog self)
{
    SvWidget animation = svSettingsWidgetCreate(self->appGlobals->res, "Dialog.Animation");
    float duration = svSettingsGetDouble("Dialog.Animation", "stepDuration", 0.2f);
    int bmpCnt = svSettingsGetInteger("Dialog.Animation", "steps", 0);
    if (bmpCnt > 0) {
        SvBitmap* bitmaps = calloc(bmpCnt, sizeof(SvBitmap));
        int i;
        for (i = 0; i < bmpCnt; i++) {
            char *frameName;
            asprintf(&frameName, "frame%i", i);
            bitmaps[i] = SVRETAIN(svSettingsGetBitmap("Dialog.Animation", frameName));
            free(frameName);
        }
        SvEffect effect = svEffectAnimNew(animation, bitmaps, bmpCnt, SV_EFFECT_ANIM_FORWARD, SV_ANIM_LOOP_INFINITE, duration);
        for (i = 0; i < bmpCnt; i++)
            SVRELEASE(bitmaps[i]);
        free(bitmaps);
        svAppRegisterEffect(self->appGlobals->res, effect);
    }

    return animation;
}

SvLocal void QBUSBPVRDialogClean(SvApplication app, void *ptr)
{
    QBUSBPVRDialog self = ptr;
    QBPVRMounterRemoveListener(self->appGlobals->PVRMounter, (SvObject)self);
    SVRELEASE(self);
}

SvLocal bool QBUSBPVRDialogNotificationCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBUSBPVRDialog self = ptr;
    if (buttonTag && SvStringEqualToCString(buttonTag, "button")) {
        QBDialogBreak(self->dialog);
        return true;
    }
    return false;
}
SvLocal void QBUSBPVRDialogShowFormatting(QBUSBPVRDialog self, QBPVRMounterState state)
{
    switch(state)
    {
        case QBPVRMounterState_disk_formatting:
            svSettingsRestoreContext(self->settingsCtx);
            SvWidget animation = QBUSBPVRDialogCreateAnimationWidget(self);
            QBDialogRemoveItem(self->dialog, SVSTRING("message"));
            QBDialogRemoveItem(self->dialog, SVSTRING("button"));
            QBDialogAddLabel(self->dialog, SVSTRING("content"), gettext("Formatting..."), SVSTRING("message"), 0);
            QBDialogAddWidget(self->dialog, SVSTRING("content"), animation, SVSTRING("animation"), 1, "Dialog.Animation");
            svSettingsPopComponent();
            break;

        case QBPVRMounterState_disk_mounted:
        case QBPVRMounterState_disk_compatible:
        case QBPVRMounterState_disk_incompatible:
            svSettingsRestoreContext(self->settingsCtx);
            if (self->lastPassedTestId >= 0) {
                QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
                QBDialogRemoveItem(self->dialog, SVSTRING("warning"));
                QBUSBPVRDialogShowNewMessage(self, self->testInfo ? SvStringCString(self->testInfo) : "USB speed test finished!");
                SvWidget okButton = QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("OK"), 1);
                svWidgetSetFocus(okButton);
            } else if (!QBPVRLogicIsSpeedTestEnabled(self->appGlobals->pvrLogic) || self->onlyOnePartitionFormat) {
                QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
                QBDialogRemoveItem(self->dialog, SVSTRING("warning"));
                QBUSBPVRDialogShowNewMessage(self, gettext("Formatting finished!"));
                SvWidget okButton = QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("OK"), 1);
                svWidgetSetFocus(okButton);
            }
            svSettingsPopComponent();
            break;

        case QBPVRMounterState_error_formatting:
        case QBPVRMounterState_error_mounting:
        case QBPVRMounterState_error_unmounting:
            QBDialogRemoveItem(self->dialog, SVSTRING("animation"));
            QBDialogRemoveItem(self->dialog, SVSTRING("warning"));
            QBUSBPVRDialogShowNewMessage(self, gettext("Error while formatting disk!"));
            SvWidget okButton = QBDialogAddButton(self->dialog, SVSTRING("button"), gettext("OK"), 1);
            svWidgetSetFocus(okButton);
            break;
        default:
            break;
    }
}

void QBUSBPVRDialogSetOnlyOneParitionFormat(QBUSBPVRDialog self)
{
    self->onlyOnePartitionFormat = true;
}

QBUSBPVRDialog
QBUSBPVRDialogCreate(AppGlobals appGlobals)
{
    QBUSBPVRDialog self = (QBUSBPVRDialog) SvTypeAllocateInstance(QBUSBPVRDialog_getType(), NULL);

    self->appGlobals = appGlobals;
    self->speedTestStarted = false;
    self->errorEncountered = false;
    self->onlyOnePartitionFormat = false;
    self->lastPassedTestId = -1;
    SvWidget helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    helper->prv = self;
    helper->clean = QBUSBPVRDialogClean;

    self->helper = helper;

    svSettingsPushComponent("QBUSBPVRDialog.settings");

    self->settingsCtx = svSettingsSaveContext();

    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(self->helper)
    };
    self->dialog = QBDialogGlobalNew(&params, SVSTRING("QBUSBSpeedTest"));
    svWidgetAttach(self->dialog, helper, 0, 0, 0);

    QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);
    if (QBPVRLogicIsSpeedTestEnabled(self->appGlobals->pvrLogic)) {
        QBDialogSetTitle(self->dialog, gettext(svSettingsGetString("Dialog", "title")));
        QBDialogAddLabel(self->dialog, SVSTRING("content"), gettext("Do not eject your USB drive before the speed test is finished!"), SVSTRING("warning"), 0);
    } else {
        QBDialogSetTitle(self->dialog, gettext(svSettingsGetString("Dialog", "formatTitle")));
        QBDialogAddLabel(self->dialog, SVSTRING("content"), gettext("Do not eject your USB drive before the formatting is finished!"), SVSTRING("warning"), 0);
    }

    SvWidget animation = QBUSBPVRDialogCreateAnimationWidget(self);
    QBDialogAddWidget(self->dialog, SVSTRING("content"), animation, SVSTRING("animation"), 1, "Dialog.Animation");

    svSettingsPopComponent();
    QBPVRMounterAddListener(self->appGlobals->PVRMounter, (SvObject)self);

    self->results = SvHashTableCreate(11, NULL);
    QBDialogSetNotificationCallback(self->dialog, self, QBUSBPVRDialogNotificationCallback);
    return self;
}

SvWidget
QBUSBPVRDialogGetDialog(QBUSBPVRDialog self)
{
    return self->dialog;
}

void QBUSBPVRDialogRegisterSpeedTest(QBUSBPVRDialog self, QBUSBPerfTester perfTester)
{
    svSettingsRestoreContext(self->settingsCtx);
    self->progressBar = svGaugeNewFromSM(self->appGlobals->res, "Dialog.Progress",
                                         SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, QBUSBPerfTesterGetTestsCount(perfTester), 0);
    svSettingsPopComponent();
    self->perfTester = SVRETAIN(perfTester);
    QBUSBPerfTesterAddListener(perfTester, (SvObject) self);
    QBHotplugMountAgentAddListener(self->appGlobals->hotplugMountAgent, (SvGenericObject) self);
}
