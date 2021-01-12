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

#ifndef INNOV8ON_EPG_EVENT_H_
#define INNOV8ON_EPG_EVENT_H_

/**
 * @file Innov8onEPGEvent.h
 * @brief Innov8on EPG event class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


#include <time.h>
#include <stdbool.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGCatchup.h>

/**
 * @defgroup Innov8onEPGEvent EPG event class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * Innov8on EPG event class.
 * @class Innov8onEPGEvent
 * @extends SvEPGEvent
 **/
typedef struct Innov8onEPGEvent_ *Innov8onEPGEvent;

/**
 * flags determining EPG event settings
 **/
enum Innov8onEPGEventFlags {
    Innov8onEPGEventFlags_canBeRecorded = 1 << 0, ///< determine if event can be recorded on the middleware side
    Innov8onEPGEventFlags_startOver     = 1 << 1, ///< determine if is possible to start over (play event from the beginning)
    Innov8onEPGEventFlags_startCatchup  = 1 << 2, ///< determine if is possible to start catch-up (play past event)
    Innov8onEPGEventFlags_premiere      = 1 << 3  ///< determine if EPG event is marked as a premiere (first showing)
};

/**
 * Innov8onEPGEvent class internals.
 **/
struct Innov8onEPGEvent_ {
    struct SvEPGEvent_ super_;  ///< super class
    Innov8onEPGCatchup catchup; ///< catchup
    SvString fullID;            ///< alphanumeric identifier of event
    SvString seriesID;          ///< identifier that has the same value for related events, for example: episodes of a TV series
    SvString episodeID;         ///< identifier that has unique value for each episode
    uint8_t flags;              ///< determine a set of boolean properties enclosed in #Innov8onEPGEventFlags
};

/**
 * Get runtime type identification object representing Innov8on EPG event class.
 *
 * @return  EPG Innov8on EPG event class
 **/
extern SvType
Innov8onEPGEvent_getType(void);

/**
 * create and initialize Innov8on EPG event instance.
 *
 * @memberof Innov8onEPGEvent
 *
 * @param[in] channelID     channel EPG ID
 * @param[in] startTime     start time
 * @param[in] endTime       end time
 * @param[in] fullID        event full ID
 * @param[out] errorOut     error info
 * @return               newly created EPG event object or @c NULL in case of error
 **/
extern Innov8onEPGEvent
Innov8onEPGEventCreate(SvValue channelID,
                       time_t startTime,
                       time_t endTime,
             SvString fullID,
                       SvErrorInfo *errorOut);
/**
 * @}
 **/
#endif // INNOV8ON_EPG_EVENT_H_
