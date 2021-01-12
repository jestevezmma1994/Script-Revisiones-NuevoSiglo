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

#ifndef QB_SHARED_CONTENT_PROTECTION_H
#define QB_SHARED_CONTENT_PROTECTION_H

/**
 * @file QBSharedContentProtection.h
 * @brief Wrapper for sv_content_protection structure.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBSharedContentProtection QBSharedContentProtection.h
 * @{
 */

#include "content_protection.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SvObject wrapper for sv_content_protection structure.
 */
typedef struct QBSharedContentProtection_ *QBSharedContentProtection;

/**
 * @brief QBSharedContentProtectionCreate Create new SvObject with content protection data.
 *
 * Wrapper makes a deep copy of cp pointer so there is no need to keep it externally.
 *
 * @param[in] cp data to be wrapped
 * @return content protection SvObject
 */
QBSharedContentProtection QBSharedContentProtectionCreate(struct sv_content_protection const * const cp);

/**
 * @brief QBSharedContentProtectionGetData get sv_content_protection data.
 *
 * Returned pointer is valid until self object is destroyed.
 *
 * @param[in] self self object
 * @return data pointer to data included in object
 */
struct sv_content_protection* QBSharedContentProtectionGetData(QBSharedContentProtection self);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // QB_SHARED_CONTENT_PROTECTION_H
