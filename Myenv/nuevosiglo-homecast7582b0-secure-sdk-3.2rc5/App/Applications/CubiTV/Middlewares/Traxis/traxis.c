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

#include "traxis.h"

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <settings.h>
#include <player_hints/rtsp_input.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <TraxisWebClient/TraxisWebVoDProvider.h>
#include <TraxisWebClient/TraxisWebVoDTransactions.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Menus/menuchoice.h>
#include <Widgets/XMBCarousel.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/movieInfo.h>
#include <Widgets/extendedInfo.h>
#include <Widgets/eventISMovieInfo.h>
#include <Utils/authenticators.h>
#include <Utils/QBContentInfo.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBConf.h>
#include <Middlewares/productPurchase.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <main.h>
#include <QBJSONUtils.h>


struct QBTraxisPlayContent_t {
    struct SvObject_ super_;
    SvWidget dialog;
    TraxisWebVoDGetOfferTransaction transaction;
    bool feature;
    AppGlobals appGlobals;
    TraxisWebVoDProvider provider;
    bool lastPosition;
};

SvLocal void QBTraxisPlayContentPopupCallback(void *self_, SvWidget w, SvString buttonTag, unsigned keyCode)
{
    QBTraxisPlayContent self = self_;
    self->dialog = NULL;
}

