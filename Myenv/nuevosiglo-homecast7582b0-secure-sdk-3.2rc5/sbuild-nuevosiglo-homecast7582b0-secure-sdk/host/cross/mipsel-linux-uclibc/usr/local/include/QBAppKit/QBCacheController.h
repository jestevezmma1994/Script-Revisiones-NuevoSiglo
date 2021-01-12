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

#ifndef QB_CACHE_CONTROLLER_H_
#define QB_CACHE_CONTROLLER_H_

/**
 * @file QBCacheController.h
 * @brief Cache controller class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBCacheController Cache controller class
 * @ingroup QBAppKit
 * @{
 *
 * @link QBCacheController @endlink is a utility class for controlling the amount
 * of objects cached in memory and removing them on demand.
 **/

/**
 * Cache controller listener interface.
 **/
typedef const struct QBCacheControllerListener_ {
    /**
     * Ask listener to free a collection of cached objects.
     *
     * @param[in] self_     listener handle
     * @param[in] collection collection of objects to be freed
     * @return              @c true if @a collection was removed,
     *                      @c false if it wasn't possible
     **/
    bool (*tryFreeObjects)(SvObject self_,
                           SvObject collection);
} *QBCacheControllerListener;


/**
 * Get runtime type identification object representing
 * QBCacheControllerListener interface.
 *
 * @return QBObserver interface object
 **/
extern SvInterface
QBCacheControllerListener_getInterface(void);


/**
 * Cache controller class.
 * @class QBCacheController
 * @extends SvObject
 **/
typedef struct QBCacheController_ *QBCacheController;

/**
 * Get runtime type identification object representing QBCacheController class.
 *
 * @return cache controller class
 **/
extern SvType
QBCacheController_getType(void);

/**
 * Create an instance of the cache controller class.
 *
 * @memberof QBCacheController
 *
 * @param[out] errorOut error info
 * @return              created cache controller, @c NULL in case of error
 **/
extern QBCacheController
QBCacheControllerCreate(SvErrorInfo *errorOut);

/**
 * Get number of objects in the cache.
 *
 * @param[in] self      cache controller handle
 * @return              number of objects, @c -1 in case of error
 **/
extern ssize_t
QBCacheControllerGetObjectsCount(QBCacheController self);

/**
 * Get number of objects in pinned (non-removable) collections in the cache.
 *
 * @param[in] self      cache controller handle
 * @return              number of objects in pinned collections, @c -1 in case of error
 **/
extern ssize_t
QBCacheControllerGetPinnedObjectsCount(QBCacheController self);

/**
 * Get number of object collections in the cache.
 *
 * @param[in] self      cache controller handle
 * @return              number of collections, @c -1 in case of error
 **/
extern ssize_t
QBCacheControllerGetCollectionsCount(QBCacheController self);

/**
 * Set limits on number and age of objects kept in cache.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] maxObjectsCount max number of objects allowed to be kept in cache,
 *                      @c 0 for no limit
 * @param[in] maxObjectAge time (in seconds) since object was added or touched
 *                      before controller tries to drop it, @c 0 to disable
 *                      this function
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerSetLimits(QBCacheController self,
                           size_t maxObjectsCount,
                           unsigned int maxObjectAge,
                           SvErrorInfo *errorOut);

/**
 * Get limits on number and age of objects kept in cache.
 *
 * @memberof QBCacheController
 *
 * @param[in]  self            cache controller handle
 * @param[out] maxObjectsCount max number of objects allowed to be kept in cache,
 *                             @c 0 for no limit
 * @param[out] maxObjectAge    time (in seconds) since object was added or touched
 *                             before controller tries to drop it, @c 0 to disable
 *                             this function
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerGetLimits(QBCacheController self,
                           size_t *maxObjectsCount,
                           unsigned int *maxObjectAge,
                           SvErrorInfo *errorOut);

/**
 * Start internal asynchronous tasks.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] scheduler scheduler to be used for internal fibers
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerStart(QBCacheController self,
                       SvScheduler scheduler,
                       SvErrorInfo *errorOut);

/**
 * Stop internal tasks.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerStop(QBCacheController self,
                      SvErrorInfo *errorOut);

/**
 * Register a collection of objects to be tracked by the controller.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] collection object representing a collection of cached objects
 * @param[in] length    number of objects in the @a collection
 * @param[in] listener  handle to the @ref QBCacheControllerListener
 *                      responsible for removing this collection of objects
 *                      from cache on controller's request
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerRegisterObjects(QBCacheController self,
                                 SvObject collection,
                                 size_t length,
                                 SvObject listener,
                                 SvErrorInfo *errorOut);

/**
 * Stop tracking previously registered collection of objects.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] collection registered collection of cached objects
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerUnregisterObjects(QBCacheController self,
                                   SvObject collection,
                                   SvErrorInfo *errorOut);

/**
 * Mark collection of objects as recently used.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] collection registered collection of cached objects
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerTouchObjects(QBCacheController self,
                              SvObject collection,
                              SvErrorInfo *errorOut);

/**
 * Mark a collection of objects as non-removable.
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] collection registered collection of cached objects
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerPinObjects(QBCacheController self,
                            SvObject collection,
                            SvErrorInfo *errorOut);

/**
 * Revert effect of QBCacheControllerPinObjects().
 *
 * @memberof QBCacheController
 *
 * @param[in] self      cache controller handle
 * @param[in] collection registered collection of cached objects
 * @param[out] errorOut error info
 **/
extern void
QBCacheControllerUnpinObjects(QBCacheController self,
                              SvObject collection,
                              SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
