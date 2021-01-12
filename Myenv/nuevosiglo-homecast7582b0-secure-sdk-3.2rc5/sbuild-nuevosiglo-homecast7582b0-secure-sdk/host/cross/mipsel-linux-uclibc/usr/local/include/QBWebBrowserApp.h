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
#ifndef QBWEBBROWSERAPP_H
#define QBWEBBROWSERAPP_H

#include <QBApp.h>
#include <IWebBrowser.h>
#include <IWebBrowserListener.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <stdint.h>

/**
 * @file QBWebBrowserApp.h
 * @brief QBWebBrowserApp class provides interface for setting browser applications specific parameters.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBWebBrowserApp class.
 * @ingroup QBAppsManager
 * @{
 **/

/**
  * @brief Popup handling policy.
  *
  * TODO add proper documentation once popuphandlingpolicy is actually used
  */
typedef enum {
    QBWebBrowserAppPopupHandlingPolicy_AcceptAll = 0, /**< accept all */
    QBWebBrowserAppPopupHandlingPolicy_CancelAll, /**< cancel all */
    QBWebBrowserAppPopupHandlingPolicy_HandleAll, /**< handle all */
} QBWebBrowserAppPopupHandlingPolicy;

/**
 * @brief Structure containing app's web browser params.
 */
typedef struct {
    SvString address; /**< starting url adress */
    SvString userAgent; /**< web services will recognize app under this name */
    bool spatialNavigation; /**< flag indicating if spatial navigation is used */
    bool enableUserJS; /**<  flag indicatiing if User JavaScript should be loaded */
    bool remoteDebug; /**< enables remote debugging */
    uint16_t remoteDebugPort; /**< remote debugging port */
    QBWebBrowserAppPopupHandlingPolicy popupHandlingPolicy; /**< way of handling popups */
} QBWebBrowserAppParams;

/**
  * @brief Possible states of web browser application.
  */
typedef enum {
    QBWebBrowserAppState_Idle = 0, /**< WebBrowser is doing nothing */
    QBWebBrowserAppState_StartedLoadingNewPage, /**< WebBrowser is loading new web page */
    QBWebBrowserAppState_FinishedLoadingNewPage, /**< WebBrowser has finished loading web page */
} QBWebBrowserAppState;

/**
  * @brief Buttons used in popups requested by web browser.
  */
typedef enum {
    QBWebBrowserAppPopupButton_OK = 0, /**< OK button */
    QBWebBrowserAppPopupButton_Cancel, /**< Cancel button */
    QBWebBrowserAppPopupButton_Open, /**< Open button */
    QBWebBrowserAppPopupButton_Save, /**< Save button */
    QBWebBrowserAppPopupButton_Yes, /**< Yes button */
    QBWebBrowserAppPopupButton_No, /**< No button */
    QBWebBrowserAppPopupButton_Install, /**< Install button */
    QBWebBrowserAppPopupButton_Delete, /**< Install button */
} QBWebBrowserAppPopupButton;

/**
  * @brief Data used to create popup requested by web browser.
  */
typedef struct {
    int16_t id; /**< popup's identity number */
    SvString title; /**< popup's name */
    SvString message; /**< message that should be displayed in popup */
    int16_t buttonsMask; /**< "code" of buttons that should be displayed */
} QBWebBrowserAppPopupData;

/**
  * @brief Action taken by user on popup.
  */
typedef struct {
    int16_t id; /**< popup's identity number */
    QBWebBrowserAppPopupButton selectedButton; /**< which button user has selected */
} QBWebBrowserAppPopupActionData;

/**
 * @brief QBWebBrowserApp is a class derived from QBApp, enables user to
 * setup browser applicatons parameters.
 */
typedef struct QBWebBrowserApp_* QBWebBrowserApp;

/**
 * @brief This function is used to start web browser app after RCP connection has been
 * established (QBApp started properly).
 * @param self      QBWebBrowserApp handle
 */
void
QBWebBrowserAppStart(QBWebBrowserApp self);

