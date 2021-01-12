/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef XMB_VERTICAL_MENU_H_
#define XMB_VERTICAL_MENU_H_

/**
 * @file XMBVerticalMenu.h
 * @brief Xross Media Bar vertical menu widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <QBDataModel3/QBActiveArray.h>
#include <XMB2/XMBTypes.h>
#include <XMB2/XMBMenu.h>

/**
 * @defgroup XMBVerticalMenu XMB vertical menu widget
 * @ingroup XMB
 * @{
 *
 * Widget implementing default XMB vertical menu.
 **/
/**
 * Determines how silding should be performed
 */
typedef enum XMBVerticalMenuSlideMode_e {
    XMBVerticalMenuSlideMode_InPlace, ///< Default behavior, focus stays in one place
    XMBVerticalMenuSlideMode_AtEdges, ///< Focus can move from top to bottom. If focus reach top or bottom, then sliding is performed.
    XMBVerticalMenuSlideMode_Disabled ///< Sliding disabled
} XMBVerticalMenuSlideMode;
/**
 * Create new XMB vertical menu widget.
 *
 * @param[in] app           CUIT application handle
 * @param[in] name          widget name
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
XMBVerticalMenuNew(SvApplication app,
                   const char *name,
                   SvErrorInfo *errorOut);

/**
 * Set custom background for vertical menu widget.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] bg            background widget handle,
 *                          @c NULL to remove background
 **/
extern void
XMBVerticalMenuSetBG(SvWidget menu,
                     SvWidget bg);


/**
 * Set target for menu notification events.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] target        object that will receive
 *                          notification events from menu widget,
 *                          @c 0 to disable notifications
 * @param[in] callback      callback used to notify target about changes,
                            implementation of XMBMenuNotificationCallback
 **/
extern void
XMBVerticalMenuSetNotificationTarget(SvWidget menu,
                                     void *target,
                                     XMBMenuNotificationCallback callback);

/**
 * Attach vertical menu widget to data source.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] dataSource    handle to a QBListModel implementation
 * @param[in] controller    handle to an item controller
 * @param[out] errorOut     error info
 **/
extern void
XMBVerticalMenuConnectToDataSource(SvWidget menu,
                                   SvObject dataSource,
                                   SvObject controller,
                                   SvErrorInfo *errorOut);

/**
 * Get item controller used by vertical menu widget.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @return                  handle to an item controller,
 *                          @c NULL if not available
 **/
extern SvObject
XMBVerticalMenuGetItemController(SvWidget menu);

/**
 * Set item controller to be used by vertical menu widget.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] controller    handle to an item controller
 * @param[out] errorOut     error info
 **/
extern void
XMBVerticalMenuSetItemController(SvWidget menu,
                                 SvObject controller,
                                 SvErrorInfo *errorOut);

/**
 * Get cursor position.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @return                  current cursor position,
 *                          @c -1 in case of error
 **/
extern int
XMBVerticalMenuGetPosition(SvWidget menu);

/**
 * Set cursor position in a vertical menu.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] pos           new cursor position
 * @param[in] immediately   @c true to skip animations
 * @param[out] errorOut     error info
 **/
extern void
XMBVerticalMenuSetPosition(SvWidget menu,
                           unsigned int pos,
                           bool immediately,
                           SvErrorInfo *errorOut);

/**
 * Get current vertical menu state.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @return                  menu state
 **/
extern XMBMenuState
XMBVerticalMenuGetState(SvWidget menu);

/**
 * Change menu state.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] state         new state
 **/
extern void
XMBVerticalMenuSetState(SvWidget menu,
                        XMBMenuState state);

/**
 * Register new range listener.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] listener      range listener handle
 * @param[out] errorOut     error info
 **/
extern void
XMBVerticalMenuAddRangeListener(SvWidget menu,
                                SvObject listener,
                                SvErrorInfo *errorOut);

/**
 * Remove previously registered range listener.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] listener      range listener handle
 * @param[out] errorOut     error info
 **/
extern void
XMBVerticalMenuRemoveRangeListener(SvWidget menu,
                                   SvObject listener,
                                   SvErrorInfo *errorOut);

/**
 * Get slots count of specific XMB Vertical Menu
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @return                  slots count of this XMB vertical menu
 **/
extern size_t
XMBVerticalMenuGetSlotsCount(SvWidget menu);

/**
 * Set scrolling range.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] start         start slot index
 * @param[in] stop         stop slot index
 **/
extern void
XMBVerticalMenuSetScrollRange(SvWidget menu,
                              int start,
                              int stop);

/**
 * Get item widget at index.
 *
 * @param[in] menu          XMB vertical menu widget handle
 * @param[in] idx           item index
 * @return                  item widget or @c NULL in case of error
 **/
extern SvWidget
XMBVerticalMenuGetItemWidgetAtIndex(SvWidget menu,
                                    size_t idx);

/**
 * @}
 **/

#endif
