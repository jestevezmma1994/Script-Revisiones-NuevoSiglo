/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBINNOV8ONLOADABLEPANE_H_
#define QBINNOV8ONLOADABLEPANE_H_

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Services/core/QBMiddlewareManager.h>

/**
 * Fetch data service.
 * This class is used to fetch data on the product from MW
 **/
typedef struct QBInnov8onFetchService_t *QBInnov8onFetchService;

/**
 * callback function called when the data fetch is finished.
 *
 * @param[in] ptr        data associated with the callback
 * @param[in] service    fetch service that is calling this callback
 * @param[in] product   product fetched from MW or @c NULL in case of error
 **/
typedef void (*QBInnov8onFetchServiceCallback)(void *ptr, QBInnov8onFetchService service, SvGenericObject product);

/**
 * Create data fetch service.
 * The service is responsible for obtaining object data form MW.
 *
 * @param[in] middlewareManager     handle to the middlewareManager used to refresh data
 * @param[in] callback              callbcak called when data is fetched or error is encountered
 * @param[in] callbackData          data passed by to the callback
 * @return                          instance of fetch service or @c NULL in case of error
 */
QBInnov8onFetchService
QBInnov8onFetchServiceCreate(QBMiddlewareManager middlewareManager, QBInnov8onFetchServiceCallback callback, void *callbackData);

/**
 * Set flag deciding if object data should be refreshed after fetch.
 *
 * @param[in] self              pane handle
 * @param[in] refreshObject     flag deciding if the object data should be refreshed (@c true) or not (@c false)
 **/
void
QBInnov8onFetchServiceSetRefreshObject(QBInnov8onFetchService self, bool refreshObject);

/**
 * Start the fetch service.
 *
 * @param[in] self      this object
 * @param[in] product   product whose data needs to be fetched
 * @param[in] scheduler sheduler to use for transaction manager
 **/
void
QBInnov8onFetchServiceStart(QBInnov8onFetchService self, SvGenericObject product, SvScheduler scheduler);

/**
 * Cancel fetch if it is in progress.
 *
 * @param[in] self      this object
 **/
void
QBInnov8onFetchServiceCancel(QBInnov8onFetchService self);

typedef struct QBInnov8onLoadablePaneRefreshParams_ {
    SvObject provider;
    SvObject category;
    SvObject product;
} QBInnov8onLoadablePaneRefreshParams;

typedef struct QBInnov8onLoadablePane_ *QBInnov8onLoadablePane;
typedef void (*QBInnov8onLoadablePaneCallback)(void *ptr, QBInnov8onLoadablePane pane, SvObject product);

SvType
QBInnov8onLoadablePane_getType(void);

void
QBInnov8onLoadablePaneInit(QBInnov8onLoadablePane self, QBContextMenu ctxMenu, AppGlobals appGlobals);

/**
 * Set flag deciding if object data should be refreshed after fetch.
 *
 * @param[in] self              pane handle
 * @param[in] refreshObject     flag deciding if the object data should be refreshed (@c true) or not (@c false)
 **/
void
QBInnov8onLoadablePaneSetRefreshObject(QBInnov8onLoadablePane self, bool refreshObject);

void
QBInnov8onLoadablePaneSetCallback(QBInnov8onLoadablePane self, QBInnov8onLoadablePaneCallback callback, void *callbackData);

void
QBInnov8onLoadablePaneSetRefreshParams(QBInnov8onLoadablePane self, QBInnov8onLoadablePaneRefreshParams *params);

QBBasicPaneItem
QBInnov8onLoadablePaneAddOption(QBInnov8onLoadablePane self, SvString id, SvString caption, QBBasicPaneItemCallback callback, void *callbackData);

QBBasicPaneItem
QBInnov8onLoadablePaneAddOptionWithContainer(QBInnov8onLoadablePane self, SvString id, SvString caption, SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData);

void
QBInnov8onLoadablePaneOptionPropagateObjectChange(QBInnov8onLoadablePane self, QBBasicPaneItem item);

void
QBInnov8onLoadablePaneOnLoadFinished(QBInnov8onLoadablePane self);

/**
 * Set internal loading state of Pane.
 * Normally the loading state of the pane is set inside its internals,
 * but in some cases it might be wanted to force the state from the outside of the pane.
 *
 * @param[in] self      QBInnov8onLoadablePane handle.
 * @param[in] loaded    Flag indicating the state of the pane that should be set.
 **/
void
QBInnov8onLoadablePaneSetLoadingState(QBInnov8onLoadablePane self, bool loaded);

QBBasicPane
QBInnov8onLoadablePaneGetBasicPane(QBInnov8onLoadablePane self);

void
QBInnov8onLoadablePaneShowConnectionErrorPopup(QBInnov8onLoadablePane self, AppGlobals appGlobals);

#endif /* QBINNOV8ONLOADABLEPANE_H_ */
