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
#ifndef QBAPP_H_
#define QBAPP_H_

/**
 * @file QBApp.h
 * @brief QBApp class representing external application launched by QBAppsManager.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBApp class.
 * @ingroup QBAppsManager
 * @{
 **/

#include <QBRPC/QBRPCUUID.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <fibers/c/fibers.h>

/**
 * QBApp class represents external application launched by QBAppsManager.
 * It enables basic operations like setting/getting name, getting pid etc.
 * as well as getting remote service handle and calling remote procedures
 * using IDL defined interfaces.
 */
typedef struct QBApp_ *QBApp;

/**
 * App types list.
 **/
typedef enum {
    QBAppType_WebBrowser = 0, /**< web browser app type */
    QBAppType_DIALServer = 1, /**< DIAL server app type */
    QBAppType_Count /**< possible app types count */
} QBAppType;

/**
 * QBAppInitParams structure is used for all QBApps basic initialization.
 */
typedef struct {
    SvString appName;   /**< application's name, used to find apps descriptor */
    QBAppType appType; /**< applications type, required if name hasn't been found */
    size_t appId; /**< application's id */
    QBRPCUUID uuid; /**< uuid used by external app to connect */
    SvString sharedDirPath; /**< path to the shared directory, which application can use to share resources with CubiTV */
    SvScheduler scheduler; /**< scheduler managing QBApp's fibers */
    int launchTimeout; /**< external app has that many seconds to connect */
    int monitoringTimeout; /**< if external app doesn't respond in that time it's considered disconnected */
    int monitoringInterval; /**< how frequent external app is polled if it's still working */
    int closeTimeout; /**< external app has that many seconds to close properly before being killed */
} QBAppInitParams;

/**
 * Get runtime type identification object representing QBApp class
 *
 * @return     object representing QBApp class
 */
SvType
QBApp_getType(void);

/**
 * Method returns hash table with names of services (keys, SvString) and
 * arrays of strings listing interfaces (SvArray of SvStrings)implemented
 * by given QBApp.
 *
 * @param [in] self     QBApp handle
 *
 * @return              hash table with services and implemented interfaces
 */
SvHashTable
QBAppGetImplementedInterfaces(QBApp self);

/**
 * @brief This function gets QBApp's name.
 *
 * @param[in] self      QBApp handle
 * @return              QBApp's name
 */
SvString
QBAppGetName(QBApp self);

/**
 * @brief This function gets QBApp's pid.
 *
 * @param[in] self      QBApp handle
 * @return              QBApp's pid
 */
pid_t
QBAppGetPid(QBApp self);

/**
 * @brief This function gets QBApp's type.
 *
 * @param[in] self      QBApp handle
 * @return              QBApp's type
 */
QBAppType
QBAppGetType(QBApp self);

/**
 * @brief This function gets QBApp's id.
 *
 * @param[in] self      QBApp handle
 * @return              Identifier number given by QBAppsManager.
 */
size_t
QBAppGetId(QBApp self);

/**
 * @brief This function gets C-string representation of QBAppType.
 *
 * @param[in] appType   appType for which C-string representation
 *                      should be returned
 * @return              C-string representing appType
 */
const char*
QBAppGetTypeName(QBAppType appType);

/**
 * @brief This function gets uuid of QBApp's local rpc endpoint.
 *
 * @param[in] self      QBApp handle
 * @return              uuid of QBApp's local rpc endpoint
 */
QBRPCUUID
QBAppGetUUID(QBApp self);

/**
 * @brief This function closes app.
 *
 * @param[in] self      QBApp handle
 *
 * @return              @c false if failed, true otherwise
 */
bool
QBAppQuit(QBApp self);

/**
 * @brief Function adds new listener to QBApp. Listener
 * will be notified about any changes in interfaces
 * implemented by given type of QBApp.
 *
 * @param self          QBApp handle
 * @param listener      Listener handle
 */
void
QBAppAddListener(QBApp self, SvObject listener);

/**
 * @brief Get runtime type identification object representing QBAppFactory interface.
 *
 * @return      QBAppFactory interface object
 */
SvInterface
QBAppFactory_getInterface(void);

/**
 * @brief QBAppFactory class creates QBApp instance.
 */
typedef const struct QBAppFactory_ {
    /**
     * @brief This function creates QBApp instance.
     *
     * @param[in] self                   QBAppFactory handle
     * @param[in] params                 initialization parameters
     * @return                           created QBApp handle
     */
    QBApp (*create)(SvObject self_, QBAppInitParams params);
} *QBAppFactory;

/**
 * @}
 **/
#endif /* QBAPP_H_ */
