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


#ifndef _SOCIAL_MEDIA_REMOTE_ACTION_H_
#define _SOCIAL_MEDIA_REMOTE_ACTION_H_

#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <main_decl.h>

typedef struct QBSocialMediaRemoteAction_t* QBSocialMediaRemoteAction;

typedef void (*QBSocialMediaRemoteActionCallback)(void *self_,
                                                  SvHashTable action,
                                                  SvGenericObject result);

SvType
QBSocialMediaRemoteAction_getType(void);

QBSocialMediaRemoteAction
QBSocialMediaRemoteActionCreate(AppGlobals appGlobals);

void
QBSocialMediaRemoteActionInit(QBSocialMediaRemoteAction self,
                              AppGlobals appGlobals);

void
QBSocialMediaRemoteActionDoAction(QBSocialMediaRemoteAction self,
                                  SvHashTable action,
                                  int serviceId,
                                  QBSocialMediaRemoteActionCallback callback,
                                  void *callbackData);

void
QBSocialMediaRemoteActionCancelAction(QBSocialMediaRemoteAction self);

#endif // _SOCIAL_MEDIA_REMOTE_ACTION_H_

