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

#ifndef BASICPANE_H_
#define BASICPANE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/JSONserializer.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBExtendedInfoPane.h>

typedef struct QBBasicPane_t* QBBasicPane;
typedef struct QBBasicPaneItem_t* QBBasicPaneItem;
typedef struct QBBasicPaneItemParams_t* QBBasicPaneItemParams;
typedef void (*QBBasicPaneItemCallback)(void *self, SvString id, QBBasicPane pane, QBBasicPaneItem item);

SvType QBBasicPane_getType(void);
SvType QBBasicPaneItem_getType(void);

struct QBBasicPaneItem_t {
    struct SvObject_ super_;
    SvString caption, subcaption;
    SvString id;
    QBBasicPaneItemCallback callback;
    void *callbackData;
    SvGenericObject subpane;
    SvHashTable metadata;
    bool itemDisabled;
};

QBBasicPane QBBasicPaneCreateFromSettings(const char *settings, SvApplication app, SvScheduler scheduler, QBTextRenderer textRenderer, QBContextMenu ctxMenu, unsigned level, SvString widgetName);
void QBBasicPaneInit(QBBasicPane self, SvApplication app, SvScheduler scheduler, QBTextRenderer textRenderer, QBContextMenu ctxMenu, unsigned level, SvString widgetName);
void QBBasicPaneSetItemConstructor(QBBasicPane self, SvGenericObject itemCtor);

/**
 * Get index of item.
 *
 * @param[in] options   items handle
 * @param[in] id        id of item
 * @return              @c item index, if success, @c -1, otherwise
 **/
int QBBasicPaneFindItemIdxById(SvGenericObject options, SvString id);

/**
 * Parse file that contains description of BasicPaneItems.
 *
 * @param[in] app               SvApplication handle
 * @param[in] itemNamesFilename name of file
 * @return                      @c object that holds parsed items, if success, @c NULL, otherwise
 **/
SvGenericObject QBBasicPaneParseOptionsFile(SvApplication app, SvString itemNamesFilename);

void QBBasicPaneLoadOptionsFromFile(QBBasicPane self, SvString itemNamesFilename);
QBBasicPaneItem QBBasicPaneAddOption(QBBasicPane self, SvString id, SvString caption, QBBasicPaneItemCallback callback, void *callbackData);
QBBasicPaneItem QBBasicPaneAddOptionWithSubcaption(QBBasicPane self, SvString id, SvString caption, SvString subcaption, QBBasicPaneItemCallback callback, void *callbackData);
//Maybe insead of using (options,ids) it would be better to just give an array of QBBasicPaneItems ?
QBBasicPaneItem QBBasicPaneAddOptionWithConfirmation(QBBasicPane self, SvString id, SvString caption, SvString widgetName, SvArray options, SvArray ids, QBBasicPaneItemCallback callback, void *callbackData);
QBBasicPaneItem QBBasicPaneAddOptionWithOSK(QBBasicPane self, SvString id, SvString caption, SvString widgetName, QBOSKPaneKeyTyped callback, void *callbackData);
QBBasicPaneItem QBBasicPaneAddOptionWithContainer(QBBasicPane self, SvString id, SvString caption, SvString widgetName, QBContainerPaneCallbacks callbacks, void *callbackData);
QBBasicPaneItem QBBasicPaneAddOptionWithSubpane(QBBasicPane self, SvString id, SvString caption, SvGenericObject pane);
/**
 * Add subtitled option containing subpane.
 *
 * @param[in] self             handle to object QBBasicPane
 * @param[in] id               id of the option
 * @param[in] caption          string containing caption text
 * @param[in] subcaption       string containing subcaption text
 * @param[in] pane             handle to the subpane attached to option
 * @return                     handle to QBBasicPaneItem on success, @c NULL otherwise
 **/
QBBasicPaneItem QBBasicPaneAddOptionWithSubpaneAndSubcaption(QBBasicPane self, SvString id, SvString caption, SvString subcaption, SvObject pane);
void QBBasicPaneRemoveAll(QBBasicPane self);
size_t QBBasicPaneGetOptionsCount(QBBasicPane self);
SvWidget QBBasicPaneGetTitle(QBBasicPane self);
void QBBasicPaneSetTitle(QBBasicPane self, SvString title);
int QBBasicPaneGetLevel(QBBasicPane self);
void QBBasicPaneSetWidth(QBBasicPane self, int width);

/**
 * Set focus position on given option ID.
 *
 * @param[in] self             handle to object QBBasicPane
 * @param[in] id               id of the option on which the focus will be set
 * @param[in] immediately      @c true to skip animations
 **/
void QBBasicPaneSetPosition(QBBasicPane self, SvString id, bool immediately);

/**
 * Set focus position on given index.
 *
 * @param[in] self             handle to object QBBasicPane
 * @param[in] idx              index of the option
 * @param[in] immediately      @c true to skip animations
 **/
void QBBasicPaneSetPositionByIndex(QBBasicPane self, size_t idx, bool immediately);
SvString QBBasicPaneItemGetID(QBBasicPaneItem self);
void QBBasicPaneOptionPropagateObjectChange(QBBasicPane self, QBBasicPaneItem item);
void QBBasicPaneSetPrv(QBBasicPane self, void *prv);
void *QBBasicPaneGetPrv(QBBasicPane self);
QBContextMenu QBBasicPaneGetContextMenu(QBBasicPane self);

#endif
