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

#ifndef SV_EPG_EVENT_H_
#define SV_EPG_EVENT_H_

/**
 * @file SvEPGEvent.h
 * @brief EPG event class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvURL.h>

#include "SvContentMetaData.h"
#include <SvPlayerKit/SvTimeRange.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvEPGEvent EPG event class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * EPG event description class.
 * @class SvEPGEventDesc
 * @extends SvObject
 **/
typedef struct SvEPGEventDesc_ {
    struct SvObject_ super_;    ///< super class
    SvString title;             ///< event title
    SvString shortDescription;  ///< short synopsis of the event
    SvString description;       ///< long decription
} *SvEPGEventDesc;

/**
 * Get runtime type identification object representing EPG event description class.
 *
 * @return EPG event description class
 **/
extern SvType
SvEPGEventDesc_getType(void);

/**
 * Create new EPG event description.
 *
 * @memberof SvEPGEventDesc
 *
 * @param[in] title     event title (optional, can be @c NULL)
 * @param[in] shortDescription short event title (optional, can be @c NULL)
 * @param[in] description full event description (optional, can be @c NULL)
 * @return              created EPG event, @c NULL in case of error
 **/
extern SvEPGEventDesc
SvEPGEventDescCreate(SvString title,
                     SvString shortDescription,
                     SvString description);

/**
 * EPG event class.
 * @class SvEPGEvent
 * @extends SvObject
 **/
typedef struct SvEPGEvent_ {
    struct SvObject_ super_;    ///< super class
    unsigned int hash;          ///< hash value
    int id;                     ///< DVB-EIT event ID, @c -1 if unknown
    SvValue channelID;          ///< channel EPG ID
    time_t startTime;           ///< start time
    time_t endTime;             ///< end time
    unsigned short category;    ///< content category
    unsigned short subcategory; ///< content subcategory
    size_t descriptionsSize;    ///< size of array descriptions
                                /**
                                 * parental control ratings serialized to string representation,
                                 * see SvPCRatingLevelCreateFromCStr().
                                 * don't edit this value directly - use SvEPGEventSetPCRating()
                                 **/
    SvString PC;
    struct {
        char langCode[4];       ///< language code in ISO639-3
        SvEPGEventDesc desc;    ///< can't be NULL
    } *descriptions;            /**<
                                 *    array (!sic) of descriptions - we don't use SvArray or SvHashTable to reduce memory usage
                                 *   don't edit this structure directly - use SvEPGEventGetDescription(), SvEPGEventSetDescription() or SvEPGEventGetAnyDescription()
                                 **/
} *SvEPGEvent;

/**
 * SvEPGEvent virtual methods table.
 **/
