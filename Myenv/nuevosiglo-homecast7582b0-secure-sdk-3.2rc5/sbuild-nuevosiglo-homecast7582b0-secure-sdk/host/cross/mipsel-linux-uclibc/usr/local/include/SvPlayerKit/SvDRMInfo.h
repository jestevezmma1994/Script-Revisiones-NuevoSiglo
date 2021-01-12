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

#ifndef SV_DRM_INFO_H_
#define SV_DRM_INFO_H_

/**
 * @file SvDRMInfo.h Abstract DRM information class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDRMInfo Abstract DRM information class
 * @ingroup SvPlayerKit
 * @{
 **/

#define DRM_INIT_BEFORE_INPLUGIN 0
#define DRM_INIT_AFTER_INPLUGIN  1

/**
 * DRM Encryption level enum
 * @typedef SvDRMInfoEncryptionLevel
 * @brief It is used to give information how TS is encrypted.
 */
typedef enum SvDRMInfoEncryptionLevel_e {
    SvDRMInfoEncryptionLevel_None,      //!< TS is not encrypted
    SvDRMInfoEncryptionLevel_Payload,   //!< TS payload is encrypted, the header is not encrypted
    SvDRMInfoEncryptionLevel_TsPacket,  //!< Whole data are encrypted.
    SvDRMInfoEncryptionLevel_File,      //!< Whole media-file is encrypted and needs to be decrypted before any processing
} SvDRMInfoEncryptionLevel;

/**
 * DRM information class.
 * @class SvDRMInfo
 * @extends SvObject
 **/
typedef struct SvDRMInfo_ {
    /// super class
    struct SvObject_ super_;

    /// DRM plugin identifier
    int    drm_id;
    /// flag set to DRM_INIT_BEFORE_INPLUGIN or DRM_INIT_AFTER_INPLUGIN
    int    init_before;

    /// defines the level of encryption see SvDRMInfoEncryptionLevel
    SvDRMInfoEncryptionLevel encryptionLevel;

    /// @c true if DRM does not descramble but just reads ECMs from the TS stream
    bool   ecm_only;
} *SvDRMInfo;


/**
 * Get runtime type identification object representing DRM information class.
 *
 * @return DRM information class
 **/
extern SvType
SvDRMInfo_getType(void);

/**
 * Initialize an abstract DRM information object.
 *
 * @memberof SvDRMInfo
 *
 * @param[in] self      DRM information object handle
 * @param[in] drm_id    DRM plugin ID
 * @param[in] init_before flag informing if DRM plugin should be initialized before or after opening an input plugin
 * @param[in] encryptionLevel It defines level of encryption used in drm solution see SvDRMInfoEncryptionLevel
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvDRMInfo
SvDRMInfoInit(SvDRMInfo self,
              int drm_id,
              int init_before,
              SvDRMInfoEncryptionLevel encryptionLevel,
              SvErrorInfo *errorOut);

/**
 * Get DRM plugin ID.
 *
 * @memberof SvDRMInfo
 *
 * @param[in] self      DRM information object handle
 * @return              DRM plugin ID
 **/
extern int
SvDRMInfoGetDrmID(SvDRMInfo self);

/**
 * Get DRM encryption level
 *
 * @memberof SvDRMInfo
 *
 * @param[in] self      DRM information object handle
 * @return DRM encryption level
 */
extern SvDRMInfoEncryptionLevel
SvDRMInfoGetEncryptionLevel(SvDRMInfo self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
