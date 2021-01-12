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

#ifndef QB_CHANNEL_SCANNING_WIDGET_H_
#define QB_CHANNEL_SCANNING_WIDGET_H_

/**
 * @file QBChannelScanningWidget.h Channel Scanning Widget
 **/

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>
#include <fibers/c/fibers.h>
#include <Windows/channelScanningParams.h>

/**
 * @defgroup QBChannelScanningWidget Channel Scanning Widget
 * @ingroup CubiTV_widgets
 * @{
 */

/**
 * Channel Scanning Widget class.
 *
 * @class QBChannelScanningWidget QBChannelScanningWidget.h <Widgets/QBChannelScanningWidget.h>
 * @extends SvObject
 **/
typedef struct QBChannelScanningWidget_ *QBChannelScanningWidget;

/**
 * Create new Channel Scanning Widget.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] app           CUIT application handle
 * @param[in] scheduler     fiber scheduler
 * @param[in] name          channel scanning widget name
 * @param[in] conf          params with configuration
 * @param[out] *errorOut    error info
 * @return    New channel scanning widget or @c NULL in case of error
 **/
SvWidget
QBChannelScanningWidgetCreate(SvApplication app,
                              SvScheduler scheduler,
                              const char *name,
                              QBChannelScanningParams conf,
                              SvErrorInfo *errorOut);

/**
 * Create new Channel Scanning Widget with Ratio.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] app           CUIT application handle
 * @param[in] scheduler     fiber scheduler
 * @param[in] name          channel scanning widget name
 * @param[in] params        params with configuration
 * @param[in] parentWidth   Parent Width
 * @param[in] parentHeight  Parent Height
 * @param[out] *errorOut    error info
 * @return    New System Settings menu item choice or @c NULL in case of error
 **/
SvWidget
QBChannelScanningWidgetCreateFromRatio(SvApplication app,
                                       SvScheduler scheduler,
                                       const char *name,
                                       QBChannelScanningParams params,
                                       int parentWidth,
                                       int parentHeight,
                                       SvErrorInfo *errorOut);

/**
 * Use channel scanning widget to handle input event.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] w       channel scanning widget widget handle
 * @param[in] e       event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
bool
QBChannelScanningWidgetHandleInputEvent(SvWidget w, SvInputEvent e);

/**
 * Checks if side menu bar is enabled.
 *
 * @public @memberof QBDiagnosticsWidget
 *
 * @param[in] w       channel scanning widget widget handle
 *
 * @return            flag indicating if side menu is enabled or not
 **/
bool
QBChannelScanningWidgetSideMenuEnabled(SvWidget w);

/**
 * @}
 **/


#endif /* QB_CHANNEL_SCANNING_WIDGET_H_ */
