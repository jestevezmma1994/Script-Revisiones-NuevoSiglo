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


#include "QBSearchByCategoryPane.h"
#include <settings.h>
#include <main.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBSearch/QBSearchAgent.h>
#include <QBSearch/QBSearchEngine.h>
#include <Windows/searchResults.h>
#include <ContextMenus/QBBasicPaneInternal.h>
#include <SearchAgents/EPGManSearchAgent.h>

struct QBSearchByCategoryPane_t {
    struct QBBasicPane_t super_;

    AppGlobals appGlobals;
    SvHashTable categoryIdToEventId;
    SvEPGView channelList;
    QBWindowContext searchResults; ///< context managing search results or lack thereof
};

SvLocal int QBSearchByCategoryResolveNameToID(QBSearchByCategoryPane self, SvString categoryName)
{
    return SvValueGetInteger((SvValue) SvHashTableFind(self->categoryIdToEventId, (SvObject) categoryName));
}

SvLocal void QBSearchByCategoryPaneOnEventTypeChosen(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBSearchByCategoryPane self = (QBSearchByCategoryPane) self_;
    SVTESTRELEASE(self->searchResults);
    self->searchResults = QBSearchResultsContextCreate(self->appGlobals);

    SvArray agents = SvArrayCreate(NULL);
    SvObject agent = QBInitLogicCreateSearchAgent(self->appGlobals->initLogic,
                                                  EPGManSearchAgentMode_findByType,
                                                  QBSearchByCategoryResolveNameToID(self, id),
                                                  (SvObject) self->channelList);

    SvArrayAddObject(agents, agent);
    SVRELEASE(agent);

    SvObject search = QBSearchEngineCreate(agents, item->caption, NULL);
    SVRELEASE(agents);

    if (SvArrayCount(QBSearchEngineGetKeywords(search))) {
        QBSearchResultsSetDataSource(self->searchResults, search);
        QBSearchResultsContextExecute(self->searchResults, self->super_.contextMenu);
    }

    SVRELEASE(search);
}

/*
 * Typical stuff for pane management
 */

SvLocal void QBSearchByCategoryPaneDestroy(void *ptr)
{
    QBSearchByCategoryPane self = ptr;
    SVRELEASE(self->categoryIdToEventId);
    SVRELEASE(self->channelList);
    SVTESTRELEASE(self->searchResults);
}

SvType QBSearchByCategoryPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSearchByCategoryPaneDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSearchByCategoryPane",
                            sizeof(struct QBSearchByCategoryPane_t),
                            QBBasicPane_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBSearchByCategoryPaneAddOption(QBSearchByCategoryPane self, SvString id, int eventId)
{
    QBBasicPaneAddOption((QBBasicPane) self, id, NULL, QBSearchByCategoryPaneOnEventTypeChosen, self);
    SvValue eventIdVal = SvValueCreateWithInteger(eventId, NULL);
    SvHashTableInsert(self->categoryIdToEventId, (SvObject) id, (SvObject) eventIdVal);
    SVRELEASE(eventIdVal);
}

/*
 * Initialization
 */

SvLocal void QBSearchByCategoryPaneInit(QBSearchByCategoryPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvString itemNamesFilename, int level, SvEPGView channelList)
{
    self->appGlobals = appGlobals;
    self->categoryIdToEventId = SvHashTableCreate(11, NULL);
    self->channelList = SVRETAIN(channelList);
    QBBasicPaneInit((QBBasicPane) self, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer, ctxMenu, level, widgetName);
    QBBasicPaneLoadOptionsFromFile((QBBasicPane) self, itemNamesFilename);

    QBSearchByCategoryPaneAddOption(self, SVSTRING("movie"), 1);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("news"), 2);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("show"), 3);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("sports"), 4);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("children"), 5);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("music"), 6);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("arts"), 7);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("social"), 8);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("education"), 9);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("leisure"), 10);
    QBSearchByCategoryPaneAddOption(self, SVSTRING("special"), 11);
}

QBBasicPane QBSearchByCategoryPaneCreateFromSettings(const char *settings, const char *itemNamesFilenameStr, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, int level, SvEPGView channelList)
{
    svSettingsPushComponent(settings);
    QBSearchByCategoryPane self = (QBSearchByCategoryPane) SvTypeAllocateInstance(QBSearchByCategoryPane_getType(), NULL);
    SvString itemNamesFilename = SvStringCreate(itemNamesFilenameStr, NULL);
    QBSearchByCategoryPaneInit(self, appGlobals, ctxMenu, widgetName, itemNamesFilename, level, channelList);
    SVRELEASE(itemNamesFilename);
    svSettingsPopComponent();

    return (QBBasicPane) self;
}
