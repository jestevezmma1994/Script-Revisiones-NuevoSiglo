/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBBookmarkManagerPlugin.h"
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>

SvInterface
QBBookmarkManagerPlugin_getInterface(void)
{
   static SvInterface interface = NULL;
   SvErrorInfo error = NULL;

   if (!interface) {
      SvInterfaceCreateManaged("QBBookmarkManagerPlugin", sizeof(struct QBBookmarkManagerPlugin_t),
                               NULL, &interface, &error);
      if (error) {
         SvErrorInfoWriteLogMessage(error);
         SvErrorInfoDestroy(error);
         abort();
      }
   }

   return interface;
}

bool
QBBookmarkManagerPluginIsTypeHandled(const QBBookmarkType *handledTypes,
                                     size_t typesCnt,
                                     QBBookmarkType bookmarkType)
{
    size_t i;
    for (i = 0; i < typesCnt; ++i) {
        if (handledTypes[i] == bookmarkType)
            return true;
    }
    return false;
}

