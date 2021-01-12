/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2018 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBJWT_H_
#define QBJWT_H_

#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCommonDefs.h>
#include <stdbool.h>

/**
 * @defgroup QBJWT Cubiware JSON Web Token utilities
 * @ingroup QBUtils
 * @{
 **/

/**
 * JSON Web Token Algorithm
 * @class QBJWTAlgorithm
 */
typedef enum {
    QBJWTAlgorithm_HS512,
    QBJWTAlgorithm_max = QBJWTAlgorithm_HS512
} QBJWTAlgorithm;

/**
* JSON Web Token header parameters
* @class QBJWTHeader
*/
typedef struct QBJWTHeader_ {
    bool useTypValue;           ///< whether to use typ field
    QBJWTAlgorithm algorithm;   ///< encrypting algorithm
} *QBJWTHeader;

/**
 * Create and return a JSON Web Token string.
 *
 * This method creates a JSON Web Token using a specified in the header
 * encrypting algorithm and a secret key.
 *
 * @param[in] jwtHeader         JSON Web Token header
 * @param[in] jsonPayload       JSON Web Token payload
 * @param[in] secret            JSON Web Token secret key
 * @param[out] errorOut         error info
 * @return                      JSON Web Token enrypted string
 **/
SvString
QBJWTCreate(QBJWTHeader jwtHeader, const char *jsonPayload, const char *secret, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBJWT_H_ */
