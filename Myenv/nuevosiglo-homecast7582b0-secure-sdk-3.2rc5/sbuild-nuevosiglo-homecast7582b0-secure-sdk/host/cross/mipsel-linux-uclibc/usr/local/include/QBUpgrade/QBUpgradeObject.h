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

#ifndef QB_UPGRADE_OBJECT_H_
#define QB_UPGRADE_OBJECT_H_

/**
 * @file QBUpgradeObject.h
 * @brief Upgrade object class
 **/

/**
 * @defgroup QBUpgradeObject Upgrade object class
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvImmutableArray.h>

/**
 * Upgrade object class.
 * @class QBUpgradeObject
 * @extends SvObject
 **/
typedef struct QBUpgradeObject_ *QBUpgradeObject;


/**
 * Get runtime type identification object
 * representing QBUpgradeObject class.
 *
 * @return QBUpgradeObject type identification object
 **/
extern SvType
QBUpgradeObject_getType(void);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeObjectGetFullName(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeObjectGetVendorName(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeObjectGetBoardName(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeObjectGetProjectName(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeObjectGetVersion(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern bool
QBUpgradeObjectIsForced(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return              FIXME
 **/
extern SvImmutableArray
QBUpgradeObjectGetFiles(QBUpgradeObject self);

/**
 * FIXME
 *
 * @memberof QBUpgradeObject
 *
 * @param[in] self      upgrade object handle
 * @return FIXME
 **/
extern SvURI
QBUpgradeObjectGetOrigin(QBUpgradeObject self);

/**
 * @}
 **/


#endif
