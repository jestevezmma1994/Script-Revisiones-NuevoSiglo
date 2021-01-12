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

#include "QBInnov8onLoadablePane.h"
#include "QBInnov8onLoadablePaneInternal.h"
#include <QBSecureLogManager.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBBasicPaneInternal.h>
#include <ContextMenus/QBLoadablePane.h>
#include <ContextMenus/QBLoadablePaneListener.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/contentCategoryUtils.h>
#include <Utils/value.h>
#include <Utils/productUtils.h>
#include <main.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBContentProviderListener.h>
#include <QBContentManager/Innov8onProviderTransaction.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/Innov8onUtils.h>
#include <QBDataModel3/QBSearchableListModel.h>
#include <QBStringUtils.h>
#include <QBJSONUtils.h>
#include <libintl.h>

#define TIMEOUT 15 * 1000

struct QBInnov8onFetchService_t {
    struct SvObject_ super;

    QBMiddlewareManager middlewareManager; ///< middleware manager used to fetch data

    Innov8onProviderTransaction transaction; ///< data fetch transaction
    QBInnov8onFetchServiceCallback callback; ///< callback called upon fetch finish
    void *callbackData; ///< callback private data
    SvDataBucket storage; ///< storage for fetched data
    bool refreshObject; ///< flag indicationg if the data obtained from MW should be writen to the source object overwriting old values
    SvWeakReference objectRef; ///< reference to the source object which data will be fetched
};

SvLocal void
QBInnov8onFetchService__dtor__(void *_self)
{
    QBInnov8onFetchService self = _self;
    if(self->transaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->transaction, cancel);
        SVRELEASE(self->transaction);
    }

    SVTESTRELEASE(self->storage);
    SVTESTRELEASE(self->objectRef);
}

/**
 * Write data fetched from MW to the source object if appropriate
 *
 * @param [in] self     QBInnov8onFetchService handle
 * @param [in] product  Data fetched from MW
 **/
SvLocal void
QBInnov8onFetchServiceUpdateSourceObjectData(QBInnov8onFetchService self, SvDBRawObject product)
{
    if (!self->refreshObject || !self->objectRef) {
        return;
    }
    SvDBRawObject object = (SvDBRawObject) SvWeakReferenceTakeReferredObject(self->objectRef);
    if (object && SvObjectIsInstanceOf((SvObject) object, SvDBRawObject_getType())) {
        SvObject attrValue = SvDBRawObjectGetAttrValue(product, "offers");
        SvDBRawObjectSetAttrValue(object, "offers", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "expires_at");
        SvDBRawObjectSetAttrValue(object, "expires_at", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "source_array");
        SvDBRawObjectSetAttrValue(object, "source_array", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "source");
        SvDBRawObjectSetAttrValue(object, "source", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "auth_token");
        SvDBRawObjectSetAttrValue(object, "auth_token", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "drm");
        SvDBRawObjectSetAttrValue(object, "drm", attrValue);
        attrValue = SvDBRawObjectGetAttrValue(product, "trailer");
        SvDBRawObjectSetAttrValue(object, "trailer", attrValue);
    }
    SVTESTRELEASE(object);
}

