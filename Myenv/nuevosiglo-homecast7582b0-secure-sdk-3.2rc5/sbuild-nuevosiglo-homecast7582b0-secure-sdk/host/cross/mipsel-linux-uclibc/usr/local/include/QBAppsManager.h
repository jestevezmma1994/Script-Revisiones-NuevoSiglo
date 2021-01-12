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
#ifndef QBAPPSMANAGER_H_
#define QBAPPSMANAGER_H_

/**
 * @file QBAppsManager.h
 * @brief QBAppsManager class enables launching and managing external apps.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAppsManager class.
 * @{
 **/

#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <QBApp.h>

typedef struct QBAppsManager_t *QBAppsManager;

/**
 * Function adds new listener to QBAppsManager. Listener
 * will be notified about any changes in QBAppsManager.
 *
 * @param self          QBAppsManager handle
 * @param listener      listener object implementing QBAppsManagerListener
 *                      interface handle
 */
void
QBAppsManagerAddListener(QBAppsManager self,
                         SvObject listener);

/**
 * Function removes listener from QBAppsManager's listeners list.
 *
 * @param self          QBAppsManager handle
 * @param listener      listener object handle
 */
void
QBAppsManagerRemoveListener(QBAppsManager self, SvObject listener);

/**
 * Create new apps manager instance.
 * Apps manager service allows launching different applications
 * such as a web browser. Each app has to be defined in apps whitelist.
 * Apps manager uses QBShellCmds to launch apps so, if shell cmds whitelist
 * is present in project, it has to be in sync with apps whitelist.
 *
 * @return               @a self or @c NULL in case of error
 **/
QBAppsManager
QBAppsManagerCreate(void);

/**
 * Start apps manager service.
 *
 * @param[in] self       apps manager handle
 * @return               true if apps manager started successfully,
 *                       false otherwise
 **/
bool QBAppsManagerStart(QBAppsManager self);

/**
 * Stop apps manager service.
 *
 * @param[in] self       apps manager handle
 **/
void
QBAppsManagerStop(QBAppsManager self);

/**
 * @brief This function is used to register app (name, type and factory - object
 * used to create instances of given app) in QBAppsManager.
 *
 * It is required to register app in QBAppsManager before launching app.
 *
 * @param[in] self          QBAppsManager handle
 * @param[in] appName       app's name
 * @param[in] factory       factory object handle
 * @return                  true if app has been registered correctyle, false otherwise
 */
bool
QBAppsManagerRegisterApp(QBAppsManager self, SvString appName, SvObject factory);

/**
 * @brief This function unregisteres app from QBAppsManager. That kind of app
 * cannot be created unless registered again.
 *
 * @param self              QBAppsManager handle
 * @param appName           app's name
 * @return                  true if unregistered successfully, false otherwise
 */
bool
QBAppsManagerUnregisterApp(QBAppsManager self, SvString appName);

/**
 * Launch an app.
 * If appName is not specified, apps manager will try to launch
 * first app that matches given appType.
 *
 * @param[in] self       apps manager handle
 * @param[in] appName    app name
 * @param[in] appType    app type
 * @return               @a app id or @c -1 in case of error
 **/
ssize_t
QBAppsManagerLaunchApp(QBAppsManager self,
                       SvString appName,
                       QBAppType appType);

/**
 * Close an app using RPC's quit call.
 *
 * @param[in] self       apps manager handle
 * @param[in] app        handle to application that should be closed
 * @return               true if successful, false otherwise
 **/
bool
QBAppsManagerCloseApp(QBAppsManager self,
                      QBApp app);

/**
 * @brief This function can be used to get app name by app type.
 * First app's name that matches app type is returned.
 *
 * @param[in] self          QBAppsManager handle
 * @param[in] appType       app type
 * @return                  @c NULL if no app was found, app name otherwise
 */
SvString
QBAppsManagerGetAppNameByType(QBAppsManager self, QBAppType appType);

/**
 * @brief This function can be used to get app handle by app type.
 * First app's handle that matches app type is returned.
 *
 * @param[in] self          QBAppsManager handle
 * @param[in] appType       app type
 * @return                  @c NULL if no app was found, app handle otherwise
 */
QBApp
QBAppsManagerGetFirstAppByType(QBAppsManager self, QBAppType appType);

/**
 * @brief This function gets app handle by app id.
 *
 * @param[in] self          QBAppsManager handle
 * @param[in] appId         app id
 * @return                  @c NULL if no app was found, app handle otherwise
 */
QBApp
QBAppsManagerGetAppById(QBAppsManager self, size_t appId);

/**
 * @}
 **/
#endif /* QBAPPSMANAGER_H_ */
