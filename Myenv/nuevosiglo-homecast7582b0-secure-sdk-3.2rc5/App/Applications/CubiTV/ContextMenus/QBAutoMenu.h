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

#ifndef QBAUTOMENU_H_
#define QBAUTOMENU_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <main_decl.h>

typedef struct QBAutoMenu_ * QBAutoMenu;

struct QBAutoMenuEventHandler_ {
    /**
     * Notify that item in SideMenu has been selected (clicked).
     *
     * @param[in] self_     handle to object implementing event handler
     * @param[in] node      handle to the selected tree node
     * @param[in] nodePath  path to @a node
     * @return              @c true if event have been handled (in that case
     *                      @c SV_EVENT_QB_MENU_SELECTED user event will
     *                      not be sent), @c false otherwise
     **/
    bool (*itemSelected)(SvGenericObject self_,
                         SvGenericObject node,
                         SvGenericObject nodePath,
                         QBBasicPane pane,
                         QBBasicPaneItem item);
};
typedef struct QBAutoMenuEventHandler_ * QBAutoMenuEventHandler;

SvInterface
QBAutoMenuEventHandler_getInterface(void);

/**
 * Create new QBAutoMenu.
 *
 * @param[in] app                  CUIT application handle
 * @param[in] ctxMenu              QBContextMenu object
 * @param[in] menuDescriptionFile  menu description file
 * @param[in] eventHandler         event handler
 * @param[out] errorOut            error info
 * @return    New QBAutoMenu or @c NULL in case of error
 **/
QBAutoMenu
QBAutoMenuCreate(SvApplication app,
                 QBContextMenu ctxMenu,
                 SvString menuDescriptionFile,
                 SvGenericObject eventHandler,
                 SvErrorInfo *errorOut);

#endif /* QBAUTOMENU_H_ */
