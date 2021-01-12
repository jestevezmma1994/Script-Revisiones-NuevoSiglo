/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include <Widgets/QBVoDGrid.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBListProxy.h>
#include <Utils/vod/QBListProxyFilter.h>
#include <Utils/vod/QBListProxyNegativeFilter.h>
#include <QBCarousel/QBVerticalCarousel.h>
#include <main.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Utils/QBMovieUtils.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <QBContentManager/QBContentProvider.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenu.h>
#include <Windows/QBVoDMovieDetails.h>
#include <ctype.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/QBUIEvent.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvEnv.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <Utils/QBVODUtils.h>
#include <QBDataModel3/QBTreePath.h>
#include <libintl.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBVoDGridLogLevel", "");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)


/**
 * VoD grid class.
 * @class QBVoDGrid
 * @extends SvObject
 **/
typedef struct QBVoDGrid_ *QBVoDGrid;

/**
 * VoD grid controller class.
 * @class QBVoDGridController
 * @extends SvObject
 **/
typedef struct QBVoDGridController_ *QBVoDGridController;

/**
 * QBVoDGridController class internals.
 **/
struct QBVoDGridController_ {
    struct SvObject_ super_;
    QBVoDGrid parent;
    SvString verticalMenuName;
};

/**
 * QBVoDGrid class internals.
 **/
struct QBVoDGrid_ {
    struct SvObject_ super_;

    SvWidget w;

    SvWidget verticalMenu;
    SvWidget focusedVerticalMenu;
    SvWidget frontClipping;

    SvWidgetId activeCarouselId;

    SvWeakReference activeRow;
    SvWeakReference focusedActiveRow;

    SvObject highlightedProduct;

    SvWidgetId notificationTarget;

    unsigned int settingsContext;

    SvWidget focus;
    SvWidget BackgroundDim;

    QBVoDGridController verticalMenuController;
    QBVoDGridController focusedVerticalMenuController;

    SvObject gridDataSource;
    SvObject tree;
    QBContentCategory category;
    SvObject path;

    AppGlobals appGlobals;

    QBVoDGridState state;
    SvWidget animation;
    SvWidget resultsPane;

    int menuPos;
    int menuPosItem;
    SvObject menuPosPath;

    SvObject owner;
    QBVoDGridCallbacks callbacks;
    SvSet unlockedGridItems; ///< Set with categories which have been unlocked.

    bool searchResultsAvailable;
};

typedef struct {
    SvObject path;
    bool isFocused;
} QBVoDGridMenuItemInitParams;

typedef struct QBVoDGridMenuItem_ *QBVoDGridMenuItem;

struct QBVoDGridMenuItem_ {
    struct SvObject_ super_;

    SvWidget w;

    SvWidget name;
    SvWidget carousel;
    SvWidget frame;

    QBFrameParameters frameParams;
    SvBitmap inactiveBg;
    SvBitmap activeBg;

    SvWidget focusWidget;
    SvWidget focusCarousel;
    SvWidget focusCarouselSlot;

    QBContentCategory category;
    SvObject dataSource;
    SvObject path;
    SvObject highlightedObject;

    QBVoDGridState state;

    bool active;
    bool isAdult;
    int settingsCtx;

    int activeElementIdx;

    SvWidget animation;

    QBVoDGrid grid;

    unsigned int idx;

    QBVoDGridMenuItemInitParams initParams;
    
    SvWidget tileBlack; // AMERELES [#3580] Al salir de un título VOD parece "ruedita" de carga en posters ya cargado
};

enum { QBVoDGrid_NoItemSelected = -1 };

SvLocal void
QBVoDGridEventDestroy(void *self_)
{
    QBVoDGridEvent self = self_;

    SVRELEASE(self->category);
    SVRELEASE(self->categoryPath);
    SVTESTRELEASE(self->object);
}

SvType
QBVoDGridEvent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridEventDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridEvent",
                            sizeof(struct QBVoDGridEvent_),
                            QBUIEvent_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
        SvTypeSetAbstract(type, NULL);
    }

    return type;
}

SvType
QBVoDGridItemFocusedEvent_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridItemFocusedEvent",
                            sizeof(struct QBVoDGridItemFocusedEvent_),
                            QBVoDGridEvent_getType(),
                            &type,
                            NULL);
    }

    return type;
}

SvType
QBVoDGridItemSelectedEvent_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridItemSelectedEvent",
                            sizeof(struct QBVoDGridItemSelectedEvent_),
                            QBVoDGridEvent_getType(),
                            &type,
                            NULL);
    }

    return type;
}


SvLocal void QBVoDGridMenuItemSetCarouselsVisibility(QBVoDGridMenuItem item, bool show);
SvLocal void QBVoDGridMenuItemRefreshCarousel(QBVoDGridMenuItem self);
SvLocal bool QBVoDGridShouldBeBlocked(QBVoDGridMenuItem self, SvObject slot);

SvLocal void QBVoDGridHandleCarouselNotification(QBVoDGrid self, SvWidgetId src, SvUserEvent e);

SvLocal SvWidget
QBVoDGridCreateAnimation(SvApplication app, const char *widgetName)
{
    SvWidget animation = QBWaitAnimationCreate(app, widgetName);
    return animation;
}

SvLocal bool
QBVoDGridMenuItemIsLocked(QBVoDGridMenuItem self)
{
    if (!self || !self->category || !SvObjectIsInstanceOf((SvObject) self->category, QBContentCategory_getType())) {
        return true;
    }

    // When parental control is disabled all assets should be available.
    if (!QBParentalControlLogicAdultIsBlocked(self->grid->appGlobals->parentalControlLogic)) {
        return false;
    }

    bool isLocked = SvSetContainsElement(self->grid->unlockedGridItems, (SvObject) QBContentCategoryGetId(self->category));
    return !isLocked;
}

SvLocal void
QBVoDGridNewHighlight(QBVoDGrid self, SvObject newHighlight, QBVoDGridMenuItem item)
{
    if (SvObjectEquals((SvObject) newHighlight, (SvObject) self->highlightedProduct) &&
        SvObjectEquals((SvObject) self->category, (SvObject) item->category))
        return;

    SVTESTRETAIN(newHighlight);
    SVTESTRELEASE(self->highlightedProduct);
    self->highlightedProduct = newHighlight;

    QBVoDGridEvent event = (QBVoDGridEvent) SvTypeAllocateInstance(QBVoDGridItemFocusedEvent_getType(), NULL);
    if (event) {
        event->category = SVRETAIN(item->category);
        event->categoryPath = SVRETAIN(item->path);
        event->object = SVTESTRETAIN(self->highlightedProduct);
        event->menuPostion = self->menuPos;
        event->menuItemPostion = self->menuPosItem;
        event->isLocked = QBVoDGridShouldBeBlocked(item, newHighlight);
        event->isAdult = QBVoDUtilsIsAdult(self->highlightedProduct);

        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusPostEvent(eventBus, (QBPeerEvent) event, (SvObject) self, NULL);
        SVRELEASE(event);
    }

    bool isSearchResultItem = SvObjectIsInstanceOf((SvObject) item->category, QBContentSearch_getType());
    self->callbacks.QBVoDGridSearchResultsFocusChanged(self->owner, isSearchResultItem);
}

typedef struct QBVoDGridSlot_ {
    int settingsCtx;
    char *widgetName;

    SvWidget box;
    SvWidget icon;
    SvWidget label;
    SvWidget labelBackground;

    QBVoDGridMenuItem menuItem;
    SvObject object;

    void *owner;
} *QBVoDGridSlot;

SvLocal void
QBVoDGridSlotClean(SvApplication app, void *self_)
{
    QBVoDGridSlot self = self_;
    free(self->widgetName);
    SVTESTRELEASE(self->object);
    SVTESTRELEASE(self->menuItem);
    free(self);
}

SvLocal void
QBVoDGridCarouselSlotDestroyIcon(SvWidget w)
{
    QBVoDGridSlot self = w->prv;
    if (self->icon) {
        svWidgetDestroy(self->icon);
        self->icon = NULL;
    }
}

