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

#ifndef QB_ASYNC_SERVICE_H_
#define QB_ASYNC_SERVICE_H_

/**
 * @file QBAsyncService.h
 * @brief Abstract asynchronous service interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvImmutableArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBAsyncService Abstract asynchronous service interface
 * @ingroup QBAppKit
 * @{
 **/

/**
 * State of an asynchronous service.
 **/
typedef enum {
    /// unknown state marker
    QBAsyncServiceState_unknown = -1,
    /// service is not running
    QBAsyncServiceState_idle = 0,
    /// service is running, but it is not initialized yet
    QBAsyncServiceState_starting,
    /// service completed initialization and is running
    QBAsyncServiceState_running,
    /// service stops its background tasks, but it is still running
    QBAsyncServiceState_stopping
} QBAsyncServiceState;


/**
 * Abstract asynchronous service interface.
 **/
typedef const struct QBAsyncService_ {
    /**
     * Get well-known name of an asynchronous service.
     *
     * @param[in] self_     handle to an object implementing @ref QBAsyncService
     * @return              well-known service name
     **/
    SvString (*getName)(SvObject self_);

    /**
     * Get names of other services that must be running before service is started.
     *
     * @param[in] self_     handle to an object implementing @ref QBAsyncService
     * @return              array of service names, @c NULL if there are no dependencies
     **/
    SvImmutableArray (*getDependencies)(SvObject self_);

    /**
     * Get current state of an asynchronous service.
     *
     * @param[in] self_     handle to an object implementing @ref QBAsyncService
     * @return              service state
     **/
    QBAsyncServiceState (*getState)(SvObject self_);

    /**
     * Start an asynchronous service.
     *
     * This method will be called by the service registry only for
     * services that are currently in @ref QBAsyncServiceState_idle state.
     * As a result of this method, service state must change to either
     * @ref QBAsyncServiceState_starting or @ref QBAsyncServiceState_running.
     * Before calling this method, service registry will wait until all
     * services described by QBAsyncService::getDependencies() change
     * their state to @ref QBAsyncServiceState_running.
     *
     * @param[in] self_     handle to an object implementing @ref QBAsyncService
     * @param[in] scheduler scheduler to be used by the service
     * @param[out] errorOut error info
     **/
    void (*start)(SvObject self,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut);

    /**
     * Stop an asynchronous service.
     *
     * This method will be called by the service registry only for
     * services that are currently in either @ref QBAsyncServiceState_starting
     * or @ref QBAsyncServiceState_running state.
     * As a result of this method, service state must change to either
     * @ref QBAsyncServiceState_stopping or @ref QBAsyncServiceState_idle.
     *
     * @param[in] self_     handle to an object implementing @ref QBAsyncService
     * @param[out] errorOut error info
     **/
    void (*stop)(SvObject self,
                 SvErrorInfo *errorOut);
} *QBAsyncService;

/**
 * Get runtime type identification object representing
 * QBAsyncService interface.
 *
 * @return QBAsyncService interface object
 **/
extern SvInterface
QBAsyncService_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
