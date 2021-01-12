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

#ifndef QB_DVB_SUBS_MANAGER_H
#define QB_DVB_SUBS_MANAGER_H

/**
 * @file QBDvbSubsManager.h
 * @brief Teletext manager class.
 */


/**
 * Dvb subtitle Manager class.
 **/
typedef struct QBDvbSubsManager_s* QBDvbSubsManager;

#include <SvPlayerManager/SvPlayerManager.h>
#include <QBSubsManager.h>

struct svdataformat;

/**
 * @defgroup QBDvbSubsManager_create Creating the dvb subtitle manager.
 * @ingroup QBDvbSubsManager
 * @{
 **/


/**
 * Create QBDvbSubsManager object.
 *
 * This is a convenience method that allocates and initializes
 * QBDvbSubsManager object.
 *
 * @param[in] app             CUIT application context
 * @param[in] playerTask      SvPlayerTask
 * @param[in] format          current dataformat describing streams with subtitle
 * @return                    QBDvbSubsManager, NULL in case of error
 **/
QBDvbSubsManager QBDvbSubsManagerCreate(SvApplication app, SvPlayerTask playerTask, const struct svdataformat* format, QBSubsManager subsManager);

typedef struct QBDvbSubsTrack_t *QBDvbSubsTrack;
SvType QBDvbSubsTrack_getType(void);
bool QBDvbSubsTrackisHardOfHearing(QBDvbSubsTrack self);
void QBDvbSubsManagerStop(QBDvbSubsManager self);


/**
 * How to handle subtitle pages with no DDS (Display Definition Segment) present.
 **/
typedef enum {
    /// when no DDS, assume 720x576
    QBDvbSubsManagerDDSMode_PAL = 0,
    /// when no DDS, assume 720x480
    QBDvbSubsManagerDDSMode_NTSC,
    /// default value is 720x576, as defined by ETSI EN 300 743
    QBDvbSubsManagerDDSMode_default = QBDvbSubsManagerDDSMode_PAL,
} QBDvbSubsManagerDDSMode;

extern void QBDvbSubsManagerSetDDSMode(QBDvbSubsManager self, QBDvbSubsManagerDDSMode mode);

static inline void QBDvbSubEnableNTSCHack(QBDvbSubsManager self)
{
    QBDvbSubsManagerSetDDSMode(self, QBDvbSubsManagerDDSMode_NTSC);
}

#endif //QB_DVB_SUBS_MANAGER_H
