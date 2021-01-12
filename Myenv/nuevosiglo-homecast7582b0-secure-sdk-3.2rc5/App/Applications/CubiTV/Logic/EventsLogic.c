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

#include <Logic/EventsLogic.h>

#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvValue.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/langPreferences.h>
#include <QBPCRatings/QBPCUtils.h>

#define MPAA_RATING_OFFSET  20

struct EventsLogic_ {
    struct SvObject_ super;

    SvEPGManager epgManager;
    QBParentalControl pc;
    QBParentalControlLogic pcLogic;
    QBLangPreferences langPreferences;

    QBPCList notRatedRating;
};

SvLocal void
EventsLogicDestroy(void *self_)
{
    EventsLogic self = self_;

    SVRELEASE(self->epgManager);
    SVRELEASE(self->pc);
    SVRELEASE(self->pcLogic);
    SVRELEASE(self->langPreferences);

    SVTESTRELEASE(self->notRatedRating);
}

SvType
EventsLogic_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = EventsLogicDestroy
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("EventsLogic",
                            sizeof(struct EventsLogic_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

EventsLogic
EventsLogicCreate(QBParentalControl pc,
                  QBParentalControlLogic pcLogic,
                  QBLangPreferences langPreferences,
                  SvErrorInfo *errorOut)
{
    EventsLogic self = NULL;
    SvErrorInfo error = NULL;

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));

    if (!epgManager || !pc || !pcLogic || !langPreferences) {
        error = SvErrorInfoCreate(SvCoreErrorDomain,
                                  SvCoreError_invalidArgument,
                                  "NULL argument passed [epgManager = %p, pc = %p, pcLogic = %p, langPreferences = %p]",
                                  epgManager, pc, pcLogic, langPreferences);
        goto out;
    }

    self = (EventsLogic) SvTypeAllocateInstance(EventsLogic_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain,
                                           SvCoreError_noMemory,
                                           error,
                                           "unable to allocate EventsLogic instance");
        goto out;
    }

    self->epgManager = SVRETAIN(epgManager);
    self->pc = SVRETAIN(pc);
    self->pcLogic = SVRETAIN(pcLogic);
    self->langPreferences = SVRETAIN(langPreferences);

out:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

SvLocal bool
EventsLogicDescShouldBeBlocked(EventsLogic self, SvEPGEvent event)
{
    if (QBParentalControlEventDescShouldBeBlocked(self->pc, event)) {
        return !QBParentalControlLogicIsAuthenticated(self->pcLogic);
    }

    return false;
}

bool EventsLogicGetDescFromEvent(EventsLogic self, SvEPGEvent event, SvEPGEventDesc* desc)
{
    if (!self || !event) {
        return false;
    }

    if (EventsLogicDescShouldBeBlocked(self, event)) {
        *desc = QBParentalControlGetBlockedDescription(self->pc);
    } else {
        *desc = QBLangPreferencesGetDescFromEvent(self->langPreferences, event);
    }

    return true;
}

SvString EventsLogicPCRatingToString(EventsLogic self, SvString ratingStr)
{
    if (!self || !ratingStr)
        return NULL;

    if (SvStringGetLength(ratingStr) == 0)
        return NULL;

    SvString descriptionString = NULL;
    SvErrorInfo error = NULL;
    QBPCList rating = QBPCListCreate(&error);
    if (error)
        goto err;
    QBPCListLockRatingsFromString(rating, ratingStr, &error);
    if (error) {
        SVRELEASE(rating);
        goto err;
    }
    descriptionString = QBParentalControlLogicCreateDescriptionStringFromPCList(self->pcLogic, rating, &error);
    SVRELEASE(rating);
    if (error)
        goto err;
    return descriptionString;
err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return NULL;
}

bool EventsLogicGetSearchableDescFromEvent(EventsLogic self, SvEPGEvent event, SvEPGEventDesc* desc)
{
    if (!self || !event) {
        return false;
    }

    if (EventsLogicDescShouldBeBlocked(self, event)) {
        *desc = NULL;
    } else {
        *desc = QBLangPreferencesGetDescFromEvent(self->langPreferences, event);
    }

    return true;
}

bool EventsLogicGetTitleFromEvent(EventsLogic self, SvEPGEvent event, SvString* title)
{
    return false;
}

bool EventsLogicCreateNotRatedEventRating(EventsLogic self, SvTVChannel channel, QBPCList* event)
{
    if (!event) {
        return false;
    }

    if (!self->notRatedRating) {
        self->notRatedRating = QBPCListCreate(NULL);
        if (!self->notRatedRating) {
            return false;
        }
    }

    *event = SVRETAIN(self->notRatedRating);

    return true;
}

bool EventsLogicIsEventFeatureAllowed(EventsLogic self, SvEPGEvent event, EventsLogicAllowedEventFeatureType featureType)
{
    return true;
}
