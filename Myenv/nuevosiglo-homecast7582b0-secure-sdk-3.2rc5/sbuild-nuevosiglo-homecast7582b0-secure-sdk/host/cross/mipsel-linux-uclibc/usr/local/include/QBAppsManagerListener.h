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
#ifndef QBAPPSMANAGERLISTENER_H_
#define QBAPPSMANAGERLISTENER_H_

/**
 * @file QBAppsManagerListener.h
 * @brief QBAppsManagerListener class enables listening to changes in QAppsManager.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAppsManagerListener class.
 * @ingroup QBAppsManager
 * @{
 **/

#include <SvFoundation/SvObject.h>
#include <QBApp.h>

/**
  QBAppFinishType describes what way app was closed.
  */
typedef enum {
    QBAppFinishType_SelfClose = 0, /**< application closed without quit call */
    QBAppFinishType_RequestedClose = 1, /**< application closed due to quit call */
    QBAppFinishType_Killed = 2, /**< application was killed */
    QBAppFinishType_Crash = 3, /**< application crashed */
    QBAppFinishType_Count /**< count of possible app close reasons */
} QBAppFinishType;

/**
 * Function returns QBAppsManagerListener interface instance handle.
 *
 * @return     QBAppsManagerListener interface handle
 */
SvInterface
QBAppsManagerListener_getInterface(void);

/**
 * Interface for listening to service changes in QBAppsManagerListener.
 * @class QBAppsManagerListener
 **/
struct QBAppsManagerListener_ {
    /**
     * Notification about application initialization.
     * New QBApp instance has been created and initialized.
     *
     * @param[in] self_              listener handler
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*applicationInitialized)(SvObject self_, QBApp application);
    /**
     * Notification about started application.
     * Communication with process associated with QBApp handle is now available.
     *
     * @param[in] self_              listener handler
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*applicationStarted)(SvObject self_, QBApp application);
    /**
     * Notification about finished application
     *
     * @param[in] self_              listener handler
     * @param[in] application        QBApp handler
     * @param[in] finishType         how application was finished
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*applicationFinished)(SvObject self_, QBApp application, QBAppFinishType finishType);
    /**
     * Notification about restarted application
     *
     * @param[in] self_              listener handler
     * @param[in] application        QBApp handler
     * @param[in] restartReason      why application was restarted
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*applicationRestarted)(SvObject self_, QBApp application, QBAppFinishType restartReason);
};

typedef struct QBAppsManagerListener_* QBAppsManagerListener;

/**
 * @}
 **/
#endif /* QBAPPSMANAGERLISTENER_H_ */
