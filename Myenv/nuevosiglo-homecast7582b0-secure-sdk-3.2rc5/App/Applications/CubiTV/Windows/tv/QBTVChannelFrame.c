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

#include <Windows/tv/QBTVChannelFrame.h>

#include <libintl.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/types.h>
#include <SWL/icon.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <QBViewport.h>
#include <SWL/QBFrame.h>
#include <Utils/QBOSDInputHandler.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Utils/QBPlaylistUtils.h>
#include <Logic/TVLogic.h>

typedef struct QBTVChannelFrame_ *QBTVChannelFrame;

struct QBTVChannelFrame_ {
    SvObject view;
    SvTVChannel channel;
    SvWidget frame, logo, number, name;
    QBAsyncLabelConstructor numberConstructorDisabled, numberConstructorEnabled, nameConstructorDisabled, nameConstructorEnabled;
    int numberX, numberY, numberDisabledX, numberDisabledY;
    int nameX, nameY, nameDisabledX, nameDisabledY;
    SvRID tmpRID;
    int frameRightMargin;

    SvTimerId selectTimer;
    QBOSDInputHandler inputHandler;
    QBTVChannelFrameCallback callback;
    void *callbackData;
    QBTVChannelFrameNameCallback nameCallback;
    void *nameCallbackData;

    QBPlaylistManager playlists;
    QBTVLogic tvLogic;
};

static const unsigned int QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY = 0;
static const unsigned int QBTVChannelFrame_LOGO_ICON_INDEX_ACTUAL = 1;

SvLocal void QBTVChannelFrameClean(SvApplication app, void *ptr)
{
    QBTVChannelFrame prv = (QBTVChannelFrame)ptr;
    SVTESTRELEASE(prv->channel);
    SVRELEASE(prv->nameConstructorDisabled);
    SVRELEASE(prv->nameConstructorEnabled);
    SVRELEASE(prv->numberConstructorDisabled);
    SVRELEASE(prv->numberConstructorEnabled);
    QBOSDInputHandlerDestroy(prv->inputHandler);
    SVRELEASE(prv->playlists);
    SVRELEASE(prv->tvLogic);

    if (prv->selectTimer)
       svAppTimerStop(app, prv->selectTimer);

    free(prv);
}

SvLocal
int QBTVChannelFrameGetMaxDigits(QBTVChannelFrame prv)
{
    int maxDigits = 3;
    size_t channelCount = SvInvokeInterface(SvEPGChannelView, prv->view, getCount);
    if (channelCount > 999) {
        maxDigits = 4;
    } else if (channelCount > 0) {
        SvTVChannel maxChannel = SvInvokeInterface(SvEPGChannelView, prv->view, getByIndex, channelCount - 1);
        if (SvInvokeInterface(SvEPGChannelView, prv->view, getChannelNumber, maxChannel) > 999) {
            maxDigits = 4;
        } else {
            maxDigits = 3;
        }
    }

    return maxDigits;
}

SvLocal void QBTVChannelFrameSelectionClear(SvWidget w)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    QBOSDInputHandlerResetState(prv->inputHandler);
    if (prv->selectTimer)
        svAppTimerStop(w->app, prv->selectTimer);
}

SvLocal void QBTVChannelFrameSetChosenChannel(SvWidget w)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    SvTVChannel channel = QBTVLogicGetChannelByNumber(prv->tvLogic, QBOSDInputHandlerGetChannel(prv->inputHandler), prv->view);
    if (channel) {
        QBTVChannelFrameSetChannel(w, channel);
    } else if (prv->channel) {
        // When there is no chosen channel, set previous one.
        char numStr[10];
        int maxDigits = QBTVChannelFrameGetMaxDigits(prv);
        snprintf(numStr, 10, "%0*d", maxDigits, QBPlaylistManagerGetNumberOfChannelInPlaylist(prv->playlists, prv->view, prv->channel));
        QBAsyncLabelSetCText(prv->number, numStr);
        QBTVChannelFrameSelectionClear(w);
    } else {
        QBAsyncLabelSetText(prv->number, SVSTRING("---"));
        QBTVChannelFrameSelectionClear(w);
    }

    if (prv->callback)
        prv->callback(prv->callbackData, prv->channel);
}

SvLocal void QBTVChannelFrameTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBTVChannelFrame prv = w->prv;
    if (e->id == prv->selectTimer) {
        prv->selectTimer = 0;
        QBTVChannelFrameSetChosenChannel(w);
        return;
    }
}

SvLocal void QBTVChannelFrameReposition(SvWidget w)
{
    QBTVChannelFrame prv = w->prv;
    QBFrameSetWidth(prv->frame, prv->name->off_x + QBAsyncLabelGetWidth(prv->name) + prv->frameRightMargin);
    w->width = prv->frame->off_x + prv->frame->width;
}

SvLocal void QBTVChannelFrameAsyncLabelCallback(void *ptr_, SvWidget label)
{
    SvWidget w = ptr_;
    QBTVChannelFrameReposition(w);
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    if (prv->nameCallback && prv->nameCallbackData)
        prv->nameCallback(prv->nameCallbackData);
}

