/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_CACHE_MONITOR_H_
#define SV_EPG_CACHE_MONITOR_H_

/**
 * @file SvEPGCacheMonitor.h
 * @brief EPG cache monitor interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGCacheMonitor EPG cache monitor interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * Get runtime type identification object representing
 * EPG cache monitor interface.
 **/
extern SvInterface
SvEPGCacheMonitor_getInterface(void);


/**
 * SvEPGCacheMonitor interface.
 **/
typedef const struct SvEPGCacheMonitor_ {
    /**
     * Notify that contents of EPG cache have changed.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGCacheMonitor
     * @param[in] channel   channel @a event belongs to
     * @param[in] addedEvents array of EPG events added to cache
     * @param[in] removedEvents array of EPG events removed from cache
     **/
    void (*contentsChanged)(SvGenericObject self_,
                            SvTVChannel channel,
                            SvImmutableArray addedEvents,
                            SvImmutableArray removedEvents);
} *SvEPGCacheMonitor;

/**
 * @}
 **/


#endif