SvLocal bool
QBVoDGridShouldBeBlocked(QBVoDGridMenuItem self, SvObject object)
{
    if (!self)
        return true;

    if (!QBVoDGridMenuItemIsLocked(self))
        return false;

    if (object && SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
        if (SvSetContainsElement(self->grid->unlockedGridItems, (SvObject) QBContentCategoryGetId((QBContentCategory) object)))
            return false;
    }

    return true;
}

SvLocal void
QBVoDGridCarouselSlotSetObject(SvWidget w, SvObject object)
{
    QBVoDGridSlot self = w->prv;
    bool isBlocked = QBVoDGridShouldBeBlocked(self->menuItem, object) && QBVoDUtilsIsAdult(object);
    if (!SvObjectEquals(self->object, object) || !self->icon || !self->label || isBlocked) {
        QBVoDGridCarouselSlotDestroyIcon(w);

        svSettingsRestoreContext(self->settingsCtx);
        self->icon = svIconNew(w->app, self->widgetName);
        svIconSetBitmap(self->icon, 1, svSettingsGetBitmap(self->widgetName, "bgLocked"));
        if (object && SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
            svIconSetBitmap(self->icon, 0, svSettingsGetBitmap(self->widgetName, "bgCategory"));
        }
        svWidgetAttach(self->box, self->icon, 0, 0, 1);

        char *labelName = NULL;
        asprintf(&labelName, "%s.Label", self->widgetName);
        if (svSettingsIsWidgetDefined(labelName)) {
            if (self->label) {
                svWidgetDestroy(self->label);
                self->label = NULL;
            }

            if (self->labelBackground) {
                svWidgetDestroy(self->labelBackground);
                self->labelBackground = NULL;
            }

            char *labelBackgroundName = NULL;
            asprintf(&labelBackgroundName, "%s.Background", labelName);
            self->labelBackground = svSettingsWidgetCreate(w->app, labelBackgroundName);
            svSettingsWidgetAttach(self->box, self->labelBackground, labelBackgroundName, 1);
            svWidgetSetHidden(self->labelBackground, true);
            free(labelBackgroundName);

            self->label = QBAsyncLabelNew(w->app, labelName, self->menuItem->grid->appGlobals->textRenderer);
            svSettingsWidgetAttach(self->box, self->label, labelName, 1);
        }
        free(labelName);
        svSettingsPopComponent();

        SVTESTRETAIN(object);
        SVTESTRELEASE(self->object);
        self->object = object;
    }

    if (self->label && self->object) {
        if (SvObjectIsInstanceOf(self->object, SvDBRawObject_getType())) {
            SvValue val = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->object, "name");
            if (!val) {
                val = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) self->object, "title");
            }
            if (!isBlocked && val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
                QBAsyncLabelSetText(self->label, SvValueGetString(val));
                svWidgetSetHidden(self->labelBackground, false);
            } else if (isBlocked) {
                SvString blockedString = SvStringCreate(gettext("Title blocked"), NULL);
                QBAsyncLabelSetText(self->label, blockedString);
                svWidgetSetHidden(self->labelBackground, false);
                SVRELEASE(blockedString);
            } else {
                QBAsyncLabelSetText(self->label, NULL);
                svWidgetSetHidden(self->labelBackground, true);
            }
        } else if (SvObjectIsInstanceOf(self->object, QBContentCategory_getType())) {
            SvString categoryName = QBContentCategoryGetName((QBContentCategory) self->object);
            if (!isBlocked && categoryName) {
                QBAsyncLabelSetText(self->label, categoryName);
                svWidgetSetHidden(self->labelBackground, false);
            } else if (isBlocked) {
                SvString blockedString = SvStringCreate(gettext("Adult"), NULL);
                QBAsyncLabelSetText(self->label, blockedString);
                svWidgetSetHidden(self->labelBackground, false);
                SVRELEASE(blockedString);
            } else {
                QBAsyncLabelSetText(self->label, NULL);
                svWidgetSetHidden(self->labelBackground, true);
            }
        }
    }

    bool isStartMarker = self->object && SvObjectIsInstanceOf(self->object, SvBitmap_getType());
    bool isFocusCarousel = self->owner == self->menuItem->focusCarousel;

    unsigned int iconTargetIdx = 0;
    double iconDuration = 0.0;
    bool setImage = false;
    SvObject imageSource = NULL;
    if (isBlocked) {
        iconTargetIdx = 1;
        iconDuration = -1.0;
    } else if (!self->object) {
        iconDuration = -1.0;
    } else if (SvObjectIsInstanceOf(self->object, SvDBRawObject_getType())) {
        imageSource = SvDBRawObjectGetAttrValue((SvDBRawObject) self->object, "image");
        setImage = true;
    } else if (SvObjectIsInstanceOf(self->object, QBContentCategory_getType())) {
        imageSource = QBContentCategoryGetAttribute((QBContentCategory) self->object, SVSTRING("thumbnail"));
        setImage = true;
    } else if (isStartMarker) {
        if (isFocusCarousel) {
            QBVoDGridCarouselSlotDestroyIcon(w);
        } else {
            iconTargetIdx = 2;
            svIconSetBitmap(self->icon, iconTargetIdx, (SvBitmap) self->object);
        }
    } else {
        return;
    }

    if (setImage) {
        if (imageSource) {
            if (SvObjectIsInstanceOf((SvObject) imageSource, SvArray_getType())) {
                SvString url = QBMovieUtilsGetThumbnailUrl((SvArray) imageSource, w->width, w->height);
                if (url) {
                    iconTargetIdx = 1;
                    iconDuration = -1.0;
                    svIconSetBitmapFromURI(self->icon, iconTargetIdx, SvStringCString(url));
                }
            } else if (SvObjectIsInstanceOf((SvObject) imageSource, SvValue_getType()) && SvValueIsString((SvValue) imageSource)) {
                SvString url = SvValueGetString((SvValue) imageSource);
                if (url) {
                    iconTargetIdx = 1;
                    iconDuration = -1.0;
                    svIconSetBitmapFromURI(self->icon, iconTargetIdx, SvStringCString(url));
                }
            }
        } else {
            QBMovieUtilsSetThumbnailIntoIcon(self->icon, (SvObject) self->object, "covers", "cover_name", "grid", 1);
            return;
        }
    }

    if (self->icon) {
        svIconSwitch(self->icon, iconTargetIdx, 0, iconDuration);
    }
}

SvLocal SvWidget
QBVoDGridCarouselSlotCreate(SvApplication app, const char *widgetName, QBVoDGridMenuItem item, void *owner)
{
    QBVoDGridSlot self = calloc(1, sizeof(struct QBVoDGridSlot_));
    self->box = svWidgetCreateBitmap(app, 0, 0, NULL);
    self->box->prv = self;
    self->box->clean = QBVoDGridSlotClean;
    self->settingsCtx = svSettingsSaveContext();
    asprintf(&self->widgetName, "%s", widgetName);
    self->box->width = svSettingsGetInteger(widgetName, "width", 0);
    self->box->height = svSettingsGetInteger(widgetName, "height", 0);
    self->menuItem = NULL;
    self->menuItem = SVRETAIN(item);
    self->owner = owner;

    return self->box;
}

SvLocal int
QBVoDGridMenuItemGetCarouselStoredIndex(QBVoDGridMenuItem self)
{
    size_t verticalMenuSlotsCount = XMBVerticalMenuGetSlotsCount(self->grid->verticalMenu);
    for (size_t idx = 0; idx < verticalMenuSlotsCount; idx++) {
        SvWidget oldVerticalMenuItemWidget = XMBVerticalMenuGetItemWidgetAtIndex(self->grid->verticalMenu, idx);
        if (!oldVerticalMenuItemWidget)
            continue;

        QBVoDGridMenuItem oldItem = oldVerticalMenuItemWidget->prv;
        if (!SvObjectEquals(self->path, oldItem->path))
            continue;

        return oldItem->activeElementIdx;
    }
    return -1;
}

