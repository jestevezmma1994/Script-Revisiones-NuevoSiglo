/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRemoteControl.h"

#include <libintl.h>

#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>

#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SvXMLRPCClient/SvXMLRPCRequestsQueue.h>

#include <QBPlayerControllers/QBAnyPlayerController.h>

#include <player_hints/http_input.h>

#include <Services/core/QBPushReceiver.h>
#include <Services/core/GlobalPlayer.h>
#include <Services/QBProvidersControllerService.h>
#include <CubiwareMWClient/Monitor/QBMWServiceMonitor.h>
#include <Services/upgradeWatcher.h>

#include <Services/core/QBMiddlewareManager.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/QBInputDevice.h>
#include <QBInput/QBInputService.h>
#include <QBInput/Drivers/QBVRCDevice.h>

#include <QBContentManager/QBContentProvider.h>
#include <QBDataModel3/QBListProxy.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <QBContentManager/QBContentSearch.h>
#include <Windows/QBVoDCarousel.h>

#include <QBShellCmds.h>
#include <Logic/AnyPlayerLogic.h>
#include <Windows/pvrplayer.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/QBContentInfo.h>
#include <fibers/c/fibers.h>
#include <main.h>
#include <main_decl.h>
#include <string.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, "QBRemoteControlLogLevel", "");

#define log_error(fmt, ...) do { if (env_log_level() >= 0) { SvLogError(COLBEG() "%s: " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); }; } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) { SvLogWarning(COLBEG() "%s: " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); }; } while (0)
#define log_notice(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "%s: " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); }; } while (0)
#define log_debug(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "%s: " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); }; } while (0)

#define INNOV8ON_REQUEST_VERSION "2"

#define REMOTE_COMMAND_ID "RC"

struct QBRemoteControl_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvXMLRPCRequestsQueue xmlRPCReqQueue;

    QBBookmark currentBookmark;
    SvString currentSessionId;
    SvDataBucket bucket;

    bool handleRebootCommand;
    SvFiber fiber;
    SvFiberTimer timer;

    SvFiber resyncFiber;
    SvFiberTimer resyncTimer;

    SvWeakList listeners;
    QBVRCDevice device;

    QBMWServiceMonitor serviceMonitor;
};

SvLocal void
QBRemoteControlInitPlayback(QBRemoteControl self, SvDBRawObject movie)
{
    if (!movie)
        return;

    SvString URI = QBMovieUtilsGetAttr(movie, "source", NULL);
    if (!URI)
        return;

    SvValue productIdV = SvDBObjectGetID((SvDBObject) movie);
    if (!productIdV || !SvValueIsString(productIdV))
        return;

    SvContent content = SvContentCreateFromCString(SvStringCString(URI), NULL);
    SvString title = SvValueGetString((SvValue) SvDBRawObjectGetAttrValue(movie, "name"));
    if (title)
        SvContentMetaDataSetStringProperty(SvContentGetMetaData(content), SVSTRING(SV_PLAYER_META__TITLE), title);

    SvString proxyURI = QBMovieUtilsGetAttr(movie, "proxyURI", NULL);
    if (proxyURI) {
        SvContentMetaDataSetStringProperty(SvContentGetHints(content), SVSTRING(PLAYER_HINT__HTTP_INPUT_PROXY), proxyURI);
    }

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);

    bool useBookmarks = false;
    if (self->appGlobals->bookmarkManager) {
        SvString productId = SvValueGetString(productIdV);
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, productId);
        if (self->currentBookmark) {
            if (bookmark)
                QBBookmarkManagerRemoveBookmark(self->appGlobals->bookmarkManager, bookmark);

            QBBookmarkAddParameter(self->currentBookmark, (SvGenericObject) SVSTRING("LAST_POSITION"), (SvGenericObject) SVSTRING("LAST_POSITION"));
            bookmark = self->currentBookmark;
        } else {
            if (!bookmark)
                bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, productId, 0.0, QBBookmarkType_Generic);
        }

        if (bookmark) {
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
            useBookmarks = true;
        }
    }

    QBContentInfo contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvGenericObject) movie, NULL, NULL, NULL);
    SvGenericObject controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
    SvInvokeInterface(QBAnyPlayerController, controller, setContent, (SvGenericObject) content);
    QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
    SVRELEASE(contentInfo);
    SVRELEASE(controller);

    // AMERELES BoldContentTracking Youbora
    //QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);
    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic, time(NULL));

    QBAnyPlayerLogicSetupPlaybackStateController(anyPlayerLogic);

    QBPVRPlayerContextUseBookmarks(pvrPlayer, useBookmarks);
    QBPVRPlayerContextSetVodMetaData(pvrPlayer, movie); // pvrplayer takes product id from here and looks for bookmark
    title = SvStringCreate(gettext("Sent from remote device"), NULL);
    QBPVRPlayerContextSetTitle(pvrPlayer, title);
    SVRELEASE(title);

    SvString coverURI = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);
    if (coverURI) {
        QBPVRPlayerContextSetCoverURI(pvrPlayer, coverURI);
    }

    SvValue playerFeaturesV = (SvValue) SvDBRawObjectGetAttrValue(movie, "playerFeatures");
    if (playerFeaturesV) {
        const char *playerFeatures = SvStringCString(SvValueGetString(playerFeaturesV));
        while (playerFeatures && *playerFeatures) {
            if (!strncmp(playerFeatures, "disableAll", strlen("disableAll"))) {
                playerFeatures += strlen("disableAll");
                QBAnyPlayerLogicToggleAll(anyPlayerLogic, false);
            } else if (!strncmp(playerFeatures, "loop", strlen("loop"))) {
                playerFeatures += strlen("loop");
                QBAnyPlayerLogicToggleLoop(anyPlayerLogic, true);
            } else
                break;

            if(*playerFeatures)
                playerFeatures++;   //skip ','
        }
    }
    SVRELEASE(anyPlayerLogic);

    QBPVRPlayerContextSetContent(pvrPlayer, content);

    SVTESTRELEASE(content);
    QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal void
