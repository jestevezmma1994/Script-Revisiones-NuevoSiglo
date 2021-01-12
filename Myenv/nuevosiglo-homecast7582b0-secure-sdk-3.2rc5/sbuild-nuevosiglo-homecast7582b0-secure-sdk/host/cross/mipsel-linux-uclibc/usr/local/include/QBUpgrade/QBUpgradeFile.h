/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_UPGRADE_FILE_H_
#define QB_UPGRADE_FILE_H_

/**
 * @file QBUpgradeFile.h
 * @brief Upgrade file class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBUpgradeFile Upgrade file class
 * @ingroup QBUpgrade
 * @{
 **/

#include <sys/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>


/**
 * Upgrade file class.
 * @class QBUpgradeFile
 * @extends SvObject
 **/
typedef struct QBUpgradeFile_ *QBUpgradeFile;


/**
 * Get runtime type identification object
 * representing QBUpgradeFile class.
 *
 * @return QBUpgradeFile type identification object
 **/
extern SvType
QBUpgradeFile_getType(void);

/**
 * Get relative path of the upgrade file.
 *
 * @memberof QBUpgradeFile
 *
 * @param[in] self      upgrade file handle
 * @return              upgrade file path relative to top-level
 *                      directory of the upgrade object
 **/
extern SvString
QBUpgradeFileGetName(QBUpgradeFile self);

/**
 * Get size of the upgrade file.
 *
 * @memberof QBUpgradeFile
 *
 * @param[in] self      upgrade file handle
 * @return              size of the upgrade file in bytes
 **/
extern off_t
QBUpgradeFileGetSize(QBUpgradeFile self);

/**
 * Get MD5 checksum of the upgrade file.
 *
 * @memberof QBUpgradeFile
 *
 * @param[in] self      upgrade file handle
 * @return              MD5 checksum of the upgrade file
 *                      in hexadecimal textual representation
 **/
extern SvString
QBUpgradeFileGetHash(QBUpgradeFile self);

/**
 * @}
 **/


#endif
