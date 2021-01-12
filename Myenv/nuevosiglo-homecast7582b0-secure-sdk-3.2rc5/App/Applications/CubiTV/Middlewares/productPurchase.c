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

#include <Middlewares/productPurchase.h>

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <settings.h>
#include <Utils/productUtils.h>

#include <QBContentManager/QBContentMgrTransaction.h>
#include <Widgets/authDialog.h>
#include <Widgets/QBSimpleDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Menus/menuchoice.h>
#include <main.h>
#include <Utils/productUtils.h>
#include <QBJSONUtils.h>

struct QBProductPurchase_t {
    struct SvObject_ super_;
    SvWidget dialog;
    SvGenericObject transaction;
    AppGlobals appGlobals;
    SvDBRawObject product;
    SvWeakReference listener;
};

SvLocal void QBProductPurchaseStatusChanged(SvGenericObject self_, SvGenericObject transaction_, const QBContentMgrTransactionStatus *const status);


SvLocal void
QBProductPurchase__dtor__(void *self_)
{
    QBProductPurchase self = self_;
    SVTESTRELEASE(self->transaction);
    if(self->dialog) {
        QBDialogBreak(self->dialog);
        self->dialog = NULL;
    }
    SVTESTRELEASE(self->listener);
    SVRELEASE(self->product);
}

SvLocal SvType
QBProductPurchase_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBProductPurchase__dtor__
    };
    static SvType type = NULL;
    static const struct QBContentMgrTransactionListener_ transactionMethods = {
        .statusChanged = QBProductPurchaseStatusChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBProductPurchase",
                            sizeof(struct QBProductPurchase_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentMgrTransactionListener_getInterface(), &transactionMethods,
                            NULL);
    }

    return type;
}

SvWidget
QBProductPurchaseCreateErrorDialog(AppGlobals appGlobals, const char *title, const char *message)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    svSettingsPopComponent();
    return dialog;
}

SvLocal void
QBProductPurchasePopupCallback(void *self_,
                               SvWidget dialog,
                               SvString buttonTag,
                               unsigned keyCode)
{
    QBProductPurchase self = self_;
    self->dialog = NULL;
}

SvLocal void
QBProductPurchaseStatusChanged(SvGenericObject self_,
                               SvGenericObject transaction_,
                               const QBContentMgrTransactionStatus *const status)
{
    QBProductPurchase self = (QBProductPurchase)self_;
    if(!self->transaction)
        return;

    if(status->status.state == QBRemoteDataRequestState_finished) {
        if(self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }
        SVRELEASE(self->transaction);
        self->transaction = NULL;
    } else if (status->status.state == QBRemoteDataRequestState_failed) {
        if(self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }
        SVRELEASE(self->transaction);
        self->transaction = NULL;
        const char* message = gettext("An error has occurred. Please contact your service provider.\n");
        SvString errorMsg = SvStringCreateWithFormat("%s[%s]", message, status->status.message ? SvStringCString(status->status.message) : "Unknown error");
        self->dialog = QBProductPurchaseCreateErrorDialog(self->appGlobals, gettext("A problem occurred"), SvStringCString(errorMsg));
        QBDialogRun(self->dialog, self, QBProductPurchasePopupCallback);
        char* escapedMsg = QBStringCreateJSONEscapedString(status->status.message ? SvStringCString(status->status.message) : "Unknown error");
        char *escapedErrorMsg = QBStringCreateJSONEscapedString(message);
        SvString data = QBProductUtilsCreateEventData(self->product, NULL);
        QBSecureLogEvent("ProductPurchase", "ErrorShown.Middleware.ProductPurchaseFailed", "JSON:{\"description\":\"%s[%s]\"%s}",
                         escapedErrorMsg, escapedMsg, data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        free(escapedMsg);
        free(escapedErrorMsg);
        SVRELEASE(errorMsg);
    }

    if (self->listener) {
        SvObject list = SvWeakReferenceTakeReferredObject(self->listener);
        if (!list) {
            SVRELEASE(self->listener);
            self->listener = NULL;
            return;
        }

        SvInvokeInterface(QBContentMgrTransactionListener, list, statusChanged, self_, status);
        SVRELEASE(list);
    }
}

QBProductPurchase
QBProductPurchaseNew(SvGenericObject transaction,
                     SvGenericObject product,
                     AppGlobals appGlobals)
{
    QBProductPurchase self = NULL;
    if (unlikely(!transaction || !product || !appGlobals)) {
        SvLogError("%s NULL argument passed!", __func__);
        goto fini;
    }
    if (unlikely(!SvObjectIsImplementationOf(transaction, QBContentMgrTransaction_getInterface()))) {
        goto fini;
    }
    self = (QBProductPurchase)SvTypeAllocateInstance(QBProductPurchase_getType(), NULL);
    self->transaction = SVRETAIN(transaction);
    self->appGlobals = appGlobals;
    self->product = (SvDBRawObject) SVRETAIN(product);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, setListener, (SvGenericObject)self, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, start, appGlobals->scheduler);
    svSettingsPushComponent("Carousel_VOD.settings");
    self->dialog = QBAnimationDialogCreate(appGlobals->res, appGlobals->controller);
    QBDialogRun(self->dialog, self, QBProductPurchasePopupCallback);
    svSettingsPopComponent();
fini:
    return self;
}

void
QBProductPurchaseSetListener(QBProductPurchase self, SvGenericObject listener)
{
    if (unlikely(!self || !listener)) {
        return;
    }

    SVTESTRELEASE(self->listener);
    self->listener = SvWeakReferenceCreateWithObject(listener, NULL);
}

SvDBRawObject
QBProductPurchaseGetProduct(QBProductPurchase self)
{
    if (unlikely(!self)) {
        return NULL;
    }
    return self->product;
}

void
QBProductPurchaseCancel(QBProductPurchase self)
{
    if(self->transaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->transaction, cancel);
        SVRELEASE(self->transaction);
        self->transaction = NULL;
    }
}
