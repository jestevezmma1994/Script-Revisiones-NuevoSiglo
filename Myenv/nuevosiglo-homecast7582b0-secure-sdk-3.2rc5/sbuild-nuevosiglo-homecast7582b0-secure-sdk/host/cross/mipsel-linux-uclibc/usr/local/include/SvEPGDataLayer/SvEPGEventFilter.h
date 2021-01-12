/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* App/Libraries/SvEPGDataLayer/SvEPGEventFilter.h */

#ifndef SV_EPG_EVENT_FILTER_H_
#define SV_EPG_EVENT_FILTER_H_

/**
 * @file SvEPGEventFilter.h
 * @brief EPG Event Filter Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGEventFilter EPG event filter interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * Get runtime type identification object representing
 * EPG event filter interface.
 **/
extern SvInterface
SvEPGEventFilter_getInterface(void);


/**
 * SvEPGEventFilter interface.
 **/
typedef const struct SvEPGEventFilter_ {
    /**
     * Check if event should be shown or not.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGEventFilter
     * @param[in] channel   channel @a event belongs to
     * @param[in] event     EPG event to be checked
     * @return              @c true to show this event, @c false to hide
     **/
    bool (*test)(SvGenericObject self_,
                 SvTVChannel channel,
                 SvEPGEvent event);
} *SvEPGEventFilter;

/**
 * @}
 **/


#endif
