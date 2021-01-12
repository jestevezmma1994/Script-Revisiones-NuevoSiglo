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

#ifndef QB_VOD_GRID_H_
#define QB_VOD_GRID_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvSet.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>
#include <ContextMenus/QBContextMenu.h>
#include <Services/core/QBTextRenderer.h>
#include <main_decl.h>


/**
 * VoD grid event class.
 * @class QBVoDGridEvent
 * @extends QBUIEvent
 **/
typedef struct QBVoDGridEvent_ *QBVoDGridEvent;

/**
 * QBVoDGridEvent class internals.
 */
struct QBVoDGridEvent_ {
    struct QBUIEvent_ super_; ///< super class

    unsigned int menuPostion;
    unsigned int menuItemPostion;

    /**
     * Flag indicating if selected slot is locked.
     *
     * Selected slot is locked when it belongs to locked category (all categories are
     * locked by default but when we unlock at least one item we also unlock all category)
     * and it is for adult or it's rating is blocked by parental control.
     **/
    bool isLocked;

    /// Flag indicating if selected slot contains product for adults.
    bool isAdult;

    SvObject category;
    SvObject categoryPath;
    SvObject object;
};

/**
 * Get runtime type identification object representing QBVoDGridEvent class.
 * @return VoD grid event class
 **/
SvType
QBVoDGridEvent_getType(void);


/**
 * VoD grid item focused event class.
 * @class QBVoDGridItemFocusedEvent
 * @extends QBVoDGridEvent
 **/
typedef struct QBVoDGridItemFocusedEvent_ *QBVoDGridItemFocusedEvent;

/**
 * QBVoDGridItemFocusedEvent class internals.
 **/
struct QBVoDGridItemFocusedEvent_ {
    struct QBVoDGridEvent_ super_; ///< super class;
};

/**
 * Get runtime type identification object representing QBVoDGridItemFocusedEvent class.
 * @return VoD grid item focused event class
 **/
SvType
QBVoDGridItemFocusedEvent_getType(void);


/**
 * VoD grid item selected event class.
 * @class QBVoDGridItemSelectedEvent
 * @extends QBVoDGridEvent
 **/
typedef struct QBVoDGridItemSelectedEvent_ *QBVoDGridItemSelectedEvent;

/**
 * QBVoDGridItemSelectedEvent class internals.
 **/
struct QBVoDGridItemSelectedEvent_ {
    struct QBVoDGridEvent_ super_; ///< super class;
};

/**
 * Get runtime type identification object representing QBVoDGridItemSelectedEvent class.
 * @return VoD grid item selected event class
 **/
SvType
QBVoDGridItemSelectedEvent_getType(void);


typedef enum {
    QBVoDGridState_unknown = 0,
    QBVoDGridState_initializing,
    QBVoDGridState_initialized,
    QBVoDGridState_loading,
    QBVoDGridState_noresults,
    QBVoDGridState_loaded,
} QBVoDGridState;

/**
 * Basic menu item attributes
 */
typedef struct QBVoDGridMenuItemInfo_s {
    SvWidget item;
    bool active;
    unsigned int idx;
    unsigned int count;
    bool isAdult;
    bool isLocked;
    SvObject category;
    QBVoDGridState state;

    SvTimerId timer;
    double longInitDelay;
    double shortInitDelay;
} *QBVoDGridMenuItemInfo;

/**
 * VoD Grid callbacks
 */
typedef struct {
    /**
     * New menu item was has been created.
     *
     * @param[in] owner_        callback owner
     * @param[in] menuItemInfo  info about created item
     **/
    void (*QBVoDGridItemCreated)(SvObject owner_, QBVoDGridMenuItemInfo menuItemInfo);

    /**
     * Menu item state has changed.
     *
     * @param[in] owner_        callback owner
     * @param[in] menuItemInfo  info about created item
     **/
    void (*QBVoDGridItemStateChanged)(SvObject owner_, QBVoDGridMenuItemInfo menuItemInfo);

    /**
     * Category state has changed.
     *
     * @param[in] owner_        callback owner
     * @param[in] state         new state of category
     **/
    void (*QBVoDGridCategoryStateChanged)(SvObject owner_, QBVoDGridState state);

    /**
     * Search results focus has changed.
     *
     * @param[in] owner_        callback owner
     * @param[in] focused       true if search results are now focused
     **/
    void (*QBVoDGridSearchResultsFocusChanged)(SvObject owner_, bool focused);
} QBVoDGridCallbacks;

