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

#include "audioDelayDialog.h"

#include <libintl.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformAudioOutput.h>
#include <settings.h>
#include <QBConf.h>
#include <QBWidgets/QBDialog.h>
#include <main.h>
#include <QBWidgets/QBComboBox.h>


typedef struct QBAudioDelayDialog_ {
    unsigned int audioOutputID;
    unsigned int savedLatency;
    SvWidget dialog;
    SvWidget comboBox;
} *QBAudioDelayDialog;


SvLocal int
QBAudioDelayDialogLoadAudioDelay(unsigned int audioOutputID)
{
    QBAudioOutputConfig cfg;

    if (QBPlatformGetAudioOutputConfig(audioOutputID, &cfg) < 0)
        return -1;

    if (cfg.type == QBAudioOutputType_SPDIF) {
        int latency = 0;
        const char *val = QBConfigGet("SPDIFAUDIODELAY");
        if (val) {
            latency = atoi(val);
            if (latency < 0 || latency > 250 || latency % 10 != 0)
                latency = 0;
        }
        return latency;
    }

    return -1;
}

SvLocal void
QBAudioDelayDialogSaveAudioDelay(unsigned int audioOutputID, unsigned int latency)
{
    QBAudioOutputConfig cfg;

    if (QBPlatformGetAudioOutputConfig(audioOutputID, &cfg) < 0)
        return;

    if (cfg.type == QBAudioOutputType_SPDIF) {
        char buffer[5];
        snprintf(buffer, sizeof(buffer), "%u", latency);
        QBConfigSet("SPDIFAUDIODELAY", buffer);
    } else {
        SvLogWarning("%s(): can't save delay for %s audio output", __func__, cfg.name);
    }
}

SvLocal bool
QBAudioDelayDialogInputCallback(void *self_, SvWidget comboBox, SvInputEvent e)
{
    return false;
}

SvLocal SvString
QBAudioDelayDialogPrepareDataCallback(void *self_, SvWidget comboBox, SvGenericObject value)
{
    return SVRETAIN(value);
}

SvLocal int
QBAudioDelayDialogChangeCallback(void *self_, SvWidget comboBox, SvGenericObject value, int key)
{
    return 0;
}

SvLocal void
QBAudioDelayDialogCurrentCallback(void *self_, SvWidget comboBox, SvGenericObject value)
{
    QBAudioDelayDialog self = self_;
    int latency = atoi(SvStringCString((SvString) value));
    QBPlatformSetAudioOutputLatency(self->audioOutputID, latency);
}

SvLocal void
QBAudioDelayDialogFinishCallback(void *self_, SvWidget comboBox, SvGenericObject selectedValue, SvGenericObject userValue)
{
}

SvLocal bool
QBAudioDelayDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBAudioDelayDialog self = ptr;

    if (self->dialog) {
        if (!buttonTag) {
            return true;
        } else if (SvStringEqualToCString(buttonTag, "OK-button")) {
            SvString valueStr = (SvString) QBComboBoxGetValue(self->comboBox);
            QBAudioDelayDialogSaveAudioDelay(self->audioOutputID, atoi(SvStringCString(valueStr)));
        } else if (SvStringEqualToCString(buttonTag, "cancel-button")) {
            QBAudioDelayDialogSaveAudioDelay(self->audioOutputID, self->savedLatency);
            QBPlatformSetAudioOutputLatency(self->audioOutputID, self->savedLatency);
        }
        self->dialog = NULL;
        QBDialogBreak(dialog);
        return true;
    }

    return false;
}

#define MAX_DELAY  250

SvWidget QBAudioDelayDialogCreate(AppGlobals appGlobals,
                                  unsigned int audioOutputID)
{
    QBAudioOutputConfig cfg;
    unsigned int i;
    SvWidget helper;
    char *txt = NULL;

    if (QBPlatformGetAudioOutputConfig(audioOutputID, &cfg) < 0)
        return NULL;

    int latency = QBAudioDelayDialogLoadAudioDelay(audioOutputID);
    if (!cfg.latencyControlSupported || latency < 0) {
        SvLogWarning("CubiTV: can't control latency of %s audio output", cfg.name);
        return NULL;
    }

    QBAudioDelayDialog self = calloc(1, sizeof(struct QBAudioDelayDialog_));

    svSettingsPushComponent("AudioDelayDialog.settings");

    helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    helper->prv = self;

    self->audioOutputID = audioOutputID;
    self->savedLatency = latency;

    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };
    self->dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    svWidgetAttach(self->dialog, helper, 0, 0, 0);

    asprintf(&txt, gettext("Select audio delay on %s output"), cfg.name);
    QBDialogSetTitle(self->dialog, txt);
    free(txt);

    QBDialogSetNotificationCallback(self->dialog, self, QBAudioDelayDialogNotificationCallback);
    QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);

    SvArray values = SvArrayCreateWithCapacity(MAX_DELAY/10 + 1, NULL);
    for (i = 0; i <= MAX_DELAY; i += 10) {
        SvString v = SvStringCreateWithFormat("%u ms", i);
        SvArrayAddObject(values, (SvGenericObject) v);
        SVRELEASE(v);
    }

    self->comboBox = QBComboBoxNewFromSM(appGlobals->res, "Dialog.ComboBox");
    QBComboBoxCallbacks cb = { QBAudioDelayDialogPrepareDataCallback,
                               QBAudioDelayDialogInputCallback,
                               QBAudioDelayDialogChangeCallback,
                               QBAudioDelayDialogCurrentCallback,
                               NULL,
                               QBAudioDelayDialogFinishCallback };
    QBComboBoxSetCallbacks(self->comboBox, self, cb);
    QBComboBoxSetContent(self->comboBox, values);
    SVRELEASE(values);
    QBComboBoxSetPosition(self->comboBox, self->savedLatency / 10);
    QBDialogAddWidget(self->dialog, SVSTRING("content"), self->comboBox, SVSTRING("comboBox"), 1, "Dialog.ComboBox");
    svWidgetSetFocusable(self->comboBox, true);
    svWidgetSetFocus(self->comboBox);

    svSettingsPopComponent();

    QBDialogAddButton(self->dialog, SVSTRING("OK-button"), gettext("OK"), 1);
    QBDialogAddButton(self->dialog, SVSTRING("cancel-button"), gettext("Cancel"), 2);

    return self->dialog;
}