QBRemoteControlXMLRPCRequestStateChanged(SvGenericObject self_,
                                             SvXMLRPCRequest req,
                                             SvXMLRPCRequestState state)
{
    QBRemoteControl self = (QBRemoteControl) self_;

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader) SvXMLRPCRequestGetDataListener(req, NULL);
        if (reader) {
            SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (!results || !SvObjectIsInstanceOf((SvObject) results, SvHashTable_getType())) {
                log_error("invalid data format");
            } else {
                SVAUTOSTRING(keyVal, "products");
                SvArray products = (SvArray) SvHashTableFind(results, (SvGenericObject) keyVal);
                if (!products || !SvObjectIsInstanceOf((SvObject) products, SvArray_getType())) {
                    log_error("invalid data format");
                } else if (SvArrayCount(products) > 0) {
                    SvValue xmlV = (SvValue) SvArrayAt(products, 0);
                    if (SvValueIsString(xmlV)) {
                        log_notice("%s", SvObjectGetTypeName((SvGenericObject)xmlV));
                        SvDBObject media_obj = SvDataBucketUpdate(self->bucket, SvValueGetStringAsCString(xmlV, NULL), NULL);
                        if (media_obj) {
                            log_notice("Media object: %s %s", SvValueGetStringAsCString(SvDBObjectGetID(media_obj), NULL), SvDBObjectGetTypeAsCString(media_obj));
                            QBRemoteControlInitPlayback(self, (SvDBRawObject) media_obj);
                            SVRELEASE(media_obj);
                        } else {
                            log_error("invalid data format");
                        }
                    } else
                        log_error("invalid data format");
                } else {
                    log_error("No products in reply");
                }
            }
        }
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(req, &code, &desc, NULL);
        log_debug("Exception: %d: %s", code, SvStringCString(desc));
    } else if (state != SvXMLRPCRequestState_error) {
        return;
    }

    SvXMLRPCRequestsQueueFinishRequest(self->xmlRPCReqQueue);
    SvXMLRPCRequestsQueueProcessNext(self->xmlRPCReqQueue);
}

SvLocal void
QBRemoteControlXMLRPCRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal int
QBRemoteControlGetProducts(QBRemoteControl self, SvString productId, SvString productType)
{
    if (!self->appGlobals->middlewareManager)
        return -1;

    SvString customerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!customerId || !serverInfo || !hasNetwork) {
        SvLogWarning("%s cannot create request - server=%p, customerId=%p, hasNetwork=%s",
                     __func__, serverInfo, customerId, hasNetwork ? "yes" : "no");
        return -1;
    }

    SvArray searchCriteria = SvArrayCreateWithCapacity(1, NULL);
    SvArray sortCriteria = SvArrayCreate(NULL);

    SvString crit = SvStringCreateWithFormat("products.id = %s", SvStringCString(productId));
    SvArrayAddObject(searchCriteria, (SvGenericObject) crit);
    SVRELEASE(crit);

    // TODO: when product_type parameter will be supported by MW,
    //       defaultProductType can be changed to NULL (when no type is sent, MW resolves it internally - worse performance)
    const char *defaultProductType = "vod";
    const char *productTypePtr = productType ? SvStringCString(productType) : defaultProductType;
    SvImmutableArray params = SvImmutableArrayCreateWithTypedValues("@s@ii@s", NULL, customerId, productTypePtr, searchCriteria, 0, 0, sortCriteria, "true");
    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(req, serverInfo, SVSTRING("stb.GetProducts"), params, NULL);
    SvXMLRPCRequestSetListener(req, (SvGenericObject) self, NULL);

    SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
    SVRELEASE(req);

    SVRELEASE(params);
    SVRELEASE(searchCriteria);
    SVRELEASE(sortCriteria);
    return 0;
}

