/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_GLOBAL_STORAGE_H_
#define QB_GLOBAL_STORAGE_H_

/**
 * @file QBGlobalStorage.h
 * @brief Global storage class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBGlobalStorage Global storage class
 * @ingroup QBAppKit
 * @{
 *
 * @link QBGlobalStorage @endlink provides external storage for items.
 * Objects are registered under given SvType id.
 **/

/**
 * Global storage singleton class.
 * @class QBGlobalStorage QBGlobalStorage.h <QBAppKit/QBGlobalStorage.h>
 * @extends SvObject
 **/
typedef struct QBGlobalStorage_ *QBGlobalStorage;

/**
 * Get handle to the global storage singleton.
 *
 * @return                      global storage handle
 **/
extern QBGlobalStorage
QBGlobalStorageGetInstance(void);

/**
 * Set item with given id.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] id                id of item to add
 * @param[in] item              item to add
 * @param[out] errorOut         error info
 **/
void
QBGlobalStorageSetItem(QBGlobalStorage self,
                       SvType id,
                       SvObject item,
                       SvErrorInfo *errorOut);

/**
 * Remove item with given id.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] id                id of extension to remove
 * @param[out] errorOut         error info
 **/
void
QBGlobalStorageRemoveItem(QBGlobalStorage self,
                          SvType id,
                          SvErrorInfo *errorOut);

/**
 * Get item with given id.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] id                id of extension to get
 * @param[out] errorOut         error info
 * @return                      extension with given id,
 *                              @c NULL if not found
 **/
SvObject
QBGlobalStorageGetItem(QBGlobalStorage self,
                       SvType id,
                       SvErrorInfo *errorOut);

/**
 * Set item by name.
 * Item is not set if another item is present under given name.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] name              name under which item will be set
 * @param[in] item              item to be added
 * @param[out] errorOut         error info
 **/
void
QBGlobalStorageSetItemByName(QBGlobalStorage self,
                             SvString name,
                             SvObject item,
                             SvErrorInfo *errorOut);

/**
 * Remove item set with given name.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] name              name under which item is set
 * @param[out] errorOut         error info
 **/
void
QBGlobalStorageRemoveItemByName(QBGlobalStorage self,
                                SvString name,
                                SvErrorInfo *errorOut);

/**
 * Get item with given name.
 *
 * @memberof QBGlobalStorage
 *
 * @param[in] self              global storage handle
 * @param[in] name              name under which item was set
 * @param[out] errorOut         error info
 * @return                      item with given name,
 *                              @c NULL if not found
 **/
SvObject
QBGlobalStorageGetItemByName(QBGlobalStorage self,
                             SvString name,
                             SvErrorInfo *errorOut);

/**
 * Clear global storage.
 *
 * @memberof QBGlobalStorage
 *
 * This method removes all items kept in global storage.
 *
 * @param[in] self              global storage handle
 * @param[out] errorOut         error info
 **/
void
QBGlobalStorageClear(QBGlobalStorage self,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_GLOBAL_STORAGE_H_ */