SvLocal void
QBVoDGridMenuItemCheckState(QBVoDGridMenuItem self)
{
    QBVoDGridState newState = self->state;
    if (!SvObjectIsInstanceOf((SvObject) self->category, QBContentCategory_getType()))
        return;
    QBContentCategoryLoadingState categoryState = QBContentCategoryGetLoadingState(self->category);
    size_t itemsCnt = 0;

    itemsCnt = SvInvokeInterface(QBListModel, self->dataSource, getLength);
    if (self->state != QBVoDGridState_loaded || itemsCnt == 0) {
        if (categoryState == QBContentCategoryLoadingState_idle && itemsCnt >= 1) {
            newState = QBVoDGridState_loaded;
        } else if (categoryState == QBContentCategoryLoadingState_initial || categoryState == QBContentCategoryLoadingState_active) {
            newState = QBVoDGridState_loading;
        } else {
            newState = QBVoDGridState_noresults;
        }
    }

    if (newState != self->state) {
        if (self->animation) {
            svWidgetDestroy(self->animation);
            self->animation = NULL;
        }
        
        // AMERELES [#3580] Al salir de un título VOD parece "ruedita" de carga en posters ya cargado
        if (self->tileBlack) {
            svWidgetDestroy(self->tileBlack);
            self->tileBlack = NULL;
        }

        if (newState == QBVoDGridState_loading) {
            if (!svWidgetIsHidden(self->w)) {
                if (self->focusCarouselSlot) {
                    svWidgetSetHidden(self->focusCarouselSlot, true);
                }
                QBVoDGridMenuItemSetCarouselsVisibility(self, false);
                
                // BEGIN AMERELES [#3580] Al salir de un título VOD parece "ruedita" de carga en posters ya cargado
                //svSettingsRestoreContext(self->settingsCtx);
                //self->animation = QBVoDGridCreateAnimation(self->w->app, "VerticalMenu.Item.Animation");
                //svSettingsWidgetAttach(self->w, self->animation, "VerticalMenu.Item.Animation", 3);
                //svSettingsPopComponent();
                if (itemsCnt < 2)
                {
                    svSettingsRestoreContext(self->settingsCtx);
                    
                    self->animation = QBVoDGridCreateAnimation(self->w->app, "VerticalMenu.Item.Animation");
                    svSettingsWidgetAttach(self->w, self->animation, "VerticalMenu.Item.Animation", 3);
                    
                    self->tileBlack = QBVoDGridCreateAnimation(self->w->app, "VerticalMenu.Item.TileBlack");
                    svSettingsWidgetAttach(self->focusCarouselSlot, self->tileBlack, "VerticalMenu.Item.TileBlack", 2);
                    
                    svSettingsPopComponent();
                }
                // END AMERELES [#3580] Al salir de un título VOD parece "ruedita" de carga en posters ya cargado
            }
        } else if (newState == QBVoDGridState_loaded) {
            if (itemsCnt >= 1) {
                svSettingsRestoreContext(self->settingsCtx);
                size_t initialFocusColumn = (size_t) svSettingsGetInteger("VerticalMenu.Item.Carousel", "initialFocusColumn", 4);
                svSettingsPopComponent();
                size_t pos = initialFocusColumn;

                if (self->grid->menuPosItem > 0 && self->path && self->grid->menuPosPath && SvObjectEquals(self->path, self->grid->menuPosPath)) {
                    pos = self->grid->menuPosItem;
                }

                bool isFlatUI = (self->grid->focusedVerticalMenuController != NULL);
                bool menuIsNotFocusedVerticalMenu = !strcmp(svWidgetGetName(self->carousel), "VerticalMenu.Item.Carousel");
                if (isFlatUI && !menuIsNotFocusedVerticalMenu) {
                    int oldActiveElementIdx = QBVoDGridMenuItemGetCarouselStoredIndex(self);
                    if (oldActiveElementIdx != -1)
                        pos = oldActiveElementIdx;
                }

                QBVerticalCarouselSetActive(self->carousel, (itemsCnt > pos) ? pos : (itemsCnt - 1));
                if (self->focusCarousel) {
                    QBVerticalCarouselSetActive(self->focusCarousel, (itemsCnt > pos) ? pos : (itemsCnt - 1));
                }

                bool showCarousel = true;
                if (isFlatUI && menuIsNotFocusedVerticalMenu && self->grid->activeRow) {
                    QBVoDGridMenuItem activeItem = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->grid->activeRow);
                    if (activeItem) {
                        showCarousel = (self->carousel != activeItem->carousel);
                        SVRELEASE(activeItem);
                    }
                }
                QBVoDGridMenuItemSetCarouselsVisibility(self, showCarousel);

                if (self->focusCarouselSlot) {
                    svWidgetSetHidden(self->focusCarouselSlot, false);
                }
                if (self->active && svWidgetIsFocused(self->grid->w) && self->focusWidget)
                    svWidgetSetHidden(self->focusWidget, false);
            }
        } else if (newState == QBVoDGridState_noresults) {
            svSettingsRestoreContext(self->settingsCtx);
            self->animation = QBAsyncLabelNew(self->w->app, "VerticalMenu.Item.NoResultsLabel", self->grid->appGlobals->textRenderer);
            svSettingsWidgetAttach(self->w, self->animation, "VerticalMenu.Item.NoResultsLabel", 3);
            SvString labelStr = SVTESTRETAIN((SvString) QBContentCategoryGetAttribute(self->category, SVSTRING("noResultsMessage")));
            if (!labelStr) {
                labelStr = SvStringCreate(gettext("No Content"), NULL);
            }
            if (self->animation) {
                QBAsyncLabelSetText(self->animation, labelStr);
            }
            SVRELEASE(labelStr);
            svSettingsPopComponent();
        }
        self->state = newState;

        if (self->grid->callbacks.QBVoDGridItemStateChanged) {
            struct QBVoDGridMenuItemInfo_s data = {
                .item     = self->w,
                .active   = self->active,
                .idx      = self->idx,
                .category = (SvObject) self->category,
                .count    = SvInvokeInterface(QBListModel, self->dataSource, getLength),
                .isAdult  = self->isAdult,
                .isLocked = QBVoDGridMenuItemIsLocked(self),
                .state    = self->state
            };

            self->grid->callbacks.QBVoDGridItemStateChanged(self->grid->owner, &data);
        }
    }
}

SvLocal void
QBVoDGridMenuItemCategoryStateChanged(SvObject self_, QBContentCategory category, QBContentCategoryLoadingState previousState, QBContentCategoryLoadingState currentState)
{
    QBVoDGridMenuItem self = (QBVoDGridMenuItem) self_;
    QBVoDGridMenuItemCheckState(self);
}

SvLocal void
QBVoDGridMenuItemDestroy(void *self_)
{
    QBVoDGridMenuItem self = self_;

    SVTESTRELEASE(self->dataSource);
    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->highlightedObject);
    SVTESTRELEASE(self->activeBg);
    SVTESTRELEASE(self->inactiveBg);
}


SvLocal SvType
QBVoDGridMenuItem_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridMenuItemDestroy
    };
    static SvType type = NULL;

    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBVoDGridMenuItemCategoryStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridMenuItem",
                            sizeof(struct QBVoDGridMenuItem_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentCategoryListener_getInterface(),
                            &categoryListenerMethods, NULL);
    }

    return type;
}

SvLocal void
QBVodGridBrowserMenuItemEventHandler(SvWidget w, SvWidgetId sender, SvUserEvent e)
{
    QBVoDGridMenuItem self = w->prv;

    QBVoDGridHandleCarouselNotification(self->grid, sender, e);

    if (self->state == QBVoDGridState_initializing)
        return;

    if (e->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION && sender == svWidgetGetId(self->carousel)) {
        QBVerticalCarouselNotification not = e->prv;
        SvObject newHighlightedProduct = NULL;

        if (not->dataObject)
            newHighlightedProduct = not->dataObject;

        SVTESTRETAIN(newHighlightedProduct);
        SVTESTRELEASE(self->highlightedObject);
        self->highlightedObject = newHighlightedProduct;
    }
}

#define ARG_ERROR(msg) { \
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, msg); \
        goto out; \
}

SvLocal void
menuItemClean(SvApplication app, void *prv)
{
    QBVoDGridMenuItem self = (QBVoDGridMenuItem) prv;
    if (self->state != QBVoDGridState_initializing && self->category && SvObjectIsInstanceOf((SvObject) self->category, QBContentCategory_getType()))
        QBContentCategoryRemoveListener(self->category, (SvObject) self, NULL);

    SVRELEASE(self);
}

