/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBPVODSTORAGE_H_
#define QBPVODSTORAGE_H_

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>

#include <main_decl.h>
#include <Services/core/hotplugMounts.h>

typedef struct QBPVODStorageService_ *QBPVODStorageService;

struct QBPVODStorageListener_ {
    void (*storageConnected)(SvGenericObject self_, SvString path, bool forced);
    void (*storageDisconnected)(SvGenericObject self_);
};
typedef struct QBPVODStorageListener_ *QBPVODStorageListener;

SvInterface
QBPVODStorageListener_getInterface(void);

QBPVODStorageService
QBPVODStorageServiceCreate(AppGlobals appGlobals,
                           SvErrorInfo *errorOut);

void
QBPVODStorageServiceStart(QBPVODStorageService self);

void
QBPVODStorageServiceStop(QBPVODStorageService self);

void
QBPVODStorageDisconnect(QBPVODStorageService self);

void
QBPVODStorageReconnect(QBPVODStorageService self);

bool
QBPVODStorageServiceIsDiskAllowed(QBPVODStorageService self,
                                  QBDiskInfo diskInfo);

SvString
QBPVODStorageServiceGetStoragePath(QBPVODStorageService self,
                                   SvString storage);

void
QBPVODStorageServiceAddListener(QBPVODStorageService self,
                                SvGenericObject listener);

void
QBPVODStorageServiceRemoveListener(QBPVODStorageService self,
                                   SvGenericObject listener);

bool
QBPVODStorageServiceIsStorageSupported(QBPVODStorageService self,
                                       SvString storage);

#endif /* QBPVODSTORAGE_H_ */
