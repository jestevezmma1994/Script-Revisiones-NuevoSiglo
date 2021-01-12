/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBREMINDEREDITORPANE_H
#define QBREMINDEREDITORPANE_H

/**
 * @file QBReminderEditorPane.h Pane for an EPG event
 **/

#include <main_decl.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <ContextMenus/QBContextMenu.h>

/**
 * @defgroup QBReminderEditorPane Pane for an EPG event
 * @ingroup CubiTV
 * @{
 **/

/**
 * Edition pane for the EPG event
 *
 * @class QBReminderEditorPane
 **/
typedef struct QBReminderEditorPane_t* QBReminderEditorPane;

/**
 * Get runtime type identification object representing QBReminderEditorPane class.
 *
 * @return QBReminderEditorPane class
 **/
SvType QBReminderEditorPane_getType(void);

/**
 * Pane type for the appropriate recording type: manual, keyword or series
 **/
typedef enum {
    QBReminderEditorPaneType_Manual,
    QBReminderEditorPaneType_Keyword,
    QBReminderEditorPaneType_Series
} QBReminderEditorPaneType;

/**
 * Create Reminder editor pane basing on settings.
 * @param[in] settings              settings which are supposed to be used when creating pane
 * @param[in] itemNamesFilename     file containing translations for pane options
 * @param[in] appGlobals            application global data handle
 * @param[in] ctxMenu               context menu to which the pane is supposed to be added
 * @param[in] widgetName            name for the pane
 * @param[in] channel               channel for which the pane is created
 * @param[in] event                 event for which the pane is created
 * @return QBReminderEditorPane object or NULL in case of error
 */
QBReminderEditorPane QBReminderEditorPaneCreateFromSettings(const char *settings, const char *itemNamesFilename, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event);

/**
 * Create and push Reminder editor pane basing on settings but starting inside the recording menu. The parameter type decides which sub-menu will be created
 * @param[in] settings              settings which are supposed to be used when creating pane
 * @param[in] appGlobals            application global data handle
 * @param[in] ctxMenu               context menu to which the pane is supposed to be added
 * @param[in] widgetName            name for the pane
 * @param[in] channel               channel for which the pane is created
 * @param[in] event                 event for which the pane is created
 * @param[in] type                  type of nPVR recording which sub-menu is to be created - manual, keyword or series
 * @return QBReminderEditorPane object or NULL in case of error
 */
QBReminderEditorPane QBReminderEditorPaneCreateAndPushWithNPvrFromSettings(const char *settings, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, QBReminderEditorPaneType type);

/**
 * Set the coursor to a position in menu with a given ID
 * @param[in] self                  reminder editor pane handle
 * @param[in] id                    id of menu entry to which the cursor is to be set
 * @param[in] immediately           true if the change is to take place immediately
 */
void QBReminderEditorPaneSetPosition(QBReminderEditorPane self, SvString id, bool immediately);

/**
 * Set the coursor to a position in menu with a given index
 * @param[in] self                  reminder editor pane handle
 * @param[in] idx                 index of menu entry to which the cursor is to be set
 * @param[in] immediately           true if the change is to take place immediately
 */
void QBReminderEditorPaneSetPositionByIndex(QBReminderEditorPane self, size_t idx, bool immediately);

/**
 * Creates recording just as if the user demanded the recording through the appropriate menu option
 * @param[in] self                  reminder editor pane handle
 */
void QBReminderEditorPaneRecord(QBReminderEditorPane self);

/**
 * @}
**/

#endif
