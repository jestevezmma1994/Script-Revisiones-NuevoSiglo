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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#ifndef QBCRYPTO_KEYS_STORE_H_
#define QBCRYPTO_KEYS_STORE_H_

#include <SvFoundation/SvData.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoKeysStore.h QBCryptoKeysStore.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoKeysStore QBCryptoKeysStore class
 **/

/**
 * @ingroup CryptoEngine
 * @{
 **/

/**
 * @brief Defines list of available keys.
 */
typedef enum QBCryptoKeysStoreKey_e {
    QBCryptoKeysStoreKey_pvrKey /**< root key for PVR security */
} QBCryptoKeysStoreKey;

/**
 * @brief QBCryptoKeysStore class
 */
typedef struct QBCryptoKeysStore_s* QBCryptoKeysStore;

/**
 * @brief Get selected key from keys store.
 * @param self instatnce of @link QBCryptoKeysStore @endlink
 * @param key selected key
 * @return instatnce of @link SvData @endlink with key, or NULL if key is not available.
 */
SvData QBCryptoKeysStoreGetKey(QBCryptoKeysStore self, QBCryptoKeysStoreKey key);

/**
 * @}
 **/

#ifdef  __cplusplus
}
#endif

#endif /* QBCRYPTO_KEYS_STORE_H_ */
