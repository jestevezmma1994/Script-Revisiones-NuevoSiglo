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

#ifndef QBLOADABLE_DIALOG_H_
#define QBLOADABLE_DIALOG_H_

/**
 * @file QBLoadableDialog.h LoadableDialog class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBLoadableDialog LoadableDialog class
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * QBLoadableDialog shows dialog with "wait" animation.
 * If there isn't specify 'title' parameter while dialog is created.
 * Dialog title is set to "Please wait for result".
 * "Wait" animation is displayed while dialog isn't destroyed or dialog data weren't changed.
 **/

/**
 * Set title and text to displayed dialog.
 *
 * @param[in] w     widget handle
 * @param[in] title title handle
 * @param[in] text  text handle
 **/
void
QBLoadableDialogSetData(SvWidget w, SvString title, SvString text);

/**
 * Create dialog widget with given title.
 *
 * @param[in] appGlobals    appGlobals handle
 * @param[in] title         title handle
 * @param[out] errorOut     error info
 * @return                  @c widget handle if widget was created with success, @c NULL otherwise
 **/
SvWidget
QBLoadableDialogCreate(AppGlobals appGlobals, SvString title, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QBLOADABLE_DIALOG_H_
