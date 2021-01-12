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

#include "QBFileBrowserMenu.h"
#include <libintl.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvStringBuffer.h>
#include <settings.h>
#include <SWL/icon.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <Menus/menuchoice.h>
#include <DataModels/loadingProxy.h>
#include "QBCarousel/QBVerticalCarousel.h"
#include <QBFSEntry.h>
#include <QBFSFile.h>
#include <QBFSTree.h>
#include <QBFSSortingFilter.h>
#include <Widgets/QBFSEntryInfo.h>
#include <Widgets/XMBCarousel.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <SWL/QBFrame.h>
#include "Widgets/QBMenuCaptionGetter.h"
#include <Logic/HotplugLogic.h>
#include "QBFileBrowserMenuChoice.h"
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <QBContentManager/QBContentStub.h>
#include "main.h"
#include <QBContentManager/QBLoadingStub.h>

struct QBFileBrowserMenuController_t {
    struct SvObject_ super_;

    AppGlobals                  appGlobals;
    QBXMBItemConstructor        itemConstructor;
    QBFrameConstructor*         focus;
    QBFrameConstructor*         inactiveFocus;
    bool                        showEPG;

    QBTextRenderer              renderer;
    unsigned int                settingsCtx;

    SvBitmap                    fileIcon;
    SvBitmap                    dirIcon;
    SvBitmap                    imageIcon;
    SvBitmap                    videoIcon;
    SvBitmap                    audioIcon;
    SvBitmap                    playlistIcon;
    SvBitmap                    noEntryIcon;
    SvRID                       fileIconRID;
    SvGenericObject             itemChoice;
};
typedef struct QBFileBrowserMenuController_t* QBFileBrowserMenuController;

static const int maxEntriesToRead = 300;

SvLocal SvWidget QBFileBrowserMenuControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState state)
{
    QBFileBrowserMenuController self = (QBFileBrowserMenuController) self_;
    QBXMBItem item = QBXMBItemCreate();

    item->caption = SVSTRING("????");
    item->loadingRID = self->fileIconRID;

    if (SvObjectIsInstanceOf(node_, QBFSEntry_getType())) {
        QBFSEntry entry = (QBFSEntry) node_;

        if (QBFSEntryGetType(entry) == QBFSEntryType_directory) {
            item->caption = SVRETAIN(QBFSEntryGetName(entry));
            item->icon = SVRETAIN(self->dirIcon);
        } else if(QBFSEntryGetType(entry) == QBFSEntryType_empty) {
            item->caption = SvStringCreate(gettext("No entries"), NULL);
            item->icon = SVRETAIN(self->noEntryIcon);
        } else {
            QBFSFile file = (QBFSFile) node_;
            QBFSFileType type = QBFSFileGetFileType(file);
            item->caption = SVRETAIN(QBFSEntryGetName(entry));
            switch(type)
            {
            case QBFSFileType_image:
                item->icon = SVRETAIN(self->imageIcon);
                break;
            case QBFSFileType_video:
                item->icon = SVRETAIN(self->videoIcon);
                break;
            case QBFSFileType_audio:
                item->icon = SVRETAIN(self->audioIcon);
                break;
            case QBFSFileType_playlist:
                item->icon = SVRETAIN(self->playlistIcon);
                break;
            default:
                item->icon = SVRETAIN(self->fileIcon);
                break;
            }
        }
    } else if (SvObjectIsInstanceOf(node_, QBLoadingStub_getType()) || SvObjectIsInstanceOf(node_, QBContentStub_getType())) {
        item->caption = SvStringCreate(gettext("Please wait, loading…"), NULL);
    }
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, state);
    SVRELEASE(item);

    return ret;
}

SvLocal void QBFileBrowserMenuVerticalCarouselSlotStateChangeHandler(SvWidget w, bool isInactive)
{
    SvWidget contentImage = QBFSEntryInfoGetContentIcon(w);
    if (contentImage)
        svIconSetState(contentImage, isInactive);
}