SvLocal void
QBInnov8onFetchServiceTransactionStatusChanged(SvGenericObject self_,
                                               SvGenericObject transaction_,
                                               const QBContentMgrTransactionStatus * const status)
{
    QBInnov8onFetchService self = (QBInnov8onFetchService) self_;
    if(!self->transaction) {
        return;
    }

    SvGenericObject product = NULL;
    if(status->status.state == QBRemoteDataRequestState_finished) {
        SvGenericObject answer = Innov8onProviderTransactionGetAnswer(self->transaction, NULL);
        if (answer && SvObjectIsInstanceOf(answer, SvHashTable_getType())) {
            SvArray products = (SvArray) SvHashTableFind((SvHashTable)answer, (SvGenericObject) SVSTRING("products"));
            if (products && SvArrayCount(products) > 0) {
                SvValue desc = (SvValue)SvArrayObjectAtIndex(products, 0);
                product = (SvGenericObject) SvDataBucketUpdate(self->storage, SvStringCString(SvValueGetString(desc)), NULL);
                if (product && !SvObjectIsInstanceOf((SvGenericObject)product, SvDBRawObject_getType())) {
                    SVRELEASE(product);
                    product = NULL;
                    SvLogNotice("%s(): Refresh product failed (product is not SvDBRawObject, pane=%p)", __func__, self);
                }
            }
        }
        if (product) {
            SvLogNotice("%s(): Refresh product succeeded, %s (pane=%p)", __func__, QBInnov8onProductIsEntitled((SvDBRawObject)product) ? "product already purchased" : "product not yet purchased", self);
        } else {
            SvLogNotice("%s(): Refresh product failed (request succeeded but product is NULL, pane=%p)", __func__, self);
        }

        SVRELEASE(self->transaction);
        self->transaction = NULL;

        QBInnov8onFetchServiceUpdateSourceObjectData(self, (SvDBRawObject) product);

        if (self->callback) {
            self->callback(self->callbackData, self, product);
        }

    } else if(status->status.state == QBRemoteDataRequestState_failed) {
        SvLogNotice("%s Refresh product failed (pane=%p)", __func__,  self);
        if (self->callback) {
            self->callback(self->callbackData, self, product);
        }

        SVTESTRELEASE(self->transaction);
        self->transaction = NULL;
    }
    SVTESTRELEASE(product);
}

SvLocal SvType
QBInnov8onFetchService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onFetchService__dtor__
    };
    static SvType type = NULL;
    static const struct QBContentMgrTransactionListener_ transactionMethods = {
        .statusChanged = QBInnov8onFetchServiceTransactionStatusChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onFetchService",
                            sizeof(struct QBInnov8onFetchService_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentMgrTransactionListener_getInterface(), &transactionMethods,
                            NULL);
    }

    return type;
}

QBInnov8onFetchService
QBInnov8onFetchServiceCreate(QBMiddlewareManager middlewareManager, QBInnov8onFetchServiceCallback callback, void *callbackData)
{
    QBInnov8onFetchService self = (QBInnov8onFetchService) SvTypeAllocateInstance(QBInnov8onFetchService_getType(), NULL);
    if (!self) {
        return NULL;
    }
    self->middlewareManager = middlewareManager;
    self->storage = SvDataBucketCreate(NULL);
    self->callback = callback;
    self->callbackData = callbackData;
    self->refreshObject = false;
    return self;
}

void
QBInnov8onFetchServiceSetRefreshObject(QBInnov8onFetchService self, bool refreshObject)
{
    self->refreshObject = refreshObject;
}

void
QBInnov8onFetchServiceStart(QBInnov8onFetchService self, SvGenericObject product, SvScheduler scheduler)
{
    if (unlikely(!self || !product)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (self->transaction) {
        return;
    }

    // send request to check if we have entitlement for the product
    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString mac = QBMiddlewareManagerGetId(self->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (unlikely(!server || !mac)) {
        SvLogError("%s invalid params", __func__);
        return;
    }

    SvString id = NULL;
    self->objectRef = SvWeakReferenceCreate(product, NULL);
    if (SvObjectIsInstanceOf(product, SvDBRawObject_getType())) {
        id = SvValueGetString(SvDBRawObjectGetDomainId((SvDBRawObject) product, SvDBRawObjectDomain_Cubiware));
    } else {
        id = SvValueGetString(SvDBObjectGetID((SvDBObject) product));
    }
    if (unlikely(!id)) {
        SvLogError("%s invalid product", __func__);
        return;
    }
    SvString searchCriteria = SvStringCreateWithFormat("products.id=%s", SvStringCString(id));
    SvXMLRPCRequest req = SvXMLRPCRequestCreateWithJSONTemplate(server, SVSTRING("stb.GetProducts"), NULL,
                                                                "[%s, \"\", [%s], %d, %d, [\"\"], true]",
                                                                SvStringCString(mac), SvStringCString(searchCriteria), 0, 1);
    SVRELEASE(searchCriteria);
    if (unlikely(!req)) {
        SvLogError("%s failed to create XML-RPC request", __func__);
        return;
    }

    self->transaction = Innov8onProviderTransactionCreate(req, 15, NULL);
    SVRELEASE(req);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, setListener, (SvGenericObject) self, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, start, scheduler);
    SvLogNotice("%s Refresh product id=%s (pane=%p)", __func__, id ? SvStringCString(id) : "--", self);
}

void
QBInnov8onFetchServiceCancel(QBInnov8onFetchService self)
{
    if (self->transaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->transaction, cancel);
        SVRELEASE(self->transaction);
        self->transaction = NULL;
    }
}

