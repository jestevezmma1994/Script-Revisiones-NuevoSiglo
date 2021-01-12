/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_OTP_H_
#define QB_PLATFORM_OTP_H_

/**
 * @file QBPlatformOTP.h One Time Programming control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <QBPlatformHAL/QBPlatformTypes.h>
#include <stdbool.h>

/**
 * @defgroup QBPlatformOTP One Time Programming
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Result of an asynchronous call to a QBPlatformOTP function returned in @c argA field of @c QBPlatformEvent_otpChanged event
 **/
typedef enum {
    /// burning security fuses succeeded (from QBPlatformOTPSetSecurityFuses())
    QBPlatformEventOtpChangedStatus_success,
    /// burning security fuses failed (from QBPlatformOTPSetSecurityFuses())
    QBPlatformEventOtpChangedStatus_failure,
    /// OTP is locked (from QBPlatformOTPIsSecured())
    QBPlatformEventOtpChangedStatus_locked,
    /// OTP is unlocked (from QBPlatformOTPIsSecured())
    QBPlatformEventOtpChangedStatus_unlocked,
    /// failed to read status of OTP (from QBPlatformOTPIsSecured())
    QBPlatformEventOtpChangedStatus_error,
    /// number of @c QBPlatformEventOtpChangedStatus distinct values
    QBPlatformEventOtpChangedStatus_cnt
} QBPlatformEventOtpChangedStatus;

/**
 * Check if QBPlatformOTP API is supported by QBPlatformHAL.
 *
 * Check if QBPlatformOTP API is supported by QBPlatformHAL on this particular platform. This function must be called prior to any
 * other function from this module; if it returns false then no other function from this module must be called. This function might
 * be called many times.
 *
 * @return @c true if QBPlatformHAL supports QBPlatformOTP API, @c false if it does not
 **/
bool QBPlatformOTPIsSupported(void);

/**
 * Burn OTP security fuses.
 *
 * Burn some in-SoC OTP security fuses; which fuses are to be burnt is defined in platform-specific fuse map. This operation is
 * irreversible.
 *
 * This function might be used both synchronously and asynchronously depending on @c async flag. If @c async parameter is set to
 * false then this function blocks until burning of fuses operation either completes or fails, otherwise the burning of fuses
 * process is performed in the background and the actual result will be returned asynchronously by @link QBPlatformEvent_otpChanged
 * @endlink event; note that this function might also return an error in the latter case and no @a platform @a event will then
 * appear.
 *
 * @param[in] async asynchronous mode flag
 * @return @c 0 on success, @c -1 in case of an error
 **/
int QBPlatformOTPSetSecurityFuses(bool async);

/**
 * Check status of OTP security fuses.
 *
 * Return status of OTP security fuses.
 * This function might be used both synchronously and asynchronously depending on @c async flag in the same manner as
 * link QBPlatformOTPSetSecurityFuses().
 *
 * @return      @c 0 in synchronous mode: success, in synchronous mode: OTP flash area is unlocked,
 *              @c 1 OTP flash area it is locked (synchronous mode only),
 *              @c -1 an error occurred
 **/
int QBPlatformOTPIsSecured(bool async);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif          // QB_PLATFORM_OTP_H
