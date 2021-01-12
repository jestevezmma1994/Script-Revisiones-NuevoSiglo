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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QB_CONNECTION_MONITOR_H_
#define QB_CONNECTION_MONITOR_H_

/**
 * @file QBConnectionMonitor.h IP Connection Monitor Service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvData.h>
#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

/**
* @defgroup ConnectionMonitor IP Connection Monitor Service
* @ingroup CubiTV_services
* @{
*
* This service is responsible for sending out information about IP connection status.
*
**/

struct QBConnectionMonitorListener_ {
    /**
     * This function is invoked when connection is permanently lost
     *
     * @param[in] self_ object implementing QBConnectionTimer interface
     **/
    void (*ipConnectionPermanentlyLost)(SvObject self_);
    /**
     * This function is invoked when no connection period limit is reached
     *
     * @param[in] self_ object implementing QBConnectionTimer interface
     **/
    void (*ipNoConnectionPeriodLimitReached)(SvObject self_);
    /**
     * This function is invoked when IP connection is stable
     *
     * @param[in] self_ object implementing QBConnectionTimer interface
     **/
    void (*ipConnectionStable)(SvObject self_);
};

/**
* This data type defines an interface for QBConnectionTimer listeners.
*
* @class QBConnectionMonitorListener
**/
typedef struct QBConnectionMonitorListener_* QBConnectionMonitorListener;

/**
 * Create interface for the IP Connection Monitor listeners.
 *
 * @return                      created interface for the listeners, @c NULL in case of error
 **/
SvInterface QBConnectionMonitorListener_getInterface(void);

/**
* This service is responsible for sending out information about IP connection status.
*
* @class QBConnectionMonitor
*
* This service allows to inform its listeners if IP connection is available or not.
**/
typedef struct QBConnectionMonitor_ *QBConnectionMonitor;

/**
 * Add a listener to IP connection Monitor
 *
 * @param[in] self                      QBConnectionMonitor handle
 * @param[in] listener                  object implementing QBConnectionMonitor interface
 * @param[out] errorOut                 error info
 **/
void QBConnectionMonitorAddListener(QBConnectionMonitor self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Remove a listener previously added by QBConnectionMonitorAddListener from
 * active listeners list.
 *
 * @param[in] self                      QBConnectionMonitor handle
 * @param[in] listener                  listener which will be removed from listeners list
 * @param[out] errorOut                 error info
 **/
void QBConnectionMonitorRemoveListener(QBConnectionMonitor self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Create IP connection monitor service.
 *
 * @param[in] scheduler                 Handle to SvScheduler object
 * @return                              returns created QBConnectionMonitor, @c NULL in case of error
 **/
QBConnectionMonitor QBConnectionMonitorCreate(SvScheduler scheduler);

/**
* @}
**/

#endif /* QB_CONNECTION_MONITOR_H_ */
