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

#include "QBFSEntryInfo.h"

#include <QBAppKit/QBServiceRegistry.h>
#include <QBResourceManager/rb.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/icon.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <Menus/menuchoice.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <QBFSEntry.h>
#include <QBFSFile.h>

struct  QBFSEntryInfo_t {
    bool            imageMode;
    SvWidget        content;
    SvWidget        contentImage;

    SvWidget        icons;

    SvApplication   app;

    unsigned int    ctxID;
    SvRID           directoryBmpRID;
    SvRID           waitBmpRID;
    SvRID           imageBmpRID;
    SvRID           videoBmpRID;
    SvRID           audioBmpRID;
    SvRID           playlistBmpRID;
    SvRID           unsupportedBmpRID;

    SvGenericObject object;
};
typedef struct QBFSEntryInfo_t *QBFSEntryInfo;

SvLocal void
QBFSEntryInfoCleanup(SvApplication app, void* ptr)
{
    QBFSEntryInfo self = (QBFSEntryInfo) ptr;
    SVTESTRELEASE(self->object);
    free(ptr);
}

SvWidget
QBFSEntryInfoCreate(SvApplication app)
{
    QBFSEntryInfo prv = calloc(1, sizeof(struct QBFSEntryInfo_t));

    prv->app                = app;
    // Additional images and RIDs for different MIME Types should be added
    prv->waitBmpRID         = svSettingsGetResourceID("FSEntryInfo.Image", "waitImage");
    prv->directoryBmpRID    = svSettingsGetResourceID("FSEntryInfo.Image", "directoryImage");
    prv->imageBmpRID        = svSettingsGetResourceID("FSEntryInfo.Image", "imageImage");
    prv->audioBmpRID        = svSettingsGetResourceID("FSEntryInfo.Image", "audioImage");
    prv->videoBmpRID        = svSettingsGetResourceID("FSEntryInfo.Image", "videoImage");
    prv->playlistBmpRID     = svSettingsGetResourceID("FSEntryInfo.Image", "playlistImage");
    prv->unsupportedBmpRID  = svSettingsGetResourceID("FSEntryInfo.Image", "unsupportedImage");

    SvWidget background     = QBFrameCreateFromSM(prv->app, "FSEntryInfo.Background");
    prv->content            = svWidgetCreate(prv->app, background->width, background->height);
    svWidgetAttach(prv->content, background, 0, 0, 0);
    prv->ctxID              = svSettingsSaveContext();

    SvWidget w              = svWidgetCreate(app, prv->content->width, prv->content->height);
    w->prv                  = prv;
    svWidgetAttach(w, prv->content, 0, 0, 3);

    w->clean                = QBFSEntryInfoCleanup;
    svWidgetSetFocusable(w, true);

    prv->contentImage = NULL;
    prv->object = NULL;

    return w;
}

SvLocal SvRID
QBFSEntryInfoLoadAsyncBitmap(QBFSEntryInfo self, SvString path)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));

    SvRID rid = SV_RID_INVALID;
    const char *URIStr = SvStringGetCString(path);
    SvRBBitmap bmp = (SvRBBitmap) svRBFindItemByURI(resourceManager, URIStr);
    if (bmp) {
        if (SvObjectIsInstanceOf((SvObject) bmp, SvRBBitmap_getType())) {
            rid = SvRBObjectGetID((SvRBObject) bmp);
        }
    } else {
        bmp = SvRBBitmapCreateWithURICString(URIStr, SvBitmapType_static);
        if (likely(bmp)) {
            SvRBBitmapSetAsync(bmp, true);
            rid = svRBAddItem(resourceManager, (SvRBObject) bmp, SvRBPolicy_auto);
            SVRELEASE(bmp);
        }
    }

    return rid;
}

SvLocal void
QBFSEntryInfoCreateContentImage(QBFSEntryInfo self, SvRID bmpRID, SvString bmpPath)
{
    self->contentImage = svIconNew(self->app, "FSEntryInfo.Image");
    svIconSetBitmapFromRID(self->contentImage, 0, self->unsupportedBmpRID);
    svIconSetBitmapFromRID(self->contentImage, 1, self->waitBmpRID);
    if (bmpPath != NULL) {
        bmpRID = QBFSEntryInfoLoadAsyncBitmap(self, bmpPath);
    }
    if (bmpRID != SV_RID_INVALID) {
        svIconSetBitmapFromRID(self->contentImage, 2, bmpRID);
        svIconSwitch(self->contentImage, 1, 0, 0);
        svIconSwitch(self->contentImage, 2, 0, -1.0);
    }

    svSettingsWidgetAttach(self->content, self->contentImage, "FSEntryInfo.Image", 3);
}

SvLocal void
QBFSEntryInfoCreateEntryWidget(QBFSEntryInfo self, QBFSEntry entry)
{
    QBFSEntryType entryType = QBFSEntryGetType(entry);
    if (entryType == QBFSEntryType_directory) {
        QBFSEntryInfoCreateContentImage(self, self->directoryBmpRID, NULL);
    } else if (entryType == QBFSEntryType_empty || entryType == QBFSEntryType_unknown) {
        QBFSEntryInfoCreateContentImage(self, self->unsupportedBmpRID, NULL);
    } else {
        SvRID           destRID = SV_RID_INVALID;
        QBFSFileType    fileType = QBFSFileGetFileType((QBFSFile) entry);
        SvString        fullPath = NULL;

        switch(fileType)
        {
        case QBFSFileType_image:
            fullPath = QBFSEntryCreateFullPathString(entry);
            break;
        case QBFSFileType_audio:
            destRID = self->audioBmpRID;
            break;
        case QBFSFileType_video:
            destRID = self->videoBmpRID;
            break;
        case QBFSFileType_playlist:
            destRID = self->playlistBmpRID;
            break;
        default:
            destRID = self->unsupportedBmpRID;
            break;
        }
        QBFSEntryInfoCreateContentImage(self, destRID, fullPath);
        SVTESTRELEASE(fullPath);
    }
}

void
QBFSEntryInfoSetObject(SvWidget w, SvGenericObject object)
{
    QBFSEntryInfo prv = (QBFSEntryInfo) w->prv;

    if (SvObjectEquals(prv->object, object))
        return;

    SVTESTRELEASE(prv->object);
    prv->object = SVTESTRETAIN(object);

    svSettingsRestoreContext(prv->ctxID);

    svWidgetDestroy(prv->content);
    //contentImage needs to be nullified because it is destroyed as a child of prv->content.
    prv->contentImage = NULL;
    SvWidget background = QBFrameCreateFromSM(prv->app, "FSEntryInfo.Background");
    prv->content = svWidgetCreate(prv->app, background->width, background->height);
    svWidgetAttach(prv->content, background, 0, 0, 0);
    svWidgetAttach(w, prv->content, 0, 0, 3);

    if (!object) {
        svSettingsPopComponent();
        return;
    }
    if (SvObjectIsInstanceOf(object, QBFSEntry_getType())) {
        QBFSEntry entry = (QBFSEntry) object;
        QBFSEntryInfoCreateEntryWidget(prv, entry);
    }

    svSettingsPopComponent();
}

SvWidget
QBFSEntryInfoGetContentIcon(SvWidget w)
{
    QBFSEntryInfo prv = (QBFSEntryInfo) w->prv;
    return prv->contentImage;
}
