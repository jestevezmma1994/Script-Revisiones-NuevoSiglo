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

#ifndef QBWEBSHELLCONTEXT_H_
#define QBWEBSHELLCONTEXT_H_


#include <QBWindowContext.h>
#include <QBAppsManager.h>
#include <Services/core/GlobalPlayer.h>
#include <QBViewport.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>

/**
 * @file QBWebshellContext.h
 * @brief QBWebshellContext is context displaying and managing web browser application.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBWebshellContext class.
 * @{
 **/

/**
 * This structure is used to pass global objects that are used by context, and normally are kept in AppGlobals
 */
struct QBWebshellContextGlobalParams_ {
    SvApplication app; ///< application handle
    QBApplicationController controller; ///< allplication controller handle
    QBAppsManager appsManager; ///< apps manager handle
    QBViewport viewport; ///< viewport handle
    QBGlobalPlayerController playerController; ///< player controller handle
    SvScheduler scheduler; ///< scheduler handle
};
typedef struct QBWebshellContextGlobalParams_ *QBWebshellContextGlobalParams;

/**
 * This structure is used for storaging QBWebShellContext parameters associated with web browser application.
 */
struct QBWebshellContextParams_s {
    SvString url;   /**< Url to web page that should be opened when browser app starts. */
    SvString customUserAgent; /**< Web browser application will be recognised as customUserAgent str. */
    bool spatialNavigation; /**< Flag indicating using spatial navigation. */
    bool remoteDebug; /**< Enables remote debugging. */
    int remoteDebugPort; /**< Remote debugging port. */
    SvString appName; /**< Web browser application name. */
    bool handlePopups; /**< If set to false, web browser popups will not be displayed. */
};
typedef struct QBWebshellContextParams_s *QBWebshellContextParams;

/**
 * This function creates QBWebShellContext instance.
 *
 * @param[in] globals           pointer to structure with global params of app
 * @param[in] params            structure with web browser params
 * @param[out] errorOut         error handle
 *
 * @return                      QBWebShellContext handle
 */
QBWindowContext QBWebshellContextCreate(QBWebshellContextGlobalParams globals, QBWebshellContextParams params, SvErrorInfo *errorOut);

/**
 * This function closes QBWebShellContext.
 *
 * @param[in] ctx_              QBWebShellContext handle
 */
void QBWebshellContextClose(QBWindowContext ctx_);

/**
 * This function initializes QBWebShellContext params using
 * web browser menu node handle.
 *
 * @param[in] params            params handle
 * @param[in] node              web browser menu node handle
 * @param[in] appsManager       apps manager handle
 * @param[in] appName           web browser application name
 *
 * @return                      @c -1 in case of error, @c 0 otherwise
 */
int QBWebshellContextParamsInit(QBWebshellContextParams params, SvObject node, QBAppsManager appsManager, SvString appName);

/**
 * This function adds listener of QBWebshellContextListener interface.
 *
 * @param[in] self             QBWebShellContext handle
 * @param[in] listener         listener to be added
 */
void QBWebshellContextAddListener(QBWindowContext self, SvObject listener);

/**
 * This function removes listener of QBWebshellContextListener interface.
 *
 * @param[in] self             QBWebShellContext handle
 * @param[in] listener         listener to be removed
 */
void QBWebshellContextRemoveListener(QBWindowContext self, SvObject listener);

/**
 * Function returns QBWebshellContextListener interface instance handle.
 *
 * @return     QBWebshellContextListener interface handle
 */
SvInterface QBWebshellContextListener_getInterface(void);

struct QBWebshellContextListener_ {
    /**
     * Notification about webshell context start.
     * New QBApp instance has been created and initialized.
     *
     * @param[in] self_              listener handler
     * @param[in] ctx                window context
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method, @c positive value in case of handled method
     *
     **/
    unsigned int (*webshellContextStarted)(SvObject self_, QBWindowContext ctx, QBApp application);
};

typedef struct QBWebshellContextListener_ *QBWebshellContextListener;

/**
 * @}
 **/
#endif /* QBBROWSERCONTEXT_H_ */