SvLocal void
QBInnov8onLoadablePaneRemoveConnectionErrorPopup(QBInnov8onLoadablePane self);

SvLocal void
QBInnov8onLoadablePane__dtor__(void *self_)
{
    QBInnov8onLoadablePane self = self_;
    QBInnov8onLoadablePaneRemoveConnectionErrorPopup(self);
    if (self->refreshParams) {
        SVTESTRELEASE(self->provider);
        SVTESTRELEASE(self->category);
        SVRELEASE(self->product);
    }

    SVTESTRELEASE(self->fetchService);

    SVTESTRELEASE(self->basicPane);
    SVTESTRELEASE(self->loadablePane);
}

SvLocal void
QBInnov8onLoadablePaneErrorOccured(SvObject self_, SvErrorInfo errorInfo)
{
    SvErrorInfoWriteLogMessage(errorInfo);

    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;

    if (!self->product)
        return;

    SvString productType = SvDBObjectGetType((SvDBObject) self->product);

    if (!productType)
        return;

    if (SvStringEqualToCString(productType, "MStore.product")) {
        if (!self->loaded && self->callback && self->callbackData) {
            self->loaded = true;
            self->callback(self->callbackData, self, NULL);
        }
    }
}

SvLocal void
QBInnov8onLoadablePaneActivityStarted(SvObject self_, QBContentCategory category)
{
}

SvLocal void
QBInnov8onLoadablePaneActivityEnded(SvObject self_, QBContentCategory category)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;

    if ((QBContentCategory) self->category != category || self->loaded)
        return;

    self->loaded = true;
    QBInnov8onLoadablePaneOnLoadFinished(self);
    if (!self->callbackData)
        return;

    SvObject itemsSource = QBContentCategoryGetItemsSource(category);
    if (SvInvokeInterface(QBSearchableListModel, itemsSource, contains, self->product)) {
        self->callback(self->callbackData, self, self->product);
    } else {
        self->callback(self->callbackData, self, NULL);
    }
}

SvLocal void
QBInnov8onLoadablePaneFetchEnded(void *self_, QBInnov8onFetchService service, SvGenericObject product)
{
    QBInnov8onLoadablePane self = self_;
    if (self->loaded) {
        return;
    }
    self->loaded = true;
    QBInnov8onLoadablePaneOnLoadFinished(self);
    if (!self->callbackData)
        return;
    if (self->callback) {
        self->callback(self->callbackData, self, product);
    }
}

SvLocal void
QBInnov8onLoadablePaneStateChanged(SvObject self_, QBLoadablePane pane,
                                   QBLoadablePaneState state)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;
    if ((state == QBLoadablePaneStateShown || state == QBLoadablePaneStateStarted) &&
        self->refreshParams) {
        if (self->category) {
            QBContentProviderAddListener((QBContentProvider) self->provider, (SvObject) self);
            QBContentProviderRefresh((QBContentProvider) self->provider,
                                     (QBContentCategory) self->category, self->productPosition, true, true);
        } else {
            QBInnov8onFetchServiceStart(self->fetchService, self->product, self->app->scheduler);
        }
    } else if (state == QBLoadablePaneStateTimeOut) {
        self->loaded = true;
        if (self->callback)
            self->callback(self->callbackData, self, NULL);
    }
}

SvLocal void
QBInnov8onLoadablePaneShow(SvObject self_)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;
    SvObject pane = self->loaded ? (SvObject) self->basicPane : (SvObject) self->loadablePane;
    SvInvokeInterface(QBContextMenuPane, pane, show);
}

