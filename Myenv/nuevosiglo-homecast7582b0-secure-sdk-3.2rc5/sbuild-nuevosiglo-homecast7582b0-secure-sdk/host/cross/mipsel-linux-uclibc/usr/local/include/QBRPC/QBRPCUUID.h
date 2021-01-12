/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RPC_UUID_H_
#define QB_RPC_UUID_H_

/**
 * @file QBRPCUUID.h
 * @brief QBRPCUUID API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup UUID UUID
 * @ingroup QBRPC
 * @{
 */

/**
 * Length of a uuid string including @c NULL terminator.
 */
#define QBRPCUUID_STRING_LENGTH 37


/**
 * QBRPCUUID class.
 *
 * Contains valid, normalized (lowercase) uuid string.
 *
 * @class QBRPCUUID
 */
typedef struct QBRPCUUID_* QBRPCUUID;

/**
 * Get runtime type identification object representing QBRPCUUID class.
 *
 * @memberof QBRPCUUID
 *
 * @return QBRPCUUID class
 **/
SvType QBRPCUUID_getType(void);

/**
 * Create QBRPCUUID object from string.
 *
 * @memberof QBRPCUUID
 *
 * @throw SvCoreError_invalidArgument if failed to parse uuidString
 *
 * @param[in] uuidString    uuid string
 * @param[out] errorOut     error info
 * @return                  new QBRPCUUID object handle, or @c NULL in case of error
 */
QBRPCUUID QBRPCUUIDCreateFromString(SvString uuidString, SvErrorInfo* errorOut);

/**
 * Create QBRPCUUID object from C string.
 *
 * @memberof QBRPCUUID
 *
 * @throw SvCoreError_invalidArgument if failed to parse uuidString
 *
 * @param[in] uuidCString   uuid C string
 * @param[out] errorOut     error info
 * @return                  new QBRPCUUID object handle, or @c NULL in case of error
 */
QBRPCUUID QBRPCUUIDCreateFromCString(const char* uuidCString, SvErrorInfo* errorOut);

/**
 * Create QBRPCUUID object with random uuid.
 *
 * @memberof QBRPCUUID
 *
 * @param[out] errorOut     error info
 * @return                  new QBRPCUUID object handle, or @c NULL in case of error
 */
QBRPCUUID QBRPCUUIDCreateRandom(SvErrorInfo* errorOut);

/**
 * Get uuid string.
 *
 * @memberof QBRPCUUID
 *
 * @param[in] self          QBRPCUUID object handle
 * @param errorOut          error info
 * @return                  uuid string, or @c NULL in case of error
 */
SvString QBRPCUUIDGetString(QBRPCUUID self, SvErrorInfo* errorOut);


/**
 * Get uuid C string.
 *
 * @memberof QBRPCUUID
 *
 * @param[in] self          QBRPCUUID object handle
 * @param errorOut          error info
 * @return                  uuid C string, or @c NULL in case of error
 */
const char* QBRPCUUIDGetCString(QBRPCUUID self, SvErrorInfo* errorOut);

/**
 * @}
 */

#endif // QB_RPC_UUID_H_
