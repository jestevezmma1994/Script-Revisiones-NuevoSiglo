/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CONT_HLS_KEY_PROVIDER_H
#define CONT_HLS_KEY_PROVIDER_H

/**
 * @file HlsKeyProvider.h hls key provier API file
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvURI.h>
#include <SvPlayerKit/SvDrmSystemInfo.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <stdbool.h>

#include "HlsMediaProvider.h"
#include "../cont_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HlsKeyProvider
 * @{
 **/

/**
 * First key with id == 0 is an empty key. Which is used for not encrypted data chunks.
 */
#define HLS_KEY_PROVIDER_EMPTY_KEY_ID    0
/**
 * @enum HlsEncMethod
 * @brief It indicates encryption method. It is used to define key's format.
 */
typedef enum {
    HlsEncMethod_NONE,   //!< No encryption method - it indicates on empty key.
    HlsEncMethod_AES_128,//!< AES key with size of 16 bytes is expected
    HlsEncMethod_UNKNOWN //!< Unsupported encryption method.
} HlsEncMethod;

/**
 * @enum HlsIVMode
 * @brief It indicates initialization vector mode.
 */
typedef enum {
    HlsIVMode_NONE,            //!< There is no IV
    HlsIVMode_VALUE,           //!< explicit value of IV is set
    HlsIVMode_SEQUENCE_NUMBER, //!< IV is equal to media sequence number
} HlsIVMode;

/**
 * @class HlsKey
 * @brief It represents instance of a key, and define its state (It can be not downloaded).
 */
typedef struct HlsKey_ *HlsKey;

/**
 * @class HlsKeyProvider
 * @brief It is responsible for key management. It receives keys URI and it is responsible for downloading the key's data if it is needed.
 */
typedef struct HlsKeyProvider_ *HlsKeyProvider;

/**
 * @class HlsKeyVariant
 * @brief It keeps set of keys with common bitrate variant. It also keeps shared information, which are common for all keys at variant.
 */
typedef struct HlsKeysVariant_* HlsKeysVariant;

/**
 * @struct key_provider_callbacks
 * @brief Key provider callbacks are used to notify listener about key changes
 */
struct key_provider_callbacks {
    ///empty key object created
    void (*key) (void* hlsContParser, SvSampleEncryptionInfo key_);
    ///Key download error
    void (*error) (void* hlsContParser);
};

/**
 * @class HlsKeyAttributes
 * @brief It stores all key's information which are read from HLS file. It gives complete information what type of key will be downloaded and from where.
 */
typedef struct HlsKeyAttributes_ {
    /// Super class
    struct SvObject_ super_;
    /// Encryption method - it defines key format.
    HlsEncMethod encMethod;
    /// URI of a key.
    SvURI keyURI;
    /// IV mode
    HlsIVMode ivMode;
    /// IV which is needed for AES-CBC algorithm
    uint8_t* iv;
} *HlsKeyAttributes;

/**
 * @brief It creates instance of hls key provider
 * @param hlsContParser hls content parser instance
 * @param hlsMediaProvider hlsMediaProvider instance.
 * @param httpEngine HTTP Client Engine
 * @return hlsKeyProvider instance. In case of error it returns NULL.
 */
extern HlsKeyProvider
HlsKeyProviderCreate(void* hlsContParser,
                     HlsMediaProvider hlsMediaProvider,
                     SvHTTPClientEngine httpEngine,
                     SvDRMInfo drmInfo);

/**
 * @brief It is used for setting up listener and its callbacks.
 * @param self hlsKeyProvider handler
 * @param owner listener instance
 * @param hlsContParserCallbacks structure filled with
 */
extern void
HlsKeyProviderInitOwner(HlsKeyProvider self,
                        void* owner,
                        const struct cont_parser_callbacks* hlsContParserCallbacks);
/**
 * @brief It adds key parsed from hls file.
 * @param self hlsKeyProvider handler
 * @param key which was read from hls file
 */
extern void
HlsKeyProviderAddKey(HlsKeyProvider self,
                     HlsKey key);

/**
 * @brief It adds key variant.
 * @param self hlsKeyProvider handler
 * @param hlsKeyVariant hlsKeyVariant instance
 */
extern void
HlsKeyProviderAddKeyVariant(HlsKeyProvider self,
                            HlsKeysVariant hlsKeyVariant);

