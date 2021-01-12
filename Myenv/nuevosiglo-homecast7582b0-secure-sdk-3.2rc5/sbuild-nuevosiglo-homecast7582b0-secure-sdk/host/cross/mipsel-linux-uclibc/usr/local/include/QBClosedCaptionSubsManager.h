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

#ifndef QB_CLOSED_CAPTION_SUBS_MANAGER_H
#define QB_CLOSED_CAPTION_SUBS_MANAGER_H

/**
 * @file QBClosedCaptionSubsManager.h
 * @brief Closed Caption subtitles manager class.
 */


/**
 * Closed Caption subtitles manager class.
 **/
typedef struct QBClosedCaptionSubsManager_ *QBClosedCaptionSubsManager;

#include <SvPlayerManager/SvPlayerManager.h>
#include <QBSubsManager.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>

struct svdataformat;

/**
 * Create QBClosedCaptionSubsManager object.
 *
 * This is a convenience method that allocates and initializes
 * QBClosedCaptionSubsManager object.
 *
 * @param[in] app             CUIT application context
 * @param[in] ccConfig        path to a JSON file that defines closed captioning options
 * @param[in] playerTask      SvPlayerTask
 * @param[in] format          current dataformat describing streams with subtitle
 * @param[in] subsManager     subtitle manager
 * @return                    QBClosedCaptionSubsManager, NULL in case of error
 **/

QBClosedCaptionSubsManager QBClosedCaptionSubsManagerCreate(SvApplication app, SvString ccConfig, SvPlayerTask playerTask, const struct svdataformat *format, QBSubsManager subsManager);
void QBClosedCaptionSubsManagerStop(QBClosedCaptionSubsManager self);

SvType QBDTVCCTrack_getType(void);

SvType QBAnalogCCTrack_getType(void);
const char *QBAnalogCCTrackGetChannelName(SvObject track_);

#endif
