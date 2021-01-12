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

#include "osd.h"

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBTimerManager.h>
#include <QBAppKit/QBRunnable.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <SWL/fade.h>
#include <SWL/move.h>
#include <QBConf.h>
#include <string.h>


typedef enum {
   SvOSDMode_top,
   SvOSDMode_bottom,
   SvOSDMode_fixed,
} SvOSDMode;

typedef enum {
   SvOSDEffectType_up,
   SvOSDEffectType_down,
   SvOSDEffectType_fadeIn,
   SvOSDEffectType_fadeOut,
} SvOSDEffectType;


/**
 * OSD widget class.
 * @class SvOSD
 * @extends SvObject
 **/
typedef struct SvOSD_ *SvOSD;

/**
 * SvOSD class internals.
 **/
struct SvOSD_ {
   /// super class
   struct SvObject_ super_;

   SvWidget slave;

   int noHideHeight;
   QBTimerTask hideTimerTask;
   int initialYOffset;
   double duration;
   double hideDelay;
   SvOSDMode mode;
   bool visible;
   bool locked;
   bool hideGently;
   SvEffectId showEffectId;
};


SvType SvOSDStateChangedEvent_getType(void)
{
   static SvType type = NULL;

   if (unlikely(!type)) {
      SvTypeCreateManaged("SvOSDStateChangedEvent",
                          sizeof(struct SvOSDStateChangedEvent_),
                          QBUIEvent_getType(),
                          &type,
                          NULL);
   }

   return type;
}

SvLocal void SvOSDStartTimer(SvOSD self)
{
   if (!self->hideTimerTask) {
      QBTimerManager timerManager = (QBTimerManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("TimerManager"));
      self->hideTimerTask = QBTimerManagerCreateTask(timerManager, (SvObject) self, NULL);
   }
   QBTimerTaskSetup(self->hideTimerTask, SvTimeAdd(SvTimeGet(), SvTimeConstruct(self->hideDelay, 0)), 0, NULL);
}

SvLocal void SvOSDNotify(SvOSD self)
{
   SvOSDStateChangedEvent event = (SvOSDStateChangedEvent) SvTypeAllocateInstance(SvOSDStateChangedEvent_getType(), NULL);
   if (event) {
      event->OSDIsVisible = self->visible;

      QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
      QBEventBusPostEvent(eventBus, (QBPeerEvent) event, (SvObject) self, NULL);
      SVRELEASE(event);
   }
}

bool svOSDIsVisible(SvWidget osd)
{
   SvOSD self = osd->prv;
   return self->visible;
}

void svOSDTouch(SvWidget osd)
{
   SvOSD self = osd->prv;

   if (self->hideTimerTask)
      QBTimerTaskStop(self->hideTimerTask);
   const char *hideDelayStr = QBConfigGet("HIDEINFORMATION");
   if (hideDelayStr)
      self->hideDelay = atoi(hideDelayStr);
   SvOSDStartTimer(self);
}

