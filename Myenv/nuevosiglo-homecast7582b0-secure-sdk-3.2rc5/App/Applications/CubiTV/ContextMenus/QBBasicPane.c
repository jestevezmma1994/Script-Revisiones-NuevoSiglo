/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBBasicPaneInternal.h"
#include <libintl.h>
#include "QBBasicPane.h"
#include <SvCore/SvCommonDefs.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/SideMenu.h>
#include <safeFiles.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>

#define log_error(FMT, ...) SvLogError("QBBasicPane " FMT, ##__VA_ARGS__)

struct QBBasicPaneItemConstructor_t {
    struct SvObject_ super_;
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focusConstructor;
    QBFrameConstructor* disabledFocusConstructor;
};

struct QBBasicPaneItemParams_t {
    struct SvObject_ super_;
    SvString id;
    SvString text;
    SvHashTable metadata;
    bool withSubpane;
};


SvLocal SvGenericObject QBBasicPaneReadItemParams(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut);
SvLocal QBBasicPaneItemParams QBBasicPaneGetItemParams(QBBasicPane self, SvString id);

SvLocal void QBBasicPaneItem__dtor__(void *ptr)
{
    QBBasicPaneItem self = ptr;
    SVTESTRELEASE(self->subpane);
    SVTESTRELEASE(self->metadata);
    SVRELEASE(self->caption);
    SVTESTRELEASE(self->subcaption);
    SVTESTRELEASE(self->id);
}

SvType QBBasicPaneItem_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBasicPaneItem__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBasicPaneItem",
                            sizeof(struct QBBasicPaneItem_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal SvWidget QBBasicPaneItemConstructorCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBBasicPaneItem_getType()))
        return NULL;

    QBBasicPaneItemConstructor self = (QBBasicPaneItemConstructor) self_;
    QBBasicPaneItem item = (QBBasicPaneItem) node_;
    QBXMBItem itemCtor = QBXMBItemCreate();
    itemCtor->caption = SVRETAIN(item->caption);
    itemCtor->subcaption = SVTESTRETAIN(item->subcaption);
    itemCtor->focus = SVRETAIN(self->focusConstructor);
    itemCtor->disabledFocus = SVTESTRETAIN(self->disabledFocusConstructor);
    itemCtor->disabled = item->itemDisabled;

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, itemCtor, app, initialState);
    SVRELEASE(itemCtor);

    return ret;
}

SvLocal void QBBasicPaneItemConstructorSetItemState(SvGenericObject self_, SvWidget item, XMBMenuState menuState, bool isFocused)
{
    QBBasicPaneItemConstructor self = (QBBasicPaneItemConstructor) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item, menuState, isFocused);
}

SvLocal void QBBasicPaneItemConstructor__dtor__(void *ptr)
{
    QBBasicPaneItemConstructor self = ptr;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focusConstructor);
    SVTESTRELEASE(self->disabledFocusConstructor);
}