SvLocal void
QBInnov8onLoadablePaneHide(SvObject self_, bool immediately)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;
    SvObject pane = self->loaded ? (SvObject) self->basicPane : (SvObject) self->loadablePane;
    SvInvokeInterface(QBContextMenuPane, pane, hide, immediately);
    QBInnov8onFetchServiceCancel(self->fetchService);
}

SvLocal void
QBInnov8onLoadablePaneSetActive(SvObject self_)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;
    SvObject pane = self->loaded ? (SvObject) self->basicPane : (SvObject) self->loadablePane;
    SvInvokeInterface(QBContextMenuPane, pane, setActive);
}

SvLocal bool
QBInnov8onLoadablePaneHandleInputEvent(SvObject self_,
                                       SvObject src,
                                       SvInputEvent e)
{
    return false;
}

SvType
QBInnov8onLoadablePane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onLoadablePane__dtor__
    };
    static SvType type = NULL;

    static const struct QBContentProviderListener_ providerMethods = {
        .errorOccured    = QBInnov8onLoadablePaneErrorOccured,
        .activityStarted = QBInnov8onLoadablePaneActivityStarted,
        .activityEnded   = QBInnov8onLoadablePaneActivityEnded
    };

    static const struct QBLoadablePaneListener_ loadablePaneMethods = {
        .stateChanged = QBInnov8onLoadablePaneStateChanged
    };

    static const struct QBContextMenuPane_ methods = {
        .show             = QBInnov8onLoadablePaneShow,
        .hide             = QBInnov8onLoadablePaneHide,
        .setActive        = QBInnov8onLoadablePaneSetActive,
        .handleInputEvent = QBInnov8onLoadablePaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onLoadablePane",
                            sizeof(struct QBInnov8onLoadablePane_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBContentProviderListener_getInterface(), &providerMethods,
                            QBLoadablePaneListener_getInterface(), &loadablePaneMethods,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void
QBInnov8onLoadablePaneConnectionErrorPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBInnov8onLoadablePane self = (QBInnov8onLoadablePane) self_;
    QBInnov8onLoadablePaneRemoveConnectionErrorPopup(self);
}

SvLocal void
QBInnov8onLoadablePaneRemoveConnectionErrorPopup(QBInnov8onLoadablePane self)
{
    if (self->connectionErrorPopup) {
        QBDialogBreak(self->connectionErrorPopup);
        self->connectionErrorPopup = NULL;
    }
}

void
QBInnov8onLoadablePaneShowConnectionErrorPopup(QBInnov8onLoadablePane self, AppGlobals appGlobals)
{
    const char* message = gettext("There´s a connectivity problem, please try again,"
                          " if the problem remains please call your provider");
    const QBConfirmationDialogParams_t dialogParams = {
        .title                 = gettext("Error occurred"),
        .message               = message,
        .local                 = true,
        .focusOK               = true,
        .isCancelButtonVisible = false,
    };

    QBInnov8onLoadablePaneRemoveConnectionErrorPopup(self);
    self->connectionErrorPopup = QBConfirmationDialogCreate(appGlobals->res, &dialogParams);
    QBDialogRun(self->connectionErrorPopup, self, QBInnov8onLoadablePaneConnectionErrorPopupCallback);

    char* escapedErrorMsg = QBStringCreateJSONEscapedString(message);
    SvString data = NULL;
    if (SvObjectIsInstanceOf(self->product, SvDBRawObject_getType())) {
        data = QBProductUtilsCreateEventData((SvDBRawObject) self->product, NULL);
    }
    QBSecureLogEvent("Innov8onLoadablePane", "ErrorShown.ContextMenus.ConnectionError",
                     "JSON:{\"description\":\"%s\"%s}", escapedErrorMsg, data ? SvStringCString(data) : "");
    free(escapedErrorMsg);
    SVTESTRELEASE(data);
}

void
QBInnov8onLoadablePaneInit(QBInnov8onLoadablePane self,
                           QBContextMenu ctxMenu,
                           AppGlobals appGlobals)
{
    int level = 1;
    self->loadablePane = QBLoadablePaneCreate(ctxMenu, level, TIMEOUT, appGlobals, NULL);
    QBLoadablePaneSetStateListener(self->loadablePane, (SvObject) self, NULL);
    SvApplication app = appGlobals->res;
    self->app = app;
    self->basicPane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->basicPane, app, app->scheduler, appGlobals->textRenderer, ctxMenu,
                    level, SVSTRING("BasicPane"));

    self->fetchService = QBInnov8onFetchServiceCreate(appGlobals->middlewareManager, QBInnov8onLoadablePaneFetchEnded, self);
    QBInnov8onFetchServiceSetRefreshObject(self->fetchService, self->refreshObject);
}

