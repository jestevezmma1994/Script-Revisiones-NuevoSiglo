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

#ifndef QB_CRYPTO_HAL_ENGINE_H_
#define QB_CRYPTO_HAL_ENGINE_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvWeakReference.h>
#include <fibers/c/fibers.h>
#include <QBCryptoTypes.h>
#include <QBCryptoEngine.h>
#include <QBCryptoEngineContext.h>
#include <QBCryptoKeysStore.h>
#include <QBAppKit/QBLockedDeque.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoHALEngine.h QBCryptoEngine protected.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @ingroup QBCryptoEngine
 * @{
 **/

/**
 * @brief QBCryptoEngine structure
 */
struct QBCryptoEngine_s {
    struct SvObject_ super_;  /**< super type */
    bool isInitialized;  /**< true if component has been initialized */
    QBCryptoCASystemType casType;  /**< type of CA */
    QBCryptoKeysStore keysStore; /**< keys store */
    SvScheduler scheduler;  /**< used scheduler */
    SvWeakList sharedCasContexts;  /**< shared cas contex instances */
    SvWeakList hwContexts;  /**< hardware context instances */

    pthread_mutex_t mutex;  /** mutex */
};

/**
 * @brief Create crypto engine.
 * @return instance @link QBCryptoEngine @endlink or NULL in case of error.
 */
QBCryptoEngine QBCryptoHALEngineCreate(void);

/**
 * @brief Check if hardware layer supports selected settings.
 * @param self handle to @link QBCryptoEngine @endlink
 * @param algorithm algorithm settings
 * @param key
 * @return
 */
bool QBCryptoEngineHALSupports(QBCryptoEngine self, const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Get max context count.
 *
 * @param self handle to @link QBCryptoEngine @endlink
 * @return max context count
 */
int QBCryptoEngineHALGetMaxContextCount(QBCryptoEngine self);

/**
 * @brief Create hardware context for clear key operations.
 * @param self handle to @link QBCryptoEngine @endlink
 * @param algorithm algorithm settings
 * @param key key
 * @return handle to @link QBCryptoEngineContext @endlink instance or NULL in case of error.
 */
QBCryptoEngineContext QBCryptoEngineHALCreateContextClearKey(QBCryptoEngine self, const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Create hardware context for CA operations.
 * @param self self handle to @link QBCryptoEngine @endlink
 * @param algorithm algorithm settings
 * @param key key
 * @return handle to @link QBCryptoEngineContext @endlink instance or NULL in case of error.
 */
QBCryptoEngineContext QBCryptoEngineHALCreateContextCAS(QBCryptoEngine self, const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Calculate message authentication code for keys store. This is blocking method.
 * @param self handle to @link QBCryptoEngine @endlink
 * @param store key store data
 * @param signatureKey signature key
 * @return message authentication code
 */
SvData QBCryptoEngineHALStoreCalculateMac(QBCryptoEngine self, SvData store, SvData signatureKey);

/**
 * @brief Generate new keys store
 * @param self handle to @link QBCryptoEngine @endlink
 * @param storeSize size of key store
 * @return new key store
 */
SvData QBCryptoEngineHALStoreGenerate(QBCryptoEngine self, size_t storeSize);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QB_CRYPTO_HAL_ENGINE_H_