/**
 * @brief This function is used to obtain current web browser params from QBWebBrowserApp.
 * Caller should implement QBWebBrowserAppListener to be notified about
 * params.
 *
 * @param[in] self      QBWebBrowserApp handle
 * @return              current QBWebBrowserAppParams
 */
void
QBWebBrowserAppGetParams(QBWebBrowserApp self);

/**
 * @brief This function is used to set web browser params in QBWebBrowserApp.
 * Caller should implement QBWebBrowserAppListener to be notified if params
 * were - or were not - set.
 *
 * @param[in] self      QBWebBrowserApp handle
 * @param[in] params    QBWebBrowserAppParams handle
 */
void
QBWebBrowserAppSetParams(QBWebBrowserApp self, QBWebBrowserAppParams params);

/**
 * @brief This function is used to send data representing action taken
 * on requested popup to web browser application.
 *
 * @param[in] self           QBWebBrowserApp handle
 * @param[in] popupAction    popup action data
 */
void
QBWebBrowserAppPopupAction(QBWebBrowserApp self, QBWebBrowserAppPopupActionData* popupAction);

/**
 * Method returns hash table with names of services (keys, SvString) and
 * arrays of strings listing interfaces (SvArray of SvStrings)implemented
 * by QBWebBrowserApp.
 *
 * @param [in] self     QBWebBrowserApp handle
 *
 * @return              hash table with services and implemented interfaces
 */
SvHashTable QBWebBrowserAppGetImplementedInterfaces(QBWebBrowserApp self);

/**
 * @brief QBWebBrowserAppFactory is a class used to create QBWebBrowserApp objects.
 */
typedef struct QBWebBrowserAppFactory_* QBWebBrowserAppFactory;

/**
 * @brief This function is used to create QBWebBrowserAppFactory instances.
 *
 * @return          created QBWebBrowserAppFactory handle
 */
QBWebBrowserAppFactory
QBWebBrowserAppFactoryCreate(void);

/**
 * @brief Function returns handle to QBWebBrowserAppListener interface.
 *
 * @return          QBWebBrowserAppListener interface handle
 */
SvInterface
QBWebBrowserAppListener_getInterface(void);

struct QBWebBrowserAppListener_ {
    /**
     * Notification about web browser starting.
     *
     * @param[in] self               self (listener) handle
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method,
     *                               @c positive value in case of handled method
     *
     **/
    unsigned int (*startNotification)(SvObject self_, QBApp application);

    /**
     * Notification about web browser parameters being set.
     *
     * @param[in] self               self (listener) handle
     * @param[in] application        QBApp handler
     *
     * @return                       @a 0 in case of unhandled method,
     *                               @c positive value in case of handled method
     *
     **/
    unsigned int (*setParamsNotification)(SvObject self_, QBApp application);

    /**
     * Notification about web browser parameters being set.
     *
     * @param[in] self               self (listener) handle
     * @param[in] application        QBApp handler
     * @param[in] result             ture if web browser parameters were set correctly, false otherwise
     *
     * @return                       @a 0 in case of unhandled method,
     *                               @c positive value in case of handled method
     *
     **/
    unsigned int (*getParamsNotification)(SvObject self_, QBApp application,
                                          QBWebBrowserAppParams params);

    /**
     * Web browser application's state has changed.
     *
     * @param[in] self              self (listener) handle
     * @param[in] application       handle to application whose state has changed
     * @param[in] state             current state
     *
     * @return                       @a 0 in case of unhandled method,
     *                               @c positive value in case of handled method
     */
    unsigned int (*stateChanged)(SvObject self_, QBApp application,
                                 QBWebBrowserAppState state);

    /**
     * Web browser application has requested popup.
     *
     * @param[in] self              self (listener) handle
     * @param[in] application       handle to application that requested popup
     * @param[in] popupData         data used to create popup
     *
     * @return                       @a 0 in case of unhandled method,
     *                               @c positive value in case of handled method
     */
    unsigned int (*popupRequest)(SvObject self_, QBApp application,
                                 QBWebBrowserAppPopupData const * data);
};
typedef struct QBWebBrowserAppListener_* QBWebBrowserAppListener;

/**
 * @}
 **/
#endif // QBWEBBROWSERAPP_H
