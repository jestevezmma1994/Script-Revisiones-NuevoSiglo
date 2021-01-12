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

#ifndef QB_VIEWRIGHTIPTV_USER_MESSAGE_H_
#define QB_VIEWRIGHTIPTV_USER_MESSAGE_H_

/**
 * @file QBViewRightIPTVUserMessage.h QBViewRightIPTVUserMessage API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBViewRightIPTVInterface.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvTime.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewRightIPTVUserMessage QBViewRightIPTVUserMessage
 * @ingroup QBViewRightIPTV
 * @{
 *
 **/

/**
 * @brief Type of user message. It indicates if message can be removable or fixed.
 */
typedef enum QBViewRightIPTVUserMessageType_e
{
    QBViewRightIPTVUserMessageType_Removable = 0x00,/**< Message can be removed by user */
    QBViewRightIPTVUserMessageType_Fixed = 0x01,    /**< Message can not be removed by user */
    QBViewRightIPTVUserMessageType_Max,             /**< Enum max value */
} QBViewRightIPTVUserMessageType;

/**
 * @brief Type definition of QBViewRightIPTVUserMessage
 *
 * User messages are used by operator to send some informations via ECM or EMM to user.
 * The are displayed on the screen in TV and PVR contexts.
 *
 * Verimatrix do not defines format or appearance of On Screen Messages. They can be defined by us.
 *
 * Format is defined as follows:
 * 1st byte - defines type of the OSM look at QBViewRightIPTVUserMessageType for details
 * 2nd byte - It is duration expressed in seconds. It can have values from 0 to 255
 * 3th - * bytes - There is user text encoded in UTF-8
 *
 * Example OSM : "\x01\x10This is test OSM"
 * - It is fixed OSM - can not be removed by user
 * - It's duration is equal to 16 seconds
 * - It contains text "This is test OSM"
 * *
 */
typedef struct QBViewRightIPTVUserMessage_s * QBViewRightIPTVUserMessage;

/**
 * @brief GetType method of QBViewRightIPTVUserMessage
 * @return SvType of QBViewRightIPTVUserMessage
 */
SvType QBViewRightIPTVUserMessage_getType(void);

/**
 * @brief Constructor of QBViewRightIPTVUserMessage, it can parse data returned by ViewRight library
 * @param messageSize length of onScreenMessage data
 * @param onScreenMessage On Screen Message in proper format
 * @return instance of QBViewRightIPTVUserMessage or NULL in case of parsing error
 */
QBViewRightIPTVUserMessage QBViewRightIPTVUserMessageCreate(const unsigned int messageSize,  const uint8_t* onScreenMessage);

/**
 * @brief Gets duration of user message expressed in SvTime
 * @param self instance of QBViewRightIPTVUserMessage
 * @return duration of user message as SvTime
 */
SvTime QBViewRightIPTVUserMessageGetDuration(QBViewRightIPTVUserMessage self);

/**
 * @brief Gets type of message.
 * @param self instance of QBViewRightIPTVUserMessage
 * @return type of user message see QBViewRightIPTVUserMessageType for details
 */
QBViewRightIPTVUserMessageType QBViewRightIPTVUserMessageGetType(QBViewRightIPTVUserMessage self);

/**
 * @brief Gets text of user message
 * @param self instance of QBViewRightIPTVUserMessage
 * @return string with text which supposed to be shown on a screen
 */
SvString QBViewRightIPTVUserMessageGetText(QBViewRightIPTVUserMessage self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
