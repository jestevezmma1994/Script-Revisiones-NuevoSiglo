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


#ifndef QB_XHTML_WINDOW_CONTEXT_H
#define QB_XHTML_WINDOW_CONTEXT_H

/**
 * @file QBXHTMLContext.h
 * @brief XHTML Context Class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <Services/QBXHTMLWidgets/QBXHTMLWindowsCreator.h>
#include <QBApplicationController.h>
#include <main_decl.h>

/**
 * @defgroup QBXHTMLContext XHTML context
 * @ingroup CubiTV_windows
 * @{
 *
 */

/**
 * XHTML Context
 *
 * @class QBXHTMLContext
 **/
typedef struct QBXHTMLContext_ *QBXHTMLContext;

/**
 * Creates the XHTML Context.
 *
 * @param[in] appGlobals CubiTV application
 * @return newly created XHTML Context handle
 **/
QBWindowContext QBXHTMLContextCreate(AppGlobals appGlobals);

/**
 * Set XHTML window which be displayed in context.
 *
 * @param[in] self  Handle of XHTML context
 * @param[in] window window to be displayed
 */
void QBXHTMLContextSetWidget(QBXHTMLContext self, QBXHTMLWindow window);

/**
 * Set message which be displayed in case where XHTM window is not set.
 *
 * @param[in] self  Handle of XHTML context
 * @param[in] message message to be displayed
 */
void QBXHTMLContextSetMessage(QBXHTMLContext self, SvString message);

/**
 * @}
 */

#endif //QB_XHTML_WINDOW_CONTEXT_H