void
QBInnov8onLoadablePaneSetRefreshObject(QBInnov8onLoadablePane self, bool refreshObject)
{
    self->refreshObject = refreshObject;
    if (self->fetchService) {
        QBInnov8onFetchServiceSetRefreshObject(self->fetchService, self->refreshObject);
    }
}

void
QBInnov8onLoadablePaneSetRefreshParams(QBInnov8onLoadablePane self,
                                       QBInnov8onLoadablePaneRefreshParams *params)
{
    if (unlikely(!self || !params || !params->product)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    // refresh poduct to check if we have entitlement

    if (params->category && !SvObjectIsInstanceOf(params->category, QBContentCategory_getType())) {
        SvLogError("%s invalid category type", __func__);
        return;
    }
    if (params->provider && !SvObjectIsInstanceOf(params->provider, QBContentProvider_getType())) {
        SvLogError("%s invalid provider type", __func__);
        return;
    }

    int i = 0;
    if (params->category) {
        i = QBContentCategoryUtilsGetProductIdxInCategory((QBContentCategory) params->category, params->product);
        if (i < 0) {
            SvLogError("%s() failed: There's no such product=%p at given category.", __func__, params->product);
            return;
        }
    }

    self->refreshParams = true;
    self->provider = SVTESTRETAIN(params->provider);
    self->product = SVRETAIN(params->product);
    self->productPosition = (size_t) i;
    self->category = SVTESTRETAIN(params->category);
}

void
QBInnov8onLoadablePaneSetCallback(QBInnov8onLoadablePane self,
                                  QBInnov8onLoadablePaneCallback callback,
                                  void *callbackData)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }
    self->callback = callback;
    self->callbackData = callbackData;
}

QBBasicPaneItem
QBInnov8onLoadablePaneAddOption(QBInnov8onLoadablePane self,
                                SvString id,
                                SvString caption,
                                QBBasicPaneItemCallback callback,
                                void *callbackData)
{
    if (self->basicPane)
        return QBBasicPaneAddOption(self->basicPane, id, caption, callback, callbackData);
    else
        return NULL;
}

QBBasicPaneItem
QBInnov8onLoadablePaneAddOptionWithContainer(QBInnov8onLoadablePane self,
                                             SvString id,
                                             SvString caption,
                                             SvString widgetName,
                                             QBContainerPaneCallbacks callbacks,
                                             void *callbackData)
{
    if (self->basicPane)
        return QBBasicPaneAddOptionWithContainer(self->basicPane, id, caption, widgetName, callbacks, callbackData);
    else
        return NULL;
}

void QBInnov8onLoadablePaneOptionPropagateObjectChange(QBInnov8onLoadablePane self, QBBasicPaneItem item)
{
    if (self->basicPane)
        QBBasicPaneOptionPropagateObjectChange(self->basicPane, item);
}

void
QBInnov8onLoadablePaneOnLoadFinished(QBInnov8onLoadablePane self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL self passed", __func__);
        return;
    }
    SvInvokeInterface(QBContextMenuPane, self->loadablePane, hide, false);
    SvInvokeInterface(QBContextMenuPane, self->basicPane, show);
    SvInvokeInterface(QBContextMenuPane, self->basicPane, setActive);
}

void
QBInnov8onLoadablePaneSetLoadingState(QBInnov8onLoadablePane self, bool loaded)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL self passed", __func__);
        return;
    }

    self->loaded = loaded;
}

QBBasicPane
QBInnov8onLoadablePaneGetBasicPane(QBInnov8onLoadablePane self)
{
    if (!self) {
        SvLogError("%s NULL self passed", __func__);
        return NULL;
    }

    return self->basicPane;
}
