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

#ifndef QB_UPGRADE_CATALOG_H_
#define QB_UPGRADE_CATALOG_H_

/**
 * @file QBUpgradeCatalog.h
 * @brief Upgrade catalog class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBUpgradeCatalog Upgrade catalog class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>
#include <SvFoundation/SvImmutableArray.h>


/**
 * Upgrade catalog class.
 * @class QBUpgradeCatalog
 * @extends SvObject
 **/
typedef struct QBUpgradeCatalog_ *QBUpgradeCatalog;


/**
 * Get runtime type identification object
 * representing QBUpgradeCatalog class.
 *
 * @return QBUpgradeCatalog type identification object
 **/
extern SvType
QBUpgradeCatalog_getType(void);

/**
 * Create upgrade catalog object from XML representation stored in file.
 *
 * @memberof QBUpgradeCatalog
 *
 * @param[in] filePath  path to the file containing upgrade catalog XML document
 * @param[out] errorOut error info
 * @return              created upgrade catalog, @c NULL in case of error
 **/
extern QBUpgradeCatalog
QBUpgradeCatalogCreateFromFile(const char *filePath,
                               SvErrorInfo *errorOut);

/**
 * Get MD5 checksum of the upgrade catalog.
 *
 * @memberof QBUpgradeCatalog
 *
 * @param[in] self      upgrade catalog handle
 * @return              MD5 checksum of the upgrade catalog
 *                      in hexadecimal textual representation,
 *                      @c NULL in case of error
 **/
extern SvString
QBUpgradeCatalogGetHash(QBUpgradeCatalog self);

/**
 * Get array of upgrade objects contained in the catalog.
 *
 * @memberof QBUpgradeCatalog
 *
 * @param[in] self      upgrade catalog handle
 * @return              an array of upgrade objects
 *                      (instances of @ref QBUpgradeObject),
 *                      @c NULL in case of error
 **/
extern SvImmutableArray
QBUpgradeCatalogGetObjects(QBUpgradeCatalog self);

/**
 * Get in-memory XML representation of the upgrade catalog.
 *
 * @memberof QBUpgradeCatalog
 *
 * @param[in] self      upgrade catalog handle
 * @return              XML document, @c NULL if not available
 **/
extern SvData
QBUpgradeCatalogGetRawVersion(QBUpgradeCatalog self);

/**
 * Write XML representation of the upgrade catalog to
 * 'catalog.xml' file in specified directory.
 *
 * @memberof QBUpgradeCatalog
 *
 * @param[in] self      upgrade catalog handle
 * @param[in] dirPath   path to a directory to create 'catalog.xml' file in
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeCatalogSaveToDirectory(QBUpgradeCatalog self,
                                const char *dirPath,
                                SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
