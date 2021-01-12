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

#ifndef QBCONTEXTMENU_H
#define QBCONTEXTMENU_H

#include <CUIT/Core/types.h>
#include <QBApplicationController.h>
#include <QBApplicationControllerTypes.h>
#include <CUIT/Core/types.h>
#include <QBLocalWindowManager.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>


typedef struct QBContextMenu_t* QBContextMenu;
typedef void (*QBContextMenuCallback)(void *ptr, QBContextMenu menu);
typedef bool (*QBContextMenuInputEventHandler)(void *prv, SvObject src, SvInputEvent e);

struct QBContextMenu_t {
    struct QBLocalWindow_t super_;
    SvWidget sideMenu;
    SvWidget window;
    QBApplicationController controller;

    QBContextMenuCallback callback;
    void *callbackData;

    SvArray stack;

    QBContextMenuInputEventHandler inputEventHandler;
    void *inputData;

    bool isShown;
};

typedef const struct QBContextMenuPane_ {
    void (*show)(SvObject self_);
    void (*hide)(SvObject self_, bool immediately);
    void (*setActive)(SvObject self_);
    bool (*handleInputEvent)(SvObject self_, SvObject src, SvInputEvent e);
} *QBContextMenuPane;

SvInterface QBContextMenuPane_getInterface(void);


/**
 * QBContextMenu virtual methods.
 **/
typedef const struct QBContextMenuVTable_ {
    /// virtual methods of the base class
    struct QBWindowVTable_ super_;

    /**
     * Method called when menu should be showed.
     *
     * @param[in] self_ context menu handle
     **/
    void (*show)(QBContextMenu self_);

    /**
     * Method called when menu should be hidden.
     *
     * @param[in] self_ context menu handle
     * @param[in] immediately true iff no effects should be used and the change should
     *            be immediately visible to the user
     **/
    void (*hide)(QBContextMenu self_, bool immediately);

    /**
     * Method which performs check if menu is shown.
     *
     * @param[in] self_ context menu handle
     * @param[out] result indicates if menu is is shown
     **/
    void (*isShown)(QBContextMenu self_, bool* result);
} *QBContextMenuVTable;

SvType QBContextMenu_getType(void);

QBContextMenu QBContextMenuCreateFromSettings(const char *settings, QBApplicationController controller, SvApplication app, SvString sideMenuName);
void QBContextMenuInit(QBContextMenu self, QBApplicationController controller, SvApplication app, SvString sideMenuName);
void QBContextMenuSetCallbacks(QBContextMenu self, QBContextMenuCallback callback, void *ptr);
void QBContextMenuShow(QBContextMenu self);
void QBContextMenuHide(QBContextMenu self, bool immediately);

/**
 * Check if context menu is shown
 *
 * @param[in] self context menu handle
 * @return @c true if shown, @c false otherwise
 */
bool QBContextMenuIsShown(QBContextMenu self);
void QBContextMenuSetDepth(QBContextMenu self, int level, bool immediately);
void QBContextMenuPushPane(QBContextMenu self, SvGenericObject pane);
void QBContextMenuPopPane(QBContextMenu self);
void QBContextMenuSwitchPane(QBContextMenu self, SvGenericObject pane);
bool QBContextMenuHandleInput(QBContextMenu self, SvGenericObject src, SvInputEvent e);
void QBContextMenuSetInputEventHandler(QBContextMenu self, void *prv, QBContextMenuInputEventHandler);
int QBContextMenuGetCurrentPaneLevel(QBContextMenu self);

#endif