SvLocal void
QBVoDGridRealignFocusCarousel(void *owner, SvWidget w, QBVerticalCarouselScrollState scrollState, int i, int offset)
{
    QBVoDGridMenuItem item = (QBVoDGridMenuItem) owner;
    if (item->focusCarousel) {
        QBVerticalCarouselRealignColumn(item->focusCarousel, scrollState, i, offset);
    }
}

SvLocal void
QBVoDGridMenuItemSetCarouselsVisibility(QBVoDGridMenuItem item, bool show)
{
    svWidgetSetHidden(item->w, !show);
    svWidgetSetHidden(item->carousel, !show);
    if (item->focusCarousel) {
        svWidgetSetHidden(item->focusCarousel, !show);
        svWidgetSetHidden(item->w, !show);
    }
}

SvLocal QBVoDGridMenuItem
QBVoDGridMenuItemInitialize(QBVoDGridMenuItem self, QBVoDGrid grid, SvObject category, SvObject path, SvString verticalMenuName, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvApplication app = grid->w->app;

    if (!self || !path || !verticalMenuName)
        ARG_ERROR("Invalid arguments passed!");

    const char *cName = SvStringCString(verticalMenuName);
    char *buf;

    asprintf(&buf, "%s.Item", cName);
    self->w = svSettingsWidgetCreate(app, buf);
    free(buf);
    if (!self->w)
        ARG_ERROR("Couldn't create main widget");

    self->grid = grid;
    self->state = QBVoDGridState_unknown;
    self->path = SvObjectCopy(path, NULL);

    QBActiveArray staticItems = QBActiveArrayCreate(1, NULL);
    asprintf(&buf, "%s.Item.Carousel.Slot", cName);
    SvBitmap listStartMarker = svSettingsGetBitmap(buf, "bgListStartMarker");
    free(buf);
    if (listStartMarker)
        QBActiveArrayAddObject(staticItems, (SvObject) listStartMarker, NULL);

    QBListProxy listProxy = (QBListProxy) QBListProxyCreateWithStaticItems((SvObject) grid->tree, path, (SvObject) staticItems, NULL);
    SvObject dataSource = (SvObject) QBListProxyNegativeFilterCreate(listProxy, QBContentSearch_getType(), NULL);
    SVRELEASE(listProxy);
    self->dataSource = dataSource;
    SVRELEASE(staticItems);
    asprintf(&buf, "%s.Item.Frame", cName);
    if (svSettingsIsWidgetDefined(buf)) {
        self->inactiveBg = svSettingsGetBitmap(buf, "inactiveBg");
        self->activeBg = svSettingsGetBitmap(buf, "activeBg");

        SVRETAIN(self->inactiveBg);
        SVRETAIN(self->activeBg);

        self->frameParams.width = svSettingsGetInteger(buf, "width", -1);
        self->frameParams.height = svSettingsGetInteger(buf, "height", -1);
        self->frameParams.bitmap = self->activeBg;

        self->frame = QBFrameCreate(app, &self->frameParams);
        svSettingsWidgetAttach(self->w, self->frame, buf, 1);
    } else {
        self->inactiveBg = NULL;
        self->activeBg = NULL;
        self->frame = NULL;
    }
    free(buf);
    self->category = SVRETAIN(category);
    asprintf(&buf, "%s.Item.Name", cName);
    if (svSettingsIsWidgetDefined(buf)) {
        self->name = QBAsyncLabelNew(app, buf, grid->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->w, self->name, buf, 2);
    } else {
        self->name = NULL;
    }
    free(buf);

    asprintf(&buf, "%s.Item.FocusCarouselSlot", cName);
    if (svSettingsIsWidgetDefined(buf)) {
        self->focusCarouselSlot = svSettingsWidgetCreate(app, buf);
        if (self->focusCarouselSlot) {
            svSettingsWidgetAttach(self->w, self->focusCarouselSlot, buf, 3);
        }
    } else {
        self->focusCarouselSlot = NULL;
    }
    free(buf);

    asprintf(&buf, "%s.Item.FocusCarousel", cName);
    if (svSettingsIsWidgetDefined(buf)) {
        self->focusCarousel = QBVerticalCarouselNewFromSM(app, buf);
        if (self->focusCarousel && self->focusCarouselSlot) {
            QBVerticalCarouselSetNotificationTarget(self->focusCarousel, svWidgetGetId(self->w));
            svSettingsWidgetAttach(self->focusCarouselSlot, self->focusCarousel, buf, 4);
            svWidgetSetHidden(self->focusCarousel, true);
        }
    } else {
        self->focusCarousel = NULL;
    }
    free(buf);

    asprintf(&buf, "%s.Item.Carousel", cName);
    self->carousel = QBVerticalCarouselNewFromSM(app, buf);
    QBVerticalCarouselSetNotificationTarget(self->carousel, svWidgetGetId(self->w));
    svSettingsWidgetAttach(self->w, self->carousel, buf, 2);
    svWidgetSetHidden(self->carousel, true);
    free(buf);

    self->isAdult = false;

    int count = QBVerticalCarouselGetSlotCount(self->carousel);
    for (int i = 0; i < count; ++i) {
        asprintf(&buf, "%s.Item.Carousel.Slot", cName);
        QBVerticalCarouselSetContentAtIndex(self->carousel, i, QBVoDGridCarouselSlotCreate(app, buf, self, self->carousel));
        free(buf);
        QBVerticalCarouselSetDataHandler(self->carousel, i, QBVoDGridCarouselSlotSetObject);
        if (self->focusCarousel) {
            asprintf(&buf, "%s.Item.FocusCarousel.Slot", cName);
            QBVerticalCarouselSetContentAtIndex(self->focusCarousel, i, QBVoDGridCarouselSlotCreate(app, buf, self, self->focusCarousel));
            free(buf);
            QBVerticalCarouselSetDataHandler(self->focusCarousel, i, QBVoDGridCarouselSlotSetObject);
        }
    }
    if (grid->focusedVerticalMenuController == NULL) {
        QBVerticalCarouselSetRealignColumnCallback(self->carousel, self, QBVoDGridRealignFocusCarousel);
    }
    QBVerticalCarouselConnectToSource(self->carousel, self->dataSource, NULL);
    if (self->focusCarousel) {
        QBVerticalCarouselConnectToSource(self->focusCarousel, self->dataSource, NULL);
    }
    int len = SvInvokeInterface(QBListModel, self->dataSource, getLength);
    asprintf(&buf, "%s.Item.Carousel", cName);
    int firstFocusableColumn = svSettingsGetInteger(buf, "initialFocusColumn", 4);
    free(buf);
    if (len > firstFocusableColumn)
        self->activeElementIdx = firstFocusableColumn;
    else
        self->activeElementIdx = len - 1;

    QBVerticalCarouselSetActive(self->carousel, self->activeElementIdx);
    bool isFlatUI = (self->grid->focusedVerticalMenuController != NULL);
    if (isFlatUI) {
        if (self->focusCarousel)
            QBVerticalCarouselSetActive(self->focusCarousel, self->activeElementIdx);
    }

    if (grid->focus) {
        self->focusWidget = grid->focus;
    } else if (SvStringEqualToCString(verticalMenuName, "FocusedVerticalMenu")) {
        asprintf(&buf, "%s.Item.Focus", cName);
        self->focusWidget = QBFrameCreateFromSM(app, buf);
        svSettingsWidgetAttach(self->w, self->focusWidget, buf, 8);
        free(buf);
    } else {
        self->focusWidget = NULL;
    }

    self->settingsCtx = grid->settingsContext;

    self->w->prv = self;
    svWidgetSetUserEventHandler(self->w, QBVodGridBrowserMenuItemEventHandler);
    self->w->clean = menuItemClean;

out:
    SvErrorInfoPropagate(error, errorOut);

    if (error)
        return NULL;

    return self;
}

