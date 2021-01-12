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

#ifndef QB_CRYPTO_UTILS_H_
#define QB_CRYPTO_UTILS_H_

#include <QBCryptoTypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCryptoUtils.h QBCryptoUtils
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCryptoUtils QBCryptoUtils set of crypto utils methods.
 * @ingroup CryptoEngine
 * @{
 **/

/**
 * @brief Get key length
 * @param[in] key key
 * @return length of key
 */
int QBCryptoUtilsKeyGetKeyLength(const QBCryptoKey key);

/**
 * @brief Get IV length
 * @param[in] algorithm algorithm
 * @return length of IV, or -1 if error
 */
int QBCryptoUtilsKeyGetIVLength(const QBCryptoAlgorithm algorithm);

/**
 * @brief Get output size
 * @param[in] algorithm algorithm
 * @param[in] inputLength input data length
 * @return output data size
 */
int QBCryptoUtilsGetOutputSize(const QBCryptoAlgorithmSettings algorithm, const size_t inputLength);

/**
 * Get block size of an encryption algorithm.
 * @param encryptionAlgorithm algorithm
 * @return block size or -1 in case of unknown algorithm type
 */
int QBCryptoUtilsGetBlockSize(const QBCryptoAlgorithm encryptionAlgorithm);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif //QB_CRYPTO_UTLIS_H_