SvLocal void
QBRemoteControlHandlePlaybackCommand(QBRemoteControl self, SvHashTable data)
{
    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
    if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
        return;

    SvString productId = NULL;
    SvValue bookmarkIDVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("bookmark_id"));

    SvString productType = NULL;
    SvValue productTypeVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("product_type"));
    if (productTypeVal && SvObjectIsInstanceOf((SvObject) productTypeVal, SvValue_getType()) && SvValueIsString(productTypeVal))
        productType = SvValueGetString(productTypeVal);

    SVTESTRELEASE(self->currentBookmark);
    self->currentBookmark = NULL;

    if (bookmarkIDVal && self->appGlobals->bookmarkManager) {
        if (!SvObjectIsInstanceOf((SvObject) bookmarkIDVal, SvValue_getType()) || !SvValueIsString(bookmarkIDVal))
            return;

        QBBookmark bookmark = QBBookmarkManagerGetBookmarkByID(self->appGlobals->bookmarkManager, SvValueGetString(bookmarkIDVal));
        if (!bookmark)
            return;

        self->currentBookmark = SVRETAIN(bookmark);
        productId = QBBookmarkGetContentId((QBBookmark) bookmark);
    } else {
        SvValue productIDVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("product_id"));
        if (!productIDVal || !SvObjectIsInstanceOf((SvObject) productIDVal, SvValue_getType()) || !SvValueIsString(productIDVal))
            return;

        productId = SvValueGetString(productIDVal);

        if (self->appGlobals->bookmarkManager) {
            SvValue positionVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("position"));
            if (positionVal && SvObjectIsInstanceOf((SvObject) positionVal, SvValue_getType()) && SvValueIsDouble(positionVal)) {
                QBBookmark bookmark = QBBookmarkCreate(NULL, productId, SvValueGetDouble(positionVal), QBBookmarkType_Generic);
                QBBookmarkManagerAddBookmark(self->appGlobals->bookmarkManager, bookmark);
                self->currentBookmark = SVRETAIN(bookmark);
                SVRELEASE(bookmark);
            }
        }
    }

    QBRemoteControlGetProducts(self, productId, productType);
}

SvLocal int
QBRemoteControlReadDelay(SvHashTable data)
{
    int delayTime = -1;
    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
    if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
        return delayTime;

    SvValue delayTimeVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("delay_time"));
    if (!delayTimeVal || !SvObjectIsInstanceOf((SvObject) delayTimeVal, SvValue_getType()))
        return delayTime;

    if (SvValueIsInteger(delayTimeVal)) {
        delayTime = SvValueGetInteger(delayTimeVal);
    } else if (SvValueIsString(delayTimeVal)) {
        char *delim;
        delayTime = strtol(SvStringCString(SvValueGetString(delayTimeVal)), &delim, 10);
        if (*delim != '\0') {
            delayTime = -1;
        }
    }

    delayTime = delayTime < 0 ? -1 : delayTime;
    return delayTime;
}

