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
#ifndef QB_DIAL_SERVER_APP_H
#define QB_DIAL_SERVER_APP_H

#include <QBApp.h>
#include <IDIAL.h>
#include <IDIALListener.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvType.h>

/**
 * @file QBDIALServerApp.h
 * @brief QBDIALServerApp class provides interface for setting DIAL server application specific parameters.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDIALServerApp class.
 * @ingroup QBAppsManager
 * @{
 **/

/**
 * @brief Defines target application launch type.
 */
typedef enum {
    QBDIALServerApplicationLaunchType_Unknown = 0, //!< default, unknown launch type
    QBDIALServerApplicationLaunchType_Native,      //!< app is launched as native app
    QBDIALServerApplicationLaunchType_WebBrowser   //!< app is launched as web browser app
} QBDIALServerApplicationLaunchType;

/**
 * @brief Defines target application state.
 */
typedef enum {
    QBDIALServerApplicationState_Unknown = 0, //!< default, unknown state
    QBDIALServerApplicationState_Stopped,     //!< application is stopped
    QBDIALServerApplicationState_Running      //!< application is running
} QBDIALServerApplicationState;

/**
 * @brief Structure containing app's DIAL server params.
 */
typedef struct QBDIALServerParams_s {
    SvString networkInterfaceId; //!< network interface id
    uint32_t dialPort;           //!< DIAL server port
    uint32_t ssdpPort;           //!< SSDP server port
    SvString friendlyName;       //!< DIAL server presents itself under this name in network
    SvString modelName;          //!< DIAL server model name
    SvString uuid;               //!< DIAL server uuid
} QBDIALServerParams;

/**
 * @brief Structure containing target application's start params.
 */
typedef struct QBDIALServerApplicationStartParams_s {
    SvString name;              //!< application name
    SvString payload;           //!< application start command payload
    SvString additionalDataUrl; //!< url used to pass additional application data
} QBDIALServerApplicationStartParams;

/**
 * @brief Structure containing params of supported application.
 */
typedef struct QBDIALApplication_s {
    SvString name;              //!< application name
    SvString corsAllowedOrigin; //!< Cross-Origin Resource Sharing allowed domain name
    bool useAdditionalData;     //!< additional application data usage flag
    QBDIALServerApplicationLaunchType type; //!< application launch type
    SvHashTable params; //!< application's internal params
} QBDIALApplication;

/**
 * @brief Structure containing list of supported applications.
 */
typedef struct QBDIALServerApps_s {
    QBDIALApplication *elements; //!< application descriptors
    unsigned int count;          //!< number of available applications
} QBDIALServerApps;

/**
 * @brief QBDIALServerApp is a class derived from QBApp.
 * It provides an interface for controlling a DIAL server application.
 */
typedef struct QBDIALServerApp_ *QBDIALServerApp;

/**
 * @brief QBDIALServerAppFactory is a class used to create QBDIALServerApp objects.
 */
typedef struct QBDIALServerAppFactory_ *QBDIALServerAppFactory;

/**
 * @brief This function is used to notify DIAL server clients
 * that target application state has changed.
 *
 * @param[in] self               DIAL server app handle
 * @param[in] targetAppName      target application name
 * @param[in] targetAppState     target application state
 */
void
QBDIALServerAppTargetAppStateChanged(QBDIALServerApp self, SvString targetAppName, QBDIALServerApplicationState targetAppState);


/**
 * @brief This function is used to create url with additionalData
 * for given application name.
 *
 * @param[in]  self               DIAL server app handle
 * @param[in]  url                url that is to be expanded with additionalData
 * @param[in]  appName            target application name
 *
 * @return     created string containing url with additionalData
 */
SvString
QBDIALServerAppCreateUrlWithAdditionalData(QBDIALServerApp self, SvString url, SvString appName);

/**
 * @brief This function is used to create QBDIALServerAppFactory instances.
 *
 * @return          created QBDIALServerAppFactory handle
 */
QBDIALServerAppFactory
QBDIALServerAppFactoryCreate(void);

/**
 * @brief Function returns handle to QBDIALServerAppListener interface.
 *
 * @return          QBDIALServerAppListener interface handle
 */
SvInterface
QBDIALServerAppListener_getInterface(void);

struct QBDIALServerAppListener_ {
    /**
     * Request to start an application.
     *
     * @param[in] self               self (listener) handle
     * @param[in] app                DIAL server app handle
     * @param[out] params            DIAL server params
     *
     * @return                       @a 0 in case of success,
     *                               @c -1 otherwise
     **/
    int (*getServerParams)(SvObject self_, QBApp app, QBDIALServerParams *params);

    /**
     * Get list of registered application.
     *
     * @param[in] self              self (listener) handle
     * @param[in] app               DIAL server app handle
     * @param[out] apps             registered applications
     *
     * @return                      @a 0 in case of success,
     *                              @c -1 otherwise
     **/
    int (*getRegisteredApplications)(SvObject self_, QBApp app, QBDIALServerApps *apps);

    /**
     * Request to start an application.
     *
     * @param[in] self               self (listener) handle
     * @param[in] params             application params
     *
     * @return                       @a 0 in case of success,
     *                               @c -1 otherwise
     **/
    int (*startApplication)(SvObject self_, QBDIALServerApplicationStartParams params);

    /**
     * Request to stop an application.
     *
     * @param[in] self               self (listener) handle
     * @param[in] appName            application name
     *
     * @return                       @a 0 in case of success,
     *                               @c -1 otherwise
     **/
    int (*stopApplication)(SvObject self_, SvString appName);
};
typedef struct QBDIALServerAppListener_ *QBDIALServerAppListener;

/**
 * @}
 **/
#endif // QB_DIAL_SERVER_APP_H
