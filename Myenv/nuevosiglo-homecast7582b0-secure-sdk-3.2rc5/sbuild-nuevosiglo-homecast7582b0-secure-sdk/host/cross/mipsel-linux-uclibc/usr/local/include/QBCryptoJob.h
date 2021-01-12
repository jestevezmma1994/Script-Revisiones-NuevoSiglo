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

#ifndef QB_CRYPTO_JOB_H_
#define QB_CRYPTO_JOB_H_

#include <QBCryptoTypes.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoJob.h QBCryptoJob protected.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoJob QBCryptoJob main class
 **/

/**
 * @ingroup QBCryptoJob
 * @{
 **/

/**
 * @brief Crypto job class
 */
typedef struct QBCryptoJob_s *QBCryptoJob;

/**
 * @brief Crypto operation callback
 * @param[in] target callback target
 * @param[in] job instance of @link QBCryptoJob @endlink
 * @param[in] status finished job status
 */
typedef void (*QBCryptoOperationCallback)(void *target, QBCryptoJob job);

/**
 * @brief Get operation status.
 * @param[in] self instance of @link QBCryptoJob @endlink
 * @return status as @link QBCryptoOperationStatus @endlink
 */
QBCryptoOperationStatus QBCryptoJobGetOperationStatus(QBCryptoJob self);

/**
 * @brief Get output data.
 * @param[in] self instance of @link QBCryptoJob @endlink
 * @return output data or NULL
 */
SvData QBCryptoJobGetOutput(QBCryptoJob self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_CRYPTO_JOB_H_ */
