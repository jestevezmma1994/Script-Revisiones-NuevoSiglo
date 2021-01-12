/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CRYPTO_JOB_PROTECTED_H_
#define QB_CRYPTO_JOB_PROTECTED_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvType.h>
#include <QBCryptoTypes.h>
#include <QBCryptoJob.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoJobProtected.h QBCryptoJob protected.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @ingroup QBCryptoJob
 * @{
 **/

/**
 * @brief QBCryptoJob_s structure
 */
struct QBCryptoJob_s {
    struct SvObject_ super_;  /**< super type */
    SvData input;  /**< input data block */
    SvData output;  /**< output data block */
    QBCryptoOperationStatus status; /** operation status */
    bool isCancelled; /**< indicates if job has been cancelled */
    /**
     * @brief Callback listener.
     */
    struct {
        void *target; /**< target */
        QBCryptoOperationCallback callback; /**< callback */
    } listener;
};

/**
 * Get runtime type identification object representing @link QBCryptoJob @endlink class.
 *
 * @return @link QBCryptoJob @endlink class type
 **/
SvType QBCryptoJob_getTyp(void);

/**
 * @brief Create crypto job.
 * @param[in] input input data block
 * @param[in] output output data block, if it is NULL output will be allocated internally.
 * @param[in] target target for callback
 * @param[in] callback callback
 * @return instance of @link QBCryptoJob @endlink in case of success, NULL in case of failure.
 */
QBCryptoJob QBCryptoJobCreate(SvData input, SvData output, void *target, QBCryptoOperationCallback callback);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_CRYPTO_JOB_PROTECTED_H_ */