void
QBTVChannelFrameSetChannelName(SvWidget w, SvString name)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    QBAsyncLabelSetText(prv->name, name);
    QBTVChannelFrameReposition(w);
}

void
QBTVChannelFrameSetChannel(SvWidget w, SvTVChannel channel)
{
    int maxDigits = 3;
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    prv->view = QBPlaylistManagerGetCurrent(prv->playlists);

    if (channel != prv->channel) {
        SVTESTRELEASE(prv->channel);
        prv->channel = SVTESTRETAIN(channel);
    }

    if (prv->number) {
        svWidgetDetach(prv->number);
        svWidgetDestroy(prv->number);
        prv->number = NULL;
    }

    if (prv->name) {
        svWidgetDetach(prv->name);
        svWidgetDestroy(prv->name);
        prv->name = NULL;
    }

    if (!channel) {
        prv->number = QBAsyncLabelNewFromConstructor(w->app, prv->numberConstructorEnabled);
        prv->name = QBAsyncLabelNewFromConstructor(w->app, prv->nameConstructorEnabled);
        svWidgetAttach(w, prv->number, prv->numberX, prv->numberY, 0);
        svWidgetAttach(w, prv->name, prv->nameX, prv->nameY, 0);
        QBAsyncLabelSetCallback(prv->name, QBTVChannelFrameAsyncLabelCallback, w);

        QBAsyncLabelSetText(prv->number, SVSTRING("---"));
        QBAsyncLabelSetCText(prv->name, gettext("No channel"));
        svIconSwitch(prv->logo, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, 0.0f);
    } else {
        maxDigits = QBTVChannelFrameGetMaxDigits(prv);

        char numStr[10];
        snprintf(numStr, 10, "%0*d", maxDigits, QBPlaylistManagerGetNumberOfChannelInPlaylist(prv->playlists, prv->view, channel));

        if (channel->isDisabled) {
            prv->number = QBAsyncLabelNewFromConstructor(w->app, prv->numberConstructorDisabled);
            prv->name = QBAsyncLabelNewFromConstructor(w->app, prv->nameConstructorDisabled);
            svWidgetAttach(w, prv->number, prv->numberDisabledX, prv->numberDisabledY, 0);
            svWidgetAttach(w, prv->name, prv->nameDisabledX, prv->nameDisabledY, 0);
        } else {
            prv->number = QBAsyncLabelNewFromConstructor(w->app, prv->numberConstructorEnabled);
            prv->name = QBAsyncLabelNewFromConstructor(w->app, prv->nameConstructorEnabled);
            svWidgetAttach(w, prv->number, prv->numberX, prv->numberY, 0);
            svWidgetAttach(w, prv->name, prv->nameX, prv->nameY, 0);
        }

        QBAsyncLabelSetCallback(prv->name, QBTVChannelFrameAsyncLabelCallback, w);
        QBAsyncLabelSetCText(prv->number, numStr);
        QBAsyncLabelSetText(prv->name, channel->name);

        const char *uriCStr = NULL;
        if (channel->logoURL) {
           uriCStr = SvStringCString(SvURLString(channel->logoURL));
        }

        if (svIconSetBitmapFromURI(prv->logo, QBTVChannelFrame_LOGO_ICON_INDEX_ACTUAL, uriCStr) != SV_RID_INVALID) {
           svIconSwitch(prv->logo, QBTVChannelFrame_LOGO_ICON_INDEX_ACTUAL, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, 0.0f);
        } else {
           svIconSwitch(prv->logo, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, 0.0f);
        }
    }

    QBTVChannelFrameReposition(w);
    QBTVChannelFrameSelectionClear(w);
}

void
QBTVChannelFrameSetChannelNumberString(SvWidget w, const char *numStr)
{
    QBTVChannelFrame prv = (QBTVChannelFrame)w->prv;
    QBAsyncLabelSetCText(prv->number, numStr);
}

void
QBTVChannelFrameSetChannelNumber(SvWidget w, int num)
{
    QBTVChannelFrame prv = (QBTVChannelFrame)w->prv;
    char buf[30];
    snprintf(buf, sizeof(buf), "%i", num);
    QBAsyncLabelSetCText(prv->number, buf);
}

SvLocal void QBTVChannelFrameFillChannelNumber(SvWidget w, int channum, int digits, int digitsMax)
{
    char numStr[digitsMax + 1];

    snprintf(numStr, digitsMax + 1, "%0*d", digitsMax, channum);
    for (int i = digitsMax - 1 - digits; i >= 0; i--)
        numStr[i] = '-';

    QBTVChannelFrameSetChannelNumberString(w, numStr);
}

int
QBTVChannelFrameInputEventHandler(SvWidget w, const QBInputEvent *ev)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    int ret = QBOSDInputHandlerHandleInput(prv->inputHandler, *ev, &prv->selectTimer, w, prv->playlists);
    if (QBOSDInputHandlerGetSelection(prv->inputHandler) > 0) {
        QBTVChannelFrameFillChannelNumber(w, QBOSDInputHandlerGetChannel(prv->inputHandler),
                                          QBOSDInputHandlerGetSelection(prv->inputHandler),
                                          QBOSDInputHandlerGetSelectionMax(prv->inputHandler));
    }
    if (QBOSDInputHandlerGetChangeChannnel(prv->inputHandler)) {
        QBTVChannelFrameSetChosenChannel(w);
    }
    return ret;
}

