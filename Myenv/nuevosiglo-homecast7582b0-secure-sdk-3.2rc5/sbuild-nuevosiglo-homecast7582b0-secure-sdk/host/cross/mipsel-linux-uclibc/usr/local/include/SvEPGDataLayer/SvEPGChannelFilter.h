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

/* App/Libraries/SvEPGDataLayer/SvEPGChannelFilter.h */

#ifndef SV_EPG_CHANNEL_FILTER_H_
#define SV_EPG_CHANNEL_FILTER_H_

/**
 * @file SvEPGChannelFilter.h
 * @brief EPG Channel Filter Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGChannelFilter EPG channel filter interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * Get runtime type identification object representing
 * EPG channel filter interface.
 **/
extern SvInterface
SvEPGChannelFilter_getInterface(void);


/**
 * SvEPGChannelFilter interface.
 **/
typedef const struct SvEPGChannelFilter_ {
    /**
     * Check if we are interested in a given channel.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelFilter
     * @param[in] channel   handle to a TV channel
     * @return              @c false to ignore this channel
     **/
    bool (*test)(SvGenericObject self_,
                 SvTVChannel channel);

    /**
     * Check how two channels should be ordered.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelFilter
     * @param[in] channelA  first channel to compare
     * @param[in] channelB  second channel to compare
     * @return              @c 0 if channelA == channelB,
     *                      less than @c 0 if channelA should be shown
     *                      before channelB, more than @c 0 otherwise
     **/
    int (*compare)(SvGenericObject self_,
                   SvTVChannel channelA, int numA, SvString nameA,
                   SvTVChannel channelB, int numB, SvString nameB);
} *SvEPGChannelFilter;

/**
 * Check if we are interested in a given channel.
 *
 * @param[in] self      handle to an object implementing @ref SvEPGChannelFilter
 * @param[in] channel   handle to a TV channel
 * @return              @c false to ignore this channel
 **/
static inline bool SvEPGChannelFilterTest(SvObject self, SvTVChannel channel)
{
    return SvInvokeInterface(SvEPGChannelFilter, self, test, channel);
}

/**
 * Check how two channels should be ordered.
 *
 * @param[in] self      handle to an object implementing @ref SvEPGChannelFilter
 * @param[in] channelA  first channel to compare
 * @param[in] numA      first channel number
 * @param[in] nameA     first channel name
 * @param[in] channelB  second channel to compare
 * @param[in] numB      second channel number
 * @param[in] nameB     second channel name
 * @return              @c 0 if channelA == channelB,
 *                      less than @c 0 if channelA should be shown
 *                      before channelB, more than @c 0 otherwise
 **/
static inline int SvEPGChannelFilterCompare(SvObject self,
                                            SvTVChannel channelA, int numA, SvString nameA,
                                            SvTVChannel channelB, int numB, SvString nameB)
{
    return SvInvokeInterface(SvEPGChannelFilter, self, compare, channelA, numA, nameA, channelB, numB, nameB);
}

/**
 * @}
 **/


#endif
