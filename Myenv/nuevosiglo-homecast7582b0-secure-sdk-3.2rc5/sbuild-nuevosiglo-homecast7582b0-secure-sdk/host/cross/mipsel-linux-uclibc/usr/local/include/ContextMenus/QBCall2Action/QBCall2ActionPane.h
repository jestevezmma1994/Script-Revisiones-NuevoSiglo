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
#ifndef QBCALL2ACTION_PANE_H_
#define QBCALL2ACTION_PANE_H_

/**
 * @file QBCall2ActionPane.h Call2ActionPane class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <main_decl.h>
#include <QBCall2ActionClient/QBCall2ActionProvider.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBCall2ActionPane Call2ActionPane class
 * @ingroup QBCall2ActionContext
 * @{
 **/

/**
 * Call2ActionPane class.
 * Pane sets himself as provider listener. When new level is popped, pane creates QBLoadablePane.
 * Pane waits for data with QBTreeModelListener. When data arrive, pane adds them as basic pane options.
 * When option is selected, pane notice main controller with QBCall2ActionHandler interface.
 * @class QBCall2ActionPane
 * @extends SvObject
 **/
typedef struct QBCall2ActionPane_ * QBCall2ActionPane;

/**
 * QBCall2ActionPaneTimeOutCallback notify that time out has expired.
 **/
typedef void (*QBCall2ActionPaneTimeOutCallback)(void *ptr, QBCall2ActionPane pane);

/**
 * Set item controller.
 *
 * @param[in] self              pane handle
 * @param[in] path              path to subtree where itemConstructor controls item creation
 * @param[in] itemConstructor   itemConstructor handle
 * @param[out] errorOut         error info handle
 * @return                      @c true if itemConstructor set with success, @c false otherwise
 **/
bool
QBCall2ActionPaneSetItemController(QBCall2ActionPane self, SvObject path,
                                   SvObject itemConstructor, SvErrorInfo * errorOut);

/**
 * Set controller, that implementes QBCall2ActionHandler interface.
 *
 * @param[in] self          pane handle
 * @param[in] controller    controller handle
 * @param[out] errorOut     error info handle
 * @return                  @c true if controller set with success, @c false otherwise
 **/
bool
QBCall2ActionPaneSetController(QBCall2ActionPane self, SvObject controller,
                               SvErrorInfo * errorOut);

/**
 * Set time out callback.
 *
 * @param[in] self          pane handle
 * @param[in] callback      callback function pointer
 * @param[in] callbackData  callback object
 * @param[out] errorOut     error info handle
 * @return                  @c true if controller set with success, @c false otherwise
 **/
bool
QBCall2ActionPaneSetTimeOutCallback(QBCall2ActionPane self, QBCall2ActionPaneTimeOutCallback callback,
                                    SvObject callbackData, SvErrorInfo * errorOut);

/**
 * Push next pane with higher level.
 *
 * @param[in] self      pane handler
 * @param[out] errorOut error info handle
 * @return              @c true if pane were created and pushed with success, @c false otherwise
 **/
bool
QBCall2ActionPanePushLevel(QBCall2ActionPane self, SvErrorInfo * errorOut);

/**
 * Pop pane with highest level.
 *
 * @param[in] self      pane handle
 * @param[out] errorOut error info handle
 * @return              @c true if controller werer set with success, @c false otherwise
 **/
bool
QBCall2ActionPanePopLevel(QBCall2ActionPane self, SvErrorInfo * errorOut);

/**
 * Pop all panes and hide context menu.
 *
 * @param[in] self      pane handle
 * @param[out] errorOut error info handle
 * @return              @c true if panes were popped with success, @c false otherwise
 **/
bool
QBCall2ActionPanePopAllLevels(QBCall2ActionPane self, SvErrorInfo * errorOut);

/**
 * Add option to pane with highest level.
 *
 * @param[in] self      pane handle
 * @param[in] id        option id
 * @param[in] caption   option caption
 * @param[out] errorOut error info handle
 * @return              @c true if option were add with success, @c false otherwise
 **/
bool
QBCall2ActionPaneAddOption(QBCall2ActionPane self, SvString id, SvString caption,
                           SvErrorInfo * errorOut);

/**
 * Add action as pane option.
 *
 * @param[in] self          pane handle
 * @param[in] action        action handle
 * @param[in] callback      callback pointer
 * @param[in] callbackData  callbackData handle
 * @param[out] errorOut      error info handle
 * @return                  @c true if option were add with success, @c false otherwise
 **/
bool
QBCall2ActionPaneAddAction(QBCall2ActionPane self, QBCall2ActionTreeNode action,
                           QBBasicPaneItemCallback callback, SvObject callbackData,
                           SvErrorInfo * errorOut);

/**
 * Create pane.
 *
 * @param[in] provider      provider handle
 * @param[in] appGlobals    appGlobals handle
 * @param[in] ctxMenu       context menu handle
 * @param[in] timeOutMs     time out value in milliseconds
 * @param[out] errorOut     error info handle
 * @return                  @c pane handle if pane creation ends with success, @c NULL otherwise
 **/
QBCall2ActionPane
QBCall2ActionPaneCreate(QBCall2ActionProvider provider, AppGlobals appGlobals,
                        QBContextMenu ctxMenu, size_t timeOutMs, SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif /*QBCALL2ACTION_PANE_H_*/