SvLocal SvType QBBasicPaneItemConstructor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBasicPaneItemConstructor__dtor__
    };
    static SvType type = NULL;
    static const struct XMBItemController_t methods = {
        .createItem = QBBasicPaneItemConstructorCreateItem,
        .setItemState = QBBasicPaneItemConstructorSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBasicPaneItemConstructor",
                            sizeof(struct QBBasicPaneItemConstructor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal void QBBasicPaneItemParams__dtor__(void *ptr)
{
    QBBasicPaneItemParams self = ptr;
    SVRELEASE(self->id);
    SVRELEASE(self->text);
    SVTESTRELEASE(self->metadata);
}

SvLocal SvType QBBasicPaneItemParams_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBasicPaneItemParams__dtor__
    };
    static SvType type = NULL;
    if(unlikely(!type)) {
        SvTypeCreateManaged("QBBasicPaneItemParams",
                            sizeof(struct QBBasicPaneItemParams_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}
SvLocal QBBasicPaneItemParams QBBasicPaneItemParamsCreate(SvString id, SvString text, SvHashTable metadata)
{
    QBBasicPaneItemParams self = (QBBasicPaneItemParams) SvTypeAllocateInstance(QBBasicPaneItemParams_getType(), NULL);

    self->id = SVTESTRETAIN(id);
    self->text = SVTESTRETAIN(text);
    self->metadata = SVTESTRETAIN(metadata);
    return self;
}

SvLocal void QBBasicPaneShow(SvGenericObject self_)
{
    QBBasicPane self = (QBBasicPane) self_;
    if(self->shown)
        return;
    self->shown = true;

    SvWidget frame = SideMenuGetFrame(self->sideMenu, self->level);
    svWidgetAttach(frame, self->window, 0, 0, 1);
    self->savedWidth = SideMenuGetFrameWidth(self->sideMenu, self->level);
    SideMenuSetFrameWidth(self->sideMenu, self->level, self->wantedWidth, false);
    self->window->height = frame->height;
    self->window->width = frame->width;

    QBContextMenuSetDepth(self->contextMenu, self->level, false);
}

SvLocal void QBBasicPaneHide(SvGenericObject self_, bool immediately)
{
    QBBasicPane self = (QBBasicPane) self_;
    if(!self->shown)
        return;
    self->shown = false;
    svWidgetDetach(self->window);
    SideMenuSetFrameWidth(self->sideMenu, self->level, self->savedWidth, immediately);
    QBContextMenuSetDepth(self->contextMenu, self->level - 1, immediately);
}

SvLocal void QBBasicPaneSetActive(SvGenericObject self_)
{
    QBBasicPane self = (QBBasicPane) self_;
    svWidgetSetFocus(self->menu);
}

SvLocal bool QBBasicPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal void QBBasicPane__dtor__(void *ptr)
{
    QBBasicPane self = ptr;
    svWidgetDetach(self->menu);

    if(self->title) {
        svWidgetDetach(self->title);
        svWidgetDestroy(self->title);
    }
    svWidgetDestroy(self->window);
    svWidgetDestroy(self->menu);
    SVRELEASE(self->options);
    SVTESTRELEASE(self->possibleOptions);
    SVRELEASE(self->widgetName);
    SVRELEASE(self->itemConstructor);
    SVRELEASE(self->extendedOptions);
    SVTESTRELEASE(self->userItemConstructor);
    SVRELEASE(self->titleName);
}


SvType QBBasicPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBasicPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBBasicPaneShow,
        .hide             = QBBasicPaneHide,
        .setActive        = QBBasicPaneSetActive,
        .handleInputEvent = QBBasicPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBasicPane",
                            sizeof(struct QBBasicPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal QBBasicPaneItem QBBasicPaneItemCreate(SvString id, SvString caption, SvString subcaption, SvHashTable metadata, QBBasicPaneItemCallback callback, void *callbackData)
{
    QBBasicPaneItem self = (QBBasicPaneItem) SvTypeAllocateInstance(QBBasicPaneItem_getType(), NULL);
    self->caption = SVRETAIN(caption);
    self->subcaption = SVTESTRETAIN(subcaption);
    self->callback = callback;
    self->callbackData = callbackData;
    self->id = SVTESTRETAIN(id);
    self->metadata = SVTESTRETAIN(metadata);

    return self;
}

SvLocal void QBBasicPaneWindowClean(SvApplication app, void *ptr)
{

}

SvLocal bool QBBasicPaneWindowInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBBasicPane self = window->prv;
    return QBContextMenuHandleInput(self->contextMenu, (SvObject) self, e);
}

QBBasicPane QBBasicPaneCreateFromSettings(const char *settings, SvApplication app, SvScheduler scheduler, QBTextRenderer textRenderer, QBContextMenu ctxMenu, unsigned level, SvString widgetName)
{
    svSettingsPushComponent(settings);
    QBBasicPane self = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self, app, scheduler, textRenderer, ctxMenu, level, widgetName);
    svSettingsPopComponent();

    return self;
}


SvLocal void
QBBasicPaneOnChangePosition(void *w_, const XMBMenuEvent data)
{
    if(!data->clicked)
        return;

    QBBasicPane self = ((SvWidget) w_)->prv;

    if (!self->shown)
        return;

    QBBasicPaneItem item = (QBBasicPaneItem) QBActiveArrayObjectAtIndex(self->options, data->position);

    SvGenericObject subpane = NULL;

    if((subpane = (SvGenericObject) SvHashTableFind(self->extendedOptions, (SvGenericObject) item))) {
        QBContextMenuPushPane(self->contextMenu, subpane);
    }

    if(item->callback)
        item->callback(item->callbackData, item->id, self, item);
}

void QBBasicPaneInit(QBBasicPane self, SvApplication app, SvScheduler scheduler, QBTextRenderer textRenderer, QBContextMenu ctxMenu, unsigned level, SvString widgetName)
{
    char *itemName, *focusName, *disabledFocusName;
    asprintf(&itemName, "%s.Item", SvStringCString(widgetName));
    asprintf(&focusName, "%s.Focus", SvStringCString(widgetName));
    asprintf(&disabledFocusName, "%s.DisabledFocus", SvStringCString(widgetName));
    self->titleName = SvStringCreateWithFormat("%s.Title", SvStringCString(widgetName));
    self->widgetName = SVRETAIN(widgetName);
    self->sideMenu = ctxMenu->sideMenu;
    self->options = QBActiveArrayCreate(5, NULL);
    self->possibleOptions = NULL;
    self->window = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->menu = XMBVerticalMenuNew(app, SvStringCString(widgetName), NULL);
    self->itemConstructor = (QBBasicPaneItemConstructor) SvTypeAllocateInstance(QBBasicPaneItemConstructor_getType(), NULL);
    self->itemConstructor->itemConstructor = QBXMBItemConstructorCreate(itemName, textRenderer);
    self->itemConstructor->focusConstructor = QBFrameConstructorFromSM(focusName);
    if(svSettingsIsWidgetDefined(disabledFocusName)) {
        self->itemConstructor->disabledFocusConstructor = QBFrameConstructorFromSM(disabledFocusName);
    }
    self->level = level;
    self->extendedOptions = SvHashTableCreate(5, NULL);
    self->textRenderer = textRenderer;
    self->contextMenu = ctxMenu;
    self->scheduler = scheduler;
    self->wantedWidth = svSettingsGetInteger(SvStringCString(widgetName), "boxWidth",SideMenuGetFrameWidth(self->sideMenu, self->level));
    if(svSettingsIsWidgetDefined(SvStringCString(self->titleName))) {
        self->title = svLabelNewFromSM(app, SvStringCString(self->titleName));
    }
    free(itemName);
    free(focusName);
    free(disabledFocusName);

    self->window->prv = self;
    self->window->clean = QBBasicPaneWindowClean;
    svWidgetSetInputEventHandler(self->window, QBBasicPaneWindowInputEventHandler);

    svWidgetSetFocusable(self->menu, true);

    XMBVerticalMenuSetNotificationTarget(self->menu, self->window, QBBasicPaneOnChangePosition);

    XMBVerticalMenuConnectToDataSource(self->menu, (SvGenericObject) self->options, (SvGenericObject) self->itemConstructor, NULL);

    svSettingsWidgetAttach(self->window, self->menu, SvStringCString(self->widgetName), 0);
    if(self->title)
        svSettingsWidgetAttach(self->window, self->title, SvStringCString(self->titleName), 0);
}

void QBBasicPaneSetItemConstructor(QBBasicPane self, SvGenericObject itemCtor)
{
    SVTESTRELEASE(self->userItemConstructor);
    self->userItemConstructor = SVTESTRETAIN(itemCtor);
    if(itemCtor) {
        XMBVerticalMenuSetItemController(self->menu, itemCtor, NULL);
    } else {
        XMBVerticalMenuSetItemController(self->menu, (SvGenericObject) self, NULL);
    }
}

int QBBasicPaneFindItemIdxById(SvGenericObject options, SvString id)
{
    if (!options || !id || !SvObjectIsInstanceOf(options, SvArray_getType())) {
        log_error("Invalid argument. (options:%p, id:%p, options type:%s)", options, id,
                  options ? SvObjectGetTypeName(options) : NULL);
        return -1;
    }

    SvIterator it = SvArrayIterator((SvArray) options);
    int idx = 0;
    QBBasicPaneItemParams nextItem = NULL;

    while ((nextItem = (QBBasicPaneItemParams) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) nextItem->id, (SvObject) id)) {
            return idx;
        }
        idx++;
    }
    return -1;
}

