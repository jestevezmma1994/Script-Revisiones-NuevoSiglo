/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SV_CONTENT_META_DATA_H_
#define SV_CONTENT_META_DATA_H_

/**
 * @file SvContentMetaData.h Content metadata class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvContentMetaData Content metadata class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * Content metadata class.
 * @class SvContentMetaData
 * @extends SvHashTable
 **/
typedef SvHashTable SvContentMetaData;


/**
 * Get runtime type identification object representing content metadata class.
 *
 * @return content metadata class
 **/
extern SvType
SvContentMetaData_getType(void);

/**
 * Create new content metadata object.
 *
 * This method constructs an empty metadata object. This object can hold arbitrary amount
 * of key-value pairs of the following kinds:
 *  - strings,
 *  - booleans,
 *  - integers,
 *  - doubles,
 *  - any SvObject instances.
 *
 * @memberof SvContentMetaData
 *
 * @param[out] errorOut error info
 * @return created content metadata object, @c NULL in case of error
 **/
extern SvContentMetaData
SvContentMetaDataCreate(SvErrorInfo *errorOut);

/**
 * Get a string value for the given key.
 *
 * If the key is not set or is of a different type the value is not returned.
 * If the key is set the value is copied and returned.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[out] value value associated with @a name
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataGetStringProperty(SvContentMetaData self,
                                   SvString name,
                                   SvString *value);

/**
 * @cond
 **/
extern int
SvContentMetaDataGetStringProperty_c(SvContentMetaData self,
                                     const char *name,
                                     SvString *valueOut);
/**
 * @endcond
 **/

/**
 * Get an integer value for the given key.
 *
 * If the key is not set or is of a different type the value is not returned.
 * If the key is set the value is copied and returned.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[out] value value associated with @a name
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataGetIntegerProperty(SvContentMetaData self,
                                    SvString name,
                                    int *value);

/**
 * Get a double value for the given key.
 *
 * If the key is not set or is of a different type the value is not returned.
 * If the key is set the value is copied and returned.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[out] value value associated with @a name
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataGetDoubleProperty(SvContentMetaData self,
                                   SvString name,
                                   double *value);

/**
 * @cond
 **/
extern int
SvContentMetaDataGetDoubleProperty_c(SvContentMetaData self,
                                     const char *name,
                                     double *valueOut);
/**
 * @endcond
 **/

/**
 * Get a boolean value for the given key.
 *
 * If the key is not set or is of a different type the value is not returned.
 * If the key is set the value is copied and returned.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[out] value value associated with @a name
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataGetBooleanProperty(SvContentMetaData self,
                                    SvString name,
                                    bool *value);

/**
 * Get an object value for the given key.
 *
 * If the key is not set or is of a different type the value is not returned.
 * If the key is set the value is returned.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[out] value value associated with @a name
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataGetObjectProperty(SvContentMetaData self,
                                   SvString name,
                                   SvObject *value);

/**
 * Set a string value for the given key.
 *
 * The value and type of that might have been previously associated with this
 * key is discarded and replaced with the given item.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[in] value value to associate with this element
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataSetStringProperty(SvContentMetaData self,
                                   SvString name,
                                   SvString value);

/**
 * Set an integer value for the given key.
 *
 * The value and type of that might have been previously associated with this
 * key is discarded and replaced with the given item.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[in] value value to associate with this element
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataSetIntegerProperty(SvContentMetaData self,
                                    SvString name,
                                    int value);

/**
 * Set a double value for the given key.
 *
 * The value and type of that might have been previously associated with this
 * key is discarded and replaced with the given item.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[in] value value to associate with this element
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataSetDoubleProperty(SvContentMetaData self,
                                   SvString name,
                                   double value);

/**
 * Set a boolean value for the given key.
 *
 * The value and type of that might have been previously associated with this
 * key is discarded and replaced with the given item.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[in] value value to associate with this element
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataSetBooleanProperty(SvContentMetaData self,
                                    SvString name,
                                    bool value);

/**
 * Set an object value for the given key.
 *
 * The value and type of that might have been previously associated with this
 * key is discarded and replaced with the given item.
 *
 * Object given as value is retained.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @param[in] value value to associate with this element
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataSetObjectProperty(SvContentMetaData self,
                                   SvString name,
                                   SvObject value);

/**
 * Move metadata from the given content metadata object to @a self.
 *
 * Release the source structure on return.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self destination content metadata object handle
 * @param[in] source source content metadata object handle
 * @return @c 0 on success, @c -1 in case of error
 **/
