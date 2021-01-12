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

/* App/Libraries/SvEPGDataLayer/SvEPGChannelView.h */

#ifndef SV_EPG_CHANNEL_VIEW_H_
#define SV_EPG_CHANNEL_VIEW_H_

/**
 * @file SvEPGChannelView.h
 * @brief EPG Channel View Interface.
 **/

/**
 * @defgroup SvEPGChannelView EPG channel view interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * Channel number rounding method.
 *
 * If a channel cannot be found via ::getChannelByNumber(), this
 * method specifies the way another channel may be used instead.
 **/
typedef enum {
    SvEPGViewNumberRoundingMethod_exact = 0,
    SvEPGViewNumberRoundingMethod_closest,
    SvEPGViewNumberRoundingMethod_next,
} SvEPGViewNumberRoundingMethod;


/**
 * Get runtime type identification object representing
 * EPG channel filter interface.
 **/
extern SvInterface
SvEPGChannelView_getInterface(void);


/**
 * SvEPGChannelView interface.
 **/
typedef const struct SvEPGChannelView_ {
    /**
     * Get number of channels.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @return              number of channels
     **/
    size_t (*getCount)(SvGenericObject self_);

    /**
     * Get number of channels for specific plugin.
     *
     * This method filters channels on basis of source (for example: DVB or IP) expressed by appropriate plugin ID.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] pluginID  ID of a source plugin
     * @return              number of channels
     **/
    size_t (*getCountForPluginID)(SvGenericObject self_, unsigned int pluginID);

    /**
     * Find EPG ID of the channel available at given index.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] index     channel index
     * @return              EPG ID of the channel at @a index,
     *                      @c NULL if index is out of valid range
     **/
    SvValue (*getIDByIndex)(SvGenericObject self_,
                            unsigned int index);

    /**
     * Find TV channel available at given index.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] index     channel index
     * @return              channel at @a index,
     *                      @c NULL if index is out of valid range
     **/
    SvTVChannel (*getByIndex)(SvGenericObject self_,
                              unsigned int index);

    /**
     * Find TV channel with given ID.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] ID        unique channel ID
     * @return              channel with @a ID, @c NULL if not found
     **/
    SvTVChannel (*getByID)(SvGenericObject self_,
                           SvValue ID);
    /**
     * Find TV channel with given number
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] index     channel number
     * @return              channel at @a index,
     *                      @c NULL if index is out of valid range
     **/
    SvTVChannel (*getByNumber)(SvGenericObject self_,
                               int index,
                               SvEPGViewNumberRoundingMethod numberingMethod);

    /**
     * Find index of a TV channel with given EPG ID.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] channel   TV channel handle
     * @return              index of a @a channel, < @c 0 if not found
     **/
    int (*getChannelIndex)(SvGenericObject self_,
                           SvTVChannel channel);

    /**
     * Find number of a TV channel.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] channel   TV channel handle
     * @return              number of channel @a channel, @c -1 if not found
     **/
    int (*getChannelNumber)(SvGenericObject self_,
                            SvTVChannel channel);

    /**
     * Add channel list listener.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] listener  channel list listener handle
     **/
    void (*addListener)(SvGenericObject self_,
                        SvObject listener);

    /**
     * Remove channel list listener.
     *
     * @param[in] self_     handle to an object implementing @ref SvEPGChannelView
     * @param[in] listener  channel list listener handle
     **/
    void (*removeListener)(SvGenericObject self_,
                           SvObject listener);
} *SvEPGChannelView;

/**
 * @}
 **/


#endif