SvGenericObject QBBasicPaneParseOptionsFile(SvApplication app,
                                            SvString itemNamesFilename)
{
    if (!itemNamesFilename || !app)
        return NULL;

    char *buffer = NULL;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);
    char *filePath = SvRBLocatorFindFile(locator, SvStringCString(itemNamesFilename));
    if (!filePath) {
        log_error("File %s not found", SvStringCString(itemNamesFilename));
        return NULL;
    }

    QBFileToBuffer(filePath, &buffer);
    free(filePath);
    if (!buffer) {
        return NULL;
    }

    SvJSONClass helper = SvJSONClassCreate(NULL, QBBasicPaneReadItemParams, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBBasicPaneItemParams"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    SvGenericObject tmpItemNames = SvJSONParseString(buffer, true, NULL);
    free(buffer);

    if (!tmpItemNames || !SvObjectIsInstanceOf(tmpItemNames, SvArray_getType())) {
        SVTESTRELEASE(tmpItemNames);
        return NULL;
    }

    SvGenericObject currentItem = NULL;
    SvIterator it = SvArrayIterator((SvArray) tmpItemNames);
    while ((currentItem = SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) currentItem, QBBasicPaneItemParams_getType())) {
            SvLogWarning("File %s has improper object", SvStringCString(itemNamesFilename));
            SVTESTRELEASE(tmpItemNames);
            return NULL;
        }
    }
    return tmpItemNames;
}