/**
 * Create a new VoD Grid widget.
 * VoD Grid widget represents a grid menu created by combining vertical menu
 * and carousel widgets.
 * When initialized, vertical menu is created using given datasource.
 * When browsing the grid, menu items are created, each one containing a carousel
 * filled with objects from given menu item's datasource (main tree's subtree).
 * VoD Grid sends notifications and callbacks about grid object/menu item state changes.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @param[in] appGlobals    application globals
 * @return                  created widget, @c NULL in case of error
 **/
SvWidget
QBVoDGridCreate(SvApplication app, const char *widgetName, AppGlobals appGlobals);

/**
 * Connect VoD Grid menu to given datasource.
 *
 * @param[in] w             VoD Grid widget handle
 * @param[in] tree          menu tree - datasource
 * @param[in] path          menu path
 * @param[in] filterNode    node of tree that should be filtered as only category displayed by grid,
 *                          or @c NULL if no filtering is to be applied
 * @return                  @c 0, error code in case of error
 **/
int
QBVoDGridConnectToTree(SvWidget w, SvObject tree, SvObject path, SvObject filterNode);

/**
 * Focus an element at selected position.
 *
 * @param[in] w                 VoD Grid widget handle
 * @param[in] menuPos           vertical menu position (y)
 * @param[in] menuItemPos       carousel position (x)
 * @param[in] immediately       switch immediately if set
 **/
void
QBVoDGridSetPosition(SvWidget w, int menuPos, int menuItemPos, bool immediately);

/**
 * Get current menu position.
 *
 * @param[in] w                 VoD Grid widget handle
 * @param[in] menuPos           vertical menu position (y)
 * @param[in] menuItemPos       carousel position (x)
 **/
void
QBVoDGridGetPosition(SvWidget w, int* menuPos, int* menuItemPos);

/**
 * Set grid's vertical menu scrolling bounds.
 *
 * @param[in] w                 VoD Grid widget handle
 * @param[in] start             upper bound
 * @param[in] stop              lower bound
 **/
void
QBVoDGridSetScrollRange(SvWidget w, int start, int stop);

/**
 * Drop current item's isLocked flag.
 *
 * @param[in] w                 VoD Grid widget handle
 **/
void
QBVoDGridUnlockCurrentItem(SvWidget w);

/**
 * Drop flag isLocked for selected category.
 *
 * @param[in] w                 VoD Grid widget handle
 **/
void
QBVoDGridUnlockSelectedCategory(SvWidget w);

/**
 * Register callbacks.
 *
 * @param[in] w                 VoD Grid widget handle
 * @param[in] owner        callback owner
 * @param[in] callbacks         callbacks
 **/
void
QBVoDGridRegisterCallbacks(SvWidget w, SvObject owner, QBVoDGridCallbacks callbacks);

/**
 * Get list of unclocked categories.
 *
 * @param[in] w             VoD Grid widget handle
 * @return                  Unlocked categories. Key - string with category id. Value - flag set to true when category is unlocked.
 **/
SvSet
QBVoDGridGetUnlockedCategories(SvWidget w);

/**
 * Set list of unclocked categories.
 *
 * @param[in] w             VoD Grid widget handle
 * @param[in] categories    Set with unlocked categories.
 **/
void
QBVoDGridSetUnlockedCategories(SvWidget w, SvSet categories);

/**
 * Set availability of search results.
 *
 * @param[in] w             VoD Grid widget handle
 * @param[in] available     true if search results are available
 **/
void
QBVoDGridSetSearchResultAvailability(SvWidget w, bool available);

/**
 * Get active row lenght
 *
 * @param[in] w             VoD Grid widget handle
 * @return                  Current category lenght
 **/
int QBVoDGridActiveRowGetLength(SvWidget w);

#endif

