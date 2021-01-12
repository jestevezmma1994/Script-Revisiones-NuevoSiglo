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

#ifndef QB_RCU_PAIRING_SERVICE_H_
#define QB_RCU_PAIRING_SERVICE_H_

/**
 * @file QBRCUPairingService.h
 * @brief Header file containing types and function definiotions of RCU Pairing Service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @defgroup    QBRCUPairingService RCU Pairing Service
 * @ingroup     CubiTV_services
 * @{
 *
 * RCU pairing service manages a process of pairing a new remote controller
 * based on a RF4CE protocol. It provides information for the app if pairing
 * is needed, provides means to start the process of pairing and if pairing
 * is successfully finished it sends out an event on the EventBus.
 *
 **/

/**
 * @brief       RCU pairing service class
 */
typedef struct QBRCUPairingService_ *QBRCUPairingService;

/**
 * @brief       Get runtime type identification object representing QBRCUPairingService class.
 * @return      QBRCUPairingService runtime type identification object
 */
SvType
QBRCUPairingService_getType(void);

/**
 * @brief       Create RCU pairing service class.
 * @return      Created instance of the class, @c NULL in case of error.
 */
extern QBRCUPairingService
QBRCUPairingServiceCreate(void);

/**
 * @brief       Check if RF4CE module is available.
 * @param[in]   self RCU pairing service handle
 * @return      @c true if RF4CE module is available, otherwise @c false.
 */
bool
QBRCUPairingServiceIsRF4CEAvailable(QBRCUPairingService self);

/**
 * @brief       Check if RCU pairing is needed.
 * @param[in]   self RCU pairing service handle
 * @param[out]  errorOut handle to a struct in which errors are to be reported
 * @return      @c true if pairing is needed, @c false otherwise
 */
extern bool
QBRCUPairingServiceIsPairingNeeded(QBRCUPairingService self, SvErrorInfo *errorOut);

/**
 * @brief       An event class sent when device pairing has finished successfully.
 */
typedef struct QBRCUPairingServiceEvent_ *QBRCUPairingServiceEvent;

/**
 * @brief       Result of pairing process.
 */
typedef enum {
    QBRCUPairingServiceEventType_paired = 0,        ///< Successfully paired.
    QBRCUPairingServiceEventType_pairingFailed,     ///< Pairing process failed.
    QBRCUPairingServiceEventType_pairingStarted,    ///< Pairing process restarted.
} QBRCUPairingServiceEventType;

/**
 * @brief       Returns a result of pairing, caried by QBRCUPairingServiceEvent class.
 * @param[in]   self handle to event
 * @return      result of pairing process
 */
extern QBRCUPairingServiceEventType
QBRCUPairingServiceEventGetType(QBRCUPairingServiceEvent self);

/**
 * @brief       Get runtime type identification object representing QBRCUPairingServicePairedEvent class.
 * @return      QBRCUPairingServicePairedEvent runtime type identification object
 */
SvType
QBRCUPairingServiceEvent_getType(void);

/**
 * @brief       Star pairing with a remote controller unit.
 * @param[in]   self a RCU pairing service handle
 * @param[in]   maxAttempts maximum number of attempts to try (one attempts takes circa @c 10 seconds),
 *              @c -1 for infinite number of attempts.
 * @return      @c 0 if successfully started pairing process, otherwise @c -1
 */
extern int
QBRCUPairingServiceStartPairing(QBRCUPairingService self, int maxAttempts);

/**
 * @brief       Stop pairing with a remote controller unit.
 * @param[in]   self a RCU pairing service handle
 * @return      @c 0 if successfully stoped pairing process, otherwise @c -1
 */
extern int
QBRCUPairingServiceStopPairing(QBRCUPairingService self);

/**
 * @brief       Drop all RCU devices.
 * @param[in]   self RCU pairing service handle
 * @param[out]  errorOut handle to a struct in which errors are to be reported
 */
extern void
QBRCUPairingServiceDropAllDevices(QBRCUPairingService self, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_RCU_PAIRING_SERVICE_H_
