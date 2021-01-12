

#include <float.h>
#include <libintl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <settings.h>

#include <CUIT/Core/event.h>
#include <SWL/events.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <SWL/button.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBDialog.h>
#include <QBLocalWindowManager.h>

#include "AnimationCountdownDialog.h"

#define TIMER_INITIAL 10


struct AnimationCountdownDialog_ {
    SvWidget dialog;
    SvWidget helper;
    SvArray buttons;

    QBDialogNotificationCallback notificationCallback;
    QBDialogCallback callback;
    void * notificationCallbackData;
    SvWidgetId owner;

    SvString exitValue;
    unsigned int exitKeyCode;

    SvTimerId timerID;
    int timeout;
    int delay;
    int countServices;
};


typedef struct QBDialogPanel_t *QBDialogPanel;

/**
 * QBDialogItem class.
 **/
typedef struct QBDialogItem_t *QBDialogItem;
struct QBDialogItem_t {
   struct SvObject_ super_;
   SvString tag;
   SvWidget w, contents;
   QBDialogPanel parent;
   void * dialog;
   unsigned int position;
   double fadeDuration;

   // runtime state
   bool destroying;
   int startHeight, reqHeight;
   int startYPos, reqYPos;
   SvEffectId fadeID;
};




SvLocal bool AnimationCountdownDialogNotify(SvWidget dlg, SvString buttonTag, unsigned keyCode);


SvLocal void
AnimationCountdownDialogUserEventHandler(SvWidget dlg, SvWidgetId sender, SvUserEvent ev)
{
    AnimationCountdownDialog self = dlg->prv;

    if (ev->code == SV_EVENT_BUTTON_PUSHED)
    {
        SvIterator buttons = SvArrayIterator(self->buttons);
        QBDialogItem item;

        while ((item = (QBDialogItem) SvIteratorGetNext(&buttons)))
        {
            if (svWidgetGetId(item->contents) == sender)
            {
                if (self->callback)
                {
                    SVTESTRELEASE(self->exitValue);
                    self->exitValue = SVRETAIN(item->tag);
                    self->exitKeyCode = 0;
                }

                if (self->owner)
                    AnimationCountdownDialogNotify(dlg, item->tag, 0);
                else if (self->callback)
                    QBDialogBreak(dlg);
                else
                    QBDialogHide(dlg, false, true);

                return;
            }
        }
   }
}

SvLocal void
AnimationCountdownDialogTimerEventHandler(SvWidget w, SvTimerEvent ev)
{
    AnimationCountdownDialog self = w->prv;

    if (ev->id != self->timerID)
        return;

    if (self->timeout > 0)
    {
        if (--self->timeout == 0)
        {
            if (self->timerID)
                svAppTimerStop(w->app, self->timerID);
            if (self->dialog)
            {
                QBDialogBreak(self->dialog);
            }
            return;
        }
    }
    if (self->delay > 0)
    {
        --self->delay;
    }

    return;
}

SvLocal void AnimationCountdownDialogClean(SvApplication app, void *self_)
{
    AnimationCountdownDialog self = self_;

    if (self->timerID)
        svAppTimerStop(app, self->timerID);

    free(self);
}

SvLocal bool
AnimationCountdownDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    AnimationCountdownDialog self = ptr;

    if (self->dialog)
    {
        if (!buttonTag || self->delay > 0)
            return true;
        if (self->timerID)
            svAppTimerStop(dialog->app, self->timerID);

        self->timerID = 0;
        QBDialogBreak(self->dialog);
        self->dialog = NULL;

        return true;
    }

    return false;
}

SvWidget AnimationCountdownDialogCreate(AppGlobals appGlobals)
{
    AnimationCountdownDialog self = calloc(1, sizeof(struct AnimationCountdownDialog_));
    self->helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    self->helper->prv = self;

    svWidgetSetTimerEventHandler(self->helper, AnimationCountdownDialogTimerEventHandler);
    svWidgetSetUserEventHandler(self->helper, AnimationCountdownDialogUserEventHandler);
    self->helper->clean = AnimationCountdownDialogClean;

    self->timerID = svAppTimerStart(appGlobals->res, self->helper, 1, false);
    self->timeout = TIMER_INITIAL;
    self->delay = TIMER_INITIAL;
    self->countServices = 0;

    QBDialogParameters params = {
      .app        = appGlobals->res,
      .controller = appGlobals->controller,
      .widgetName = "Dialog",
      .ownerId    = 0,
    };

   self->dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
   svWidgetAttach(self->dialog, self->helper, 0, 0, 0);
   QBDialogSetNotificationCallback(self->dialog, self, AnimationCountdownDialogNotificationCallback);
   QBDialogAddPanel(self->dialog, SVSTRING("content"), NULL, 1);
   QBDialogSetTitle(self->dialog, gettext(svSettingsGetString("Popup", "title")));
   SvWidget animation = QBWaitAnimationCreate(appGlobals->res, "Popup.Animation");
   QBDialogAddWidget(self->dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");

   return self->dialog;
}

SvLocal bool
AnimationCountdownDialogNotify(SvWidget dlg, SvString buttonTag, unsigned keyCode)
{
    AnimationCountdownDialog self = dlg->prv;
    if (self->notificationCallbackData)
    {
        if (self->notificationCallback(self->notificationCallbackData, dlg, buttonTag, keyCode))
        {
            return 0;
        }
    }

    return -1;
}