SvLocal bool
QBRemoteControlMapStringToKey(SvString cmd, unsigned int *keyCode)
{
    log_debug("key_code: %s", SvStringCString(cmd));

    if (SvStringGetLength(cmd) == 5 && strncmp(SvStringCString(cmd), "KEY_x", 4) == 0) {
        static const char key[] = "0123456789";
        char *digit;
        size_t len = SvStringLength(cmd);
        digit = strchr(key, SvStringCString(cmd)[len - 1]);
        if (digit != NULL) {
            *keyCode = *digit;
            return true;
        }
    }

    if (SvStringGetLength(cmd) == 7 && strncmp(SvStringCString(cmd), "digit_x", 6) == 0) {
        static const char key[] = "0123456789";
        char *digit;
        size_t len = SvStringLength(cmd);
        digit = strchr(key, SvStringCString(cmd)[len - 1]);
        if (digit != NULL) {
            *keyCode = *digit;
            return true;
        }
    }

    if (SvStringEqualToCString(cmd, "KEY_VOLUMEDOWN"))
        *keyCode = QBKEY_VOLDN;
    else if (SvStringEqualToCString(cmd, "KEY_VOLUMEUP"))
        *keyCode = QBKEY_VOLUP;
    else if (SvStringEqualToCString(cmd, "KEY_MUTE"))
        *keyCode = QBKEY_MUTE;
    else if (SvStringEqualToCString(cmd, "KEY_CHANNELDOWN"))
        *keyCode = QBKEY_CHDN;
    else if (SvStringEqualToCString(cmd, "KEY_CHANNELUP"))
        *keyCode = QBKEY_CHUP;
    else if (SvStringEqualToCString(cmd, "KEY_FORWARD"))
        *keyCode = QBKEY_FFW;
    else if (SvStringEqualToCString(cmd, "KEY_REWIND"))
        *keyCode = QBKEY_REW;
    else if (SvStringEqualToCString(cmd, "KEY_NEXT"))
        *keyCode = QBKEY_NEXT;
    else if (SvStringEqualToCString(cmd, "KEY_PREVIOUS"))
        *keyCode = QBKEY_PREV;
    else if (SvStringEqualToCString(cmd, "KEY_PLAY"))
        *keyCode = QBKEY_PLAY;
    else if (SvStringEqualToCString(cmd, "KEY_PAUSE"))
        *keyCode = QBKEY_PAUSE;
    else if (SvStringEqualToCString(cmd, "KEY_STOP"))
        *keyCode = QBKEY_STOP;
    else if (SvStringEqualToCString(cmd, "KEY_RECORD"))
        *keyCode = QBKEY_REC;
    else if (SvStringEqualToCString(cmd, "KEY_RED"))
        *keyCode = QBKEY_RED;
    else if (SvStringEqualToCString(cmd, "KEY_BLUE"))
        *keyCode = QBKEY_BLUE;
    else if (SvStringEqualToCString(cmd, "KEY_YELLOW"))
        *keyCode = QBKEY_YELLOW;
    else if (SvStringEqualToCString(cmd, "KEY_GREEN"))
        *keyCode = QBKEY_GREEN;
    else if (SvStringEqualToCString(cmd, "KEY_EPG"))
        *keyCode = QBKEY_EPG;
    else if (SvStringEqualToCString(cmd, "KEY_FAVORITES"))
        *keyCode = QBKEY_FAVORITES;
    else if (SvStringEqualToCString(cmd, "KEY_HELP"))
        *keyCode = QBKEY_HELP;
    else if (SvStringEqualToCString(cmd, "KEY_HOME"))
        *keyCode = QBKEY_HOME;
    else if (SvStringEqualToCString(cmd, "KEY_INFO"))
        *keyCode = QBKEY_INFO;
    else if (SvStringEqualToCString(cmd, "KEY_PVR"))
        *keyCode = QBKEY_PVR;
    else if (SvStringEqualToCString(cmd, "KEY_POWER"))
        *keyCode = QBKEY_POWER;
    else if (SvStringEqualToCString(cmd, "KEY_MENU"))
        *keyCode = QBKEY_MENU;
    else if (SvStringEqualToCString(cmd, "KEY_OK"))
        *keyCode = QBKEY_ENTER;
    else if (SvStringEqualToCString(cmd, "KEY_BACK"))
        *keyCode = QBKEY_BACK;
    else if (SvStringEqualToCString(cmd, "KEY_LAST"))
        *keyCode = QBKEY_HISTORY;
    else if (SvStringEqualToCString(cmd, "KEY_DOWN"))
        *keyCode = QBKEY_DOWN;
    else if (SvStringEqualToCString(cmd, "KEY_UP"))
        *keyCode = QBKEY_UP;
    else if (SvStringEqualToCString(cmd, "KEY_RIGHT"))
        *keyCode = QBKEY_RIGHT;
    else if (SvStringEqualToCString(cmd, "KEY_LEFT"))
        *keyCode = QBKEY_LEFT;
    else if (SvStringEqualToCString(cmd, "KEY_VOD"))
        *keyCode = QBKEY_VOD;
    else
        return false;

    return true;
}

SvLocal void
QBRemoteControlStep(void *self_)
{
    QBRemoteControl self = self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->handleRebootCommand) {
        self->handleRebootCommand = false;
        QBInitStopApplication(self->appGlobals, true, "remote reboot");
        QBWatchdogReboot(self->appGlobals->watchdog, NULL);
    }
}

SvLocal void
QBRemoteControlResyncStep(void* self_)
{
    QBRemoteControl self = self_;
    SvFiberDeactivate(self->resyncFiber);
    SvFiberEventDeactivate(self->resyncTimer);

    if (self->serviceMonitor) {
        SvObject channelList = self->appGlobals->epgPlugin.IP.channelList;
        if (channelList) {
            SvInvokeInterface(SvEPGChannelListPlugin, channelList, update, NULL);
        }

        QBMWServiceMonitorSendRequest(self->serviceMonitor);
    }
}

