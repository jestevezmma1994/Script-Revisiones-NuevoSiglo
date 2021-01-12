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

#ifndef SV_DRM_SYSTEM_INFO_
#define SV_DRM_SYSTEM_INFO_

/**
 * @file SvDrmSystemInfo.h DRM system info class API
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDrmSystemInfo DRM system info class
 * @ingroup SvPlayerKit
 * @{
 **/

/**
 * DRM system types.
 **/
typedef enum {
    SvDrmSystemType_Invalid = 0,
    SvDrmSystemType_PlayReady,
    SvDrmSystemType_PlainAES,
    SvDrmSystemType_ViewRightWeb,
    SvDrmSystemType_CENC,
    SvDrmSystemType_Count,
} SvDrmSystemType;

/**
 * Drm encryption key state type
 **/
typedef enum {
    SvDrmSystemKeyState_OK, /// ready to process data
    SvDrmSystemKeyState_Waiting, /// waiting for download
    SvDrmSystemKeyState_Empty, /// empty key, disable decryption
    SvDrmSystemKeyState_Failed, /// key download failed
} SvDrmKeyState;

/**
 * PlayReady system specific information.
 **/
struct SvPlayReadyDrmSystemInfo_ {
    /// length of @a protectionHeader, in bytes
    uint32_t protectionHeaderLen;
    /// protection header, usually from "ProtectionHeader" in Smooth Streaming manifest
    uint8_t *protectionHeader;
};

/**
 * DRM system information class.
 * @class SvDrmSystemInfo
 * @extends SvObject
 **/
typedef struct SvDrmSystemInfo_ {
    /// super class
    struct SvObject_ super_;

    /// DRM system type
    SvDrmSystemType type;

    /// system specific information
    union {
        /// PlayReady information, valid if type == SvDrmSystemType_PlayReady
        struct SvPlayReadyDrmSystemInfo_ playready;
    };
} *SvDrmSystemInfo;


/**
 * Get runtime type identification object representing DRM system information class.
 *
 * @return DRM system information class
 **/
extern SvType
SvDrmSystemInfo_getType(void);


/**
 * PlayReady encryption entity.
 **/
struct SvMP4SampleEncryptionEncEntity_ {
    uint32_t bytesClear;
    uint32_t bytesEnc;
};

/**
 * MP4 specific sample encryption information.
 **/
struct SvMP4SampleEncryption_ {
    /// initialization vector length, in bytes
    uint16_t ivLen;
    /// initialization vector
    uint8_t *iv;

    /// size of @a map array
    uint32_t entityCnt;
    /// array of encryption entities
    struct SvMP4SampleEncryptionEncEntity_ *map;
};

/**
 * Typedef of SvSampleEncryptionInfo - DRM sample encryption information class.
 **/
typedef struct SvSampleEncryptionInfo_ *SvSampleEncryptionInfo;

/**
 * PlainAES specific sample encryption information
 **/
struct drm_encryption_key_info_callbacks {
    ///Key data was loaded
    void (*stateChanged) (void* owner, SvSampleEncryptionInfo sampleEncryptionInfo);
};
/**
 * AES specific sample encryption information.
 */
struct SvPlainAESSampleEncryption_ {
    /// Owner of key
    void* owner;
    /// callback called when state of key is changed
    struct drm_encryption_key_info_callbacks callbacks;
    ///Drm encryption key state
    SvDrmKeyState keyState;
    /// initialization vector length
    uint16_t ivLen;
    /// initialization vector
    uint8_t* iv;
    /// key vector length
    uint16_t keyLen;
    /// key vector
    uint8_t* key;
};

/**
 * DRM sample encryption information class.
 * @class SvSampleEncryptionInfo
 * @extends SvObject
 **/
struct SvSampleEncryptionInfo_ {
    /// super class
    struct SvObject_ super_;

    /// DRM system type
    SvDrmSystemType type;

    /// system specific information
    union {
        /// MP4 information, valid if type == SvDrmSystemType_PlayReady or SvDrmSystemType_CENC
        struct SvMP4SampleEncryption_ mp4;
        /// HLS encryption information, valid if type == SvDrmSystemType_PlainAES
        struct SvPlainAESSampleEncryption_  plainAES;
    };
};


/**
 * Get runtime type identification object representing DRM sample encryption information class.
 *
 * @return DRM sample encryption information class
 **/
extern SvType
SvSampleEncryptionInfo_getType(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
