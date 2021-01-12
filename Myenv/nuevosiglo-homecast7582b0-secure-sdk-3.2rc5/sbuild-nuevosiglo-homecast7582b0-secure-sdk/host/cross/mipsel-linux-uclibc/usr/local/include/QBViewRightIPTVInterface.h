/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_VIEWRIGHT_IPTV_INTERFACE_H_
#define QB_VIEWRIGHT_IPTV_INTERFACE_H_

/**
 * @file QBViewRightIPTVInterface.h QBViewRightIPTV API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBViewRightIPTVParentalControlRequest.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTV QBViewRightIPTV CAS
 * @ingroup DRM
 * @{
 *
 **/

/**
 * @typedef QBViewRightIPTV QBViewRightIPTV
 * @brief ViewRightIPTV is a condition access system delivered by Verimatrix.
 *
 * General idea of that system is that encrypted ECM's are transported via DVB TS. Keys for those
 * encrypted ECM are delivered from VCAS server. Those keys are valid for several hours - it depends of VCAS server settings
 *
 * Current implementation supports hardware descrambling mode of ViewRightIPTV library.
 * Supported algorithms are:
 * - DVB-CSA
 * - AES-CBC
 * - AES-EBC-T
 *
 * Not supported algorithms:
 *  - RC4
 *  - AES_EBC-L
 *
 * It can handle EMM's send inband or by multicats from VCAS server.
 *
 * Library generally can work in two states: offline and online.
 * If online mode is active full functionality is enabled.
 * When library is in offline mode functionality is limited:
 * - EMM's are not handled even if they are send inband.
 * - Decryption is enabled only if keys for content has been already cached in persistent memory.
 *
 * PVR functionality:
 * PVR is working with our retention mode. During PVR playback recored ECM's are send to VermiatrixIPTV library.
 * If those ECM's are older then several hours, library might not have keys for those ECM's. In that case it sends request
 * to VCAS server, and obtains proper key (it is called PVR key) for such ECM.
 *
 * VOD functionality:
 * VOD streams are encrypted earlier by VCAS server, and stored at external content delivery system.
 * When STB starts to play VOD content it detects ECM's. Library receives those ECM and basing on that it detects
 * that is VOD content. As in PVR scenario it sends request to VCAS server for proper key (it is called VOD key).*
 *
 * Initialization flow of library looks like this:
 * - Library enters offline mode at start - it has to be succeeded, if not it is a critical error.
 * - Then library tries to enter online mode. It can only be succeeded, when STB is online and VCAS server is working.
 *   If it fails it is not critical error, STB will retry connection to VCAS server after some time.
 * - In case of first connection to VCAS, device is provisioned.
 * - At the last step UpdateKeys is called at library. It start thread inside library which is responsible for key management.
 *
 * Provisioning device:
 * During first connection the STB is provisioned. During that process client configuration is retrieved from VCAS server.
 * Configuration file is encrypted and stored at persistent directory.
 * That process can be done many times.
 *
 * Fingerprinting:
 * System supports fingerprintig. For more details read "ENG-64 OSD Fingerprinting Integration Guidelines_RELEASED" document
 *
 * On screan messages:
 * System supports sending of OSMs, but it not specificity format and appearance of them. It can be defined by us.
 *
 * Parental control:
 * Library supports parental control. In general words it only notifies that maturity of user has to be checked.
 * It do not support checking PIN, it do not store any PIN also.
 *
 */

typedef struct QBViewRightIPTV_s * QBViewRightIPTV;

/**
 * @file QBViewRightIPTVInterface.h QBViewRightIPTVInterface common definitions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @brief Indicates state of a ViewRightIPTVLibrary
 */

typedef enum QBViewRightIPTVState_e
{
    QBViewRightIPTVState_Created,       /**< Library has been created but is not initialized. */
    QBViewRightIPTVState_Initialization,/**< Initialization process is pending. */
    QBViewRightIPTVState_Offline,       /**< Connection with VCAS has not been established, functionality is limited. */
    QBViewRightIPTVState_Online,        /**< Established connection with VCAS, full functionality is enabled. */
    QBViewRightIPTVState_Closed,        /**< Library has been closed. */
    QBViewRightIPTVState_Error,         /**< Fatal error has occurred. */
} QBViewRightIPTVState;

/**
 * @brief Indicates state of a channel
 */
typedef enum QBViewRightIPTVChannelStatus_e
{
    QBViewRightIPTVChannelStatus_Unknown,        /**< Unknown status. */
    QBViewRightIPTVChannelStatus_OK,             /**< Everything is ok, decryption can be processed. */
    QBViewRightIPTVChannelStatus_PinRequired,    /**< Maturity of user has to be checked, content should not be visible. */
    QBViewRightIPTVChannelStatus_KeyNotAvailable,/**< Could be not authorized or network error getting key. */
    QBViewRightIPTVChannelStatus_Blackout,       /**< Network blackout. */
    QBViewRightIPTVChannelStatus_Other           /**< Decryption is disabled due to not receiving EMMs. */
} QBViewRightIPTVChannelStatus;

/**
 * @brief QBViewRightIPTV interface which can be used in application
 */
typedef struct QBViewRightIPTVInterface_ {

    /**
     * Adds the listener to QBViewRightIPTV
     * @param[in] self_ QBViewRightIPTV instance
     * @param[in] listener object which listens to changes in QBViewRightIPTV instance
     */
    void (*addListener)(SvObject self_, SvObject listener);

    /**
     * Removes the listener to QBViewRightIPTV
     * @param[in] self_ QBViewRightIPTV instance
     * @param[in] listener object which listens to changes in QBViewRightIPTV instance
     */
    void (*removeListener)(SvObject self_, SvObject listener);

    /**
     * Indicates the end of processing parental control request
     * @param[in] self_ QBViewRightIPTV instance
     * @param[in] request request struct
     */
    void (*parentalControlRequestDone)(SvObject self_, QBViewRightIPTVParentalControlRequest request);

    /**
     * Initializes CAS implementation
     * @param[in] self_ QBViewRightIPTV instance
     * @param[in] ratingLevel the value of rating level
     */
    void (*parentalControlSetRatingLevel)(SvObject self_, uint8_t ratingLevel);

    /**
     * Notify ViewRightIPTV that time and date are actual
     * @param[in] self_ QBViewRightIPTV instance
     */
    void (*gotActualTimeAndDate)(SvObject self_);
} *QBViewRightIPTVInterface;

/**
 * @brief Get QBViewRightIPTVInterface
 * @return The interface of QBViewRightIPTV
 */
SvInterface QBViewRightIPTVInterface_getInterface(void);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_VIEWRIGHT_IPTV_INTERFACE_H_ */
