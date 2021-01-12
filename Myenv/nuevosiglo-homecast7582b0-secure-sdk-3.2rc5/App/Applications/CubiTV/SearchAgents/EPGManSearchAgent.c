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

#include "EPGManSearchAgent.h"

#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGDataListener.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <QBSearch/QBSearchAgent.h>
#include <QBSearch/QBSearchUtils.h>
#include <QBSearch/QBSearchResult.h>
#include <Utils/utf8utils.h>
#include <Utils/QBEventUtils.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>


struct EPGManSearchAgent_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBSearchAgentCallbacks *callbacks;
    SvWeakReference owner;

    SvFiber fiber;

    SvArray keywords;
    SvString keywordsStr;

    SvEPGManager epgManager;
    int channelIdx;
    time_t lastEventEndTime;
    SvEPGView view;
    SvEPGDataWindow dataWindow;
    EPGManSearchAgentMode mode;

    time_t backSearchLimit;             // search back limit for catchup events
    time_t startTime;
    time_t endTime;
    int category;
    bool started;
};
typedef struct EPGManSearchAgent_t *EPGManSearchAgent;

#define CHARS_IN_SIMILAR_MODE 4

SvLocal void
EPGManSearchAgentStep(EPGManSearchAgent self)
{
    SvTVChannel channel = NULL;
    SvEPGEvent event = NULL;

    SvString truncatedKeyword = NULL;
    if (self->mode == EPGManSearchAgentMode_findSimilar) {
        SvString keyword = QBUTF8CreateSubString(self->keywordsStr, 0, CHARS_IN_SIMILAR_MODE);
        truncatedKeyword = QBSearchCreateNormalizedString(keyword, NULL);
        SVRELEASE(keyword);
    }

    int size = SvInvokeInterface(SvEPGChannelView, self->view, getCount);
    if (self->channelIdx < 0 || self->channelIdx >= size){
        self->channelIdx = 0;

        SvObject owner = SvWeakReferenceTakeReferredObject(self->owner);
        self->callbacks->updateProgress(owner, (SvObject) self, 100);
        SVRELEASE(owner);

        // We detach from data window refresh, cause every small EPG change will refresh search window and that is lagging GUI.
        // Flaw of this solution is that we will be showing EPG events that could be removed from epgManager (user shouldn't be able to set reminder for them).
        SvEPGDataWindowRemoveListener(self->dataWindow, (SvGenericObject)self, NULL);
        SvFiberDeactivate(self->fiber);
        SvLogNotice("QBEPGSearch - end");
        SvTimeRange timeRange = SvTimeRangeConstruct(self->startTime, self->endTime);
        SvEPGDataWindowSetTimeRange(self->dataWindow, timeRange, NULL); // this is required to refresh search results
        SVTESTRELEASE(truncatedKeyword);
        return;
    }

    if (!self->lastEventEndTime) {
        self->lastEventEndTime = SvTimeNow() - self->backSearchLimit;
    }

    channel = SvInvokeInterface(SvEPGChannelView, self->view, getByIndex, self->channelIdx);
    SvEPGIterator iter = SvEPGManagerCreateIterator(self->epgManager, SvDBObjectGetID((SvDBObject) channel), self->lastEventEndTime);

    SvArray results = SvArrayCreate(NULL);

    while((event = SvEPGIteratorGetNextEvent(iter))){
        self->lastEventEndTime = event->endTime;
        int relevanceFactor1 = 0, relevanceFactor2 = 0;
        if (self->mode == EPGManSearchAgentMode_normal) {
            SvEPGEventDesc desc = QBEventUtilsGetSearchableDescFromEvent(self->appGlobals->eventsLogic,
                                                                         self->appGlobals->langPreferences,
                                                                         event);
            if (desc) {
                relevanceFactor1 = QBSearchGetRelevance(desc->title, self->keywords);
                relevanceFactor2 = QBSearchGetRelevance(desc->description, self->keywords);
            }
        } else if (self->mode == EPGManSearchAgentMode_titleOnly) {
            SvEPGEventDesc desc = QBEventUtilsGetSearchableDescFromEvent(self->appGlobals->eventsLogic,
                                                                         self->appGlobals->langPreferences,
                                                                         event);
            if (desc) {
                relevanceFactor1 = QBSearchGetRelevance(desc->title, self->keywords);
            }
        } else if (self->mode == EPGManSearchAgentMode_findSimilar) {
            SvEPGEventDesc desc = QBEventUtilsGetSearchableDescFromEvent(self->appGlobals->eventsLogic,
                                                                         self->appGlobals->langPreferences,
                                                                         event);
            SvString title = QBUTF8CreateSubString((desc && desc->title ? desc->title : SVSTRING("")), 0, CHARS_IN_SIMILAR_MODE);
            SvString titleNormalized = QBSearchCreateNormalizedString(title, NULL);
            relevanceFactor1 = SvObjectEquals((SvObject) titleNormalized, (SvObject) truncatedKeyword);
            SVRELEASE(title);
            SVRELEASE(titleNormalized);
        } else if (self->mode == EPGManSearchAgentMode_findByType) {
            relevanceFactor1 = self->category == event->category ? 1 : 0;
        }

        if ((relevanceFactor1 > 0 || relevanceFactor2 > 0) && (self->category == 0 || event->category == 0 || self->category == event->category)) {
            int relevanceFactor3 = QBSEARCH_MAX_RELEVANCE - ((((long long)(event->startTime - SvTimeNow() + 3600))*QBSEARCH_MAX_RELEVANCE)/(60*60*24*30));
            bool futureOrCatchupChannel = false;

            if ((SvEPGEventIsCatchupEnabled(event) != 0) || (event->endTime > SvTimeNow())) {
                // This flag allows filtering out noncatchup, past events
                futureOrCatchupChannel = true;
            }
            if ((relevanceFactor3 > 0) && (futureOrCatchupChannel == 1)) {
                if (self->startTime > event->startTime) {
                    self->startTime = event->startTime;
                }
                if (self->endTime < event->endTime) {
                    self->endTime = event->endTime;
                }
                QBSearchResult result = (QBSearchResult) SvTypeAllocateInstance(QBSearchResult_getType(), NULL);
                result->obj = SVRETAIN(event);
                result->relevance = relevanceFactor3;
                SvArrayAddObject(results, (SvGenericObject)result);
                SVRELEASE(result);
            }
        }

        if (SvFiberTimePassed()) {
            goto ret;
        }
    }

    self->channelIdx++;
    self->lastEventEndTime = 0;

ret:
    SVTESTRELEASE(iter);

    SvObject owner = SvWeakReferenceTakeReferredObject(self->owner);
    self->callbacks->addResults(owner, (SvObject) self, results);
    SVRELEASE(owner);

    SVRELEASE(results);
    SVTESTRELEASE(truncatedKeyword);
}

