/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CRYPTO_TYPES_H_
#define QB_CRYPTO_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoTypes.h QBCryptoTypes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoTypes QBCryptoTypes set of crypto primitives.
 * @ingroup CryptoEngine
 * @{
 **/

/**
 * @brief Maximum length of key data.
 */
#define QB_CRYPTO_KEY_MAX_KEY_LENGTH 32

/**
 * Length of initialization vector
 */
#define QB_CRYPTO_KEY_MAX_IV_LENGTH 32

/**
 * @brief Type of CA specific support.
 */
enum QBCryptoCASystemType_e {
    QBCryptoCASystemType_noCA,  /**< Do not initialize CA specific engine. */
    QBCryptoCASystemType_conax, /**< Initialize Conax specific engine. */
    QBCryptoCASystemType_arris, /**< Initialize Arris specific engine. */
};
typedef enum QBCryptoCASystemType_e QBCryptoCASystemType;

/**
 * @brief Type of algorithm.
 *
 * There are two major types of cryptographic algorithms.
 * - encryption - decrypts or encrypts message
 * - mac - calculates message authentication code
 */
enum QBCryptoAlgorithmType_e {
    QBCryptoAlgorithmType_encryption = 0, /**< encryption algorithm type */
    QBCryptoAlgorithmType_mac,            /**< message authentication code algorithm type */
    QBCryptoAlgorithmType_random,         /**< random data generator algorithm type */
};
typedef enum QBCryptoAlgorithmType_e QBCryptoAlgorithmType;

/**
 * @brief Type of mac algorithm.
 */
enum QBCryptoMacAlgorithm_e {
    QBCryptoMacAlgorithm_hmacSha1,        /**< hmac sha1 mac */
    QBCryptoMacAlgorithm_hmacSha256,      /**< hmac sha256 mac */
};
typedef enum QBCryptoMacAlgorithm_e QBCryptoMacAlgorithm;

/**
 * @brief Type of encryption algorithm.
 */
enum QBCryptoAlgorithm_e {
    QBCryptoAlgorithm_Aes,               /**< Aes encryption algorithm */
    QBCryptoAlgorithm_Des,               /**< Des encryption algorithm - it is not secure anymore. */
    QBCryptoAlgorithm_3DesAba,           /**< 3DesAba encryption algorithm */
    QBCryptoAlgorithm_3DesAbc,           /**< 3DesAbc encryption algorithm */
};
typedef enum QBCryptoAlgorithm_e QBCryptoAlgorithm;

/**
 * @brief Block mode of encryption algorithm.
 */
enum QBCryptoAlgorithmBlockMode_e {
    QBCryptoAlgorithmBlockMode_ecb,     /**< Electronic code book encryption algorithm */
    QBCryptoAlgorithmBlockMode_cbc,     /**< Cipher block chaining encryption algorithm */
    QBCryptoAlgorithmBlockMode_ctr,     /**< Counter mode block mode */
};
typedef enum QBCryptoAlgorithmBlockMode_e QBCryptoAlgorithmBlockMode;

/**
 * @brief Type of crypto operation.
 */
enum QBCryptoOperation_e {
    QBCryptoOperation_decrypt,          /**< Perform decryption operation */
    QBCryptoOperation_encrypt,          /**< Perform encryption operation */
};
typedef enum QBCryptoOperation_e QBCryptoOperation;

/**
 * @brief Algorithm settings.
 *
 * Full description of algorithm.
 */
struct QBCryptoAlgorithmSettings_s {
    QBCryptoAlgorithmType type;         /**< Type of the algorithm it can be mac or encryption */
    /**
     * @brief Select one depending on algorithm type.
     */
    union {
        /**
         * @brief Those settings are used if type of algorithm is encryption.
         */
        struct {
            QBCryptoAlgorithm algorithm;    /**< Encryption algorithm */
            QBCryptoAlgorithmBlockMode blockMode;    /**< Block mode */
            QBCryptoOperation operation;    /**< Type of operation */
        };
        /**
         * @brief Those settings are used if type of algorithm is message authentication code.
         */
        struct {
            QBCryptoMacAlgorithm macAlgorithm;    /**< Mac algorithm */
        };
    };
};
typedef struct QBCryptoAlgorithmSettings_s *QBCryptoAlgorithmSettings;

/**
 * @brief Type of key.
 *
 * Key might been given in clear or it might be encrypted in CA specific way.
 * Make sure that @link QBCryptoEngine @endlink has been configured with correct CA.
 */
enum QBCryptoKeyType_e {
    QBCryptoKeyType_clear,    /**< key data is given in clear */
    QBCryptoKeyType_encryptedConaxUnique,    /**< key data is encrypted with conax box unique key */
    QBCryptoKeyType_encryptedConaxLPK,    /**< key data is encrypted with conax LPK key */
    QBCryptoKeyType_encryptedConaxPVRKey,   /**< key data is encrypted with conax PVR Key */
    QBCryptoKeyType_encryptedArrisKey4,   /**< key data is encrypted with Arris Key4 */
};
typedef enum QBCryptoKeyType_e QBCryptoKeyType;

/**
 * @brief Key
 *
 * Full description of key.
 */
struct QBCryptoKey_s {
    QBCryptoKeyType type;    /**< Type of key */
    int keyLen;    /**< length of the key */
    unsigned char key[QB_CRYPTO_KEY_MAX_KEY_LENGTH];    /**< Key's data */
    unsigned char iv[QB_CRYPTO_KEY_MAX_IV_LENGTH];    /**< Initialization vector */
};
typedef struct QBCryptoKey_s *QBCryptoKey;

/**
 * @brief Status of cryptographic operation.
 */
enum QBCryptoOperationStatus_e {
    QBCryptoOperationStatus_scheduled,    /**< Operation has been scheduled and it is processed. */
    QBCryptoOperationStatus_success,    /**< Operation has finished with success. */
    QBCryptoOperationStatus_failed,    /**< Operation has failed. */
};
typedef enum QBCryptoOperationStatus_e QBCryptoOperationStatus;

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QB_CRYPTO_TYPES_H_
