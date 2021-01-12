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

#ifndef QBVIEWRIGHT_IPTV_LISTENER_H_
#define QBVIEWRIGHT_IPTV_LISTENER_H_

/**
 * @file QBViewRightIPTVListener.h QBViewRightIPTVListener API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <QBViewRightIPTVInterface.h>
#include <QBViewRightIPTVUserMessage.h>
#include <QBViewRightIPTVParentalControlRequest.h>
#include <QBViewRightIPTVUserNotifications.h>
#include <QBViewRightIPTVFingerprint.h>
#include <QBViewRightIPTVCopyControl.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTVListener QBViewRightIPTVListener
 * @ingroup QBViewRightIPTV
 * @{
 */

/**
 * @brief Listener of QBViewRightIPTV. An instance which implements that interface can receive notifications about changes in
 *        QBViewRightIPTVLibrary
 */
typedef struct QBViewRightIPTVListener_s {
    /**
     * @brief Method is invoked each time QBViewRightIPTV changes its state
     * @param self_ instance of listener
     * @param state new state of QBViewRightIPTV
     * @param errorCode if state is equals to QBViewRightIPTVState_Error, errorCode contains error code returned by library
     */
    void (*stateChanged) (SvObject self_, QBViewRightIPTVState state, int errorCode);

    /**
     * @brief Method is invoked each time new user message is delivered
     * @param self_ instance of listener
     * @param userMessage instance of @ref QBViewRightIPTVUserMessage
     */
    void (*userMessage) (SvObject self_, QBViewRightIPTVUserMessage userMessage);

    /**
     * @brief Method is invoked each time new fingerprint is delivered
     * @param self_ instance of listener
     * @param fingerprint instance of @ref QBViewRightIPTVFingerprint
     */
    void (*fingerprint) (SvObject self_, QBViewRightIPTVFingerprint fingerprint);

    /**
     * @brief Method is invoked each time new copy control msg is delivered
     * @param self_ instance of listener
     * @param cc instance of @ref QBViewRightIPTVCopyControl
     */
    void (*copyControl)(SvObject self_, QBViewRightIPTVCopyControl cc);

    /**
     * @brief Method is invoked each time parental control request is delivered
     * @param self_ instance of listener
     * @param request instance of @ref QBViewRightIPTVParentalControlRequest
     */
    void (*parentalControlRequest) (SvObject self_, QBViewRightIPTVParentalControlRequest request);

    /**
     * @brief Method is invoked each time channel status has been changed.
     * @param self_ instance of listener
     * @param channelStatus instance of @ref QBViewRightIPTVChannelStatusNotification
     */
    void (*channelStatus) (SvObject self_, QBViewRightIPTVChannelStatusNotification channelStatus);
} *QBViewRightIPTVListener;

/**
 * @brief GetInterface method of QBViewRightIPTVListener
 * @return SvInterface instatnce of QBViewRightIPTVListener
 */
SvInterface QBViewRightIPTVListener_getInterface(void);

/**
 * @}
 **/
#ifdef __cplusplus
}
#endif

#endif
