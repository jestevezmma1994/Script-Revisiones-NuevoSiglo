/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/SvEPGDataLayer/SvEPGExtendedChannelListListener.h */

#ifndef SV_EPG_EXTENDED_CHANNEL_LIST_LISTENER_H
#define SV_EPG_EXTENDED_CHANNEL_LIST_LISTENER_H

/**
 * @file SvEPGExtendedChannelListListener.h
 * @brief Channel list litener interface extending SvEPGChannelListListener with
 *        additional 'channelReplaced' method.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>

/**
 * @defgroup SvEPGExtendedChannelListListener Extended channel list listener interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * Get runtime type identification object representing
 * Extended channel list listener interface.
 **/
extern SvInterface
SvEPGExtendedChannelListListener_getInterface(void);


/**
 * SvEPGExtendedChannelListListener interface.
 **/
typedef const struct SvEPGExtendedChannelListListener_ {
    struct SvEPGChannelListListener_ super_;

    /**
     * Notify that channel has been replaced changed.
     *
     * @param[in] self_         handle to an object implementing @ref SvEPGExtendedChannelListListener
     * @param[in] oldChannel    handle to the old SvTVChannel object
     * @param[in] newChannel    handle to the new SvTVChannel object
     **/
    void (*channelReplaced)(SvGenericObject self_,
                            SvTVChannel oldChannel,
                            SvTVChannel newChannel);
} *SvEPGExtendedChannelListListener;

/**
 * @}
 **/

#endif // SV_EPG_EXTENDED_CHANNEL_LIST_LISTENER_H
