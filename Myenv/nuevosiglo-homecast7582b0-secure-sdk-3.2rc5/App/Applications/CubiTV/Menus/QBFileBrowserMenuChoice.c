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

#include "QBFileBrowserMenuChoice.h"

#include <libintl.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <settings.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <Menus/menuchoice.h>
#include <Utils/authenticators.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBGrid.h>
#include <Widgets/authDialog.h>
#include <Widgets/eventISMovieInfo.h>
#include <Widgets/movieInfo.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Windows/imagebrowser.h>
#include <SWL/label.h>
#include <Logic/AnyPlayerLogic.h>
#include <player_hints/http_input.h>
#include <QBFSEntry.h>
#include <QBFSFile.h>
#include <Hints/hintsGetter.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include "Windows/imagebrowser/QBFSImageBrowserController.h"
#include <QBDataModel3/QBTreeModel.h>
#include <main.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBActiveArray.h>
#include <ContextMenus/QBListPane.h>
#include <ContextMenus/QBContainerPane.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <QBWidgets/QBComboBox.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <QBOSK/QBOSKKey.h>
#include <SvFoundation/SvData.h>
#include <SvPlayerKit/SvContent.h>
#include <Logic/QBParentalControlLogic.h>

struct QBFileBrowserMenuChoice_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    struct {
        QBContextMenu ctx;
    } sidemenu;
    unsigned int countImages;
    unsigned int countMusic;
    unsigned int countPlaylist;
    SvObject parentPath;
    int position;
    QBFSEntry lastEntry;
    QBActiveArray selectedImages;
    QBActiveArray selectedMusic;
    QBActiveArray allFiles;
    QBBasicPaneItem activeOption;
    QBBasicPane activeSubMenu;
    QBContainerPane durationPane;
    SvWidget durationEdit;
    SvWidget duration;
    int lastSlideshowDuration;
    SvString slideshowMusicPlaylist;
    bool slideshowMusicAll;
};

//Playlist creation needs write access on USB devices
#define CREATE_PLAYLIST_SUPPORT 0
#define MAX_PLAYLIST_COUNT_IN_SIDEMENU 30
#define MAX_FILES_COUNT_IN_SIDEMENU 300

SvLocal void QBFileBrowserMenuChoiceUpdateParentPath(QBFileBrowserMenuChoice self, SvGenericObject nodePath)
{
    SVTESTRELEASE(self->parentPath);
    self->parentPath = SvObjectCopy(nodePath, NULL);
    SvInvokeInterface(QBTreePath, self->parentPath, truncate, -1);
}

SvLocal void QBFileBrowserMenuChoiceUpdatePosition(QBFileBrowserMenuChoice self, SvGenericObject nodePath_, SvGenericObject node_)
{
    AppGlobals appGlobals = self->appGlobals;
    SvGenericObject lpath;
    SvGenericObject lnode;
    self->position = 0;
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, appGlobals->menuTree,
                                            getIterator, self->parentPath, 0);
    do {
        lpath = QBTreeIteratorGetCurrentNodePath(&iter);
        if (nodePath_ && SvObjectEquals(lpath, nodePath_))
            break;
        lnode = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, lpath);
        if (node_ && SvObjectEquals(lnode, node_))
            break;
        self->position++;
    } while (QBTreeIteratorGetNextNode(&iter));
}

SvLocal void QBFileBrowserPlay(QBFileBrowserMenuChoice self, QBFSEntry entry)
{
    QBFSFileType type = QBFSFileGetFileType((QBFSFile) entry);
    SvString URI = QBFSEntryCreateFullPathString(entry);

    SvContent c = NULL;
    c = SvContentCreateFromCString(SvStringCString(URI), NULL);

    SvContentMetaDataSetStringProperty(SvContentGetMetaData(c), SVSTRING(SV_PLAYER_META__TITLE), QBFSEntryGetName(entry));

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);
    QBPVRPlayerContextSetTitle(pvrPlayer, SVSTRING("File"));
    QBPVRPlayerSetContentSourceType(pvrPlayer, QBParentalControlLogicContentSource_USB);

    if (self->appGlobals->remoteControl) {
        QBPlaybackStateController playbackStateController = QBRemoteControlCreatePlaybackStateController(self->appGlobals->remoteControl, NULL);
        QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
        SVRELEASE(playbackStateController);
    }

    if (type != QBFSFileType_playlist && self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, URI);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, URI, 0.0, QBBookmarkType_Generic);

        if (bookmark) {
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
        }
    }

    SVRELEASE(anyPlayerLogic);

    switch (type) {
        case QBFSFileType_audio:
            QBPVRPlayerContextSetMusic(pvrPlayer, c);
            break;
        case QBFSFileType_playlist:
            QBPVRPlayerContextSetMusicPlaylist(pvrPlayer, URI);
            break;
        case QBFSFileType_video:
            QBPVRPlayerContextSetContent(pvrPlayer, c);
            break;
        default:
            break;
    }

    SVRELEASE(URI);
    SVTESTRELEASE(c);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);
}

