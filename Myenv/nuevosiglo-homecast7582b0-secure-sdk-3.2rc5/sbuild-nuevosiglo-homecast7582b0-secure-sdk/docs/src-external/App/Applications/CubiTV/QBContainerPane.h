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

#ifndef QBCONTAINERPANE_H_
#define QBCONTAINERPANE_H_

#include <CUIT/Core/types.h>
#include <ContextMenus/QBContextMenu.h>

typedef struct QBContainerPane_t* QBContainerPane;

typedef void (*QBContainerPaneCallbackOnHide)(void *ptr, QBContainerPane pane, SvWidget frame);
typedef void (*QBContainerPaneCallbackOnShow)(void *ptr, QBContainerPane pane, SvWidget frame);
typedef void (*QBContainerPaneCallbackSetActive)(void *ptr, QBContainerPane pane, SvWidget frame);
typedef void (*QBContainerPaneCallbackOnDestroy)(void *ptr, QBContainerPane pane);

struct QBContainerPaneCallbacks_t {
    QBContainerPaneCallbackOnHide onHide;
    QBContainerPaneCallbackOnShow onShow;
    QBContainerPaneCallbackSetActive setActive;
    QBContainerPaneCallbackOnDestroy onDestroy;
};
typedef struct QBContainerPaneCallbacks_t* QBContainerPaneCallbacks;

SvType QBContainerPane_getType(void);

QBContainerPane QBContainerPaneCreateFromSettings(const char *settings, SvApplication app, QBContextMenu ctxMenu, int level,  SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData);
void QBContainerPaneInit(QBContainerPane self, SvApplication app, QBContextMenu ctxMenu, int level, SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData);

int QBContainerPaneGetLevel(QBContainerPane self);

#endif