SvLocal void
QBVoDGridMenuItemSetActive(QBVoDGridMenuItem self, bool active)
{
    self->active = active;

    if (active) {
        if (self->frame) {
            self->frameParams.bitmap = self->activeBg;
            QBFrameSetBitmap(self->frame, &self->frameParams);
        }

        if (self->state == QBVoDGridState_initializing && self->focusWidget) {
            svWidgetSetHidden(self->focusWidget, true);
            return;
        }

        size_t len = SvInvokeInterface(QBListModel, self->dataSource, getLength);
        if (len >= 1 && self->state == QBVoDGridState_loaded && svWidgetIsFocused(self->grid->w)) {
            if (self->focusWidget) {
                svWidgetSetHidden(self->focusWidget, false);
            }
            if (self->focusCarouselSlot) {
                svWidgetSetHidden(self->focusCarouselSlot, false);
            }
            QBVoDGridMenuItemSetCarouselsVisibility(self, true);
        } else {
            if (self->focusWidget) {
                svWidgetSetHidden(self->focusWidget, true);
            }
            if (self->focusCarouselSlot) {
                svWidgetSetHidden(self->focusCarouselSlot, true);
            }

            if (len < 1)
                QBVoDGridMenuItemSetCarouselsVisibility(self, false);
        }
    } else {
        if (self->frame) {
            self->frameParams.bitmap = self->inactiveBg;
            QBFrameSetBitmap(self->frame, &self->frameParams);
        }
        if (self->state == QBVoDGridState_initializing)
            return;

        QBVerticalCarouselStopScrolling(self->carousel);
        if (self->focusCarouselSlot && !self->grid->focusedVerticalMenu) {
            svWidgetSetHidden(self->focusCarouselSlot, true);
        }
        QBVoDGridMenuItemRefreshCarousel(self);
    }

    if (self->grid->callbacks.QBVoDGridItemStateChanged) {
        struct QBVoDGridMenuItemInfo_s data = {
            .item     = self->w,
            .active   = self->active,
            .idx      = self->idx,
            .category = (SvObject) self->category,
            .count    = SvInvokeInterface(QBListModel, self->dataSource, getLength),
            .isAdult  = self->isAdult,
            .isLocked = QBVoDGridMenuItemIsLocked(self),
            .state    = self->state
        };

        self->grid->callbacks.QBVoDGridItemStateChanged(self->grid->owner, &data);
    }
}

SvLocal bool
QBVoDGridMenuItemHandleInputEvent(QBVoDGridMenuItem self, SvInputEvent ev)
{
    return QBVerticalCarouselHandleInputEvent(self->carousel, ev);
}

SvLocal void
QBVoDGridMenuItemRefreshCarousel(QBVoDGridMenuItem self)
{
    if (self->state == QBVoDGridState_initializing)
        return;

    size_t len = SvInvokeInterface(QBListModel, self->dataSource, getLength);
    SvInvokeInterface(QBListModelListener, self->carousel->prv, itemsChanged, 0, len);
    if (self->focusCarousel) {
        SvInvokeInterface(QBListModelListener, self->focusCarousel->prv, itemsChanged, 0, len);
    }
}

SvLocal void
QBVoDGridSetVisibilityOfPreviouslyFocusedItem(QBVoDGrid self)
{
    if (!self->activeRow) {
        return;
    }

    QBVoDGridMenuItem previousItem = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
    if (!previousItem) {
        return;
    }

    bool isFlatUI = (self->focusedVerticalMenuController != NULL);
    bool userSwitchedToNavigationBar = !svWidgetIsFocused(self->w);
    bool isSearchResultItem = SvObjectIsInstanceOf((SvObject) previousItem->category, QBContentSearch_getType());
    bool hideItem;
    if (isFlatUI) {
        hideItem = userSwitchedToNavigationBar;
    } else {
        hideItem = isSearchResultItem && !self->searchResultsAvailable;
    }
    QBVoDGridMenuItemSetCarouselsVisibility(previousItem, !hideItem);

    SVRELEASE(previousItem);
}

SvLocal void
QBVoDGridSetActiveItem(QBVoDGrid self, QBVoDGridMenuItem item, SvString verticalMenuName)
{
    bool menuIsFocusedVerticalMenu = SvStringEqualToCString(verticalMenuName, "FocusedVerticalMenu");
    if (menuIsFocusedVerticalMenu) {
        SVTESTRELEASE(self->focusedActiveRow);
        self->focusedActiveRow = SvWeakReferenceCreateWithObject((SvObject) item, NULL);
    } else {
        QBVoDGridSetVisibilityOfPreviouslyFocusedItem(self);
        SVTESTRELEASE(self->activeRow);
        self->activeRow = SvWeakReferenceCreateWithObject((SvObject) item, NULL);
    }

    self->activeCarouselId = svWidgetGetId(item->carousel);
    self->menuPosItem = item->activeElementIdx;

    bool isFlatUI = (self->focusedVerticalMenuController != NULL);
    bool showCarousel = (isFlatUI ? menuIsFocusedVerticalMenu : true);
    bool isSearchResultItem = SvObjectIsInstanceOf((SvObject) item->category, QBContentSearch_getType());
    if (isSearchResultItem) {
        showCarousel = showCarousel && self->searchResultsAvailable;
    }
    QBVoDGridMenuItemSetCarouselsVisibility(item, showCarousel);

    size_t len = SvInvokeInterface(QBListModel, item->dataSource, getLength);
    if (len >= 1 && svWidgetIsFocused(self->w)) {
        if (item->focusWidget) {
            svWidgetSetHidden(item->focusWidget, false);
        }

        if (item->focusCarouselSlot) {
            svWidgetSetHidden(item->focusCarouselSlot, false);
        }
    }

    QBVoDGridNewHighlight(self, item->highlightedObject, item);
}

SvLocal SvWidget
QBVoDGridControllerCreateMenuItem(SvObject self_, SvObject dataSource, size_t idx, SvApplication app, XMBMenuState initialState, bool isFocused)
{
    QBVoDGridController controller = (QBVoDGridController) self_;
    QBVoDGrid self = (QBVoDGrid) controller->parent;
    SvObject path = NULL;
    SvObject category = NULL;
    QBVoDGridMenuItem item;
    if (SvObjectIsInstanceOf(dataSource, QBListProxyFilter_getType())) {
        category = QBListProxyFilterGetTreeNode((QBListProxyFilter) dataSource, idx, &path);
    } else {
        category = QBListProxyGetTreeNode((QBListProxy) dataSource, idx, &path);
    }

    if (!category || !path) {
        return NULL;
    }

    svSettingsRestoreContext(self->settingsContext);

    item = (QBVoDGridMenuItem) SvTypeAllocateInstance(QBVoDGridMenuItem_getType(), NULL);

    item->idx = idx;

    item->initParams.path = path;
    item->initParams.isFocused = isFocused;

    QBVoDGridMenuItemInitialize(item, self, category, path, controller->verticalMenuName, NULL);

    SvString categoryName = NULL;
    if (category && SvObjectIsInstanceOf(category, QBContentCategory_getType())) {
        categoryName = QBContentCategoryGetName((QBContentCategory) category);
        SvValue isAdult = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) category, SVSTRING("isAdult"));
        if (isAdult) {
            if (SvValueIsInteger(isAdult))
                item->isAdult = (SvValueGetInteger(isAdult) == 0 ? false : true);
            else if (SvValueIsBoolean(isAdult))
                item->isAdult = SvValueGetBoolean(isAdult);
        }
    }

    svSettingsPopComponent();

    if (category && SvObjectIsInstanceOf(category, QBContentCategory_getType())) {
        SvValue catPath = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) category, SVSTRING("categoryPath"));
        bool isSearchResultItem = SvObjectIsInstanceOf((SvObject) category, QBContentSearch_getType());
        if (isSearchResultItem)
        {
            if (item->name)
            {
                QBAsyncLabelSetText(item->name, SvStringCreate(gettext("Search results"),NULL));
            }
        }
        else if (catPath && SvValueIsString(catPath)) {
            SvString categoryPath = SvValueGetString(catPath);
            if (item->name) {
                QBAsyncLabelSetText(item->name, categoryPath);
            }
        } else {
            if (item->name) {
                QBAsyncLabelSetText(item->name, categoryName);
            }
        }

        // If current category is unlocked then all subcategories also shoud be unlocked
        if (SvSetContainsElement(self->unlockedGridItems, (SvObject) QBContentCategoryGetId(self->category)))
            SvSetAddElement(self->unlockedGridItems, (SvObject) QBContentCategoryGetId((QBContentCategory) category));


        QBContentCategoryAddListener(item->category, (SvObject) item, NULL);

        QBVoDGridMenuItemSetActive(item, isFocused);
        if (isFocused)
            QBVoDGridSetActiveItem(self, item, controller->verticalMenuName);
    }

    if (self->callbacks.QBVoDGridItemCreated) {
        struct QBVoDGridMenuItemInfo_s data = {
            .item     = item->w,
            .active   = item->active,
            .idx      = item->idx,
            .category = (SvObject) item->category,
            .count    = SvInvokeInterface(QBListModel, item->dataSource, getLength),
            .isAdult  = item->isAdult,
            .isLocked = QBVoDGridMenuItemIsLocked(item),
            .state    = self->state
        };

        self->callbacks.QBVoDGridItemCreated(self->owner, &data);
    }

    QBVoDGridMenuItemCheckState(item);

    return item->w;
}

