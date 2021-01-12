/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CRYPTO_ENGINE_CONTEXT_PROTECTED_H_
#define QB_CRYPTO_ENGINE_CONTEXT_PROTECTED_H_

#include <QBCryptoTypes.h>
#include <QBCryptoEngineContext.h>
#include <QBCryptoProtected/QBCryptoJobProtected.h>
#include <QBAppKit/QBLockedDeque.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoEngineContextProtected.h QBCryptoEngineContext protected.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @ingroup QBCryptoEngineContext
 * @{
 **/

/**
 * @brief QBCryptoEngineContext protected virtual methods
 */
typedef const struct QBCryptoEngineContextVTable_ {
    struct SvObjectVTable_ super_;  /**< super type */
    /**
     * @brief QBCryptoEngineContextUpdateAlgorithm
     * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
     * @param[in] algorithm algorithm settings
     * @return '0' on success, '-1' on error
     */
    int (*onAlgorithmUpdate)(QBCryptoEngineContext self, const QBCryptoAlgorithmSettings algorithm);

    /**
     * @brief Update key.
     * @param[in] self instance fo @link QBCryptoEngineContext @endlink or its successor
     * @param[in] key crypto key
     * @return '0' on success, '-1' on error
     */
    int (*onKeyUpdate)(QBCryptoEngineContext self, const QBCryptoKey key);

    /**
     * @brief Process job.
     * @param[in] self instance fo @link QBCryptoEngineContext @endlink or its successor
     * @param[in] job crypto job with data
     * @return '0' on success, '-1' on error
     */
    int (*onProcessBlock)(QBCryptoEngineContext self, QBCryptoJob job);
} *QBCryptoEngineContextVTable;

/**
 * @brief QBCryptoEngineContext structure
 */
struct QBCryptoEngineContext_s {
    struct SvObject_ super_;  /**< super type */
    SvScheduler scheduler;  /**< scheduler used internally */
    SvFiber handleCommand;  /**< fiber which is used for command handling */
    QBLockedDeque commands;  /**< commands thread safe queue */
    SvFiber handleFinishedJob;  /**< fiber which is used for calling callbacks */
    QBLockedDeque finishedJobs;  /**< completed jobs thread safe queue */

    bool isBusy;    /**< indicates that context waits for current job */

    pthread_mutex_t mutex;    /**< internal mutex */
    bool deactivated;    /**< if true no callback will be called */
    bool validAlgorithm; /**< @c true if currentAlgorithm is valid */
    struct QBCryptoAlgorithmSettings_s currentAlgorithm; /**< currently used algorithm settings */
    bool validKey; /**< @c true if currentKey is valid */
    struct QBCryptoKey_s currentKey;    /**< currently used key settings */
    SvFiber autoDestroy;    /**< auto destroy fiber */
};

/**
 * Get runtime type identification object representing @link QBCryptoEngineContext @endlink class.
 *
 * @return @link QBCryptoEngineContext @endlink class type
 **/
SvType QBCryptoEngineContext_getType(void);

/**
 * @brief Schedule job for execution.
 *
 * Each job is scheduled at thread safe queue.
 *
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @param[in] job handle to @link QBCryptoJob @endlink
 * @return '0' on success, '-1' on error
 */
int QBCryptoEngineContextScheduleJob(QBCryptoEngineContext self, QBCryptoJob job);

/**
 * @brief Scheduler job on finished job queue.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @param[in] job handle to @link QBCryptoJob @endlink
 * @param status finished job status
 */
void QBCryptoEngineContextJobFinished(QBCryptoEngineContext self, QBCryptoJob job, const QBCryptoOperationStatus status);

/**
 * @brief Initialize base class.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @param[in] scheduler scheduler
 * @param[in] algorithm algorithm settings
 * @param[in] key key settings
 * @return 'true' on success, 'false' on error
 */
bool QBCryptoEngineContextInitialize(QBCryptoEngineContext self, SvScheduler scheduler,
                                     const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Mark if engine context is busy. If the context is busy it can not handle next message.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @param[in] busy @c true if context is busy, @c false if context can handle next message.
 */
void QBCryptoEngineContextSetBusy(QBCryptoEngineContext self, const bool busy);

/**
 * @brief Deactivate context, no callback will be called.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 */
void QBCryptoEngineContextDeactivate(QBCryptoEngineContext self);

/**
 * @brief Destroy context thread safety. It has to be done in context of callback scheduler.
 * In other case there is posible race in SvFiberDestroy on active fiber.
 *
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 */
void QBCryptoEngineContextAutoDestroy(QBCryptoEngineContext self);

/**
 * @brief Update algorithm.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @param[in] algorithm algorithm settings
 */
int QBCryptoEngineContextUpdateAlgorithm(QBCryptoEngineContext self, const QBCryptoAlgorithmSettings algorithm);

/**
 * @brief Get algorithm type.
 * @param[in] self instance of @link QBCryptoEngineContext @endlink or its successor
 * @return type type of algorithm which was set up in constructor
 */
QBCryptoAlgorithmType QBCryptoEngineContextGetAlgorithmType(QBCryptoEngineContext self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QB_CRYPTO_ENGINE_CONTEXT_PROTECTED_H_