SvLocal void
QBRemoteControlHandleRebootCommand(QBRemoteControl self, SvHashTable data)
{
    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
    if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
        return;

    SvValue delayTimeVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("delay_time"));
    if (!delayTimeVal || !SvObjectIsInstanceOf((SvObject) delayTimeVal, SvValue_getType()))
        return;

    int delayTime = -1;
    bool validValue = true;
    if (SvValueIsInteger(delayTimeVal)) {
        delayTime = SvValueGetInteger(delayTimeVal);
    } else if (SvValueIsString(delayTimeVal)) {
        char *delim;
        delayTime = strtol(SvStringCString(SvValueGetString(delayTimeVal)), &delim, 10);
        if (*delim != '\0') {
            validValue = false;
        }
    }

    if (delayTime != -1 && validValue) {
        int randomDelayTimeMs = (delayTime == 0) ? 0 : rand() % (1000 * delayTime);
        self->handleRebootCommand = true;
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(randomDelayTimeMs));
    } else {
        log_error("invalid delay time value");
    }
}

SvLocal void
QBRemoteControlNotifyListeners(QBRemoteControl self, SvHashTable data)
{
    if (!self->listeners)
        return;

    SvGenericObject listener = NULL;
    SvIterator it = SvWeakListIterator(self->listeners);
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBPushReceiverListener, listener, dataReceived, SVSTRING(REMOTE_COMMAND_ID), (SvGenericObject) data);
    }
}

SvLocal void
QBRemoteControlHandleResyncEntitlementsCommand(QBRemoteControl self, SvHashTable data)
{
    int delayTime = QBRemoteControlReadDelay(data);

    if (delayTime != -1) {
        int randomDelayTimeMs = rand() % (1000 * delayTime);
        SvFiberTimerActivateAfter(self->resyncTimer, SvTimeFromMs(randomDelayTimeMs));
    } else {
        log_error("invalid delay time value");
    }
}

SvLocal void
QBRemoteControlGetContentSearch(SvGenericObject source,
                                SvGenericObject *search,
                                SvGenericObject *path)
{
    SvGenericObject dataSource = source;
    int length = SvInvokeInterface(QBListModel, dataSource, getLength);
    for (int i = 0; i < length; ++i) {
        SvGenericObject contentSearch = SvInvokeInterface(QBListModel, dataSource, getObject, i);
        if (SvObjectIsInstanceOf(contentSearch, QBContentSearch_getType())) {
            if (search)
                *search = contentSearch;
            if (path)
                QBListProxyGetTreeNode((QBListProxy) dataSource, i, path);
            return;
        }
    }
    if (search)
        *search = NULL;
    if (path)
        *path = NULL;
}

SvLocal void
QBRemoteControlHandleSearchVODCommand(QBRemoteControl self, SvHashTable data)
{
    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
    if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
        return;

    SvArray mounts = QBProvidersControllerServiceGetServices(self->appGlobals->providersController, SVSTRING("VOD"),
                                                             QBProvidersControllerService_INNOV8ON);
    if (!mounts || SvArrayCount(mounts) == 0)
        return;

    bool vodMenuFound = false;
    SvIterator it = SvArrayIterator(mounts);
    QBCarouselMenuItemService menu;

    while ((menu = (QBCarouselMenuItemService) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(QBCarouselMenuGetServiceName(menu), "VOD")) {
            vodMenuFound = true;
            break;
        }
    }

    if (!vodMenuFound)
        return;

    SvGenericObject contentSearch = NULL;
    SvGenericObject contentSearchPath = NULL;
    QBListProxy proxy = QBListProxyCreate((SvGenericObject) self->appGlobals->menuTree, QBCarouselMenuGetPath(menu), NULL);
    QBRemoteControlGetContentSearch((SvGenericObject) proxy, &contentSearch, &contentSearchPath);
    SVRELEASE(proxy);

    if (!contentSearch || !contentSearchPath)
        return;

    SvValue searchPhraseVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("search_phrase"));

    if (searchPhraseVal) {
        if (!SvObjectIsInstanceOf((SvObject) searchPhraseVal, SvValue_getType()) || !SvValueIsString(searchPhraseVal))
            return;

        SvString searchPhrase = SvValueGetString(searchPhraseVal);
        QBContentSearchStartNewSearch((QBContentSearch) contentSearch, searchPhrase, self->appGlobals->scheduler);
        QBWindowContext ctx = QBVoDCarouselContextCreate(self->appGlobals,
                                                         contentSearchPath,
                                                         QBCarouselMenuGetProvider(menu),
                                                         QBCarouselMenuGetServiceID(menu),
                                                         QBCarouselMenuGetServiceName(menu),
                                                         false, true);

        if (ctx) {
            QBContentProviderStart((QBContentProvider) QBCarouselMenuGetProvider(menu), self->appGlobals->scheduler);
            QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
            SVRELEASE(ctx);
        }
    }
}