void QBBasicPaneLoadOptionsFromFile(QBBasicPane self, SvString itemNamesFilename)
{
    if (!itemNamesFilename)
        return;

    SvGenericObject tmpItemNames = QBBasicPaneParseOptionsFile(self->sideMenu->app,
                                                               itemNamesFilename);

    if (!tmpItemNames)
        return;

    SVTESTRELEASE(self->possibleOptions);
    self->possibleOptions = (SvArray) tmpItemNames;
}

void QBBasicPaneOptionPropagateObjectChange(QBBasicPane self, QBBasicPaneItem item)
{
    int idx = QBActiveArrayIndexOfObject(self->options, (SvGenericObject)item, NULL);
    if(idx >= 0)
        QBActiveArrayPropagateObjectsChange(self->options, idx, 1, NULL);
}

SvLocal bool QBBasicPaneAddItemToOptionsWithIndexFromJson(QBBasicPane self, QBBasicPaneItem item)
{
    SvGenericObject currentOption, currentItem;
    bool wasOptionAddedToPane = true;

    if (self->possibleOptions) {
        // find json index for new item
        int jsonIndexForNewItem = QBBasicPaneFindItemIdxById((SvGenericObject) self->possibleOptions,
                                                             item->id);

        // check json position for already inserted options
        if (jsonIndexForNewItem >= 0) {
            int currentPosition = 0;
            SvIterator optionsIter = QBActiveArrayIterator(self->options);
            while ((currentOption = SvIteratorGetNext(&optionsIter))) {
                SvIterator it = SvArrayIterator(self->possibleOptions);
                int jsonIndexForCurrentOption = -1;
                int idx = 0;
                while ((currentItem = SvIteratorGetNext(&it))) {
                    if (SvObjectEquals(((SvObject) ((QBBasicPaneItemParams) currentItem)->id), (SvObject) ((QBBasicPaneItem) currentOption)->id)) {
                        jsonIndexForCurrentOption = idx;
                        break;
                    }
                    idx++;
                }
                if (jsonIndexForCurrentOption >= 0 && jsonIndexForNewItem < jsonIndexForCurrentOption) {
                    break;
                }
                currentPosition++;
            }

            QBActiveArrayInsertObjectAtIndex(self->options, currentPosition, (SvGenericObject) item, NULL );
        } else {
            // This item wasn't included in json file, should not appeared in menu
            wasOptionAddedToPane = false;
        }
    } else
        // add to end of array
        QBActiveArrayAddObject(self->options, (SvGenericObject) item, NULL );

    return wasOptionAddedToPane;
}

SvLocal QBBasicPaneItem QBBasicPaneAddOptionWithItemParams(QBBasicPane self, SvString id, SvString caption_, SvString subcaption, QBBasicPaneItemParams params, QBBasicPaneItemCallback callback, void *callbackData)
{
    SvString caption = SVTESTRETAIN(caption_);
    SvHashTable metadata = NULL;

    if (!caption) {
        if (!params) {
            return NULL;
        } else {
            if (!params->text) {
                return NULL;
            } else {
                caption = SvStringCreate(gettext(SvStringCString(params->text)), NULL);
            }
        }
    }

    if (params && params->metadata) {
        metadata = SVRETAIN(params->metadata);
    } else {
        metadata = SvHashTableCreate(5, NULL);
    }


    QBBasicPaneItem item = QBBasicPaneItemCreate(id, caption, subcaption, metadata, callback, callbackData);
    bool wasOptionAdded = QBBasicPaneAddItemToOptionsWithIndexFromJson(self, item);

    SVRELEASE(item);
    SVTESTRELEASE(metadata);
    SVRELEASE(caption);
    return (wasOptionAdded) ? item : NULL ;
}