SvLocal SvWidget
QBFileBrowserMenuControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    QBFileBrowserMenuController self = (QBFileBrowserMenuController) self_;

    if (SvObjectIsInstanceOf(node_, QBFSEntry_getType())) {
        QBFSEntry entry = (QBFSEntry) node_;
        if (QBFSEntryGetType(entry) != QBFSEntryType_directory) {
            //QBFileBrowserMenuFileChosen
            return NULL;
        }
    }
    svSettingsRestoreContext(self->settingsCtx);

    SvWidget w = XMBCarouselNew(app, "menu");
    SvWidget carousel = XMBCarouselGetCarouselWidget(w);

    int i;
    for (i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++) {
        QBVerticalCarouselSetContentAtIndex(carousel, i, QBFSEntryInfoCreate(app));
        QBVerticalCarouselSetDataHandler(carousel, i, QBFSEntryInfoSetObject);
    }
    QBVerticalCarouselSetSlotStateChangeHandler(carousel, QBFileBrowserMenuVerticalCarouselSlotStateChangeHandler);
    svSettingsPopComponent();

    return w;
}

SvLocal void QBFileBrowserMenuControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBFileBrowserMenuController self = (QBFileBrowserMenuController) self_;

    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBFileBrowserMenuControllerDestroy(void *self_)
{
    QBFileBrowserMenuController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->itemChoice);
    SVRELEASE(self->fileIcon);
    SVRELEASE(self->dirIcon);
    SVRELEASE(self->focus);
    SVRELEASE(self->imageIcon);
    SVRELEASE(self->videoIcon);
    SVRELEASE(self->audioIcon);
    SVRELEASE(self->playlistIcon);
    SVRELEASE(self->noEntryIcon);
    SVTESTRELEASE(self->inactiveFocus);
}

SvLocal SvString QBFileBrowserMenuControllerGetCaption(SvGenericObject self_, SvGenericObject path, SvGenericObject node_)
{
    SvString caption = NULL;
    if (node_ && SvObjectIsInstanceOf(node_, QBFSEntry_getType())) {
        QBFSEntry entry = (QBFSEntry) node_;
        caption = QBFSEntryGetName(entry);
    }
    return caption;
}

SvLocal SvType QBFileBrowserMenuController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFileBrowserMenuControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t item_methods = {
        .createItem      = QBFileBrowserMenuControllerCreateItem,
        .setItemState    = QBFileBrowserMenuControllerSetItemState,
    };
    static const struct XMBMenuController_t controller_methods = {
        .createSubMenu          = QBFileBrowserMenuControllerCreateSubMenu
    };

    static const struct QBMenuCaptionGetter_t caption_methods = {
        .getMenuCaptionForPath = QBFileBrowserMenuControllerGetCaption,
    };

    if (!type) {
        SvTypeCreateManaged("QBFileBrowserMenuController",
                            sizeof(struct QBFileBrowserMenuController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBMenuController_getInterface(), &controller_methods,
                            XMBItemController_getInterface(), &item_methods,
                            QBMenuCaptionGetter_getInterface(), &caption_methods,
                            NULL);
    }

    return type;
}

typedef struct QBFileBrowserMenuHandler_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
} *QBFileBrowserMenuHandler;

SvLocal void
QBFileBrowserMenuUnregister(SvObject self_, SvWidget menu, QBActiveTree menuTree,
                            QBTreePathMap pathMap, SvObject path)
{
    QBTreePathMapRemove(pathMap, path, NULL);
    QBActiveTreeNode node           = QBActiveTreeFindNodeByPath(menuTree, path);
    SvGenericObject fileSystem      = QBActiveTreeNodeGetAttribute(node, SVSTRING("FileSystem"));

    if (fileSystem) {
        QBActiveTreeUmountSubTree(menuTree, fileSystem, NULL);
    }
    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, NULL, NULL);
}