SvLocal bool QBFileBrowserIsRegularFile(SvGenericObject node)
{
    if (!SvObjectIsInstanceOf(node, QBFSEntry_getType()) || SvObjectIsInstanceOf(node, QBFSEmptyEntry_getType())) {
        return false;
    }

    return QBFSEntryGetType((QBFSEntry) node) == QBFSEntryType_regularFile;
}

SvLocal void QBFileBrowserMenuChoosen(SvGenericObject self_, SvGenericObject node, SvGenericObject nodePath_, int position)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    if (!QBFileBrowserIsRegularFile(node)) {
        return;
    }

    QBFSEntry entry = (QBFSEntry) node;

    QBFSFileType type = QBFSFileGetFileType((QBFSFile) entry);
    if (type == QBFSFileType_unsupported)
        return;

    if (type == QBFSFileType_image) {
        QBFileBrowserMenuChoiceUpdateParentPath(self, nodePath_);
        QBFSImageBrowserController controller = QBFSImageBrowserControllerCreate(self->appGlobals, NULL, NULL);
        QBWindowContext imageBrowser = QBImageBrowserContextCreate(self->appGlobals, self->parentPath, position, (SvGenericObject) controller, false, -1);
        QBApplicationControllerPushContext(self->appGlobals->controller, imageBrowser);
        SVRELEASE(imageBrowser);
        SVRELEASE(controller);
    } else {
        QBFileBrowserPlay(self, entry);
    }
}

SvLocal void QBFileBrowserMenuChoiceCheckContent(SvGenericObject self_, SvGenericObject path)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;
    self->countPlaylist = self->countImages = self->countMusic = 0;
    SvGenericObject parentPath = SvObjectCopy(path, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree,
                                            getIterator, parentPath, 0);
    do {
        SvGenericObject nodePath = QBTreeIteratorGetCurrentNodePath(&iter);
        SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
        if (!QBFileBrowserIsRegularFile(node))
            continue;
        QBFSEntry entry = (QBFSEntry) node;
        QBFSFileType type = QBFSFileGetFileType((QBFSFile) entry);
        switch (type) {
            case QBFSFileType_audio:
                self->countMusic++;
                break;
            case QBFSFileType_image:
                self->countImages++;
                break;
            case QBFSFileType_playlist:
                self->countPlaylist++;
                break;
            default:
                break;
        }
    } while (QBTreeIteratorGetNextNode(&iter));
    SVRELEASE(parentPath);
}

SvLocal SvString QBFileBrowserGetHintsForPath(SvGenericObject self_, SvGenericObject path)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;
    QBFileBrowserMenuChoiceCheckContent(self_, path);
    if (self->countImages || self->countMusic)
        return SVSTRING("filebrowser_hint");
    return NULL;
}

SvLocal void QBFileBrowserMenuContextHide(QBFileBrowserMenuChoice self)
{
    SVTESTRELEASE(self->lastEntry);
    SVTESTRELEASE(self->parentPath);
    SVTESTRELEASE(self->selectedImages);
    SVTESTRELEASE(self->selectedMusic);
    SVTESTRELEASE(self->allFiles);
    SVTESTRELEASE(self->slideshowMusicPlaylist);
    SVTESTRELEASE(self->activeSubMenu);
    self->selectedImages = NULL;
    self->selectedMusic = NULL;
    self->allFiles = NULL;
    self->activeOption = NULL;
    self->activeSubMenu = NULL;
    self->lastEntry = NULL;
    self->parentPath = NULL;
    self->slideshowMusicPlaylist = NULL;
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBFileBrowserMenuContextChoosenPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = self_;
    SvString URI = QBFSEntryCreateFullPathString(self->lastEntry);
    QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, URI);
    SVRELEASE(URI);
    QBBookmarkUpdatePosition(bookmark, 0.0);
    QBFileBrowserPlay(self, self->lastEntry);
    QBFileBrowserMenuContextHide(self);
}

