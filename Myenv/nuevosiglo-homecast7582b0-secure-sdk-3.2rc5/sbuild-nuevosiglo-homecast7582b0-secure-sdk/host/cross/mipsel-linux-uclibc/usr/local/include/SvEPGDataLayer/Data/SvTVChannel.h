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

/* App/Libraries/SvEPGDataLayer/Data/SvTVChannel.h */

#ifndef SV_TV_CHANNEL_H_
#define SV_TV_CHANNEL_H_

/**
 * @file SvTVChannel.h
 * @brief TV Channel Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvTVChannel TV channel class
 * @ingroup SvEPGData
 * @{
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>
#include <SvDataBucket2/SvDBObject.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvDataBucket2/SvDataBucket.h>


/**
 * TV channel source type.
 **/
typedef enum {
   SvTVChannelSourceType_unknown = 0,
   SvTVChannelSourceType_direct
} SvTVChannelSourceType;

/**
 * TV channel medium type.
 **/
typedef enum {
   SvTVChannelMediumType_unknown = 0,
   SvTVChannelMediumType_IP,
   SvTVChannelMediumType_DVB
} SvTVChannelMediumType;

/**
 * TV channel class.
 **/
typedef struct SvTVChannel_ {
   /// base class
   struct SvDBObject_ super_;
   /// hash value
   unsigned int hash;

   /// original id - in case of the external_channel_id presence
   SvValue originalId;

   /// unique identifier of the EPG plugin that created this channel
   unsigned int pluginID;

   /// channel number
   int number;
   /// channel name
   SvString name;
   /// source type
   SvTVChannelSourceType sourceType;
   /// source URL
   SvURL sourceURL;
   /// channel logo URL
   SvURL logoURL;

   /// channel EPG ID (optional)
   SvValue EPG_ID;

   /// DRM info (optional)
   SvDBRawObject DRM;

   /// Is currently not being emited (but may exist e.g. in favourites)
   bool isDead;

   bool needsAlternative;
   SvValue alternativeID;

   /// optional extended attributes
   SvHashTable metaData;
   /// flag informing about subscription
   bool isDisabled;

} *SvTVChannel;

/**
 * Get runtime type identification object
 * representing SvTVChannel class.
 **/
extern SvType
SvTVChannel_getType(void);

/**
 * Initialize SvTVChannel instance.
 *
 * @param[in] self      TV channel handle
 * @param[in] type      channel type ("TV", "Radio", "QAM")
 * @param[in] pluginID  EPG plugin's unique ID
 * @param[in] number    channel number (unique ID)
 * @param[in] extID     channel external ID (may be @c NULL)
 * @param[in] name      channel name
 * @param[in] EPG_ID    channel EPG ID (may be @c NULL)
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvTVChannel
SvTVChannelInit(SvTVChannel self,
                SvString type,
                unsigned int pluginID,
                unsigned int number,
                SvValue extID,
                SvString name,
                SvValue EPG_ID,
                SvErrorInfo *errorOut);

/**
 * Get channel name.
 *
 * @param[in] self      TV channel handle
 * @return              channel name
 **/
extern SvString
SvTVChannelGetName(SvTVChannel self);

/**
 * Get unique ID of the TV channel.
 *
 * @param[in] self      TV channel handle
 * @return              unique ID
 **/
extern SvValue
SvTVChannelGetID(SvTVChannel self);

/**
 * Get original ID of the TV channel.
 *
 * @param[in] self      TV channel handle
 * @return              original ID
 **/
extern SvValue
SvTVChannelGetOriginalID(SvTVChannel self);

/**
 * Get unique identifier of the EPG plugin that created this channel.
 *
 * @param[in] self      TV channel handle
 * @return              unique identifier of the EPG plugin
 **/
extern unsigned int
SvTVChannelGetPluginID(SvTVChannel self);

/**
 * Get medium type of the TV channel.
 *
 * @param[in] self      TV channel handle
 * @return              medium type
 **/
extern SvTVChannelMediumType
SvTVChannelGetMediumType(SvTVChannel self);

/**
 * Get extended attribute of the TV channel.
 *
 * @param[in] self      TV channel handle
 * @param[in] key       attribute key
 * @return              attribute value, @c NULL in case of error
 **/
extern SvGenericObject
SvTVChannelGetAttribute(SvTVChannel self,
                        SvString key);
/**
 * Set extended attribute of the TV channel.
 * @param[in] self      TV channel handle
 * @param[in] key       attribute key
 * @param[in] value     attribute value
 **/
extern void
SvTVChannelSetAttribute(SvTVChannel self,
                        SvString key,
                        SvGenericObject value);

/**
 * Set logo URL of the TV channel.
 * @param[in] self      TV channel handle
 * @param[in] url       logo URL
 **/
void
SvTVChannelSetLogoURL(SvTVChannel self,
                      SvURL url);

/**
 * Remove extended attribute of the TV channel.
 *
 * @param[in] self      TV channel handle
 * @param[in] key       attribute key
 **/
extern void
SvTVChannelRemoveAttribute(SvTVChannel self,
                           SvString key);

/**
 * Combine attributes of two channels, overwriting all attributes
 * of @a self with attributes from @a new.
 */
extern bool
SvTVChannelCombine(SvTVChannel self,
                   SvTVChannel new);


/**
 * Register TV channel merger.
 *
 * @param[in] db        data bucket handle
 **/
extern void
SvTVChannelRegisterMerger(SvDataBucket db);

/**
 * Register JSON serializers for TV channel class.
 **/
extern void
SvTVChannelRegisterJSONSerializers(void);

/**
 * @}
 **/

#endif