SvLocal bool svOSDStartEffect(SvOSD self, SvOSDEffectType dir, bool immediate)
{
   SvEffect showEffect = NULL;
   int standardMoveDist = (self->slave->height - self->noHideHeight) * ((dir == SvOSDEffectType_up) ? -1 : 1);
   int moveDist = standardMoveDist;
   if (self->showEffectId <= 0)
      self->initialYOffset = self->slave->off_y;
   else {
      int currMoveDist = self->initialYOffset - self->slave->off_y; // distance of move to initial start position
      if (currMoveDist == 0) {
         return false;
      } else {
         svAppCancelEffect(self->slave->app, self->showEffectId, SV_EFFECT_ABANDON);
         self->showEffectId = 0;
         moveDist = currMoveDist;
         SvOSDEffectType initialDir = (moveDist > 0) ? SvOSDEffectType_up : SvOSDEffectType_down;
         if (initialDir == dir)           // is initial direction re-applied
            moveDist += standardMoveDist;  // as result distance of move to initial destination position
      }
   }
   if (immediate) {
      if (dir == SvOSDEffectType_down || dir == SvOSDEffectType_up)
         self->slave->off_y += moveDist;
      else
         self->slave->tree_alpha = (dir == SvOSDEffectType_fadeIn) ? ALPHA_SOLID : ALPHA_TRANSPARENT;
      svWidgetForceRender(self->slave);
      svOSDTouch(self->slave);
   } else {
      if (dir == SvOSDEffectType_down || dir == SvOSDEffectType_up)
         showEffect = svEffectMoveNew(self->slave, self->slave->off_x, self->slave->off_y + moveDist,
                                      self->duration * moveDist / standardMoveDist, SV_EFFECT_MOVE_LINEAR, 0);
      else
         showEffect = svEffectFadeNew(self->slave, (dir == SvOSDEffectType_fadeIn) ? ALPHA_SOLID : ALPHA_TRANSPARENT, true, self->duration,
                                      SV_EFFECT_FADE_LINEAR);
   }
   if (showEffect) {
      self->showEffectId = svAppRegisterEffect(self->slave->app, showEffect);
   }
   svWidgetForceRender(self->slave);
   return true;
}

void svOSDShow(SvWidget osd, bool immediate)
{
   SvOSD self = osd->prv;

   if (self->locked) {
      return;
   }

   if (self->visible) {
      svOSDTouch(osd);
      return;
   }

   if (self->hideTimerTask) {
      QBTimerTaskStop(self->hideTimerTask);
   }

   bool result = false;
   switch (self->mode) {
      case SvOSDMode_top:
         result = svOSDStartEffect(self, SvOSDEffectType_down, immediate);
         break;
      case SvOSDMode_bottom:
         result = svOSDStartEffect(self, SvOSDEffectType_up, immediate);
         break;
      case SvOSDMode_fixed:
         result = svOSDStartEffect(self, SvOSDEffectType_fadeIn, immediate);
         break;
   }

   if (result) {
      self->visible = true;
      if (immediate)
         SvOSDNotify(self);
   }
}

void svOSDHide(SvWidget osd, bool immediate)
{
   SvOSD self = osd->prv;

   if (!self->visible)
      return;

   if (self->locked) {
      return;
   }

   if (self->hideTimerTask) {
      QBTimerTaskStop(self->hideTimerTask);
   }

   bool result = false;
   switch (self->mode) {
      case SvOSDMode_top:
         result = svOSDStartEffect(self, SvOSDEffectType_up, immediate);
         break;
      case SvOSDMode_bottom:
         result = svOSDStartEffect(self, SvOSDEffectType_down, immediate);
         break;
      case SvOSDMode_fixed:
         result = svOSDStartEffect(self, SvOSDEffectType_fadeOut, immediate);
         break;
   }

   if (result) {
      self->visible = false;
      if (immediate)
         SvOSDNotify(self);
   }
}

void svOSDLock(SvWidget osd)
{
   SvOSD self = osd->prv;

   if (self->locked)
      return;

   self->locked = true;

   if (self->hideTimerTask) {
      QBTimerTaskStop(self->hideTimerTask);
   }
}

void svOSDUnlock(SvWidget osd)
{
   SvOSD self = osd->prv;

   if (!(self->locked))
      return;

   self->locked = false;
   SvOSDStartTimer(self);
}

SvLocal void SvOSDEffectEventHandler(SvWidget osd, SvEffectEvent e)
{
   SvOSD self = osd->prv;

   if (e->id == self->showEffectId) {
      self->showEffectId = 0;
      if (self->visible)
         svOSDTouch(osd);
      SvOSDNotify(self);
   }
}

SvLocal void SvOSDCleanup(SvApplication app, void *self_)
{
   SvOSD self = (SvOSD) self_;

   if (self->hideTimerTask)
      QBTimerTaskStop(self->hideTimerTask);
   if (self->showEffectId > 0) {
      svAppCancelEffect(app, self->showEffectId, SV_EFFECT_FINISH);
   }

   self->slave = NULL;
   SVRELEASE(self);
}