typedef const struct SvEPGEventVTable_ {
    ///< virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * serialize EPG event to metadata
     *
     * @param[in] self     EPG event handle
     * @param[out] metaData serialized EPG event handle
     * @return              true on success, false otherwise
     **/
    bool (*serializeToMeta)(SvEPGEvent self, SvContentMetaData metaData);

    /**
     * Initialize EPG event from metadata
     *
     * @param[out] self      deserialized EPG event handle
     * @param[in]  metaData    metadata handle
     * @return               true on success, false otherwise
     **/
    bool (*initFromMeta)(SvEPGEvent self, SvContentMetaData metaData);

    /**
     * Check if @a self EPG event's ID is set.
     *
     * @param[in] self      EPG event handle
     * @return              @c true if event's ID is set, @c false otherwise
     **/
    bool (*hasID)(SvEPGEvent self);

    /**
     * Create identifier of the EPG event.
     *
     * @note It might seem strange why we use @a create() instead of @a get(),
     *       but the actual event class might keep the ID in other form than
     *       SvValue instance (for example: as an ordinary @a int value.)
     *
     * @qb_allocator
     *
     * @param[in] self      EPG event handle
     * @return              created identifier
     **/
    SvValue (*createID)(SvEPGEvent self);

    /**
     * Create string identifier of the EPG event.
     *
     * @qb_allocator
     *
     * @param[in] self      EPG event handle
     * @return              created identifier
     **/
    SvString (*createStringID)(SvEPGEvent self);

    /**
     * Set full ID for EPG event.
     *
     * @param[in]  self     EPG event handle
     * @param[in]  id       full ID
     * @param[out] errorOut error message
     **/
    void (*setID)(SvEPGEvent self, SvValue id, SvErrorInfo *errorOut);

    /**
     * Check if @a other EPG event has the same ID.
     *
     * @note This method exists as an optimized way to compare two events
     *       by their IDs. The same effect can be achieved by creating IDs
     *       with the other method and comparing them directly.
     *
     * @param[in] self      EPG event handle
     * @param[in] other     other EPG event handle to compare with @a self
     * @return              @c true if events have the same ID, @c false otherwise
     **/
    bool (*hasEqualID)(SvEPGEvent self, SvEPGEvent other);

    /**
     * Return series ID of the EPG event.
     * Some events which come from operator may store series ID.
     * Unique name allows to identify related events, for example:
     * a TV series (@see SvEPGEventVTable::getEpisodeID())
     *
     * @param[in] self    EPG event handle
     * @return            series ID or @c NULL
     **/
    SvString (*getSeriesID)(SvEPGEvent self);

    /**
     * Set series ID for EPG event.
     *
     * @param[in] self      EPG event handle
     * @param[in] seriesID   series ID, @c NULL to remove previous value
     **/
    void (*setSeriesID)(SvEPGEvent self, SvString seriesID);

    /**
     * Return episode ID of the EPG event.
     * Some events which comes from operator may store episod ID.
     * Unique name allows to identify related events, for example:
     * a TV series have episodes (@see SvEPGEventVTable::getSeriesID())
     *
     * @param[in] self   EPG event handle
     * @return           episode ID or @c NULL
     **/
    SvString (*getEpisodeID)(SvEPGEvent self);

    /**
     * Set episode ID for EPG event.
     *
     * @param[in] self      EPG event handle
     * @param[in] episodeID  episode ID, @c NULL to remove previous value
     **/
    void (*setEpisodeID)(SvEPGEvent self, SvString episodeID);

    /**
     * Check if EPG event can be remotely recorded.
     * Operator may offer network PVR functionality. Some events
     * can be recorded by the operator and available for watching
     * later on the STB.
     *
     * @param[in] self   EPG event handle
     * @return           @c true if EPG event might be recorded on the operator
     *                   side or @c false otherwise
     **/
    bool (*canBeRemotelyRecorded)(SvEPGEvent self);

    /**
     * Set EPG event can be remotely recorded.
     *
     * @param[in] self         EPG event handle
     * @param[in] canBeRecorded @c true if EPG event can be recorded on the middleware, otherwise @c false
     **/
    void (*setCanBeRemotelyRecorded)(SvEPGEvent self, bool canBeRecorded);

    /**
     * Check if EPG event can be remotely start over (started from the beginning).
     * Operator may offer network PVR functionality. If EPG event was recorded
     * by operator you may check if this event might be started from the beginning.
     *
     * @param[in] self    EPG event handle
     * @return            @c true if EPG event offered by network PVR operator might be started from the beginning,
     *                    @c false otherwise
     **/
    bool (*canBeRemotelyStartedOver)(SvEPGEvent self);

    /**
    * Set EPG event can be remotely start over - can play event from the beginning.
    *
    * @param[in] self         EPG event handle
    * @param[in] startOver     @c true if EPG event allow start over on the middleware, otherwise @c false
    */
    void (*setCanBeRemotelyStartedOver)(SvEPGEvent self, bool startOver);

    /**
    * Get EPG event is catch-up enabled flag - can start catch-up service.
    *
    * @param[in] self         EPG event handle
    * @return              @c true if EPG event allow start catch-up on the middleware, otherwise @c false
    */
    bool (*isCatchupEnabled)(SvEPGEvent self);

    /**
    * Set EPG event is catch-up enabled flag - can start catch-up service
    *
    * @param[in] self         EPG event handle
    * @param[in] catchup       @c true if EPG event allow start catch-up on the middleware, otherwise @c false
    */
    void (*setCatchupEnabled)(SvEPGEvent self, bool catchup);

    /**
    * Get EPG event is premiere enabled flag - EPG event is marked as a premiere.
    *
    * @param[in] self         EPG event handle
    * @return              @c true if EPG event is marked as a premiere on the middleware, otherwise @c false
    */
    bool (*isPremiere)(SvEPGEvent self);

    /**
    * Set EPG event is premiere enabled flag - EPG event is marked as a premiere.
    *
    * @param[in] self         EPG event handle
    * @param[in] premiere      @c true if EPG event is marked as a premiere on the middleware, otherwise @c false
    */
    void (*setPremiere)(SvEPGEvent self, bool premiere);

    /**
    * Get poster URL of the EPG event.
    * @param[in] self      EPG event handle
    * @return              poster URL of the EPG
    **/
    SvURL (*getPosterURL)(SvEPGEvent self);

    /**
    * Set poster URL of the EPG event.
    * @param[in] self      EPG event handle
    * @param[in] posterURL poster URL of the EPG
    **/
    void (*setPosterURL)(SvEPGEvent self, SvURL posterURL);

    /**
    * Get EPG event collection type (e.g "tvShow", "series", "movie").
    *
    * @param[in] self      EPG event handle
    * @return              collection type
    **/
    SvString (*getCollectionType)(SvEPGEvent self);

    /**
    * Set EPG event collection type.
    *
    * @param[in] self           EPG event handle
    * @param[in] collectionType  collection type
    **/
    void (*setCollectionType)(SvEPGEvent self, SvString collectionType);
} *SvEPGEventVTable;

