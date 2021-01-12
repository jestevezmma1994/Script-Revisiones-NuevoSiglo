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

#ifndef TRAXIS_EPG_EVENT_H_
#define TRAXIS_EPG_EVENT_H_

/**
 * @file TraxisEPGEvent.h Traxis.Web EPG event class API
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <time.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvPlayerKit/SvEPGEvent.h>


/**
 * @defgroup TraxisEPGEvent Traxis.Web EPG event class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis EPG event class.
 * @class TraxisEPGEvent TraxisEPGEvent.h <TraxisWebClient/TraxisEPGEvent.h>
 * @extends SvEPGEvent
 **/
typedef struct TraxisEPGEvent_ *TraxisEPGEvent;

/**
 * Get runtime type identification object
 * representing Traxis EPG Event class.
 * @return runtime type identification object
 **/
extern SvType
TraxisEPGEvent_getType(void);

/**
 * Create and initialize Traxis EPG event instance.
 *
 * @param[in] channelID channel EPG ID
 * @param[in] startTime start time
 * @param[in] endTime   end time
 * @param[in] fullID    event full ID
 * @param[out] errorOut error info
 * @return              newly created EPG event object or @c NULL in case of error
 **/
extern TraxisEPGEvent
TraxisEPGEventCreate(SvValue channelID,
                     time_t startTime,
                     time_t endTime,
                     SvString fullID,
                     SvErrorInfo *errorOut);

/**
 * Get ID of the product object representing TV channel this EPG event is on.
 *
 * @param[in] self Traxis EPG event handle
 * @return         channel product identifier, @c NULL if not available
 **/
extern SvString
TraxisEPGEventGetChannelProductID(TraxisEPGEvent self);

/**
 * @}
 **/

#endif
