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

/* App/Libraries/SvEPGDataLayer/SvEPGChannelListListener.h */

#ifndef SV_EPG_CHANNEL_LIST_LISTENER_H_
#define SV_EPG_CHANNEL_LIST_LISTENER_H_

/**
 * @file SvEPGChannelListListener.h
 * @brief EPG Channel List Listener Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup SvEPGChannelListListener EPG channel list listener interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * Get runtime type identification object representing
 * EPG channel list listener interface.
 **/
extern SvInterface
SvEPGChannelListListener_getInterface(void);


/**
 * SvEPGChannelListListener interface.
 **/
typedef const struct SvEPGChannelListListener_ {
   /**
    * Notify that new channel has been found.
    *
    * @param[in] self_     handle to an object implementing @ref SvEPGChannelListListener
    * @param[in] channel   handle to an SvTVChannel object representing
    *                      newly found channel
    **/
   void (*channelFound)(SvGenericObject self_,
                        SvTVChannel channel);

   /**
    * Notify that one of the channels is not available anymore.
    *
    * @param[in] self_     handle to an object implementing @ref SvEPGChannelListListener
    * @param[in] channel   handle to an SvTVChannel object representing
    *                      channel that has been lost
    **/
   void (*channelLost)(SvGenericObject self_,
                       SvTVChannel channel);

   /**
    * Notify that channel meta data had changed.
    *
    * @param[in] self_     handle to an object implementing @ref SvEPGChannelListListener
    * @param[in] channel   handle to the changed SvTVChannel object
    **/
   void (*channelModified)(SvGenericObject self_,
                           SvTVChannel channel);

   /**
    * Notify that channel list processing has been finished.
    *
    * @param[in] self_     handle to an object implementing @ref SvEPGChannelListListener
    * @param[in] pluginID  plugin's unique ID
    **/
   void (*channelListCompleted)(SvObject self_, int pluginID);
} *SvEPGChannelListListener;

// compatibility macros, remove when not needed anymore
#define SvEPGChannelListListener_t SvEPGChannelListListener_

/**
 * @}
 **/


#endif