QBBasicPaneItem QBBasicPaneAddOption(QBBasicPane self, SvString id, SvString caption_, QBBasicPaneItemCallback callback, void *callbackData)
{
    QBBasicPaneItemParams params = QBBasicPaneGetItemParams(self, id);
    QBBasicPaneItem item = QBBasicPaneAddOptionWithItemParams(self, id, caption_, NULL, params, callback, callbackData);

    SVTESTRELEASE(params);
    return item;
}

QBBasicPaneItem QBBasicPaneAddOptionWithSubcaption(QBBasicPane self, SvString id, SvString caption_, SvString subcaption, QBBasicPaneItemCallback callback, void *callbackData)
{
    QBBasicPaneItemParams params = QBBasicPaneGetItemParams(self, id);
    QBBasicPaneItem item = QBBasicPaneAddOptionWithItemParams(self, id, caption_, subcaption, params, callback, callbackData);

    SVTESTRELEASE(params);
    return item;
}


QBBasicPaneItem QBBasicPaneAddOptionWithConfirmation(QBBasicPane self, SvString id, SvString caption_, SvString widgetName, SvArray options, SvArray ids, QBBasicPaneItemCallback callback, void *callbackData)
{
    SvString caption = caption_;

    QBBasicPaneItem item = QBBasicPaneAddOption(self, id, caption, NULL, NULL);
    if (item) {
        QBBasicPane confirmation = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL );
        QBBasicPaneInit(confirmation, self->menu->app, self->scheduler, self->textRenderer, self->contextMenu, self->level + 1, widgetName);

        SvIterator it = SvArrayIterator(options);
        SvIterator it2;
        if (ids)
            it2 = SvArrayIterator(ids);

        SvString option;
        while ((option = (SvString) SvIteratorGetNext(&it))) {
            SvObject id2 = NULL;
            if (ids)
                id2 = SvIteratorGetNext(&it2);

            QBBasicPaneAddOption(confirmation, (SvString) id2, option, callback, callbackData);
        }

        SvHashTableInsert(self->extendedOptions, (SvGenericObject) item, (SvGenericObject) confirmation);
        SVRELEASE(confirmation);

        item->subpane = SVRETAIN(confirmation);
    }

    return item;
}

QBBasicPaneItem QBBasicPaneAddOptionWithSubpane(QBBasicPane self, SvString id, SvString caption_, SvGenericObject pane)
{
    SvString caption = caption_;
    QBBasicPaneItem item = QBBasicPaneAddOption(self, id, caption, NULL, NULL);
    if (item) {
        SvHashTableInsert(self->extendedOptions, (SvGenericObject) item, pane);
        item->subpane = SVRETAIN(pane);
    }
    return item;
}

QBBasicPaneItem QBBasicPaneAddOptionWithSubpaneAndSubcaption(QBBasicPane self, SvString id, SvString caption, SvString subcaption, SvObject pane)
{
    QBBasicPaneItem item = QBBasicPaneAddOptionWithSubcaption(self, id, caption, subcaption, NULL, NULL);
    if (item) {
        SvHashTableInsert(self->extendedOptions, (SvGenericObject) item, pane);
        item->subpane = SVRETAIN(pane);
    }
    return item;
}

SvLocal QBBasicPaneItem QBBasicPaneAddOptionWithSubpaneWithItemParams(QBBasicPane self, SvString id, SvString caption_, QBBasicPaneItemParams params, SvGenericObject pane)
{
    SvString caption = caption_;
    QBBasicPaneItem item = QBBasicPaneAddOptionWithItemParams(self, id, caption, NULL, params, NULL, NULL);
    if (item) {
        SvHashTableInsert(self->extendedOptions, (SvGenericObject) item, pane);
        item->subpane = SVRETAIN(pane);
    }
    return item;
}

