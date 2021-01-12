/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SEARCH_PROGRESS_DIALOG_H_
#define QB_SEARCH_PROGRESS_DIALOG_H_

/**
 * @file  QBSearchProgressDialog.h QBSearchProgressDialog Class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <QBApplicationController.h>
#include <QBApplicationControllerTypes.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvType.h>

/**
 * QBSearchProgressDialog is class allowing the user displaying popup dialog in two states:
 * progress, no result. In progress state there is displaying progress animation with button Cancel.
 * In second state popup displays information "No results" and button Ok. Progress dialog should have
 * registered listener implementing QBSearchProgressDialogListener insterface.
 *
 **/

typedef struct QBSearchProgressDialog_ *QBSearchProgressDialog;

/** Interface structure.
 *
 * Definition of interface QBSearchProgressDialogListener
 * Object implementing QBSearchProgressDialogListener is reponsible for implementation
 * cancel and ok notifications.
 **/
typedef struct QBSearchProgressDialogListener_ {
    /**
     * Notify that cancel button has been pressed.
     *
     * @param[in] self                      listener handler
     * @param[in] searchProgressDialog      QBSearchProgressDialog handler
     */
    void (*onCancelPressed)(SvObject self, QBSearchProgressDialog searchProgressDialog);
    /**
     * Notify that ok button has been pressed.
     *
     * @param[in] self                      listener handler
     * @param[in] searchProgressDialog      QBSearchProgressDialog handler
     **/
    void (*onOkPressed)(SvObject self, QBSearchProgressDialog searchProgressDialog);
} *QBSearchProgressDialogListener;

/**
 * QBSearchProgressDialogState defines dialog states.
 **/
typedef enum {
    QBSearchProgressDialog_uninitializedState = 0, //!< default state after dialog creation. In this state dialog can't be displayed
    QBSearchProgressDialog_progressState,          //!< progress state
    QBSearchProgressDialog_noResultState,          //!< popup displays information "No results"
    QBSearchProgressDialog_finishedState           //!< search is done, dialog should be hidden
} QBSearchProgressDialogState;

/**
 * Get runtime type identification object representing
 * QBSearchProgressDialogListener interface.
 *
 * @return       QBSearchProgressDialogListener interface object
 **/
extern SvInterface
QBSearchProgressDialogListener_getInterface(void);

/**
 * Create search progress dialog.
 *
 * @param[in] app             application handle
 * @param[in] appController   application controller
 * @param[out] errorOut       error info
 *
 * @return                    newly created QBSearchProgressDialog or @c NULL in case of error
 **/
QBSearchProgressDialog
QBSearchProgressDialogCreate(SvApplication app, QBApplicationController appController, SvErrorInfo *errorOut);


/**
 * Set listener to the dialog.
 *
 * Listener should be the object implementing interface : QBSearchProgressDialogListener.
 *
 * @param[in] self           QBSearchProgressDialog handle
 * @param[in] listener       object implementing interface QBSearchProgressDialogListener
 * @param[out] errorOut      error info
 **/
void
QBSearchProgressDialogSetListener(QBSearchProgressDialog self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Set state of dialog.
 *
 * State can be one of the value from QBSearchProgressDialogState enum.
 *
 * @param[in] self          QBSearchProgressDialog handle
 * @param[in] state         state of the dialog.
 * @param[out] errorOut     error info
 **/
void
QBSearchProgressDialogSetState(QBSearchProgressDialog self, QBSearchProgressDialogState state, SvErrorInfo *errorOut);

/**
 * Display dialog with current state.
 *
 * @param[in] self          QBSearchProgressDialog handle
 * @param[out] errorOut     error info
 */
void
QBSearchProgressDialogShow(QBSearchProgressDialog self, SvErrorInfo *errorOut);

/**
 * Hide dialog.
 *
 * @param[in] self          QBSearchProgressDialog handle
 * @param[out] errorOut     error info
 **/
void
QBSearchProgressDialogHide(QBSearchProgressDialog self, SvErrorInfo *errorOut);

#endif