SvLocal SvArray
QBFileBrowserMenuContextGetAllMusic(void *self_)
{
    QBFileBrowserMenuChoice self = self_;

    SvArray music = SvArrayCreateWithCapacity(self->countMusic, NULL);
    if (!music)
        return NULL;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree,
                                            getIterator, self->parentPath, 0);
    do {
        SvObject path = QBTreeIteratorGetCurrentNodePath(&iter);
        SvObject entry = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
        if (QBFSEntryGetType((QBFSEntry) entry) == QBFSEntryType_regularFile &&
            QBFSFileGetFileType((QBFSFile) entry) == QBFSFileType_audio) {
            SvString fullpath = QBFSEntryCreateFullPathString((QBFSEntry) entry);
            SvContent content = SvContentCreateFromCString(SvStringGetCString(fullpath), NULL);
            SVRELEASE(fullpath);
            if (content) {
                SvArrayAddObject(music, (SvObject) content);
                SVRELEASE(content);
            }
        }
    } while (QBTreeIteratorGetNextNode(&iter));
    return music;
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowStart(void *self_, QBActiveArray filter)
{
    QBFileBrowserMenuChoice self = self_;
    QBFSImageBrowserController controller = QBFSImageBrowserControllerCreate(self->appGlobals, filter, NULL);

    if (filter && QBActiveArrayCount(filter)) {
        //if we have playlist then start from first image
        SVTESTRELEASE(self->lastEntry);
        self->lastEntry = SVRETAIN(QBActiveArrayObjectAtIndex(filter, 0));
        QBFileBrowserMenuChoiceUpdatePosition(self, NULL, (SvObject) self->lastEntry);
    } else {
        //make sure self->position points to first image
        QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree,
                                                getIterator, self->parentPath, 0);
        do {
            SvObject path = QBTreeIteratorGetCurrentNodePath(&iter);
            SvObject entry = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
            if (QBFSEntryGetType((QBFSEntry) entry) == QBFSEntryType_regularFile &&
                QBFSFileGetFileType((QBFSFile) entry) == QBFSFileType_image) {
                SVTESTRELEASE(self->lastEntry);
                self->lastEntry = SVRETAIN(entry);
                QBFileBrowserMenuChoiceUpdatePosition(self, NULL, (SvObject) self->lastEntry);
                break;
            }
        } while (QBTreeIteratorGetNextNode(&iter));
    }
    QBWindowContext imageBrowser = QBImageBrowserContextCreate(self->appGlobals, self->parentPath, self->position, (SvGenericObject) controller, true, self->lastSlideshowDuration);
    if (self->slideshowMusicAll) {
        QBImageBrowserContextSetBackgroundMusicPlaylist(imageBrowser, QBFileBrowserMenuContextGetAllMusic(self));
    } else
        QBImageBrowserContextSetBackgroundMusicM3uPlaylist(imageBrowser, self->slideshowMusicPlaylist);
    QBApplicationControllerPushContext(self->appGlobals->controller, imageBrowser);
    SVRELEASE(imageBrowser);
    SVRELEASE(controller);
    QBFileBrowserMenuContextHide(self);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowAll(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuContextChoosenSlideshowStart(self_, NULL);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowSelected(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = self_;
    if (item->itemDisabled)
        return;
    QBFileBrowserMenuContextChoosenSlideshowStart(self_, self->selectedImages);
}

SvLocal ChannelChoiceTickState QBFileBrowserMenuContextIsTicked(void *self_, SvGenericObject node)
{
    QBFileBrowserMenuChoice self = self_;
    if ((self->selectedImages && QBActiveArrayIndexOfObject(self->selectedImages, node, NULL) != -1) ||
        (self->selectedMusic && QBActiveArrayIndexOfObject(self->selectedMusic, node, NULL) != -1))
        return ChannelChoiceTickState_On;
    return ChannelChoiceTickState_Off;
}

SvLocal SvString QBFileBrowserMenuContextCreateCaption(void *self_, SvGenericObject node)
{
    QBFSEntry entry = (QBFSEntry) node;
    SvString name = QBFSEntryGetName(entry);
    SVRETAIN(name);
    return name;
}

/** Called i.e. when we select file (there may be yes-check GUI widget also). */
SvLocal void QBFileBrowserMenuContextToggle(void *self_, QBListPane pane, SvGenericObject node, int pos)
{
    QBFileBrowserMenuChoice self = self_;
    QBFSFileType type = QBFSFileGetFileType((QBFSFile) node);
    QBActiveArray selected = NULL;

    if (type == QBFSFileType_image)
        selected = self->selectedImages;
    else if (type == QBFSFileType_audio)
        selected = self->selectedMusic;
    else
        return;

    ssize_t idx = QBActiveArrayIndexOfObject(selected, node, NULL);
    if (idx != -1)
        QBActiveArrayRemoveObjectAtIndex(selected, idx, NULL);
    else
        QBActiveArrayAddObject(selected, node, NULL);

    self->activeOption->itemDisabled = QBActiveArrayCount(selected) == 0;
    // self->activeSubMenu can not be NULL here
    assert(self->activeSubMenu != NULL);
    QBBasicPaneOptionPropagateObjectChange(self->activeSubMenu, self->activeOption);

    idx = QBActiveArrayIndexOfObject(self->allFiles, node, NULL);
    QBActiveArrayPropagateObjectsChange(self->allFiles, idx, 1, NULL);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowSelectToPlaylist(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;
    QBFSFileType requiredType = QBFSFileType_unsupported;

    SVTESTRELEASE(self->allFiles);
    self->allFiles = QBActiveArrayCreate(16, NULL);

    if (SvStringEqualToCString(id, "selectimages")) {
        requiredType = QBFSFileType_image;
        if (!self->selectedImages)
            self->selectedImages = QBActiveArrayCreate(16, NULL);
    } else if (SvStringEqualToCString(id, "selectmusic")) {
        requiredType = QBFSFileType_audio;
        if (!self->selectedMusic)
            self->selectedMusic = QBActiveArrayCreate(16, NULL);
    } else {
        return;
    }

    unsigned int count = 0;
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree,
                                            getIterator, self->parentPath, 0);
    do {
        SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, QBTreeIteratorGetCurrentNodePath(&iter));
        QBFSEntry entry = (QBFSEntry) node;
        QBFSFileType type = QBFSFileGetFileType((QBFSFile) entry);
        if (type == requiredType) {
            QBActiveArrayAddObject(self->allFiles, node, NULL);
            if (++count >= MAX_FILES_COUNT_IN_SIDEMENU)
                break;
        }
    } while (QBTreeIteratorGetNextNode(&iter));

    QBListPane content = (QBListPane) SvTypeAllocateInstance(QBListPane_getType(), NULL);
    static struct QBChannelChoiceControllerCallbacks_t callbacks = {
        .isTicked          = QBFileBrowserMenuContextIsTicked,
        .createNodeCaption = QBFileBrowserMenuContextCreateCaption
    };
    QBChannelChoiceController constructor = QBChannelChoiceControllerCreateFromSettings("ChannelChoiceMenu.settings", self->appGlobals->textRenderer, (SvObject) self->allFiles, self, &callbacks);

    static struct QBListPaneCallbacks_t listCallbacks = {
        .selected = QBFileBrowserMenuContextToggle
    };
    svSettingsPushComponent("ListPane.settings");
    QBListPaneInit(content, self->appGlobals->res, (SvGenericObject) self->allFiles, (SvGenericObject) constructor, &listCallbacks, self, self->sidemenu.ctx, 3, SVSTRING("ListPane"));
    svSettingsPopComponent();
    SVRELEASE(constructor);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) content);
    SVRELEASE(content);
}

