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

/* App/Libraries/SvEPGDataLayer/SvEPGDataListener.h */

#ifndef SV_EPG_DATA_LISTENER_H_
#define SV_EPG_DATA_LISTENER_H_

/**
 * @file SvEPGDataListener.h
 * @brief EPG Data Listener Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGDataListener EPG data listener interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvValue.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/SvEPGDataRequest.h>


/**
 * Get runtime type identification object representing
 * EPG data listener interface.
 **/
extern SvInterface
SvEPGDataListener_getInterface(void);


/**
 * SvEPGDataListener interface.
 **/
typedef const struct SvEPGDataListener_ {
    /**
     * Notify that EPG event data have been updated.
     *
     * This method is called by the EPG data window when new EPG event data
     * matching the data window is available.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGDataListener
     * @param[in] timeRange time range of new EPG event data
     * @param[in] channelID unique ID of the channel described by new events
     **/
    void (*dataUpdated)(SvGenericObject self_,
                        const SvTimeRange *timeRange,
                        SvValue channelID);
} *SvEPGDataListener;


/**
 * @}
 **/

#endif