/**
 * Serialize EPG event to content metadata representation.
 *
 * @memberof SvEPGEvent
 * @qb_allocator
 *
 * @param[in] self      EPG event handle
 * @return              created metadata content, @c NULL in case of error
 **/
extern SvContentMetaData
SvEPGEventSerializeToMeta(SvEPGEvent self);

/**
 * Create EPG event from a serialized representation as content metadata.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] serial  metadata handle
 * @return            created EPG event or @c NULL in case of error
 **/
extern SvEPGEvent
SvEPGEventCreateFromMeta(SvContentMetaData serial);

/// @copydoc SvEPGEventVTable::createID()
static inline SvValue
SvEPGEventCreateID(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, createID);
}

/// @copydoc SvEPGEventVTable::hasID()
static inline bool
SvEPGEventHasID(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, hasID);
}

/// @copydoc SvEPGEventVTable::setID()
static inline void
SvEPGEventSetID(SvEPGEvent self, SvValue id, SvErrorInfo *errorOut)
{
    SvInvokeVirtual(SvEPGEvent, self, setID, id, errorOut);
}

/// @copydoc SvEPGEventVTable::hasEqualID()
static inline bool
SvEPGEventHasEqualID(SvEPGEvent self,
                     SvEPGEvent other)
{
    return SvInvokeVirtual(SvEPGEvent, self, hasEqualID, other);
}

/// @copydoc SvEPGEventVTable::getSeriesID()
static inline SvString
SvEPGEventGetSeriesID(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, getSeriesID);
}

/// @copydoc SvEPGEventVTable::setSeriesID()
static inline void
SvEPGEventSetSeriesID(SvEPGEvent self, SvString seriesID)
{
    SvInvokeVirtual(SvEPGEvent, self, setSeriesID, seriesID);
}

/// @copydoc SvEPGEventVTable::getEpisodeID()
static inline SvString
SvEPGEventGetEpisodeID(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, getEpisodeID);
}

/// @copydoc SvEPGEventVTable::setEpisodeID()
static inline void
SvEPGEventSetEpisodeID(SvEPGEvent self, SvString episodeID)
{
    SvInvokeVirtual(SvEPGEvent, self, setEpisodeID, episodeID);
}

/// @copydoc SvEPGEventVTable::canBeRemotelyRecorded()
static inline bool
SvEPGEventCanBeRemotelyRecorded(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, canBeRemotelyRecorded);
}

/// @copydoc SvEPGEventVTable::setCanBeRemotelyRecorded()
static inline void
SvEPGEventSetCanBeRemotelyRecorded(SvEPGEvent self, bool canBeRecorded)
{
    SvInvokeVirtual(SvEPGEvent, self, setCanBeRemotelyRecorded, canBeRecorded);
}

/// @copydoc SvEPGEventVTable::canBeRemotelyStartedOver()
static inline bool
SvEPGEventCanBeRemotelyStartedOver(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, canBeRemotelyStartedOver);
}

/// @copydoc SvEPGEventVTable::setCanBeRemotelyStartedOver()
static inline void
SvEPGEventSetCanBeRemotelyStartedOver(SvEPGEvent self, bool startOver)
{
    SvInvokeVirtual(SvEPGEvent, self, setCanBeRemotelyStartedOver, startOver);
}

/// @copydoc SvEPGEventVTable::isCatchupEnabled
static inline bool
SvEPGEventIsCatchupEnabled(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, isCatchupEnabled);
}