struct QBDurationInput_t {
    SvWidget w;
    SvWidget comboBox;
    int durationStep;
    int inputPos;
    int minDuration;
    int maxDuration;
};
typedef struct QBDurationInput_t *QBDurationInput;

SvLocal SvString
QBFileBrowserMenuContextChoosenSlideshowDurationPrepare(void *self_, SvWidget combobox, SvGenericObject value)
{
    QBDurationInput self = self_;
    int duration = SvValueGetInteger((SvValue) value);

    if (duration < self->minDuration)
        duration = self->minDuration;
    else if (duration >= self->maxDuration)
        duration = self->maxDuration;

    return SvStringCreateWithFormat(gettext("%i secs"), duration);
}

SvLocal bool
QBFileBrowserMenuContextChoosenSlideshowDurationInput(void *target, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBDurationInput self = target;
    if (key < '0' || key > '9')
        return false;
    int digit = key - '0';
    int durationMax = self->maxDuration;

    SvValue value = (SvValue) QBComboBoxGetValue(combobox);
    int durationValue = self->inputPos ? SvValueGetInteger(value) : 0;
    if (durationValue > durationMax / 10)
        return true;
    durationValue = durationValue * 10 + digit;
    self->inputPos = 1;

    value = SvValueCreateWithInteger(durationValue, NULL);
    QBComboBoxSetValue(combobox, (SvGenericObject) value);
    SVRELEASE(value);

    return true;
}

