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

#ifndef QB_VIEWRIGHTIPTV_USER_NOTIFICATIONS_H_
#define QB_VIEWRIGHTIPTV_USER_NOTIFICATIONS_H_

/**
 * @file QBViewRightIPTVUserNotifications.h QBViewRightIPTVUserNotifications API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBViewRightIPTVInterface.h>
#include <SvFoundation/SvType.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTVUserNotifications QBViewRightIPTVUserNotifications
 * @ingroup QBViewRightIPTV
 * @{
 */

/**
 * @brief Type definition of QBViewRightIPTVChannelStatusNotification
 * It is used to notify CAS listener about status of current channel
 */
typedef struct QBViewRightIPTVChannelStatusNotification_s * QBViewRightIPTVChannelStatusNotification;

/**
 * @brief GetType method of QBViewRightIPTVChannelStatusNotification
 * @return SvType of QBViewRightIPTVChannelStatusNotification
 */
SvType QBViewRightIPTVChannelStatusNotification_getType(void);

/**
 * @brief Constructor of QBViewRightIPTVChannelStatusNotification
 * @param[in] serviceIdx service index of current channel
 * @param[in] status status of current channel
 * @return instance of QBViewRightIPTVChannelStatusNotification
 */
QBViewRightIPTVChannelStatusNotification QBViewRightIPTVChannelStatusNotificationCreate(int serviceIdx, const QBViewRightIPTVChannelStatus status);

/**
 * @brief Gets status of channel
 * @param self instance of QBViewRightIPTVChannelStatusNotification
 * @return status of channel
 */
QBViewRightIPTVChannelStatus QBViewRightIPTVChannelStatusNotificationGetStatus(QBViewRightIPTVChannelStatusNotification self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
