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

/* App/Libraries/QBDSMCC/Headers/QBDSMCC/QBDSMCCMonitor.h */

#ifndef QB_DSMCC_MONITOR_H_
#define QB_DSMCC_MONITOR_H_

/**
 * @file QBDSMCCMonitor.h
 * @brief DSM-CC Monitor API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDSMCCClient.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBDSMCCMonitor DSM-CC Monitor
 * @ingroup QBDSMCC
 * @{
 **/

/**
 * QBDSMCCMonitor class.
 *
 * Object of this class handles proper tuning and pushing data packets to QBDSMCCClient.
 **/
typedef struct QBDSMCCMonitor_s *QBDSMCCMonitor;

/**
 * Create QBDSMCCMonitor.
 *
 * @param[in] tunerCnt      number of available tuners
 * @return                  QBDSMCCMonitor handle or NULL
 **/
QBDSMCCMonitor QBDSMCCMonitorCreate(int tunerCnt);

/**
 * Create QBDSMCCMonitor with config from file.
 *
 * @param[in] tunerCnt      number of available tuners
 * @param[in] filePath      Path to config file
 *
 * Expected config file format:
 *
 @verbatim
     {
         "carouselId": <number>
     }
 @endverbatim
 *
 * @return                  QBDSMCCMonitor handle or NULL
 */
QBDSMCCMonitor QBDSMCCMonitorCreateWithConfig(int tunerCnt, SvString filePath);

/**
 * Start service.
 *
 * @param[in] self          QBDSMCCMonitor handle
 **/
void QBDSMCCMonitorStart(QBDSMCCMonitor self);

/**
 * Stop service.
 *
 * @param[in] self          QBDSMCCMonitor handle
 **/
void QBDSMCCMonitorStop(QBDSMCCMonitor self);

/**
 * Set carouselId.
 *
 * Creates DSM-CC client for given carouselId.
 *
 * @param[in] self          QBDSMCCMonitor handle
 * @param[in] carouselId    carousel id
 * @return
 **/
bool QBDSMCCMonitorSetCarouselId(QBDSMCCMonitor self, uint32_t carouselId);

/**
 * Set PID
 *
 * Sets tuner filters to non-default PID (default is 0x105)
 *
 * @param[in] self          QBDSMCCMonitor handle
 * @param[in] PID           PID
 * @return                  @c true on success, @c false on incorrect parameters
 **/
bool QBDSMCCMonitorSetPID(QBDSMCCMonitor self, uint16_t PID);

/**
 * Get DSM-CC client.
 *
 * @param[in] self          QBDSMCCMonitor handle
 * @return                  Associated DSM-CC client or NULL
 **/
QBDSMCCClient QBDSMCCMonitorGetClient(QBDSMCCMonitor self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_DSMCC_MONITOR_H_ */
