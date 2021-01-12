/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_EVENT_H_
#define QB_PLATFORM_EVENT_H_

/**
 * @file QBPlatformEvent.h API for notifying about platform events
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformEvent Generic platform events
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Types of platform events.
 **/
typedef enum {
    /// marker for invalid event type
    QBPlatformEvent_unknown = 0,
    /**
     * audio output muted or unmuted: @a argA contains audio output ID
     **/
    QBPlatformEvent_audioOutputMuted,
    /**
     * audio output volume changed: @a argA contains audio output ID
     **/
    QBPlatformEvent_volumeChanged,
    /**
     * audio output latency changed: @a argA contains audio output ID
     **/
    QBPlatformEvent_audioLatencyChanged,
    /**
     * audio output preferences changed: @a argA contains audio output ID
     **/
    QBPlatformEvent_audioPreferencesChanged,
    /**
     * HDCP status changed: @a argA contains HDMI output ID
     **/
    QBPlatformEvent_HDCPStatusChanged,
    /**
     * viewport changed: @a argA contains output ID, a@ argB conatins viewport information
     **/
    QBPlatformEvent_viewportChanged,
    /**
     * output changed: @a argA contains output ID
     **/
    QBPlatformEvent_outputChanged,
    /**
     * HDMI-CEC action: @a argA contains output ID, a@ argB contains CEC action @see QBPlatformCECAction
     **/
    QBPlatformEvent_cecAction,
    /**
     * OTP action: @a argA contains status of last function from QBPlatformOTP module called asynchronously;
     * @see enum QBPlatformEventOtpChangedStatus, QBPlatformOTPSetSecurityFuses(), QBPlatformOTPIsSecured()
     **/
    QBPlatformEvent_otpChanged,
} QBPlatformEvent;


/**
 * Platform event callback type.
 *
 * @param[in] prv       opaque pointer to callback function's private data
 * @param[in] event     event type (what happened)
 * @param[in] argA      first optional argument
 * @param[in] argB      second optional argument
 **/
typedef void (*QBPlatformEventCallback)(void *prv,
                                        QBPlatformEvent event,
                                        long long int argA,
                                        SvObject argB);

/**
 * Add platform event callback.
 *
 * This function registers a callback that will be called
 * to notify about platform events.
 *
 * @param[in] fn        callback function
 * @param[in] prv       opaque pointer to callback function's private data
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformAddEventCallback(QBPlatformEventCallback fn,
                           void *prv);

/**
 * Remove previously registered platform event callback.
 *
 * @param[in] fn        callback function
 * @param[in] prv       opaque pointer to callback function's private data
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformRemoveEventCallback(QBPlatformEventCallback fn,
                              void *prv);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