/// @copydoc SvEPGEventVTable::setCatchupEnabled
static inline void
SvEPGEventSetCatchupEnabled(SvEPGEvent self, bool catchup)
{
    SvInvokeVirtual(SvEPGEvent, self, setCatchupEnabled, catchup);
}

/// @copydoc SvEPGEventVTable::isPremiere
static inline bool
SvEPGEventIsPremiere(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, isPremiere);
}

/// @copydoc SvEPGEventVTable::setPremiere
static inline void
SvEPGEventSetPremiere(SvEPGEvent self, bool premiere)
{
    SvInvokeVirtual(SvEPGEvent, self, setPremiere, premiere);
}

/// @copydoc SvEPGEventVTable::getPosterURL()
static inline SvURL
SvEPGEventGetPosterURL(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, getPosterURL);
}

/// @copydoc SvEPGEventVTable::setPosterURL()
static inline void
SvEPGEventSetPosterURL(SvEPGEvent self, SvURL posterURL)
{
    return SvInvokeVirtual(SvEPGEvent, self, setPosterURL, posterURL);
}

/// @copydoc SvEPGEventVTable::getCollectionType()
static inline SvString
SvEPGEventGetCollectionType(SvEPGEvent self)
{
    return SvInvokeVirtual(SvEPGEvent, self, getCollectionType);
}

/// @copydoc SvEPGEventVTable::setCollectionType()
static inline void
SvEPGEventSetCollectionType(SvEPGEvent self, SvString collectionType)
{
    return SvInvokeVirtual(SvEPGEvent, self, setCollectionType, collectionType);
}

/**
 * Get runtime type identification object representing EPG event class.
 * @qb_allocator
 * @return EPG event class
 **/
extern SvType
SvEPGEvent_getType(void);

/**
 * Initialize EPG event instance.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] self      EPG event handle
 * @param[in] channelID channel EPG ID
 * @param[in] startTime start time
 * @param[in] endTime   end time
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvEPGEvent
SvEPGEventInit(SvEPGEvent self,
               SvValue channelID,
               time_t startTime, time_t endTime,
               SvErrorInfo *errorOut);

/**
 * create and initialize EPG event instance.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] channelID channel EPG ID
 * @param[in] startTime start time
 * @param[in] endTime   end time
 * @param[out] errorOut error info
 * @return               newly created EPG event object or @c NULL in case of error
 **/
extern SvEPGEvent
SvEPGEventCreate(SvValue channelID,
                 time_t startTime, time_t endTime,
                 SvErrorInfo *errorOut);

/**
 * Set parentral control of the EPG vent.
 * @param[in] self      EPG event handle
 * @param[in] PC        parental control rating
 **/
extern void
SvEPGEventSetPCRating(SvEPGEvent self,
                      SvString PC);

/**
 * Set description for EPG event for specified language.
 *
 * @memberof SvEPGEvent
 * @note If description was set for this language, old values is released.
 *
 * @param[in] self      EPG event handle
 * @param[in] langCode  name of language in ISO 639-3
 * @param[in] desc      description in specified language (can't be NULL)
 **/
extern void
SvEPGEventSetDescription(SvEPGEvent self,
                         SvString langCode,
                         SvEPGEventDesc desc);

/**
 * Get description for EPG event for specified language.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] self      EPG event handle
 * @param[in] langCode  name of language
 * @return              event description if found, otherwise @c NULL
 **/
extern SvEPGEventDesc
SvEPGEventGetDescription(SvEPGEvent self,
                         SvString langCode);

/**
 * Get any description for EPG event.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] self      EPG event handle
 * @return              the first description or @c NULL if EPG event
 *                      doesn't have any description
 **/
extern SvEPGEventDesc
SvEPGEventGetAnyDescription(SvEPGEvent self);

/**
* Get EPG event's category name.
*
 * @memberof SvEPGEvent
 *
* @param[in] self       EPG event handle
* @return               category name, @c NULL if unknown
*/
extern const char *
SvEPGEventGetCategoryName(SvEPGEvent self);

/**
* Get EPG event's length.
*
 * @memberof SvEPGEvent
 *
* @param[in] self       EPG event handle
* @return               length, @c NULL if unknown
*/
extern time_t
SvEPGEventGetLength(SvEPGEvent self);

/**
 * Get EPG event time range.
 *
 * @memberof SvEPGEvent
 *
 * @param[in] self       EPG event handle
 * @param[out] timeRange time range
 * @param[out] errorOut  error info
 **/
void SvEPGEventGetTimeRange(SvEPGEvent self, SvTimeRange *timeRange, SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