extern int
SvContentMetaDataMove(SvContentMetaData self,
                      SvContentMetaData source);

/**
 * Delete an entry for the given key.
 *
 * @memberof SvContentMetaData
 *
 * @param[in] self content metadata handle
 * @param[in] name the name of this metadata element
 * @return @c 0 on success, @c -1 in case of erro
 **/
extern int
SvContentMetaDataDelete(SvContentMetaData self,
                        SvString name);


/**
 * @defgroup SvContentMetaData_well_known_keys Definitions of well known metadata keys
 * @{
 **/


/** Is the content being played from live dvb source (boolean, always set to true) */
#define SV_PLAYER_META__LIVE_DVB      "meta:live_dvb"


/** Metadata key used to access content title. */
#define SV_PLAYER_META__TITLE "com.sentivision.meta.title"

/** Metadata key used to access content author. */
#define SV_PLAYER_META__AUTHOR "com.sentivision.meta.author"

/** Metadata key used to access the abstract/description of the content. */
#define SV_PLAYER_META__DESCRIPTION "com.sentivision.meta.description"

/** Metadata key used to access the copyright information of a contnet. */
#define SV_PLAYER_META__COPYRIGHT "com.sentivision.meta.copyright"

/**
 * Metadata key used to access the name of the codec of the content.
 * This property is expressed as string and uses the SvString data type.
 **/
#define SV_PLAYER_META__VIDEOCODEC "com.sentivision.meta.codec.video"
#define SV_PLAYER_META__AUDIOCODEC "com.sentivision.meta.codec.audio"


#define SV_PLAYER_META__AUDIO_TRACKS "com.sentivision.meta.codec.audio_tracks"
#define SV_PLAYER_META__TEST_BITRATE "com.sentivision.meta.test.bitrate"

#define SV_PLAYER_META__PROGRAM_NUMBER  "qb.program-number"
#define SV_PLAYER_META__PMT_PID         "qb.pmt-pid"
#define SV_PLAYER_META__CHANNEL_ID      "qb.channel-id"

#define SV_PLAYER_AUDIO_CODEC_MPEG     "MPEG"
#define SV_PLAYER_AUDIO_CODEC_LPCM     "LPCM"
#define SV_PLAYER_AUDIO_CODEC_AC3      "AC3"

#define SV_PLAYER_AUDIO_CODEC_WMA1     "WMA1"
#define SV_PLAYER_AUDIO_CODEC_WMA2     "WMA2"
#define SV_PLAYER_AUDIO_CODEC_WMAPRO   "WMAPRO"
#define SV_PLAYER_AUDIO_CODEC_WMALSL   "WMALSL"

#define SV_PLAYER_AUDIO_CODEC_AACMAIN  "AACMAIN"
#define SV_PLAYER_AUDIO_CODEC_AACLC    "AACLC"
#define SV_PLAYER_AUDIO_CODEC_AACSSR   "AACSSR"
#define SV_PLAYER_AUDIO_CODEC_AACLTP   "AACLTP"

#define SV_PLAYER_AUDIO_CODEC_NONE        "NONE"
#define SV_PLAYER_AUDIO_CODEC_UNSUPPORTED "Unsupported"


#define SV_PLAYER_VIDEO_CODEC_H264     "H264"
#define SV_PLAYER_VIDEO_CODEC_MPG2     "MPG2"
#define SV_PLAYER_VIDEO_CODEC_MPG4     "MPG4"
#define SV_PLAYER_VIDEO_CODEC_WMV3     "WMV3"
#define SV_PLAYER_VIDEO_CODEC_VC10     "VC10"

#define SV_PLAYER_VIDEO_CODEC_NONE        "NONE"
#define SV_PLAYER_VIDEO_CODEC_UNSUPPORTED "Unsupported"

/**
 * @}
 **/

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
