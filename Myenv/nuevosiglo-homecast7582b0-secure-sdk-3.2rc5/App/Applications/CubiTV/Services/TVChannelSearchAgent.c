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

#include <QBSearch/QBSearchAgent.h>
#include <QBSearch/QBSearchUtils.h>
#include <QBSearch/QBSearchResult.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include "TVChannelSearchAgent.h"

struct QBTVChannelSearchAgent_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    QBSearchAgentCallbacks *callbacks;
    SvWeakReference owner;
};
typedef struct QBTVChannelSearchAgent_t *QBTVChannelSearchAgent;

SvLocal void
QBTVChannelSearchSetCallbacks(SvObject self_, QBSearchAgentCallbacks *callbacks, SvObject owner)
{
    QBTVChannelSearchAgent self = (QBTVChannelSearchAgent) self_;

    SVTESTRELEASE(self->owner);
    self->owner = SvWeakReferenceCreate(owner, NULL);
    self->callbacks = callbacks;
}

SvLocal void
QBTVChannelSearchStart(SvGenericObject self_)
{
    QBTVChannelSearchAgent self = (QBTVChannelSearchAgent) self_;
    if (!self->owner ||
        !self->callbacks) {
        SvLogError("%s(): owner or callbacks not set", __func__);
        return;
    }

    SvObject owner = SvWeakReferenceTakeReferredObject(self->owner);
    SvArray keywords = self->callbacks->getKeywords(owner);
    SVRELEASE(owner);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject tvChannels = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
    int i, n;
    SvArray results = SvArrayCreate(NULL);
    n = SvInvokeInterface(QBListModel, tvChannels, getLength);
    for(i=0; i<n; i++){
        SvTVChannel channel = (SvTVChannel)SvInvokeInterface(QBListModel, tvChannels, getObject, i);
        int relevance = QBSearchGetRelevance(channel->name, keywords);
        if(relevance > 0){
            QBSearchResult result = (QBSearchResult) SvTypeAllocateInstance(QBSearchResult_getType(), NULL);
            result->obj = SVRETAIN(channel);
            result->relevance = relevance;
            SvArrayAddObject(results, (SvGenericObject)result);
            SVRELEASE(result);
        }
    }

    // send those results to owner
    if (SvArrayCount(results) > 0) {
        owner = SvWeakReferenceTakeReferredObject(self->owner);
        self->callbacks->addResults(owner, (SvObject) self, results);
        SVRELEASE(owner);
    }

    SVRELEASE(results);
}

SvLocal void
QBTVChannelSearchStop(SvGenericObject self_)
{
}

SvLocal void
QBTVChannelSearchAgent__dtor__(void *self_)
{
    QBTVChannelSearchAgent self = self_;
    QBTVChannelSearchStop((SvGenericObject) self);

    SVTESTRELEASE(self->owner);
}

SvLocal SvType
QBTVChannelSearchAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVChannelSearchAgent__dtor__
    };
    static const struct QBSearchAgent_ methods = {
        .setCallbacks     = QBTVChannelSearchSetCallbacks,
        .start            = QBTVChannelSearchStart,
        .stop             = QBTVChannelSearchStop,
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVChannelSearchAgent",
                            sizeof(struct QBTVChannelSearchAgent_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSearchAgent_getInterface(), &methods,
                            NULL);
    }

    return type;
}


SvGenericObject QBTVChannelSearchAgentCreate(AppGlobals appGlobals){
    QBTVChannelSearchAgent self = (QBTVChannelSearchAgent) SvTypeAllocateInstance(QBTVChannelSearchAgent_getType(), NULL);
    self->appGlobals = appGlobals;

    return (SvGenericObject) self;
}