SvLocal int
QBFileBrowserMenuContextChoosenSlideshowDurationChange(void *target, SvWidget combobox, SvGenericObject value, int key)
{
    QBDurationInput self = target;
    self->inputPos = 0;
    int duration = SvValueGetInteger((SvValue) value);
    duration += key == QBKEY_RIGHT ? self->durationStep : -self->durationStep;
    return (duration - self->minDuration) / self->durationStep;
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationCurrent(void *target, SvWidget combobox, SvGenericObject value)
{
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationFinish(void *target, SvWidget combobox, SvGenericObject selectedValue, SvGenericObject userValue)
{
}

SvLocal SvWidget
QBFileBrowserMenuContextChoosenSlideshowDurationInputNew(AppGlobals appGlobals, const char *widgetName)
{
    SvWidget w = svSettingsWidgetCreate(appGlobals->res, widgetName);
    QBDurationInput prv = malloc(sizeof(*prv));

    w->prv = prv;
    char nameBuf[1024];
    snprintf(nameBuf, sizeof(nameBuf), "%s.comboBox", widgetName);
    prv->comboBox = QBComboBoxNewFromSM(appGlobals->res, nameBuf);
    svWidgetSetFocusable(prv->comboBox, true);
    QBComboBoxCallbacks cb = { QBFileBrowserMenuContextChoosenSlideshowDurationPrepare,
                               QBFileBrowserMenuContextChoosenSlideshowDurationInput,
                               QBFileBrowserMenuContextChoosenSlideshowDurationChange,
                               QBFileBrowserMenuContextChoosenSlideshowDurationCurrent,
                               NULL,
                               QBFileBrowserMenuContextChoosenSlideshowDurationFinish };
    QBComboBoxSetCallbacks(prv->comboBox, prv, cb);

    svSettingsPushComponent("ImageBrowser.settings");
    prv->minDuration = svSettingsGetDouble("ImageBrowser", "slideshowDurationMin", 1.0);
    prv->maxDuration = svSettingsGetDouble("ImageBrowser", "slideshowDurationMax", 10.0);
    prv->durationStep = svSettingsGetDouble("ImageBrowser", "durationChangeStep", 1.0);
    svSettingsPopComponent();

    SvArray values = SvArrayCreate(NULL);
    for (int idx = prv->minDuration; idx <= prv->maxDuration; idx += prv->durationStep) {
        SvGenericObject v = (SvGenericObject) SvValueCreateWithInteger(idx, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(prv->comboBox, values);
    SVRELEASE(values);

    svSettingsWidgetAttach(w, prv->comboBox, nameBuf, 1);

    return w;
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    if (!self->durationEdit)
        return;

    svWidgetDetach(self->durationEdit);
    svWidgetDestroy(self->durationEdit);
    self->durationEdit = NULL;
    SVRELEASE(self->durationPane);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    if (!self->durationEdit)
        return;

    svWidgetAttach(frame, self->durationEdit, self->durationEdit->off_x, self->durationEdit->off_y, 0);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    if (!self->durationEdit) {
        return;
    }
    QBDurationInput prv = (QBDurationInput) self->duration->prv;
    svWidgetSetFocus(prv->comboBox);
}

SvLocal void fakeClean(SvApplication app, void *prv)
{
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowDurationSetDuration(SvWidget w, int value)
{
    QBDurationInput prv = (QBDurationInput) w->prv;
    SvValue val = SvValueCreateWithInteger(value, NULL);
    QBComboBoxSetValue(prv->comboBox, (SvGenericObject) val);
    SVRELEASE(val);
}

SvLocal bool
QBFileBrowserMenuContextChoosenSlideshowDurationEditInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBFileBrowserMenuChoice self = w->prv;

    if (e->ch == QBKEY_ENTER) {
        QBDurationInput prv = (QBDurationInput) self->duration->prv;
        SvValue value = (SvValue) QBComboBoxGetValue(prv->comboBox);
        self->lastSlideshowDuration = SvValueGetInteger(value);
        QBContextMenuPopPane(self->sidemenu.ctx);
        return true;
    } else if (e->ch == QBKEY_DOWN) {
        return true;
    }

    return false;
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowSetDuration(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBFileBrowserMenuContextChoosenSlideshowDurationOnHide,
        .onShow    = QBFileBrowserMenuContextChoosenSlideshowDurationOnShow,
        .setActive = QBFileBrowserMenuContextChoosenSlideshowDurationSetActive
    };

    svSettingsPushComponent("FileBrowserMenu.settings");
    SvWidget duration = QBFileBrowserMenuContextChoosenSlideshowDurationInputNew(self->appGlobals, "slideshowduration");
    if (!duration) {
        svSettingsPopComponent();
        return;
    }

    SvWidget durationEdit = svSettingsWidgetCreate(self->appGlobals->res, "slideshowdurationedit");
    svSettingsWidgetAttach(durationEdit, duration, "slideshowduration", 1);
    durationEdit->off_x = svSettingsGetInteger("slideshowdurationedit", "xOffset", 0);
    durationEdit->off_y = svSettingsGetInteger("slideshowdurationedit", "yOffset", 0);
    durationEdit->prv = self;
    durationEdit->clean = fakeClean;
    svWidgetSetInputEventHandler(durationEdit, QBFileBrowserMenuContextChoosenSlideshowDurationEditInputEventHandler);

    SvWidget w = QBAsyncLabelNew(self->appGlobals->res, "slideshowduration.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(durationEdit, w, svWidgetGetName(w), 1);
    QBAsyncLabelSetCText(w, gettext("Duration"));

    if (self->lastSlideshowDuration < 1) {
        svSettingsPushComponent("ImageBrowser.settings");
        self->lastSlideshowDuration = svSettingsGetDouble("ImageBrowser", "slideshowDuration", 3.0);
        svSettingsPopComponent();
    }

    QBFileBrowserMenuContextChoosenSlideshowDurationSetDuration(duration, self->lastSlideshowDuration);
    self->durationEdit = durationEdit;
    self->duration = duration;
    svSettingsPopComponent();

    self->durationPane = QBContainerPaneCreateFromSettings("CalcPane.settings", self->appGlobals->res,
                                                           self->sidemenu.ctx, 3, SVSTRING("CalcPane"), &callbacks, self);
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->durationPane);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowSetMusic(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    SVTESTRELEASE(self->slideshowMusicPlaylist);
    self->slideshowMusicAll = false;
    self->slideshowMusicPlaylist = NULL;
    if (SvStringEqualToCString(id, "allmusic"))
        self->slideshowMusicAll = true;
    else if (!SvStringEqualToCString(id, "nomusic"))
        self->slideshowMusicPlaylist = SVRETAIN(id);
    QBContextMenuPopPane(self->sidemenu.ctx);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshowSelectMusic(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane music = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(music, self->appGlobals->res, self->appGlobals->scheduler,
                    self->appGlobals->textRenderer, self->sidemenu.ctx, 3, SVSTRING("BasicPane"));

    if (self->slideshowMusicPlaylist || self->slideshowMusicAll) {
        SvString option = SvStringCreate(gettext("No Music"), NULL);
        QBBasicPaneAddOption(music, SVSTRING("nomusic"), option, QBFileBrowserMenuContextChoosenSlideshowSetMusic, self);
        SVRELEASE(option);
    }
    if (self->countMusic) {
        SvString option = SvStringCreate(gettext("All Music"), NULL);
        QBBasicPaneAddOption(music, SVSTRING("allmusic"), option, QBFileBrowserMenuContextChoosenSlideshowSetMusic, self);
        SVRELEASE(option);
    }

    unsigned int count = 0;
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree,
                                            getIterator, self->parentPath, 0);
    do {
        SvObject path = QBTreeIteratorGetCurrentNodePath(&iter);
        QBFSEntry entry = (QBFSEntry) SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
        if (QBFSEntryGetType(entry) == QBFSEntryType_regularFile &&
            QBFSFileGetFileType((QBFSFile) entry) == QBFSFileType_playlist) {
            SvString fullPath = QBFSEntryCreateFullPathString(entry);
            QBBasicPaneAddOption(music, fullPath, QBFSEntryGetName(entry),
                                 QBFileBrowserMenuContextChoosenSlideshowSetMusic, self);
            SVRELEASE(fullPath);
            if (++count >= MAX_PLAYLIST_COUNT_IN_SIDEMENU)
                break;
        }
    } while (QBTreeIteratorGetNextNode(&iter));
    svSettingsPopComponent();
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) music);
    SVRELEASE(music);
}

SvLocal void
QBFileBrowserMenuContextChoosenSlideshow(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    SVTESTRELEASE(self->activeSubMenu);

    svSettingsPushComponent("BasicPane.settings");
    self->activeSubMenu = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->activeSubMenu, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));

    SvString option = SvStringCreate(gettext("All files"), NULL);
    QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("allfiles"), option, QBFileBrowserMenuContextChoosenSlideshowAll, self);
    SVRELEASE(option);
    if (self->countImages <= MAX_FILES_COUNT_IN_SIDEMENU) {
        option = SvStringCreate(gettext("Select images"), NULL);
        QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("selectimages"), option, QBFileBrowserMenuContextChoosenSlideshowSelectToPlaylist, self);
        SVRELEASE(option);
    }
    option = SvStringCreate(gettext("Show selected"), NULL);
    self->activeOption = QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("showselected"), option, QBFileBrowserMenuContextChoosenSlideshowSelected, self);
    self->activeOption->itemDisabled = (QBActiveArrayCount(self->selectedImages) == 0);
    QBBasicPaneOptionPropagateObjectChange(self->activeSubMenu, self->activeOption);
    SVRELEASE(option);
    option = SvStringCreate(gettext("Set duration"), NULL);
    QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("setduration"), option, QBFileBrowserMenuContextChoosenSlideshowSetDuration, self);
    SVRELEASE(option);
    if (self->countPlaylist || self->countMusic) {
        option = SvStringCreate(gettext("Background music"), NULL);
        QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("selectmusic"), option, QBFileBrowserMenuContextChoosenSlideshowSelectMusic, self);
        SVRELEASE(option);
    }

    svSettingsPopComponent();

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->activeSubMenu);
}