SvLocal SvWidget
QBVoDGridControllerCreateItem(SvObject self_, SvObject node_, SvObject path, SvApplication app, XMBMenuState initialState)
{
    QBVoDGridController self = (QBVoDGridController) self_;
    QBVoDGrid gridInfo = (QBVoDGrid) self->parent;

    svSettingsRestoreContext(gridInfo->settingsContext);

    SvWidget w = NULL;
    if (SvStringEqualToCString(self->verticalMenuName, "VerticalMenu")) {
        w = svSettingsWidgetCreate(app, "VerticalMenu.Item");
    } else if (SvStringEqualToCString(self->verticalMenuName, "FocusedVerticalMenu")) {
        w = svSettingsWidgetCreate(app, "FocusedVerticalMenu.Item");
    }

    svSettingsPopComponent();
    return w;
}

SvLocal void
QBVoDGridControllerSetItemState(SvObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBVoDGridController self = (QBVoDGridController) self_;
    QBVoDGrid grid = (QBVoDGrid) self->parent;
    QBVoDGridMenuItem item = item_->prv;

    if (!SvObjectIsInstanceOf((SvObject) item->category, QBContentCategory_getType()))
        return;
    QBVoDGridMenuItemSetActive(item, isFocused);

    if (isFocused)
        QBVoDGridSetActiveItem(grid, (QBVoDGridMenuItem) item_->prv, self->verticalMenuName);
}

SvLocal void
QBVoDGridCheckCategoryState(QBVoDGrid self)
{
    if (!SvObjectIsInstanceOf((SvObject) self->category, QBContentCategory_getType())) {
        return;
    }

    QBContentCategoryLoadingState categoryLoadingState = QBContentCategoryGetLoadingState(self->category);

    QBVoDGridState newState = self->state;
    size_t newLength = 0;

    if (self->state != QBVoDGridState_loaded) {
        if ((newLength = SvInvokeInterface(QBListModel, self->gridDataSource, getLength)) != 0) {
            newState = QBVoDGridState_loaded;
        } else if (categoryLoadingState == QBContentCategoryLoadingState_initial || categoryLoadingState == QBContentCategoryLoadingState_active) {
            newState = QBVoDGridState_loading;
        } else {
            newState = QBVoDGridState_noresults;
        }
    }

    if (newState != self->state) {
        if (self->state == QBVoDGridState_loading) {
            if (self->animation)
                svWidgetDestroy(self->animation);
        }

        if (newState == QBVoDGridState_loading) {
            svSettingsRestoreContext(self->settingsContext);
            self->animation = QBVoDGridCreateAnimation(self->w->app, "Animation");
            svSettingsWidgetAttach(self->w, self->animation, "Animation", 10);
            svSettingsPopComponent();
        } else if (newState == QBVoDGridState_loaded) {
            svWidgetSetHidden(self->resultsPane, false);
        }
        self->state = newState;

        if (self->state == QBVoDGridState_loaded) {
            if (self->focusedVerticalMenu) {
                XMBVerticalMenuSetPosition(self->focusedVerticalMenu, self->menuPos, true, NULL);
            }
            XMBVerticalMenuSetPosition(self->verticalMenu, self->menuPos, true, NULL);
        }
    }

    self->callbacks.QBVoDGridCategoryStateChanged(self->owner, self->state);
}

SvLocal void
QBVoDGridCategoryStateChanged(SvObject self_, QBContentCategory category, QBContentCategoryLoadingState previousState, QBContentCategoryLoadingState currentState)
{
    QBVoDGrid self = (QBVoDGrid) self_;
    QBVoDGridCheckCategoryState(self);
}

SvLocal void
QBVoDGridFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBVoDGrid self = w->prv;

    switch (e->kind) {
        case SvFocusEventKind_GET:
            if (self->focusedVerticalMenu) {
                SvInvokeInterface(XMBMenu, self->focusedVerticalMenu->prv, setState, XMBMenuState_normal);
            }
            SvInvokeInterface(XMBMenu, self->verticalMenu->prv, setState, XMBMenuState_normal);
            break;
        case SvFocusEventKind_LOST:
            if (self->focusedVerticalMenu) {
                SvInvokeInterface(XMBMenu, self->focusedVerticalMenu->prv, setState, XMBMenuState_inactive);
            }
            SvInvokeInterface(XMBMenu, self->verticalMenu->prv, setState, XMBMenuState_inactive);
            break;
        default:
            SvLogWarning("%s() : unexpected effect event kind [%d]", __func__, e->kind);
            return;
    }
}

SvLocal void
QBVoDGridRefreshMenuPosPath(QBVoDGrid self)
{
    SVTESTRELEASE(self->menuPosPath);
    self->menuPosPath = NULL;
    if (SvObjectIsInstanceOf(self->gridDataSource, QBListProxyFilter_getType())) {
        QBListProxyFilterGetTreeNode((QBListProxyFilter) self->gridDataSource, self->menuPos, &self->menuPosPath);
    } else {
        QBListProxyGetTreeNode((QBListProxy) self->gridDataSource, self->menuPos, &self->menuPosPath);
    }
    SVTESTRETAIN(self->menuPosPath);
}

SvLocal void
QBVoDGridSetMenuPos(QBVoDGrid self, int newMenuPos)
{
    if (newMenuPos != self->menuPos) {
        self->menuPos = newMenuPos;
        QBVoDGridRefreshMenuPosPath(self);
    }
}

SvLocal void
QBVoDGridOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBVoDGrid self = ((SvWidget) w_)->prv;

    if (self->focusedVerticalMenu) {
        QBVoDGridSetMenuPos(self, XMBVerticalMenuGetPosition(self->focusedVerticalMenu));
    }
    QBVoDGridSetMenuPos(self, XMBVerticalMenuGetPosition(self->verticalMenu));
    if (!data->clicked) {
        return;
    }

    QBVoDGridMenuItem item = NULL;
    if (self->activeRow)
        item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
    if (item) {
        bool isLocked = QBVoDGridMenuItemIsLocked(item);
        if (isLocked)
            QBVerticalCarouselStopScrolling(item->carousel);

        QBVoDGridEvent event = (QBVoDGridEvent) SvTypeAllocateInstance(QBVoDGridItemSelectedEvent_getType(), NULL);
        if (event) {
            event->category = SVRETAIN(item->category);
            event->categoryPath = SVRETAIN(item->path);
            event->object = SVTESTRETAIN(self->highlightedProduct);
            event->menuPostion = self->menuPos;
            event->menuItemPostion = self->menuPosItem;
            event->isLocked = QBVoDGridShouldBeBlocked(item, self->highlightedProduct);
            event->isAdult = QBVoDUtilsIsAdult(self->highlightedProduct);

            QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
            QBEventBusPostEvent(eventBus, (QBPeerEvent) event, (SvObject) self, NULL);
            SVRELEASE(event);
        }

        SVRELEASE(item);
    }
}