SvLocal void
EPGManSearchAgentSetCallbacks(SvObject self_, QBSearchAgentCallbacks *callbacks, SvObject owner)
{
    EPGManSearchAgent self = (EPGManSearchAgent) self_;

    SVTESTRELEASE(self->owner);
    self->owner = SvWeakReferenceCreate(owner, NULL);
    self->callbacks = callbacks;
}

SvLocal void
EPGManSearchAgentStart(SvGenericObject self_)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;

    if (self->started) {
        return;
    }

    if (!self->owner ||
        !self->callbacks) {
        SvLogError("%s(): owner or callbacks not set", __func__);
        return;
    }

    SvObject owner = SvWeakReferenceTakeReferredObject(self->owner);
    self->keywords = SVRETAIN(self->callbacks->getKeywords(owner));
    self->keywordsStr = SVRETAIN(self->callbacks->getKeywordsString(owner));
    SVRELEASE(owner);

    self->started = true;

    self->channelIdx = 0;
    SvFiberActivate(self->fiber);
    SvEPGDataWindowAddListener(self->dataWindow, (SvGenericObject)self, NULL);
    SvEPGDataWindowSetChannelsRange(self->dataWindow, 0, 0, NULL);
    self->startTime = self->endTime = SvTimeNow();
}

SvLocal void
EPGManSearchAgentStop(SvGenericObject self_)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;

    if (!self->started) {
        return;
    }

    SVRELEASE(self->keywords);
    SVRELEASE(self->keywordsStr);

    self->started = false;

    SvEPGDataWindowRemoveListener(self->dataWindow, (SvGenericObject)self, NULL);
    SvFiberDeactivate(self->fiber);
}