SvLocal void QBTraxisPlayContentStartPlayback(QBTraxisPlayContent self)
{
    SvDBRawObject content = NULL;
    SvString bookmarkId = NULL;

    if (self->feature) {
        content = TraxisWebVoDGetOfferTransactionGetFeatureContent(self->transaction, NULL);
        SvDBRawObject title = TraxisWebVoDGetOfferTransactionGetTitle(self->transaction, NULL);
        SvObject id = SvDBRawObjectGetAttrValue(title, "id");
        if (id && SvObjectIsInstanceOf(id, SvValue_getType()) && SvValueIsString((SvValue) id)) {
            bookmarkId = SvValueGetString((SvValue) id);
        }
    } else {
        SvImmutableArray contents = TraxisWebVoDGetOfferTransactionGetContents(self->transaction, NULL);
        SvIterator i = SvImmutableArrayIterator(contents);
        while ((content = (SvDBRawObject) SvIteratorGetNext(&i))) {
            SvValue isPreviewV = (SvValue) SvDBRawObjectGetAttrValue(content, "IsPreview");
            if (isPreviewV && SvValueIsBoolean(isPreviewV) && SvValueGetBoolean(isPreviewV))
                break;
        }
    }

    if (!content) {
        SvLogError("QBTraxisPlayContentStartPlayback() - no content");
        return;
    }

    SvHashTable playInfo = (SvHashTable) SvDBRawObjectGetAttrValue(content, "PlayInfo");
    if (!playInfo) {
        SvLogError("QBTraxisPlayContentStartPlayback() - no PlayInfo");
        return;
    }
    if (!SvObjectIsInstanceOf((SvObject) playInfo, SvHashTable_getType())) {
        SvLogError("QBTraxisPlayContentStartPlayback() - bad PlayInfo object");
        return;
    }

    SvImmutableArray locations = (SvArray) SvHashTableFind(playInfo, (SvGenericObject) SVSTRING("Location"));
    if (!locations || !SvObjectIsInstanceOf((SvObject) locations, SvImmutableArray_getType()) || SvImmutableArrayGetCount(locations) < 1) {
        SvLogError("QBTraxisPlayContentStartPlayback() - no locationS");
        return;
    }

    SvHashTable location = (SvHashTable) SvImmutableArrayObjectAtIndex(locations, 0);
    if (!SvObjectIsInstanceOf((SvObject) location, SvHashTable_getType())) {
        SvLogError("QBTraxisPlayContentStartPlayback() - bad location object");
        return;
    }

    SvValue uriV = (SvValue) SvHashTableFind(location, (SvGenericObject) SVSTRING("Uri"));
    SvValue hostNameV = (SvValue) SvHashTableFind(location, (SvGenericObject) SVSTRING("HostName"));
    SvValue protocolV = (SvValue) SvHashTableFind(location, (SvGenericObject) SVSTRING("Protocol"));
    SvValue contentIDv = NULL;

    SvString uriStr = NULL;

    if (uriV) {
        contentIDv = SvDBObjectGetID((SvDBObject) content);
        if (!SvValueIsString(uriV))
            return;
        uriStr = SVRETAIN(SvValueGetString(uriV));
    } else {
        contentIDv = (SvValue) SvHashTableFind(location, (SvObject) SVSTRING("Id"));
        if (!hostNameV || !SvValueIsString(hostNameV) ||
            !contentIDv || !SvValueIsString(contentIDv) ||
            !protocolV || !SvValueIsString(protocolV)) {
            return;
        }

        uriStr = SvStringCreateWithFormat("%s://%s/%s",
                                          SvStringCString(SvValueGetString(protocolV)),
                                          SvStringCString(SvValueGetString(hostNameV)),
                                          SvStringCString(SvValueGetString(contentIDv))
                                          );
    }

    if (!uriStr)
        return;

    const char *groupId = QBConfigGet("EVENTIS_GROUP_ID");
    if (unlikely(!groupId)) {
        groupId = "1";
    }
    SvString CPEID = TraxisWebSessionManagerGetCPEID(self->appGlobals->traxisWebSessionManager);
    SvString fullUri = SvStringCreateWithFormat("%s?VODServingAreaId=%s&STBId=%s",
                                                SvStringCString(uriStr), groupId,
                                                SvStringCString(CPEID));
    SVRELEASE(uriStr);

    SvLogNotice("Play: [%s]", SvStringCString(fullUri));
    SvURI URI = SvURICreateWithString(fullUri, NULL);
    SVRELEASE(fullUri);
    SvContent c = SvContentCreateFromURI(URI, NULL);
    SVRELEASE(URI);

    SvValue titleV = (SvValue) SvDBRawObjectGetAttrValue(content, "Name");
    if (titleV) {
        SvString title = SvValueGetString(titleV);
        SvContentMetaDataSetStringProperty(SvContentGetMetaData(c), SVSTRING(SV_PLAYER_META__TITLE), title);
    }

    // eventis rtsp proxy is not introducing itself properly,
    // so we have to tell the player what server it is talking to:
    SvContentMetaDataSetStringProperty(SvContentGetHints(c), SVSTRING(PLAYER_HINT__RTSP_INPUT_SERVER_TYPE), SVSTRING("eventis_vod"));

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    SvValue propertyKey = SvValueCreateWithString(SVSTRING("VoD"), NULL);
    SvValue propertyValue = SvValueCreateWithString(SVSTRING("traxis"), NULL);
    QBAnyPlayerLogicSetProperty(anyPlayerLogic, propertyKey, propertyValue);
    SVRELEASE(propertyKey);
    SVRELEASE(propertyValue);

    QBContentInfo contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvGenericObject) content, NULL, (SvGenericObject) self->provider, NULL);
    SvGenericObject controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
    SvInvokeInterface(QBAnyPlayerController, (SvGenericObject)controller, setContent, (SvGenericObject) c);
    QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
    SVRELEASE(controller);
    SVRELEASE(contentInfo);

    if (self->feature && bookmarkId && self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, bookmarkId);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, bookmarkId, 0.0, QBBookmarkType_Generic);
        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
    }

    SvDBRawObject title = TraxisWebVoDGetOfferTransactionGetTitle(self->transaction, NULL);

    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

    QBAnyPlayerLogicSetupPlaybackStateController(anyPlayerLogic);

    QBPVRPlayerContextUseBookmarks(pvrPlayer, self->lastPosition);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, title);
    SvString service = TraxisWebVoDProviderGetRootName(self->provider);
    SvString serviceName = SvStringCreate(gettext(SvStringCString(service)), NULL);
    QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
    SVRELEASE(serviceName);
    SVRELEASE(anyPlayerLogic);
    QBPVRPlayerContextSetContent(pvrPlayer, c);
    SVRELEASE(c);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBTraxisPlayContentStatusChanged(SvGenericObject self_,
                          SvGenericObject transaction_,
                          const QBContentMgrTransactionStatus *const status)
{
    QBTraxisPlayContent self = (QBTraxisPlayContent)self_;
    if(!self->transaction)
        return;

    if(status->status.state == QBRemoteDataRequestState_finished) {
        if(self->dialog) {
            QBDialogBreak(self->dialog);
            self->dialog = NULL;
        }
        QBTraxisPlayContentStartPlayback(self);
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
        QBDialogRun(self->dialog, self, QBTraxisPlayContentPopupCallback);
        char* escapedMsg = QBStringCreateJSONEscapedString(status->status.message ? SvStringCString(status->status.message) : "Unknown error");
        char *escapedErrorMsg = QBStringCreateJSONEscapedString(message);
        QBSecureLogEvent("Traxis", "ErrorShown.Middleware.Playback", "JSON:{\"description\":\"%s[%s]\"}", escapedErrorMsg, escapedMsg);
        free(escapedMsg);
        free(escapedErrorMsg);
        SVRELEASE(errorMsg);
    }
}

