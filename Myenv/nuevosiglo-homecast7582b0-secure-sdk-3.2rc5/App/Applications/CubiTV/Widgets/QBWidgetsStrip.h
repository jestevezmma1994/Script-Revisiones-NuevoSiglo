/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2009-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_WIDGETS_STRIP_H
#define QB_WIDGETS_STRIP_H

/**
 * @file QBWidgetsStrip.h widget for displaying elements of a list as a horizontal strip of widgets
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * @defgroup QBWidgetsStrip widget strip class
 * @ingroup CubiTV_widgets
 * @{
 **/

/// carousel notification type
#define QB_WIDGETS_STRIP_NOTIFICATION 0x1554AC4

/**
 * Notification event types for widgets strip
 **/
typedef enum {
    QB_WIDGETS_STRIP_NOTIFICATION_ACTIVE_ELEMENT,   ///< the element has been focused
    QB_WIDGETS_STRIP_NOTIFICATION_SELECTED_ELEMENT, ///< the element has been selected (clicked)
    QB_WIDGETS_STRIP_NOTIFICATION_FOCUS_LOST        ///< the widget strip lost focus
} QBWidgetsStripNotificationType;

/**
 * Focus policies of a widget strip
 **/
typedef enum {
    QBWidgetsStripFocusPolicy_FocusFirst,   ///< focus widget at index 0 when receiving focus
    QBWidgetsStripFocusPolicy_FocusLast,    ///< focus widget at last index when receiving focus
    QBWidgetsStripFocusPolicy_FocusRecent   ///< focus widget that was focused when the widget lost focus
} QBWidgetsStripFocusPolicy;

/**
 * QBWidgetsStrip notification class.
 **/
typedef struct QBWidgetsStripNotification_ {
    QBWidgetsStripNotificationType type;        ///< notification type
    int dataIndex;                              ///< index of object for which notification was created
    int widgetIndex;                            ///< index of the widget that represents the data object
    SvGenericObject dataObject;                 ///< object handle for which notification was created
    SvWidget childWidget;                       ///< widget representing focused element
} *QBWidgetsStripNotification;

/**
 * Create a new widgets strip
 *
 * @param[in] app           application handle
 * @param[in] name          name of the widget in the settings file
 * @param[out] errorOut     error info
 * @return                  new instance of a widget strip or @c NULL in case of error
 **/
SvWidget QBWidgetsStripNew(SvApplication app,
                           const char *name,
                           SvErrorInfo *errorOut);

/**
 * Attach strip widget to data source.
 *
 * @param[in] strip         strip widget handle
 * @param[in] dataSource    handle to a QBListModel implementation, or @c NULL to disconnect widget from data source
 * @param[in] controller    handle to an item controller
 * @param[out] errorOut     error info
 **/
extern void
QBWidgetsStripConnectToDataSource(SvWidget strip,
                                  SvGenericObject dataSource,
                                  SvGenericObject controller,
                                  SvErrorInfo *errorOut);

/**
 * Refresh all items
 *
 * @param[in] strip         strip widget handle
 **/
extern void
QBWidgetsStripRefreshItems(SvWidget strip);

/**
 * Set focus policy of a strip widget
 *
 * @param[in] strip         strip widget handle
 * @param[in] policy        new policy of focus
 **/
extern void
QBWidgetsStripConnectSetFocusPolicy(SvWidget strip,
                                    QBWidgetsStripFocusPolicy policy);

/**
 * Set active element index
 *
 * @param[in] strip         strip widget handle
 * @param[in] index         item to be activated
 **/
extern void
QBWidgetsStripSetActive(SvWidget strip,
                        int index);

/**
 * Get the number of widgets in a strip
 *
 * @param[in] strip         strip widget handle
 * @return                  number of widgets in a strip
 **/
extern size_t
QBWidgetsStripGetWidgetsCount(SvWidget strip);

/**
 * Set Widget that will recieve Notifications of user events
 *
 * @param[in] strip         strip widget handle
 * @param[in] target        id of the widget that will recieve notifications
 **/
extern void
QBWidgetsStripSetNotificationTarget(SvWidget strip, SvWidgetId target);

/**
 * @}
 **/

#endif //QB_WIDGETS_STRIP_H
