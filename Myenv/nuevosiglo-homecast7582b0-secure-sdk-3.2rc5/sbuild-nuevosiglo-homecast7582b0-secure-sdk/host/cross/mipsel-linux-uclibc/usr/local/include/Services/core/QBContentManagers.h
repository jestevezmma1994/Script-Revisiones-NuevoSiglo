/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONTENT_MANAGERS_H_
#define QB_CONTENT_MANAGERS_H_

#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>

typedef enum {
    QBContentManagerType_cubiware = 0,
    QBContentManagerType_traxis,
    QBContentManagerType_dlna,
    QBContentManagerType_mwClient,
    QBContentManagerType_mwClientDiscoveryBar,

    QBContentManagerType_count,
} QBContentManagerType;

/**
 * Content managers service.
 *
 * @class QBContentManagersService QBContentManagers.h <Services/core/QBContentManagers.h>
 * @extends Svobject
 * @implements QBInitializable
 * @implements QBAsyncService
 **/
typedef struct QBContentManagersService_ *QBContentManagersService;

/**
 * Create QBContentManagersService.
 *
 * @return      created content managers service instance, @c NULL if failed
 **/
QBContentManagersService
QBContentManagersServiceCreate(void);

/**
 * Get runtime type identification object representing
 * content managers service type.
 *
 * @relates QBContentManagersService
 *
 * @return content managers service type identification object
 **/
SvType
QBContentManagersService_getType(void);

int
QBContentManagersServiceAdd(QBContentManagersService self,
                            SvURL address,
                            QBContentManagerType type,
                            SvObject manager);

int
QBContentManagersServiceRemove(QBContentManagersService self,
                               SvObject manager);

SvObject
QBContentManagersServiceFindFirst(QBContentManagersService self,
                                  QBContentManagerType type);

SvObject
QBContentManagersServiceGetServiceMonitor(QBContentManagersService self,
                                          SvObject manager);

#endif

