/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
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
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <main.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBContentManager/Innov8onProvider.h>
#include <QBContentManager/Innov8onProviderTransaction.h>
#include <QBContentManager/Innov8onProviderParams.h>
#include <Middlewares/Innov8on/innov8onTransaction.h>
#include <Services/core/QBMiddlewareManager.h>

struct QBInnov8onPurchaseTransaction_ {
    struct SvObject_ super_;

    SvGenericObject provider;
    QBInnov8onPurchaseTransactionParams params;
    int offerId;
};

SvLocal void
QBInnov8onPurchaseTransaction__dtor__(void *self_)
{
    QBInnov8onPurchaseTransaction self = self_;

    SVRELEASE(self->provider);
}


SvLocal SvType
QBInnov8onPurchaseTransaction_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onPurchaseTransaction__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onPurchaseTransaction",
                            sizeof(struct QBInnov8onPurchaseTransaction_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBInnov8onPurchaseTransaction QBInnov8onPurchaseTransactionCreate(SvGenericObject provider,
                                                                  int offerId,
                                                                  QBInnov8onPurchaseTransactionParams params,
                                                                  SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBInnov8onPurchaseTransaction self = NULL;
    if (!provider) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "NULL argument passed");
        goto fini;
    }

    self = (QBInnov8onPurchaseTransaction) SvTypeAllocateInstance(QBInnov8onPurchaseTransaction_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error, "can't allocate Innov8onProviderTransaction");
        goto fini;
    }

    self->provider = SVRETAIN(provider);
    self->offerId = offerId;
    self->params = params;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

void QBInnov8onPurchaseTransactionSetPrice(QBInnov8onPurchaseTransaction self, int price, bool isPriceRequired)
{
    self->params.price = price;
    self->params.isPriceRequired = isPriceRequired;
}

SvGenericObject QBInnov8onPurchaseTransactionCreateRequest(AppGlobals appGlobals, QBInnov8onPurchaseTransaction self)
{
    SvGenericObject ret = NULL;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(appGlobals->middlewareManager);
    if (unlikely(!server || !customerId || !hasNetwork)) {
        SvLogError("%s cannot create request - server=%p, customerId=%p, hasNetwork=%d", __func__, server, customerId, hasNetwork);
        goto fini;
    }

    SvStringBuffer additionalParams = SvStringBufferCreate(NULL);
    SvHashTable paramsTable = SvHashTableCreate(1, NULL);
    // add additional parameters to a purchase request
    if (self->params.isPriceRequired) {
        if (SvStringBufferLength(additionalParams))
            SvStringBufferAppendFormatted(additionalParams, NULL, ", ");
        SvStringBufferAppendFormatted(additionalParams, NULL, "\"price\":%d", self->params.price);

        SvValue priceVal = SvValueCreateWithInteger(self->params.price, NULL);
        SvHashTableInsert(paramsTable, (SvGenericObject) SVSTRING("price"), (SvGenericObject) priceVal);
        SVRELEASE(priceVal);
    }

    if (self->params.retCodeRequired) {
        if (SvStringBufferLength(additionalParams))
            SvStringBufferAppendFormatted(additionalParams, NULL, ", ");
        SvStringBufferAppendFormatted(additionalParams, NULL, "\"return_success_code\":true");

        SvValue trueVal = SvValueCreateWithBoolean(true, NULL);
        SvHashTableInsert(paramsTable, (SvGenericObject) SVSTRING("return_success_code"), (SvGenericObject) trueVal);
        SVRELEASE(trueVal);
    }

    /*if (CHANGE_ME:isParamRequired) {
        if (SvStringBufferLength(additionalParams))
            SvStringBufferAppendFormatted(additionalParams, NULL, ", ");
        SvStringBufferAppendFormatted(additionalParams, NULL, "\"CHANGE_ME:name\":%d", CHANGE_ME:value);

        SvValue CHANGE_ME:value = SvValueCreateWithInteger(CHANGE_ME:value, NULL);
        SvHashTableInsert(paramsTable, (SvGenericObject)SVSTRING("CHANGE_ME:name"), (SvGenericObject)CHANGE_ME:value);
        SVRELEASE(priceVal);
    }*/

    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvImmutableArray requestParams = NULL;
    if (SvHashTableCount(paramsTable)) {
        SvString additionalParamsStr = SvStringBufferCreateContentsString(additionalParams, NULL);
        requestParams = SvImmutableArrayCreateWithTypedValues("v@i@", NULL, customerId, self->offerId, (SvGenericObject) paramsTable);
        SvLogNotice("%s sending request UseOffer(mac=%s, offerId=%d, %s)", __func__, SvStringCString(customerId), self->offerId, SvStringCString(additionalParamsStr));
        SVRELEASE(additionalParamsStr);
    } else {
        requestParams = SvImmutableArrayCreateWithTypedValues("v@i", NULL, customerId, self->offerId);
        SvLogNotice("%s sending request UseOffer(mac=%s, offerId=%d)", __func__, SvStringCString(customerId), self->offerId);
    }
    SvXMLRPCRequestInit(req, server, SVSTRING("stb.UseOffer"), requestParams, NULL);
    SVRELEASE(requestParams);

    SVRELEASE(paramsTable);
    SVRELEASE(additionalParams);

    ret = (SvGenericObject) Innov8onProviderTransactionCreate(req, 20, NULL);

    SVTESTRELEASE(req);
fini:
    return ret;
}
