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

#ifndef QB_TIMER_MANAGER_H_
#define QB_TIMER_MANAGER_H_

/**
 * @file QBTimerManager.h
 * @brief Timer manager service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBAppKit/QBTimerTask.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @page ReferenceManual
 *
 * @section TimerManager Timer manager service
 * Timer manager service is a simpliest way to schedule execution of a task
 * for later. There is no public API for creating own instances of this service.
 * The default instance can be retrieved from the service registry,
 * it is registered with well-known name "TimerManager".
 *
 * Timer manager service manages a set of tasks represented as @ref QBTimerTask.
 * @link QBTimerTask @endlink is an abstract class: you can subclass it
 * and implement its QBRunnable::run() virtual method or just implement
 * @ref QBRunnable and call QBTimerManagerCreateTask() to wrap it in a task:
@code
  #include <SvFoundation/SvType.h>
  #include <SvFoundation/SvObject.h>
  #include <QBAppKit/QBServiceRegistry.h>
  #include <QBAppKit/QBRunnable.h>
  #include <QBAppKit/QBTimerManager.h>

  SvLocal void
  DelayedOperationRun(SvObject self, SvObject caller)
  {
      // timer task is our caller
  }

  SvType
  DelayedOperation_getType(void)
  {
      static const struct QBRunnable_ runnableMethods = {
          .run = DelayedOperationRun
      };
      static SvType type = NULL;

      if (!type) {
          SvTypeCreateManaged("DelayedOperation",
                              sizeof(struct DelayedOperation_),
                              SvObject_getType(),
                              &type,
                              QBRunnable_getInterface(), &runnableMethods,
                              NULL);
      }

      return type;
  }

  SvObject operation = SvTypeAllocateInstance(DelayedOperation_getType(), NULL);
  QBTimerManager manager = (QBTimerManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("TimerManager"));
  QBTimerTask task = QBTimerManagerCreateTask(manager, operation, NULL);
  SVRELEASE(operation);
@endcode
 *
 * Scheduling existing task for execution is as simple as calling
 * QBTimerTaskSetup().
**/


/**
 * @defgroup QBTimerManager Timer manager service
 * @ingroup QBAppKit
 * @{
 *
 * @link QBTimerManager @endlink is a service that can execute requested
 * operation at given time, one time or periodically.
 * Refer to @ref TimerManager for more information.
 **/

/**
 * Event bus class.
 * @class QBTimerManager QBTimerManager.h <QBAppKit/QBTimerManager.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 **/
typedef struct QBTimerManager_ *QBTimerManager;


/**
 * Get runtime type identification object representing QBTimerManager class.
 *
 * @relates QBTimerManager
 *
 * @return timer manager class
 **/
extern SvType
QBTimerManager_getType(void);

/**
 * Create a timer task for given runnable operation.
 *
 * @memberof QBTimerManager
 *
 * @param[in] self       timer manager handle
 * @param[in] runnable   object implementing @ref QBRunnable
 * @param[out] errorOut  error info
 * @return               created timer task, @c NULL in case of error
 **/
extern QBTimerTask
QBTimerManagerCreateTask(QBTimerManager self,
                         SvObject runnable,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
