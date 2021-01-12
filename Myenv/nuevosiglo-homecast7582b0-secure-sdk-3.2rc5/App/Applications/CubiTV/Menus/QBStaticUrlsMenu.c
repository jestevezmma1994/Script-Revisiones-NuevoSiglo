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

#include "QBStaticUrlsMenu.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <Windows/pvrplayer.h>
#include <Menus/menuchoice.h>
#include <Logic/AnyPlayerLogic.h>
#include <QBApplicationController.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvViewRightWebDRMInfo.h>
#include <SvPlayerKit/SvViewRightDvbDRMInfo.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <main.h>
#include <Services/QBViewRightManagers/QBViewRightWebManager.h>

struct QBStaticUrlsMenuHandler_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
};
typedef struct QBStaticUrlsMenuHandler_t* QBStaticUrlsMenuHandler;


SvLocal SvDRMInfo QBStaticUrlsMenuViewRightDvbDrmInfoCreate(QBStaticUrlsMenuHandler self, QBActiveTreeNode node)
{
    SvString globalInfoPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("globalInfo"));
    SvString storeInfoPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("storeInfo"));

    if (!globalInfoPath || !storeInfoPath) {
        SvLogError("GlobalInfo path or storeInfo path is missing");
        return NULL;
    }

    SvData globalInfo = SvDataCreateFromFile(SvStringGetCString(globalInfoPath), -1, NULL);
    SvData storeInfo = SvDataCreateFromFile(SvStringGetCString(storeInfoPath), -1, NULL);

    if (!globalInfo || !storeInfo) {
        SvLogError("GlobalInfo file or storeInfo file is missing");
        SVTESTRELEASE(globalInfo);
        SVTESTRELEASE(storeInfo);
        return NULL;
    }

    SvDRMInfo drmInfo = (SvDRMInfo) SvViewRightDvbDRMInfoCreate(globalInfo, storeInfo, NULL);
    SVRELEASE(globalInfo);
    SVRELEASE(storeInfo);

    return drmInfo;
}

SvLocal void QBStaticUrlsMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBStaticUrlsMenuHandler self = (QBStaticUrlsMenuHandler) self_;
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString globalURL = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("URI"));
    if (!globalURL)
        return;

    SvDRMInfo drmInfo = NULL;
    SvString drmName = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("DRM"));
    if (drmName) {
        if (SvStringEqualToCString(drmName, "ViewRightWeb")) {
            drmInfo = (SvDRMInfo) SvViewRightWebDRMInfoCreate(NULL);
        }

        if (SvStringEqualToCString(drmName, "ViewRightDvb")) {
            drmInfo = QBStaticUrlsMenuViewRightDvbDrmInfoCreate(self, node);
        }
    }

    SvLogNotice("%s(): Opening %s", __func__, SvStringCString(globalURL));
    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    QBWindowContext ctx = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);

    if (self->appGlobals->remoteControl) {
        QBPlaybackStateController playbackStateController = QBRemoteControlCreatePlaybackStateController(self->appGlobals->remoteControl, NULL);
        QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
        SVRELEASE(playbackStateController);
    }

    if(self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, globalURL);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, globalURL, 0.0, QBBookmarkType_Generic);

        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
    }

    SVRELEASE(anyPlayerLogic);
    SvContent c = SvContentCreateFromCString(SvStringCString(globalURL), NULL);
    SvContentSetDrmInfo(c, drmInfo);
    QBPVRPlayerContextSetContent(ctx, c);
    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    SVTESTRELEASE(drmInfo);
    SVRELEASE(c);
    SVRELEASE(ctx);
}

SvLocal SvType QBStaticUrlsMenuHandler_getType(void)
{
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBStaticUrlsMenuChoosen
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStaticUrlsMenuHandler",
                            sizeof(struct QBStaticUrlsMenuHandler_t),
                            SvObject_getType(), &type,
                            QBMenuChoice_getInterface(), &menuMethods,
                            NULL);
    }
    return type;
}

void QBStaticUrlsMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("StaticUrls"));
    if (!path)
        return;

    QBStaticUrlsMenuHandler handler = (QBStaticUrlsMenuHandler) SvTypeAllocateInstance(QBStaticUrlsMenuHandler_getType(), NULL);
    handler->appGlobals = appGlobals;

    QBTreePathMapInsert(pathMap, path, (SvObject) handler, NULL);
    SVRELEASE(handler);

    SVRELEASE(path);
}