#if CREATE_PLAYLIST_SUPPORT
SvLocal void
QBFileBrowserMenuContextChoosenPlaylistSave(QBFileBrowserMenuChoice self, SvString filename)
{
    SvErrorInfo error = NULL;
    SvStringBuffer buff = SvStringBufferCreate(NULL);
    if (!buff)
        return;
    SvIterator iter = QBActiveArrayIterator(self->selectedMusic);
    QBFSEntry next = NULL;
    do {
        next = (QBFSEntry) SvIteratorGetNext(&iter);
        SvStringBufferAppendFormatted(buff, &error, "%s\n", SvStringGetCString(QBFSEntryGetName(next)));
        if (error)
            break;
    } while (SvIteratorHasNext(&iter));

    if (!error) {
        SvString contentString = SvStringBufferCreateContentsString(buff, NULL);
        SvString fullpath = SvStringCreateWithFormat("%s/%s.m3u", SvStringGetCString(QBFSEntryGetRootDirPath(next)), SvStringGetCString(filename));
        if (contentString && fullpath) {
            SvData fileContent = SvDataCreateWithBytesAndLength(SvStringGetCString(contentString), SvStringGetLength(contentString), NULL);
            if (fileContent) {
                SvDataWriteToFile(fileContent, SvStringGetCString(fullpath), true, NULL);
                SVRELEASE(fileContent);
                QBActiveTreePropagateSubTreeChange(self->appGlobals->menuTree, self->parentPath, NULL);
            }
        }
        SVTESTRELEASE(contentString);
        SVTESTRELEASE(fullpath);
    }
    SVRELEASE(buff);
    SvErrorInfoPropagate(error, NULL);
}

