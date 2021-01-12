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

#ifndef QBOSKPANE_H_
#define QBOSKPANE_H_

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <fibers/c/fibers.h>
#include <CUIT/Core/types.h>
#include <ContextMenus/QBContextMenu.h>
#include <QBOSK/QBOSK.h>


typedef struct QBOSKPane_t* QBOSKPane;
typedef void (*QBOSKPaneKeyTyped)(void *ptr, QBOSKPane oskPane, SvString input, unsigned int layout, QBOSKKey key);

SvType QBOSKPane_getType(void);


QBOSKPane QBOSKPaneCreateFromSettings(const char *settings, SvScheduler scheduler, QBContextMenu ctxMenu, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData);

/**
 * Initialize pane.
 *
 * @param[in] self              QBOSK pane
 * @param[in] scheduler         scheduler handle
 * @param[in] ctxMenu           context menu handle
 * @param[in] level             pane level
 * @param[in] widgetName        widget name in settings
 * @param[in] callback          callback for notification of key being typed
 * @param[in] callbackData      callback data
 * @param[in] errorOut          error info
 **/
void QBOSKPaneInit(QBOSKPane self, SvScheduler scheduler, QBContextMenu ctxMenu, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData, SvErrorInfo *errorOut);

/**
 * Initialize pane layout.
 *
 * @param[in] self              QBOSK pane
 * @param[in] scheduler         scheduler handle
 * @param[in] ctxMenu           context menu handle
 * @param[in] file              oskmap filepath
 * @param[in] level             pane level
 * @param[in] widgetName        widget name in settings
 * @param[in] callback          callback for notification of key being typed
 * @param[in] callbackData      callback data
 * @param[in] errorOut          error info
 **/
void QBOSKPaneInitLayout(QBOSKPane self, SvScheduler scheduler, QBContextMenu ctxMenu, const char * file, unsigned int level, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData, SvErrorInfo *errorOut);

SvWidget QBOSKPaneGetTitle(QBOSKPane self);
void QBOSKPaneSetInput(QBOSKPane self, SvString input);
void QBOSKPaneSetRawInput(QBOSKPane self, const char * input);

void QBOSKPaneSetInputWidget(QBOSKPane self, SvWidget input); // Don't call this if input widget is created from settings

void QBOSKPaneSetPasswordMode(QBOSKPane self, bool passwordMode, unsigned int maskCharCode);

#endif

