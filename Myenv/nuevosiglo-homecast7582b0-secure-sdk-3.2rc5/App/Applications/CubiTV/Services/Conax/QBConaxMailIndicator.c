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

#include "QBConaxMailIndicator.h"
#include <QBSmartcard2Interface.h>
#include <QBCAS.h>

#include <SvCore/SvLog.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <Services/Conax/QBConaxMailManager.h>
#include <QBApplicationController.h>
#include <QBGlobalWindowManager.h>
#include <main.h>


struct ConaxMailIndicator_s
{
  struct QBGlobalWindow_t super_;

  AppGlobals appGlobals;
  bool is_visible;
};
typedef struct ConaxMailIndicator_s  ConaxMailIndicator;

SvLocal void ConaxMailIndicator_show(ConaxMailIndicator* self)
{
  if (self->is_visible)
    return;

  if (!QBApplicationControllerAddGlobalWindow(self->appGlobals->controller, (QBGlobalWindow)self))
    return;
  self->is_visible = true;
}

SvLocal void ConaxMailIndicator_hide(ConaxMailIndicator* self)
{
  if (!self->is_visible)
    return;

  self->is_visible = false;
  QBApplicationControllerRemoveGlobalWindow(self->appGlobals->controller, (QBGlobalWindow) self);
}

SvLocal void ConaxMailIndicatorDestroy(void *ptr)
{
  ConaxMailIndicator* self = ptr;
  ConaxMailIndicator_hide(self);
}

SvLocal void
ConaxMailIndicator_mail_changed(SvObject self_, size_t mailCount, size_t unreadMailCount)
{
  ConaxMailIndicator* self = (ConaxMailIndicator*) self_;
  if (unreadMailCount > 0) {
    ConaxMailIndicator_show(self);
  } else {
    ConaxMailIndicator_hide(self);
  }
}

SvLocal SvType ConaxMailIndicator_getType(void)
{
  static const struct QBWindowVTable_ vtable = {
    .super_    = {
      .destroy = ConaxMailIndicatorDestroy
    }
  };

  static const struct QBConaxMailManagerListener_ mailManagerListenerMethods = {
    .mailChanged = ConaxMailIndicator_mail_changed
  };
  static SvType type = NULL;

  if (unlikely(!type)) {
    SvTypeCreateManaged("ConaxMailIndicator",
                        sizeof(struct ConaxMailIndicator_s),
                        QBGlobalWindow_getType(),
                        &type,
                        SvObject_getType(), &vtable,
                        QBConaxMailManagerListener_getInterface(), &mailManagerListenerMethods,
                        NULL);
  }
  return type;
}

QBGlobalWindow ConaxMailIndicatorCreate(AppGlobals appGlobals)
{
  if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
    SvLogError("%s :: Conax Mail Indicator should be only created for Conax CAS type", __func__);
    return NULL;
  }

  if (!appGlobals->conaxMailManager) {
    SvLogError("%s(): conaxMailManager missing, can't create ConaxMailIndicator", __func__);
    return NULL;
  }
  SvApplication app = appGlobals->res;

  ConaxMailIndicator* self = (ConaxMailIndicator*) SvTypeAllocateInstance(ConaxMailIndicator_getType(), NULL);
  QBGlobalWindow super = (QBGlobalWindow) self;

  svSettingsPushComponent("ConaxMailIndicator.settings");
  {
    SvWindow window = svSettingsWidgetCreate(app, "ConaxMailIndicator");
    QBGlobalWindowInit(super, window, SVSTRING("ConaxMailIndicator"));
    SvWidget icon = svSettingsWidgetCreate(app, "ConaxMailIndicator.icon");
    svSettingsWidgetAttach(super->window, icon, "ConaxMailIndicator.icon", 1);
  };
  svSettingsPopComponent();

  self->appGlobals = appGlobals;
  self->is_visible = false;

  QBConaxMailManagerAddListener(self->appGlobals->conaxMailManager, (SvObject) self);
  return (QBGlobalWindow) self;
}