SvLocal void SvOSDRunTimerTask(SvObject self_, SvObject caller)
{
   SvOSD self = (SvOSD) self_;

   if (!self->slave)
      return;

   if (caller == (SvObject) self->hideTimerTask) {
      if (!self->locked) {
         svOSDHide(self->slave, !self->hideGently);
         SvOSDStartTimer(self);
      }
   }
}

void svOSDAttachToParent(SvWidget osd, SvWidget parent, int level)
{
   SvOSD self = osd->prv;

   switch (self->mode) {
      case SvOSDMode_top:
         svWidgetAttach(parent, osd, 0, -(osd->height - self->noHideHeight), level);
         break;
      case SvOSDMode_bottom:
         svWidgetAttach(parent, osd, 0, parent->height - self->noHideHeight, level);
         break;
      case SvOSDMode_fixed:
         svSettingsWidgetAttach(parent, osd, svWidgetGetName(osd), level);
         osd->tree_alpha = ALPHA_TRANSPARENT;
         break;
   }
}

SvLocal void SvOSDDescribe(void *self_, void *outputStream_)
{
   SvOSD self = self_;
   QBOutputStream outputStream = outputStream_;

   if (self->slave) {
      QBOutputStreamWriteFormatted(outputStream, "<SvOSD@%p name:%s>", self, svWidgetGetName(self->slave));
   } else {
      QBOutputStreamWriteFormatted(outputStream, "<SvOSD@%p>", self);
   }
}

SvLocal void SvOSDDestroy(void *self_)
{
   SvOSD self = self_;
   SVTESTRELEASE(self->hideTimerTask);
}

SvLocal SvType SvOSD_getType(void)
{
   static const struct SvObjectVTable_ objectVTable = {
      .destroy  = SvOSDDestroy,
      .describe = SvOSDDescribe
   };
   static const struct QBRunnable_ runnableMethods = {
      .run = SvOSDRunTimerTask
   };
   static SvType type = NULL;

   if (unlikely(!type)) {
      SvTypeCreateManaged("SvOSD",
                          sizeof(struct SvOSD_),
                          SvObject_getType(),
                          &type,
                          SvObject_getType(), &objectVTable,
                          QBRunnable_getInterface(), &runnableMethods,
                          NULL);
   }

   return type;
}

SvWidget svOSDNew(SvApplication app, const char *widgetName)
{
   SvOSD self = NULL;

   const char *modeStr;
   double duration, hideDelay;
   bool hideGently;
   int noHideHeight;
   duration = svSettingsGetDouble(widgetName, "duration", 0.0);
   hideDelay = svSettingsGetDouble(widgetName, "hideDelay", 2.0);
   modeStr = svSettingsGetString(widgetName, "mode");
   hideGently = svSettingsGetBoolean(widgetName, "hideGently", false);
   noHideHeight = svSettingsGetInteger(widgetName, "noHideHeight", 0);

   self = (SvOSD) SvTypeAllocateInstance(SvOSD_getType(), NULL);
   if (unlikely(!self))
      return NULL;

   if (modeStr) {
      if (strcmp(modeStr, "bottom") == 0) {
         self->mode = SvOSDMode_bottom;
      } else if (!strcmp(modeStr, "top")) {
         self->mode = SvOSDMode_top;
      } else {
         self->mode = SvOSDMode_fixed;
      }
   } else {
      self->mode = SvOSDMode_top;
   }
   self->noHideHeight = noHideHeight;
   self->duration = duration;
   self->hideDelay = hideDelay;
   self->visible = false;
   self->locked = false;
   self->hideGently = hideGently;
   self->showEffectId = 0;

   self->slave = svSettingsWidgetCreate(app, widgetName);
   if (!self->slave) {
      SVRELEASE(self);
      return NULL;
   }

   self->slave->prv = self;
   self->slave->clean = SvOSDCleanup;
   svWidgetSetFocusable(self->slave, false);
   svWidgetSetEffectEventHandler(self->slave, SvOSDEffectEventHandler);

   return self->slave;
}
