/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBAUTHDIALOG_H
#define QBAUTHDIALOG_H

/**
 * @file authDialog.h
 * @brief Provides full flow base don popups for authenticating customers.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <Services/QBAccessController/QBAccessControllerTypes.h>

/**
 * @defgroup QBAuthDialogAuthenticator Auth Dialog Authenticator interface
 * @ingroup CubiTV_widgets
 * @{
 **/

typedef void (*QBAuthDialogCheckPin)(SvGenericObject self, SvWidget helper, const char *pin);
typedef void (*QBAuthDialogCancelPinCheck)(SvGenericObject self);
typedef QBAccessCode (*QBAuthDialogCheckAccess)(SvGenericObject self, SvWidget helper);
typedef void (*QBAuthDialogCheckAuthParams)(SvGenericObject self, QBAuthParams *params);
typedef SvString (*QBAuthDialogGetDomainFun)(SvGenericObject self);

/**
 * Structure describing QBAuthDialogAuthenticator interface. Objects can implement
 * this interface to communicate with QBAuthDialog to notify it about
 * authentication status changes. This interface is used to separate QBAuthDialog
 * from the authentication logic, allowing for multiple algorithms checking
 * authentication.
 **/
struct QBAuthDialogAuthenticator_t {
    /**
     * Requests pin verification.
     **/
    QBAuthDialogCheckPin checkPin;
    /**
     * Cancel pin verification request.
     **/
    QBAuthDialogCancelPinCheck cancelPinCheck;
    /**
     * Checks current authentication status.
     **/
    QBAuthDialogCheckAccess checkAccess;
    /**
     * Used to query pin details like minimal length.
     **/
    QBAuthDialogCheckAuthParams checkAuthParams;
    /**
     * Returns QBAccessManager domain name
     **/
    QBAuthDialogGetDomainFun getDomain;
};
typedef struct QBAuthDialogAuthenticator_t* QBAuthDialogAuthenticator;

/**
 * Get runtime type identification object representing
 * QBAuthDialogAuthenticator interface;
 *
 * @return QBAuthDialogAuthenticator interface object
 **/
SvInterface QBAuthDialogAuthenticator_getInterface(void);

/**
 * @}
 **/

/**
 * @defgroup QBAuthDialog Authentication Dialog
 * @ingroup CubiTV_widgets
 * @brief Popup used for authenticating customers and to notify them about current notification status.
 * @{
 *
 * In order to use it caller has to provide basic data how to popup widget
 * has to be constructed and also an object implementing QBAuthDialogAuthenticator
 * interface. This object will allow to obtain current authentication
 * status and will notify about authentication status changes using
 * QBAuthDialog*Pin*() methods.
 * QBAuthDialog inherits from QBDialog, so all functions from QBDialog
 * may be called on it, although it is best not to remove items that QBAuthDialog
 * manages.
 **/


/**
 * It may happen that the user should see the popup, but is disallowed
 * from entering a new pin. Mainly used to notify that pin attempts have exceeded.
 * @param[in] helper        QBAuthDialog handle
 * @return returns if entering Pin is enabled
 */
bool QBAuthDialogIsEnteringPinDisabled(SvWidget helper);

/**
 * Called when Pin was accepted by QBAuthDialogAuthenticator
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogPinAccepted(SvWidget helper);

/**
 * Called when entered Pin was wrong and user should get another chance to enter it
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogPinDeclined(SvWidget helper);

/**
 * Called when entered Pin was wrong but user won't get another chance to enter it (only info will be displayed)
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogPinRejected(SvWidget helper);

/**
 * Called when Pin authentication was blocked due to too many tries with wrong Pin
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogPinBlocked(SvWidget helper);

/**
 * Called when Pin authentication failed - for ex. MW connection failed
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogPinFailed(SvWidget helper);

/**
 * Called when Pin authentication was successful, but user is not authorized
 * @param[in] helper        QBAuthDialog handle
 */
void QBAuthDialogNotAuthorized(SvWidget helper);

/**
 * Creates a new instance of QBAuthDialog. To show it on screen call QBDialogRun().
 * @param[in] appGlobals    CubiTV Application
 * @param[in] authenticator object implementing QBAuthDialogAuthenticator interface
 * @param[in] title         title of the popup, may be NULL
 * @param[in] message       message that will be shown on the popup, may be NULL
 * @param[in] local         true if popup should be a QBLocalWindow, false if a QBGlobalWindow
 * @param[in] globalName    string representing name of the QBGlobalWindow (valid only if local is set to false)
 * @param[out] helperPtr    internal pointer to widget that implements QBAuthDialog behaviour
 * @return widget representing new instance of QBAuthDialog
 **/
SvWidget QBAuthDialogCreate(AppGlobals appGlobals, SvGenericObject authenticator, const char *title, const char *message, bool local, SvString globalName, SvWidget *helperPtr);

/**
 * Allows to replace the message string with a specialized message widget.
 *
 * @param[in] dialogWidget  QBAuthDialog handle
 * @param[in] w             widget with message
 **/
void QBAuthDialogSetMessageWidget(SvWidget dialogWidget, SvWidget w);

/**
 * @}
 **/

#endif