SvLocal void
QBFileBrowserMenuRegister(SvObject self_, SvWidget menu, QBActiveTree menuTree,
                          QBTreePathMap pathMap, SvObject path,
                          SvString fsPath)
{
    QBFileBrowserMenuHandler self = (QBFileBrowserMenuHandler) self_;
    if (!fsPath)
        return;

    QBActiveTreeNode node           = QBActiveTreeFindNodeByPath(menuTree, path);
    SvGenericObject filter          = QBHotplugLogicCreateFSFilter();
    if (!filter) {
        filter = (SvGenericObject)QBFSSortingFilterCreate();
    }
    QBFSTree fsTree                 = QBFSTreeCreate(fsPath, filter, maxEntriesToRead, NULL);
    if (!fsTree) {
        SvLogError("%s() : can't create QBFSTree for path %s", __func__,
                   fsPath ? SvStringCString(fsPath) : NULL);
        SVRELEASE(filter);
        return;
    }

    QBFileBrowserMenuController itemControler = (QBFileBrowserMenuController) SvTypeAllocateInstance(
              QBFileBrowserMenuController_getType(), NULL);

    itemControler->appGlobals       = self->appGlobals;

    QBFSTreeStart(fsTree, SvSchedulerGet(), NULL);
    QBActiveTreeNodeSetAttribute(node, SVSTRING("FileSystem"), (SvGenericObject) fsTree);
    QBActiveTreeMountSubTree(menuTree, (SvGenericObject) fsTree, path, NULL);

    SVRELEASE(fsTree);
    SVRELEASE(filter);
    SvGenericObject itemChoice  = (SvGenericObject) QBFileBrowserMenuChoiceCreate(self->appGlobals);
    QBTreePathMapInsert(pathMap, path, (SvGenericObject) itemChoice, NULL);
    itemControler->itemChoice   = itemChoice;

    svSettingsPushComponent("FileBrowserMenu.settings");

    itemControler->itemConstructor  = QBXMBItemConstructorCreate("FileName", self->appGlobals->textRenderer);
    itemControler->fileIcon         = SVRETAIN(svSettingsGetBitmap("FileName", "fileIcon"));
    itemControler->dirIcon          = SVRETAIN(svSettingsGetBitmap("FileName", "dirIcon"));
    itemControler->imageIcon        = SVRETAIN(svSettingsGetBitmap("FileName", "imageIcon"));
    itemControler->videoIcon        = SVRETAIN(svSettingsGetBitmap("FileName", "videoIcon"));
    itemControler->audioIcon        = SVRETAIN(svSettingsGetBitmap("FileName", "audioIcon"));
    itemControler->playlistIcon     = SVRETAIN(svSettingsGetBitmap("FileName", "playlistIcon"));
    itemControler->noEntryIcon      = SVRETAIN(svSettingsGetBitmap("FileName", "noEntryIcon"));
    itemControler->fileIconRID      = svSettingsGetResourceID("FileName", "fileIcon");
    itemControler->focus            = QBFrameConstructorFromSM("FileName.focus");
    if (svSettingsIsWidgetDefined("FileName.inactiveFocus"))
        itemControler->inactiveFocus = QBFrameConstructorFromSM("FileName.inactiveFocus");
    itemControler->settingsCtx      = svSettingsSaveContext();
    itemControler->renderer         = self->appGlobals->textRenderer;


    svSettingsPopComponent();

    SvInvokeInterface(QBMenu, menu->prv, setItemControllerForPath, path, (SvObject) itemControler, NULL);
    SVRELEASE(itemControler);
}

SvLocal SvType
QBFileBrowserMenuHandler_getType(void)
{
    static SvType type = NULL;

    static const struct QBFileBrowserMounterHandler_ mounterHandlerMethods = {
        .menuRegister = QBFileBrowserMenuRegister,
        .menuUnregister = QBFileBrowserMenuUnregister,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBFileBrowserMenuHandler",
                            sizeof(struct QBFileBrowserMenuHandler_),
                            SvObject_getType(), &type,
                            QBFileBrowserMounterHandler_getInterface(), &mounterHandlerMethods,
                            NULL);
    }

    return type;
}

SvObject
QBFileBrowserMenuHandlerCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBFileBrowserMenuHandler self = (QBFileBrowserMenuHandler) SvTypeAllocateInstance(QBFileBrowserMenuHandler_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBFileBrowserMenuHandler");
        goto out;
    }
    self->appGlobals = appGlobals;
out:
    SvErrorInfoPropagate(error, errorOut);
    return (SvObject) self;
}