SvLocal void
QBRemoteControlHandleRemoteTvCommand(QBRemoteControl self, SvString command, SvHashTable data)
{
    QBWindowContext curContext = QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
    if (curContext != self->appGlobals->newTV) {
        //handle this commands in tv window only
        return;
    }
    if (SvObjectEquals((SvObject) command, (SvObject) SVSTRING("channelup"))) {
        QBTVLogicPlayChannelByOffset(self->appGlobals->tvLogic, 1, SVSTRING("KEY_UP"));
    } else if (SvObjectEquals((SvObject) command, (SvObject) SVSTRING("channeldown"))) {
        QBTVLogicPlayChannelByOffset(self->appGlobals->tvLogic, -1, SVSTRING("KEY_DOWN"));
    } else if (SvObjectEquals((SvObject) command, (SvObject) SVSTRING("channelselect"))) {
        SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));
        if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
            return;

        SvGenericObject channelIdVal = SvHashTableFind(args, (SvGenericObject) SVSTRING("channel_id"));

        if (likely(channelIdVal && SvObjectIsInstanceOf(channelIdVal, SvValue_getType()))) {
            int channelId = -1;
            bool validValue = true;
            if (SvValueIsInteger((SvValue) channelIdVal)) {
                channelId = SvValueGetInteger((SvValue) channelIdVal);
            } else if (SvValueIsString((SvValue) channelIdVal)) {
                char *delim;
                channelId = strtol((SvStringCString(SvValueGetString((SvValue) channelIdVal))), &delim, 10);
                if (*delim != '\0') {
                    validValue = false;
                }
            }
            if (channelId != -1 && validValue) {
                QBTVLogicPlayChannelByNumber(self->appGlobals->tvLogic, channelId, SVSTRING("ManualSelection"));
            } else {
                log_error("invalid channel number");
            }
        }
    }
}

SvLocal void
QBRemoteControlHandlePlayerCommand(QBRemoteControl self, SvString command, SvHashTable data)
{
    QBWindowContext curContext = QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
    if (!SvObjectIsInstanceOf((SvObject) curContext, QBPVRPlayerContext_getType())) {
        //handle only in player window
        return;
    }
    SvHashTable args = (SvHashTable) SvHashTableFind(data, (SvGenericObject) SVSTRING("args"));

    if (SvObjectEquals((SvObject) command, (SvObject) SVSTRING("play"))) {
        if (!args || !SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()))
            return;

        SvValue speedVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("speed"));
        if (!speedVal || !SvValueIsDouble(speedVal))
            return;

        double position = -1.0;
        SvValue positionVal = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("position"));
        if (positionVal) {
            if (!SvValueIsDouble(positionVal))
                return;
            position = SvValueGetDouble(positionVal);
        }
        QBAnyPlayerLogic anyPlayerLogic = QBPVRPlayerContextGetAnyPlayerLogic(curContext);
        double speed = SvValueGetDouble(speedVal);
        if (speed == 1.0)
            QBAnyPlayerLogicPlay(anyPlayerLogic, position);
        else if (speed == 0.0)
            QBAnyPlayerLogicPause(anyPlayerLogic);
        else if (speed > 1.0)
            QBAnyPlayerLogicFFW(anyPlayerLogic);
        else if (speed < 0.0)
            QBAnyPlayerLogicREW(anyPlayerLogic);
    } else if (SvObjectEquals((SvObject) command, (SvObject) SVSTRING("stop"))) {
        if (args && SvObjectIsInstanceOf((SvObject) args, SvHashTable_getType()) && self->appGlobals->bookmarkManager) {
            QBAnyPlayerLogic anyPlayerLogic = QBPVRPlayerContextGetAnyPlayerLogic(curContext);
            QBBookmark bookmark = QBAnyPlayerLogicGetLastPositionBookmark(anyPlayerLogic);
            if (!bookmark)
                return;

            SvValue tag = (SvValue) SvHashTableFind(args, (SvGenericObject) SVSTRING("bookmark_tag"));

            if (!tag || !SvObjectIsInstanceOf((SvObject) tag, SvValue_getType()) || !SvValueIsString(tag))
                return;

            // Don't update bookmark here, it will be done when context will change
            QBBookmarkAddParameter(bookmark, (SvGenericObject) SVSTRING("tag"), (SvGenericObject) SvValueGetString(tag));
            QBBookmarkChangeType(bookmark, QBBookmarkType_Innov8on);
        }
        QBApplicationControllerPopContext(self->appGlobals->controller);
    }
}