SvLocal void
QBTraxisPlayContent__dtor__(void *self_)
{
    QBTraxisPlayContent self = self_;
    SVTESTRELEASE(self->transaction);
    SVRELEASE(self->provider);
    if(self->dialog) {
        QBDialogBreak(self->dialog);
        self->dialog = NULL;
    }
}

SvLocal SvType
QBTraxisPlayContent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTraxisPlayContent__dtor__
    };
    static SvType type = NULL;
    static const struct QBContentMgrTransactionListener_ transactionMethods = {
        .statusChanged = QBTraxisPlayContentStatusChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTraxisPlayContent",
                            sizeof(struct QBTraxisPlayContent_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentMgrTransactionListener_getInterface(), &transactionMethods,
                            NULL);
    }

    return type;
}

QBTraxisPlayContent
QBTraxisPlayContentNew(TraxisWebVoDProvider provider, SvDBRawObject title, AppGlobals appGlobals, bool feature)
{
    QBTraxisPlayContent self = (QBTraxisPlayContent)SvTypeAllocateInstance(QBTraxisPlayContent_getType(), NULL);
    self->transaction = TraxisWebVoDGetOfferTransactionCreate(provider, title, NULL);
    if (!self->transaction) {
        SvLogError("%s Failed to create Traxis VOD GetOffer transaction.", __func__);
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }
    self->provider = SVRETAIN(provider);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, setListener, (SvGenericObject)self, NULL);
    SvInvokeInterface(QBContentMgrTransaction, self->transaction, start, appGlobals->scheduler);
    svSettingsPushComponent("Carousel_VOD.settings");
    self->dialog = QBAnimationDialogCreate(appGlobals->res, appGlobals->controller);
    QBDialogRun(self->dialog, self, QBTraxisPlayContentPopupCallback);
    svSettingsPopComponent();
    self->feature = feature;
    self->lastPosition = true;
    self->appGlobals = appGlobals;

fini:
    return self;
}

void
QBTraxisPlayContentCancel(QBTraxisPlayContent self)
{
    if(self->transaction) {
        SvInvokeInterface(QBContentMgrTransaction, self->transaction, cancel);
        SVRELEASE(self->transaction);
        self->transaction = NULL;
    }
}

void
QBTraxisToggleLastPosition(QBTraxisPlayContent self, bool lastPosition)
{
    self->lastPosition = lastPosition;
}
