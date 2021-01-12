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

#ifndef QB_NETWORK_SETTINGS_ADVANCED_WIDGET_H_
#define QB_NETWORK_SETTINGS_ADVANCED_WIDGET_H_

/**
 * @file QBNetworkSettingsAdvancedWidget.h Advanced network settings widget for QBNetworking2
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Services/core/QBTextRenderer.h>
#include <QBNetworkMonitor.h>
#include <QBNetworkMonitor/QBNetworkInterface.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup QBNetworkSettingsAdvancedWidget QBNetworkSettingsAdvancedWidget for QBNetworking2
 * @ingroup CubiTV
 * @{
 *
 * QBNetworkSettingsAdvancedWidget is widget by which user can set advanced network options
 * e.g. IP, network mask, gateway, DNS for given network interface.
 **/

typedef void (*QBNetworkSettingsAdvancedWidgetOnFinishCallback)(void *target);

/**
 * Create advanced network settings widget
 *
 * @param[in] res               application handle
 * @param[in] textRenderer      text renderer handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] widgetName        widget name in settings
 * @param[in] interface         network interface for which the widget is created
 * @param[out] *errorOut        error info
 * @return                      created widget, @c NULL in case of error
  **/
extern SvWidget
QBNetworkSettingsAdvancedWidgetCreate(SvApplication res,
                                      QBTextRenderer textRenderer,
                                      QBNetworkMonitor networkMonitor,
                                      const char *widgetName,
                                      QBNetworkInterface interface,
                                      SvErrorInfo *errorOut);

/**
 * Create advanced network settings widget with ratio
 *
 * @param[in] res               application handle
 * @param[in] textRenderer      text renderer handle
 * @param[in] networkMonitor    network monitor handle
 * @param[in] widgetName        widget name in settings
 * @param[in] parentWidth       parent width
 * @param[in] parentHeight      parent height
 * @param[in] interface         network interface for which the widget is created
 * @param[out] *errorOut        error info
 * @return                      created widget, @c NULL in case of error
  **/
extern SvWidget
QBNetworkSettingsAdvancedWidgetCreateFromRatio(SvApplication res,
                                               QBTextRenderer textRenderer,
                                               QBNetworkMonitor networkMonitor,
                                               const char *widgetName,
                                               unsigned int parentWidth,
                                               unsigned int parentHeight,
                                               QBNetworkInterface interface,
                                               SvErrorInfo *errorOut);

/**
 * Set callback that will be called when edition of settings have been finished.
 *
 * @param[in] widget        widget handle
 * @param[in] target        pointer passed to callback as target argument
 * @param[in] callback      callback handle
 **/
extern void
QBNetworkSettingsAdvancedWidgetSetOnFinishCallback(SvWidget widget, void *target, QBNetworkSettingsAdvancedWidgetOnFinishCallback callback);

/**
 * Set focus for widget
 *
 * @param[in] widget        widget handle
 **/
extern void
QBNetworkSettingsAdvancedWidgetSetFocus(SvWidget widget);

/**
 * @}
**/

#endif /* QB_NETWORK_SETTINGS_ADVANCED_WIDGET_H_ */
