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
#ifndef QBCALL2ACTION_CONTEXT_H_
#define QBCALL2ACTION_CONTEXT_H_

/**
 * @file QBCall2ActionContext.h Call2ActionContext class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <ContextMenus/QBBasicPane.h>
#include <SvCore/SvErrorInfo.h>
#include <main_decl.h>
#include <QBCall2ActionClient/QBCall2ActionTreeNode.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @defgroup QBCall2ActionContext Call2ActionContext class
 * @{
 **/

/**
 * Call2Action feature defines ability to control context menus from middleware plugins.
 * When content node from service is selected, QBCall2ActionContext send request to plugin using QBCall2ActionProvider.
 * QBCall2ActionContext shows QBCall2ActionPane and wait for notification through QBCall2ActionHandler.
 **/


/**
 * Call2ActionContext class.
 * @class QBCall2ActionContext
 * @extends SvObject
 **/
typedef struct QBCall2ActionContext_ * QBCall2ActionContext;

typedef void (*QBCall2ActionContextOnCloseCallback)(void * ptr);

/**
 * Add pane option.
 *
 * @param[in] self          context object
 * @param[in] id            new option id
 * @param[in] caption       new option caption
 * @param[out] errorOut     error info handler
 * @return                  @c true, if option was added with success, @c false, otherwise
**/
bool
QBCall2ActionContextAddOption(QBCall2ActionContext self,
                              SvString id,
                              SvString caption,
                              SvErrorInfo * errorOut);

/**
 * Add pane option represent with action.
 *
 * @param[in] self              context object
 * @param[in] action            new option action
 * @param[in] callback          new option callback
 * @param[in] callbackData      new option callbackData
 * @param[out] errorOut         error info handler
 * @return                      @c true, if option was added with success, @c false, otherwise
**/
bool
QBCall2ActionContextAddAction(QBCall2ActionContext self,
                              QBCall2ActionTreeNode action,
                              QBBasicPaneItemCallback callback,
                              SvObject callbackData,
                              SvErrorInfo * errorOut);

/**
 * Set item controller for given path and subpathes.
 *
 * @param[in] self          context object
 * @param[in] controller    item controller
 * @param[in] path          path
 * @param[out] errorOut     error handler
 * @return                  @c true, if controller was set with success, @c false, otherwise
**/
bool
QBCall2ActionContextSetItemController(QBCall2ActionContext self,
                                      SvObject controller,
                                      SvObject path,
                                      SvErrorInfo * errorOut);

/**
 * Set handler. Handler must implement QBCall2ActionHandler interface.
 *
 * @param[in] self          context object
 * @param[in] handler       handler
 * @param[in] path          path
 * @param[out] errorOut     error info handler
 * @return                  @c true, if handler was set with success, @c false, otherwise
**/
bool
QBCall2ActionContextSetHandler(QBCall2ActionContext self,
                               SvObject handler,
                               SvObject path,
                               SvErrorInfo * errorOut);

/**
 * Create QBCall2ActionContext.
 *
 * @param[in] appGlobals
 * @param[in] contentTree   tree
 * @param[in] path          path to product
 * @param[in] serviceId     plugin service id
 * @param[in] serverInfo    plugin server info
 * @param[out] errorOut     error info handle
 * @return                  @c class handle, if creation ends with success, @c NULL, otherwise
**/
QBCall2ActionContext
QBCall2ActionContextCreate(AppGlobals appGlobals,
                           SvObject contentTree,
                           SvObject path,
                           SvString serviceId,
                           SvObject serverInfo,
                           SvErrorInfo * errorOut);

/**
 * Start QBCall2ActionContext.
 * Start provider and loading pane.
 *
 * @param[in] self      context object
 * @param[out] errorOut error info handle
 * @return              @c true, if method ends with success, @c false, otherwise
**/
bool
QBCall2ActionContextStart(QBCall2ActionContext self, SvErrorInfo * errorOut);

/**
 * Stop QBCall2ActionContext.
 * Stop provider and hide loading pane.
 *
 * @param[in] self      context object
 * @param[out] errorOut error info handle
 * @return              @c true, if method ends with success, @c false, otherwise
**/
bool
QBCall2ActionContextStop(QBCall2ActionContext self, SvErrorInfo * errorOut);

/**
 * Set callback that is called when c2a context was closed.
 *
 * @param[in] self      context object
 * @param[in] ptr       callback data pointer
 * @param[in] callback  callback method pointer
 * @param[out] errorOut error info handle
 * @return              @c true, if method ends with success, @c false, otherwise
**/
bool
QBCall2ActionContextSetOnCloseCallback(QBCall2ActionContext self, void * ptr,
                                       QBCall2ActionContextOnCloseCallback callback,
                                       SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif
