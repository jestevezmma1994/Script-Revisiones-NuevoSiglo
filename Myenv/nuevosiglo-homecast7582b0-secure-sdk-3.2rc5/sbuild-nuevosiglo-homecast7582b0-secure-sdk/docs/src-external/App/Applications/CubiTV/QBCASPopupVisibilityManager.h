/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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
#ifndef QBCASPOPUPVISIBILITYMANAGER_H_
#define QBCASPOPUPVISIBILITYMANAGER_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>

/**
 * @file QBCASPopupVisibilityManager.h
 * @brief QBCASPopupVisibilityManager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBCASPopupVisibilityManager CAS Popup Visibility Service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * @typedef QBCASPopupVisibilityManager
 * Used for blocking displaying CAS popups
 */
typedef struct QBCASPopupVisibilityManager_ *QBCASPopupVisibilityManager;

/**
 * Constructor of QBCASPopupVisibilityManager
 *
 * @param[in] casPopupManager CAS Popup manager
 * @return instance of @link QBCASPopupVisibilityManager @endlink
 */
QBCASPopupVisibilityManager QBCASPopupVisibilityManagerCreate(SvObject casPopupManager);

/**
 * Register block request
 *
 * @param[in] self CAS Popup Visibility service handle
 * @param[in] contextId id of Visibility which want to block popups or NULL when request comes not from window context
 * @param[in] windowId id of window which wants to block popups
 */
void QBCASPopupVisibilityManagerBlock(QBCASPopupVisibilityManager self, SvString contextId, SvString windowId);

/**
 * Unregister block request
 *
 * @param[in] self CAS Popup Visibility service handle
 * @param[in] contextId id of context which want to unblock popups or NULL when request comes not from window context
 * @param[in] windowId id of window which wants to unblock popups
 */
void QBCASPopupVisibilityManagerUnblock(QBCASPopupVisibilityManager self, SvString contextId, SvString windowId);

/**
 * Unregister all block requests of given context
 *
 * @param[in] self CAS Popup Visibility service handle
 * @param[in] contextId id of context which want to unblock popups or NULL when request comes not from window context
 */
void QBCASPopupVisibilityManagerUnblockAll(QBCASPopupVisibilityManager self, SvString contextId);

/**
 * @}
 **/

#endif //QBCASPOPUPVISIBILITYMANAGER_H_
