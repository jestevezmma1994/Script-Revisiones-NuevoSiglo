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

#ifndef QB_DIAGNOSTICS_WIDGET_H_
#define QB_DIAGNOSTICS_WIDGET_H_

/**
 * @file QBDiagnosticsWidget.h Diagnostics Widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>
#include <Configurations/QBDiagnosticsWindowConfManager.h>
#include <main_decl.h>

/**
 * @defgroup QBDiagnosticsWidget Diagnostics Widget
 * @ingroup CubiTV_widgets
 * @{
 */

/**
 * Diagnostic Widget class.
 *
 * @class QBDiagnosticsWidget QBDiagnosticsWidget.h <Widgets/QBDiagnosticsWidget.h>
 * @extends SvObject
 **/
typedef struct QBDiagnosticsWidget_ *QBDiagnosticsWidget;

/**
 * Create new Diagnostics Widget.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] appGlobals    application globals
 * @param[in] name          diagnostic widget name
 * @param[in] conf          diagnostic widget configuration from QBDiagnosticsWindowConfManager
 * @param[out] *errorOut    error info
 * @return    New System Settings menu item choice or @c NULL in case of error
 **/
SvWidget
QBDiagnosticsWidgetCreate(AppGlobals appGlobals, const char *name, QBDiagnosticsWindowConf conf, SvErrorInfo *errorOut);

/**
 * Create new Diagnostics Widget with ratio settings.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] appGlobals    application globals
 * @param[in] parentWidth   parent width
 * @param[in] parentHeight  parent height
 * @param[in] name          diagnostic widget name
 * @param[in] conf          diagnostic widget configuration from QBDiagnosticsWindowConfManager
 * @param[out] *errorOut    error info
 * @return    New System Settings menu item choice or @c NULL in case of error
 **/
SvWidget
QBDiagnosticsWidgetCreateWithRatio(AppGlobals appGlobals,
                                   unsigned int parentWidth,
                                   unsigned int parentHeight,
                                   const char *name,
                                   QBDiagnosticsWindowConf conf,
                                   SvErrorInfo *errorOut);

/**
 * Use Diagnostic Widget to handle input event.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] w       diagnostic widget handle
 * @param[in] e       event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
bool
QBDiagnosticsWidgetHandleInputEvent(SvWidget w, SvInputEvent e);


/**
 * @}
 **/

#endif /* QB_DIAGNOSTICS_WIDGET_H_ */