SvLocal void
QBRemoteControlPushDataReceived(SvGenericObject self_,
                                SvString type,
                                SvGenericObject data_)
{
    QBRemoteControl self = (QBRemoteControl) self_;

    SvHashTable data = (SvHashTable) data_;
    if (!SvObjectEquals((SvObject) type, (SvObject) SVSTRING(REMOTE_COMMAND_ID)))
        return;

    SvValue cmdVal = (SvValue) SvHashTableFind(data, (SvObject) SVSTRING("command"));
    if (!cmdVal || !SvObjectIsInstanceOf((SvObject) cmdVal, SvValue_getType()) || !SvValueIsString(cmdVal))
        return;

    SvString cmd = SvValueGetString(cmdVal);
    log_debug("got command %s", SvStringCString(cmd));

    if (SvObjectEquals((SvObject) cmd, (SvGenericObject) SVSTRING("universal"))) {
        cmdVal = (SvValue) SvHashTableFind(data, (SvObject) SVSTRING("key_code"));
        if (!cmdVal || !SvObjectIsInstanceOf((SvObject) cmdVal, SvValue_getType()) || !SvValueIsString(cmdVal)) {
            log_error("incorrect key_code");
            return;
        }
        unsigned keyCode;
        if (QBRemoteControlMapStringToKey(SvValueGetString(cmdVal), &keyCode)) {
            QBVRCDeviceHandleCommand(self->device, keyCode);
            return;
        } else {
            cmd = SvValueGetString(cmdVal);
        }
    }

    if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("start_playback"))) {
        QBRemoteControlHandlePlaybackCommand(self, data);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("reboot"))) {
        QBRemoteControlHandleRebootCommand(self, data);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("mute"))) {
        QBVRCDeviceHandleCommand(self->device, QBKEY_MUTE);
    } else if (SvObjectEquals((SvObject) cmd, (SvGenericObject) SVSTRING("volup"))) {
        QBVolumeUp(self->appGlobals->volume);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("voldown"))) {
        QBVolumeDown(self->appGlobals->volume);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("resync_entitlements"))) {
        QBRemoteControlHandleResyncEntitlementsCommand(self, data);
    } else if (SvObjectEquals((SvObject) cmd, (SvGenericObject) SVSTRING("force_upgrade"))) {
        if (self->appGlobals->upgradeWatcher) {
            UpgradeWatcherForceHTTPUpgradeCheck(self->appGlobals->upgradeWatcher);
        }
    } else if (SvObjectEquals((SvObject) cmd, (SvGenericObject) SVSTRING("update_state"))) {
        QBWindowContext curContext = QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
        if (SvObjectIsInstanceOf((SvObject) curContext, QBPVRPlayerContext_getType())) {
            QBAnyPlayerLogic anyPlayerLogic = QBPVRPlayerContextGetAnyPlayerLogic(curContext);
            QBAnyPlayerLogicReportState(anyPlayerLogic);
        }
    } else if (SvObjectEquals((SvObject) cmd, (SvGenericObject) SVSTRING("searchvod"))) {
        QBRemoteControlHandleSearchVODCommand(self, data);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("channelup")) ||
               SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("channeldown")) ||
               SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("channelselect"))) {
        QBRemoteControlHandleRemoteTvCommand(self, cmd, data);
    } else if (SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("play")) ||
               SvObjectEquals((SvObject) cmd, (SvObject) SVSTRING("stop"))) {
        QBRemoteControlHandlePlayerCommand(self, cmd, data);
    } else {
        QBRemoteControlNotifyListeners(self, data);
        log_debug("Command not supported by QBRemoteControl - passing to listeners");
        return;
    }
}

SvLocal void
QBRemoteControlPushStatusChanged(SvObject self_,
                                 QBPushReceiverStatus status)
{
}

SvLocal void
QBRemoteControlDestroy(void *self_)
{
    QBRemoteControl self = (QBRemoteControl) self_;

    SVTESTRELEASE(self->currentBookmark);
    SVTESTRELEASE(self->bucket);
    SVTESTRELEASE(self->device);
    SVRELEASE(self->xmlRPCReqQueue);
    SVRELEASE(self->listeners);
    SVTESTRELEASE(self->serviceMonitor);

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    if (self->resyncFiber) {
        SvFiberDestroy(self->resyncFiber);
        self->resyncFiber = NULL;
    }
}

