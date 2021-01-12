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

#ifndef QB_NIT_UPGRADE_MONITOR_H_
#define QB_NIT_UPGRADE_MONITOR_H_

/**
 * @file QBNITUpgradeMonitor.h
 * @brief DVB NIT upgrade monitor class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBNITUpgradeMonitor DVB NIT upgrade monitor class
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBUpgrade/QBFirmwareVersion.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>
#include <QBDVBMuxesMap.h>
#include <QBDVBSatellitesDB.h>


/**
 * DVB NIT upgrade monitor class.
 * @class QBNITUpgradeMonitor
 * @extends SvObject
 **/
typedef struct QBNITUpgradeMonitor_ *QBNITUpgradeMonitor;


/**
 * Get runtime type identification object
 * representing QBNITUpgradeMonitor class.
 *
 * @return QBNITUpgradeMonitor type identification object
 **/
extern SvType
QBNITUpgradeMonitor_getType(void);

/**
 * Initialize DVB NIT upgrade monitor.
 *
 * @memberof QBNITUpgradeMonitor
 *
 * @param[in] self      DVB NIT upgrade monitor handle
 * @param[in] service   upgrade service handle
 * @param[in] selector  upgrade selector values
 * @param[in] tunersCnt number of tuners to use
 * @param[in] satellitesDB satellites database handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBNITUpgradeMonitor
QBNITUpgradeMonitorInit(QBNITUpgradeMonitor self,
                        QBUpgradeService service,
                        const QBDVBUpgradeSelector *const selector,
                        unsigned int tunersCnt,
                        QBDVBSatellitesDB satellitesDB,
                        SvErrorInfo *errorOut);

/**
 * Set NID to be monitored.
 *
 * @memberof QBNITUpgradeMonitor
 *
 * @param[in] self      DVB NIT upgrade monitor handle
 * @param[in] chosenNID NID to be monitored
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern void QBNITUpgradeMonitorSetChosenNID(QBNITUpgradeMonitor self,
                                            int chosenNID,
                                            SvErrorInfo *errorOut);

/**
 * Set map of DVB muxes that can carry DVB upgrade carousel.
 *
 * @memberof QBNITUpgradeMonitor
 *
 * @param[in] self      DVB NIT upgrade monitor handle
 * @param[in] map       DVB upgrade muxes map handle
 * @param[out] errorOut error info
 **/
extern void
QBNITUpgradeMonitorSetMuxesMap(QBNITUpgradeMonitor self,
                               QBDVBMuxesMap map,
                               SvErrorInfo *errorOut);

/**
 * Get information about the firmware upgrade data stream.
 *
 * @memberof QBNITUpgradeMonitor
 *
 * @param[in] self      DVB NIT upgrade monitor handle
 * @param[out] uri      DVB upgrade URI
 * @param[out] forced   @c true if upgrade is forced, @c false otherwise
 * @param[out] errorOut error info
 * @return              firmware version on @c NULL if not available
 **/
extern QBFirmwareVersion
QBNITUpgradeMonitorGetUpgradeInfo(QBNITUpgradeMonitor self,
                                  SvURI *uri,
                                  bool *forced,
                                  SvErrorInfo *errorOut);

/**
 * Get name of the network where the firmware upgrade was found.
 *
 * @memberof QBNITUpgradeMonitor
 *
 * @param[in] self      DVB NIT upgrade monitor handle
 * @return              network name, @c NULL if not available
 **/
extern SvString
QBNITUpgradeMonitorGetNetworkName(QBNITUpgradeMonitor self);

/**
 * @}
 **/


#endif
