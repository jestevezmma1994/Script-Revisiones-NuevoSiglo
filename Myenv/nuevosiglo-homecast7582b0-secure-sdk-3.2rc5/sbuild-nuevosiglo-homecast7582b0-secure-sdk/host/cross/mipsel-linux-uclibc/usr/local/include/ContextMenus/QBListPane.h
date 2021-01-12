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

#ifndef QBLISTPANE_H_
#define QBLISTPANE_H_

#include <CUIT/Core/types.h>
#include <ContextMenus/QBContextMenu.h>

typedef struct QBListPane_t* QBListPane;

typedef void (*QBListPaneSelected)(void *self_, QBListPane pane, SvGenericObject object, int pos);
typedef void (*QBListPaneReordered)(void *self_, int oldPos, int newPos);

struct QBListPaneCallbacks_t {
    QBListPaneSelected selected;
    QBListPaneReordered reordered;
};
typedef struct QBListPaneCallbacks_t* QBListPaneCallbacks;

SvType QBListPane_getType(void);

QBListPane QBListPaneCreateFromSettings(const char *settings, SvApplication app, SvGenericObject source, SvGenericObject constructor, QBListPaneCallbacks callback, void *callbackData, QBContextMenu ctxMenu, unsigned level, SvString widgetName);
void QBListPaneInit(QBListPane self, SvApplication app, SvGenericObject source, SvGenericObject constructor, QBListPaneCallbacks callbacks, void *callbackData, QBContextMenu ctxMenu, unsigned level, SvString widgetName);
SvWidget QBListPaneGetMenu(QBListPane self);
void QBListPaneSetSource(QBListPane self, SvGenericObject source);
int QBListPaneGetLevel(QBListPane self);

#endif