SvLocal void QBFileBrowserMenuContextChoosenCreatePlaylistOSKKeyPressed(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    if (key->type == QBOSKKeyType_enter) {
        if (SvStringGetLength(input) == 0)
            return;
        QBFileBrowserMenuContextChoosenPlaylistSave(self, input);
        QBContextMenuHide(self->sidemenu.ctx, false);
    }
}

SvLocal void
QBFileBrowserMenuContextChoosenCreatePlaylist(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;

    SVTESTRELEASE(self->activeSubMenu);

    svSettingsPushComponent("BasicPane.settings");
    self->activeSubMenu = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->activeSubMenu, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 2, SVSTRING("BasicPane"));

    SvString option = SvStringCreate(gettext("Select music"), NULL);
    QBBasicPaneAddOption(self->activeSubMenu, SVSTRING("selectmusic"), option, QBFileBrowserMenuContextChoosenSlideshowSelectToPlaylist, self);
    SVRELEASE(option);

    option = SvStringCreate(gettext("Save"), NULL);
    svSettingsPushComponent("OSKPane.settings");
    self->activeOption = QBBasicPaneAddOptionWithOSK(self->activeSubMenu, SVSTRING("showselected"), option, SVSTRING("OSKPane"), QBFileBrowserMenuContextChoosenCreatePlaylistOSKKeyPressed, self);
    svSettingsPopComponent();
    SvWidget title = QBOSKPaneGetTitle((QBOSKPane) self->activeOption->subpane);
    svLabelSetText(title, gettext("Playlist name"));
    self->activeOption->itemDisabled = QBActiveArrayCount(self->selectedMusic) == 0;
    QBBasicPaneOptionPropagateObjectChange(self->activeSubMenu, self->activeOption);
    SVRELEASE(option);

    svSettingsPopComponent();
    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) self->activeSubMenu);
}
#endif