/**
 * @brief It returns time how long it took to download last key.
 * @param self hlsKeyProvider handler
 * @return time in milliseconds
 */
extern int
HlsKeyProviderGetDownloadTimeMs(HlsKeyProvider self);

/**
 * @brief Setter method for callbacks
 * @param self hlsKeyProvider handler
 * @param keyCallback callback used for notification about keys
 * @param errorCallback callback used for notification about errors
 */
extern void
HlsKeyProviderSetCallbacks(HlsKeyProvider self,
                           void* keyCallback,
                           void* errorCallback);

/**
 * @brief It optimize key's data stored in selected variant. Generally it compress keys URI's
 * @param self hlsKeyProvider handler
 * @param keyVariantId identification number of selected variant
 */
extern void
HlsKeyProviderOptimizeKeyCommonData(HlsKeyProvider self,
                                    int keyVariantId);
/**
 * @brief It returns chosen hlskey instance
 * @param self hlsKeyProvider handler
 * @param keyId identification number of selected key
 * @param variantId identification number of selected variant
 * @return hlsKey instance or NULL if key can not be found
 */
extern HlsKey
HlsKeyProviderGetKey(HlsKeyProvider self,
                     int keyId,
                     int variantId);

/**
 * @brief It changes current bitrate variant. The keys can be downloaded only from current variant
 * @param self hlsKeyProvider handler
 * @param idx identification number of variant
 */
extern void
HlsKeyProviderSetCurrentVariant(HlsKeyProvider self,
                                int idx);

/**
 * @brief It cleans keys in current bitrate variant.
 * @param self hlsKeyProvider handler
 */
extern void
HlsKeyProviderCleanCurrentVariant(HlsKeyProvider self);
/**
 * @brief It removes current bitrate variant.
 * @param self hlsKeyProvider handler
 * @param idx identification number of variant
 */
extern void
HlsKeyProviderRemoveCurrentVariant(HlsKeyProvider self);

/**
 * @brief It returns current variant identification number
 * @param self hlsKeyProvider handler
 * @return identification number of current variant
 */
extern int
HlsKeyProviderGetCurrentVariant(HlsKeyProvider self);

/**
 * @brief Request for next key. If key will be ready, the key notification callback will be called.
 * @param self hlsKeyProvider handler
 * @param keyId identification number of requested key
 * @param mediaFileIdx identification number of media file
 */
extern void
HlsKeyProviderRequestNextKey(HlsKeyProvider self,
                             int keyId,
                             int mediaFileIdx);

/**
 * @brief Set current playback speed.
 * @param self hlsKeyProvider handler
 * @param playSpeed
 */
extern void
HlsKeyProviderSetPlaySpeed(HlsKeyProvider self,
                           double playSpeed);

/**
 * @brief Set mode of playback.
 * @param self hlsKeyProvider handler
 * @param isLive if true the playback is in live mode. If false the playback in pvr mode
 */
extern void
HlsKeyProviderSetLivePlayMode(HlsKeyProvider self,
                              bool isLive);

/**
 * @brief Creates instance of hlsKeyAttribute class.
 * @return handler of hlsKeyAttribute
 */
extern HlsKeyAttributes
HlsKeyAttributesCreate(void);

/**
 * @brief Constructor of HlsKey class
 * @param hlsKeyAttributes
 * @return HlsKey instance
 */
extern HlsKey
HlsKeyCreate(HlsKeyAttributes hlsKeyAttributes);

/**
 * @brief Constructor of HlsKeysVariant.
 * @return HlsKeysVariant instance
 */
extern HlsKeysVariant
HlsKeysVariantCreate(void);

/**
 * @brief Get encryption method associated with key
 * @param self hlsKey handler
 * @return encryption method
 */
extern HlsEncMethod
HlsKeyGetEncMethod(HlsKey self);

/**
 * @brief Set variant id number associated with number. Key variant keeps some shared and common information og keys.
 * @param self hlsKey handler
 * @param keyVariantId
 */
extern void
HlsKeySetKeyVariantId(HlsKey self,
                         int keyVariantId);
/**
 * @brief Getter for identification number of key
 * @param self hlsKey handler
 * @return identification number of key
 */
extern int
HlsKeyGetId(HlsKey self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