QBBasicPaneItem QBBasicPaneAddOptionWithOSK(QBBasicPane self, SvString id, SvString caption_, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData)
{
    QBBasicPaneItemParams params = QBBasicPaneGetItemParams(self, id);

    if ((!caption_) && (!params || !params->text)) {
            SVTESTRELEASE(params);
            return NULL;
    }

    QBOSKPane osk = (QBOSKPane) SvTypeAllocateInstance(QBOSKPane_getType(), NULL);
    SvErrorInfo error = NULL;
    QBOSKPaneInit(osk, self->scheduler, self->contextMenu, self->level + 1, widgetName, callback, callbackData, &error);
    QBBasicPaneItem item = NULL;
    if (!error) {
        item = QBBasicPaneAddOptionWithSubpaneWithItemParams(self, id, caption_, params, (SvGenericObject)osk);
    } else {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
    SVRELEASE(osk);
    SVTESTRELEASE(params);
    return item;
}

QBBasicPaneItem QBBasicPaneAddOptionWithContainer(QBBasicPane self, SvString id, SvString caption_, SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData)
{
    QBBasicPaneItemParams params = QBBasicPaneGetItemParams(self, id);

    if ((!caption_) && (!params || !params->text)) {
            SVTESTRELEASE(params);
            return NULL;
    }

    QBContainerPane container = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(container, self->sideMenu->app, self->contextMenu, self->level + 1, widgetName, callbacks, callbackData);
    QBBasicPaneItem item = QBBasicPaneAddOptionWithSubpane(self, id, caption_, (SvGenericObject)container);
    SVRELEASE(container);
    SVTESTRELEASE(params);
    return item;
}

void QBBasicPaneRemoveAll(QBBasicPane self)
{
    int cnt;
    for(cnt = QBActiveArrayCount(self->options);cnt;cnt--)
        QBActiveArrayRemoveObjectAtIndex(self->options, cnt - 1, NULL);
    SvHashTableRemoveAllObjects(self->extendedOptions);
}

size_t QBBasicPaneGetOptionsCount(QBBasicPane self)
{
    return QBActiveArrayCount(self->options);
}

SvWidget QBBasicPaneGetTitle(QBBasicPane self)
{
    return self->title;
}

void QBBasicPaneSetTitle(QBBasicPane self, SvString title)
{
    svLabelSetText(self->title, title ? SvStringCString(title) : "");
}

int QBBasicPaneGetLevel(QBBasicPane self)
{
    return self->level;
}

void QBBasicPaneSetWidth(QBBasicPane self, int width)
{
    self->wantedWidth = width;
}

SvString QBBasicPaneItemGetID(QBBasicPaneItem self)
{
    return self->id;
}

void QBBasicPaneSetPosition(QBBasicPane self, SvString id, bool immediately)
{
    SvIterator it = QBActiveArrayIterator(self->options);
    QBBasicPaneItem item;
    int pos = 0;
    while((item = (QBBasicPaneItem) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) item->id, (SvObject) id)) {
            XMBVerticalMenuSetPosition(self->menu, pos, immediately, NULL);
            break;
        }
        pos++;
    }
}

void QBBasicPaneSetPositionByIndex(QBBasicPane self, size_t idx, bool immediately)
{
    XMBVerticalMenuSetPosition(self->menu, idx, immediately, NULL);
}

void QBBasicPaneSetPrv(QBBasicPane self, void *prv)
{
    self->prv = prv;
}

void *QBBasicPaneGetPrv(QBBasicPane self)
{
    return self->prv;
}

SvLocal QBBasicPaneItemParams QBBasicPaneGetItemParams(QBBasicPane self, SvString id)
{
    QBBasicPaneItemParams params = NULL;
    if (self->possibleOptions) {
        SvIterator it = SvArrayIterator(self->possibleOptions);
        SvGenericObject currentItem;
        while((currentItem = SvIteratorGetNext(&it))) {
            if (SvObjectEquals(((SvObject) ((QBBasicPaneItemParams) currentItem)->id), (SvObject) id)) {
                params = SVRETAIN((QBBasicPaneItemParams) currentItem);
                break;
            }
        }
    }
    return params;
}

SvLocal SvGenericObject QBBasicPaneReadItemParams(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue id = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("id"));
    SvValue text = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("text"));
    SvHashTable metadata = (SvHashTable) SvHashTableFind(desc, (SvGenericObject) SVSTRING("metadata"));

    if (!id || !text)
        return NULL;

    if (!SvObjectIsInstanceOf((SvObject) id, SvValue_getType()) || !SvObjectIsInstanceOf((SvObject) text, SvValue_getType()) || (metadata && !SvObjectIsInstanceOf((SvObject) metadata, SvHashTable_getType()))) {
        return NULL;
    }

    if (!SvValueIsString(id) || !SvValueIsString(text)) {
        return NULL;
    }
    return (SvGenericObject) QBBasicPaneItemParamsCreate(SvValueGetString(id), SvValueGetString(text), metadata);
}

QBContextMenu QBBasicPaneGetContextMenu(QBBasicPane self)
{
    return self->contextMenu;
}