SvLocal void
QBVoDGridHandleCarouselNotification(QBVoDGrid self, SvWidgetId src, SvUserEvent e)
{
    if (e->code == SV_EVENT_QB_CAROUSEL_NOTIFICATION && src == self->activeCarouselId) {
        QBVerticalCarouselNotification not = e->prv;
        if (self->focusedVerticalMenu) {
            QBVoDGridSetMenuPos(self, XMBVerticalMenuGetPosition(self->focusedVerticalMenu));
        }
        QBVoDGridSetMenuPos(self, XMBVerticalMenuGetPosition(self->verticalMenu));
        self->menuPosItem = not->dataIndex;

        if (not->type == QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT) {
            QBVoDGridMenuItem item = NULL;
            if (self->activeRow)
                item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
            SvObject newHighlight = NULL;
            if (not->dataObject)
                newHighlight = not->dataObject;
            if (item) {
                item->activeElementIdx = not->dataIndex;;
                SVTESTRELEASE(item->highlightedObject);
                item->highlightedObject = SVTESTRETAIN(newHighlight);

                if (svWidgetIsFocused(item->grid->w) && item->focusCarouselSlot) {
                    svWidgetSetHidden(item->focusCarouselSlot, false);
                }
                if (newHighlight) {
                    QBVoDGridNewHighlight(self, newHighlight, item);
                }

                SVRELEASE(item);
            }
        }
    }
}

SvLocal bool
QBVoDGridInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBVoDGrid self = w->prv;
    bool res = false;

    if (self->focusedVerticalMenu) {
        SvInvokeInterface(XMBMenu, self->focusedVerticalMenu->prv, handleInputEvent, e);
    }
    if (SvInvokeInterface(XMBMenu, self->verticalMenu->prv, handleInputEvent, e)) {
        res = true;
        goto out;
    }

    int focusCarouselSlotIdx = QBVoDGrid_NoItemSelected;
    if (self->activeRow) {
        QBVoDGridMenuItem item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
        if (item) {
            focusCarouselSlotIdx = QBVerticalCarouselGetCurrentSlot(item->carousel);
            res = QBVoDGridMenuItemHandleInputEvent(item, e);
            SVRELEASE(item);
        }
    }

    if (self->focusedVerticalMenuController && self->focusedActiveRow) {
        QBVoDGridMenuItem itemFocused = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->focusedActiveRow);
        if (itemFocused) {
            if (focusCarouselSlotIdx != QBVoDGrid_NoItemSelected) {
                QBVerticalCarouselSlideToIndex(itemFocused->carousel, focusCarouselSlotIdx);
                QBVerticalCarouselSlideToIndex(itemFocused->focusCarousel, focusCarouselSlotIdx);
            }
            QBVoDGridMenuItemHandleInputEvent(itemFocused, e);
            QBVerticalCarouselHandleInputEvent(itemFocused->focusCarousel, e);
            SVRELEASE(itemFocused);
        }
    }

out:
    return res;
}

SvLocal void
QBVoDGridClean(SvApplication app, void *self_)
{
    QBVoDGrid self = self_;

    if (self->category) {
        QBContentCategoryRemoveListener(self->category, (SvObject) self, NULL);
    }

    self->w = NULL;
    SVRELEASE(self);
}

SvLocal void
QBVoDGridDestroy(void *self_)
{
    QBVoDGrid self = (QBVoDGrid) self_;

    assert(!self->w);

    SVTESTRELEASE(self->activeRow);
    SVTESTRELEASE(self->focusedActiveRow);
    SVTESTRELEASE(self->highlightedProduct);
    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->menuPosPath);

    SVTESTRELEASE(self->gridDataSource);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->tree);
    SVRELEASE(self->unlockedGridItems);
    SVRELEASE(self->verticalMenuController);
    SVTESTRELEASE(self->focusedVerticalMenuController);
}

SvLocal SvType
QBVoDGrid_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridDestroy
    };
    static const struct QBContentCategoryListener_ categoryListenerMethods = {
        .loadingStateChanged = QBVoDGridCategoryStateChanged
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGrid",
                            sizeof(struct QBVoDGrid_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContentCategoryListener_getInterface(), &categoryListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBVoDGridControllerDestroy(void *self_)
{
    QBVoDGridController self = (QBVoDGridController) self_;
    SVRELEASE(self->verticalMenuName);
}

SvLocal SvType
QBVoDGridController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBVoDGridControllerDestroy
    };
    static const struct XMBExtItemController_ xmbMethods = {
        .super_           = {
            .createItem   = QBVoDGridControllerCreateItem,
            .setItemState = QBVoDGridControllerSetItemState,
        },
        .createMenuItem   = QBVoDGridControllerCreateMenuItem
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBVoDGridController",
                            sizeof(struct QBVoDGridController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBExtItemController_getInterface(), &xmbMethods,
                            NULL);
    }
    return type;
}

SvLocal QBVoDGridController
QBVoDGridControllerCreate(const char *name, QBVoDGrid parent)
{
    QBVoDGridController self = (QBVoDGridController) SvTypeAllocateInstance(QBVoDGridController_getType(), NULL);
    self->parent = parent;
    self->verticalMenuName = SvStringCreate(name, NULL);
    return self;
}

SvWidget
QBVoDGridCreate(SvApplication app, const char *widgetName, AppGlobals appGlobals)
{
    QBVoDGrid self = (QBVoDGrid) SvTypeAllocateInstance(QBVoDGrid_getType(), NULL);

    SvWidget window = svSettingsWidgetCreate(app, "Grid");

    self->verticalMenuController = QBVoDGridControllerCreate("VerticalMenu", self);

    self->resultsPane = svSettingsWidgetCreate(app, "Grid");
    svWidgetAttach(window, self->resultsPane, 0, 0, 1);
    svWidgetSetHidden(self->resultsPane, true);

    self->settingsContext = svSettingsSaveContext();

    self->verticalMenu = XMBVerticalMenuNew(app, "VerticalMenu", NULL);
    svSettingsWidgetAttach(self->resultsPane, self->verticalMenu, "VerticalMenu", 3);

    if (svSettingsIsWidgetDefined("BackgroundDim")) {
        self->BackgroundDim = QBFrameCreateFromSM(app, "BackgroundDim");
        svSettingsWidgetAttach(self->resultsPane, self->BackgroundDim, "BackgroundDim", 4);
        svWidgetSetHidden(self->BackgroundDim, false);
    }

    if (svSettingsIsWidgetDefined("FrontClipping") && svSettingsIsWidgetDefined("FocusedVerticalMenu")) {
        self->focusedVerticalMenuController = QBVoDGridControllerCreate("FocusedVerticalMenu", self);

        self->frontClipping = svSettingsWidgetCreate(app, "FrontClipping");
        svSettingsWidgetAttach(self->resultsPane, self->frontClipping, "FrontClipping", 5);

        self->focusedVerticalMenu = XMBVerticalMenuNew(app, "FocusedVerticalMenu", NULL);
        svSettingsWidgetAttach(self->frontClipping, self->focusedVerticalMenu, "FocusedVerticalMenu", 7);
    } else {
        self->focusedVerticalMenuController = NULL;
        self->frontClipping = NULL;
        self->focusedVerticalMenu = NULL;
    }
    if (svSettingsIsWidgetDefined("Focus")) {
        self->focus = QBFrameCreateFromSM(app, "Focus");
        if (self->focus) {
            svSettingsWidgetAttach(self->resultsPane, self->focus, "Focus", 8);
        }
    } else {
        self->focus = NULL;
    }

    window->prv = self;
    window->clean = QBVoDGridClean;
    svWidgetSetInputEventHandler(window, QBVoDGridInputEventHandler);
    svWidgetSetFocusEventHandler(window, QBVoDGridFocusEventHandler);
    svWidgetSetFocusable(window, true);

    XMBVerticalMenuSetNotificationTarget(self->verticalMenu, window, QBVoDGridOnChangePosition);

    self->appGlobals = appGlobals;
    self->w = window;

    self->owner = NULL;
    self->callbacks.QBVoDGridItemCreated = NULL;
    self->callbacks.QBVoDGridItemStateChanged = NULL;
    self->callbacks.QBVoDGridCategoryStateChanged = NULL;
    self->callbacks.QBVoDGridSearchResultsFocusChanged = NULL;
    self->unlockedGridItems = SvSetCreate(1, NULL);

    self->searchResultsAvailable = false;

    return window;
}

