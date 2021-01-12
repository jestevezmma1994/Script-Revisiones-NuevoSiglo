/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVIEWRIGHT_IPTV_FINGERPRINT_H_
#define QBVIEWRIGHT_IPTV_FINGERPRINT_H_

/**
 * @file QBViewRightIPTVFingerprint.h QBViewRightIPTVFingerprint API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTVFingerprint QBViewRightIPTVFingerprint
 * @ingroup QBViewRightIPTV
 * @{
 **/

/**
 * @brief Defines horizontal position of fingerprint on the screen
 */
typedef enum QBViewRightIPTVFingerprintPositionHorizontal_e {
    QBViewRightIPTVFingerprintPositionHorizontal_Left,    /**< Left side of the screen */
    QBViewRightIPTVFingerprintPositionHorizontal_Centered,/**< Center of the screen */
    QBViewRightIPTVFingerprintPositionHorizontal_Right    /**< Right side of the screen */
} QBViewRightIPTVFingerprintPositionHorizontal;

/**
 * @brief Defines vertical position of finger on the screen
 */
typedef enum QBViewRightIPTVFingerprintPositionVeritical_e {
    QBViewRightIPTVFingerprintPositionVeritical_Top,     /**< Top of the screen */
    QBViewRightIPTVFingerprintPositionVeritical_Centered,/**< Center of the screen */
    QBViewRightIPTVFingerprintPositionVeritical_Bottom   /**< Bottom of the screen */
} QBViewRightIPTVFingerprintPositionVertical;

/**
 * @brief Defines type of the fignerprint. It indicates what should be shown on the screen
 */
typedef enum QBViewRightIPTVFingerprintType_e {
    QBViewRightIPTVFingerprintType_Test,                  /**< Show simple test message */
    QBViewRightIPTVFingerprintType_Logo,                  /**< Show customers logo stored at persistent memory */
    QBViewRightIPTVFingerprintType_UniqueDeviceIdentifier,/**< Show unique device identifier */
    QBViewRightIPTVFingerprintType_LocalTime,             /**< Show local time */
    QBViewRightIPTVFingerprintType_Version,               /**< Show library version */
    QBViewRightIPTVFingerprintType_Unknown                /**< Unknown type of fingerprint do not show anything */
} QBViewRightIPTVFingerprintType;

/**
 * @brief Class declaration of QBViewRightIPTVFingerprint
 * For more information about fingerprinting in ViewrightIPTV please refer to ENG-64 OSD Fingerprinting Integration Guidelines_RELEASED
 */
typedef struct QBViewRightIPTVFingerprint_s* QBViewRightIPTVFingerprint;

/**
 * @brief Constructor of QBViewRightIPTVFingerprint. It parses data received from VMX library
 *
 * @param[in] fingerprintData fingerprint data received from library
 * @param[in] localData local data received from library
 * @return instance of QBViewRightIPTVFingerprint in case of success or NULL in case of error
 */
QBViewRightIPTVFingerprint QBViewRightIPTVFingerprintCreate(SvData fingerprintData, SvData localData);

/**
 * @brief It returns position of fingerprint
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @param[out] horizontal horizontal position of fingerprint
 * @param[out] vertical vertical position of fingerprint
 */
void QBViewRightIPTVFingerprintGetPosition(QBViewRightIPTVFingerprint self, QBViewRightIPTVFingerprintPositionHorizontal *horizontal, QBViewRightIPTVFingerprintPositionVertical *vertical);

/**
 * @brief Gets type of fingerprint
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @return type of fingerprint
 */
QBViewRightIPTVFingerprintType QBViewRightIPTVFingerprintGetType(QBViewRightIPTVFingerprint self);

/**
 * @brief Gets fingerprint duration in seconds
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @return duration of fingerprint in seconds.
 */
unsigned int QBViewRightIPTVFingerprintGetDuration(QBViewRightIPTVFingerprint self);

/**
 * @brief Gets version of the library parsed from fingerprint data
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @return version of the library, it can be NULL
 */
SvString QBViewRightIPTVFingerprintGetParsedLibraryVersion(QBViewRightIPTVFingerprint self);

/**
 * @brief Gets unique identifier of STB
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @return unique identifier of STB, it can be NULL
 */
SvString QBViewRightIPTVFingerprintGetUniqueId(QBViewRightIPTVFingerprint self);

/**
 * @brief Gets string of local time received from ViewRightLibrary
 * @param[in] self instance of QBViewRightIPTVFingerprint
 * @return string which represent local time
 */
SvString QBViewRightIPTVFingerprintGetLocalTime(QBViewRightIPTVFingerprint self);

/**
 * @}
 **/
#ifdef __cplusplus
}
#endif

#endif //QBVIEWRIGHT_IPTV_FINGERPRINT_H_