void
QBTVChannelFrameNameChangedSetCallback(SvWidget w, QBTVChannelFrameNameCallback callback, void *callbackData)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    prv->nameCallback = callback;
    prv->nameCallbackData = callbackData;
}

void
QBTVChannelFrameSetCallback(SvWidget w, QBTVChannelFrameCallback callback, void *callbackData)
{
    QBTVChannelFrame prv = (QBTVChannelFrame) w->prv;
    prv->callback = callback;
    prv->callbackData = callbackData;
}

SvWidget
QBTVChannelFrameNew(SvApplication app,
                    const char *name,
                    QBTextRenderer textRenderer,
                    QBPlaylistManager playlists,
                    QBTVLogic tvLogic)
{
    QBTVChannelFrame prv = calloc(1, sizeof(struct QBTVChannelFrame_));
    SvWidget w = svSettingsWidgetCreate(app, name);
    char childName[1024];

    w->prv = prv;
    w->clean = QBTVChannelFrameClean;
    svWidgetSetTimerEventHandler(w, QBTVChannelFrameTimerEventHandler);
    prv->playlists = SVRETAIN(playlists);
    prv->tvLogic = SVRETAIN(tvLogic);
    prv->view = QBPlaylistManagerGetCurrent(prv->playlists);
    prv->inputHandler = QBOSDInputHandlerCreate();

    snprintf(childName, sizeof(childName), "%s.%s", name, "frame");
    prv->frame = QBFrameCreateFromSM(app, childName);
    prv->frameRightMargin = svSettingsGetInteger(childName, "rightMargin", 0);
    svSettingsWidgetAttach(w, prv->frame, childName, 0);

    snprintf(childName, sizeof(childName), "%s.%s", name, "number");
    prv->numberConstructorEnabled = QBAsyncLabelConstructorCreate(app, childName, textRenderer, NULL);
    prv->numberX = svSettingsGetInteger(childName, "xOffset", 0);
    prv->numberY = svSettingsGetInteger(childName, "yOffset", 0);
    snprintf(childName, sizeof(childName), "%s.%s", name, "number.Disabled");
    if (svSettingsIsWidgetDefined(childName)) {
        prv->numberConstructorDisabled = QBAsyncLabelConstructorCreate(app, childName, textRenderer, NULL);
        prv->numberDisabledX = svSettingsGetInteger(childName, "xOffset", 0);
        prv->numberDisabledY = svSettingsGetInteger(childName, "yOffset", 0);
    } else {
        prv->numberConstructorDisabled = SVRETAIN(prv->numberConstructorEnabled);
        prv->numberDisabledX = prv->numberX;
        prv->numberDisabledY = prv->numberY;
    }
    prv->number = QBAsyncLabelNewFromConstructor(app, prv->numberConstructorEnabled);
    QBAsyncLabelSetCallback(prv->number, QBTVChannelFrameAsyncLabelCallback, w);
    svWidgetAttach(w, prv->number, prv->numberX, prv->numberY, 0);

    snprintf(childName, sizeof(childName), "%s.%s", name, "name");
    prv->nameConstructorEnabled = QBAsyncLabelConstructorCreate(app, childName, textRenderer, NULL);
    prv->nameX = svSettingsGetInteger(childName, "xOffset", 0);
    prv->nameY = svSettingsGetInteger(childName, "yOffset", 0);
    snprintf(childName, sizeof(childName), "%s.%s", name, "name.Disabled");
    if (svSettingsIsWidgetDefined(childName)) {
        prv->nameConstructorDisabled = QBAsyncLabelConstructorCreate(app, childName, textRenderer, NULL);
        prv->nameDisabledX = svSettingsGetInteger(childName, "xOffset", 0);
        prv->nameDisabledY = svSettingsGetInteger(childName, "yOffset", 0);
    } else {
        prv->nameConstructorDisabled = SVRETAIN(prv->nameConstructorEnabled);
        prv->nameDisabledX = prv->nameX;
        prv->nameDisabledY = prv->nameY;
    }
    prv->name = QBAsyncLabelNewFromConstructor(app, prv->nameConstructorEnabled);
    svWidgetAttach(w, prv->name, prv->nameX, prv->nameY, 0);
    QBAsyncLabelSetCallback(prv->name, QBTVChannelFrameAsyncLabelCallback, w);

    snprintf(childName, sizeof(childName), "%s.%s", name, "logo");
    SvRID tmpRID = svSettingsGetResourceID(childName, "tmpBg");
    prv->tmpRID = tmpRID;
    prv->logo = svIconNew(app, childName);

    svIconSetBitmapFromRID(prv->logo, QBTVChannelFrame_LOGO_ICON_INDEX_TEMPORARY, tmpRID);
    svSettingsWidgetAttach(w, prv->logo, childName, 0);

    return w;
}