void
QBVoDGridSetPosition(SvWidget w, int menuPos, int menuItemPos, bool immediately)
{
    QBVoDGrid self = w->prv;

    QBVoDGridSetVisibilityOfPreviouslyFocusedItem(self);

    if (menuPos >= 0) {
        QBVoDGridSetMenuPos(self, menuPos);
    }
    if (menuItemPos >= 0)
        self->menuPosItem = menuItemPos;

    if (menuPos >= 0 && self->state == QBVoDGridState_loaded) {
        if (self->focusedVerticalMenu) {
            XMBVerticalMenuSetPosition(self->focusedVerticalMenu, self->menuPos, immediately, NULL);
        }
        XMBVerticalMenuSetPosition(self->verticalMenu, self->menuPos, immediately, NULL);
    }

    if (menuItemPos >= 0 && self->activeRow) {
        QBVoDGridMenuItem item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
        QBVoDGridMenuItem itemFocused = NULL;
        if (self->focusedActiveRow) {
            itemFocused = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->focusedActiveRow);
        }
        if (item) {
            size_t itemsCnt = SvInvokeInterface(QBListModel, item->dataSource, getLength);
            int pos = menuItemPos < (int) itemsCnt ? menuItemPos : (int) itemsCnt - 1;

            QBVerticalCarouselSetActive(item->carousel, pos);
            if (item->focusCarousel) {
                QBVerticalCarouselSetActive(item->focusCarousel, pos);
            }
            if (itemFocused) {
                QBVerticalCarouselSetActive(itemFocused->carousel, pos);
                if (itemFocused->focusCarousel) {
                    QBVerticalCarouselSetActive(itemFocused->focusCarousel, pos);
                }
                SVRELEASE(itemFocused);
            }
            self->menuPosItem = pos;
            SVRELEASE(item);
        }
    }
}

void
QBVoDGridGetPosition(SvWidget w, int *menuPos, int *menuItemPos)
{
    QBVoDGrid self = w->prv;

    if (menuPos)
        *menuPos = self->menuPos;
    if (menuItemPos)
        *menuItemPos = self->menuPosItem;
}

int
QBVoDGridConnectToTree(SvWidget w, SvObject tree, SvObject path, SvObject filterNode)
{
    QBVoDGrid self = w->prv;
    SvObject pathCopy = path != NULL ? SvObjectCopy(path, NULL) : NULL;
    QBListProxy proxy;
    QBListProxyFilter filter = NULL;
    SvObject root;

    root = SvInvokeInterface(QBTreeModel, tree, getNode, pathCopy);
    proxy = QBListProxyCreate(tree, pathCopy, NULL);

    SVTESTRELEASE(self->gridDataSource);
    if (filterNode) {
        filter = QBListProxyFilterCreate(proxy, filterNode, QBContentSearch_getType(), NULL);
        self->gridDataSource = (SvObject) SVRETAIN(filter);
    } else {
        self->gridDataSource = (SvObject) SVRETAIN(proxy);
    }

    SVTESTRELEASE(self->tree);
    self->tree = SVRETAIN(tree);

    SVTESTRELEASE(self->path);
    self->path = pathCopy;

    if (self->category) {
        QBContentCategoryRemoveListener(self->category, (SvObject) self, NULL);
        SVRELEASE(self->category);
        self->category = NULL;
    }

    if (SvObjectIsInstanceOf(root, QBContentCategory_getType())) {
        self->category = (QBContentCategory) SVRETAIN(root);
        QBContentProvider provider = (QBContentProvider) QBContentCategoryTakeProvider(self->category);
        if (provider) {
            QBContentProviderStart(provider, self->appGlobals->scheduler);
            SVRELEASE(provider);
        }
    } else {
        SvObject subTree = QBActiveTreeGetMountedSubTree((QBActiveTree) self->tree, self->path);
        if (subTree) {
            SvObject node = SvInvokeInterface(QBTreeModel, subTree, getNode, NULL);
            if (node && SvObjectIsInstanceOf(node, QBContentCategory_getType())) {
                self->category = (QBContentCategory) SVRETAIN(node);
            }
        }
    }

    QBVoDGridRefreshMenuPosPath(self);

    if (self->focusedVerticalMenu) {
        XMBVerticalMenuConnectToDataSource(self->focusedVerticalMenu, (SvObject) self->gridDataSource, (SvObject) self->focusedVerticalMenuController, NULL);
    }
    XMBVerticalMenuConnectToDataSource(self->verticalMenu, (SvObject) self->gridDataSource, (SvObject) self->verticalMenuController, NULL);

    QBVoDGridCheckCategoryState(self);

    if (self->category) {
        QBContentCategoryAddListener(self->category, (SvObject) self, NULL);
    }

    SVTESTRELEASE(filter);
    SVRELEASE(proxy);

    return 0;
}

void
QBVoDGridSetScrollRange(SvWidget w, int start, int stop)
{
    QBVoDGrid self = w->prv;

    if (self->focusedVerticalMenu) {
        XMBVerticalMenuSetScrollRange(self->focusedVerticalMenu, start, stop);
    }
    XMBVerticalMenuSetScrollRange(self->verticalMenu, start, stop);
}

void
QBVoDGridUnlockCurrentItem(SvWidget w)
{
    QBVoDGrid self = w->prv;
    QBVoDGridMenuItem item = NULL;

    if (self->activeRow)
        item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
    if (item) {
        log_debug("Unlocked category Id: %s Name: %s", SvStringCString(QBContentCategoryGetId(item->category)),
                  QBContentCategoryGetName(item->category) ? SvStringCString(QBContentCategoryGetName(item->category)) : "N/A");
        SvSetAddElement(self->unlockedGridItems, (SvObject) QBContentCategoryGetId(item->category));

        QBVoDGridMenuItemRefreshCarousel(item);
        SVRELEASE(item);
    }
}

void
QBVoDGridUnlockSelectedCategory(SvWidget w)
{
    if (!w) {
        log_error("null parameter passed");
        return;
    }
    QBVoDGrid self = w->prv;

    if (!self || !self->highlightedProduct || !self->activeRow)
        return;

    QBVoDGridMenuItem item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);

    if (item) {
        QBContentCategory highlightedCategory = SvObjectIsInstanceOf(item->highlightedObject, QBContentCategory_getType()) ? (QBContentCategory) item->highlightedObject : NULL;
        if (highlightedCategory) {
            log_debug("Unlocked category Id: %s Name: %s", SvStringCString(QBContentCategoryGetId(highlightedCategory)),
                      QBContentCategoryGetName(highlightedCategory) ? SvStringCString(QBContentCategoryGetName(highlightedCategory)) : "N/A");
            SvSetAddElement(self->unlockedGridItems, (SvObject) QBContentCategoryGetId(highlightedCategory));
            QBVoDGridMenuItemRefreshCarousel(item);
        }
        SVRELEASE(item);
    }
}

void
QBVoDGridRegisterCallbacks(SvWidget w, SvObject owner, QBVoDGridCallbacks callbacks)
{
    QBVoDGrid self = w->prv;

    self->owner = owner;
    self->callbacks.QBVoDGridItemCreated = callbacks.QBVoDGridItemCreated;
    self->callbacks.QBVoDGridItemStateChanged = callbacks.QBVoDGridItemStateChanged;
    self->callbacks.QBVoDGridCategoryStateChanged = callbacks.QBVoDGridCategoryStateChanged;
    self->callbacks.QBVoDGridSearchResultsFocusChanged = callbacks.QBVoDGridSearchResultsFocusChanged;
}

SvSet
QBVoDGridGetUnlockedCategories(SvWidget w)
{
    if (!w) {
        log_error("null widget passed");
        return NULL;
    }
    QBVoDGrid self = w->prv;

    return self->unlockedGridItems;
}

void
QBVoDGridSetUnlockedCategories(SvWidget w, SvSet categories)
{
    if (!w || !categories) {
        log_error("null pointer passed (w: %p categories: %p)", w, categories);
        return;
    }

    QBVoDGrid self = w->prv;

    SVRELEASE(self->unlockedGridItems);
    self->unlockedGridItems = SVRETAIN(categories);
}

void
QBVoDGridSetSearchResultAvailability(SvWidget w, bool available)
{
    QBVoDGrid self = w->prv;

    self->searchResultsAvailable = available;
}

int
QBVoDGridActiveRowGetLength(SvWidget w)
{
    QBVoDGrid self = w->prv;
    
    if (self->activeRow)
    {
        QBVoDGridMenuItem item = (QBVoDGridMenuItem) SvWeakReferenceTakeReferredObject(self->activeRow);
        if (item)
        {
            if (item->category)
            {
                return QBContentCategoryGetLength(item->category);
            }
        }
    }
    
    return 0;
}
