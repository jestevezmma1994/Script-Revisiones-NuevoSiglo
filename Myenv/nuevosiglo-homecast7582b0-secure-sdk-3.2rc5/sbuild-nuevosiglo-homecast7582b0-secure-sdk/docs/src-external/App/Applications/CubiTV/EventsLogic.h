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

#ifndef EVENTSLOGIC_H
#define EVENTSLOGIC_H

#include <Logic/QBParentalControlLogic.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/langPreferences.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBPCRatings/QBPCList.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>

#include <stdbool.h>

/**
 * @relates EventsLogic
 *
 * Feature type to be managed
 **/
typedef enum EventsLogicAllowedEventFeatureType_ {
    EventsLogicAllowedEventFeatureType_CUTV, /**< CatchUp TV event feature */
    EventsLogicAllowedEventFeatureType_PVR, /**< PVR event feature */
    EventsLogicAllowedEventFeatureType_NPVR /**< NPVR event feature */
} EventsLogicAllowedEventFeatureType;

/**
 * Client dependent EPG events logic
 *
 * @class EventsLogic EventsLogic.h <Logic/EventsLogic.h>
 * @extends SvObject
 *
 * This class provides operations on EPG events that may vary depending on client.
 **/
typedef struct EventsLogic_ *EventsLogic;

/**
 * Get runtime type identification object representing EventsLogic class.
 *
 * @relates EventsLogic
 *
 * @return EventsLogic runtime type identification object
 **/
SvType
EventsLogic_getType(void);

/**
 * Create new EventsLogic instance
 *
 * @public @memberof EventsLogic
 *
 * @param[in]  pc              parental control
 * @param[in]  pcLogic         parental control logic
 * @param[in]  langPreferences language preferences
 * @param[out] errorOut        error info
 * @return                     new EventsLogic instance or
 *                             @c NULL in case of error
 **/
EventsLogic
EventsLogicCreate(QBParentalControl pc,
                  QBParentalControlLogic pcLogic,
                  QBLangPreferences langPreferences,
                  SvErrorInfo *errorOut) __attribute__((weak));

bool EventsLogicGetDescFromEvent(EventsLogic self, SvEPGEvent event, SvEPGEventDesc* desc) __attribute__((weak));
bool EventsLogicGetSearchableDescFromEvent(EventsLogic self, SvEPGEvent event, SvEPGEventDesc* desc) __attribute__((weak));
bool EventsLogicGetTitleFromEvent(EventsLogic self, SvEPGEvent event, SvString* title) __attribute__((weak));
bool EventsLogicCreateNotRatedEventRating(EventsLogic self, SvTVChannel channel, QBPCList* outRating) __attribute__((weak));
/**
 * Check if a feature of an event is allowed
 * @param[in] self          EventsLogic handle
 * @param[in] event         Event for which a feature is to be blocked
 * @param[in] featureType   Type of feature to be blocked
 * @return                  True if feature is allowed, false if not
 */
bool EventsLogicIsEventFeatureAllowed(EventsLogic self, SvEPGEvent event, EventsLogicAllowedEventFeatureType featureType) __attribute__((weak));
SvString EventsLogicPCRatingToString(EventsLogic self, SvString rating) __attribute__((weak));

#endif // EVENTSLOGIC_H
