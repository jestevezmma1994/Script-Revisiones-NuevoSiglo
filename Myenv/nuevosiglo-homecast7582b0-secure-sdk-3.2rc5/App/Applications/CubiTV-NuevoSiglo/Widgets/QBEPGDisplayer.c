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

#include <SvEPG/SvEPGDisplayer.h>
#include <settings.h>
#include <main.h>
#include <Widgets/QBRecordingIcon.h>
#include <Widgets/QBEPGDisplayer.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <Widgets/tvEPGOverlay.h>
#include <SWL/QBFrame.h>
#include <SvFoundation/SvInterface.h>

#include <Windows/newtvguide.h>

typedef struct QBEPGDisplayer_t* QBEPGDisplayer;
struct QBEPGDisplayer_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvBitmap defaultBitmap;
    SvBitmap focusBitmap;
    SvBitmap recordableBitmap;
    
    // AMERELES EPG: CatchUp con barra diferencial
    SvBitmap cutvNonFocusBitmap;
};

SvLocal SvBitmap
QBEPGDisplayerGetBackground(SvGenericObject self_,
                                    SvEPGEvent event);

SvLocal SvWidget
QBEPGDisplayerWidgetStateChanged(SvGenericObject self,
                                 SvWidget widget,
                                 SvEPGDisplayerWidgetState state);

SvLocal void
QBEPGDisplayer__dtor__(void* self_)
{
    QBEPGDisplayer self = (QBEPGDisplayer) self_;

    SVTESTRELEASE(self->defaultBitmap);
    SVTESTRELEASE(self->focusBitmap);
    SVTESTRELEASE(self->recordableBitmap);
    
    // AMERELES EPG: CatchUp con barra diferencial
    SVTESTRELEASE(self->cutvNonFocusBitmap);
}

SvLocal SvType
QBEPGDisplayer_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBEPGDisplayer__dtor__
    };
    static const struct SvEPGDisplayer_t methods = {
            .getBackground = QBEPGDisplayerGetBackground,
            .stateChanged = QBEPGDisplayerWidgetStateChanged
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBEPGDisplayer",
                            sizeof(struct QBEPGDisplayer_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvEPGDisplayer_getInterface(), &methods, NULL);
    }

    return type;
}


SvGenericObject
QBEPGDisplayerCreate(AppGlobals globals, const char* widgetName)
{
    if (!widgetName) {
        SvLogError("NULL widget's name passed");
        return NULL;
    }

    QBEPGDisplayer self = (QBEPGDisplayer) SvTypeAllocateInstance(QBEPGDisplayer_getType(), NULL);
    self->appGlobals = globals;

    self->defaultBitmap = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "nonFocusBg"));
    self->focusBitmap = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "focusBg"));
    self->recordableBitmap = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "recordableBg"));
    
    // AMERELES EPG: CatchUp con barra diferencial
    self->cutvNonFocusBitmap = SVTESTRETAIN(svSettingsGetBitmap(widgetName, "cutvNonFocusBg"));

    return (SvGenericObject) self;
}

// BEGIN AMERELES EPG: CatchUp con barra diferencial
SvLocal bool QBEPGDisplayerIsCatchUpEvent(QBEPGDisplayer self, SvEPGEvent event)
{
    time_t now = SvTimeNow();
    if (self->appGlobals->cutvManager && event)
    {
        time_t cutvStart, cutvEnd;
        if (CutvManagerGetEventAvailableTimes(self->appGlobals->cutvManager, event, &cutvStart, &cutvEnd))
        {
            if (EventsLogicIsEventFeatureAllowed(self->appGlobals->eventsLogic, event, EventsLogicAllowedEventFeatureType_CUTV))
            {
                if (cutvStart <= now && cutvEnd > now)
                {
                    if (event->startTime < now)
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    //bool eventCanBeRecorded = event && self->appGlobals->nPVRProvider && SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, isEventRecordable, event);
    bool eventCanBeRecorded = event && SvEPGEventCanBeRemotelyStartedOver(event);
    if (eventCanBeRecorded && event->startTime < now && now < event->endTime)
    {
        return true;
    }
    
    return false;
}
// END AMERELES EPG: CatchUp con barra diferencial

/**
 * Getting an appropriate bitmap for each EPG event. Checking if the event should be highlighted in EPG
 * if yes - returning npvr bitmap,
 * if not - returning default (non focused) bitmap.
 *
 * @param[in] self_         QBEPGDisplayer handler
 * @param[in] event         EPG Event handler
 *
 * @return                  appropriate bitmap for this EPG Event
 */
SvLocal SvBitmap
QBEPGDisplayerGetBackground(SvGenericObject self_,
                               SvEPGEvent event)
{
    QBEPGDisplayer self = (QBEPGDisplayer) self_;
    SvBitmap bmp = NULL;

    if (QBEPGLogicShouldEventBeHighlighted(self->appGlobals->epgLogic, event)) {
        bmp = self->recordableBitmap;
    }
    
    // BEGIN AMERELES EPG: CatchUp con barra diferencial
    if (QBEPGDisplayerIsCatchUpEvent(self, event))
    {
        bmp = self->cutvNonFocusBitmap;
    }
    // END AMERELES EPG: CatchUp con barra diferencial

    if (!bmp) {
        bmp = self->defaultBitmap;
    }

    return bmp;
}

SvLocal SvWidget
QBEPGDisplayerWidgetStateChanged(SvGenericObject self_,
                                 SvWidget widget,
                                 SvEPGDisplayerWidgetState state)
{
    QBEPGDisplayer self = (QBEPGDisplayer) self_;
    SvWidget focusWidget = NULL;

    if ((self->focusBitmap) && (state == SvEPGDisplayerWidgetState_Focused)) {
        QBFrameConstructData params = {
            .bitmap = self->focusBitmap,
            .width = self->focusBitmap->width,
            .height = self->focusBitmap->height,
            .minWidth = 0,
        };

        focusWidget = QBFrameCreate(widget->app, &params);
        QBFrameSetWidth(focusWidget, widget->width);
    }

    return focusWidget;
}