void
EPGManSearchAgentSetMode(SvGenericObject self_, EPGManSearchAgentMode mode)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;
    self->mode = mode;
}

void
EPGManSearchAgentSetCategory(SvGenericObject self_, int category)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;
    self->category = category;
}

void
EPGManSearchAgentSetView(SvGenericObject self_, SvGenericObject view)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;
    SVTESTRELEASE(self->view);
    self->view = SVRETAIN(view);
    SvEPGDataWindowSetChannelView(self->dataWindow, (SvGenericObject) self->view, NULL);
}

SvLocal void
EPGManSearchAgent__dtor__(void *self_)
{
    EPGManSearchAgent self = self_;
    EPGManSearchAgentStop(self_);
    SvFiberDestroy(self->fiber);
    SVRELEASE(self->epgManager);
    SVRELEASE(self->view);
    SVTESTRELEASE(self->dataWindow);

    SVTESTRELEASE(self->owner);
}

SvLocal void
EPGManSearchAgentDataUpdated(SvGenericObject self_,
                                   const SvTimeRange *timeRange,
                                   SvValue channelID)
{
    EPGManSearchAgent self = (EPGManSearchAgent)self_;
    SvObject owner = SvWeakReferenceTakeReferredObject(self->owner);
    self->callbacks->changeNotify(owner);
    SVRELEASE(owner);
}

SvLocal void
QBEPGWatcherStubFunction(SvGenericObject self_,
                         SvTVChannel channel)
{
}

SvLocal void
QBEPGWatcherChannelListCompletedStubFunction(SvGenericObject self_, int pluginID)
{
}

SvLocal SvType
EPGManSearchAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = EPGManSearchAgent__dtor__
    };
    static SvType type = NULL;

    static const struct QBSearchAgent_ methods = {
        .setCallbacks   = EPGManSearchAgentSetCallbacks,
        .start          = EPGManSearchAgentStart,
        .stop           = EPGManSearchAgentStop,
    };

    static const struct SvEPGDataListener_ dlMethods = {
        .dataUpdated = EPGManSearchAgentDataUpdated
    };

    static const struct SvEPGChannelListListener_ listMethods = {
        .channelFound         = QBEPGWatcherStubFunction,
        .channelLost          = QBEPGWatcherStubFunction,
        .channelModified      = QBEPGWatcherStubFunction,
        .channelListCompleted = QBEPGWatcherChannelListCompletedStubFunction
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("EPGManSearchAgent",
                            sizeof(struct EPGManSearchAgent_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSearchAgent_getInterface(), &methods,
                            SvEPGDataListener_getInterface(), &dlMethods,
                            SvEPGChannelListListener_getInterface(), &listMethods,
                            NULL);
    }

    return type;
}

SvGenericObject
EPGManSearchAgentCreate(AppGlobals appGlobals)
{
    EPGManSearchAgent self = (EPGManSearchAgent) SvTypeAllocateInstance(EPGManSearchAgent_getType(), NULL);
    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "EPGSearch", (SvFiberStepFun*)&EPGManSearchAgentStep, self);
    SvFiberSetPriority(self->fiber, 50);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->view = SVRETAIN(QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels")));
    self->dataWindow = SvEPGManagerCreateDataWindow(self->epgManager, NULL);
    SvEPGDataWindowSetPassive(self->dataWindow, true);

    return (SvGenericObject) self;
}

void
EPGManSearchAgentSetSearchBackLimitSeconds(SvObject self_, time_t limit)
{
    EPGManSearchAgent self = (EPGManSearchAgent) self_;
    self->backSearchLimit = limit;
}
