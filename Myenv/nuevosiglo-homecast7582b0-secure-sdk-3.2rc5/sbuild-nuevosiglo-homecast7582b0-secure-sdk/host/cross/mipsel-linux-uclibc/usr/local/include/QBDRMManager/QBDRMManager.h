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

#ifndef QB_DRM_MANAGER_H
#define QB_DRM_MANAGER_H

#include <QBDRMManager/QBDRMManagerInternal.h>
#include <QBDRMManager/QBDRMManagerListener.h>
#include <QBDRMManager/QBDRMInitializationData.h>
#include <QBAppKit/QBFuture.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @file QBDRMManager.h
 * @brief DRM Manager service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDRMManager DRM Manager service
 * @{
 **/

typedef struct QBDRMManager_* QBDRMManager;

/**
 * Creates DRM Manager object
 * @return new DRM Manager instance
 **/
extern QBDRMManager QBDRMManagerCreate(void);

/**
 * Add listener to DRM manager.
 *
 * @param[in] self     DRM manager handle
 * @param[in] listener listener handle
 **/
extern void QBDRMManagerAddListener(QBDRMManager self, SvObject listener);

/**
 * Remove listener from DRM manager.
 *
 * @param[in] self     DRM manager handle
 * @param[in] listener listener handle
 **/
extern void QBDRMManagerRemoveListener(QBDRMManager self, SvObject listener);

/**
 * Send notification about new DRM data.
 *
 * @param[in] self    DRM manager handle
 * @param[in] DRMData data DRM manager
 */
extern void QBDRMManagerInterfaceNotify(QBDRMManager self, const QBDRMManagerData* DRMData);

/**
 * Get last DRM manager data.
 *
 * @param[in] self DRM manager handle
 * @return         manager data
 */
extern const QBDRMManagerData* QBDRMManagerGetDRMManagerData(QBDRMManager self);

/** Get runtime type identification object representing QBDRMManager class
 * @return object representing QBDRMManager class
 */
extern SvType QBDRMManager_getType(void);

/**
 * Open new DRM session.
 *
 * @param[in] self      DRM manager handle
 * @param[out] errorOut errorInfo
 * @return  assigned sessionId
 */
extern int QBDRMManagerOpenSession(QBDRMManager self, SvErrorInfo *errorOut);

/**
 * Close DRM session.
 *
 * @param[in] selfs      DRM manager handle
 * @param[in] sessionId ID of the session to be closed
 * @param[out] errorOut error info
 * @return  @c 0 in case of success, @c -1 otherwise
 */
extern int QBDRMManagerCloseSession(QBDRMManager self, int sessionId, SvErrorInfo *errorOut);

/**
 * Set DRM initialization data.
 *
 * @param[in] self                  DRM manager handle
 * @param[in] sessionId             ID of the session for which the initialization data will be set
 * @param[in] initializationData    DRM initialization data to be set
 * @param[out] errorOut             error info
 * @return  future initialized session, @c NULL in case of error
 */
extern QBFuture QBDRMManagerSetDRMInitializationData(QBDRMManager self, int sessionId,
                                                     QBDRMInitializationData initializationData,
                                                     SvErrorInfo *errorOut);

/**
 * Get DRM session manager that manages specified session.
 *
 * @param[in] self      DRM manager handle
 * @param[in] sessionId ID of the session to which the DRM session manager is assigned to
 * @param[out] errorOut error info
 * @return  DRM session manager that manages specified session, @c NULL otherwise
 */
extern SvObject QBDRMManagerGetDRMSessionManager(QBDRMManager self, int sessionId, SvErrorInfo *errorOut);

/**
 * Get DRM system type of specified session.
 *
 * @param[in] self      DRM manager handle
 * @param[in] sessionId ID of the session for which DRM system type will be returned
 * @param[out] errorOut error info
 * @return  DRM system type of specified session
 */
extern QBDRMSystemType QBDRMManagerGetDRMSystemType(QBDRMManager self, int sessionId, SvErrorInfo *errorOut);

/**
 * Register DRM session manager for specified DRM system type.
 * If already registered, the function will overwrite current session manager
 * for the given DRM system type.
 *
 * @param[in] self                  DRM manager handle
 * @param[in] systemType            DRM system type that will be managed by the DRM session manager
 * @param[in] sessionManager        DRM session manager to be registered
 * @param[out] errorOut             error info
 * @return  @c 0 in case of success, @c -1 otherwise
 */
extern int QBDRMManagerRegisterDRMSessionManager(QBDRMManager self, QBDRMSystemType systemType,
                                                 SvObject sessionManager, SvErrorInfo *errorOut);

/**
 * Deregister DRM session manager.
 *
 * @param[in] self                  DRM manager handle
 * @param[in] sessionManager        DRM session manager to be deregistered
 * @param[out] errorOut             error info
 * @return  @c 0 in case of success, @c -1 otherwise
 */
extern int QBDRMManagerDeregisterDRMSessionManager(QBDRMManager self, SvObject sessionManager, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_DRM_MANAGER_H
