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

#ifndef QB_LONG_TEXT_DIALOG_H_
#define QB_LONG_TEXT_DIALOG_H_

#include <main_decl.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * @file QBLongTextDialog.h
 * @brief Dialog widget with capability to display long text message.
 * Can be with or without the button 'OK' that allows user to close the dialog.
 * When no 'OK' button is visible, dialog can be closed only by destroying the widget.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBLongTextDialog Dialog with long text message
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Callback called while cancel button will be pressed.
 *
 * @param[in] target    callback target
 * @param[in] w         calling widget handle
 **/
typedef void (*QBLongTextCallbackOnPressedButton)(void *target, SvWidget w);


/**
 * Create dialog widget with capability to display long text message.
 * Appearance of this widget is controlled by the Settings Manager with following variables:
 *
 * | Name               | Type        | Description
 * | ------------------ | ----------- | ----------------
 * | width              | integer     | widget width
 * | height             | integer     | widget height
 * | TextBox            | SvWidget    | text field appearance, see QBTextBoxCreate()
 * | Button             | SvWidget    | button appearance, see svButtonNew()
 *
 * @param[in] appGlobals    appGlobals handle
 * @param[in] widgetName    widget name defined in settings
 * @param[in] message       message to show
 * @param[in] withButton    if \a true dialog will have 'OK' button that allows user to close dialog,
 *                          otherwise user won't be able to close the dialog
 * @param[out] errorOut     error output
 * @return                  newly created widget or NULL in case of error
 **/
extern SvWidget
QBLongTextDialogCreate(AppGlobals appGlobals, const char *widgetName, SvString message, bool withButton, SvErrorInfo *errorOut);

/**
 * Sets callback that will be called while cancel button will be pressed.
 *
 * @param[in] w             dialog widget handle
 * @param[in] target        pointer passed to callback as target argument
 * @param[in] callback      callback handle
 **/
extern void
QBLongTextDialogSetCallbackOnPressedButton(SvWidget w, void *target, QBLongTextCallbackOnPressedButton callback);

/**
 * @}
 **/

#endif /* QB_LONG_TEXT_DIALOG_H_ */
