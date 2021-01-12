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

#ifndef QB_CRYPTO_ENGINE_H_
#define QB_CRYPTO_ENGINE_H_

#include <fibers/c/fibers.h>
#include <QBCryptoTypes.h>
#include <QBCryptoEngineContext.h>
#include <QBCryptoKeysStore.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoEngine.h QBCryptoEngine class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup CryptoEngine CryptoEngine
 *
 * Library for cryptographic operations.
 */

/**
 * @defgroup QBCryptoEngine QBCryptoEngine main class
 * @ingroup CryptoEngine
 * @{
 **/

/**
 * @brief QBCryptoEngine is a singleton object which gives support for cryptographic operations.
 *
 * It provides functions to check if requested cryptographic operation (based on algorithm settings and key settings) can be performed.
 * QBCryptoEngine creates QBCryptoEngineContexts. Cryptographic operations are performed at those contexts.
 * Each context can be used from different thread.
 *
 * Methods for creating and releasing contexts are thread safe.
 *
 * There are 3 major types of contexts:
 *  - software context - it performs cryptographic operations using openSSL.
 *  - hardware context - it performs cryptographic operations using hardware engine (using clear key).
 *  - cas context      - it performs CA specific cryptographic operations (using encrypted key).
 */
typedef struct QBCryptoEngine_s *QBCryptoEngine;

/**
 * Indicates on type of crypto engine.
 */
typedef enum QBCryptoEngineType_e {
    QBCryptoEngineType_software, /**< indicates on software support (openSSL) */
    QBCryptoEngineType_hardware, /**< indicates on hardware support (dedicated crypto engine) */
    QBCryptoEngineType_cas,      /**< indicates on CA hardware support */
} QBCryptoEngineType;

/**
 * Get runtime type identification object representing @link QBCryptoEngine @endlink class.
 *
 * @return @link QBCryptoEngine @endlink class type
 **/
SvType QBCryptoEngine_getType(void);

/**
 * @brief It returns singleton instance of @link QBCryptoEngine @endlink
 * @return instance of QBCryptoEngine
 */
QBCryptoEngine QBCryptoEngineGetInstance(void);

/**
 * @brief It initializes @link QBCryptoEngine @endlink instance.
 * @param[in] self handle to @link QBCryptoEngine @endlink
 * @param[in] scheduler scheduler which can be used internally by @link QBCryptoEngine @endlink
 * @param[in] casType type of CA system used to set up @link QBCryptoEngine @endlink
 * @return '0' on success, '-1' on error
 */
int QBCryptoEngineInit(QBCryptoEngine self, SvScheduler scheduler, const QBCryptoCASystemType casType);

/**
 * @brief It deinitializes @link QBCryptoEngine @endlink instance.
 * @param[in] self handle to @link QBCryptoEngine @endlink
 */
void QBCryptoEngineDeinit(QBCryptoEngine self);

/**
 * @brief It cleans up singleton instance.
 */
void QBCryptoEngineCleanUp(void);

/**
 * @brief Get instance of @link QBCryptoKeysStore @endlink
 * @param self handle to @link QBCryptoEngine @endlink
 * @return instance of @link QBCryptoKeysStore @endlink, if there is no keys store on the platform, NULL is returned
 */
QBCryptoKeysStore QBCryptoEngineGetKeysStore(QBCryptoEngine self);

/**
 * @brief It checks if there is specific support for chosen algorithm and key
 * @param[in] self handle to @link QBCryptoEngine @endlink
 * @param[in] engineType selected type of cryptographic engine
 * @param[in] algorithm description of an algorithm
 * @param[in] key description of a key
 * @return true if selected type of support is available, false in other case.
 */
bool QBCryptoEngineHasSupport(QBCryptoEngine self, const QBCryptoEngineType engineType,
                              const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Creates instance of @link QBCryptoEngineContext @endlink which performs cryptographic operations using OpenSSL.
 * @param[in] self handle to @link QBCryptoEngine @endlink
 * @param[in] engineType selected type of cryptographic engine
 * @param[in] algorithm description of an algorithm
 * @param[in] key description of a key
 * @return handle to @link QBCryptoEngineContext @endlink instance or NULL in case of error.
 */
QBCryptoEngineContext QBCryptoEngineCreateContext(QBCryptoEngine self, const QBCryptoEngineType engineType,
                                                  const QBCryptoAlgorithmSettings algorithm, const QBCryptoKey key);

/**
 * @brief Destroy context in secure way.
 * It has to be used if @link QBCryptoEngineContext @endlink is removed from diffrent thread.
 * In other case there are might be some races during destruction of context.
 *
 * After calling that method no callback will be called from this context.
 *
 * @param[in] self handle to @link QBCryptoEngine @endlink
 * @param[in] context handle to @link QBCryptoEngineContext @endlink
 */
void QBCryptoEngineDestroyContext(QBCryptoEngine self, QBCryptoEngineContext context);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QB_CRYPTO_ENGINE_H_
