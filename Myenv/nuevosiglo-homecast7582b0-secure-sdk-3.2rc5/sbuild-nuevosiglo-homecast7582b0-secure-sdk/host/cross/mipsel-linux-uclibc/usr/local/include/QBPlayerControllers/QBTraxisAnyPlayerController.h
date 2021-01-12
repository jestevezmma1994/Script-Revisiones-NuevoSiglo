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

#ifndef QBTRAXISANYPLAYERCONTROLLER_H_
#define QBTRAXISANYPLAYERCONTROLLER_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <fibers/c/fibers.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <TraxisWebClient/TraxisWebPlaybackSessionManager.h>
#include <tunerReserver.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <SvPlayerKit/SvDRMInfo.h>

typedef struct QBTraxisAnyPlayerController_ *QBTraxisAnyPlayerController;

QBTraxisAnyPlayerController QBTraxisAnyPlayerControllerCreate(SvScheduler scheduler,
                                                              TraxisWebSessionManager traxisWebSessionManager,
                                                              TraxisWebPlaybackSessionManager traxisWebPlaybackSessionManager,
                                                              QBTunerReserver *tunerReserver,
                                                              QBNATMonitor natMonitor,
                                                              SvObject listener);

/**
 * Set data used by TraxisWebPlaybackSession
 *
 * Typically it is the same object as object being examined in QBTraxisPlayContentStartPlayback.
 * Traxis controller may use this object to obtain additional data about content.
 *
 * @param[in] self traxis player controller handle
 * @param[in] data move description data handle
 */
void QBTraxisAnyPlayerControllerSetWebPlaybackSessionData(QBTraxisAnyPlayerController self, SvDBRawObject data);

/**
 * Set default DRMInfo used by TraxisWebPlaybackSession
 *
 * In case of missing DRM info in content, Traxis uses the default DRM for decryption.
 *
 * @param[in] self traxis player controller handle
 * @param[in] defaultInfo default DRM Info
 */
void QBTraxisAnyPlayerControllerSetDefaultDRMInfo(QBTraxisAnyPlayerController self, SvDRMInfo defaultInfo);

SvType QBTraxisAnyPlayerController_getType(void);

#endif

