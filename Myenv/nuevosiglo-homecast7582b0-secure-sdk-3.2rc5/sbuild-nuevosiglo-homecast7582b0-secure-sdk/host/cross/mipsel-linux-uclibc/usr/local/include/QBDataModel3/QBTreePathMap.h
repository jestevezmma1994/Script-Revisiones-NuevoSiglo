/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TREE_PATH_MAP_H_
#define QB_TREE_PATH_MAP_H_

/**
 * @file QBTreePathMap.h Tree path map class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTreePathMap Tree path map class
 * @ingroup QBDataModel3
 * @{
 *
 * A utility container class that manages a map between
 * tree paths (used as keys) and any object used as values.
 **/

/**
 * Tree path map class.
 * @class QBTreePathMap
 * @extends SvObject
 **/
typedef struct QBTreePathMap_ *QBTreePathMap;


/**
 * Get runtime type identification object representing
 * type of tree path map class.
 *
 * @return QBTreePathMap type identification object
 **/
extern SvType
QBTreePathMap_getType(void);

/**
 * Create new tree path map.
 *
 * @memberof QBTreePathMap
 *
 * @param[out] errorOut     error info
 * @return                  created map, @c NULL in case of error
 **/
extern QBTreePathMap
QBTreePathMapCreate(SvErrorInfo *errorOut);

/**
 * Search for a value in tree path map.
 *
 * @memberof QBTreePathMap
 *
 * @param[in] self          path map object handle
 * @param[in] path          tree path handle
 * @return                  value associated with @a path, @c NULL if not found
 **/
extern SvObject
QBTreePathMapFind(QBTreePathMap self,
                  SvObject path);

/**
 * Add new key/value pair to the map.
 *
 * @memberof QBTreePathMap
 *
 * @param[in] self          path map object handle
 * @param[in] path          tree path handle
 * @param[in] value         value to associate with @a path
 * @param[out] errorOut     error info
 **/
extern void
QBTreePathMapInsert(QBTreePathMap self,
                    SvObject path,
                    SvObject value,
                    SvErrorInfo *errorOut);

/**
 * Remove key/value pair from the map.
 *
 * @memberof QBTreePathMap
 *
 * @param[in] self          path map object handle
 * @param[in] path          tree path handle
 * @param[out] errorOut     error info
 **/
extern void
QBTreePathMapRemove(QBTreePathMap self,
                    SvObject path,
                    SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
