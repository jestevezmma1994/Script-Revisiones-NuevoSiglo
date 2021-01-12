/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SERVICE_REGISTRY_H_
#define QB_SERVICE_REGISTRY_H_

/**
 * @file QBServiceRegistry.h
 * @brief Service registry (asynchronous services manager) class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBPropertiesMap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBServiceRegistry Service registry class
 * @ingroup QBAppKit
 * @{
 **/

/**
 * Service registry (asynchronous services manager) singleton class.
 * @class QBServiceRegistry
 * @extends SvObject
 **/
typedef struct QBServiceRegistry_ *QBServiceRegistry;


/**
 * Get handle to the service registry singleton.
 *
 * @return              service registry handle
 **/
extern QBServiceRegistry
QBServiceRegistryGetInstance(void);

/**
 * Get current state of the service registry.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @return              service registry state
 **/
extern QBAsyncServiceState
QBServiceRegistryGetState(QBServiceRegistry self);

/**
 * Start all registered services.
 *
 * This method will initiate the process of starting all
 * registered services. Services are started in the order
 * defined by their dependencies, so that a service is
 * started only when all other services it depends upon
 * are already running.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[in] scheduler scheduler handle
 * @param[out] errorOut error info
 **/
extern void
QBServiceRegistryStartServices(QBServiceRegistry self,
                               SvScheduler scheduler,
                               SvErrorInfo *errorOut);

/**
 * Stop all registered services.
 *
 * This method will initiate the process of stopping all
 * running services. Services are started in the order
 * defined by their dependencies, similarly (but obviously
 * in reverse direction) to how services are started.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[in] timeout   time after which registry will give up trying
 *                      to stop services in correct order, in seconds
 *                      (pass @c -1 for infinity)
 * @param[out] errorOut error info
 **/
extern void
QBServiceRegistryStopServices(QBServiceRegistry self,
                              int timeout,
                              SvErrorInfo *errorOut);

/**
 * Create and return an array of all registered services.
 *
 * @memberof QBServiceRegistry
 * @qb_allocator
 *
 * @param[in] self      service registry handle
 * @param[out] errorOut error info
 * @return              created array of all registered services
 **/
extern SvArray
QBServiceRegistryCreateServicesList(QBServiceRegistry self,
                                    SvErrorInfo *errorOut);

/**
 * Get handle to one of registered services.
 *
 * This method will return a handle to the service with given @a name.
 * If such service does not exist yet, but its type was registered
 * using QBServiceRegistryRegisterServiceType(), it is created
 * (and also started if service registry was started.)
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[in] name      service name
 * @return              service handle
 **/
extern SvObject
QBServiceRegistryGetService(QBServiceRegistry self,
                            SvString name);

/**
 * Register new service.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[in] service   service handle, must implement @ref QBAsyncService
 * @param[out] errorOut error info
 **/
extern void
QBServiceRegistryRegisterService(QBServiceRegistry self,
                                 SvObject service,
                                 SvErrorInfo *errorOut);

/**
 * Register new service with its type used as factory.
 *
 * This method registers new service with its type.
 * The actual service object will be created when service
 * registry is asked for this service for the first time
 * using QBServiceRegistryGetService().
 *
 * If the service requires some additional information to be initialized,
 * this information must be passed in @a properties. In such case, the
 * service must implement @ref QBInitializable.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[in] serviceName service well-known name
 * @param[in] serviceType type of the service
 * @param[in] properties properties to initialize service with,
 *                      pass @c NULL if not needed
 * @param[out] errorOut error info
 **/
extern void
QBServiceRegistryRegisterServiceType(QBServiceRegistry self,
                                     SvString serviceName,
                                     SvType serviceType,
                                     QBPropertiesMap properties,
                                     SvErrorInfo *errorOut);

/**
 * Remove all services.
 *
 * @memberof QBServiceRegistry
 *
 * @param[in] self      service registry handle
 * @param[out] errorOut error info
 **/
extern void
QBServiceRegistryRemoveAllServices(QBServiceRegistry self,
                                   SvErrorInfo *errorOut);

/**
 * Save current state.
 *
 * @memberof QBServiceRegistry
 *
 * This methods saves list of registered services.
 * Use QBServiceRegistryRestoreState() to restore them.
 *
 * @warning This method assumes that all services can be started and stopped
 *          synchronously. For more info see ASYNCHRONOUS_START_STOP macro
 *          description.
 *
 * @param[in]  self      service registry handle
 * @param[in]  stateName state name
 * @param[out] errorOut  error info
 **/
extern void
QBServiceRegistrySaveState(QBServiceRegistry self,
                           SvString stateName,
                           SvErrorInfo *errorOut);

/**
 * Restore saved state.
 *
 * @memberof QBServiceRegistry
 *
 * This methods restores all services saved by QBServiceRegistrySaveState().
 *
 * @warning This method assumes that all services can be started and stopped
 *          synchronously. For more info see ASYNCHRONOUS_START_STOP macro
 *          description.
 *
 * @param[in]  self      service registry handle
 * @param[in]  stateName name of the saved state
 * @param[in]  timeout   time after which registry will give up trying
 *                       to stop services in correct order, in seconds
 *                       (pass @c -1 for infinity) (UNUSED)
 * @param[out] errorOut  error info
 **/
extern void
QBServiceRegistryRestoreState(QBServiceRegistry self,
                              SvString stateName,
                              size_t timeout,
                              SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
