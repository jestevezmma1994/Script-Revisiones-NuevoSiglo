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

#ifndef SV_EPG_CHANNEL_LIST_H_
#define SV_EPG_CHANNEL_LIST_H_

/**
 * @file SvEPGChannelList.h
 * @brief EPG channel list class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvSet.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup SvEPGChannelList EPG channel list class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * EPG channel list class.
 **/
typedef struct SvEPGChannelList_ *SvEPGChannelList;


/**
 * Get runtime type identification object
 * representing SvEPGChannelList class.
 **/
extern SvType
SvEPGChannelList_getType(void);

/**
 * Get number of TV channels on a list.
 *
 * @param[in] self      EPG channel list handle
 * @return              number of channels or @c -1 in case of error
 **/
extern ssize_t
SvEPGChannelListGetCount(SvEPGChannelList self);

/**
 * Find TV channel with given unique ID.
 *
 * @param[in] self      EPG channel list handle
 * @param[in] ID        unique channel ID
 * @return              TV channel with given @a ID or @c NULL if not found
 **/
extern SvTVChannel
SvEPGChannelListGetChannelByID(SvEPGChannelList self,
                               SvValue ID);

/**
 * Find TV channel with given unique original ID.
 *
 * @param[in] self      EPG channel list handle
 * @param[in] ID        unique channel original ID
 * @return              TV channel with given @a ID or @c NULL if not found
 **/
extern SvTVChannel
SvEPGChannelListGetChannelByOriginalID(SvEPGChannelList self,
                                       SvValue ID);

/**
 * Find TV channels with given name.
 *
 * @param[in] self      EPG channel lsit handle
 * @param[in] name      channel name
 * @param[in] channels  array to add matching channels to
 * @param[out] errorOut error info
 * @return              number of matching channels found,
 *                      @c -1 in case of error
 **/
extern ssize_t
SvEPGChannelListGetChannelsByName(SvEPGChannelList self,
                                  SvString name,
                                  SvArray channels,
                                  SvErrorInfo *errorOut);

/**
 * Check if the EPG channel list is active
 * Returns true iff the are any scheduled channel changes propagations.
 * @param[in] self      EPG channel list handle
 * @return              true iff there are any pending channel (list) change requests
 */
extern bool
SvEPGChannelListIsPropagatingChannels(SvEPGChannelList self);


/**
 * Find all adult TV channels.
 *
 * @param[in] self      EPG channel lsit handle
 * @param[in] channels  set to add matching channels to
 * @param[out] errorOut error info
 * @return              number of matching channels found,
 *                      @c -1 in case of error
 **/
extern ssize_t
SvEPGChannelListGetAdultChannels(SvEPGChannelList self,
                                 SvSet channels,
                                 SvErrorInfo *errorOut);

/**
 * Find all blocked TV channels.
 *
 * @param[in] self      EPG channel lsit handle
 * @param[in] attribute attribute to be checked
 * @param[in] channels  set to add matching channels to
 * @param[out] errorOut error info
 * @return              number of matching channels found,
 *                      @c -1 in case of error
 **/
extern ssize_t
SvEPGChannelListGetChannelsWithAttribute(SvEPGChannelList self,
                                         SvString attribute,
                                         SvSet channels,
                                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