SvLocal SvType
QBRemoteControl_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRemoteControlDestroy
    };
    static SvType type = NULL;


    static const struct QBPushReceiverListener_t pushReceiverMethods = {
        .dataReceived = QBRemoteControlPushDataReceived,
        .statusChanged = QBRemoteControlPushStatusChanged
    };

    static const struct SvXMLRPCClientListener_t xmlrpcMethods = {
        .stateChanged = QBRemoteControlXMLRPCRequestStateChanged,
        .setup = QBRemoteControlXMLRPCRequestSetup,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBRemoteControl",
                            sizeof(struct QBRemoteControl_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPushReceiverListener_getInterface(), &pushReceiverMethods,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            NULL);
    }

    return type;
}

// public API

QBPlaybackStateController
QBRemoteControlCreatePlaybackStateController(QBRemoteControl self,
                                             SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBPlaybackStateController playbackStateController = QBPlaybackStateControllerCreate(self->appGlobals->scheduler, self->appGlobals->middlewareManager);
    if (!playbackStateController) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "Can't create playbackStateController");
    }

    SvErrorInfoPropagate(error, errorOut);
    return playbackStateController;
}

QBRemoteControl
QBRemoteControlCreate(AppGlobals appGlobals,
                      SvErrorInfo *errorOut)
{
    QBRemoteControl self = NULL;
    SvErrorInfo error = NULL;

    SvDataBucket bucket = NULL;
    SvWeakList listeners = NULL;
    SvXMLRPCRequestsQueue reqQueue = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "NULL argument passed: appGlobals");
        goto err;
    }

    listeners = SvWeakListCreate(NULL);
    if (unlikely(!listeners)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                "can't create SvWeakList");
        goto err;
    }

    bucket = SvDataBucketCreate(&error);
    if (unlikely(!bucket))
        goto err;

    reqQueue = SvXMLRPCRequestsQueueCreate();
    if (!reqQueue) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                "Can't create xmlrpc requests queue");
        goto err;
    }

    self = (QBRemoteControl) SvTypeAllocateInstance(QBRemoteControl_getType(), &error);
    if (unlikely(!self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create QBRemoteControl");
        goto err;
    }

    self->bucket = bucket;
    self->listeners = listeners;
    self->xmlRPCReqQueue = reqQueue;
    self->handleRebootCommand = false;

    self->appGlobals = appGlobals;
    self->serviceMonitor = SVTESTRETAIN(appGlobals->serviceMonitor);

err:
    if (error) {
        SVTESTRELEASE(listeners);
        SVTESTRELEASE(bucket);
        SVTESTRELEASE(reqQueue);
    }
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

int
QBRemoteControlStart(QBRemoteControl self,
                     SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self->appGlobals->pushReceiver) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "pushReceiver doesn't exist");
        goto fini;
    }

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBRemoteControl", QBRemoteControlStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    self->resyncFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBRemoteControlResyncStep", QBRemoteControlResyncStep, self);
    self->resyncTimer = SvFiberTimerCreate(self->resyncFiber);

    QBPushReceiverAddListener(self->appGlobals->pushReceiver, (SvGenericObject) self,
                              SVSTRING(REMOTE_COMMAND_ID), &error);

    if (!self->device) {
        self->device = (QBVRCDevice) SvTypeAllocateInstance(QBVRCDevice_getType(), &error);
        QBVRCDeviceInit(self->device, SVSTRING("QBRemoteControl virtual RC"), NULL);
        QBInputServiceAddDevice((SvGenericObject) self->device, NULL);
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}

int
QBRemoteControlStop(QBRemoteControl self,
                    SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvXMLRPCRequestsQueueCancelAllRequests(self->xmlRPCReqQueue);

    if (!self->appGlobals->pushReceiver) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "pushReceiver doesn't exist");
        goto fini;
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    if (self->resyncFiber) {
        SvFiberDestroy(self->resyncFiber);
        self->resyncFiber = NULL;
        self->resyncTimer = NULL;
    }

    if (self->device) {
        QBInputDeviceID deviceID = SvInvokeInterface(QBInputDevice, self->device, getID);
        QBInputServiceRemoveDevice(deviceID, NULL);
    }

    QBPushReceiverRemoveListener(self->appGlobals->pushReceiver, (SvGenericObject) self,
                                 &error);

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}

int
QBRemoteControlAddListener(QBRemoteControl self,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto err;
    } else if (!listener || !SvObjectIsImplementationOf(listener, QBPushReceiverListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "invalid listener passed");
        goto err;
    }

    SvWeakListPushFront(self->listeners, (SvGenericObject) listener, NULL);

err:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}

int
QBRemoteControlRemoveListener(QBRemoteControl self,
                              SvGenericObject listener,
                              SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto err;
    } else if (!listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                "NULL listener passed");
        goto err;
    }

    SvWeakListRemoveObject(self->listeners, listener);

err:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}
