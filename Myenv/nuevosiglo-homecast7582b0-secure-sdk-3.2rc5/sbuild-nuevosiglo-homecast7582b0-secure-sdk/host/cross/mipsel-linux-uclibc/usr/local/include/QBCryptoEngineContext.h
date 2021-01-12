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

#ifndef QB_CRYPTO_ENGINE_CONTEXT_H_
#define QB_CRYPTO_ENGINE_CONTEXT_H_

#include <QBCryptoTypes.h>
#include <QBCryptoJob.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoEngineContext.h QBCryptoEngineContext class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoEngineContext QBCryptoEngineContext main class
 * @ingroup CryptoEngine
 * @{
 **/

/**
 * @brief QBCryptoEngineContext object on which cryptographic operations can be performed.
 *
 * Method QBCryptoEngineContextOperation can be called few times in the row without waiting until previous job finish.
 * For each call callback will be called.
 *
 * Method QBCryptoEngineContextUpdateKey can be also called between calls of QBCryptoEngineContextOperation methods.
 *
 * Object can be used from any thread. It has to be release by method @see QBCryptoEngineReleaseContext,
 * to avoid races during object destruction.
 *
 */
typedef struct QBCryptoEngineContext_s *QBCryptoEngineContext;

/**
 * @brief Update key at crypto context.
 * @param[in] self handle to @link QBCryptoEngineContext @endlink
 * @param[in] key crypto key
 * @return '0' on success, '-1' on error
 */
int QBCryptoEngineContextUpdateKey(QBCryptoEngineContext self, const QBCryptoKey key);

/**
 * @brief This method creates and schedules crypto operation.
 *
 * Input and output data have to be diffrent instances.
 *
 * Callback is mandatory. It will be called in context of scheduler
 * passed to @link QBCryptoEngine @endlink at initialization.
 *
 * Method can be called few times in the row without waiting until previous job finish.
 * For each call callback will be called.
 *
 * If method returns handle to job, it means that it has been created and scheduled successfully.
 * If method returns NULL, it means that creation of job has failed or it couldn't have been scheduled.
 *
 * @param[in] self handle to @link QBCryptoEngineContext @endlink
 * @param[in] input instance of @link SvData @endlink with input data.
 * @param[in] output instance of @link SvData @endlink where output data will be stored.
 * @param[in] target target for callback, can be NULL
 * @param[in] callback callback called after operation finish
 * @return instance of @link QBCryptoJob @endlink or NULL in case of error.
 */
QBCryptoJob QBCryptoEngineContextCreateAndScheduleOperation(QBCryptoEngineContext self, SvData input, SvData output,
                                                            void *target, QBCryptoOperationCallback callback);

/**
 * @brief Method cancels pending crypto operation. Operation callback will not be called.
 * @param[in] self handle to @link QBCryptoEngineContext @endlink
 * @param[in] job instance of @link QBCryptoJob @endlink to be cancelled.
 */
void QBCryptoEngineContextCancelJob(QBCryptoEngineContext self, QBCryptoJob job);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QB_CRYPTO_ENGINE_CONTEXT_H_
