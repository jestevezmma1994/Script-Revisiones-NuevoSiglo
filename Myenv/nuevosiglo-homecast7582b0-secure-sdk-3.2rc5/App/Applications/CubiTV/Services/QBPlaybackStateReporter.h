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

#ifndef QBPLAYBACKSTATEREPORTER_H
#define QBPLAYBACKSTATEREPORTER_H

/**
 * @file QBPlaybackStateReporter.h
 * @brief Cubiware MW playback state reporter.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 **/

#include <Services/core/QBMiddlewareManager.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBPlaybackStateReporter Cubiware MW playback state reporter
 * @ingroup CubiTV_services
 * @{
 *
 * This service is responsible for sending player state changes to Cubiware MW.
 * It uses Stb.SetState MW call and is used by newtv and pvrplayer.
 **/

/**
 * QBPlaybackStateReporter object handle.
 */
typedef struct QBPlaybackStateReporter_ *QBPlaybackStateReporter;

/**
 * QBPlaybackStateReporterDataSource interface.
 */
typedef struct QBPlaybackStateReporterDataSource_ {
    void (*reportState)(SvObject owner);    /**< force state report to be sent */
} *QBPlaybackStateReporterDataSource;

/**
 * Get QBPlaybackStateReporterDataSource interface object.
 *
 * @return QBPlaybackStateReporterDataSource interface object
 */
SvInterface QBPlaybackStateReporterDataSource_getInterface(void);

/**
 * Create new QBPlaybackStateReporter instance.
 *
 * @param[in] mwManager     QBMiddlewareManager handle
 * @param[out] errorOut     Error information
 * @return created object or @c NULL in case of error
 */
QBPlaybackStateReporter QBPlaybackStateReporterCreate(QBMiddlewareManager mwManager, SvErrorInfo *errorOut);

/**
 * Start QBPlaybackStateReporter service.
 *
 * @param[in] self          QBPlaybackStateReporter handle
 */
void QBPlaybackStateReporterStart(QBPlaybackStateReporter self);

/**
 * Stop QBPlaybackStateReporter service.
 *
 * @param[in] self          QBPlaybackStateReporter handle
 */
void QBPlaybackStateReporterStop(QBPlaybackStateReporter self);

/**
 * Report that new playback was started.
 *
 * Previous playback must be finished before calling that function.
 * To start request productId of title is neccessary.
 *
 * @param[in] self          QBPlaybackStateReporter handle
 * @param[in] productId     Id of started product
 * @param[in] title         Title of started product
 * @param[in] source        Player implementing \ref QBPlaybackStateReporterDataSource that started playback
 */
void QBPlaybackStateReporterReportPlaybackStarted(QBPlaybackStateReporter self, SvValue productId, SvString title, SvObject source);

/**
 * Report that playback state was changed.
 *
 * @param[in] self              QBPlaybackStateReporter handle
 * @param[in] currentPosition   Current playback position
 * @param[in] currentSpeed      Current playback speed
 */
void QBPlaybackStateReporterReportChange(QBPlaybackStateReporter self, int currentPosition, double currentSpeed);

/**
 * Report that playback state was changed.
 *
 * @param[in] self              QBPlaybackStateReporter handle
 * @param[in] currentPosition   Current playback position
 * @param[in] currentSpeed      Current playback speed
 * @param[in] timeshiftDelay    Current timeshift delay (for TV only)
 */
void QBPlaybackStateReporterReportChangeWithTimeshift(QBPlaybackStateReporter self, int currentPosition, double currentSpeed, int timeshiftDelay);

/**
 * Report that current playback was finished.
 *
 * @param[in] self  QBPlaybackStateReporter handle
 */
void QBPlaybackStateReporterReportPlaybackFinished(QBPlaybackStateReporter self);

/**
 * Send current playback state report to Cubiware MW.
 *
 * This function forces MW request to be sent with current playback state.
 * Player is selected based on last playback started.
 *
 * @param[in] self      QBPlaybackStateReporter handle
 */
void QBPlaybackStateReporterForceReport(QBPlaybackStateReporter self);

/**
 * @}
 **/

#endif // QBPLAYBACKSTATEREPORTER_H
