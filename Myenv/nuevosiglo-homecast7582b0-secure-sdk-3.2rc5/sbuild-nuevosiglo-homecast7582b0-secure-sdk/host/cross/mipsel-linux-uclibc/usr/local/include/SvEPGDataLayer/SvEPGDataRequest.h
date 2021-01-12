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

/* App/Libraries/SvEPGDataLayer/SvEPGDataRequest.h */

#ifndef SV_EPG_DATA_REQUEST_H_
#define SV_EPG_DATA_REQUEST_H_

/**
 * @file SvEPGDataRequest.h
 * @brief EPG Data Request Class API.
 **/

/**
 * @defgroup SvEPGDataRequest EPG data request class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <time.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvSet.h>
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvTimeRange.h>


/**
 * EPG data request status.
 **/
typedef enum {
   SvEPGDataRequestState_created = 0,
   SvEPGDataRequestState_ready,
   SvEPGDataRequestState_processing,
   SvEPGDataRequestState_finished,
   SvEPGDataRequestState_cancelled,
   SvEPGDataRequestState_failed
} SvEPGDataRequestState;

/**
 * EPG data request class.
 **/
typedef struct SvEPGDataRequest_ {
   /// super class
   struct SvObject_ super_;
   /// unique request ID
   SvValue ID;
   /// unique plugin ID
   unsigned int pluginID;
   /// request state
   SvEPGDataRequestState state;

   /// a set of channel IDs to fetch
   SvSet channelsSet;
   /// time range of EPG data to fetch
   SvTimeRange range;
} *SvEPGDataRequest;


/**
 * Get runtime type identification object
 * representing SvEPGDataRequest class.
 **/
extern SvType
SvEPGDataRequest_getType(void);

/**
 * Initialize SvEPGDataRequest instance.
 *
 * @param[in] self      EPG data request handle
 * @param[in] pluginID  unique EPG plugin ID
 * @param[in] timeRage  time range of EPG data to fetch
 * @param[in] channelsSet a set of channel IDs to fetch (pass NULL to use
 *                      an empty set, that can be filled later)
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern SvEPGDataRequest
SvEPGDataRequestInit(SvEPGDataRequest self,
                     unsigned int pluginID,
                     SvTimeRange timeRange,
                     SvSet channelsSet,
                     SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