SvLocal void QBFileBrowserMenuContextChoosen(SvGenericObject self_, SvGenericObject nodePath)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) self_;
    SvGenericObject node = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, nodePath);
    QBFSEntry entry = (QBFSEntry) node;
    QBFSFileType type = QBFSFileType_unsupported;

    if (QBFileBrowserIsRegularFile(node))
        type = QBFSFileGetFileType((QBFSFile) entry);

    QBFileBrowserMenuChoiceUpdateParentPath(self, nodePath);
    QBFileBrowserMenuChoiceUpdatePosition(self, nodePath, NULL);

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));

    if (self->countImages) {
        SvString option = SvStringCreate(gettext("Slideshow"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("slideshow"), option, QBFileBrowserMenuContextChoosenSlideshow, self);
        SVRELEASE(option);
    }
#if CREATE_PLAYLIST_SUPPORT
    if (self->countMusic) {
        SvString option = SvStringCreate(gettext("Create playlist"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("createplaylist"), option, QBFileBrowserMenuContextChoosenCreatePlaylist, self);
        SVRELEASE(option);
    }
#endif
    if (QBFileBrowserIsRegularFile(node) && (type == QBFSFileType_audio || type == QBFSFileType_video) && self->appGlobals->bookmarkManager) {
        SvString URI = QBFSEntryCreateFullPathString(entry);
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, URI);
        SVRELEASE(URI);
        if (bookmark && QBBookmarkGetPosition(bookmark) > 0.00) {
            svSettingsPushComponent("BasicPane.settings");
            SvString option = SvStringCreate(gettext("Play from the beginning"), NULL);
            QBBasicPaneAddOption(options, SVSTRING("playFromTheBeginning"), option, QBFileBrowserMenuContextChoosenPlay, self);
            SVRELEASE(option);
            svSettingsPopComponent();
        }
    }

    if (QBBasicPaneGetOptionsCount(options) > 0) {
        QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) options);
        QBContextMenuShow(self->sidemenu.ctx);
    } else {
        SvLogNotice("%s() :: Cannot show pane without any option.", __func__);
    }
    svSettingsPopComponent();
    SVRELEASE(options);

    if (QBFileBrowserIsRegularFile(node) && (type == QBFSFileType_audio || type == QBFSFileType_video || type == QBFSFileType_image))
        self->lastEntry = SVRETAIN(entry);
}

SvLocal void
QBFileBrowserMenuChoiceDestroy(void *self_)
{
    QBFileBrowserMenuChoice self = self_;

    if (self->sidemenu.ctx) {
        QBContextMenuSetCallbacks(self->sidemenu.ctx, NULL, NULL);
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }

    SVTESTRELEASE(self->activeSubMenu);
    SVTESTRELEASE(self->lastEntry);
}

SvLocal SvType QBFileBrowserMenuChoice_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBFileBrowserMenuChoiceDestroy
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBFileBrowserMenuChoosen
    };

    static const struct QBHintsGetter_t hintsMethods = {
        .getHintsForPath = QBFileBrowserGetHintsForPath
    };

    static const struct QBMenuContextChoice_t contextMethods = {
        .contextChoosen = QBFileBrowserMenuContextChoosen
    };

    if (!type) {
        SvTypeCreateManaged("QBFileBrowserMenuChoice",
                            sizeof(struct QBFileBrowserMenuChoice_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBHintsGetter_getInterface(), &hintsMethods,
                            QBMenuContextChoice_getInterface(), &contextMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBFileBrowserMenuHandlerOnSideMenuClose(void *self_, QBContextMenu ctx)
{
    QBFileBrowserMenuContextHide((QBFileBrowserMenuChoice) self_);
}

QBFileBrowserMenuChoice QBFileBrowserMenuChoiceCreate(AppGlobals appGlobals)
{
    QBFileBrowserMenuChoice self = (QBFileBrowserMenuChoice) SvTypeAllocateInstance(QBFileBrowserMenuChoice_getType(), NULL);

    self->appGlobals = appGlobals;
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    self->lastSlideshowDuration = -1;
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBFileBrowserMenuHandlerOnSideMenuClose, self);
    return self;
}
