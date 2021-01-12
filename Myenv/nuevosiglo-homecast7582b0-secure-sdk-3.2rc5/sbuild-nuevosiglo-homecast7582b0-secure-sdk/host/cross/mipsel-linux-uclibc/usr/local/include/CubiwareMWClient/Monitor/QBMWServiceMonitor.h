/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef INNOV8ONSERVICEMONITOR_H_
#define INNOV8ONSERVICEMONITOR_H_

/**
 * @file QBMWServiceMonitor.h
 * @brief Middleware service monitor API
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvDataBucket2/SvDataBucket.h>


/**
 * @defgroup QBMWServiceMonitor Middleware service monitor class
 * @ingroup CubiTV_services
 * @{
 **/

enum QBMWServiceMonitorEvent_e {
    QBMWServiceMonitor_serviceChanged = 0,
    QBMWServiceMonitor_serviceEnabled,
    QBMWServiceMonitor_serviceDisabled
};
typedef enum QBMWServiceMonitorEvent_e QBMWServiceMonitorEvent;

/**
 * It monitors and notifies about changes in available services for current
 * customer. The list of available services is obtained via stb.GetServices
 * call. The call is executed on start and occasionally redone (for example
 * 2 times a day).
 * When entitlement to a service changes it notifies all listeners.
 *
 * All services have various attributes, but the main ones are 'tag', 'id',
 * 'name'.
 * - tag, it is the type of the service, like 'VOD', 'WEBONTV'
 * - id, unique identifier
 * - name, user presented service name
 *
 * @class QBMWServiceMonitor
 **/
typedef struct QBMWServiceMonitor_t* QBMWServiceMonitor;

typedef void (*QBMWServiceStatusChanged)(SvObject self_, SvDBObject service, QBMWServiceMonitorEvent event);

/**
 * Interface for listening to service changes in QBMWServiceMonitor.
 * @class QBMWServiceMonitorListener
 **/
struct QBMWServiceMonitorListener_t {
    QBMWServiceStatusChanged serviceStatusChanged;
};

typedef struct QBMWServiceMonitorListener_t* QBMWServiceMonitorListener;

/**
 * Get runtime type indentification object representing
 * type of MW Service Monitor listener interface.
 * @return MW Service Monitor listener interface handle
 **/
SvInterface
QBMWServiceMonitorListener_getInterface(void);

/**
 * Adds a new listener that will be notified about service availability
 * changes. Listener must implement the QBMWServiceMonitorListener
 * interface
 * @param[in] self              service monitor handle
 * @param[in] listener          listener object
 **/
void
QBMWServiceMonitorAddListener(QBMWServiceMonitor self,
                              SvObject listener);

/**
 * Remove listener from the service monitor notification list.
 * @param[in] self              service monitor handle
 * @param[in] listener          listener object
 **/
void
QBMWServiceMonitorRemoveListener(QBMWServiceMonitor self,
                                 SvObject listener);

/**
 * Create middleware service monitor.
 *
 * @param[in] middlewareManager    middleware manager
 * @param[in] scheduler         scheduler for running background tasks of service monitor. Usually main app scheduler
 * @param[in] refreshTimeSec    time in seconds between subsequent GetServices requests
 * @param[out] errorOut          error info
 * @return new service monitor instance
 **/
QBMWServiceMonitor
QBMWServiceMonitorCreate(QBMiddlewareManager middlewareManager,
                         SvScheduler scheduler,
                         int refreshTimeSec,
                         SvErrorInfo *errorOut);

/**
 * Starts the service, it will now refresh service availability and
 * will do that periodically.
 * @param[in] self              service monitor handle
 **/
void
QBMWServiceMonitorStart(QBMWServiceMonitor self);

/**
 * Stops the service, it will no longer do periodic checks.
 * @param[in] self              service monitor handle
 **/
void
QBMWServiceMonitorStop(QBMWServiceMonitor self);

/**
 * Used to find a service based on service id. As id is unique it may
 * return at most one service.
 * @param[in] self              service monitor handle
 * @param[in] id                id of service to be found
 * @return service handle iff service with such id exists, otherwise @c NULL
 **/
SvObject
QBMWServiceMonitorFindService(QBMWServiceMonitor self, SvString id);

typedef void (*QBMWServiceMonitorAdaptService)(SvObject self_, SvObject service);

/**
 * Set middleware service adapter.
 *
 * @param[in] self                      middleware service monitor instance handle
 * @param[in] serviceAdapter            service adapter handle
 * @param[in] serviceAdapterCallback    service adaptation request handler
 **/
void
QBMWServiceMonitorSetServiceAdapter(QBMWServiceMonitor self,
                                    SvObject serviceAdapter,
                                    QBMWServiceMonitorAdaptService serviceAdapterCallback);

/**
 * Send GetServices request right now. Cancels pending previous request if any
 *
 * @param[in] self                      middleware service monitor instance handle
 **/
void
QBMWServiceMonitorSendRequest(QBMWServiceMonitor self);

/**
 * @}
 **/


#endif
